/*
This Code is published by Yannik Wertz under the "Creativ Common Attribution-NonCommercial-ShareAlike 4.0 International (CC BY-NC-SA 4.0)" License 
For more Information see : http://creativecommons.org/licenses/by-nc-sa/4.0/


#########################################################
#########################################################

Open Lap
Infrared Laptimer for Miniquad Racing

IR Slave Receiver V0.3 (23.10.15) Single Sensor Version

Signal:
Start Bits | Data (e.g) | Checksum (0 if number of 1s is even or 1 if it's odd)
   0   0   | 0  1  1  0 | 0  

/*#######################################################
##                       Defines                       ##
#######################################################*/

#include <Wire.h>

//#define DEBUGMODE

#define VERSION      "0.3"  //Version number

#define ID              11  //I2C adress. 10 ist first adress. For more slaves increment adress and configure master

#define NUMBITS          7  //Starts Bits, data and checksum
#define DATABITS         4  //4 Databits -> 0-15

#define CRITTIME       318  //µS border for decision 0 or 1
#define SIGNALSPACE   1500  //µS
#define LOWBORDER       40  //µS (160µS-(4/38000)s = 54µS minus a little tolerance)
#define TOPBORDER      610  //µS (425µS+(6/38000)s = 583µS plus a little tolerance) 

//#define ANALYSEDELAY    50  //µS
#define DELETELAST     500  //µS Delete last remembered IDs

#define MINSPACE      1000  //ms to register a new completet lap

#define TALKPIN          4  //Pin D4 will be HIGH if slave has new data for master

/*#######################################################
##                      Variables                      ##
#######################################################*/

uint32_t fallTime0;
uint32_t getTime0; 
uint32_t lastValue0;
uint8_t counter0;
uint16_t buf0[8];
uint8_t message0[7];
boolean dataReady0 = false;
boolean checksum0;
uint8_t data0;

//uint32_t fallTime1;
//uint32_t getTime1; 
//uint32_t lastValue1;
//uint8_t counter1;
//uint16_t buf1[8];
//uint8_t message1[7];
//boolean dataReady1 = false;
//boolean checksum1;
//uint8_t data1;


boolean getValueFalling0_Flag = false;
boolean getValueRising0_Flag = false;

//boolean getValueFalling1_Flag = false;
//boolean getValueRising1_Flag = false;

uint32_t dataReadTime0;
//uint32_t dataReadTime1;

boolean pilot[16] = {false, false, false, false, false, false, false, false,      //buffer for I2C output. set true if pilot has passed
                     false, false, false, false, false, false, false, false};

uint32_t lastPilotTime[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

//uint8_t lastPilot1 = 255;
uint32_t lastPilotTime0 = 0;
//uint32_t lastPilotTime1 = 0;

uint8_t lastPilot0[2] = {255, 255}; //Save last two values for colision avoidance

/*#######################################################
##                        Setup                        ##
#######################################################*/

void setup() 
{
  Serial.begin(250000); //Only for debugging
  Serial.println("Open Lap");
  Serial.print("RX Slave Version "); Serial.println(VERSION);

  Wire.begin(ID);
  Wire.setClock(400000L);       //Fast I2C Mode - 400kHz (standard is 100kHz)
  Wire.onRequest(requestEvent);

  pinMode(TALKPIN, OUTPUT);

  //Sensor interrupts
  attachInterrupt(0, getValueFalling0, FALLING);
  //attachInterrupt(1, getValueFalling1, FALLING);
}

/*#######################################################
##                        Loop                         ##
#######################################################*/

void loop() 
{ 
  //Flags
  if (getValueFalling0_Flag)
  {
    getValueFalling0_Flag = false; //reset flag
    
    fallTime0 = micros();  
    attachInterrupt(0, getValueRising0, RISING);
  }
  if (getValueRising0_Flag)
  {
    getValueRising0_Flag = false; //reset flag
    
    uint32_t time = micros();
  
    getTime0 = time - fallTime0;  //Calculate puls-time

    if(getTime0 >= LOWBORDER && getTime0 <= TOPBORDER)  //Check for possible range
    {    
      if(time - lastValue0 <= SIGNALSPACE)  //Still the same message
      {
        buf0[counter0] = getTime0;  //save time in input-buffer
        counter0++;
      }
      else  //New Message
      {        
        if(counter0 != 0)    //Error occurred. New message but last message wasn't completed
        {
          #ifdef DEBUGMODE
          Serial.println("0 NEB"); 
          #endif
          
          resetValue();
        }
        
        buf0[0] = getTime0;   //Start new message
        counter0 = 1;
      }
      lastValue0 = time;  //save actual time for next input
      
      if(counter0 == NUMBITS)  //All bits read -> ready for check
      {
        detachInterrupt(0);  //While data is not analysed, we don't want new data
        counter0 = 0;
        dataReady0 = true;    
        dataReadTime0 = micros();
      }
      else  //message isn't completed yet - waiting for next value
        attachInterrupt(0, getValueFalling0, FALLING);
    }
    else  //measured time can't be possible -> Cancle this message and try new one.
    {
      #ifdef DEBUGMODE
        Serial.print("0 IT "); Serial.println(getTime0);
      #endif
      resetValue(); //Reset last inputs on a wrong input
      
      counter0 = 0; //reset counter and activate interrupt
      attachInterrupt(0, getValueFalling0, FALLING);
    }
  }

  
//  if (getValueFalling1_Flag)
//  {
//    getValueFalling1_Flag = false; //reset flag
//    
//    fallTime1 = micros();  
//    attachInterrupt(1, getValueRising1, RISING);
//  }
//  if (getValueRising1_Flag)
//  {
//    getValueRising1_Flag = false; //reset flag
//    
//    uint32_t time = micros();
//  
//    getTime1 = time - fallTime1;  //Calculate puls-time
//    
//    if(getTime1 >= LOWBORDER && getTime0 <= TOPBORDER)  //Check for possible range
//    {
//      if(time - lastValue1 <= SIGNALSPACE)  //Still the same message
//      {
//        buf1[counter1] = getTime1;  //save time in input-buffer
//        counter1++;
//      }
//      else  //New Message
//      {
//        #ifdef DEBUGMODE
//          if(counter1 != 0)    //Error occurred. New message but last message wasn't completed
//            Serial.println("1 NEB"); 
//        #endif
//        
//        buf1[0] = getTime1;   //Start new message
//        counter1 = 1;
//      }
//      lastValue1 = time;  //save actual time for next input
//      
//      if(counter1 == NUMBITS)  //All bits read -> ready for check
//      {
//        detachInterrupt(1);  //While data is not analysed, we don't want new data
//        counter1 = 0;
//        dataReady1 = true;    
//        dataReadTime1 = micros();
//      }
//      else  //message isn't completed yet - waiting for next value
//        attachInterrupt(1, getValueFalling1, FALLING);
//    }
//    else  //measured time can't be possible -> Cancle this message and try new one.
//    {
//      #ifdef DEBUGMODE
//        Serial.print("1 IT "); Serial.println(getTime1);
//      #endif
//      
//      counter0 = 0; //reset counter and activate interrupt
//      attachInterrupt(0, getValueFalling0, FALLING);
//    }
//  }

  
  
  if(dataReady0)  //New message is ready
  {
    dataReady0 = false;
      
    for(int8_t a = 0; a < NUMBITS ; a++)  //Calculate bit message
    { 
      message0[a] = getValue(buf0[a]);
    }
  
    if(message0[0] == 0 && message0[1] == 0)  //Start Bits
    {      
      checksum0 = false;
      for(uint8_t b = 0; b < DATABITS; b++)  //Calculating Checksum
      {
          if(message0[2+b])    //if it's a one, toggle checksum
            checksum0 = !checksum0;
      }
      
      if(checksum0 == message0[2 + DATABITS])  //is caculated checksum the same as read checksum?
      {
        data0 = ((message0[2] << 3) + (message0[3] << 2) + (message0[4] << 1) + (message0[5]));  //calculate command
        
        if(twoOfThree(data0))   //ID has to be read 2 times to avoid single mistaken readings
        {
          if(millis() >= (lastPilotTime[data0] + MINSPACE))  //New lap
          {
            pilot[data0] = true; 
            lastPilotTime[data0] = millis();
            digitalWrite(TALKPIN, HIGH);     //New data, lets give it to master
          }
        }
        saveValue(data0);
        lastPilotTime0 = millis();

        #ifdef DEBUGMODE
          Serial.print("0 ND "); Serial.println(data0);
        #endif
      }
      #ifdef DEBUGMODE
      else
        Serial.print("0 CI ");       
      #endif       
    }
    #ifdef DEBUGMODE
    else
      Serial.println("0 SBI");
    #endif
      
    attachInterrupt(0, getValueFalling0, FALLING);  //Reactiavte Interrupt
  }

//  if(dataReady1 && micros() >= (dataReadTime1 + ANALYSEDELAY))  //New message is ready
//  {
//    dataReady1 = false;
//    
//    for(int8_t a = 0; a < NUMBITS ; a++)  //Calculate bit message
//    { 
//      message1[a] = getValue(buf1[a]);
//    }
//  
//    if(message1[0] == 0 && message1[1] == 0)  //Start Bits
//    {      
//      checksum1 = false;
//      for(uint8_t b = 0; b < DATABITS; b++)  //Calculating Checksum
//      {
//          if(message1[2+b])    //if it's a one, toggle checksum
//            checksum1 = !checksum1;
//      }
//      
//      if(checksum1 == message1[2 + DATABITS])  //is caculated checksum the same as read checksum?
//      {
//        data1 = ((message1[2] << 3) + (message1[3] << 2) + (message1[4] << 1) + (message1[5]));  //calculate command
//        
//        if(data1 == lastPilot)   //ID has to be read 2 times to avoid single mistaken readings
//        {
//          if(millis() >= (lastPilotTime[data1] + MINSPACE))  //New lap
//          {
//            pilot[data1] = true; 
//            lastPilotTime[data1] = millis();
//            digitalWrite(TALKPIN, HIGH);     //New data, lets give it to master
//          }
//        }
//        lastPilot = data1;
//        lastValue = millis();
//
//        #ifdef DEBUGMODE
//          Serial.print("1 ND "); Serial.println(data1);
//        #endif
//      }
//      #ifdef DEBUGMODE
//      else
//        Serial.print("1 CI ");       
//      #endif       
//    }
//    #ifdef DEBUGMODE
//    else
//      Serial.println("1 SBI");
//    #endif
//      
//    attachInterrupt(1, getValueFalling1, FALLING);  //Reactiavte Interrupt
//  }

  if(millis() >= (lastPilotTime0 + DELETELAST))
    resetValue();
}

/*#######################################################
##                      Functions                      ##
#######################################################*/

uint8_t getValue(uint16_t time)
{
  if(time <= CRITTIME)  //if time < CRITTIME it's a zero, if it's bigger its a one
    return 0;
  else
    return 1;
}

//Tell master who has passed the Gate
void requestEvent()
{
  digitalWrite(TALKPIN, LOW);
  
  if(pilot[0])
  {
    Wire.write(0);
    pilot[0] = false;
  }
  else if(pilot[1])
  {
    Wire.write(1);
    pilot[1] = false;
  }
  else if(pilot[2])
  {
    Wire.write(2);
    pilot[2] = false;
  }
  else if(pilot[3])
  {
    Wire.write(3);
    pilot[3] = false;
  }
  else if(pilot[4])
  {
    Wire.write(4);
    pilot[4] = false;
  }
  else if(pilot[5])
  {
    Wire.write(5);
    pilot[5] = false;
  }
  else if(pilot[6])
  {
    Wire.write(6);
    pilot[6] = false;
  }
  else if(pilot[7])
  {
    Wire.write(7);
    pilot[7] = false;
  }
  else if(pilot[8])
  {
    Wire.write(8);
    pilot[8] = false;
  }
  else if(pilot[9])
  {
    Wire.write(9);
    pilot[9] = false;
  }
  else if(pilot[10])
  {
    Wire.write(10);
    pilot[10] = false;
  }
  else if(pilot[11])
  {
    Wire.write(11);
    pilot[11] = false;
  }
  else if(pilot[12])
  {
    Wire.write(12);
    pilot[12] = false;
  }
  else if(pilot[13])
  {
    Wire.write(13);
    pilot[13] = false;
  }
  else if(pilot[14])
  {
    Wire.write(14);
    pilot[14] = false;
  }
  else if(pilot[15])
  {
    Wire.write(15);
    pilot[15] = false;
  }
}

//check if the actual value was read before in one of the last two inputs 
boolean twoOfThree(uint8_t id)
{
  if(id == lastPilot0[0] || id == lastPilot0[1])
    return true;
  else
    return false;
}

//shift last values one part and save new
void saveValue(uint8_t id)
{
  lastPilot0[0] = lastPilot0[1];
  lastPilot0[1] = id;
}

//Reset last read IDs
void resetValue()
{
  lastPilot0[0] = 255;
  lastPilot0[1] = 255;
}

/*#######################################################
##                         ISR                         ##
#######################################################*/

void getValueFalling0()    //Puls begins
{
  getValueFalling0_Flag = true;
}

void getValueRising0()    //Puls ended
{ 
  getValueRising0_Flag = true;  
}

//void getValueFalling1()    //Puls begins
//{
//  getValueFalling1_Flag = true;
//}
//
//void getValueRising1()    //Puls ended
//{ 
//  getValueRising1_Flag = true;  
//}
