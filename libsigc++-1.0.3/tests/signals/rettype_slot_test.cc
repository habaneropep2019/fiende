// -*- c++ -*-
#include <sigc++/signal_system.h>
#include <sigc++/rettype.h>
#include <iostream>

/* 
Copyright 1999, Karl Nelson

This program shows how to alter the return type of a slot 
to match a signal using rettype().  

*/

#ifdef SIGC_CXX_NAMESPACES
using namespace std;
using namespace SigC;
#endif

int  foo1(int i)       {cout<<"foo2 "<< i <<endl;return 1;}

int main(int argc,char **argv)
  {
   cout << ">> Sizes "<<endl;
   cout << "sizeof(Slot1<void,int>)                    = "
        <<  sizeof(Slot1<void,int>)<<endl;
   cout << "sizeof(Slot1_<void,int>)                   = "
        <<  sizeof(SlotData)<<endl;
   cout << "sizeof(AdaptorRettypeSlot0_<void,int>::Node) = "
        <<  sizeof(AdaptorRettypeSlot0_<void,int>::Node)<<endl;

   cout << ">> Connecting Slots"<<endl;
   Slot1<void,int>    slot1=rettype<void>(slot(foo1));
   Slot1<float,int>   slot2=rettype<float>(slot(foo1));
   cout << ">> Calling Slots"<<endl;
   slot1.call(1);
   slot2.call(1);

   return 0;  // compiling is passing
  }

