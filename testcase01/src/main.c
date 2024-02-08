#include <zephyr.h>
#include <stdio.h>

#include <rcl/rcl.h>
#include <rcl/error_handling.h>
#include <std_msgs/msg/int32.h>

#include <rclc/rclc.h>
#include <rclc/executor.h>

#include <rmw_microros/rmw_microros.h>
#include <microros_transports.h>

#define MY_STACK_SIZE 1024
#define MY_PRIORITY 3

#define RCCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){printk("Failed status on line %d: %d. Aborting.\n",__LINE__,(int)temp_rc);}}
#define RCSOFTCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){printk("Failed status on line %d: %d. Continuing.\n",__LINE__,(int)temp_rc);}}

rcl_publisher_t publisher;
rcl_publisher_t publisher_thread1;
std_msgs__msg__Int32 msg;
std_msgs__msg__Int32 msg_thread1;

K_THREAD_STACK_DEFINE(my_stack_area, MY_STACK_SIZE);
struct k_thread my_thread_data;

void my_entry_point(void *p1, void *p2, void *p3)
{
	rcl_publisher_t *pub = (rcl_publisher_t *)p1;

	while(1){
		RCSOFTCHECK(rcl_publish(pub, &msg_thread1, NULL));
		msg_thread1.data++;
		//k_yield();
	}
	
}


void timer_callback(rcl_timer_t * timer, int64_t last_call_time)
{	
	RCLC_UNUSED(last_call_time);
	if (timer != NULL) {
		RCSOFTCHECK(rcl_publish(&publisher, &msg, NULL));
		msg.data++;
	}
}

void main(void)
{	
	// Set custom transports
	rmw_uros_set_custom_transport(
		MICRO_ROS_FRAMING_REQUIRED,
		(void *) &default_params,
		zephyr_transport_open,
		zephyr_transport_close,
		zephyr_transport_write,
		zephyr_transport_read
	);

	// Init micro-ROS
	rcl_allocator_t allocator = rcl_get_default_allocator();
	rclc_support_t support;

	// create init_options
	RCCHECK(rclc_support_init(&support, 0, NULL, &allocator));

	// create node
	rcl_node_t node;
	RCCHECK(rclc_node_init_default(&node, "zephyr_int32_publisher", "", &support));

	// create publisher
	RCCHECK(rclc_publisher_init_default(
		&publisher,
		&node,
		ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32),
		"zephyr_int32_publisher"));

	RCCHECK(rclc_publisher_init_default(
		&publisher_thread1,
		&node,
		ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32),
		"publisher_thread1"));

	// create timer,
	rcl_timer_t timer;
	const unsigned int timer_timeout = 1000;
	RCCHECK(rclc_timer_init_default(
		&timer,
		&support,
		RCL_MS_TO_NS(timer_timeout),
		timer_callback));

	// create executor
	// rclc_executor_t executor;
	// RCCHECK(rclc_executor_init(&executor, &support.context, 0, &allocator));
	// RCCHECK(rclc_executor_add_timer(&executor, &timer));

	msg.data = 0;
	msg_thread1.data = 7;

	// Start the thread 
	k_tid_t my_tid = k_thread_create(&my_thread_data, my_stack_area,
                                      K_THREAD_STACK_SIZEOF(my_stack_area),
                                      my_entry_point, &publisher_thread1, NULL, NULL,
                                      MY_PRIORITY, 0, K_NO_WAIT);
    k_thread_name_set(my_tid, "my_thread");
	
	while(1){
		RCSOFTCHECK(rcl_publish(&publisher, &msg, NULL));
		msg.data++;
    	// rclc_executor_spin_some(&executor, RCL_MS_TO_NS(100));
		// usleep(1000);
	}

	RCCHECK(rcl_publisher_fini(&publisher, &node))
	RCCHECK(rcl_node_fini(&node))
}
