/*#######################################################
#########################################################

IR Miniquad Laptimer

IR Transponder V0.1 (06.09.15) (for Atmega328)

Signal:
Start Bits | Data (e.g) | Checksum (0 if number of 1s is even or 1 if it's odd)
   0   0   | 0  1  1  0 | 0  
   
#########################################################
#######################################################*/

/*#######################################################
##                       Defines                       ##
#######################################################*/

#define COMMAND         4  //Individual number between 0-15

#define ZERO          300  //µS
#define ONE           600  //µS
#define SPACE        1000  //µS

#define SIGNALSPACE    10  //mS

#define DATABITS        4  //4 Databits -> 0-15

#define LEDPIN          3  //LED on D3

/*#######################################################
##                        Setup                        ##
#######################################################*/

void setup() 
{
  pinMode(LEDPIN, OUTPUT);
  
  //clear Timer2
  TCCR2A = 0;
  TCCR2B = 0;
  
  //set timer2 on 76kHz (2*38) toggle Pin D3
  TCCR2A |= (1 << COM2B0);    //Toggle OC2B on compare match
  TCCR2A |= (1 << WGM21);     //CTC Mode
  
  TCCR2B |= (1 << CS20);      //No Prescaler
  
  OCR2A = 209;                //(16000000 / 76000) - 1
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
  TCCR2B |= (1 << CS20);    //Enable Timer
  TCCR2A |= (1 << COM2B0);  //Enable toggle OC2B on compare match
}

void disableSignal()
{
  TCCR2B &= ~(1 << CS20);    //Disable Timer
  TCCR2A &= ~(1 << COM2B0);  //Get access for manualy set OC2B to low
  PORTD &= ~(1 << LEDPIN);   //Set Led to low
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
    if((COMMAND >> dataCount) % 2)  //Bit is "1"
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
