Gyro file is a simple class to help encapsulate the complicated features of mpu chip and still give good readings.

It needs about 6 - 10 seconds to sit still when the arduino 
turns on before it can zero in to correct calculations.

The mech people will need to reset the arduino and not move the robot at all for the 6 - 10 seconds before the match.

Include list:
#include <Wire.h>

#include <I2Cdev.h>

#include <MPU6050_6Axis_MotionApps20.h>

#include "Gyro.h"


The update method needs to be called as fast as possible to get good readings.
Do not put a delay on the update.


If you see on serial print FIFO overflow it will output bad data and is happening becaues the code is too slow.
