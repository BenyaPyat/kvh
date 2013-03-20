#include <stdlib.h>
#include "kvh_driver/kvh_imu.h"

/**
 * A simple program to print out the streamed data from the sensor
 */

int main(){
	kvh_driver::IMU imu;
	imu.open("/dev/ttyUSB0");

	imu.config(false);//make sure the device is not in config mode

	kvh_driver::imu_data_t data;
	while(true){
		imu.read_data(data);
		printf("Data Packet\n");
		printf("\tGyro: %f, %f, %f\n", data.angleX, data.angleY, data.angleZ);
		printf("\tAccel: %f, %f, %f\n", data.accelX, data.accelY, data.accelZ);
		printf("\tStatus: %02X\n", 0xFF&data.status_raw);
		printf("\tSequence #: %u\n", data.sequence_num);
		printf("\tTemperature: %u\n", data.temp);
		printf("\tCRC: %08X\n", data.crc);
	}

	imu.close();
	return 0;
}
