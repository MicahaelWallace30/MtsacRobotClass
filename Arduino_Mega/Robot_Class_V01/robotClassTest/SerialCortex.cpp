//
// PRIVATE
//
#include "SerialCortex.h"
/*-----------------------------------------------------------------------------*/
/*  Transmit one character                                                     */
/*-----------------------------------------------------------------------------*/
void SerialCortex::serialTxChar(signed char c)
{
  // software send
  mySerial->write( c );
}

/*-----------------------------------------------------------------------------*/
/*  Receive one character, return (-1) is nothing available                    */
/*-----------------------------------------------------------------------------*/
int SerialCortex::serialRxChar()
{
  int  c;

  if( mySerial->available() )
    c = mySerial->read();     
  else
    c = -1;

  return(c);  
}

/*-----------------------------------------------------------------------------*/
/*  Timeout code                                                               */
/*-----------------------------------------------------------------------------*/
void SerialCortex::StartTimeout( long *t )
{
  // Start timeout
  *t = millis();
}

void SerialCortex::StopTimeout( long *t )
{
  // Stop timeout
  *t = 0;
}

int SerialCortex::CheckTimeout( long *t )
{      
  // Is timeout tunning
  if( *t > 0 )
  {
    if(millis() - *t < 10)
      return(0);     
    else
    {
      StopTimeout( t );
      //Serial.write("timeout\n");
      return(1);
    }       
  }

  return(0);
}

/*-----------------------------------------------------------------------------*/
/*  Calculate the checksum for the VEX data                                    */
/*-----------------------------------------------------------------------------*/
int SerialCortex::VexDataChecksum( struct _vexmsg *v )
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
void SerialCortex::VexDataTransmit( struct _vexmsg *v )
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
#ifdef DEBUG
void SerialCortex::VexDataPrint( struct _vexmsg *v )
{
  int   i;
  char  str[4];
  unsigned char *p = (unsigned char *)v;

  // check for null    
  if( p == NULL )
    return;

  for(i=0;i<(sizeof( vexheader ) + v->header.datalen + 1);i++)
  {
    sprintf(str, "%02X ", *p++ );

    Serial.print(str);
  }

  Serial.println("");    
}

#endif

/*-----------------------------------------------------------------------------*/
/*  Send a message                                                             */
/*-----------------------------------------------------------------------------*/
void SerialCortex::VexDataSendMessage( int type, signed char *buffer, int len )
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
  for(i=0;i<len;i++)
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
void SerialCortex::serialRxDecode( struct _vexmsg  *v  )
{
  rdata   *data;

  if( v->header.message_type == 2 )
  {
    // Expected RDATA_BYTES * RADATA_SIZE
    if( v->header.datalen == (RDATA_BYTES * RDATA_SIZE ))
    {
	
	  //first index of the received data is for checking if driver control is active.
      data = (rdata *)&(v->buffer[0]);
      
      if(data->data[0] != 0) driverControl = true;
      
      if(driverControl)
      {   

        int index;

        //Receive data
        //Example: controller[index] = data->data[index];
        for(index = 1; index < CONTROLLER_JOYSTICK_QTY + 1; index++)
        {
          controller[index] = data->data[index];                                      
        }  
        int buttonIndex = 0;


        for(index; index < CONTROLLER_JOYSTICK_QTY + 1 + (CONTROLLER_BUTTON_QTY / (RDATA_BYTES *8)); index++)
        {
          for(int n = 1; n <= 8; n++)
          {
            button[buttonIndex] = (data->data[index] & (1 << n))? true: false;
            buttonIndex++;
          }
        }    
      }                         
    }
  }
}
/*-----------------------------------------------------------------------------*/
/*  Message receive task                                                       */
/*-----------------------------------------------------------------------------*/
void SerialCortex::serialRxTask()
{
  static  int  serialRxState      = 0;
  static  int  serialDataReceived = 0;
  static  long timeout            = 0;
  int     cs;
  int     c;

  // check for a received character
  if( (c = serialRxChar()) < 0 )
  {
    // no received char 

    // Check for inter char timeout
    if( CheckTimeout(&timeout) == 1 )
      serialRxState = 0;

    // done
    return;
  }

  // A new character has been received so process it.

  // Start inter char timeout
  StartTimeout( &timeout );  

  // Where are we in the message parsing process ?
  switch( serialRxState )
  {
  case  kRxStateIdle:
    // look for first header byte
    if( c == HEADER_PREAMBLE_1 )
      serialRxState = kRxStateHeader;
    else
      StopTimeout(&timeout);
    // for debug
    MyVexDataRx.header.header_p1 = c;
    break;

  case  kRxStateHeader:
    // look for second header byte
    if( c == HEADER_PREAMBLE_2 )
      serialRxState = kRxStateMessageType;
    else
    {
      // Bad message
      StopTimeout(&timeout);
      serialRxState = kRxStateIdle;
    }
    // for debug
    MyVexDataRx.header.header_p2 = c;
    break;

  case  kRxStateMessageType:
    // We have a good header so next is message type
    MyVexDataRx.header.message_type = c;
    serialRxState = kRxStateDatalen;
    break;

  case  kRxStateDatalen:
    // next is data length byte
    MyVexDataRx.header.datalen = c;
    serialDataReceived = 0;
    serialRxState = kRxStateId;
    break;

  case  kRxStateId:
    // next is packet id byte
    MyVexDataRx.header.id = c;
    serialRxState = kRxStatePad;
    break;

  case  kRxStatePad:
    // next is an unused word alignment byte
    serialRxState = kRxStateData;
    break;

  case  kRxStateData:
    // receive the data packet
    MyVexDataRx.buffer[ serialDataReceived ] = c;
    if( ++serialDataReceived == MyVexDataRx.header.datalen )
      serialRxState = kRxStateChecksum;
    // check for buffer overflow
    if( serialDataReceived >= sizeof(MyVexDataRx.buffer) )
    {
      // Error
      StopTimeout(&timeout);
      serialRxState = kRxStateIdle;
    }            
    break;

  case  kRxStateChecksum:
    // Received checksum byte

    // stop timeout
    StopTimeout( &timeout );

    // calculate received checksum
    cs = VexDataChecksum( &MyVexDataRx );

    // comapare checksums
    if( cs == c )
    {
      // Good data
#ifdef  DEBUG
      VexDataPrint(&MyVexDataRx);
#endif
      serialRxDecode(&MyVexDataRx);
    }
    else
    {
      // Bad data
#ifdef  DEBUG
      Serial.write("bad data\n");
#endif
    }

    // done
    serialRxState = kRxStateIdle;
    break;

  default:
    break;
  }  
}


//
//	PUBLIC
//


/*-----------------------------------------------------------------------------*/
/*  DEFAULT constructor                                                        */
/*-----------------------------------------------------------------------------*/
SerialCortex::SerialCortex()
{
  mySerial = NULL;

  for(int index = 0; index < RDATA_SIZE; index++)
  {
    controller[index] = 0;
  }

  for(int index = 0; index < SDATA_SIZE; index++)
  {
    motor[index] = 0;
  }

  for(int index = 0; index < CONTROLLER_JOYSTICK_QTY; index++)
  {
    controller[index] = 0;
  }

  for(int index = 0; index < CONTROLLER_BUTTON_QTY; index ++)
  {
    button[index] = false;
  }
}


/*-----------------------------------------------------------------------------*/
/*  update loop                                                                */
/*-----------------------------------------------------------------------------*/
void SerialCortex::updateSerial()
{
  static  uint32_t  timer = 0;

  // Call as often as possible
  serialRxTask();

  // Run every 20mS (50x per second)
  if(millis() - timer < 20)
    return;     
  timer =  millis();


  //SEND DATA
  //update all data to be sent to the cortex
  //senddata.data[1] = 100//example
  for(byte index = 1; index < SDATA_SIZE; index++)
  {
    senddata.data[index] = motor[index];
  }    

  // send message
  VexDataSendMessage( 1, (signed char *)&senddata, sizeof(senddata) );
}


/*-----------------------------------------------------------------------------*/
/*  Initialize serial communication port numbers                               */
/*-----------------------------------------------------------------------------*/
bool SerialCortex::init(HardwareSerial *serial)
{
  mySerial = serial;
  mySerial->begin(SERIAL_BAUD);

  return true;	
}

