/*
This Code is published by Yannik Wertz under the "Creativ Common Attribution-NonCommercial-ShareAlike 4.0 International (CC BY-NC-SA 4.0)" License 
For more Information see : http://creativecommons.org/licenses/by-nc-sa/4.0/


#########################################################
#########################################################

OpenLap
Open Source IR Laptimer

In this file all possible Serial commands will be saved.
*/

void initiateCommands()
{
  SCmd.addCommand("VERSION", sayVersion);       //Print Software Version
  SCmd.addCommand("DEBUGMODE", setDebugmode);   //Set no debugging, simple debugging, or full debugging
  //SCmd.addCommand("MODE", setMode);             //Switch between OpenLap operating modes
  char rtm[] = {1, 37};                         //FPV Race Tracker API, start call (1, 37, 13, 10) [(13, 10) is \cr\lf]
  SCmd.addCommand(rtm, fpvRaceTracker);         //Start FPV Race Tracker operation mode
  SCmd.addCommand("NEW_OPEN_LAP", resetRace);   //Reset all laps, times, ... 
  SCmd.addCommand("RESET#", easyRace);          //Start EasyRaceLapTimer operation mode
  SCmd.addCommand("HELP", help);                //Show possible commands
  SCmd.addDefaultHandler(unrecognized);         //Unknown Command input
}

//Just print actual software Version
void sayVersion()
{
  Serial.println(VERSION);
  SCmd.clearBuffer();
}

//Set Debugmode ( 0: no debugging ; 1: simple debugging ; 2: full debugging)
void setDebugmode()
{
  uint8_t argInt;
  char *arg;
  arg = SCmd.next();    //Get argument 0, 1 or 2 as Charstring
  if(arg != NULL)
  {
    argInt = atoi(arg); //Convert argument to an integer
    if(argInt == 0)     //No debugging (Normal mode)
    {
      simpleDebug = false;
      fullDebug = false;
      Serial.println("Debugging disabled");
    }
    else if(argInt == 1) //Simple Debugging
    {
      simpleDebug = true;
      Serial.println("Simple Debugging activated");
    }
    else if(argInt == 2) //Full Debugging
    {
      fullDebug = true;
      Serial.println("Full Debugging activated");
    }
    else
      Serial.print("Wrong argument");
  }
  else
    Serial.println("No argument");

  SCmd.clearBuffer();
}

//FPV Race Tracker Mode and Start
void fpvRaceTracker()
{
  mode = 2; //select FPV Race Tracker output mode
  uint32_t startTime = millis();
  for(uint8_t a = 0; a < 64; a++) //save start time for every pilot
  {
    pass[a] = startTime;
  }  
  char out[10] = {1, 37, 9, '1', 9, 48, 9, 48, 13, 10};
  for(uint8_t outCount = 0; outCount < 10; outCount++)  
    Serial.print(out[outCount]);
}

//Reset variables for a new Flying-Start-Race
void resetRace()
{
  mode = 0; //OpenLap Flying-Start-Race
  for(uint8_t a = 0; a < 64; a++)
  {
    firstPass[a] = true;
    lap[a] = 0; 
  }
  Serial.println("Starting new OpenLap Flying-Start-Race");
  SCmd.clearBuffer();
}

//EasyRaceLapTimer Mode Flying Start reset
void easyRace()
{
  mode = 3; //EasyRaceLapTimer output mode
  for(uint8_t a = 0; a < 64; a++)
  {
    firstPass[a] = true;
  }
  SCmd.clearBuffer();
}

//Show all possible commands
void help()
{
  Serial.println();
  Serial.println("Following commands supported :");
  Serial.println();
  Serial.println("\"VERSION\" : \t\t Show current Softwareversion");
  Serial.println("\"DEBUGMODE [MODE]\" : \t Sets Debugmode");
    Serial.println("\t   [MODE] \t 0 : No Debugging (default)");
    Serial.println("\t\t\t 1 : Simple Debugging");
    Serial.println("\t\t\t 2 : Full Debugging");
  Serial.println("\"RESET\" : \t\t Start new OpenLap Flying-Start-Race");
  Serial.println("\"HELP\" : \t\t Show this list");
  Serial.println();
  SCmd.clearBuffer();
}

//Default handler for unknown commands
void unrecognized()
{
  Serial.println("Unknown Command");
  Serial.println("Typ in \"HELP\" to show commands"); 
  SCmd.clearBuffer();
}

