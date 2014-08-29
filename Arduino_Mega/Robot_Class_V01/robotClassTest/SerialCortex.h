/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*                        Copyright (c) James Pearman                          */
/*                                   2013                                      */
/*                            All Rights Reserved                              */
/*                                                                             */
/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*        Module:     SerialCortex.h                                           */
/*                    SerialCortex.cpp                                         */
/*        Author:     James Pearman                                            */
/*        Created:    5 August 2013                                            */
/*        Editor:     Michael Wallace                                          */
/*        Edited:     16 August 2014                                           */
/*                                                                             */
/*        Revisions:  V0.1                                                     */
/*                    V0.2  17 June 2014 - Back to bi-directional              */
/*                    V0.3  16 August 2014 - Edited                            */
/*                          - modified to fit in class                         */
/*                          - transmit change size                             */
/*                          - dynamic Tx & Rx sizes                            */
/*                                                                             */
/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*    Description:                                                             */
/*                                                                             */
/*    Demonstration code for serial comms transmition from Arduino to          */
/*    VEX cortex.                                                              */
/*                                                                             */
/*    Licensed under the Apache License, Version 2.0 (the "License");          */
/*    you may not use this file except in compliance with the License.         */
/*    You may obtain a copy of the License at                                  */
/*                                                                             */
/*    http://www.apache.org/licenses/LICENSE-2.0                               */
/*                                                                             */
/*    Unless required by applicable law or agreed to in writing, software      */
/*    distributed under the License is distributed on an "AS IS" BASIS,        */
/*    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. */
/*    See the License for the specific language governing permissions and      */
/*    limitations under the License.                                           */
/*                                                                             */
/*-----------------------------------------------------------------------------*/
#include "Arduino.h"
#include "HardwareSerial.h"



#ifndef SERIAL_COMMUNICATION_H
#define SERIAL_COMMUNICATION_H

//#define DEBUG  1
#define  HEADER_PREAMBLE_1  0xAA
#define  HEADER_PREAMBLE_2  0x55
#define  RDATA_SIZE 7
#define  RDATA_BYTES 1
#define  SDATA_SIZE 11
#define  SDATA_BYTES 1
#define  CONTROLLER_JOYSTICK_QTY 4
//Button QTY needs to divide evenly by 'bits' of RDATA_SIZE
#define  CONTROLLER_BUTTON_QTY 16 
#define  SERIAL_BAUD 38400

class SerialCortex
{
  
  private:  
	// storage for user data we want to send to the cortex
	typedef struct _sdata {
		  signed char  data[SDATA_SIZE];
		  } sdata;

	// storage for user data we want to receive from cortex
	typedef struct _rdata {
		   signed char   data[RDATA_SIZE];
		  } rdata;

	// Storage for the vex communications data
	typedef struct _vexheader {
		  unsigned char  header_p1;
		  unsigned char  header_p2;
		  unsigned char  message_type;
		  unsigned char  datalen;
		  unsigned char  id;
		  unsigned char  pad;
		  } vexheader;

	// Storage for the vex communications data
	typedef struct _vexmsg {
		  struct _vexheader      header;
		  signed char  buffer[128];
		  } vexmsg;
	  
	// Storage for transmit and receive messages
	vexmsg  MyVexDataTx;
	vexmsg  MyVexDataRx;

	// Storage for parameters we want to send
	sdata   senddata;

	// Receive states
	typedef enum {
		kRxStateIdle = 0,
		kRxStateHeader,
		kRxStateMessageType,
		kRxStateDatalen,
		kRxStateId,
		kRxStatePad,
		kRxStateData,
		kRxStateChecksum
		} rxState;
	
	//
	//member functions
	//	
private:	
	/*-----------------------------------------------------------------------------*/
	/*  Transmit one character                                                     */
	/*-----------------------------------------------------------------------------*/
	void serialTxChar(signed char c);
	
	/*-----------------------------------------------------------------------------*/
	/*  Receive one character, return (-1) is nothing available                    */
	/*-----------------------------------------------------------------------------*/
	int serialRxChar();
	
	/*-----------------------------------------------------------------------------*/
	/*  Timeout code                                                               */
	/*-----------------------------------------------------------------------------*/
	void StartTimeout( long *t );
	void StopTimeout( long *t );
	int CheckTimeout( long *t );
	
	/*-----------------------------------------------------------------------------*/
	/*  Calculate the checksum for the VEX data                                    */
	/*-----------------------------------------------------------------------------*/
	int VexDataChecksum( struct _vexmsg *v );
	
	/*-----------------------------------------------------------------------------*/
	/*  send the VEX data to the serial port                                       */
	/*-----------------------------------------------------------------------------*/
	void VexDataTransmit( struct _vexmsg *v );
	
	/*-----------------------------------------------------------------------------*/
	/*  Debugging print for the VEX data                                           */
	/*-----------------------------------------------------------------------------*/
	#ifdef DEBUG
	void VexDataPrint( struct _vexmsg *v );
	#endif
	
	/*-----------------------------------------------------------------------------*/
	/*  Send a message                                                             */
	/*-----------------------------------------------------------------------------*/
	void VexDataSendMessage( int type, signed char *buffer, int len );
	
	/*-----------------------------------------------------------------------------*/
	/*  Process the received data                                                  */
	/*-----------------------------------------------------------------------------*/
	void serialRxDecode( struct _vexmsg  *v  );
	
	/*-----------------------------------------------------------------------------*/
	/*  Message receive task                                                       */
	/*-----------------------------------------------------------------------------*/
	void serialRxTask();
 
	//serial communication pointer
	HardwareSerial  *mySerial;
 
 
  public:
	//default constructor
	SerialCortex();	
	//update both send and receive serial communication
	void updateSerial();
	//default initialization of serial communication
	bool init(HardwareSerial *serial);

protected:        
	//robot motors
	int8_t motor[SDATA_SIZE];
	//controller
	int8_t controller[CONTROLLER_JOYSTICK_QTY + 1];
        bool button[CONTROLLER_BUTTON_QTY];
      
        //boolean fro enabling driver control
        bool driverControl;  
};

#endif SERIAL_COMMUNICATION_H
