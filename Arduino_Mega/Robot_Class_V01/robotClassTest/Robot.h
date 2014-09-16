#include "SerialCortex.h"


class Robot : 
public SerialCortex
{


private:
  //driverControl bool is inherited from SerialCortex.h
  //driverControl will be toggled from cortex no need to create new boolean
  //but if you do point to driverControl with new boolean
  //do not toggle the boolean from the arduino
  
  //variable to hold the autonomous current step.
  uint8_t autonomousStep;
  

  //Can change name just not order for the three enums below.
  //enum for controller mapping of joy sticks
  typedef enum {
    enabled = 0,
    joyAxisOne = 1,
    joyAxisTwo = 2,
    joyAxisThree = 3,
    joyAxisFour = 4
  }
  joyStickEnum;

  //enum for controller mapping of buttons
  typedef enum {
    Btn5U = 0,
    Btn5D,
    Btn6U,
    Btn6D,
    Btn7U,
    Btn7D,
    Btn7L,
    Btn7R,
    Btn8U,
    Btn8D,
    Btn8L,
    Btn8R
  }
  butonEnum;

  //example of motor enumeration
  typedef enum{
    motorLeftFront = 1,
    motorRightFront = 2
  }
  motorEnum;

public:

  //contructor
  Robot():
  SerialCortex(){};  
  
  //main update which also update serial communication
  void robotLoop();
  
  //to update serial and sensors to be called all the time.
  void update();
  
  //update sensors to be called from update
  void updateSensors();
  
  //when driver control is enabled this section should be used to update controls.
  void updateDriverControl();
  
  //to update autonomous code
  void updateAutonomous();
  
  
  //initialize robot and serial communication 
  void init(HardwareSerial *serial);
};


