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


IR Transponder with fixed ID V0.6 (22.12.15) (for Attiny 45 or 85 with internal 8Mhz clock)



   
#########################################################
#######################################################*/

/*#######################################################
##                      Libraries                      ##
#######################################################*/


/*#######################################################
##                       Defines                       ##
#######################################################*/

#define ID             60  //Put in your wanted ID here! Between 1 and 63.

#define ZERO          200  //µS
#define ONE           550  //µS

#define SIGNALSPACE    20  //mS (+ 0-10ms Random Space for colision avoidance)
#define RANDOMSPACE    10  //ms (Care : 5 is exclusive)
#define DATABITS        6  //6 Databits -> 0-63
#define MAXID          63  //2^6 - 1 = 63

#define LEDPIN          1  //LED on PB1 (Pin 6 on SOIC package)




/*#######################################################
##                      Variables                      ##
#######################################################*/

uint8_t id = ID;  //Do not change this one - change the "ID" above under Defines.


/*#######################################################
##                        Setup                        ##
#######################################################*/

void setup() 
{ 
  pinMode(LEDPIN, OUTPUT);  //Set Pin A6 as output
    
  //clear Timer1
  TCCR1 = 0;
  GTCCR = 0;
  
  //set timer1 on 76kHz (2*38) toggle Pin 6
  TCCR1 |= (1 << COM1A0);     //Toggle OC1A on compare match
  TCCR1 |= (1 << CTC1);       //CTC Mode
  
  TCCR1 |= (1 << CS10);       //No Prescaler
  
  OCR1C = 104;                //(8000000 / 76000) - 1

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
}

/*#######################################################
##                      Functions                      ##
#######################################################*/

void enableSignal()
{
  TCCR1 |= (1 << CS10);    //Enable Timer
  TCCR1 |= (1 << COM1A0);  //Enable toggle OC1A on compare match
}

void disableSignal()
{
  TCCR1 &= ~(1 << CS10);    //Disable Timer
  TCCR1 &= ~(1 << COM1A0);  //Get access for manualy set OC1A to low
  PORTB &= ~(1 << LEDPIN);  //Set Led to low
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
