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

IR Receiver V0.6.1 (17.12.15) for NodeMCU V1.0 ESP8266 Board

************************************************************************************************************
* Special Thanks to:                                                                                       *
*                                                                                                          *
* Markus Brunner (Co-Management, Testing and Development)                                                  *
* Andreas Neubauer and Oliver Renner (FPV Race Tracker Software https://www.facebook.com/fpvracetracker/ ) *
* Alexander Bierbraucher (EasyRaceLapTimer)                                                                *
************************************************************************************************************

/*#######################################################
##                       Defines                       ##
#######################################################*/

#define VERSION "OpenLap - RX ESP8266 - Version 0.6.1 (16.12.15)"  //Version number

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

//Wifi connection
#define FPVRT            1  //Connecting to OpenLap AP (for FPV Race Tracker)
#define EASYRACE         2  //Connecting to EasyRaceLapTimer AP


/*#######################################################
##                 Includes & Objects                  ##
#######################################################*/

#include <ESP8266WiFi.h>      //For connecting to a TCP server for wireless communication
WiFiClient client;

#include "SerialCommand.h"    //see : https://github.com/scogswell/ArduinoSerialCommand
SerialCommand SCmd;           //Serial commands
SerialCommand TCPCmd(client); //TCP commands


/*#######################################################
##                      Variables                      ##
#######################################################*/

boolean simpleDebug = false;          //print pulse times and calculated bit message
boolean fullDebug = false;            //print all readings and errors

uint8_t mode = 0;                     //0 : OpenLap - flying start (default)
                                      //1 : OpenLap - start on countdown - not supported yet
                                      //2 : FPV Race Tracker (start on countdown)
                                      //3 : EasyRaceLapTimer (flying start)
                                      //4 : FPV Race Tracker (TCP Connection)

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

uint32_t actTime;

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

//WiFi Settings
uint8_t wifiConnected = 0;

//FPV Race Tracker
const char* openLapSsid = "OpenLap";
const char* openLapPassword = "123456789";
IPAddress openLapHost(192, 168, 173, 1);
unsigned int openLapPort = 8888;

//EasyRaveLapTimer
const char* easyRaceSsid = "EasyRaceLapTimer";
const char* easyRacePassword = "0123456789";
IPAddress easyRaceHost(192, 168, 42, 1);
unsigned int easyRacePort = 3006;

/*#######################################################
##                Function Declaration                 ##
#######################################################*/

void setup();                                   //initialize everything
void loop();                                    //check the sensors and check for new round
uint8_t getValue(uint16_t inTime);              //calculate bit message
void newLap(uint8_t pilot);                     //a new lap has been triggered -> do all the output stuff
boolean twoOfFour(uint8_t sensor, uint8_t id);  //filter false readings
void saveValue(uint8_t sensor, uint8_t id);     //--------"--------
void resetValue(uint8_t sensor);                //--------"--------
void beep(uint32_t duration);                   //activate the buzzer
boolean connectWifi();                          //connect to wifi
void initiateCommands();                        //teach all the serial/tcp commands
void sayVersion();                              //print version number
void setDebugmode();                            //enable or disable debugmode
void fpvRaceTracker();                          //initiate fpv race tracker mode (serial connection)
void fpvRaceTrackerTCP();                       //initiate fpv race tracker mode (wireless connection)
void resetRace();                               //reset lap count 
void easyRace();                                //initiate EasyRaceLapTimer mode
void help();                                    //show all usercommands
void unrecognized();                            //default handler for unknown commands


/*#######################################################
##                        Setup                        ##
#######################################################*/

void setup() 
{
  Serial.begin(BAUDRATE);
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

  //Wifi
  wifiConnected = connectWifi();  //Connect to WiFi if possible and save which WiFi is used 

  if(wifiConnected == FPVRT)
  {
    mode = 4;
    if (!client.connect(openLapHost, openLapPort))  //Connecting to TCP Server
      Serial.println("Connection to OpenLap TCP server failed");
    else
      Serial.println("Connection to OpenLap TCP server successful");
  }
  else if(wifiConnected == EASYRACE)
  {
    mode = 3;
    if (!client.connect(easyRaceHost, easyRacePort))  //Connecting to TCP Server
      Serial.println("Connection to EasyRaceLapTimer TCP server failed");
    else
      Serial.println("Connection to EasyRaceLapTimer TCP server successful");
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
        actTime = micros(); //save time for upcoming calculations
        uint32_t pulseLength = actTime - lastChange[sensor];
        lastChange[sensor] = actTime;

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
              dataReadyTime[sensor] = actTime;
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

          actTime = millis();
          if(twoOfFour(sensor, data[sensor]) && (actTime >= (lastPilotTime[data[sensor]] + MINLAPTIME)))  //New lap
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
            lastPilotTime[data[sensor]] = actTime;
          }
          saveValue(sensor, data[sensor]);
          lastMessageTime[sensor] = actTime;
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

  SCmd.readSerial();    //Call Serial handler
  TCPCmd.readSerial();  //Call TCP handler

}

/*#######################################################
##                      Functions                      ##
#######################################################*/

uint8_t getValue(uint16_t inTime)
{
  if(inTime <= CRITTIME)  //if time < CRITTIME it's a zero, if it's bigger its a one
    return 0;
  else
    return 1;
}

//New Lap completed
void newLap(uint8_t pilot)
{
  uint32_t actTime = millis();

  if(mode == 0) //OpenLap flying start mode
  {
    if(firstPass[pilot])  //First pass starts the Race
    {
      pass[pilot] = actTime; 
      firstPass[pilot] = false;
      
      Serial.print("NL "); Serial.print(pilot); Serial.print(","); Serial.print(0); Serial.print(","); Serial.println(0);  //Start the race will print a laptime of "0"
    }
    else   //Round completed
    {
      uint32_t laptime = (actTime - pass[pilot]);
      lap[pilot]++;  
      pass[pilot] = actTime;
   
      Serial.print("NL "); Serial.print(pilot); Serial.print(","); Serial.print(lap[pilot]); Serial.print(","); Serial.println(laptime);  //Print pilot, lap and laptime       
    }    
  }//if(mode == 0)
  else if(mode == 2 || mode == 4)  //FPV Race Tracker Mode
  {
    uint32_t timeSinceReset = (actTime - pass[pilot]);
    uint32_t secs = timeSinceReset/1000;
    uint32_t mss = (timeSinceReset - (1000 * secs));

    char out[5] = {1, 64, 9, '1', 9};
    for(uint8_t outCount = 0; outCount < 5; outCount++)  
    {
      if(mode == 2)
        Serial.print(out[outCount]);
      else
        client.print(out[outCount]);
    }
    if(mode == 2) //Serial connection
    {
      Serial.print(String(pilot)); Serial.print((char)9); Serial.print(String(secs)); Serial.print('.'); Serial.println(String(mss));
    }
    else          //TCP connection
    {
      client.print(String(pilot)); client.print((char)9); client.print(String(secs)); client.print('.'); client.println(String(mss));
    }
  }//else if(mode == 2)
  else if(mode == 3)  //EasyRaceLapTimer Mode
  {
    if(firstPass[pilot])  //First pass starts the Race
    {
      pass[pilot] = actTime; 
      firstPass[pilot] = false;
    }
    else   //Round completed
    {
      uint32_t laptime = (actTime - pass[pilot]);
      pass[pilot] = actTime;

      client.print("LAP_TIME "); client.print(pilot); client.print(" "); client.print(laptime); client.println("#"); //"LAP_TIME [ID] [TIME_IN_MS_FOR_LAP]#\n"
      //Serial.print("LAP_TIME "); Serial.print(pilot); Serial.print(" "); Serial.print(laptime); Serial.println("#"); //"LAP_TIME [ID] [TIME_IN_MS_FOR_LAP]#\n"
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

//connect to wifi – returns true if successful or false if not
boolean connectWifi() 
{
  for(uint8_t wifimode = 1 ; wifimode <= 2 ; wifimode++)  //Try FPV Race Tracker first, then EasyRaceLapTimer
  {
    boolean state = true;
    int i = 0;
    Serial.println();
    Serial.println("Connecting to WiFi");
    
    if(wifimode == FPVRT)
      WiFi.begin(openLapSsid, openLapPassword);
    else if(wifimode == EASYRACE)
      WiFi.begin(easyRaceSsid, easyRacePassword);
  
    // Wait for connection
    Serial.print("Connecting");
    while (WiFi.status() != WL_CONNECTED) //Try connecting for 5s, before exit
    {
      delay(500);
      Serial.print(".");
      if (i > 10) 
      {
        state = false;
        break;
      }
      i++;
    }
    if (state) {
      Serial.println();
      Serial.print("Connected to ");
      
      if(wifimode == FPVRT)
        Serial.println(openLapSsid);
      else
        Serial.println(easyRaceSsid);
        
      Serial.print("IP address: ");
      Serial.println(WiFi.localIP());
      return wifimode;                  //if connection established return what wifi has connected
    }
    else {
      Serial.println();
      Serial.println("Connection failed.");
    }
  }
  return false; //return false if no wifi is in range
}
