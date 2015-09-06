# IR-Laptimer
IR Laptimer for Miniquad Racing

This projekts aim is to build a IR Laptimer for Miniquad Racing.

Every Quad will get a small transmitter with an unique ID.
If the quad pass the start/finish gate the ID will be caught an the time will be displayed.

In this moment it's still in an very early alpha phase.
First tests were promising. Even with only one rx sensor I caught nearly all gate passes.

Next steps are:
I will check if one Arduino can handle more then one Sensor (it's tricky because interrupts will be triggered at the same time). 
Also small transmitters are in production. They will be 14*17mm (wo led and cable) and hopefully smaller then 3mm to fit them between the center plates.
Also I will build a groundsation with 4 sensors to ensure every pass, even if the copter is tilted a bit, will get caught.
If the groundstation works my plan is a bluetooth connection to an android phone/tablet to visualize the data.
