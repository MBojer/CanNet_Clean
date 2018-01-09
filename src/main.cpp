#include <Arduino.h>




// -------------------------------------------- SD Card --------------------------------------------
#include <SD.h>


// -------------------------------------------- Files --------------------------------------------
#define Setting_File_Path "/CanNet/Settings.txt"



// -------------------------------------------- Logging --------------------------------------------
#include <MB_Logging.h>

MB_Logging Log(true, true)


// // -------------------------------------------- Queue's --------------------------------------------
// #include <MB_Queue.h>
// #include <MB_Queue_Delay.h>
//
// #define Max_Queue_Size 15
//
// MB_Queue Send_Queue(Max_Queue_Size);
// MB_Queue Receive_Queue(Max_Queue_Size);
//
// MB_Queue_Delay Delay_Queue;



// --------------------- REMOVE ME ---------------------------
#include <MemoryFree.h>
unsigned long freeMemory_Last;
unsigned long freeMemory_Delay_Until;
#define freeMemory_Delay_For 250
// --------------------- REMOVE ME - End ---------------------


// -------------------------------------------- Error Mode --------------------------------------------
#define Fetal 1
#define Error 2

void Error_Mode(byte Severity, String Trigger, String Text) {

  if (Severity == 1) { // Critical - Systel will halt
    /* code */
  }


}


// -------------------------------------------- Setup --------------------------------------------
void setup() {
  // -------------------------------------------- Serial --------------------------------------------
  Serial.begin(115200);
  Log.Add("Booting");


  // -------------------------------------------- SD Card --------------------------------------------
  if (!SD.begin(4)) {
    Log.Add("Initializing SD card"); // change me
    Error_Mode(1, "SD", "Initializing SD card");
  }










  Log.Add("Boot done");
} // setup

void loop() {
    // put your main code here, to run repeatedly:
}


/*
MB Queue
Version 0.1
*/


  // -------------------------------------------- Log Queue --------------------------------------------
  #include <MB_Queue.h>
  MB_Queue Log_Queue(10);


  // -------------------------------------------- Files --------------------------------------------
  #define Log_File_Dir "/Log/"
  File Log_File;


      MB_Logging(bool _Log_To_Serial, bool _Log_To_SD);

      bool Log_To_Serial;
      byte Serial_Log_Level = 4; // Log messages to and including this level

      bool Log_To_SD;
      byte SD_Log_Level = 4; // Log messages to and including this level
      byte SD_Cache_Lines = 10; // Number of lines to cache before wring to SD card


      // --------------------------------------------- Log ---------------------------------------------
      void Add(byte Log_Level, String Log_Line_Text);

      void Add(String _Log_Line_Text); // Referance only

      // Log Levels
      #define Fatal 1
      #define Error 2
      #define Warning 3
      #define Info 4
      #define Debug 5

      byte Last_Log_Level;



      // --------------------------------------------- Show ---------------------------------------------
      String Show(byte Line_Count);


      // --------------------------------------------- Write Log To SD ---------------------------------------------
      void Write_Log_To_SD();


      // --------------------------------------------- Log File Rollover ---------------------------------------------
      void Log_File_Rollover();


      // --------------------------------------------- WBus ---------------------------------------------

      #include "Arduino.h"
      #include "MB_Logging.h"

      // --------------------------------------------- Setup ---------------------------------------------
      MB_Logging::MB_Logging(bool _Log_To_Serial, bool _Log_To_SD) {
        Log_To_Serial = _Log_To_Serial;
        Log_To_SD = _Log_To_SD;


      } // MB_Logging


      // --------------------------------------------- Log ---------------------------------------------
      // Fatal 1 - Error 2 - Warning 3 - Info 4 - Debug 5
      void MB_Logging::Add(byte Log_Level, String Log_Line_Text) {

        Last_Log_Level = Log_Level;

        if (Log_To_Serial == true)
        {
          Serial.print("-- ");
          if (Log_Level == 1 && Log_Level <= Serial_Log_Level) Serial.print("Fatal");
          else if (Log_Level == 2 && Log_Level <= Serial_Log_Level) Serial.print("Error");
          else if (Log_Level == 3 && Log_Level <= Serial_Log_Level) Serial.print("Warning");
          else if (Log_Level == 4 && Log_Level <= Serial_Log_Level) Serial.print("Info");
          else if (Log_Level == 5 && Log_Level <= Serial_Log_Level) Serial.print("Debug");
          Serial.print(" -- ");

          Serial.println(Log_Line_Text);
        }
        if (Log_To_SD == true)
        {

          if (Log_Queue.Length() == SD_Cache_Lines) Write_Log_To_SD();


          Log_Queue.Push(Log_Line_Text);

        }


      } // Log

      void MB_Logging::Add(String _Log_Line_Text) { // Referance only
        Add(4, _Log_Line_Text);
      } // Log


      // --------------------------------------------- Show ---------------------------------------------
      String MB_Logging::Show(byte Line_Count) {

        return ""; // change me
      } // Show


      // --------------------------------------------- Write Log To SD ---------------------------------------------
      void MB_Logging::Write_Log_To_SD() {

        if (Log_Queue.Length() == 0) return; // Nothing to do, might as well fuck off :-P

        for (byte i = 0; i < Log_Queue.Length(); i++) {

          Log_File = SD.open("/Log/test.txt", FILE_READ);





        }



      } // Write_Log_To_SD


      // --------------------------------------------- Log File Rollover ---------------------------------------------
      void Log_File_Rollover() {

        // if ()

      } // Log_File_Rollover
