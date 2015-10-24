#Changelog

#RX Master

V0.1:
  - completly new software for master/slave system
  - I2C communication
  - laptime measurement
  
#RX Slave

V0.2:
  - completly new software for master/slave system
  - I2C communication
  - ID must be read two times to confirm (delete some false readings)

V0.3 (Single)
  - improved filtering to avoid false readings
  - setp back to single sensor on one arduino (not sure if two sensors will be securly possible in future)

#RX

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
  - 0.5.1 shorten seriel commands and set baud to 250000.. Serial is so slow :(
  - 0.5.2 add a 50µS pause between data capture completed and data analyse. Helps to get raw values of second interrupt correct
  
V0.5.2:
  - Shorten serial commands and set baud to 250000 to speed up code
  - add a 50µS pause between data capturing and analysing. Otherwise analysing blocks measurement of second sensor and produce failure in input capture 
