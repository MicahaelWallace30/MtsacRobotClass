/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*                        Copyright (c) James Pearman                          */
/*                                   2013                                      */
/*                            All Rights Reserved                              */
/*                                                                             */
/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*        Module:     SerialCortex.h                                           */
/*        Author:     James Pearman                                            */
/*        Created:    5 August 2013                                            */
/*        Editor:     Michael Wallace                                          */
/*        Edited:     16 August 2014                                           */
/*                                                                             */
/*        Revisions:  V0.1                                                     */
/*                    V0.2  17 June 2014 - Back to bi-directional              */
/*                    V0.3  16 August 2014 - Edited                            */
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

#define DEBUG   1

#define  HEADER_PREAMBLE_1  0xAA
#define  HEADER_PREAMBLE_2  0x55
#define  RDATA_SIZE 11
#define  RDATA_BYTES 1
#define  SDATA_SIZE 7
#define  SDATA_BYTES 1

bool driverControl = false;

// storage for user data we want to send to arduino
typedef struct _sdata {
       signed char data[SDATA_SIZE];
      } sdata;

// storage for user data we want to receive from arduino
typedef struct _rdata {
      signed char  data[RDATA_SIZE];
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
      vexheader      header;
      signed char  buffer[128];
      } vexmsg;

// Storage for transmit and receive messages
static  vexmsg  MyVexDataTx;
static  vexmsg  MyVexDataRx;

// Storage for parameters we want to send
static  sdata   senddata;

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

int     serialErrs = 0;


/*-----------------------------------------------------------------------------*/
/*  Transmit one character                                                     */
/*-----------------------------------------------------------------------------*/

void
serialTxChar( char c )
{
    sendChar(UART1, c);
}

/*-----------------------------------------------------------------------------*/
/*  Receive one character, returns (-1) if nothing available                   */
/*-----------------------------------------------------------------------------*/

int
serialRxChar()
{
    return(getChar(UART1));
}

/*-----------------------------------------------------------------------------*/
/*  Timeout code                                                               */
/*-----------------------------------------------------------------------------*/

void
serialTimeoutStart( long *t )
{
    // Start timeout
    *t = nSysTime;
}

void
serialTimeoutStop( long *t )
{
    // Stop timeout
    *t = 0;
}

int
serialTimeoutCheck( long *t )
{
    // Is timeout running
    if( *t > 0 )
        {
        if(nSysTime - *t < 10)
            return(0);
        else
            {
            serialTimeoutStop( t );
            return(1);
            }
        }

    return(0);
}

/*-----------------------------------------------------------------------------*/
/*  Calculate the checksum for the VEX data                                    */
/*-----------------------------------------------------------------------------*/

int
VexDataChecksum( vexmsg *v )
{
    int  i;
    int  cs = 0;
    unsigned char *p = (unsigned char *)v;

    // check for null
    if( p == NULL )
        return(0);

    // calculate checksum
    for(i=0;i<sizeof( vexheader ) + v->header.datalen;i++)
        cs += *p++;

    // checksum is one byte after all user data
    *p = 0x100 - (cs & 0xFF);

    // return the checksum for further use
    return(*p);
}

/*-----------------------------------------------------------------------------*/
/*  send the VEX data to the serial port                                       */
/*-----------------------------------------------------------------------------*/

void
VexDataTransmit( vexmsg *v )
{
    int  i;
    signed char *p = (signed char *)v;

    // check for null
    if( p == NULL )
        return;

    // send all data and checksum to serial port
    for(i=0;i<(sizeof( vexheader ) + v->header.datalen + 1);i++)
        serialTxChar( *p++ );
}

/*-----------------------------------------------------------------------------*/
/*  Debugging print for the VEX data                                           */
/*-----------------------------------------------------------------------------*/

void
VexDataPrint( vexmsg  *v )
{
    int  i;
    char  str[4];
    unsigned char *p = (unsigned char *)v;

    // check for null
    if( p == NULL )
        return;


    for(i=0;i<(sizeof( vexheader ) + v->header.datalen + 1);i++)
      {
      sprintf(str, "%02X ", *p++ );
      writeDebugStream(str);
      }

    writeDebugStreamLine("");
}

/*-----------------------------------------------------------------------------*/
/*  Send a message                                                             */
/*-----------------------------------------------------------------------------*/

void
VexDataSendMessage( int type, signed char *buffer, int len )
{
    int  i;
    signed char *p = buffer;

    // Initialize packet
    MyVexDataTx.header.header_p1    = HEADER_PREAMBLE_1;
    MyVexDataTx.header.header_p2    = HEADER_PREAMBLE_2;
    MyVexDataTx.header.message_type = type;   // message type
    MyVexDataTx.header.datalen      = len;    // Size of the user data area
    MyVexDataTx.header.id++;
    MyVexDataTx.header.pad          = 0x00;

    // Move data into transmit buffer
    for(i=0;i< len;i++)
        MyVexDataTx.buffer[i] = *p++;

    // Calculate the checksum
    VexDataChecksum( &MyVexDataTx );

    // Transmit data
    VexDataTransmit( &MyVexDataTx );

#ifdef  DEBUG
    VexDataPrint( &MyVexDataTx );
#endif
}

/*-----------------------------------------------------------------------------*/
/*  Process the received data                                                  */
/*-----------------------------------------------------------------------------*/

void
serialRxDecode( vexmsg  *v  )
{
    rdata   *data;

    if( v->header.message_type == 1 )
        {
        // Expected RDATA_BYTES * RDATA_SIZE
        if( v->header.datalen == (RDATA_BYTES * RDATA_SIZE))
            {
            data = (rdata *)&(v->buffer[0]);
            writeDebugStreamLine("var 0 is %d %d", data->data[0] );




       			//motor[motor1] = data->data[1];
       			motor[motor2] = data->data[2];
       			motor[motor3] = data->data[3];
       			motor[motor4] = data->data[4];
       			motor[motor5] = data->data[5];
       			motor[motor6] = data->data[6];
       			motor[motor7] = data->data[7];
       			motor[motor8] = data->data[8];
       			motor[motor9] = data->data[9];
       			//motor[motor10] = data->data[10];

				//toggle driver control on arduino
       			if(driverControl) senddata.data[0] = 1;
       			else senddata.data[0] = 0;

       			if(driverControl)
       			{

	            senddata.data[1] = vexRT[Ch1];
	            senddata.data[2] = vexRT[Ch2];
	            senddata.data[3] = vexRT[Ch3];
	            senddata.data[4] = vexRT[Ch4];

	            if(vexRT(Btn5U) == 1) senddata.data[5] |=  (1 << 1);
           		else senddata.data[5] &= ~(1 << 1);

           		if(vexRT(Btn5D) == 1) senddata.data[5] |= ( 1 << 2);
	            else senddata.data[5] &= ~(1 << 2);

	            if(vexRT(Btn6U) == 1) senddata.data[5] |= ( 1 << 3);
	            else senddata.data[5] &= ~(1 << 3);

	            if(vexRT(Btn6D) == 1) senddata.data[5] |= ( 1 << 4);
	            else senddata.data[5] &= ~(1 << 4);

	            if(vexRT(Btn7U) == 1) senddata.data[5] |= ( 1 << 5);
	            else senddata.data[5] &= ~(1 << 5);

	            if(vexRT(Btn7D) == 1) senddata.data[5] |= ( 1 << 6);
	            else senddata.data[5] &= ~(1 << 6);

	            if(vexRT(Btn7L) == 1) senddata.data[5] |= ( 1 << 7);
	            else senddata.data[5] &= ~(1 << 7);

	            if(vexRT(Btn7R) == 1) senddata.data[5] |= ( 1 << 8);
	            else senddata.data[5] &= ~(1 << 8);

	            if(vexRT(Btn8U) == 1) senddata.data[6] |= ( 1 << 1);
	            else senddata.data[6] &= ~(1 << 1);

	            if(vexRT(Btn8D) == 1) senddata.data[6] |= ( 1 << 2);
	            else senddata.data[6] &= ~(1 << 2);

	            if(vexRT(Btn8L) == 1) senddata.data[6] |= ( 1 << 3);
	            else senddata.data[6] &= ~(1 << 3);

	            if(vexRT(Btn8R) == 1) senddata.data[6] |= ( 1 << 4);
	            else senddata.data[6] &= ~(1 << 4);
	          }

					   // send some data back to the arduino, type 2
            VexDataSendMessage( 2, (signed char *)&senddata, sizeof(senddata) );
            }
        }
}

/*-----------------------------------------------------------------------------*/
/*  Message receive state machine                                              */
/*-----------------------------------------------------------------------------*/

void
serialRx( vexmsg  *v )
{
    static  rxState  serialRxState  = kRxStateIdle;
    static  int      serialDataReceived = 0;
    static  long     timeout            = 0;

    int    c;
    int    cs;
    int counter = 0;

    // Check for inter char timeout
    if( serialTimeoutCheck(&timeout) == 1 )
        {
        serialErrs++;
        serialRxState = kRxStateIdle;
        }


    // check for a received character
    while( (c = serialRxChar()) >= 0 )
        {

        //writeDebugStream("%02X ", c);
        //if( ++counter == 15 )
        //   writeDebugStreamLine("");
        // A new character has been received so process it.

        // Start inter char timeout
        serialTimeoutStart( &timeout );

        // Where are we in the message parsing process ?
        switch( serialRxState )
            {
            case  kRxStateIdle:
                // look for first header byte
                if( c == HEADER_PREAMBLE_1 )
                    serialRxState = kRxStateHeader;
                else
                    serialTimeoutStop(&timeout);
                // for debug
                v->header.header_p1 = c;
                break;

            case  kRxStateHeader:
                // look for second header byte
                if( c == HEADER_PREAMBLE_2 )
                    serialRxState = kRxStateMessageType;
                else
                    {
                    // Bad message
                    serialTimeoutStop(&timeout);
                    serialRxState = kRxStateIdle;
                    }
                    // for debug
                v->header.header_p2 = c;
                break;

            case  kRxStateMessageType:
                // We have a good header so next is message type
                v->header.message_type = c;
                serialRxState = kRxStateDatalen;
                break;

            case  kRxStateDatalen:
                // next is data length byte
                MyVexDataRx.header.datalen = c;
                serialDataReceived = 0;
                serialRxState = kRxStateId;
                break;

            case  kRxStateId:
                // next is id
                v->header.id = c;
                serialRxState = kRxStatePad;
                break;

            case  kRxStatePad:
                // Unused pad byte
                serialRxState = kRxStateData;
                break;

            case  kRxStateData:
                // receive the data packet
                MyVexDataRx.buffer[ serialDataReceived ] = c;
                if( ++serialDataReceived == (signed char)v->header.datalen )
                    serialRxState = kRxStateChecksum;

                // check for buffer overflow
                if( serialDataReceived >= sizeof(MyVexDataRx.buffer) )
                    {
                    // Error
                    serialTimeoutStop(&timeout);
                    serialErrs++;
                    serialRxState = kRxStateIdle;
                    }
                break;


            case  kRxStateChecksum:
                // Received checksum byte

                // stop timeout
                serialTimeoutStop( &timeout );

                // calculate received checksum
                cs = VexDataChecksum( v );

                // comapare checksums
                if( cs == c )
                    {
                    // Good data
#ifdef  DEBUG
                    VexDataPrint( v );
#endif
                    serialRxDecode( v );
                    }
                else
                    serialErrs++;

                // done
                serialRxState = kRxStateIdle;
                break;

            default:
                serialRxState = kRxStateIdle;
                break;
            }
        }
}

/*-----------------------------------------------------------------------------*/
/*  serial receive task                                                        */
/*-----------------------------------------------------------------------------*/

task serialRxTask()
{
    // bump our priority
    setTaskPriority(serialRxTask, 20);

    // check for messages
    while( true )
        {
        // run the receive data state machine
        serialRx( &MyVexDataRx );

        // check for data often
        wait1Msec(20);
        }
}
/*
task main()
{
    StartTask(serialRxTask);

    while(1)
        {

        wait1Msec(10);
        }
}
*/