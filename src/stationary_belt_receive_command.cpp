#include "ros/ros.h"
#include "std_msgs/Bool.h"
#include "robot_communication/demo_msg.h"
#include <iostream>
#include <stdio.h>
#include<sensor_msgs/Image.h>
#include <boost/thread/thread.hpp>
#include <boost/chrono/chrono.hpp>
#include <ros/spinner.h>
#include <ros/callback_queue.h>
#include <stdlib.h>
#include <sstream>
#include <iostream>
#include"pick_and_place/object_coordinate.h"
#include"learning_tf/diff_pos.h"
#include <ros/callback_queue.h>
#include <sstream>
#include<math.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#define PORT  1001

using namespace std;




class Controller
{
private:
  ros::NodeHandle node_;
  ros::Subscriber sub1_,sub2_;
  ros::Publisher bool_publisher;
  boost::mutex lock_;
  bool update_image_;
  bool running_;
  ros::AsyncSpinner spinner_;
  robot_communication::demo_msg msg;
  bool display_bool;
  char check[30],up_z[300],place[200],send_x[200],send_y[200],send_z[200],initial[100], char_y[100], char_z[100], final_s[100],char_x[100];
  double x;
  double y;
  double z;
  double final_z;
  int m_socket;
  int iPort;
  bool bConnected;
  bool status, send,rcv;
  const char* c_Host;
  char ReadString[5000];

public:
  Controller()
    : update_image_(false), running_(false),spinner_(0),x(0.0),y(0.0),z(0.0),iPort(PORT),bConnected(false), c_Host("192.168.0.124"), status(true),final_z(0.0),send(false),rcv(false)
  {
  }

  ~Controller()
  {
  }


  //Function for making the socket communication with particular protocol
  bool ClientConnect(const char* host, int port, int iIdSocket)

  {
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(host);
    if(connect(iIdSocket, (sockaddr *) &addr, sizeof(sockaddr)) <0)
    {
      return false;
    }
    else
    {
      return true;
    }

  }

  void coordinateCallback(const pick_and_place::object_coordinate& object)
  {


  //ros::Rate loop_rate(1);
    ROS_INFO("Received X coordinate is: %f",object.PosX);
    ROS_INFO("Received Y coordinate is: %f",object.PosY);
    x=object.PosX;
    y=object.PosY;
    z=object.PosZ;

    x=floor(x*100)/100;
    y=floor(y*100)/100;
    z=floor(z*100)/100;

    final_z= 0.15-(z-0.71);
    //here we write the equation for getting the distance from the end effector tip to object
    x=x*1000;
	x=1000-x;
    y= y+0.028;
    y= y*1000;
    final_z= final_z*1000;

  //loop_rate.sleep();

  }

  void presenceCallback(const robot_communication::demo_msg& msg)
  {
  //ros::Rate loop_rate(1);
  status = msg.object;
   //ROS_INFO("satus is:",status);
  //loop_rate.sleep();
  }

  void sendstring()
  {

    while(rcv && ros::ok() && running_)
    {

      if(status==true)
      {

        if(send==true)
        {

          sleep(2);
          strcpy(initial,"1,450,0,150,0,0,0,18,0\r\n");
         // sleep(2);
          send = write(m_socket,initial,strlen(initial));
          ROS_INFO("initial information send");
          sleep(2);

          if(x>400 && x<600)
          {
            //sleep(2);
           // strcpy(send_y,"1,550,");
	    strcpy(send_y,"1,");
            sprintf(char_x,"%f",x);
            strcat(send_y,char_x);
            strcat(send_y,",");
            sprintf(char_y,"%f",y);
            strcat(send_y,char_y);
            //---for z
            strcpy(send_z,send_y); //later execution of z axis action and sucking of robot ..here string is "1,550,y"
            strcpy(up_z,send_y);  //later execution of getting up position ..here string is "1,550,y"
            strcat(send_y,",150,0,0,0,18,0\r\n");
            sleep(3);
            send = write(m_socket,send_y,strlen(send_y)); //comes to the position of the object
            ROS_INFO("goes toward object...second position is send");
            //ros::spinOnce();

            if(send==true)
            {

              strcat(send_z,",");
              //sprintf(char_z,"%f",final_z);
		sprintf(char_z,"%f",15.00);
              strcat(send_z,char_z);

              strcat(send_z,",0,0,0,18,2\r\n");// here the sucking position string is "1,550,y,z,0,0,0,10,2\r\n"
              sleep(3);
              send = write(m_socket,send_z,strlen(send_z)); //z axis is lowered
              ROS_INFO("Z axis is lowered...third position is send");
              sleep(3);


              if(send==true)
              {
                strcat(up_z,",150,0,0,0,18,0\r\n");   //up position execution string
                send = write(m_socket,up_z,strlen(up_z));
                ROS_INFO("Z is raised...fourth position");
                sleep(3);
                if(send==true)
                {

                  strcpy(place,"1,0,-550,150,0,0,0,18,0\r\n");  //arbitary position to drop the object on the plate
                  send = write(m_socket,place,strlen(place));
                  ROS_INFO("goes to arbitary position");

                  sleep(3);
                  if(send==true)
                  {
                    strcpy(final_s,"1,0,-550,150,0,0,0,18,1\r\n");  //drop the object on the arbitary position
                    send = write(m_socket,final_s,strlen(final_s));
                    ROS_INFO("drops the object");
                    ROS_INFO("now will go to inital again");
                    ros::spinOnce();
                    sleep(2);   	 //2

                  }
                  else
                  {
                    ROS_INFO("The object dropped position string is not send to the controller");
                  }

                }
                else
                {
                  ROS_INFO("drop postion string not send to the controller");
                }

              }
              else
              {
                ROS_INFO("up position string not send to the controller");
              }
            }
            else
            {
              ROS_INFO("Z coordinate not send to the controller");
            }

          }
          else
          {
            ROS_INFO("y coordinate not send to the controller");
          }

        }
        else
        {
          ROS_INFO("CHECK MEssage sending failed");
        }

      }
      else
      {

        strcpy(final_s,"1,0,-550,150,0,0,0,18,1\r\n");  //drop the object on the arbitary position
        send = write(m_socket,final_s,strlen(final_s));
        sleep(2);
        ROS_INFO("Received X coordinate is: %f",x);
        ROS_INFO("staying at the initial position while no object present");
        sleep(2);
      }
    }


  }

  void execute()
  {

    running_ = true;
    m_socket = socket(AF_INET, SOCK_STREAM, 0);

    sub1_ = node_.subscribe("/object_current_pos",10, &Controller::coordinateCallback,this);
    sub2_ = node_.subscribe("/demo_topic",10, &Controller::presenceCallback,this);


    do
    {

      bConnected = ClientConnect(c_Host,iPort,m_socket); //do connection with the socket
      ROS_INFO("Trying to connect with the controller");

      if(bConnected == true)
      {
        ROS_INFO( "connected");
      }
      else
      {
        ROS_INFO("Notconnected");
      }
    }
    while(bConnected!=true);

    do
    {
      ROS_INFO("checking if its alive...");
      strcpy(check,"3,0,0,0,0,0,0,0,0\r\n");     //for reading the position of the robot from the controller
    //  strcpy(check,"4,0,0,0,0,0,0,0,0\r\n");
      send = write(m_socket,check,strlen(check));
      rcv=recv(m_socket,ReadString,5000,0);

    }
    while(rcv!=true);

    ROS_INFO("servo motor is on");

    spinner_.start();


    sendstring();





   /* boost::chrono::milliseconds duration(1);
    while(!rcv)
    {
      if(!ros::ok())
      {
        return;
      }
      boost::this_thread::sleep_for(duration);


    }*/

    spinner_.stop();

    running_ = false;

  }


};



int main(int argc, char **argv)
{

  ros::init(argc, argv, "receive_command"); //create a node name "receive_command"


  if(!ros::ok())
  {
    return 0;
  }


//Class to display and forward the images


  Controller ob;
  ob.execute();


//Close the ROS node


  ros::shutdown();

  return 0;

}
