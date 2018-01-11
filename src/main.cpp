#include <Arduino.h>


// -------------------------------------------- SD Card --------------------------------------------
#include <SD.h>


// -------------------------------------------- Files --------------------------------------------
#define Log_File_Dir "/CanNet/Log/"
File Log_File;

#define Setting_File_Path "/CanNet/Settings.txt"


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


// -------------------------------------------- Log File Rollover --------------------------------------------
String Current_Log_File;
bool Temp_Log_File_Active = false;


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


// -------------------------------------------- Touch Screen Serial Interface --------------------------------------------
#include <Auto_Serial_Speed.h>

Auto_Serial_Speed Serial_Speed_Test;

bool Serial_Touch_Screen[3] = {false, false, false};


// -------------------------------------------- CAN  --------------------------------------------
#include <mcp_can.h>
#include <SPI.h>

MCP_CAN CAN(10);


// -------------------------------------------- CanNet  --------------------------------------------
byte CanNet_ID;
bool CanNet_ID_Check_Done = false;


// --------------------- REMOVE ME ---------------------------
#include <MemoryFree.h>
unsigned long freeMemory_Last;
unsigned long freeMemory_Delay_Until;
#define freeMemory_Delay_For 250
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

// --------------------------------------------- Loggging - Log Serial ---------------------------------------------
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


void Log_SD_Queue(byte SD_LSQ_Log_Level, String SD_LSQ_Log_Line_Text) {

  String Log_String = String(Time_String()) + ".+-" + SD_LSQ_Log_Level + ".+-" + SD_LSQ_Log_Line_Text;

  Log_Queue.Push(Log_String);
}

// --------------------------------------------- Loggging - Log File Check ---------------------------------------------
void Log_Filename_Check() {

  if (SD.exists(Current_Log_File) == true) {

    String LFC_Search_String;

    if (Current_Log_File.indexOf("Temp") != -1)
    {
      LFC_Search_String = String(Log_File_Dir) + "Temp";
    }
    else
    {
      LFC_Search_String = String(Log_File_Dir) + String(year()) + "-" + String(month()) + "-" + String(day());
    }


    for (byte i = 1; i < 1337; i++) {

      if (SD.exists(LFC_Search_String + String(i) + ".log") == false)
      {
        Current_Log_File = LFC_Search_String + String(i) + ".log";
        return;
      }
      else if (i == 0)
      {
        Log_Serial(Warning, "Using logfile: " + LFC_Search_String + "0.log");
        Log_SD_Queue(Warning, "Using logfile: " + LFC_Search_String + "0.log");

        return;
      } // else if (i == 254)
    } // for (byte i = 0; i < 255; i++)
  }

  return;
} // Log_File_Check

// --------------------------------------------- Loggging - Log File Rollover ---------------------------------------------
void Log_File_Check() {

  if (year() == 1970 && Temp_Log_File_Active == false) {

    Current_Log_File = String(Log_File_Dir) + "Temp.log";

    Log_Filename_Check();

    Temp_Log_File_Active = true;
  } // if (year() == 1790)

  if (year() != 1970 && Temp_Log_File_Active == true) {

    String New_Log_File_Name = String(Log_File_Dir) + String(year()) + "-" + String(month()) + "-" + String(day()) + ".log";

    if (SD.exists(New_Log_File_Name) == true) {
      Log_Serial(Debug, "Log File appended from: " + Current_Log_File + " to: " + New_Log_File_Name);
      Log_SD_Queue(Debug, "Log File appended from: " + Current_Log_File + " to: " + New_Log_File_Name);

    }

    Log_Serial(Debug, "Log File renamed from: " + Current_Log_File + " to: " + New_Log_File_Name);
    Log_SD_Queue(Debug, "Log File renamed from: " + Current_Log_File + " to: " + New_Log_File_Name);
  }



  Serial.println("Work to do here"); // rm
  Serial.print("Current_Log_File: "); // rm
  Serial.println(Current_Log_File); // rm




// !!!!!!!!!!!!!!!!!!!!!!
// dont append to files make a new one
// add roleover if time is 23:59:59
// check if the clock starts counting from 1970 if it does not set it so it does + sec forward should be all then roleover for days should work when not time is set

} // Log_File_Check


// --------------------------------------------- Loggging - Write Log To SD ---------------------------------------------
void Write_Log_To_SD() {

  if (Log_Queue.Length() == 0) return; // Nothing to do, might as well fuck off :-P

  Log_File_Check();

  // Try and append
  Log_File = SD.open(Current_Log_File, O_RDWR | O_APPEND);
  if (!Log_File) {
      // It failed, so try and make a new file.
      Log_File = SD.open(Current_Log_File, O_RDWR | O_CREAT);
      if (!Log_File) {
        // It failed too, so give up.
        Log_Serial(2, "Unable to open log file:" + Current_Log_File);
      }
  }

  // Only write to the file if the file is actually open.
  if (Log_File) {

    Log_Serial(Debug, "Writing log file to SD card");

    byte Length_Queue = Log_Queue.Length();

      for (byte i = 0; i < Length_Queue; i++) {

        String Temp_Str = Log_Queue.Pop();
        Temp_Str.replace(".+-", ";");
        Temp_Str = Temp_Str + "\r\n";

        Log_File.print(Temp_Str);
      }

      Log_File.close();
  }





  Log_Serial(Debug, "Log file written to SD card");
} // Write_Log_To_SD

// --------------------------------------------- Loggging - Log SD ---------------------------------------------
// Fatal 1 - Error 2 - Warning 3 - Info 4 - Debug 5
void Log_SD(byte &SD_Log_Level, String &SD_Log_Line_Text) {

  if (Log_Queue.Length() == SD_Cache_Lines) {
    Log_Serial(Debug, "SD Log Queue Full");
    Write_Log_To_SD();
    Log_Queue.Clear();
    Log_Serial(Debug, "SD Log Queue Cleared");
  }


  Log_SD_Queue(SD_Log_Level, SD_Log_Line_Text);

}


void Log(byte Log_Level, String Log_Line_Text) {

  if (Log_To_Serial == true && Log_Level <= Serial_Log_Level) Log_Serial(Log_Level, Log_Line_Text);

  if (Log_To_SD == true && Log_Level <= SD_Log_Level) Log_SD(Log_Level, Log_Line_Text);

} // Log

void Log(String _Log_Line_Text) { // Referance only
  Log(Info, _Log_Line_Text);
} // Log


// --------------------------------------------- Loggging - Show ---------------------------------------------
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
    Log(1, "System Halted");
    Write_Log_To_SD();
    while (true) {
      delay(1000);
    }
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

bool Setting_Import(bool &Variable, String Search_Text) {
  if (File_Content.indexOf("\r\n" + Search_Text + " = ") != -1)
  {
    Variable = Find_Setting_Bool(File_Content, Search_Text);
    Log(Debug, "Setting imported: " + Search_Text + " = " + String(Variable));
    return true;
  }
  return false;
}

bool Setting_Import(int &Variable, String Search_Text) {
  if (File_Content.indexOf("\r\n" + Search_Text + " = ") != -1)
  {
    Variable = Find_Setting_Int(File_Content, Search_Text);
    Log(Debug, "Setting imported: " + Search_Text + " = " + String(Variable));
    return true;
  }
  return false;
}

bool Setting_Import(byte &Variable, String Search_Text) {
  if (File_Content.indexOf("\r\n" + Search_Text + " = ") != -1)
  {
    Variable = Find_Setting_Int(File_Content, Search_Text);
    Log(Debug, "Setting imported: " + Search_Text + " = " + String(Variable));
    return true;
  }
  return false;
}


// -------------------------------------------- Read Conf File --------------------------------------------
String Read_Conf_File(String File_Path, bool Error_Message) {

    File Temp_File;
    File_Content = "";

    // Checks if the file exists
    if (SD.exists(File_Path) == false) {
      if (Error_Message == true) Log(2, "File missing: " + File_Path);
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


// -------------------------------------------- CAN ID Check --------------------------------------------
void CanNet_ID_Check() {

  if (CanNet_ID_Check_Done == false) {
    Log(Debug, "CanNet ID Check");

    for (byte i = 0; i < 3; i++) {

      CanNet_Send(0x00A, CanNet_ID);

      for (byte i = 0; i < 255; i++) {

        delay(10);

        Serial.println("CanNet_Receive_Queue");
        Serial.println(CanNet_Receive_Queue.Peek_Queue());

        if (i == 200) break;
      }

      if (CanNet_Receive_Queue.Search_Peek("11," + String(CanNet_ID)) != ";") {
        Serial.println("MARKER Dumlicate ID");
      }







      // working here add can recive and check if a 0x00B message is in the queue
      // if not the can check passed and move on
      //
      // also add and auto send 0x00B if duplicte can message comes in on 0x00A

    }
  }



}


// -------------------------------------------- CanNet Receive --------------------------------------------
void CanNet_Receive(/* arguments */) {

  if(CAN_MSGAVAIL == CAN.checkReceive()) { // check if data coming

    unsigned char Buffer_Receive[8];
    INT32U CAN_ID = CAN.getCanId();

    // CAN.readMsgBuf(8, Buffer_Receive);    // read data,  len: data length, RX_Buffer: data RX_Buffer


    String Temp_String = String(CAN_ID) + ",";

    for (byte i = 0; i < 8; i++) {
      Temp_String = Temp_String + String(Buffer_Receive[i]) + ",";
    }

    if (CAN_ID > 700) // Messages with id 0 to 699 is either system or high priority
    {
      CanNet_Receive_Queue.Push(Temp_String, true);
    }
    else
    {
      CanNet_Receive_Queue.Push(Temp_String);
    }

    if (CAN_ID == 10 && CAN_ID == 11) CanNet_ID_Check(); // 0x00A (10) ID Crech Broadcast 0x00B (11) = IC Check failed reply

    Log(Debug, "CanNet RX: " + Temp_String);

  } // if(CAN_MSGAVAIL == CAN.checkReceive())

} // CanNet_Receive


// -------------------------------------------- CAN Time --------------------------------------------
void CanNet_Time_Send() {





}

void CanNet_Time_Receive() {
  /* code */
}

void CanNet_Time_Request() {






}


// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// ++++++++++++++++++++++++++++++++++++++++++++ Setup +++++++++++++++++++++++++++++++++++++++++++++++++
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void setup() {
  // -------------------------------------------- Serial --------------------------------------------
  Serial.begin(115200);
  Log("Booting");

  // -------------------------------------------- SD Card --------------------------------------------
  if (!SD.begin(4)) {
    Error_Mode(Fatal, "Initializing SD card failed");
  }

  // -------------------------------------------- Settings file import --------------------------------------------
  File_Content = Read_Conf_File(Setting_File_Path);

  if (File_Content != "") {

    if (Setting_Import(CanNet_ID, "CanNet ID") == false) { // CAN ID needed for the system to work
      Error_Mode(Fatal, "CAN ID not found in settings file not found");
    }

    Setting_Import(Log_To_Serial, "Log To Serial");
    Setting_Import(Serial_Log_Level, "Serial Log Level");

  	Setting_Import(Log_To_SD, "Log To SD");
    Setting_Import(SD_Log_Level, "SD Log Level");
    Setting_Import(SD_Cache_Lines, "SD Cache Lines");


    for (byte i = 0; i < 3; i++) {
      Setting_Import(Serial_Touch_Screen[i], "Touch Screen " + String(i + 1));
    }

  } // if (File_Content != "")

  else { // Settings file if needed for the system to work
    Error_Mode(Fatal, "Settings file not found");
  }


  // -------------------------------------------- CAN --------------------------------------------
  if (CAN_OK == CAN.begin(CAN_500KBPS)) Log(Debug, "CAN BUS Shield initialization ok");
  else Error_Mode(Fatal, "CAN BUS Shield initialization failed");


  // -------------------------------------------- Log File Dir --------------------------------------------
  SD.mkdir(Log_File_Dir); // If the dir is not there the logs will not be created


  // -------------------------------------------- Touch Screen Serial Interface --------------------------------------------
  if (Serial_Touch_Screen[0] == true)
  {
    Log(Debug, "Touch Screen Serial1 Speed Test Started");
    byte Temp_Byte = Serial_Speed_Test.Test_Speed_Master(Serial1);

    if (Temp_Byte > 89) Log(Error, "Touch Screen on Serial1 in config but connection failed with error: " + String(Serial_Speed_Test.Error_Text(Temp_Byte)));
    else Log(Debug, "Touch Screen Serial1 Speed Selected: " + String(Serial_Speed_Test.Speed_Step(Temp_Byte)));
  }
  if (Serial_Touch_Screen[1] == true)
  {
    Log(Debug, "Touch Screen Serial2 Speed Test Started");
    byte Temp_Byte = Serial_Speed_Test.Test_Speed_Master(Serial2);

    if (Temp_Byte > 89) Log(Error, "Touch Screen on Serial2 in config but connection failed with error: " + String(Serial_Speed_Test.Error_Text(Temp_Byte)));
    else Log(Debug, "Touch Screen Serial2 Speed Selected: " + String(Serial_Speed_Test.Speed_Step(Temp_Byte)));
  }
  if (Serial_Touch_Screen[2] == true)
  {
    Log(Debug, "Touch Screen Serial3 Speed Test Started");
    byte Temp_Byte = Serial_Speed_Test.Test_Speed_Master(Serial3);

    if (Temp_Byte > 89) Log(Error, "Touch Screen on Serial3 in config but connection failed with error: " + String(Serial_Speed_Test.Error_Text(Temp_Byte)));
    else Log(Debug, "Touch Screen Serial3 Speed Selected: " + String(Serial_Speed_Test.Speed_Step(Temp_Byte)));
  }







  CanNet_Time_Request();






  Log("Boot done");


} // setup

void loop() {

  // -------------------------------------------- CanNet ID Check --------------------------------------------
  CanNet_Receive();
  delay(500);


}
