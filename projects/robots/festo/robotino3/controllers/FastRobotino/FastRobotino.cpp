//---------------------------------------------------------------------------------
//  Copyright (C) 2020 Thomas Feldmeier
//
//        thomas.feldmeier@thu.de
//
//        Christian Schlegel (christian.schlegel@thu.de)
//        Service Robotics Research Center
//        University of Applied Sciences
//        Prittwitzstr. 10
//        89075 Ulm (Germany)
//
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 
//---------------------------------------------------------------------------------
// WorldInfo > contactProperties > 
//      material1 "WheelMat"
//      coulombFriction 0 2 0
//      bounce 0

// rotation error:
//   va=4 very small error
//   va=8 rotation speed 5% too high, position error<1e-07
//   va=40 rotation speed 10% too low, position error 15cm after 10 seconds
// reducing the width of the wheels would help (it already is reduced from 0.06m to 0.01m)

// movement forward/backwards very stable, any friction and acceleration would work
// movement left/right gets unstable if acceleration is too high

#include <webots/Robot.hpp>
#include <webots/Motor.hpp>
#include <webots/Keyboard.hpp>
#include <math.h>

using namespace webots;
using namespace std;

Motor* motor[3];

// move forward vx m/s, left vy m/s, rotate left va radians/s (va=Angular Velocity)
void setSpeeds(double vx, double vy, double va) {
  vx /= 0.063; // wheel radius = 0.063
  vy /= 0.063;
  va *= 0.1826 / 0.063; // distance wheel center to robot center 0.1826
  motor[0]->setVelocity(vy + va);
  motor[1]->setVelocity(-0.5*vy - sqrt(0.75)*vx + va);
  motor[2]->setVelocity(-0.5*vy + sqrt(0.75)*vx + va);
}

int main(int argc, char* argv[]) {
  Robot *robot = new Robot();
  double timeStep = robot->getBasicTimeStep();
  Keyboard* keyboard = robot->getKeyboard();
  keyboard->enable(timeStep);
  for(int i=3; i--;) {
    motor[i] = robot->getMotor("wheel"+to_string(i)+"_joint"); 
    motor[i]->setPosition(INFINITY);
    motor[i]->setVelocity(0);
  }
  double actualSpeed[3]={0,0,0};
  // this acceleration was tested with WorldInfo.basicTimeStep=16, 32, 64
  // and velocity 8 for x or y or angular
  // extremly accuate, except angular velocity may differ from set speed by 10%
  double maxAcceleration[3]={10,6,20};
  double targetSpeed[3]={0,0,0};
  while(robot->step(timeStep) !=-1) {
    double keySpeed[3] = {0,0,0};
    int key;
    while( (key = keyboard->getKey()) != -1) {
      if(key=='W' || key==keyboard->UP)
        keySpeed[0]=1;
      if(key=='S'|| key==keyboard->DOWN)
        keySpeed[0]=-1;
      if(key=='A'|| key==keyboard->LEFT)
        keySpeed[1]=1;
      if(key=='D' || key==keyboard->RIGHT)
        keySpeed[1]=-1;
      if(key=='Q')
        keySpeed[2]=-1;
      if(key=='E')
        keySpeed[2]=1;
      }
//    if(key==' ' || keySpeed[0] || keySpeed[1] || keySpeed[2])
      for(int i=3; i--;)
        targetSpeed[i]=keySpeed[i]*6;
    double maxSteps=0;
    for(int i=3; i--;) {
      double stepsNeeded = abs(targetSpeed[i]-actualSpeed[i]) / (maxAcceleration[i]*(timeStep/1000.0));
      if(stepsNeeded>maxSteps)
        maxSteps = stepsNeeded;
    }
    if(maxSteps<1)
      maxSteps=1;
    for(int i=3; i--;)
      actualSpeed[i] += (targetSpeed[i]-actualSpeed[i]) / maxSteps;
    setSpeeds(actualSpeed[0], actualSpeed[1], actualSpeed[2]);
  }
}
