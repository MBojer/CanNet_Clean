#include <Arduino.h>


// -------------------------------------------- SD Card --------------------------------------------
#include <SD.h>


// -------------------------------------------- Files --------------------------------------------
String File_Content;

#define Setting_File_Path "/CanNet/Settings.txt"



// -------------------------------------------- Queue's --------------------------------------------
#include <MB_Queue.h>
#include <MB_Queue_Delay.h>

#define Max_Queue_Size 15

MB_Queue Send_Queue(Max_Queue_Size);
MB_Queue Receive_Queue(Max_Queue_Size);

MB_Queue_Delay Delay_Queue;



// --------------------- REMOVE ME ---------------------------
#include <MemoryFree.h>
unsigned long freeMemory_Last;
unsigned long freeMemory_Delay_Until;
#define freeMemory_Delay_For 250
// --------------------- REMOVE ME - End ---------------------





// -------------------------------------------- Setup --------------------------------------------
void setup() {
  // -------------------------------------------- Serial --------------------------------------------
  Serial.begin(115200);
  Serial.println("Booting");



  // -------------------------------------------- SD Card --------------------------------------------
  if (!SD.begin(53)) {
    Error_Mode(1, "Initializing SD card");
  }



  Serial.println("Boot done");
} // setup

void loop() {
    // put your main code here, to run repeatedly:
}
