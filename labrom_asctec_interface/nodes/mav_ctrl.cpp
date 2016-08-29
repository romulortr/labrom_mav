/*************************************************************************
*   Transform commands from labrom to asctec_hl_interface format (node implementation)
*   This file is part of labrom_asctec_interface
*
*   labrom_asctec_interface is free software: you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation, either version 3 of the License, or
*   (at your option) any later version.
*
*   labrom_asctec_interface is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with labrom_asctec_interface.  If not, see <http://www.gnu.org/licenses/>.
***************************************************************************/

#include "labrom_asctec_interface/mav_ctrl.h"

namespace labrom_asctec_interface{

/**
 * Empty constructor.
 */
CtrlNode::CtrlNode(void){
  // Paramas
  nh_.param("max_thrust", _max_thrust, 17.1675);
  nh_.param("loop_rate", _loop_rate, 20);

  // Start subscribers
  sub_thrust_   = nh_.subscribe("/cmd_thrust"  , 1, &CtrlNode::ThrustCallback, this);
  sub_attitude_ = nh_.subscribe("/cmd_attitude", 1, &CtrlNode::AttitudeCallback, this);
  
  // Start publishers
  pub_mav_ctrl_ = nh_.advertise<asctec_hl_comm::mav_ctrl>("control", 1);

};

/**
 * Empty destructor. 
 */
CtrlNode::~CtrlNode(void){};

/**
 * Thrust callback. Save last thrust command received 
 */
void CtrlNode::ThrustCallback(const std_msgs::Int32::ConstPtr &msg){
  thrust_ = *msg;
}

/**
 * Attitude callback. Save last attitude command received 
 */
void CtrlNode::AttitudeCallback(const geometry_msgs::Vector3::ConstPtr &msg){
  attitude_ = *msg;
}

/**
 * Publish control message in asctec_hl_interface format
 */
void CtrlNode::PublishMavCtrl(void){
  // Control method
  mav_ctrl_.type = 1;
  // x~pitch, y~roll, z~thrust, units in rad and rad/s for yaw
  mav_ctrl_.x = attitude_.y;                  // pitch (rad)
  mav_ctrl_.y = attitude_.x;                  // roll (rad)
  mav_ctrl_.z = thrust_.data / _max_thrust;   // thrust (0 to 1.0)
  mav_ctrl_.yaw = attitude_.z;                // yaw (rad/s)
    
  // Publish message
  pub_mav_ctrl_.publish(mav_ctrl_);
}

/**
 * ROS spin loop
 */
void CtrlNode::Spin(void){
  ros::Rate rate(_loop_rate);
  
  while(ros::ok()){
    // Publish control message
    PublishMavCtrl();
    // Sleep
    rate.sleep();
  }
}

} // labrom_asctec_interface namespace

int main(int argc, char **argv){
  // Initialize ROS within this node
  ros::init(argc, argv, "labrom_asctec_interface_ctrl");
   // Ctrl interface node
  labrom_asctec_interface::CtrlNode node;
  // Loop
  node.Spin();  
}