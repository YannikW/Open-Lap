/*
This Code is published by Yannik Wertz under the "Creativ Common Attribution-NonCommercial-ShareAlike 4.0 International (CC BY-NC-SA 4.0)" License 
For more Information see : http://creativecommons.org/licenses/by-nc-sa/4.0/
*/

/*#######################################################
#########################################################

IR Miniquad Laptimer

IR Receiver V0.6 (16.10.15)

Signal:
Start Bits | Data (e.g) | Checksum (0 if number of 1s is even or 1 if it's odd)
   0   0   | 0  1  1  0 | 0  
   
#########################################################
#########################################################

Changelog:
V0.2: (thanks fisch ;) )
  - shorten ISR by only setting Flags, which checked in Loop
  - beep without delay

V0.3:
  - add second Sensor
  - print a newline if it's a new pass (printing millis isn't working, probably it takes to long)  

V0.4:
  - new sensors (TSOP32138) for shorter bursts. New crittime is 318µS (160µS + (6/38000)s = 318µS). More infos see datasheet of TSOP32138.

V0.5:
  - Check if measured time is in possible range. If it's not set counter to 0 and check for new message. (Sensor0 only)

V0.6:
  - Interrupt methode seems to get weired when connected more then one sensor
    I changed to a polling method. Code can't get weired because of to slow reenabling interrupt. Only error gets bigger.
  - Get for-loop for all sensors
  - Shorten Serial prints.. Ruins code because they take forever (100-200µS each print)


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

#define VERSION      "0.6"  //Version number

#define NUMBITS          7  //Starts Bits, data and checksum
#define DATABITS         4  //4 Databits -> 0-15

#define CRITTIME       318  //µS border for decision 0 or 1
#define SIGNALSPACE   1200  //µS
#define LOWBORDER       40  //µS (160µS-(4/38000)s = 54µS minus a little tolerance)
#define TOPBORDER      750  //µS (425µS+(6/38000)s = 583µS plus a little tolerance) 

#define TONEPIN          7  //Buzzer on pin D7
#define BEEPLENGTHONE  200  //mS for ID 1
#define BEEPLENGTHTWO  400  //mS for ID 2

#define NEWPASS        500  //mS

#define NUMSENSORS       2  //Only one Sensor connected
#define FIRSTSENSORPIN   2  //First sensor connected to D2, next D3,...

#define pinRead(x,y)  (x & (1 << y))  //e.g. read state of D2 : pinRead(PIND, 2)

#define ANALYSEDELAY   100  //µS

/*#######################################################
##                      Variables                      ##
#######################################################*/

uint32_t fallTime[NUMSENSORS];
uint32_t getTime[NUMSENSORS]; 
uint32_t lastValue[NUMSENSORS];
uint8_t counter[NUMSENSORS];
uint16_t buf[NUMSENSORS][8];
boolean dataReady[NUMSENSORS];
uint32_t dataReadyTime[NUMSENSORS];
uint8_t message[NUMSENSORS][7];
boolean checksum[NUMSENSORS];
uint8_t data[NUMSENSORS];



uint32_t beepEnd = 0; //time for beep to end, see beep()
boolean beepActive = false;

uint32_t lastPass = 0;  //print a newline if 

boolean state[NUMSENSORS];
boolean lastState[NUMSENSORS];


/*#######################################################
##                        Setup                        ##
#######################################################*/

void setup() 
{
  Serial.begin(115200);

  //Pinmodes
  pinMode(TONEPIN, OUTPUT);
  digitalWrite(TONEPIN, LOW);

  Serial.print("RX V"); Serial.println(VERSION);

  //Sensor inputs
  for(uint8_t a = 0 ; a < NUMSENSORS ; a++)
  {
    pinMode(FIRSTSENSORPIN + a, INPUT);
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
    uint32_t time = micros(); //save time each for-loop one time
    
    if(state[sensor] == lastState[sensor])  //Nothing has changed -> do nothing ;)
    { }
    else  //state has changed
    {
      if(!state[sensor])  //signal has falling edge, puls starts
      {
        fallTime[sensor] = time;  
      }
      else  //rising edge, puls ends
      {
        getTime[sensor] = time - fallTime[sensor];  //Calculate pulse-time

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
              Serial.print(sensor, DEC); Serial.println(" NEB"); 
            }
            buf[sensor][0] = getTime[sensor];   //Start new message
            counter[sensor] = 1;
          }
          lastValue[sensor] = time;  //save actual time for next input
          
          if(counter[sensor] == NUMBITS)  //All bits read -> ready for check
          {
            counter[sensor] = 0;
            dataReady[sensor] = true;    
            dataReadyTime[sensor] = time;
          }
          //else : message isn't completed yet - waiting for next value
        }
        else  //measured time can't be possible -> Cancle this message and try new one.
        {
          Serial.print(sensor,DEC); Serial.print(" IC "); Serial.println(getTime[sensor]);
      
          counter[sensor] = 0; //reset counter
        }       
      }
    }
    lastState[sensor] = state[sensor];  //everything has been checked - set state for next round
  }

  //calculate message if reading is completet
  for(int8_t sensor = (NUMSENSORS-1) ; sensor >= 0 ; sensor--)
  {
    if(dataReady[sensor] && micros() >= (dataReadyTime[sensor] + ANALYSEDELAY))  //New message is ready
    {
      dataReady[sensor] = false;

      //vvvv Testing only.. Stupid without knowing pilot number ;)
      uint32_t pass = millis();
      if(pass >= lastPass + NEWPASS)  //Just print an empty line
        Serial.println("NP");
      lastPass = pass;
      //^^^^ Testing only
        
      for(int8_t a = (NUMBITS-1); a >= 0 ; a--)  //Calculate bit message
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
          
          Serial.print(sensor, DEC); Serial.print(" ND "); Serial.println(data[sensor]);
          //vvvv Testing only
          if(data[sensor] == 1 && !beepActive)  //Short Beep for ID 1
          {
            beep(BEEPLENGTHONE);
          }
          else if(data[sensor] == 2 &&!beepActive) //Long Beep for ID 2
          {
            beep(BEEPLENGTHTWO);
          }
          //^^^^ Testing only
        }
        else
        {
          Serial.print(sensor, DEC); Serial.println(" CI");   
        }   
      }
      else
      {
        Serial.print(sensor, DEC); Serial.println(" SBI");
      }
    }
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

//void led0(uint32_t d)
//{
//  digitalWrite(LED0PIN, HIGH); //enable beep
//  led0Active = true;
//  led0End = millis() + d;
//}
//
//void led1(uint32_t d)
//{
//  digitalWrite(LED1PIN, HIGH); //enable beep
//  led1Active = true;
//  led1End = millis() + d;
//}

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

