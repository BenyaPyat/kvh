/**
 * @file	kvh_driver_node.cpp
 * @date	Feb 2, 2013
 * @author	parallels
 * @brief	//TODO fill in detailed discription here
 */

/*
* LICENSE FILE
*/
//******************* SYSTEM DEPENDANCIES ****************//
//******************* LOCAL DEPENDANCIES ****************//
#include<kvh_driver/kvh_driver_node.h>
//*********************** NAMESPACES ********************//
using namespace kvh_driver;

/*
void testCallback(kvh_driver::KVHDriverConfig& config, uint32_t levels)
{
	ROS_INFO_STREAM("Reconfigure Request: "<<config.device_address<<config.filter<<config.output_topic<<config.poll_rate<<config.update_frequency);
}
*/

int main(int argc, char **argv) {
	ros::init(argc, argv, "kvh_driver_node");
	ros::NodeHandle nh;

	/*
	dynamic_reconfigure::Server<kvh_driver::KVHDriverConfig> dr_server;
	dynamic_reconfigure::Server<kvh_driver::KVHDriverConfig>::CallbackType cb;
	cb = boost::bind(&testCallback, _1, _2);
	dr_server.setCallback(cb);
	ROS_INFO("Dynamic Reconfigure Bindings Active");
	*/

	ROS_INFO("kvh_driver_node <%s> up and running", nh.getNamespace().c_str());
	ros::spin();
	return 1;
}

