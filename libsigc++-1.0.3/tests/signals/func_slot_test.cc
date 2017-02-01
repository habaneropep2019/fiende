// -*- c++ -*-
#include <sigc++/signal_system.h>
#include <iostream>

// Function Slot Test
//   Written by Karl Nelson and assigned to the Public Domain.
// This code should compile with no errors of warnings
#ifdef SIGC_CXX_NAMESPACES
using namespace std;
using namespace SigC;
#endif

void foo1()    {cout<<"foo1"<<endl;}
int  foo2()    {cout<<"foo2"<<endl;return 1;}
void foo3(int) {cout<<"foo3"<<endl;}
int  foo4(int) {cout<<"foo4"<<endl;return 1;}

int main(int argc,char **argv)
  {
   cout << ">> Sizes"<<endl;
   cout << "sizeof(Slot0<int>) = "<<sizeof(Slot0<int>)<<endl;
   cout << "sizeof(SlotData) = "<<sizeof(SlotData)<<endl;
   cout << ">> Connecting Slots"<<endl;
   Slot0<void>      slot1=slot(foo1);
   Slot0<int>       slot2=slot(foo2);
   Slot1<void,int>  slot3=slot(foo3);
   Slot1<int,int>   slot4=slot(foo4);
   Slot0<void>      slot5=slot(&foo1);
   Slot0<int>       slot6=slot(&foo2);
   Slot1<void,int>  slot7=slot(&foo3);
   Slot1<int,int>   slot8=slot(&foo4);
   cout << ">> Calling Slots"<<endl;
   slot1.call();
   slot2.call();
   slot3.call(1);
   slot4.call(2);
   slot5.call();
   slot6.call();
   slot7.call(3);
   slot8.call(4);

   // compiling is passing
   return 0;
  }
