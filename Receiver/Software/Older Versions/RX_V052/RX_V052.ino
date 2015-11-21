/*
This Code is published by Yannik Wertz under the "Creativ Common Attribution-NonCommercial-ShareAlike 4.0 International (CC BY-NC-SA 4.0)" License 
For more Information see : http://creativecommons.org/licenses/by-nc-sa/4.0/
*/

/*#######################################################
#########################################################

Open Lap
Infrared Laptimer for Miniquad Racing

IR Receiver V0.5.2 (16.10.15)

Signal:
Start Bits | Data (e.g) | Checksum (0 if number of 1s is even or 1 if it's odd)
   0   0   | 0  1  1  0 | 0  


#########################################################
#########################################################

Serial output is :

[Sensor Number] [Command] [optional parameter]

Commands are :

ND [pilot number] = new data. successful reading
NEB = not enought bits
IC [time] = incorrect time measured
SBI = start bits incorrect
CI = checksum incorrect
NP = new pass


/*#######################################################
##                       Defines                       ##
#######################################################*/

#define VERSION      "0.5.2"  //Version number

#define NUMBITS          7  //Starts Bits, data and checksum
#define DATABITS         4  //4 Databits -> 0-15

#define CRITTIME       318  //µS border for decision 0 or 1
#define SIGNALSPACE   1500  //µS
#define LOWBORDER       40  //µS (160µS-(4/38000)s = 54µS minus a little tolerance)
#define TOPBORDER      610  //µS (425µS+(6/38000)s = 583µS plus a little tolerance) 

#define TONEPIN          7  //Buzzer on pin D7
#define BEEPLENGTHONE  200  //mS for ID 1
#define BEEPLENGTHTWO  400  //mS for ID 2

#define NEWPASS        500  //mS

#define ANALYSEDELAY   50  //µS

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

uint32_t fallTime1;
uint32_t getTime1; 
uint32_t lastValue1;
uint8_t counter1;
uint16_t buf1[8];
uint8_t message1[7];
boolean dataReady1 = false;
boolean checksum1;
uint8_t data1;


boolean getValueFalling0_Flag = false;
boolean getValueRising0_Flag = false;

boolean getValueFalling1_Flag = false;
boolean getValueRising1_Flag = false;

uint32_t beepEnd = 0; //time for beep to end, see beep()
boolean beepActive = false;

uint32_t lastPass = 0;  //print a newline if 

uint32_t dataReadTime0;
uint32_t dataReadTime1;

/*#######################################################
##                        Setup                        ##
#######################################################*/

void setup() 
{
  Serial.begin(250000);

  //Pinmodes
  pinMode(TONEPIN, OUTPUT);
  digitalWrite(TONEPIN, LOW);

  Serial.print("RX V"); Serial.println(VERSION);

  //Sensor interrupts
  attachInterrupt(0, getValueFalling0, FALLING);
  attachInterrupt(1, getValueFalling1, FALLING);
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
      //Serial.print(getTime); Serial.print(" ");
      if(time - lastValue0 <= SIGNALSPACE)  //Still the same message
      {
        buf0[counter0] = getTime0;  //save time in input-buffer
        counter0++;
      }
      else  //New Message
      {
        if(counter0 != 0)    //Error occurred. New message but last message wasn't completed
          Serial.println("0 NEB"); 
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
      Serial.print("0 IT "); Serial.println(getTime0);
      
      counter0 = 0; //reset counter and activate interrupt
      attachInterrupt(0, getValueFalling0, FALLING);
    }
  }

  
  if (getValueFalling1_Flag)
  {
    getValueFalling1_Flag = false; //reset flag
    
    fallTime1 = micros();  
    attachInterrupt(1, getValueRising1, RISING);
  }
  if (getValueRising1_Flag)
  {
    getValueRising1_Flag = false; //reset flag
    
    uint32_t time = micros();
  
    getTime1 = time - fallTime1;  //Calculate puls-time
    
    //Serial.print(getTime); Serial.print(" ");
    if(time - lastValue1 <= SIGNALSPACE)  //Still the same message
    {
      buf1[counter1] = getTime1;  //save time in input-buffer
      counter1++;
    }
    else  //New Message
    {
      if(counter1 != 0)    //Error occurred. New message but last message wasn't completed
        Serial.println("1 NEB"); 
      buf1[0] = getTime1;   //Start new message
      counter1 = 1;
    }
    lastValue1 = time;  //save actual time for next input
    
    if(counter1 == NUMBITS)  //All bits read -> ready for check
    {
      detachInterrupt(1);  //While data is not analysed, we don't want new data
      counter1 = 0;
      dataReady1 = true;    
      dataReadTime1 = micros();
    }
    else  //message isn't completed yet - waiting for next value
      attachInterrupt(1, getValueFalling1, FALLING);
  }

  
  
  if(dataReady0 && micros() >= (dataReadTime0 + ANALYSEDELAY))  //New message is ready
  {
    dataReady0 = false;
    uint32_t pass = millis();
    if(pass >= lastPass + NEWPASS)  //Just print an empty line
      Serial.println("NP");
    lastPass = pass;
      
    for(int8_t a = 0; a < NUMBITS ; a++)  //Calculate bit message
    { 
      //Serial.print(buf0[a]); Serial.print(" ");
      message0[a] = getValue(buf0[a]);
      //Serial.print(message0[a]); Serial.print(" ");
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
        
        Serial.print("0 ND "); Serial.println(data0); 
//        Serial.print("  ");
//        for(uint8_t a = 0 ; a < NUMBITS  ; a++)
//        {
//          Serial.print(buf0[a], DEC); Serial.print(" ");
//        }
//        Serial.println();
        if(data0 == 1 && !beepActive)  //Short Beep for ID 1
        {
          beep(BEEPLENGTHONE);
        }
        else if(data0 == 2 &&!beepActive) //Long Beep for ID 2
        {
          beep(BEEPLENGTHTWO);
        }
      }
      else
      {
        Serial.print("0 CI ");
//        for(uint8_t a = 0 ; a < NUMBITS  ; a++)
//        {
//          Serial.print(message0[a], DEC); Serial.print(" ");
//        }
        for(uint8_t a = 0 ; a < NUMBITS  ; a++)
        {
          Serial.print(buf0[a], DEC); Serial.print(" ");
        }
        Serial.println();
      }
              
    }
    else
      Serial.println("0 SBI");
      
    attachInterrupt(0, getValueFalling0, FALLING);  //Reactiavte Interrupt
  }

  if(dataReady1 && micros() >= (dataReadTime1 + ANALYSEDELAY))  //New message is ready
  {
    dataReady1 = false;
    uint32_t pass = millis();
    if(pass >= lastPass + NEWPASS)  //Just print an empty line
      Serial.println("NP");
    lastPass = pass;
    
    for(int8_t a = 0; a < NUMBITS ; a++)  //Calculate bit message
    { 
      //Serial.print(buf1[a]); Serial.print(" ");
      message1[a] = getValue(buf1[a]);
      //Serial.print(message1[a]); Serial.print(" ");
    }
  
    if(message1[0] == 0 && message1[1] == 0)  //Start Bits
    {      
      checksum1 = false;
      for(uint8_t b = 0; b < DATABITS; b++)  //Calculating Checksum
      {
          if(message1[2+b])    //if it's a one, toggle checksum
            checksum1 = !checksum1;
      }
      
      if(checksum1 == message1[2 + DATABITS])  //is caculated checksum the same as read checksum?
      {
        data1 = ((message1[2] << 3) + (message1[3] << 2) + (message1[4] << 1) + (message1[5]));  //calculate command
        
        Serial.print("1 ND "); Serial.println(data1); 
//        Serial.print("  ");
//        for(uint8_t a = 0 ; a < NUMBITS  ; a++)
//        {
//          Serial.print(buf1[a], DEC); Serial.print(" ");
//        }
//        Serial.println();
        if(data1 == 1 && !beepActive)  //Short Beep for ID 1
        {
          beep(BEEPLENGTHONE);
        }
        else if(data1 == 2 &&!beepActive) //Long Beep for ID 2
        {
          beep(BEEPLENGTHTWO);
        }
      }
      else
      {
        Serial.print("1 CI ");
        for(uint8_t a = 0 ; a < NUMBITS  ; a++)
        {
          Serial.print(buf1[a], DEC); Serial.print(" ");
        }
        Serial.println();
      }      
    }
    else
      Serial.println("1 SBI");
      
    attachInterrupt(1, getValueFalling1, FALLING);  //Reactiavte Interrupt
  }

  
  //BEEP
  if(beepActive && beepEnd <= millis()) //end time reached and beep active
  { 
    digitalWrite(TONEPIN,LOW); //disable beep
    beepActive = false;
  }

}

/*#######################################################
##                      Functions                      ##
#######################################################*/

void beep(uint32_t d)
{
  digitalWrite(TONEPIN, HIGH); //enable beep
  beepActive = true;
  beepEnd = millis() + d;
}

uint8_t getValue(uint16_t time)
{
  if(time <= CRITTIME)  //if time < CRITTIME it's a zero, if it's bigger its a one
    return 0;
  else
    return 1;
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

void getValueFalling1()    //Puls begins
{
  getValueFalling1_Flag = true;
}

void getValueRising1()    //Puls ended
{ 
  getValueRising1_Flag = true;  
}
