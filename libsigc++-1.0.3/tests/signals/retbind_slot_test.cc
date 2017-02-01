// -*- c++ -*-
#include <sigc++/signal_system.h>
#include <sigc++/retbind.h>
#include <iostream>

#include <string>

/* 
Copyright 1999, Karl Nelson

This program shows how to alter the return type of a slot 
to match a signal using retbind().  

*/

#ifdef SIGC_CXX_NAMESPACES
using namespace std;
using namespace SigC;
#endif

int  foo1(int i)
{
    return i;
}

int main(int argc,char **argv)
  {
   cout << ">> Sizes "<<endl;
   cout << "sizeof(Slot1<void,int>)                    = "
        <<  sizeof(Slot1<void,int>)<<endl;
   cout << "sizeof(Slot1_<void,int>)                   = "
        <<  sizeof(SlotData)<<endl;

   cout << ">> Connecting Slots"<<endl;
   Slot1<string,int>    slot1=retbind<string>(slot(foo1), string("Hello"));
   Slot1<float,int>   slot2=retbind<float>(slot(foo1), 9.5);
   cout << ">> Calling Slots"<<endl;
   cout << "Slot1: " << slot1.call(1) << endl;
   cout << "Slot2: " << slot2.call(1) << endl;

   return 0;  // compiling is passing
  }


