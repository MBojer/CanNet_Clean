#include <Arduino.h>

#include <SPI.h>



// -------------------------------------------- SD Card --------------------------------------------
#include "SdFat.h"
SdFat sd;


// -------------------------------------------- Files --------------------------------------------
#define Root_Dir "/CanNet/"
#define Log_File_Dir "Log/"

File Log_File;
String Current_Log_File = "1970-1-1.log";
byte Current_Log_File_Day = 1;

#define Setting_File_Name "Settings.txt"


// -------------------------------------------- Time --------------------------------------------
#include <TimeLib.h>


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
MB_Queue CanNet_Receive_Queue(25);

// #include <MB_Queue_Delay.h>
//
// #define Max_Queue_Size 15
//
// MB_Queue Send_Queue(Max_Queue_Size);
// MB_Queue Receive_Queue(Max_Queue_Size);
//
// MB_Queue_Delay Delay_Queue;


// -------------------------------------------- Serial Interfaces --------------------------------------------
#include <Auto_Serial_Speed.h>

Auto_Serial_Speed Serial_Speed_Test;

byte Serial_Interface[] = {0, 0, 0, 0}; // Serial 0 is not used


// -------------------------------------------- CAN  --------------------------------------------
#include <mcp_can.h>

MCP_CAN CAN(10);


// -------------------------------------------- CanNet  --------------------------------------------
byte CanNet_ID;
bool CanNet_ID_Check_Done = false;


// --------------------- REMOVE ME ---------------------------
#include <MemoryFree.h> // rm
// --------------------- REMOVE ME - End ---------------------


// --------------------------------------------- Time String ---------------------------------------------
String Time_String() {
  return
          String(day()) + "-" + String(month()) + "-" + String(year()) +
          " " +
          String(hour()) + ":" + String(minute()) + ":" + String(second());
}


// --------------------------------------------- Error Level String ---------------------------------------------
String Error_Level_String(byte &Log_Level) {
  switch (Log_Level) {
    case 1:
      return "Fatal";
    case 2:
      return "Fatal";
    case 3:
      return "Warning";
    case 4:
      return "Info";
    case 5:
      return "Debug";
  }
  return "";
} // Error_Level_String



// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// +++++++++++++++++++++++++++++++++++++++++++++ Loggging +++++++++++++++++++++++++++++++++++++++++++++
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// --------------------------------------------- Loggging ---------------------------------------------
// Fatal 1 - Error 2 - Warning 3 - Info 4 - Debug 5
void Log_Serial(byte Se_Log_Level, String Se_Log_Line_Text) {

  Serial.print(String(freeMemory()) + " "); // rm

  Serial.print(Time_String() + " - ");
  if (Se_Log_Level == 1) Serial.print("Fatal");
  else if (Se_Log_Level == 2) Serial.print("Error");
  else if (Se_Log_Level == 3) Serial.print("Warning");
  else if (Se_Log_Level == 4) Serial.print("Info");
  else if (Se_Log_Level == 5) Serial.print("Debug");
  Serial.print(": ");

  Serial.println(Se_Log_Line_Text);
}

void Disable_Log_To_SD(String Serial_String) {
  if (Log_To_SD == true) {
    Log_Serial(Error, Serial_String);
    Log_To_SD = false; // disables logging to SD since there is no place to put the logs
    Log_Serial(Error, "Logging to SD card disabled");
  }
}

void Log_SD_Queue(byte SD_LSQ_Log_Level, String SD_LSQ_Log_Line_Text, bool Echo_To_Serial) {

  if (Echo_To_Serial == true) Log_Serial(SD_LSQ_Log_Level, SD_LSQ_Log_Line_Text);

  String Log_String = String(Time_String()) + ".+-" + SD_LSQ_Log_Level + ".+-" + SD_LSQ_Log_Line_Text;

  Log_Queue.Push(Log_String);
}

void Log_SD_Queue(byte SD_LSQ_Log_Level, String SD_LSQ_Log_Line_Text) {
  Log_SD_Queue(SD_LSQ_Log_Level, SD_LSQ_Log_Line_Text, false);
}


void Write_Log_To_SD() {

  if (Log_To_SD == 0) return;

  else if (Log_Queue.Length() == 0) return; // Nothing to do, might as well fuck off :-P


  if (!Log_File) {
    Serial.println("Log file not open");

  }

  // if (!Log_File.open(Log_File_Path.c_str(), O_APPEND)) {
  //   while (true) delay(1000); // rm
  //   return;
  // }


  String Write_String;

  while (Log_Queue.Length() != 0) { // Writing the log to the file
    Write_String = Log_Queue.Pop();
    Write_String.replace(".+-", ";"); // change me - the queue function needs to use something else then ";" for spacers
    Log_File.println(Write_String);
  }

  Log_File.flush();
  // Log_File.close();

} // Write_Log_To_SD


void Log_File_Rename(String New_File_Name) {

  if (sd.exists(New_File_Name.c_str())) {
    Current_Log_File = String(year()) + "-" + String(month()) + "-" + String(day() + ".log");
    Log_Serial(Info, "FILE EXISTS: " + Current_Log_File + " to: " + New_File_Name);
    return;
  }

  // if (!Log_File.rename(sd.vwd(), New_File_Name.c_str())) {
  if (sd.rename(Current_Log_File.c_str(), New_File_Name.c_str())) {
    Disable_Log_To_SD("Unable to rename log file from: " + Current_Log_File + " to: " + New_File_Name);
  }

  else {
    Current_Log_File = String(year()) + "-" + String(month()) + "-" + String(day()) + ".log";
    Log_Serial(Info, "Log file renamed from: " + Current_Log_File + " to: " + New_File_Name);
  }

} // Log_File_Rename

void Log_SD(byte &SD_Log_Level, String &SD_Log_Line_Text) {

  if (Log_Queue.Length() >= SD_Cache_Lines) {
    Log_Serial(Debug, "SD Log Queue Full");
    Write_Log_To_SD();
    if (Log_Queue.Length() != 0) Log_Serial(Debug, "SD Log Queue Cleared");
  }

  Log_SD_Queue(SD_Log_Level, SD_Log_Line_Text, false);

}


void Log(byte Log_Level, String Log_Line_Text) {

  if (Log_To_Serial == true && Log_Level <= Serial_Log_Level) Log_Serial(Log_Level, Log_Line_Text);

  if (Log_To_SD == true && Log_Level <= SD_Log_Level) Log_SD(Log_Level, Log_Line_Text);

} // Log

void Log(String _Log_Line_Text) { // Referance only
  Log(Info, _Log_Line_Text);
} // Log

String Show(byte Line_Count) {

  return ""; // change me
} // Show



// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// +++++++++++++++++++++++++++++++++++++++++++++ Error Mode +++++++++++++++++++++++++++++++++++++++++++++
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// -------------------------------------------- Error Mode --------------------------------------------
void Error_Mode(byte Severity, String Text) {

  Log(Severity, Text); // change me

  if (Severity == 1) { // Fatal - Systel will halt
    Write_Log_To_SD();
    Log(1, "System Halted");
    while (true) { // halt the system -- Add power down
      delay(1000);
    }
  }

} // Error_Mode


// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// +++++++++++++++++++++++++++++++++++++++++++++ File Handling +++++++++++++++++++++++++++++++++++++++++++++
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


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

bool Setting_Import(bool &Variable, String Search_Text, String &File_Content) {
  if (File_Content.indexOf("\r\n" + Search_Text + " = ") != -1)
  {
    Variable = Find_Setting_Bool(File_Content, Search_Text);
    Log(Debug, "Setting imported: " + Search_Text + " = " + String(Variable));
    return true;
  }
  return false;
}

bool Setting_Import(int &Variable, String Search_Text, String &File_Content) {
  if (File_Content.indexOf("\r\n" + Search_Text + " = ") != -1)
  {
    Variable = Find_Setting_Int(File_Content, Search_Text);
    Log(Debug, "Setting imported: " + Search_Text + " = " + String(Variable));
    return true;
  }
  return false;
}

bool Setting_Import(byte &Variable, String Search_Text, String &File_Content) {
  if (File_Content.indexOf("\r\n" + Search_Text + " = ") != -1)
  {
    Variable = Find_Setting_Int(File_Content, Search_Text);
    Log(Debug, "Setting imported: " + Search_Text + " = " + String(Variable));
    return true;
  }
  return false;
}

bool Setting_Import(String &Variable, String Search_Text, String &File_Content) {
  if (File_Content.indexOf("\r\n" + Search_Text + " = ") != -1)
  {
    Variable = Find_Setting(File_Content, Search_Text);
    Log(Debug, "Setting imported: " + Search_Text + " = " + String(Variable));
    return true;
  }
  return false;
}


// -------------------------------------------- Read Conf File --------------------------------------------
String Read_Settings_File() {

    if (!sd.exists(String(String(Root_Dir) + String(Setting_File_Name)).c_str())) {
      Error_Mode(1, "Settings File missing: " + String(Root_Dir) + String(Setting_File_Name));
      return "";
    }

    File Temp_File = sd.open(String(String(Root_Dir) + String(Setting_File_Name)).c_str(), FILE_READ);

    String Temp_Content = "";
    // Reads the file and puts it into a string
    while (Temp_File.available()) {
      char Letter = Temp_File.read();
      Temp_Content += Letter;
    }

    Temp_File.close(); // Close the file

    Temp_Content = Temp_Content.substring(0, Temp_Content.indexOf("Comments:"));

    while (Temp_Content.indexOf("\r\n\r\n") != -1) {
      Temp_Content.replace("\r\n\r\n", "\r\n");
    }

    return String(Temp_Content);
}


// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// ++++++++++++++++++++++++++++++++++++++++++++ CAN +++++++++++++++++++++++++++++++++++++++++++++++++++
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// -------------------------------------------- CanNet Send --------------------------------------------
void CanNet_Send(INT32U Target, byte Byte0, byte Byte1, byte Byte2, byte Byte3, byte Byte4, byte Byte5, byte Byte6, byte Byte7) {

  unsigned char Buffer_Send[8] {Byte0, Byte1, Byte2, Byte3, Byte4, Byte5, Byte6, Byte7};

  CAN.sendMsgBuf(Target, 0, 8, Buffer_Send);

}

void CanNet_Send(INT32U Target, byte Byte0) { // Referance only
  CanNet_Send(Target, Byte0, 0, 0, 0, 0, 0, 0, 0);
} // void CanNet_Send(int Target, byte Byte0)
void CanNet_Send(INT32U Target, byte Byte0, byte Byte1) { // Referance only
  CanNet_Send(Target, Byte0, Byte1, 0, 0, 0, 0, 0, 0);
} // void CanNet_Send(int Target, byte Byte0, byte Byte1)
void CanNet_Send(INT32U Target, byte Byte0, byte Byte1, byte Byte2) { // Referance only
  CanNet_Send(Target, Byte0, Byte1, Byte2, 0, 0, 0, 0, 0);
} // void CanNet_Send(int Target, byte Byte0, byte Byte1, byte Byte2)
void CanNet_Send(INT32U Target, byte Byte0, byte Byte1, byte Byte2, byte Byte3) { // Referance only
  CanNet_Send(Target, Byte0, Byte1, Byte2, Byte3, 0, 0, 0, 0);
} // void CanNet_Send(int Target, byte Byte0, byte Byte1, byte Byte2, byte Byte3)
void CanNet_Send(INT32U Target, byte Byte0, byte Byte1, byte Byte2, byte Byte3, byte Byte4) { // Referance only
  CanNet_Send(Target, Byte0, Byte1, Byte2, Byte3, Byte4, 0, 0, 0);
} // void CanNet_Send(int Target, byte Byte0, byte Byte1, byte Byte2, byte Byte3, byte Byte4)
void CanNet_Send(INT32U Target, byte Byte0, byte Byte1, byte Byte2, byte Byte3, byte Byte4, byte Byte5) { // Referance only
  CanNet_Send(Target, Byte0, Byte1, Byte2, Byte3, Byte4, Byte5, 0, 0);
} // void CanNet_Send(int Target, byte Byte0, byte Byte1, byte Byte2, byte Byte3, byte Byte4, byte Byte5)
void CanNet_Send(INT32U Target, byte Byte0, byte Byte1, byte Byte2, byte Byte3, byte Byte4, byte Byte5, byte Byte6) { // Referance only
  CanNet_Send(Target, Byte0, Byte1, Byte2, Byte3, Byte4, Byte5, Byte6, 0);
} // void CanNet_Send(int Target, byte Byte0, byte Byte1, byte Byte2, byte Byte3, byte Byte4, byte Byte5, byte Byte6)


void CanNet_ID_Check(int ID_To_Check) {

  Serial.println("MARKER123");

  Serial.println("MARKER123123123");

  if (CanNet_ID_Check_Done == false) { // Error during boot check
    if (CanNet_ID == ID_To_Check) {
      Error_Mode(Fatal, "Duplicate CanNet ID found on network during boot, halting");
    }
  }

  if (CanNet_ID == ID_To_Check) {
    Log(Warning, "Duplicate CanNet ID found on network");
    CanNet_Send(0x00B, CanNet_ID);
  }

}

// -------------------------------------------- CAN Time --------------------------------------------
void CanNet_Time_Send() {

  CanNet_Send(0x015, 5, year(), month(), weekday(), day(), hour(), minute(), second());
  Log(Info, "Send time: Stratum: 5 - Time: " + String(hour()) + ":" + String(minute()) + ":" + String(second()) +
            " Date:" + String(day()) + "-" + String(month()) + "-" + String(year()) +
            " Weekday: " + String(weekday())
            ); // change me

}



// -------------------------------------------- CAN Time --------------------------------------------
void CanNet_Time_Receive(unsigned char &Stratum, unsigned char &Year, unsigned char &Month, unsigned char &Day, unsigned char &Hour, unsigned char &Minute, unsigned char &Secound) {

  if (Year + 2000 >= year()) {
    if (Month >= month()) {
      if (Day >= day()) {
        if (Hour >= hour()) {
          if (Minute >= minute()) {
            if (Secound >= second()) {

              String Old_Time_String = Time_String();

              if (timeStatus() == 0) { // timeNotSet
                // Adjusts the time
                setTime(Hour,Minute,Secound,Day,Month,Year);
                Log(Info, "Time adjusted from: " + Old_Time_String + " to: " + Time_String());
                Log_File_Rename(String(year()) + "-" + String(month()) + "-" + String(day()) + ".log");
              }


                // else if (timeStatus() == 1) { // timeNeedsSync
                //   Serial.println("timeStatus() == 1");
                // }
                //
                // else if (timeStatus() == 2) { // timeSet - Change log file name to YYYY-MM-DD ".log"
                //   Serial.println("timeStatus == 2");
                //
            } // Secound
          } // Minute
        } // Hour
      } // Day
    } // Month
  } // Year

} // CanNet_Time_Receive


// -------------------------------------------- CanNet Receive --------------------------------------------
void CanNet_Receive() {

  if(CAN_MSGAVAIL == CAN.checkReceive()) { // check if data coming

    unsigned char len = 0;
    unsigned char Buffer_Receive[8];
    INT32U CAN_ID = CAN.getCanId();

    CAN.readMsgBuf(&len, Buffer_Receive);    // read data,  len: data length, RX_Buffer: data RX_Buffer

    String Temp_String = String(CAN_ID) + ",";

    for (byte i = 0; i < 8; i++) {
      Temp_String = Temp_String + String(Buffer_Receive[i]) + ",";
    }

    // ++++++++++++++++++++ Messages not getting queued ++++++++++++++++++++
    if (CAN_ID == 0x00A) { // CanNet ID Check Broadcast
      CanNet_ID_Check(Buffer_Receive[0]); // 0x00A (10) ID Crech Broadcast 0x00B (11) = IC Check failed reply
      return;
    }

    if (CAN_ID == 0x00B) { // Duplicate CanNet ID Check found Broadcast
      CanNet_ID_Check(Buffer_Receive[0]); // 0x00A (10) ID Crech Broadcast 0x00B (11) = IC Check failed reply
      return;
    }

    if (CAN_ID == 0x014) { // Time Request
      CanNet_Time_Send();
      return;
    }

    if (CAN_ID == 0x015) { // Time Request Reply
      CanNet_Time_Receive(Buffer_Receive[0], // Stratum
                          Buffer_Receive[2], // Year
                          Buffer_Receive[3], // Month
                          Buffer_Receive[4], // Day
                          Buffer_Receive[5], // Hour
                          Buffer_Receive[6], // Minute
                          Buffer_Receive[7]  // Secound
                        );
      return;
    }

    if (CAN_ID > 700) // Messages with id 0 to 699 is either system or high priority
    {
      CanNet_Receive_Queue.Push(Temp_String, true);
    }
    else
    {
      CanNet_Receive_Queue.Push(Temp_String);
    }


    Log(Debug, String("CanNet RX: " + String(CAN_ID) + "-" + String(Temp_String)));

  } // if(CAN_MSGAVAIL == CAN.checkReceive())

} // CanNet_Receive


// -------------------------------------------- CAN Time --------------------------------------------
void CanNet_Time_Request() {

  Log(Debug, "Requesting time");
  CanNet_Send(0x014, 0);

} // CanNet_Time_Request


// -------------------------------------------- CAN ID Check Boot --------------------------------------------
void CanNet_ID_Boot_Check() {

  Log(Debug, "CanNet ID Boot Check");

  for (byte i = 0; i < 3; i++) {

    CanNet_Send(0x00A, CanNet_ID);

    for (byte i = 0; i < 255; i++) {

      delay(200);

      CanNet_Receive();

      if (i == 10) break;
    }


    // working here add can recive and check if a 0x00B message is in the queue
    // if not the can check passed and move on
    //
    // also add and auto send 0x00B if duplicte can message comes in on 0x00A

  }
  CanNet_ID_Check_Done = true;
  Log(Debug, "CanNet ID Boot Check Passed");

}


// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// ++++++++++++++++++++++++++++++++++++++++++++ Setup +++++++++++++++++++++++++++++++++++++++++++++++++
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void setup() {
  // -------------------------------------------- Serial --------------------------------------------
  Serial.begin(115200);
  Log("=======================================");
  Log("=============== Booting ===============");
  Log("=======================================");


  // -------------------------------------------- SD Card --------------------------------------------
  if (!sd.begin(4, SD_SCK_MHZ(50))) {
    Log_To_SD = false; // Disables logging to SD card
    sd.initErrorHalt();
    Error_Mode(Fatal, "Initializing SD card failed");
  }

  else Log(Debug, "Initializing SD card done");


  // -------------------------------------------- Settings file import --------------------------------------------

  String File_Content = Read_Settings_File();

  if (File_Content != "") {

    if (Setting_Import(CanNet_ID, "CanNet ID", File_Content) == false) { // CAN ID needed for the system to work
      Error_Mode(Fatal, "CAN ID not found in settings file not found");
    }

    Setting_Import(Log_To_Serial, "Log To Serial", File_Content);
    Setting_Import(Serial_Log_Level, "Serial Log Level", File_Content);

  	Setting_Import(Log_To_SD, "Log To SD", File_Content);
    Setting_Import(SD_Log_Level, "SD Log Level", File_Content);
    Setting_Import(SD_Cache_Lines, "SD Cache Lines", File_Content);


    String Temp_Str;
    for (byte i = 1; i < 4; i++) {
      if (Setting_Import(Temp_Str, "Serial " + String(i), File_Content) == true) {
        if (Temp_Str == "Ethernet") Serial_Interface[i] = 1;
        else if (Temp_Str == "Touch") Serial_Interface[i] = 2;
      }
    }

  } // if (File_Content != "")
  else  // Settings file if needed for the system to work
  {
    Log_To_SD = false;
    Error_Mode(Fatal, "Settings file not found");
  }


  // -------------------------------------------- Log Dir and File --------------------------------------------
  if (!sd.exists(String(String(Root_Dir) + String(Log_File_Dir)).c_str())) {

    if (!sd.mkdir(String(String(Root_Dir) + String(Log_File_Dir)).c_str())) {
      Disable_Log_To_SD("Unable to create log directory");
    }
    else {
      Log_Serial(Info, "Log directory created");
      Log_SD_Queue(Info, "Log directory created");
      sd.chdir("/");
    }
  }

  Log_File = sd.open(String(String(Root_Dir) + String(Log_File_Dir) + Current_Log_File).c_str(), O_APPEND);

  if (!Log_File) {
    Serial.println("CAnt create log file");
    while (true) delay(1000);
  }



  // -------------------------------------------- CAN --------------------------------------------
  if (CAN_OK == CAN.begin(CAN_500KBPS)) Log(Debug, "CAN BUS Shield initialization ok");
  else Error_Mode(Fatal, "CAN BUS Shield initialization failed");


  // -------------------------------------------- CanNet --------------------------------------------
  CanNet_ID_Boot_Check();


  // -------------------------------------------- Time Sync --------------------------------------------
  CanNet_Time_Request();


  // -------------------------------------------- Serial Interfaces --------------------------------------------
  for (byte i = 1; i < 4; i++) {
    if (Serial_Interface[i] != 0) {
      Log(Debug, "Serial Interface " + String(i) + " Speed Test Started");

      byte Temp_Byte;

      if (i == 1) Temp_Byte = Serial_Speed_Test.Test_Speed_Master(Serial1);
      else if (i == 2) Temp_Byte = Serial_Speed_Test.Test_Speed_Master(Serial2);
      else if (i == 3) Temp_Byte = Serial_Speed_Test.Test_Speed_Master(Serial3);

      if (Temp_Byte > 89) {
        Log(Error, "Serial Interface " + String(i) + " in config but connection failed with error: " + String(Serial_Speed_Test.Error_Text(Temp_Byte)) + ". Disabling this interface");
        Serial_Interface[i] = 0;
      }
      else {
        String Temp_St = "Serial Interface " + String(i) + ": ";
        if (Serial_Interface[1] == 1) Temp_St += "Ethernet: ";
        else if (Serial_Interface[1] == 1) Temp_St += "Touch: ";
        Log(Debug, Temp_St + "Speed Selected: " + String(Serial_Speed_Test.Speed_Step(Temp_Byte)));
      }


    }

  }


  Log("Boot done");

  // unsigned char Buffer_Receive[8];
  //
  // Buffer_Receive[0] = 3;
  // Buffer_Receive[2] = 18;
  // Buffer_Receive[3] = 1;
  // Buffer_Receive[4] = 14;
  // Buffer_Receive[5] = 22;
  // Buffer_Receive[6] = 24;
  // Buffer_Receive[7] = 17;
  //
  // CanNet_Time_Receive(Buffer_Receive[0],
  //                     Buffer_Receive[2],
  //                     Buffer_Receive[3],
  //                     Buffer_Receive[4],
  //                     Buffer_Receive[5],
  //                     Buffer_Receive[6],
  //                     Buffer_Receive[7]
  //                     );
  //

} // setup

void loop() {

  // -------------------------------------------- CanNet ID Check --------------------------------------------
  CanNet_Receive();
  delay(500);

  Serial.println("loop remove while trye below");
  while (true) delay(1000);

}
