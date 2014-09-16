#include "Robot.h"

Robot test;
//this file shouldn't really be needed to do any thing, but init robot and call the robotLoop
void setup()
{
  test.init(&Serial1);  
}

void loop()
{
  test.robotLoop();
}


