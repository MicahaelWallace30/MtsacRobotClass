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

*Example:

motor[2] = 100; 

sets motor port 2 on cortex to 100


*Warning 
motor is one byte

motor[2] = 1000; 

will produce unwanted results stay in byte range (-127 -- +126)



------------------------------
get control input
------------------------------
Robot::update()

The vex cortex has a bool driverControl which when enabled switch driver control on the arduino

*Examples:

val = controller[joyAxisOne];

val = button[Btn5U]? 100: 0;



