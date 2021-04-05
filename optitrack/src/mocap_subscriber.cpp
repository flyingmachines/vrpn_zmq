

#include "ros/ros.h"
#include "geometry_msgs/PoseStamped.h"
// ZMQ includes for messaging
#include "zhelpers.hpp"
#include<zmq.hpp>
// Local includes
#include<time.h>
#include<chrono>
#include<string.h>
#include<iostream>
#include<array>

#include "pub_socket.h"


pub_socket pub_sock;

pub_socket::pub_socket() : context(1), publisher(context,ZMQ_PUB), sub_context(1), subscriber(sub_context,ZMQ_SUB), rot_q{0.7071,-0.7071,0.0,0.0} {
    publisher.bind("tcp://127.0.0.1:3885");
}


void Callback(const geometry_msgs::PoseStamped & ps)
{
    // Print out received data
   //ROS_INFO("\n Position: \n \t x:[%f] \n \t y:[%f] \n \t z:[%f] \n Orientation: \n \t x:[%f] \n \t y:[%f] \n \t z:[%f] \n \t w:[%f] \n",
   //           ps.pose.position.x,ps.pose.position.y,ps.pose.position.z,
   //           ps.pose.orientation.x,ps.pose.orientation.y,ps.pose.orientation.z,ps.pose.orientation.w); 

    // PixHawk pose that is to be transformed from inertial frame of Optitrack (X-Forward,Y-Up,Z-Right) into PixHawk NED convention

    geometry_msgs::PoseStamped pix_pose; 
   
    // Transformation of position into NED frame
    pix_pose.pose.position.x = ps.pose.position.x;
    pix_pose.pose.position.y = ps.pose.position.z;
    pix_pose.pose.position.z = -ps.pose.position.y;

    pix_pose.pose.orientation.w = ps.pose.orientation.w;
    pix_pose.pose.orientation.x = ps.pose.orientation.x;
    pix_pose.pose.orientation.y = ps.pose.orientation.z;
    pix_pose.pose.orientation.z = -ps.pose.orientation.y;

    // Print out after mapping before sending
    std::cout<<"\n Sample count is : "<<std::endl;
    ROS_INFO("\n Position: \n \t x:[%f] \n \t y:[%f] \n \t z:[%f] \n Orientation: \n \t w:[%f] \n \t x:[%f] \n \t y:[%f] \n \t z:[%f] \n",
              pix_pose.pose.position.x,pix_pose.pose.position.y,pix_pose.pose.position.z,
              pix_pose.pose.orientation.w,pix_pose.pose.orientation.x,pix_pose.pose.orientation.y,pix_pose.pose.orientation.z);

         //Declare a ZMQ message queue  
            zmq::message_t message(70);
    
    // Serialize (convert the data into a string) over zmq message AS PER THE ATT_POS_MOCAP_MSG PACK
            snprintf((char *) message.data(),70, "%f %f %f %f %f %f %f", pix_pose.pose.orientation.w,pix_pose.pose.orientation.x,pix_pose.pose.orientation.y,pix_pose.pose.orientation.z,
            	pix_pose.pose.position.x,pix_pose.pose.position.y,pix_pose.pose.position.z);
    
    // Publish the message 
          pub_sock.publisher.send(message);
    
    // Frequency of publishing should be between 30-50 Hz. Currently set to 50 Hz.
          sleep(0.02); 
    

}


int main(int argc, char **argv)
{
    // Initialize ROS and give a unique name for this node
    ros::init(argc,argv,"mocap_sub");

    // Create a handle for this process' node
    ros::NodeHandle nh;
    
    // Subscribe to the message of type "PoseStamped" published over the topic mentioned below in a buffer of size 1000
    ros::Subscriber sub = nh.subscribe("vrpn_client_node/squeeze/pose",1000, &Callback);

    // Keeps c++ from exiting until the node is stopped
    ros::spin();

    return 0;

}

