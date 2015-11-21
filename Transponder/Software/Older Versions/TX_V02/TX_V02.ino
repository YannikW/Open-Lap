/*
This Code is published by Yannik Wertz under the "Creativ Common Attribution-NonCommercial-ShareAlike 4.0 International (CC BY-NC-SA 4.0)" License 
For more Information see : http://creativecommons.org/licenses/by-nc-sa/4.0/
*/

/*#######################################################
#########################################################

IR Miniquad Laptimer

IR Transponder V0.2 (10.09.15) (for Attiny 44)

Signal:
Start Bits | Data (e.g) | Checksum (0 if number of 1s is even or 1 if it's odd)
   0   0   | 0  1  1  0 | 0  
   
#########################################################
#######################################################*/

/*#######################################################
##                       Defines                       ##
#######################################################*/

#define ZERO          300  //µS
#define ONE           600  //µS
#define SPACE        1000  //µS

#define SIGNALSPACE    10  //mS

#define DATABITS        4  //4 Databits -> 0-15

#define LEDPIN          6  //LED on PA6

//ID Pins
#define ONEPIN          0  //PA0
#define TWOPIN          1  //PA1
#define FOURPIN         2  //PA2
#define EIGHTPIN        3  //PA3

/*#######################################################
##                      Variables                      ##
#######################################################*/

uint8_t id = 0;  //Select ID by solderjumpers

/*#######################################################
##                        Setup                        ##
#######################################################*/

void setup() 
{
  pinMode(ONEPIN, INPUT);       //Set to input
  pinMode(TWOPIN, INPUT);
  pinMode(FOURPIN, INPUT);
  pinMode(EIGHTPIN, INPUT);
  digitalWrite(ONEPIN, HIGH);   //Enable pull-ups
  digitalWrite(TWOPIN, HIGH);
  digitalWrite(FOURPIN, HIGH);
  digitalWrite(EIGHTPIN, HIGH);
  delay(1);
  
  //Get ID
  id |=  (!digitalRead(ONEPIN) << 0);
  id |=  (!digitalRead(TWOPIN) << 1);
  id |=  (!digitalRead(FOURPIN) << 2);
  id |=  (!digitalRead(EIGHTPIN) << 3);
  
  pinMode(LEDPIN, OUTPUT);
  
  //clear Timer1
  TCCR1A = 0;
  TCCR1B = 0;
  
  //set timer1 on 76kHz (2*38) toggle Pin 6
  TCCR1A |= (1 << COM1A0);    //Toggle OC1A on compare match
  TCCR1B |= (1 << WGM12);     //CTC Mode
  
  TCCR1B |= (1 << CS10);      //No Prescaler
  
  OCR1A = 209;                //(16000000 / 76000) - 1
}

/*#######################################################
##                        Loop                         ##
#######################################################*/

void loop() 
{
  sendOut();
  delay(SIGNALSPACE);
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