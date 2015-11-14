/*
This Code is published by Yannik Wertz under the "Creativ Common Attribution-NonCommercial-ShareAlike 4.0 International (CC BY-NC-SA 4.0)" License 
For more Information see : http://creativecommons.org/licenses/by-nc-sa/4.0/


#########################################################
#########################################################

Open Lap
Infrared Laptimer for Miniquad Racing

IR Slave Receiver V0.5 (13.11.15) Single Sensor Version

Signal:
Start Bits | Data (e.g) | Checksum (0 if number of 1s is even or 1 if it's odd)
   0   0   | 0  1  1  0 | 0  

/*#######################################################
##                       Defines                       ##
#######################################################*/

#include <Wire.h>

//#define DEBUGMODE

#define VERSION      "0.5"  //Version number

#define ID              11  //I2C adress. 10 ist first adress. For more slaves increment adress and configure master

#define NUMSENSORS       1  //number of sensors connecten to arduino
#define FIRSTSENSORPIN   2  //first sensor connecten to D2, seconds to D3, ...

#define NUMBITS          7  //Starts Bits, data and checksum
#define DATABITS         4  //4 Databits -> 0-15

#define CRITTIME       318  //µS border for decision 0 or 1
#define SIGNALSPACE   1500  //µS
#define LOWBORDER       40  //µS (160µS-(4/38000)s = 54µS minus a little tolerance)
#define TOPBORDER      630  //µS (425µS+(6/38000)s = 583µS plus a little tolerance) 

#define ANALYSEDELAY   100  //µS
#define DELETELAST     500  //mS Delete last remembered IDs

#define MINLAPTIME    1000  //ms to register a new completet lap

#define TALKPIN          4  //Pin D4 will be HIGH if slave has new data for master

#define pinRead(x,y)  (x & (1 << y))  //e.g. read state of D2 : pinRead(PIND, 2)

/*#######################################################
##                      Variables                      ##
#######################################################*/

uint32_t fallTime[NUMSENSORS];
uint32_t getTime[NUMSENSORS]; 
uint32_t lastValue[NUMSENSORS];
uint8_t counter[NUMSENSORS];
uint16_t buf[NUMSENSORS][8];
uint8_t message[NUMSENSORS][7];
boolean dataReady[NUMSENSORS];
boolean checksum[NUMSENSORS];
uint8_t data[NUMSENSORS];

uint32_t dataReadyTime[NUMSENSORS];

boolean pilot[16] = {false, false, false, false, false, false, false, false,      //buffer for I2C output. set true if pilot has passed
                     false, false, false, false, false, false, false, false};

uint32_t lastPilotTime[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

uint32_t lastMessageTime[NUMSENSORS];

uint8_t lastPilot[NUMSENSORS][3]; //Save last three values for colision avoidance

boolean state[NUMSENSORS];
boolean lastState[NUMSENSORS];

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

  //Sensor inputs
  for(uint8_t a = 0 ; a < NUMSENSORS ; a++)
  {
    pinMode(FIRSTSENSORPIN + a, INPUT);
  }

  for(uint8_t a = 0 ; a < NUMSENSORS ; a++)
  {
    dataReady[a] = false;
    lastPilot[a][0] = 255;
    lastPilot[a][1] = 255;
  }
}

/*#######################################################
##                        Loop                         ##
#######################################################*/

void loop() 
{ 
  //Read digital inputs
  for(int8_t sensor = (NUMSENSORS-1) ; sensor >= 0 ; sensor--)  //down-counting for-loop is faster then up-counting (compare against zero is faster)
  {
    //Using direct pin access save huge amount of time. digitalRead takes 66 Clock cycles while direct pin access only needs 2!
    state[sensor] = pinRead(PIND, (FIRSTSENSORPIN + sensor));
  }

  //Compare state against lastState
  for(int8_t sensor = (NUMSENSORS-1) ; sensor >= 0 ; sensor--)    
  {
    uint32_t time = micros(); //save time for upcoming calculations

    if(!dataReady[sensor])  //If data isn't analysed don't collect new data
    {
      if(state[sensor] == lastState[sensor])  //Nothing has changed -> do nothing ;)
      { }
      else    //state has changed
      {
        if(!state[sensor])  //signal has falling edge, puls starts
        {
          fallTime[sensor] = time;  
        }
        else  //rising edge, puls ends
        {
          getTime[sensor] = time - fallTime[sensor];  //Calculate puls-time
  
          if(getTime[sensor] >= LOWBORDER && getTime[sensor] <= TOPBORDER)  //Check for possible range
          {    
            if(time - lastValue[sensor] <= SIGNALSPACE)  //Still the same message
            {
              buf[sensor][counter[sensor]] = getTime[sensor];  //save time in input-buffer
              counter[sensor]++;
            }
            else  //New Message
            {        
              if(counter[sensor] != 0)    //Error occurred. New message but last message wasn't completed
              {
                #ifdef DEBUGMODE
                Serial.print(sensor); Serial.println(" NEB"); 
                #endif
                
                resetValue(sensor); //reset last readings
              }
              
              buf[sensor][0] = getTime[sensor];   //Start new message
              counter[sensor] = 1;
            }
            lastValue[sensor] = time;  //save actual time for next input
            
            if(counter[sensor] == NUMBITS)  //All bits read -> ready for check
            {
              counter[sensor] = 0;
              dataReady[sensor] = true;    
              dataReadyTime[sensor] = micros();
            }
          }
          else  //measured time can't be possible -> Cancle this message and try new one.
          {
            #ifdef DEBUGMODE
              Serial.print(sensor); Serial.print(" IT "); Serial.println(getTime[sensor]);
            #endif
            resetValue(sensor); //Reset last inputs on a wrong input
            
            counter[sensor] = 0; //reset counter
          }
        }//else(rising edge)
      }//else(state has changed)
    }//!dataReady
    lastState[sensor] = state[sensor];  //everything has been checked - set state for next round
  }//for(sensors)


  //calculate message if reading is completet
  for(int8_t sensor = (NUMSENSORS-1) ; sensor >= 0 ; sensor--)
  {
    if(dataReady[sensor] && micros() >= (dataReadyTime[sensor] + ANALYSEDELAY))  //New message is ready
    {
      dataReady[sensor] = false;

      for(int8_t a = 0; a < NUMBITS ; a++)  //Calculate bit message
      { 
        message[sensor][a] = getValue(buf[sensor][a]);
      }
    
      if(message[sensor][0] == 0 && message[sensor][1] == 0)  //Start Bits
      {      
        checksum[sensor] = false;
        for(uint8_t b = 0; b < DATABITS; b++)  //Calculating Checksum
        {
            if(message[sensor][2+b])    //if it's a one, toggle checksum
              checksum[sensor] = !checksum[sensor];
        }
        
        if(checksum[sensor] == message[sensor][2 + DATABITS])  //is caculated checksum the same as read checksum?
        {
          data[sensor] = ((message[sensor][2] << 3) + (message[sensor][3] << 2) + (message[sensor][4] << 1) + (message[sensor][5]));  //calculate command
          
          if(twoOfFour(sensor, data[sensor]))   //ID has to be read 2 times to avoid single mistaken readings
          {
            if(millis() >= (lastPilotTime[data[sensor]] + MINLAPTIME))  //New lap
            {
              pilot[data[sensor]] = true; 
              lastPilotTime[data[sensor]] = millis();
              digitalWrite(TALKPIN, HIGH);     //New data, lets give it to master
            }
          }
          saveValue(sensor, data[sensor]);
          lastMessageTime[sensor] = millis();
  
          #ifdef DEBUGMODE
            Serial.print(sensor); Serial.print(" ND "); Serial.println(data[sensor]);
          #endif
        }
        #ifdef DEBUGMODE
        else
        {
          Serial.print(sensor); Serial.println(" CI ");
        }       
        #endif       
      }
      #ifdef DEBUGMODE
      else
      {
        Serial.print(sensor); Serial.println(" SBI");
      }
      #endif
    }//if(dataReady)
  }//for(sensors)

  for(int8_t sensor = (NUMSENSORS-1) ; sensor >= 0 ; sensor--)
  {
    if(millis() >= (lastMessageTime[sensor] + DELETELAST))
      resetValue(sensor);
  }
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

//check if the actual value was read before in one of the last three inputs 
boolean twoOfFour(uint8_t sensor, uint8_t id)
{
  if(id == lastPilot[sensor][0] || id == lastPilot[sensor][1] || id == lastPilot[sensor][2])
    return true;
  else
    return false;
}

//shift last values one part and save new
void saveValue(uint8_t sensor, uint8_t id)
{
  lastPilot[sensor][0] = lastPilot[sensor][1];
  lastPilot[sensor][1] = lastPilot[sensor][2];
  lastPilot[sensor][2] = id;
}

//Reset last read IDs
void resetValue(uint8_t sensor)
{
  lastPilot[sensor][0] = 255;
  lastPilot[sensor][1] = 255;
  lastPilot[sensor][2] = 255;
}

