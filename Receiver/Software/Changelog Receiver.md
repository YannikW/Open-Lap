#Changelog

##Receiver

##NodeMCU V1.0 ESP8266 based

#####V0.6.1 (17.12.15)
  - Bugfix. V0.6 don't compile an Arduino IDE >=1.6.6 .
  - V0.6.1 now works on V1.6.7 of Arduini IDE

#####V0.6 (16.12.15)
  - FPV Race Tracker wireless support 
  - EasyRaceLapTimer wireless support (reset doesn't work yet)
  - Complete auto selection. Every software wired or wireless works just plug and play

#####V0.5 (09.12.15)
  - FPV Race Tracker support (should be plug and play ;) )

#####V0.3 (20.11.15)
  - compatibility with Open LapTime Protocol
  - merge arduino master/slave based software into one code
  - add "NL" (New Lap) command for output

##Arduino based
Note : Arduino based RX is not longer supported

####RX Master

#####V0.1:
  - completly new software for master/slave system
  - I2C communication
  - laptime measurement
  
####RX Slave

#####V0.5 (Single)
  - change filtering to bigger buffer, so three transponders in RX range are possible
  
#####V0.4 (Single)
  - remove Interrupts
  - polling sensor input states
  - define for number of sensors (just one number to change for two or more sensors)
  
#####V0.3 (Single)
  - improved filtering to avoid false readings
  - setp back to single sensor on one arduino (not sure if two sensors will be securly possible in future)

#####V0.2:
  - completly new software for master/slave system
  - I2C communication
  - ID must be read two times to confirm (delete some false readings)


  

  
