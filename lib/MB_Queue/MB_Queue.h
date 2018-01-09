/*
MB Queue
Version 0.1
*/

#ifndef MB_Queue_h
  #define MB_Queue_h

  #include "Arduino.h"

  class MB_Queue {

    public:
      // --------------------------------------------- Setup ---------------------------------------------
      MB_Queue(int Max_Queue_Length);


      // --------------------------------------------- Queue ---------------------------------------------
      void Push(String Push_String, bool Add_To_Front_Of_Queue);
      void Push(String Push_String);
      String Pop();
      String Peek();
      String Peek_Queue();
      int Length();
      void Clear();
      String Search_Peek(String Search_String);
      String Search_Pop(String Search_String, bool Delete_All_Matches);

      bool Queue_Is_Empthy = true;
      

    private:
      // --------------------------------------------- Queue ---------------------------------------------
      int _Max_Queue_Length;
      int _Queue_Length = 0;
      String _Queue_String = ";"; // ";" is used as seperators betweens the string in the queue

  };

#endif
