#include "Robot.h"


void Robot::init(int rx, int tx)
{
  //any robot init can be done here





  //can and will be toggled from cortex
  driverControl = false;// inherited from SerialCortex

  //init serial communication
  SerialCortex::init(rx, tx);  
}

//main update to be called only from loop();
void Robot::update()
{
  //update robot motors here
  
  //motor naming can be done in enum from Robot.h
  //motor[] is inherited from SerialCortex.h as protected member
  //example: motor[3] = 33;
  motor[2] = 100;  
  

  //controller[] and button[] is inherited from SerialCortex.h as protected members
  //controller and button arrays uses proper enum mapping from Robot.h
  //example: motor[2] = controller[joyAxisOne];
  //example:    if(button[Btn5U] == true) motor[3] = 100;
  //            else motor[3] = 0;






  //updates serial communication
  SerialCortex::updateSerial();  
}
