----------------------
Robot class inherits
----------------------
int8_t motor[];
int8_t controller[];
bool button[];
bool driverControl


------------------------
to set motor speed
------------------------
Robot::update()

set motor port number to array index
motor[0] = 100; does nothing there is no motor port 0 on the cortex it is reserved index for special cases.
motor[1] = 100; sets motor port 1 (two wire on the cortex to 100)

*Example:

motor[2] = 100; 

sets motor port 2 on cortex to 100 (2 wire)


*Warning (9/15/2014) fixed to two bytes from one.
motor is two bytes (word)
motor value is now -(2^15) to (2^15) will be cast to 8 bits


------------------------------
get control input
------------------------------
Robot::update()
now only loops through to update sensors and serial


Robot::robotLoop() is now main loop

The vex cortex has a bool driverControl which when enabled switch driver control on the arduino
Do not switch the bool on the arduino side it is done by the cortex.

*Examples:

val = controller[joyAxisOne];

val = button[Btn5U]? 100: 0;






