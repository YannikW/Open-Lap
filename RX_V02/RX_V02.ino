/*#######################################################
#########################################################

IR Miniquad Laptimer

IR Receiver V0.2 (06.09.15)

Signal:
Start Bits | Data (e.g) | Checksum (0 if data is even or 1 if data is odd)
   0   0   | 0  1  1  0 | 0  
   
#########################################################
#######################################################*/

/*#######################################################
##                       Defines                       ##
#######################################################*/

#define NUMBITS          7  //Starts Bits, data and checksum
#define DATABITS         4  //4 Databits -> 0-15

#define CRITTIME       470  //µS border for decision 0 or 1
#define SIGNALSPACE   3500  //µS

#define COMMAND          4  //Beep if 4 is read, double beep if any other number is read

#define TONEPIN          7  //Buzzer on pin D7

/*#######################################################
##                      Variables                      ##
#######################################################*/

uint32_t fallTime;
uint32_t getTime; 
uint32_t lastValue;
uint8_t counter;
uint16_t buf[8];
uint8_t message[7];
boolean dataReady = false;
boolean checksum;
uint8_t data;


boolean getValueFalling_Flag = false;
boolean getValueRising_Flag = false;

uint32_t beepEnd = 0; //time for beep to end, see beep()
boolean beepActive = false;

/*#######################################################
##                        Setup                        ##
#######################################################*/

void setup() 
{

  Serial.begin(115200);
  
  attachInterrupt(1, getValueFalling, FALLING);
  pinMode(7, OUTPUT);
  digitalWrite(7, LOW);
  
  randomSeed(analogRead(0));
}

/*#######################################################
##                        Loop                         ##
#######################################################*/

void loop() 
{ 
  //Flags
  if (getValueFalling_Flag)
  {
    getValueFalling_Flag = false; //reset flag
    
    fallTime = micros();  
    attachInterrupt(1, getValueRising, RISING);
  }
  if (getValueRising_Flag)
  {
    getValueRising_Flag = false; //reset flag
    
    uint32_t time = micros();
  
    getTime = time - fallTime;  //Calculate puls-time
    
    //Serial.print(getTime); Serial.print(" ");
    if(time - lastValue <= SIGNALSPACE)  //Still the same message
    {
      buf[counter] = getTime;  //save time in input-buffer
      counter++;
    }
    else  //New Message
    {
      if(counter != 0)    //Error occurred. New message but last message wasn't completed
        Serial.println("We haven't got enought Bits"); 
      buf[0] = getTime;   //Start new message
      counter = 1;
    }
    lastValue = time;  //save actual time for next input
    
    if(counter == NUMBITS)  //All bits read -> ready for check
    {
      detachInterrupt(1);  //While data is not analysed, we don't want new data
      counter = 0;
      dataReady = true;    
    }
    else  //message isn't completed yet - waiting for next value
      attachInterrupt(1, getValueFalling, FALLING);
  }

  
  
  if(dataReady)  //New message is ready
  {
    dataReady = false;
    Serial.print("  ||  ");
    for(int8_t a = 0; a < NUMBITS ; a++)  //Calculate bit message
    { 
      Serial.print(buf[a]); Serial.print(" ");
      message[a] = getValue(buf[a]);
      Serial.print(message[a]); Serial.print(" ");
    }
  
    if(message[0] == 0 && message[1] == 0)  //Start Bits
    {      
      checksum = false;
      for(uint8_t b = 0; b < DATABITS; b++)  //Calculating Checksum
      {
          if(message[2+b])    //if it's a one, toggle checksum
            checksum = !checksum;
      }
      
      if(checksum == message[2 + DATABITS])  //is caculated checksum the same as read checksum?
      {
        data = ((message[2] << 3) + (message[3] << 2) + (message[4] << 1) + (message[5]));  //calculate command
        
        Serial.print("We got new Data! The Value is : "); Serial.println(data);
        if(data == 4)  //Beep one time
        {
          beep(200);
        }
        else  //This shoun't be happend. Valid data, but it's wrong -> Double beep
        {
          digitalWrite(TONEPIN, HIGH);
          delay(100);
          digitalWrite(TONEPIN,LOW);
          delay(100);
          digitalWrite(TONEPIN, HIGH);
          delay(100);
          digitalWrite(TONEPIN,LOW);
          delay(100);
        }
        //delay(random(1, 5));  //Wait minimum one second for new signal
      }
      else
        Serial.println("Checksum incorrect");      
    }
    else
      Serial.println("Start Bits incorrect");
      
    attachInterrupt(1, getValueFalling, FALLING);  //Reactiavte Interrupt
  }

  
  //BEEP
  if (beepActive && beepEnd<=millis()){ //end time reached and beep active
    digitalWrite(TONEPIN,LOW); //disable beep
    beepActive=false;
  }

}

/*#######################################################
##                      Functions                      ##
#######################################################*/

void beep(uint32_t d)
{
  digitalWrite(TONEPIN, HIGH); //enable beep
  beepActive=true;
  beepEnd=millis()+d;
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

void getValueFalling()    //Puls begins
{
  getValueFalling_Flag=true;
}

void getValueRising()    //Puls ended
{ 
  getValueRising_Flag=true;  
}
