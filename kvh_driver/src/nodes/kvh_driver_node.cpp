/**
 * @file	kvh_driver_node.cpp
 * @date	Feb 2, 2013
 * @author	Adam Panzica
 * @brief	Implementation details for KVHDriverNode class, and any other implementation needed for lvh_driver_node
 */

/*
 * LICENSE FILE
 */
//******************* SYSTEM DEPENDANCIES ****************//
//******************* LOCAL DEPENDANCIES ****************//
#include "kvh_driver/kvh_imu.h"
#include <device_driver_base/driver_util.h>
#include <sensor_msgs/Imu.h>
#include <device_driver_base/reconfigurable_device_driver.h>
#include "kvh_driver/KVHDriverConfig.h"
#include <nav_msgs/Odometry.h>
#include <tf/tf.h>
#include <ros/ros.h>
#include <kvh_driver/constants.h>
//*********************** NAMESPACES ********************//
using namespace kvh_driver;
using namespace device_driver;

#define CAL_SAMPLES 10000

class KVHDriver : public ReconfigurableDeviceDriver<kvh_driver::KVHDriverConfig>{
private:
	kvh_driver::IMU imu;

	std::string device_address_;
	std::string imu_frame_;
	std::string odom_frame_;

	double _rx, _ry, _rz, _x, _y, _z;//average values
	double rx, ry, rz, x, y, z, x_dot, y_dot, z_dot;

	ReconfigurableAdvertisePtr odom_pub_;
	ReconfigurableAdvertisePtr imu_pub_;

	kvh_driver::imu_data_t data;
	nav_msgs::Odometry msg;
	sensor_msgs::Imu imu_msg;

	bool time_init_;
	struct timeval start, current, last;

	bool did_calibration_;

public:
	void openDevice(){
		imu.open(device_address_);
		imu.config(false);
		time_init_ = false;
	}
	void closeDevice(){
		imu.close();
	}

	virtual void runningLoop(){
		if(!did_calibration_)
			calibrate();
		readSensor();
	}


	void calibrate(){
		_rx = _ry = _rz = _x = _y = _z = 0;
		rx = ry = rz = x = y = z = x_dot = y_dot = z_dot = 0;

		ROS_INFO("Calibrating...");
		for(int i = 0; i<CAL_SAMPLES;){

			try{
				imu.read_data(data);

				_rx += data.angleX;
				_ry += data.angleY;
				_rz += data.angleZ;

				_x += data.accelX;
				_y += data.accelY;
				_z += data.accelZ;
				++i;
			} catch(device_driver::CorruptDataException& e){
				ROS_WARN("Got corrupt message: %s", e.what());
			}
		}

		_rx /= CAL_SAMPLES;
		_ry /= CAL_SAMPLES;
		_rz /= CAL_SAMPLES;
		_x /= CAL_SAMPLES;
		_y /= CAL_SAMPLES;
		_z /= CAL_SAMPLES;

		_x += 1;

		ROS_INFO("Calibration complete");
		ROS_DEBUG("drx = %f", _rx);
		ROS_DEBUG("dry = %f", _ry);
		ROS_DEBUG("drz = %f", _rz);
		ROS_DEBUG("dx = %f", _x);
		ROS_DEBUG("dy = %f", _y);
		ROS_DEBUG("dz = %f", _z);

		did_calibration_ = true;
	}

	void readSensor(){
		try{
			imu.read_data(data);
			gettimeofday(&current, NULL);
			long diff_last = (current.tv_sec - last.tv_sec)*1000000 + (current.tv_usec - last.tv_usec);
			last = current;
			if(!time_init_){//if we don't have a valid last time then just skip this measurement
				time_init_ = true;
				return;
			}

			/*
			 * Integrate
			 */
			rx += (data.angleX-_rx) * diff_last / 1000000;
			ry += (data.angleY-_ry) * diff_last / 1000000;
			rz += (data.angleZ-_rz) * diff_last / 1000000;

			x_dot += (data.accelX-_x)*M_S_S_PER_G * diff_last / 1000000;
			y_dot += (data.accelY-_y)*M_S_S_PER_G * diff_last / 1000000;
			z_dot += (data.accelZ-_z)*M_S_S_PER_G * diff_last / 1000000;

			x += x_dot * diff_last / 1000000;
			y += y_dot * diff_last / 1000000;
			z += z_dot * diff_last / 1000000;



			static int i = 0;
			if((i++)>=50){
			  ROS_DEBUG("%f, %f, %f : %f, %f, %f\n", rx, ry, rz, x, y, z);
			  ROS_DEBUG("\tGyro: %f, %f, %f\n", data.angleX, data.angleY, data.angleZ);
			  ROS_DEBUG("\tAccel: %f, %f, %f\n", data.accelX, data.accelY, data.accelZ);


			  /*
			   * Create odometry message
			   */
			  msg.header.stamp = ros::Time::now();
			  msg.pose.pose.orientation = tf::createQuaternionMsgFromRollPitchYaw(rx, ry, rx);
			  //msg.pose.pose.position.x = x;
			  //msg.pose.pose.position.y = y;
			  //msg.pose.pose.position.z = z;
			  msg.pose.covariance[(constants::ODOM_X_STATE()-constants::ODOM_X_STATE())*7]  = 0.001;
			  msg.pose.covariance[(constants::ODOM_Y_STATE()-constants::ODOM_X_STATE())*7]  = 0.001;
			  msg.pose.covariance[(constants::ODOM_Z_STATE()-constants::ODOM_X_STATE())*7]  = 0.001;
			  msg.pose.covariance[(constants::ODOM_RX_STATE()-constants::ODOM_X_STATE())*7] = 99999;
			  msg.pose.covariance[(constants::ODOM_RY_STATE()-constants::ODOM_X_STATE())*7] = 99999;
			  msg.pose.covariance[(constants::ODOM_RZ_STATE()-constants::ODOM_X_STATE())*7] = 99999;
			  odom_pub_->publish(msg);

			  /*
			   * Create imu message
			   */
			  //imu_msg.orientation = tf::createQuaternionMsgFromRollPitchYaw(rx, ry, rz); don't do this cause robotpose ekf expects it to be world aligned
			  imu_msg.header.frame_id = "/base_footprint";	
			  imu_msg.orientation = tf::createQuaternionMsgFromRollPitchYaw(0, 0, rx);
			  imu_msg.orientation_covariance[(0)*4]  = 0.000001;
			  imu_msg.orientation_covariance[(1)*4]  = 0.01;
			  imu_msg.orientation_covariance[(2)*4]  = 0.01;
			  //imu_msg.angular_velocity.x = data.angleX-_rx;
			  //imu_msg.angular_velocity.y = data.angleY-_ry;
			  //imu_msg.angular_velocity.z = data.angleZ-_rz;
			  imu_msg.angular_velocity_covariance[(0)*4]  = 0.001;
			  imu_msg.angular_velocity_covariance[(1)*4]  = 0.001;
			  imu_msg.angular_velocity_covariance[(2)*4]  = 0.001;
			  //imu_msg.linear_acceleration.x = (data.accelX-_x)*M_S_S_PER_G;
			  //imu_msg.linear_acceleration.y = (data.accelY-_y)*M_S_S_PER_G;
			  //imu_msg.linear_acceleration.z = (data.accelZ-_z)*M_S_S_PER_G;
			  imu_msg.linear_acceleration_covariance[(0)*4]  = 99999;
			  imu_msg.linear_acceleration_covariance[(1)*4]  = 99999;
			  imu_msg.linear_acceleration_covariance[(2)*4]  = 99999;
			  imu_msg.header.stamp = ros::Time::now();
			  imu_pub_->publish(imu_msg);


			  i = 0;
			}
		} catch(device_driver::CorruptDataException& e){
			ROS_WARN("Got corrupt message: %s\n", e.what());
		}
	}



	KVHDriver() : ReconfigurableDeviceDriver(2000), imu(1000, false), device_address_("/dev/ttyUSB0"),
			imu_frame_("/imu"), odom_frame_("/odom"),
			time_init_(false), did_calibration_(false){
		_rx = _ry = _rz = _x = _y = _z = 0;
		rx = ry = rz = x = y = z = x_dot = y_dot = z_dot = 0;

		addDriverStateFunctions(device_driver_state::OPEN, &KVHDriver::openDevice, &KVHDriver::closeDevice, this);

		odom_pub_ = addReconfigurableAdvertise<nav_msgs::Odometry>(device_driver_state::RUNNING, "/kvh/odom", 5, false);
		imu_pub_ = addReconfigurableAdvertise<sensor_msgs::Imu>(device_driver_state::RUNNING, "/kvh/imu", 5, false);
	}
	virtual void reconfigureStopped(kvh_driver::KVHDriverConfig& config){
		device_address_ = config.device_address;
	}
	virtual void reconfigureOpen(kvh_driver::KVHDriverConfig& config){
		imu_pub_->setTopic(config.imu_topic);
		odom_pub_->setTopic(config.odom_topic);
		imu_frame_ = config.imu_frame;
		odom_frame_ = config.odom_frame;

		msg.child_frame_id = imu_frame_;
		msg.header.frame_id = odom_frame_;
	}
	virtual void reconfigureRunning(kvh_driver::KVHDriverConfig& config){
	}

};


int main(int argc, char **argv){
  ros::init(argc, argv, "kvh_driver");

  KVHDriver driver;
  driver.spin();

  ROS_INFO("Initializing KVH Device");

  return 0;
}


