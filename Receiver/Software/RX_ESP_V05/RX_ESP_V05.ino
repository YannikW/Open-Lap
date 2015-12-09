/*
This Code is published by Yannik Wertz under the "Creativ Common Attribution-NonCommercial-ShareAlike 4.0 International (CC BY-NC-SA 4.0)" License 
For more Information see : http://creativecommons.org/licenses/by-nc-sa/4.0/


#########################################################
#########################################################

OpenLap
Open Source IR Laptimer

For more infos see :
www.openlap.de
https://github.com/YannikW/Open-Lap
https://www.facebook.com/groups/1047398441948165/

IR Receiver V0.5 (09.12.15) for NodeMCU V1.0 ESP8266 Board

For transmitting protocol see : https://github.com/YannikW/Open-Lap/blob/master/docs/Open%20LapTime%20Protocol.md

************************************************************************************************************
* Special Thanks to:                                                                                       *
*                                                                                                          *
* Markus Brunner (Co-Management, Testing and Development)                                                  *
* Andreas Neubauer and Oliver Renner (FPV Race Tracker Software https://www.facebook.com/fpvracetracker/ ) *
************************************************************************************************************

/*#######################################################
##                       Defines                       ##
#######################################################*/

#define VERSION "OpenLap - RX ESP8266 - Version 0.5 (09.12.15)"  //Version number

#define BAUDRATE      9600  //115200 for OpenLap / EasyRaceLapTimer
                            //9600 for FPV Race Tracker

#define NUMSENSORS       4  //number of sensors connecten to ESP (4 is maximum)

#define NUMBITS          9  //Starts Bits, data and checksum
#define DATABITS         6  //6 Databits -> 0-63

#define CRITTIME       400  //µS border for decision 0 or 1
#define LOWBORDER      100  //µS
#define TOPBORDER      900  //µS

#define MINLAPTIME    5000  //ms to register a new completed lap

#define TONEPIN         D8  //Connect Buzzer to D8
#define BEEPLENGTH     100  //ms

#define ANALYSEDELAY   150  //µS Delay between data capturing and analysing
#define DELETELAST     500  //mS Delete last remembered IDs


/*#######################################################
##                 Includes & Objects                  ##
#######################################################*/

#include "SerialCommand.h" //see : https://github.com/scogswell/ArduinoSerialCommand

SerialCommand SCmd;

/*#######################################################
##                      Variables                      ##
#######################################################*/

boolean simpleDebug = false;          //print pulse times and calculated bit message
boolean fullDebug = false;            //print all readings and errors

uint8_t mode = 0;                     //0 : OpenLap - flying start (default)
                                      //1 : OpenLap - start on countdown - not supported yet
                                      //2 : FPV Race Tracker (start on countdown)
                                      //3 : EasyRaceLapTimer (flying start)

static const uint8_t sensorPin[4] = {5, 4, 0, 2}; //eqivalent GPIOs to D1, D2, D3, D4

boolean state[NUMSENSORS];            //input sensor state
boolean lastState[NUMSENSORS];        //last input sensor state
uint32_t lastChange[NUMSENSORS];      //last time state of sensor changed

uint16_t buf[NUMSENSORS][10];          //save last raw input pulse times
uint8_t message[NUMSENSORS][9];       //"translated" buffer (bit stream)
boolean checksum[NUMSENSORS];         //checksum of read message
uint8_t data[NUMSENSORS];             //read ID
uint8_t counter[NUMSENSORS];
uint32_t lastPilotTime[64] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                              0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                              0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                              0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; //Time last pass

boolean dataReady[NUMSENSORS];        //buf is full and ready for analyse
uint32_t dataReadyTime[NUMSENSORS];   //Time data collection completed (for delaying analyse)

uint32_t time;

uint32_t pass[64];   //Save time for start/finish passes for each of the 16 pilots
boolean firstPass[64] = {true, true, true, true, true, true, true, true, //First pass will only save time and not calculate a laptime
                         true, true, true, true, true, true, true, true,
                         true, true, true, true, true, true, true, true,
                         true, true, true, true, true, true, true, true,
                         true, true, true, true, true, true, true, true, 
                         true, true, true, true, true, true, true, true,
                         true, true, true, true, true, true, true, true,
                         true, true, true, true, true, true, true, true};
uint16_t lap[64] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};  //Rounds completed for each pilot

uint32_t beepEnd = 0;       //Time for beep to end
boolean beepActive = false;

uint8_t lastPilot[NUMSENSORS][3];     //Save last three values for colision avoidance
uint32_t lastMessageTime[NUMSENSORS]; //time last read ID (even if its not a new pass)


/*#######################################################
##                        Setup                        ##
#######################################################*/

void setup() 
{
  Serial.begin(9600);
  Serial.println();
  Serial.println();
  Serial.println(VERSION);
  Serial.println();

  //Sensor inputs
  for(uint8_t a = 0 ; a < NUMSENSORS ; a++)
  {
    pinMode(sensorPin[a], INPUT);
  }

  for(uint8_t a = 0 ; a < NUMSENSORS ; a++)
  {
    dataReady[a] = false;
  }

  pinMode(TONEPIN, OUTPUT);
  
  //Serial Command Interpreter
  initiateCommands();
}

/*#######################################################
##                        Loop                         ##
#######################################################*/

void loop() 
{ 
  //Read digital inputs
  for(int8_t sensor = (NUMSENSORS-1) ; sensor >= 0 ; sensor--)  //down-counting for-loop is faster then up-counting (compare against zero is faster)
  {
    state[sensor] = digitalRead(sensorPin[sensor]);
  }

  //Compare state against lastState
  for(int8_t sensor = (NUMSENSORS-1) ; sensor >= 0 ; sensor--)    
  {
    if(!dataReady[sensor])  //If data isn't analysed don't collect new data
    {
      if(state[sensor] != lastState[sensor])  //New pulse/pause
      { 
        lastState[sensor] = state[sensor];
        time = micros(); //save time for upcoming calculations
        uint32_t pulseLength = time - lastChange[sensor];
        lastChange[sensor] = time;

        if(pulseLength >= LOWBORDER)
        {
          if(pulseLength <= TOPBORDER)  //Same Signal
          {
            buf[sensor][counter[sensor]] = pulseLength;  //save time in input-buffer
            counter[sensor]++;

            if(counter[sensor] == NUMBITS)  //All bits read -> ready for check
            {
              counter[sensor] = 0;
              dataReady[sensor] = true;    
              dataReadyTime[sensor] = time;
            }
          }//if(pulseLength <= TOPBORDER)
          else    //New Signal
          {           

            //Serial.println();Serial.println(pulseLength);
            if(counter[sensor] != 0)    //Error occurred. New message but last message wasn't completed
            {
              if(fullDebug)
              {
                Serial.print(sensor); Serial.println(" NEB"); 
              }

              resetValue(sensor);
            }
            counter[sensor] = 0;           
          }
        }//if(pulseLength >= LOWBORDER)
        else
        {
          if(fullDebug)
          {
            Serial.print(sensor); Serial.print(" IT "); Serial.println(pulseLength);
          }
          resetValue(sensor);
        }//else(pulseLength >= LOWBORDER)
        
      }
    }
  }

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
          data[sensor] = ((message[sensor][2] << 5) + (message[sensor][3] << 4) + (message[sensor][4] << 3) + (message[sensor][5] << 2) + (message[sensor][6] << 1) + (message[sensor][7]));  //calculate command

          time = millis();
          if(twoOfFour(sensor, data[sensor]) && (time >= (lastPilotTime[data[sensor]] + MINLAPTIME)))  //New lap
          {
            newLap(data[sensor]);

            if(simpleDebug)
            {
              for(uint8_t a = 0; a < NUMBITS; a++)
              {
                Serial.print(buf[sensor][a]);Serial.print("\t");
              }
              Serial.println();
              for(uint8_t a = 0; a < NUMBITS; a++)
              {
                Serial.print(message[sensor][a]);Serial.print("\t");
              }
              Serial.println();
            }
            lastPilotTime[data[sensor]] = time;
          }
          saveValue(sensor, data[sensor]);
          lastMessageTime[sensor] = time;
          if(fullDebug)
          {
            Serial.print(sensor); Serial.print(" ND "); Serial.println(data[sensor]);
          }
        }
        else
        {
          if(fullDebug)
          {
            Serial.print(sensor); Serial.println(" CI ");
          }
        }            
      }
      else
      {
        if(fullDebug)
        {
          Serial.print(sensor); Serial.println(" SBI");
        }
      }
    }//if(dataReady)
  }//for(sensors)

  for(int8_t sensor = (NUMSENSORS-1) ; sensor >= 0 ; sensor--)
  {
    if(millis() >= (lastMessageTime[sensor] + DELETELAST))
      resetValue(sensor);
  }
  
  //Check for beep to end
  if(beepActive && beepEnd <= millis()) //end time reached and beep active
  { 
    digitalWrite(TONEPIN,LOW); //disable beep
    beepActive = false;
  }

  SCmd.readSerial();  //Call Serial handler
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

//New Lap completed
void newLap(uint8_t pilot)
{
  uint32_t time = millis();

  if(mode == 0) //OpenLap flying start mode
  {
    if(firstPass[pilot])  //First pass starts the Race
    {
      pass[pilot] = time; 
      firstPass[pilot] = false;
      
      Serial.print("NL "); Serial.print(pilot); Serial.print(","); Serial.print(0); Serial.print(","); Serial.println(0);  //Start the race will print a laptime of "0"
    }
    else   //Round completed
    {
      uint32_t laptime = (time - pass[pilot]);
      lap[pilot]++;  
      pass[pilot] = time;
   
      Serial.print("NL "); Serial.print(pilot); Serial.print(","); Serial.print(lap[pilot]); Serial.print(","); Serial.println(laptime);  //Print pilot, lap and laptime       
    }    
  }//if(mode == 0)
  else if(mode == 2)  //FPV Race Tracker Mode
  {
    uint32_t timeSinceReset = (time - pass[pilot]);
    uint32_t secs = timeSinceReset/1000;
    uint32_t mss = (timeSinceReset - (1000 * secs));

    char out[5] = {1, 64, 9, '1', 9};
    for(uint8_t outCount = 0; outCount < 5; outCount++)  
      Serial.print(out[outCount]);
    Serial.print(String(pilot)); Serial.print((char)9); Serial.print(String(secs)); Serial.print('.'); Serial.println(String(mss));
  }//else if(mode == 2)
  else if(mode == 3)  //EasyRaceLapTimer Mode
  {
    if(firstPass[pilot])  //First pass starts the Race
    {
      pass[pilot] = time; 
      firstPass[pilot] = false;
    }
    else   //Round completed
    {
      uint32_t laptime = (time - pass[pilot]);
      pass[pilot] = time;
   
      Serial.print("LAP_TIME "); Serial.print(pilot); Serial.print(" "); Serial.print(laptime); Serial.println("#"); //"LAP_TIME [ID] [TIME_IN_MS_FOR_LAP]#\n"
    }    
  }//else if(mode == 3)

  //Activate Buzzer
  if(!beepActive)
  {
    beep(BEEPLENGTH);
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

//Beep
void beep(uint32_t duration)
{
  digitalWrite(TONEPIN, HIGH); //enable beep
  beepActive = true;
  beepEnd = (millis() + duration);
}

