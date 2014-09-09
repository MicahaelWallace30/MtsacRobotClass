#include "Robot.h"

Robot test;

void setup()
{


  test.init(&Serial1);
  gyro.init();
  
}

void loop()
{
  test.update();
}


