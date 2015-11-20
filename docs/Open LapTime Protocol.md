#Open LapTime Protocol
V0.1


##Signal

```
     |------------- T_signal ------------|

     |-t-|-t-|                           |-----T-----|
      ___     ___     ___     ___     ___             ___
     |   |   |   |   |   |   |   |   |   |           |
     |   |   |   |   |   |   |   |   |   |           |
_____|   |___|   |___|   |___|   |___|   |___________|

Bit :  1   2   3   4   5   6   7   8   9
```


1 : Startbit 1 "0"  
2 : Startbit 2 "0"  
3 : Databit 1 (High Bit)  
4 : Databit 2  
5 : Databit 3  
6 : Databit 4  
7 : Databit 5  
8 : Databit 6 (Low Bit)  
9 : Checksum  


##Startbits

Both startbits are always "0".


##Databits

ID as binary number. (For example : ID 12 -> 001100, or ID 62 -> 111110)


##Checksum

Checksum is "0" if the number of ones in binary ID is even oder "1" if number of ones is odd. (Even-parity)


##Times

t : Time for pulse/pause (depends on bit is 0 or 1)  

    "0" : 250µS  //This times maybe will be changed
    "1" : 650µS

T : Time between signals. 20ms fixed + 0-10ms random (colision avoidance)

T_signal : Total length of a single signal. (depends on ID, 2,65 to 4,65mS)


##Examples

ID 01 : 0 0  0 0 0 0 0 1  1  (T_signal = 3,05mS)  
ID 03 : 0 0  0 0 0 0 1 1  0  (T_signal = 3,05mS)  
ID 63 : 0 0  1 1 1 1 1 1  0  (T_signal = 4,65mS)  


##Supported Projects

Open LapTime Protocol is supported by [OpenLap](https://github.com/YannikW/Open-Lap) and [EasyRaceLapTimer](https://github.com/polyvision/EasyRaceLapTimer)
	
