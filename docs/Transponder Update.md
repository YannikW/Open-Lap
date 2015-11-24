#Transponder Update

In this tutorial I will show you how to update your transponder if necessary.

1. Hardware needed
2. Install ATtiny support for Arduino
3. Configure upload options
4. Connect transponder and upload

Note : All pictures shows the german version of Arduino, but I think you will understand what to do ;)

##1. Hardware needed

[ToDo]

##2. Install ATtiny support for Arduino

Note : You only have to do this once. If you already have ATtiny support installed you can go to step 3.

#####Open preferences  

![Screenshot](docs/pictures/Transponder Update/01 Voreinstellungen Auswahl.png)

#####Add following URL to "Additional Boards Manager URLs" (use a comma to separate it from any URLs you've already added)  
````
https://raw.githubusercontent.com/damellis/attiny/ide-1.6.x-boards-manager/package_damellis_attiny_index.json
````
![Screenshot](docs/pictures/Transponder Update/02 Voreinstellungen.png)

#####Open Board Manager

![Screenshot](docs/pictures/Transponder Update/03 Board Manager Auswahl.png)

#####Search for `attiny by David A. Mellis` and click on `Install`

![Screenshot](docs/pictures/Transponder Update/04 Board Manager.png)

Now you have successfully installed ATtiny support for Arduino.

##3. Configure upload options

#####Select following board options  
````
Board : ATtiny
Processor : ATtiny44
Clock : 16MHz (external)

Programmer : USBasp (if you use the one shown above)
````

![Screenshot](docs/pictures/Transponder Update/05 Auswahl.png)

##4. Connect transponder and upload

#####Place programmer on the pcb

[ToDo picture]

#####Check if you have opened the newest TX sketch version and press upload

![Screenshot](docs/pictures/Transponder Update/06 Upload.png)

#####If you done all right command line should look like this (Upload completed)

![Screenshot](docs/pictures/Transponder Update/07 Upload abgeschlossen.png)
