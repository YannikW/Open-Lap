/*
This Code is published by Yannik Wertz under the "Creativ Common Attribution-NonCommercial-ShareAlike 4.0 International (CC BY-NC-SA 4.0)" License 
For more Information see : http://creativecommons.org/licenses/by-nc-sa/4.0/
*/

/*#######################################################
#########################################################

IR Miniquad Laptimer

IR Transponder V0.5 (14.10.15) (for Attiny 44)

Signal:
Start Bits | Data (e.g) | Checksum (0 if number of 1s is even or 1 if it's odd)
   0   0   | 0  1  1  0 | 0  


Zero : 6  Cycles (~160µS) puls + 270µS pause (37% duty cycle) = 430µS
One : 16  Cycles (~425µS) puls + 270µS pause (61% duty cycle) = 695µS

Signal length will be between 3275µS and 4070µS. With a signalspace of 20ms, duty cycle is around 15%.


Changelog:

V0.5:
  - ID selection with a momentary switch
  - LEDs shows actual ID

   
#########################################################
#######################################################*/

/*#######################################################
##                      Libraries                      ##
#######################################################*/

#include <EEPROM.h>

/*#######################################################
##                       Defines                       ##
#######################################################*/

#define ZERO          160  //µS
#define ONE           425  //µS
#define SPACE         270  //µS

#define SIGNALSPACE    18  //mS (+ 0-4ms Random Space for colision avoidance
#define RANDOMSPACE     5  //ms (Care : 5 is exclusive)
#define DATABITS        4  //4 Databits -> 0-15
#define MAXID          15  //2^4 - 1 = 15

#define LEDPIN          6  //LED on PA6

//ID LED Pins
#define ONEPIN          0  //PA0
#define TWOPIN          1  //PA1
#define FOURPIN         2  //PA2
#define EIGHTPIN        3  //PA3

#define BUTTONPIN       7  //Momentary switch on PA7
#define INCTIME       500  //ms to increment ID again

/*#######################################################
##                      Variables                      ##
#######################################################*/

uint8_t id = 1;  //Sstandard ID is 1

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

  randomSeed(42); //Generate Random Seed. With fixed number it isn't really random, but multiple transmitters won't be plugged in exactly the same time ;)
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
    if(id >= 15)              //Max ID reached. Start from 1 again
    {
      id = 1;
    }
    else                      //Increment ID
    {
      id++;
    }
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

void zero()
{
  enableSignal();
  delayMicroseconds(ZERO);
  disableSignal();
  delayMicroseconds(SPACE);
}

void one()
{
  enableSignal();
  delayMicroseconds(ONE);
  disableSignal();
  delayMicroseconds(SPACE);
}

void sendOut()
{
  boolean checksum = 0;
  
  //Start Bits
  zero();
  zero();
  
  //Data bits
  for(int8_t dataCount = (DATABITS - 1) ; dataCount >= 0 ; dataCount--)
  {
    if((id >> dataCount) % 2)  //Bit is "1"
    {
      one();
      checksum = !checksum;  //Toggle Checksum if a "1" is send
    }
    else  //Bit is "0"
      zero();
  }
  
  //Checksum ("0" if number of 1s is even or "1" if it's odd)
  if(checksum)
    one();
  else
    zero();
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

