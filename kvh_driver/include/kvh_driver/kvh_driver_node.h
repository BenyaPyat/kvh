/**
 * @file	kvh_driver_node.h
 * @date	Feb 2, 2013
 * @author	parallels
 * @brief	//TODO fill in detailed discription here
 */

/*
* LICENSE FILE
*/

#ifndef KVH_DRIVER_NODE_H_
#define KVH_DRIVER_NODE_H_

//******************* SYSTEM DEPENDANCIES ****************//
#include<ros/ros.h>
#include<dynamic_reconfigure/server.h>
//******************* LOCAL DEPENDANCIES ****************//
//#include<kvh_driver/KVHDriverConfig.h>
//#include<kvh_driver/imufilter.hpp>
#include<kvh_driver/imu_filter.h>
#include<kvh_driver/odometryfilter.hpp>
//*********************** NAMESPACES ********************//
namespace kvh_driver
{

class KVHDriverNode
{
public:
	KVHDriverNode(ros::NodeHandle& nh);
	virtual ~KVHDriverNode();

private:

	void update(const ros::TimerEvent& event);

	bool stateToImu(const ColumnVector& state, const SymmetricMatrix& covar, sensor_msgs::Imu message) const;

	int covarIndexCalc(int r, int c);

	IMUFilter*       imu_filter_;
	OdometryFilter*  odo_filter_;
	ros::NodeHandle  nh_;
	ros::Publisher   odo_pub_;
	ros::Publisher   imu_pub_;

	ros::Duration    update_frequency_;
	ros::Timer       update_timer_;
};

} /* END KVH_DRIVER */


#endif /* KVH_DRIVER_NODE_H_ */
