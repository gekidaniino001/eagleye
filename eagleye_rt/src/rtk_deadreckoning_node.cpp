// Copyright (c) 2019, Map IV, Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
// * Redistributions of source code must retain the above copyright notice,
//   this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
// * Neither the name of the Map IV, Inc. nor the names of its contributors
//   may be used to endorse or promote products derived from this software
//   without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDER BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

/*
 * rtk_deadreckoning_node.cpp
 * Author MapIV Sekino
 */

#include "ros/ros.h"
#include "coordinate/coordinate.hpp"
#include "navigation/navigation.hpp"

static rtklib_msgs::RtklibNav rtklib_nav;
static sensor_msgs::NavSatFix fix;
static geometry_msgs::Vector3Stamped enu_vel;

static eagleye_msgs::Position enu_absolute_rtk_deadreckoning;
static sensor_msgs::NavSatFix eagleye_fix;
static ros::Publisher pub1;
static ros::Publisher pub2;

struct RtkDeadreckoningParameter rtk_deadreckoning_parameter;
struct RtkDeadreckoningStatus rtk_deadreckoning_status;

void rtklib_nav_callback(const rtklib_msgs::RtklibNav::ConstPtr& msg)
{
  rtklib_nav.header = msg->header;
  rtklib_nav.tow = msg->tow;
  rtklib_nav.ecef_pos = msg->ecef_pos;
  rtklib_nav.ecef_vel = msg->ecef_vel;
  rtklib_nav.status = msg->status;
}

void fix_callback(const sensor_msgs::NavSatFix::ConstPtr& msg)
{
  fix.header = msg->header;
  fix.status = msg->status;
  fix.latitude = msg->latitude;
  fix.longitude = msg->longitude;
  fix.altitude = msg->altitude;
  fix.position_covariance = msg->position_covariance;
  fix.position_covariance_type = msg->position_covariance_type;
}

void enu_vel_callback(const geometry_msgs::Vector3Stamped::ConstPtr& msg)
{
  enu_vel.header = msg->header;
  enu_vel.vector = msg->vector;
  enu_absolute_rtk_deadreckoning.header = msg->header;
  enu_absolute_rtk_deadreckoning.header.frame_id = "base_link";
  eagleye_fix.header = msg->header;
  eagleye_fix.header.frame_id = "gnss";
  rtk_deadreckoning_estimate(rtklib_nav,enu_vel,fix,rtk_deadreckoning_parameter,&rtk_deadreckoning_status,&enu_absolute_rtk_deadreckoning,&eagleye_fix);
  if(enu_absolute_rtk_deadreckoning.status.enabled_status == true)
  {
    pub1.publish(enu_absolute_rtk_deadreckoning);
    pub2.publish(eagleye_fix);
  }
  else if (fix.header.stamp.toSec() != 0)
  {
    pub2.publish(fix);
  }
}

int main(int argc, char** argv)
{
  ros::init(argc, argv, "rtk_deadreckoning");
  ros::NodeHandle n;

  std::string subscribe_rtklib_nav_topic_name = "/rtklib_nav";
  std::string subscribe_navsatfix_topic_name = "/navsat/fix";

  n.getParam("rtklib_nav_topic",subscribe_rtklib_nav_topic_name);
  n.getParam("navsatfix_topic",subscribe_navsatfix_topic_name);
  n.getParam("ecef_base_pos/x", rtk_deadreckoning_parameter.ecef_base_pos_x);
  n.getParam("ecef_base_pos/y", rtk_deadreckoning_parameter.ecef_base_pos_y);
  n.getParam("ecef_base_pos/z", rtk_deadreckoning_parameter.ecef_base_pos_z);
  n.getParam("ecef_base_pos/use_ecef_base_position", rtk_deadreckoning_parameter.use_ecef_base_position);
  n.getParam("rtk_deadreckoning/stop_judgment_velocity_threshold", rtk_deadreckoning_parameter.stop_judgment_velocity_threshold);

  std::cout<< "subscribe_rtklib_nav_topic_name "<<subscribe_rtklib_nav_topic_name<<std::endl;
  std::cout<< "subscribe_navsatfix_topic_name "<<subscribe_navsatfix_topic_name<<std::endl;
  std::cout<< "ecef_base_pos_x "<<rtk_deadreckoning_parameter.ecef_base_pos_x<<std::endl;
  std::cout<< "ecef_base_pos_y "<<rtk_deadreckoning_parameter.ecef_base_pos_y<<std::endl;
  std::cout<< "ecef_base_pos_z "<<rtk_deadreckoning_parameter.ecef_base_pos_z<<std::endl;
  std::cout<< "use_ecef_base_position "<<rtk_deadreckoning_parameter.use_ecef_base_position<<std::endl;
  std::cout<< "stop_judgment_velocity_threshold "<<rtk_deadreckoning_parameter.stop_judgment_velocity_threshold<<std::endl;

  ros::Subscriber sub1 = n.subscribe(subscribe_rtklib_nav_topic_name, 1000, rtklib_nav_callback, ros::TransportHints().tcpNoDelay());
  ros::Subscriber sub2 = n.subscribe("enu_vel", 1000, enu_vel_callback, ros::TransportHints().tcpNoDelay());
  ros::Subscriber sub3 = n.subscribe(subscribe_navsatfix_topic_name, 1000, fix_callback, ros::TransportHints().tcpNoDelay());
  pub1 = n.advertise<eagleye_msgs::Position>("enu_absolute_rtk_deadreckoning", 1000);
  pub2 = n.advertise<sensor_msgs::NavSatFix>("rtk_fix", 1000);

  ros::spin();

  return 0;
}
