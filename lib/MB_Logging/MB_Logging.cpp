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
