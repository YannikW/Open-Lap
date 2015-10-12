# IR-Laptimer
IR Laptimer for Miniquad Racing

This projects aim is to build a IR Laptimer for Miniquad Racing.

Every Quad will get a small transmitter with an unique ID.
If the quad pass the start/finish gate the ID will be caught an the time will be displayed.

In this moment it's still in an very early alpha phase.
First tests were promising. Even with only one rx sensor I caught nearly all gate passes.
Tests with two sensors gave - ahh - mixed results. First it worked fine. But with new receivers I reduced the pulse time to an absolut minimum and it seems this is to fast for an arduino to handle multiple sensors. It's a bit weired because sometimes it works and sometimes not. 
But Receiver size doen't matter and arduinos are less then 2 Euros, so there is nearly no argument why not to use three or four arduinos for the sensors. 

These Arduinos will all be connected to an master via I2C. The slave arduinos wil only catch the transponder passes. Only the master receiver will calculate times. This is not 100% accurate, because time will not be stopped when transponder passes, it will stopped when the slaves told the master it has passed. I think this error will be okay ;P

First batch of pcbs is now ready for testing. Soldering is a bit hard on some coponents, but it's possible ;)

Next steps are:
I will build a groundstation with 4 sensors to ensure every pass, even if the copter is tilted a bit, will get caught.
Software is nearly complete, but china post is really slow with my arduinos -.-
If the groundstation works my plan is a bluetooth connection to an android phone/tablet to visualize the data.
