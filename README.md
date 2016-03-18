#NOTE:
##OpenLap will get a V2! There are not much information to show yet, but when we have something to show, we will do! So please stay patient and check FB group for any news! 
###One more thing : OpenLap V1 Transponders will get a workaround, but this workaround isnt easy to make, so we wanna reduce this to a minimum.. Before you build some transponders yourselfe, wait for the new ones ;) It wont take so long..  


![Screenshot](docs/pictures/Readme/OpenLap Logo.png)

OpenLap is an open source IR laptimer for miniquad racing.

##Software

OpenLap is a cheap and easy to use system. It works with the famous [FPV Race Tracker Software by Andreas Neubauer](https://www.facebook.com/fpvracetracker/).


##Tags

![Screenshot](docs/pictures/Readme/Transponder.png)

OpenLap has developed own transponders. They are only 14*19mm and below 1g!
60 IDs are supported this time. On the transponders is a button and led indicators for easy change of the ID.


##Receiver

![Screenshot](docs/pictures/Readme/NodeMCU.png)

Instead of other timing hardware OpenLap is really simple!
Only one 7$ board and four 1$ Sensors are necessary to make it work.
It can be used wireless (with a small extra tool running on your pc) or wired over USB.
No setup has to be done. Once uploaded the software the hardware detects automatic the software used.


##How to start

1. Grab a transponder (build one yourself or get one from an official reseller (currently working on this..)). If you build one yourself here's [how to upload the software](docs/Transponder%20Update.md). 
2. Plug it in a free 5V source. Make sure you can draw enough current. Short pulses are over 100mA. Maybe an extra 47-100µF Elko on 5V rails is recommended if you notice some trouble. To set up your transponder ID follow [this guide](docs/Transponder.md).
3. To set up the receiver follow [this guide](docs/Receiver.md).

Now you're ready to go :)

_ToDo :_  
_- Wireless Usage_  
_- What you need_


##What's next

I'm working on a big update, but it's currently a secret :P
Just kidding. I have to check if what I'm planning is actually possible. If it is I will post some info here soon ;)

##Weblinks

Main discussion about OpenLap occurs in the following Facebook group :  
https://www.facebook.com/groups/1047398441948165/  
Most of the discussions are in German, but I will try to make an english version of my posts in the future. 

There's also an official website, but because of limited time the info there is out of date...  
www.openlap.de

##Thanks for your interest in OpenLap
Always feel free to contact me, I will try to answer asap.
Any "professional" help is welcome!
