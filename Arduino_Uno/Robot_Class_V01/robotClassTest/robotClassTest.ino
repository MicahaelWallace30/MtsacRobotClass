#include <SoftwareSerial.h>
#include "Robot.h"

Robot test;

void setup()
{

  test.init(6,7);
  
}

void loop()
{
  test.update();
}


