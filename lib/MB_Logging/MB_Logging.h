/*
MB Queue
Version 0.1
*/

#ifndef MB_Logging_h
  #define MB_Logging_h

  #include "Arduino.h"

  // -------------------------------------------- SD Card --------------------------------------------
  #include <SD.h>


  // -------------------------------------------- Log Queue --------------------------------------------
  #include <MB_Queue.h>
  MB_Queue Log_Queue(10);


  // -------------------------------------------- Files --------------------------------------------
  #define Log_File_Dir "/Log/"
  File Log_File;


  class MB_Logging {

    public:
      // --------------------------------------------- Setup ---------------------------------------------
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


    private:

  };

#endif
