# microROS_Zephyr
This is a Repository that holds example code for microROS on Zephyr

For this Project a Olimex STM32-E407 is used. The Official Tutorial from the microROS Webpage was used to build the package. [(Link to Micro-ROS on Zephyr with Olimex STM32)](https://micro.ros.org/docs/tutorials/core/first_application_rtos/zephyr/) Then a new APP/Project was added. Here the Example from [multithreaded publisher](https://github.com/micro-ROS/micro-ROS-demos/blob/feature/multithread/rclc/multithread_publisher_subscriber/main.c) was used to build this.

The goal of the project is to run a multithreaded microROS app. One thread should always check if a specific topic was published with the bool true. If that happens a shared memory value should be set and all other threads should stop or run specific code. This is needed to stop the robot in an emergency, which needs to happen quickly. (Stop driving around etc)
