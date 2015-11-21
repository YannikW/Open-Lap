/*
This Code is published by Yannik Wertz under the "Creativ Common Attribution-NonCommercial-ShareAlike 4.0 International (CC BY-NC-SA 4.0)" License 
For more Information see : http://creativecommons.org/licenses/by-nc-sa/4.0/
*/

/*#######################################################
#########################################################

OpenLap
Open Source IR Laptimer

For more infos see :
www.openlap.de
https://github.com/YannikW/Open-Lap
https://www.facebook.com/groups/1047398441948165/


IR Transponder V0.6 (20.11.15) (for Attiny 44)

For transmitting protocol see : https://github.com/YannikW/Open-Lap/blob/master/docs/Open%20LapTime%20Protocol.md

   
#########################################################
#######################################################*/

/*#######################################################
##                      Libraries                      ##
#######################################################*/

#include <EEPROM.h>

/*#######################################################
##                       Defines                       ##
#######################################################*/

#define ZERO          200  //µS
#define ONE           550  //µS

#define SIGNALSPACE    20  //mS (+ 0-10ms Random Space for colision avoidance)
#define RANDOMSPACE    10  //ms (Care : 5 is exclusive)
#define DATABITS        6  //6 Databits -> 0-63

#define LEDPIN          6  //LED on PA6

//ID LED Pins
#define ONEPIN          0  //PA0
#define TWOPIN          1  //PA1
#define FOURPIN         2  //PA2
#define EIGHTPIN        3  //PA3

#define BUTTONPIN       7  //Momentary switch on PA7
#define INCTIME       500  //ms to increment ID again
#define LONGPRESS     300  //ms Press button loger than this to change band

/*#######################################################
##                      Variables                      ##
#######################################################*/

uint8_t id = 1;  //Standard ID is 1
uint8_t band = 1;

uint32_t lastInc = 0; //Time for last increment of ID

/*#######################################################
##                        Setup                        ##
#######################################################*/

void setup() 
{
  pinMode(ONEPIN, OUTPUT);        //Set ID LEDs as Output
  pinMode(TWOPIN, OUTPUT);
  pinMode(FOURPIN, OUTPUT);
  pinMode(EIGHTPIN, OUTPUT);

  id = EEPROM.read(0);            //Read last ID

  //Band
  if(id >= 48)
    band = 4;
  else if(id >= 32)
    band = 3;
  else if(id >= 16)
    band = 2;
  else
    band = 1; 

  //Display Band
  displayBand();

  //Display ID
  digitalWrite(ONEPIN, ((id >> 0) & 1));  //Set LEDs for new ID
  digitalWrite(TWOPIN, ((id >> 1) & 1));
  digitalWrite(FOURPIN, ((id >> 2) & 1));
  digitalWrite(EIGHTPIN, ((id >> 3) & 1));
  delay(1);

  pinMode(BUTTONPIN, INPUT);
  digitalWrite(BUTTONPIN, HIGH);  //Enable pull-ups 
  
  pinMode(LEDPIN, OUTPUT);
  
  //clear Timer1
  TCCR1A = 0;
  TCCR1B = 0;
  
  //set timer1 on 76kHz (2*38) toggle Pin 6
  TCCR1A |= (1 << COM1A0);    //Toggle OC1A on compare match
  TCCR1B |= (1 << WGM12);     //CTC Mode
  
  TCCR1B |= (1 << CS10);      //No Prescaler
  
  OCR1A = 209;                //(16000000 / 76000) - 1

  randomSeed(analogRead(4)); //Generate Random Seed.
}

/*#######################################################
##                        Loop                         ##
#######################################################*/

void loop() 
{
  sendOut();                  //Generate signal
  delay(SIGNALSPACE);         //Wait some static time
  delay(random(RANDOMSPACE)); //Wait another random time for collision avoidance 

  if(buttonPressed())         //Button is pressed -> Increment ID
  {
    delay(LONGPRESS);
    if(!digitalRead(BUTTONPIN))//It's a long button press -> change band
    {
      if(band >= 4)
        band = 1;
      else
        band++;

      id = (((band-1)*16) + 1); //Select first ID in this band
      
      displayBand();
    }
    else  //Short press -> change ID
    {
      if((id%16) >= 15)         //Max ID reached. Start from first ID in this band again
      {
        id = (((band-1)*16) + 1);
      }
      else                      //Increment ID
      {
        id++;
      }
    }
    //Display ID
    digitalWrite(ONEPIN, ((id >> 0) & 1));  //Set LEDs for new ID
    digitalWrite(TWOPIN, ((id >> 1) & 1));
    digitalWrite(FOURPIN, ((id >> 2) & 1));
    digitalWrite(EIGHTPIN, ((id >> 3) & 1));

    EEPROM.write(0, id);  //Save ID to EEPROM (first Byte)
  }
}

/*#######################################################
##                      Functions                      ##
#######################################################*/

void enableSignal()
{
  TCCR1B |= (1 << CS10);    //Enable Timer
  TCCR1A |= (1 << COM1A0);  //Enable toggle OC1A on compare match
}

void disableSignal()
{
  TCCR1B &= ~(1 << CS10);    //Disable Timer
  TCCR1A &= ~(1 << COM1A0);  //Get access for manualy set OC1A to low
  PORTA &= ~(1 << LEDPIN);   //Set Led to low
}


void sendOut()
{
  boolean checksum = 0;
  
  //Start Bits
  enableSignal();           //Startbit 1
  delayMicroseconds(ZERO);

  disableSignal();          //Startbit 2
  delayMicroseconds(ZERO);  
 
  //Data bits
  for(int8_t dataCount = (DATABITS - 1) ; dataCount >= 0 ; dataCount--)
  {
    if(dataCount%2) //Bit 5, 3 and 1
    {
      enableSignal();
    }
    else            //Bit 4, 2 and 0
    {
      disableSignal();
    }    
    if((id >> dataCount) % 2)  //Bit is "1"
    {
      delayMicroseconds(ONE);
      checksum = !checksum;  //Toggle Checksum if a "1" is send
    }
    else  //Bit is "0"
      delayMicroseconds(ZERO);
  }
  
  
  enableSignal();           //Checksum ("0" if number of 1s is even or "1" if it's odd)
  if(checksum)
    delayMicroseconds(ONE);
  else
    delayMicroseconds(ZERO);

  disableSignal();          //Complete last pulse
}

boolean buttonPressed()
{
  if(!digitalRead(BUTTONPIN) && (millis() >= lastInc + INCTIME))  //Button is pressed and enought time has passed
  {
      lastInc = millis();
      return true;
  }
  else
  {
    return false;
  }
}

//Blink one, two, three or four leds to show band
void displayBand()
{
  digitalWrite(ONEPIN, LOW);    //Disable all leds
  digitalWrite(TWOPIN, LOW);
  digitalWrite(FOURPIN, LOW);
  digitalWrite(EIGHTPIN, LOW);
  
  for(uint8_t a = 0; a < 10; a++)
  {
    if(band >= 4)
      digitalWrite(EIGHTPIN, HIGH);
    if(band >= 3)
      digitalWrite(FOURPIN, HIGH);
    if(band >= 2)
      digitalWrite(TWOPIN, HIGH);

    digitalWrite(ONEPIN, HIGH);   //band is always >= 1
    
    delay(100);

    digitalWrite(ONEPIN, LOW);    //Disable all leds
    digitalWrite(TWOPIN, LOW);
    digitalWrite(FOURPIN, LOW);
    digitalWrite(EIGHTPIN, LOW);

    delay(100);
  }
}

