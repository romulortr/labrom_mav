/*************************************************************************
*   Manager Node implementation 
*   This file is part of labrom_mav_manager
*
*   labrom_mav_manager is free software: you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation, either version 3 of the License, or
*   (at your option) any later version.
*
*   labrom_mav_manager is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with labrom_mav_manager.  If not, see <http://www.gnu.org/licenses/>.
***************************************************************************/

// labrom_mav_manager libraries
#include "labrom_mav_manager/manager.h"
// labrom_mav_blind_action libraries
#include "blind_action/take_off_server.h"
#include "blind_action/take_off_client.h"
#include "blind_action/climb_server.h"
#include "blind_action/climb_client.h"
#include "blind_action/landing_server.h"
#include "blind_action/landing_client.h"
#include "blind_action/blind.h"
// labrom_control libriares
#include "labrom_control/pid_simple.h"

int main(int argc, char **argv){
  // Initialize ROS within this node 
  ros::init(argc,argv,"MAVManager");
  ros::NodeHandle nh;
  // ROS Spin thread
  boost::thread spin_thread(&manager::Spin);
  // PID Controller for blind action
  controllers::pid::Simple pid("Blind"); 
  double kp=0.2, ki=2.5, kd=10*0.02, windup_thresh=10;
  pid.SetParams(kp,ki,kd,windup_thresh);

  // Manager state machine
  manager::ManagerState state= manager::WAIT_MOTORS_ON;
  
  // Action servers
  int feedforward=42, max_thrust=50, loop_rate=20;
  blind::take_off::TakeOffServer take_off_server(pid);
  blind::climb::ClimbServer climb_server(pid);
  blind::landing::LandingServer landing_server(pid);

  while(ros::ok()){
    // Spin thread
    switch (state){
      //! \todo Wait until quad motors are on (GET THIS INFORMATION)
      case manager::WAIT_MOTORS_ON:{
        state = manager::TAKE_OFF;
        break;
      } 

      // Take off action
      case manager::TAKE_OFF: {
        // action client
        blind::take_off::TakeOffClient client;
        // Setting takeoff server parameters                                    
        take_off_server.SetParams(feedforward, max_thrust, loop_rate);
        // Setting takeoff client parameters
        double take_off_accel = 9.9;
        client.SetGoal(take_off_accel);
        // Perform take off attempt
        double timeout = 30;
        client.SendGoal(timeout);
        //! \todo verify if suceeded (MODIFY CLENT SENDGOAL return value)
        state = manager::CLIMB;
        break;
      }                                 

      // Climb action
      case manager::CLIMB: {
        // action client
        blind::climb::ClimbClient client;
        // Setting climb server parameters                                    
        climb_server.SetParams(feedforward, max_thrust, loop_rate);
        // Setting clim client parameters
        double climb_accel = 10.2, timeout = 2;
        client.SetGoal(climb_accel, timeout);
        // Perform climb
        client.SendGoal(timeout+0.5);
        //! \todo verify if suceeded (MODIFY CLENT SENDGOAL return value)
        state = manager::LAND;
        break;
      }

      // Landing action
      case manager::LAND: {
        // action client
        blind::landing::LandingClient client;
        // Setting land server parameters                                    
        landing_server.SetParams(feedforward, max_thrust, loop_rate);
        // Setting land client parameters
        double descend_accel = 9.2, land_accel = 12.0;
        client.SetGoal(descend_accel);
        // Perform land attempt
        double timeout = 30;
        client.SendGoal(timeout);
        //! \todo verify if suceeded (MODIFY CLENT SENDGOAL return value)
        state = manager::WAIT_MOTORS_OFF;
        break;
      }

      // Wait motors off (user)
      case manager::WAIT_MOTORS_OFF: {
        ROS_INFO("TURN MOTORS OFF..");
      }
    } // switch

  }
   spin_thread.join(); 

  // Shutdown ros and spin thread
  ros::shutdown();
  

  return 0;
}
