# KVH 1750 Fibe Optics Gyro IMU Driver
This is the driver for the KVG 1750 Fiber Optic Gyro IMU

### Install Steps

1. Copy repository using git clone
> git clone https://github.com/RVeR-Lab/kvh.git

2. Install necessary accompanying packages
> sudo apt-get install ros-melodic-driver-base-config

> sudo apt-get install ros-melodic-device-driver-base

> sudo apt-get install ros-melodic-device-driver
3. Clone necessary accompanying git repos
> git clone https://github.com/ros-driver/driver_common.git

> git clone https://github.com/RIVeR-Lab/river_util.git

4. Remove unecessary packages
> rm -rf safety_interface/

> rm -rf robot_teleop/

> rm -rf stereo_image_sync/

5. Commands to add user to the tty and dialout groups
> sudo usermod -a -G tty river

> sudo usermod -a -G tty dialout

> sudo usermod -a -G dialout river

5. Get package from github tag
> git fetch --all --tags

6. Reboot the computer
> sudo reboot now

7. Run launch file
> roslaunch kvh_driver master.launch
