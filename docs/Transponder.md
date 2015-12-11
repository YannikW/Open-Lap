#Transponder

1. Overview
2. LED Blink Sequence
3. Change ID
4. ID / Band / Channel

##1. Overview

Features :
  - 60 IDs
  - 4 band with 15 channels each
  - channel selection with button
  - LEDs indicate ID
  - only 14x19mm
  - 5-8m transmitting range
  - programming pads for easy update
  
##2. LED Blink Sequence

The LEDs indicate actual band and ID

When you plug in the transponder 1-4 LEDs will blink for two seconds to show band.
One LED flahing : Band 1
Two LEDs flashing : Band 2
...

After the flashing the LEDs will go solid. They indicate the actual channel as binary number.
If you don't know binary numbers, thats how it works :

The right led is the "1", the second from right is the "2", the third is the "4" and the last is the "8".

If the LED is on add the number.

````
Example :
LEDs (from left to right) : off, on, on, off.

off is 0, and on is 1, so :

channel : 0*8 + 1*4 + 1*2 + 0*1 = 6 
```` 

Here you see an example sequence at startup :

![Screenshot](pictures/Transponder/blink seqence.gif)

3 LEDs flashing -> Band 3
Only the "1" stay on -> Channel 1
So this is ID 33.

##3. Change ID

To change the band press the button long. Blink sequence will show new band.

To change the channel press the button short.

##4. ID / Band / Channel

| ID | Band | Channel | LEDs              |
|:--:|:----:|:-------:|:-----------------:|
|  1 |   1  |    1    | off, off, off, on |
|  2 |   1  |    2    | off, off, on, off |
|  3 |   1  |    3    | off, off, on, on  |
|  4 |   1  |    4    | off, on, off, off |
|  5 |   1  |    5    | off, on, off, on  |
|  6 |   1  |    2    | off, on, on, off  |
|  7 |   1  |    2    | off, on, on, on   |
|  8 |   1  |    2    | on, off, off, off |
|  9 |   1  |    2    | on, off, off, on  |
| 10 |   1  |   10    | on, off, on, off  |
| 11 |   1  |   11    | on, off, on, on   |
| 12 |   1  |   12    | on, on, off, off  |
| 13 |   1  |   13    | on, on, off, on   |
| 14 |   1  |   14    | on, on, on, off   |
| 15 |   1  |   15    | on, on, on, on    |
| 16 |   -  |    -    | _not usable_      |

| ID | Band | Channel | LEDs              |
|:--:|:----:|:-------:|:-----------------:|
| 17 |   2  |    1    | off, off, off, on |
| 18 |   2  |    2    | off, off, on, off |
| 19 |   2  |    3    | off, off, on, on  |
| 20 |   2  |    4    | off, on, off, off |
| 21 |   2  |    5    | off, on, off, on  |
| 22 |   2  |    2    | off, on, on, off  |
| 23 |   2  |    2    | off, on, on, on   |
| 24 |   2  |    2    | on, off, off, off |
| 25 |   2  |    2    | on, off, off, on  |
| 26 |   2  |   10    | on, off, on, off  |
| 27 |   2  |   11    | on, off, on, on   |
| 28 |   2  |   12    | on, on, off, off  |
| 29 |   2  |   13    | on, on, off, on   |
| 30 |   2  |   14    | on, on, on, off   |
| 31 |   2  |   15    | on, on, on, on    |
| 32 |   -  |    -    | _not usable_      |

| ID | Band | Channel | LEDs              |
|:--:|:----:|:-------:|:-----------------:|
| 33 |   3  |    1    | off, off, off, on |
| 34 |   3  |    2    | off, off, on, off |
| 35 |   3  |    3    | off, off, on, on  |
| 36 |   3  |    4    | off, on, off, off |
| 37 |   3  |    5    | off, on, off, on  |
| 38 |   3  |    2    | off, on, on, off  |
| 39 |   3  |    2    | off, on, on, on   |
| 40 |   3  |    2    | on, off, off, off |
| 41 |   3  |    2    | on, off, off, on  |
| 42 |   3  |   10    | on, off, on, off  |
| 43 |   3  |   11    | on, off, on, on   |
| 44 |   3  |   12    | on, on, off, off  |
| 45 |   3  |   13    | on, on, off, on   |
| 46 |   3  |   14    | on, on, on, off   |
| 47 |   3  |   15    | on, on, on, on    |
| 48 |   -  |    -    | _not usable_      |

| ID | Band | Channel | LEDs              |
|:--:|:----:|:-------:|:-----------------:|
| 49 |   4  |    1    | off, off, off, on |
| 50 |   4  |    2    | off, off, on, off |
| 51 |   4  |    3    | off, off, on, on  |
| 52 |   4  |    4    | off, on, off, off |
| 53 |   4  |    5    | off, on, off, on  |
| 54 |   4  |    2    | off, on, on, off  |
| 55 |   4  |    2    | off, on, on, on   |
| 56 |   4  |    2    | on, off, off, off |
| 57 |   4  |    2    | on, off, off, on  |
| 58 |   4  |   10    | on, off, on, off  |
| 59 |   4  |   11    | on, off, on, on   |
| 60 |   4  |   12    | on, on, off, off  |
| 61 |   4  |   13    | on, on, off, on   |
| 62 |   4  |   14    | on, on, on, off   |
| 63 |   4  |   15    | on, on, on, on    |
