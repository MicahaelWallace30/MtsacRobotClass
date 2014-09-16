#include "Robot.h"


void Robot::init(HardwareSerial *serial)
{
  //any robot init can be done here



  //init robot autonomous step to 1
  autonomousStep = 1;
  
  //can and will be toggled from cortex
  driverControl = false;// inherited from SerialCortex

  //init serial communication
  SerialCortex::init(serial);  
}


//to update serial and sensors all the time
void Robot::update()
{
  //call method to update sensors
  updateSensors();  
  
  //updates serial communication
  SerialCortex::updateSerial();  
}

//method to where updating sensors if needed should take place
void Robot::updateSensors()
{
    //is the sensor autonomous/ driver / or both?
  if(!driverControl)//autonomous sensors
  {
  }
  else//driver control sensors
  {
    
  }
  
  //shared by both autonomous and driver control sensors
}

//main update to be called only from loop();
void Robot::robotLoop()
{
  if(driverControl)updateDriverControl();
  else updateAutonomous();
  
  update();  

}

//whhen driver contorl is enabled this section shoud be used to update controls
void Robot::updateDriverControl()
{
  //controller[] and button[] is inherited from SerialCortex.h as protected members
  //controller and button arrays uses proper enum mapping from Robot.h
  //example: motor[2] = controller[joyAxisOne];
  //example:    if(button[Btn5U] == true) motor[3] = 100;
  //            else motor[3] = 0;
  
}

//to update autonomous code 
void Robot::updateAutonomous()
{
    //update robot motors here
  
  //motor naming can be done in enum from Robot.h
  //motor[] is inherited from SerialCortex.h as protected member
  //example: motor[3] = 33;
  
  //basic steps of robot should be used in a swtich statement then update autonomousStep by 1 for next step
  //the idea is to have code go in a loop so the robot can still update the serial (motors) and sensors.
  
  //If you want you can define the steps as numbers.
  //ex #define GO_TO_FIRST_POLE 1
  //
  //case GO_TO_FIRST_POLE: 
  //
  
  //If what ever reason you want or need to do:
  //While(some sensor dicks around)
  //{
  //  go forward;
  //} 
  
  //You must call update like this.
  //While(some sensor dicks around)
  //{
  //  go forward;
  //  update();
  //}  

  
  switch(autonomousStep)
  {
  }

}

