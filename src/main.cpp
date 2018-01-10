#include <Arduino.h>




// -------------------------------------------- SD Card --------------------------------------------
#include <SD.h>


// -------------------------------------------- Files --------------------------------------------
#define Log_File_Dir "/Log/"
File Log_File;

#define Setting_File_Path "/CanNet/Settings.txt"


// -------------------------------------------- Time --------------------------------------------
#include <Time.h>


// --------------------------------------------- Logging ---------------------------------------------
// Log Levels
#define Fatal 1
#define Error 2
#define Warning 3
#define Info 4
#define Debug 5

bool Log_To_Serial = true;
byte Serial_Log_Level = 5; // Log messages to and including this level -- CHANGE ME TO 4


bool Log_To_SD = true;
byte SD_Log_Level = 5; // Log messages to and including this level -- CHANGE ME TO 4
byte SD_Cache_Lines = 10; // Number of lines to cache before wring to SD card


// -------------------------------------------- Queue's --------------------------------------------
#include <MB_Queue.h>

MB_Queue Log_Queue(25);

// #include <MB_Queue_Delay.h>
//
// #define Max_Queue_Size 15
//
// MB_Queue Send_Queue(Max_Queue_Size);
// MB_Queue Receive_Queue(Max_Queue_Size);
//
// MB_Queue_Delay Delay_Queue;


// -------------------------------------------- Touch Screen Serial Interface --------------------------------------------
#include <Auto_Serial_Speed.h>
bool Serial_Touch_Screen[3] = {false, false, false};


// --------------------- REMOVE ME ---------------------------
#include <MemoryFree.h>
unsigned long freeMemory_Last;
unsigned long freeMemory_Delay_Until;
#define freeMemory_Delay_For 250
// --------------------- REMOVE ME - End ---------------------


// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// +++++++++++++++++++++++++++++++++++++++++++++ Loggging +++++++++++++++++++++++++++++++++++++++++++++
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// --------------------------------------------- Loggging - Write Log To SD ---------------------------------------------
void Write_Log_To_SD() {

  if (Log_Queue.Length() == 0) return; // Nothing to do, might as well fuck off :-P

  for (byte i = 0; i < Log_Queue.Length(); i++) {

    // Log_File = SD.open("/Log/test.txt", FILE_READ);





  }



} // Write_Log_To_SD

// --------------------------------------------- Loggging - Log ---------------------------------------------
// Fatal 1 - Error 2 - Warning 3 - Info 4 - Debug 5
void Log(byte Log_Level, String Log_Line_Text) {

  if (Log_To_Serial == true && Log_Level <= Serial_Log_Level)
  {
    Serial.print("-- ");
    if (Log_Level == 1) Serial.print("Fatal");
    else if (Log_Level == 2) Serial.print("Error");
    else if (Log_Level == 3) Serial.print("Warning");
    else if (Log_Level == 4) Serial.print("Info");
    else if (Log_Level == 5) Serial.print("Debug");
    Serial.print(" -- ");

    Serial.println(Log_Line_Text);
  }
  if (Log_To_SD == true && Log_Level <= SD_Log_Level)
  {

    if (Log_Queue.Length() == SD_Cache_Lines) Write_Log_To_SD();


    Log_Queue.Push(Log_Line_Text);

  }


} // Log

void Log(String _Log_Line_Text) { // Referance only
  Log(Info, _Log_Line_Text);
} // Log

// --------------------------------------------- Loggging - Show ---------------------------------------------
String Show(byte Line_Count) {

  return ""; // change me
} // Show

// --------------------------------------------- Loggging - Log File Rollover ---------------------------------------------
void Log_File_Rollover() {

  // if ()

} // Log_File_Rollover


// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// +++++++++++++++++++++++++++++++++++++++++++++ Error Mode +++++++++++++++++++++++++++++++++++++++++++++
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// -------------------------------------------- Error Mode --------------------------------------------
void Error_Mode(byte Severity, String Text) {

  Log(Severity, Text); // change me

  if (Severity == 1) { // Critical - Systel will halt
    Log(1, "System Halted"); // change me
    while (true) {
      delay(1000);
    }
  }

  if (Severity == 2) { // Error
  }


} // Error_Mode


// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// +++++++++++++++++++++++++++++++++++++++++++++ File Handling +++++++++++++++++++++++++++++++++++++++++++++
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
String File_Content;

// -------------------------------------------- Find Settings --------------------------------------------
String Find_Setting(String &File_Content, String Setting_Name) {

  String Search_String = "\r\n" + Setting_Name + " = ";

  if (File_Content.indexOf(Search_String) == -1) {
    return "";
  }


  int Settings_Position = File_Content.indexOf(Search_String) + Search_String.length();

  return File_Content.substring(
                                          Settings_Position,
                                          File_Content.indexOf("\r\n", Settings_Position)
                                        );

} // Find_Setting

bool Find_Setting_Bool(String &File_Content, String Setting_Name) {

  Setting_Name = Find_Setting(File_Content, Setting_Name);

  return Setting_Name;

} // Find_Setting

int Find_Setting_Int(String &File_Content, String Setting_Name) {

  Setting_Name = Find_Setting(File_Content, Setting_Name);

  return Setting_Name.toInt();

} // Find_Setting

void Setting_Import(bool &Variable, String Search_Text) {
  if (File_Content.indexOf("\r\n" + Search_Text + " = ") != -1)
  {
    Variable = Find_Setting_Bool(File_Content, Search_Text);
    Log(Debug, "Setting imported: " + Search_Text + " = " + String(Variable));
  }
}

void Setting_Import(int &Variable, String Search_Text) {
  if (File_Content.indexOf("\r\n" + Search_Text + " = ") != -1)
  {
    Variable = Find_Setting_Int(File_Content, Search_Text);
    Log(Debug, "Setting imported: " + Search_Text + " = " + String(Variable));
  }
}

void Setting_Import(byte &Variable, String Search_Text) {
  if (File_Content.indexOf("\r\n" + Search_Text + " = ") != -1)
  {
    Variable = Find_Setting_Int(File_Content, Search_Text);
    Log(Debug, "Setting imported: " + Search_Text + " = " + String(Variable));
  }
}

// -------------------------------------------- Read Conf File --------------------------------------------
String Read_Conf_File(String File_Path, bool Error_Message) {

    File Temp_File;
    File_Content = "";

    // Checks if the file exists
    if (SD.exists(File_Path) == false) {
      if (Error_Message == true) Error_Mode(2, "File missing: " + File_Path);
      return "";
    }

    Temp_File = SD.open(File_Path); // Open the file

    // Reads the file and puts it into a string
    while (Temp_File.available()) {
      char Letter = Temp_File.read();
      File_Content += Letter;
    }

    Temp_File.close(); // Close the file

    File_Content = File_Content.substring(0, File_Content.indexOf("Comments:"));

    while (File_Content.indexOf("\r\n\r\n") != -1) {
      File_Content.replace("\r\n\r\n", "\r\n");
    }

    return File_Content;
}

String Read_Conf_File(String File_Path) { // Referance only
  return Read_Conf_File(File_Path, true);
} // Read_Conf_File





// -------------------------------------------- Setup --------------------------------------------
void setup() {
  // -------------------------------------------- Serial --------------------------------------------
  Serial.begin(115200);
  Log("Booting");

  // -------------------------------------------- SD Card --------------------------------------------
  if (!SD.begin(4)) {
    Error_Mode(1, "Initializing SD card failed");
  }

  // -------------------------------------------- Settings file import --------------------------------------------
  File_Content = Read_Conf_File(Setting_File_Path);

  if (File_Content != "") {

    Setting_Import(Log_To_Serial, "Log To Serial");
    Setting_Import(Serial_Log_Level, "Serial Log Level");

  	Setting_Import(Log_To_SD, "Log To SD");
    Setting_Import(SD_Log_Level, "SD Log Level");
    Setting_Import(SD_Cache_Lines, "SD Cache Lines");


    for (byte i = 0; i < 3; i++) {
      Setting_Import(Serial_Touch_Screen[i], "Touch Screen " + String(i + 1));
    }

    working here setup speed check

  }

  Log("Boot done");
} // setup

void loop() {
    // put your main code here, to run repeatedly:
}
