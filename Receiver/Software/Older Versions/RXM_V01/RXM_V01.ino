/*
This Code is published by Yannik Wertz under the "Creativ Common Attribution-NonCommercial-ShareAlike 4.0 International (CC BY-NC-SA 4.0)" License 
For more Information see : http://creativecommons.org/licenses/by-nc-sa/4.0/


#########################################################
#########################################################

Open Lap
Infrared Laptimer for Miniquad Racing

IR Master Receiver V0.1 (04.10.15)

The IR Master Receiver manage all the sensor data received by the slaves, calculate times und send out to visualize.

#########################################################
#########################################################

Serial output format is :
[PILOT],[LAP],[LAPTIME]
   
/*#######################################################
##                       Defines                       ##
#######################################################*/

#include <Wire.h>

#define VERSION      "0.1"  //Version number

#define SLAVEID         10  //I2C Adress of first Slave
#define NUMBERSLAVES     2  //Number of slaves connected to Master

#define REQUESTWIRE      4  //D4 ist requestwire for first slave, D5 for second, ...

#define MINSPACE      5000  //ms to register a new completed lap

#define TONEPIN         A0  //Buzzer on A0
#define BEEPLENGTH     100  //ms


/*#######################################################
##                      Variables                      ##
#######################################################*/

uint32_t pass[16];   //Save time for start/finish passes for each of the 16 pilots
boolean firstPass[16] = {true, true, true, true, true, true, true, true, //First pass will only save time and not calculate a laptime
                         true, true, true, true, true, true, true, true};
uint32_t lap[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};  //Rounds completed for each pilot

uint32_t beepEnd = 0;       //Time for beep to end
boolean beepActive = false;

/*#######################################################
##                        Setup                        ##
#######################################################*/

void setup() 
{
  Serial.begin(115200);
  Serial.println("Open Lap");
  Serial.print("RX Master Version "); Serial.println(VERSION);
  
  Wire.begin();
  Wire.setClock(400000L);   //Fast I2C Mode - 400kHz (standard is 100kHz)

  //Configure requestwires as inputs for slaves
  for(int8_t pin = REQUESTWIRE ; pin < (REQUESTWIRE + NUMBERSLAVES) ; pin++)
  {
    pinMode(pin, INPUT); 
  }

  //Configure buzzer as output
  pinMode(TONEPIN, OUTPUT);
  digitalWrite(TONEPIN, LOW);
}

/*#######################################################
##                        Loop                         ##
#######################################################*/

void loop() 
{ 
  //Check for new data available from any slave
  for(int8_t slave = 0 ; slave < NUMBERSLAVES ; slave++)
  {
    if(digitalRead(REQUESTWIRE + slave))  //Is Signal HIGH? -> Request Data
    {
      requestData(SLAVEID + slave);
    }
  }

  //Check for beep to end
  if(beepActive && beepEnd <= millis()) //end time reached and beep active
  { 
    digitalWrite(TONEPIN,LOW); //disable beep
    beepActive = false;
  }
}

/*#######################################################
##                      Functions                      ##
#######################################################*/

//Request Data from slave
void requestData(uint8_t adress)
{
  uint8_t pilot = 0;
  uint8_t bytes = 1;  //request 1 byte
  
  Wire.requestFrom(adress, bytes);  //Request the pilot number
  if(Wire.available() == 1)     //Byte is ready
  {
    pilot = Wire.read();
  }
  else
  {
    Serial.println("I2C Error"); //Something went wrong :(
  }

  printTime(pilot); //Print out the new data
}

//Print out the new data
void printTime(uint8_t pilot)
{
  uint32_t time = millis();
  

  if(firstPass[pilot])  //First pass starts the Race
  {
    pass[pilot] = time; 
    firstPass[pilot] = false;
    
    Serial.print(pilot); Serial.print(","); Serial.print(0); Serial.print(","); Serial.println(0);  //Start the race will print a laptime of "0"
    if(!beepActive)
    {
      beep(BEEPLENGTH);
    }
  }
  else   //Round completed
  {
    uint32_t laptime = (time - pass[pilot]);
    if(laptime >= MINSPACE)
    {
      lap[pilot]++;  
      pass[pilot] = time;
   
      Serial.print(pilot); Serial.print(","); Serial.print(lap[pilot]); Serial.print(","); Serial.println(laptime);  //Print pilot, lap and laptime    
      if(!beepActive)
      {
        beep(BEEPLENGTH);
      }
    }
  }
}

//Beep
void beep(uint32_t duration)
{
  digitalWrite(TONEPIN, HIGH); //enable beep
  beepActive = true;
  beepEnd = (millis() + duration);
}

