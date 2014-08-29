#include "Robot.h"

Robot test;

void setup()
{

  test.init(&Serial1);
  
}

void loop()
{
  test.update();
}


