// -*- c++ -*-
#include <sigc++/signal_system.h>
#include <iostream>

// Object Slot Test
//   Written by Karl Nelson and assigned to the Public Domain.
// This code should compile with no errors of warnings
#ifdef SIGC_CXX_NAMESPACES
using namespace std;
using namespace SigC;
#endif

class MyObject: public Object
 {
  public:
    void foo1()    {cout<<"foo1"<<endl;}
    int  foo2()    {cout<<"foo2"<<endl;return 1;}
    void foo3(int) {cout<<"foo3"<<endl;}
    int  foo4(int) {cout<<"foo4"<<endl;return 1;}
 };

int main(int argc,char **argv)
  {
   Slot0<void>      slot1;
   Slot0<int>       slot2;
   Slot1<void,int>  slot3;
   Slot1<int,int>   slot4;
     {MyObject my;
      cout <<">> Connecting slots"<<endl;
      slot1=slot(my,&MyObject::foo1);
      slot2=slot(my,&MyObject::foo2);
      slot3=slot(my,&MyObject::foo3);
      slot4=slot(my,&MyObject::foo4);
      cout <<">> Calling slots"<<endl;
      slot1.call();
      slot2.call();
      slot3.call(1);
      slot4.call(2);
      cout <<">> Deleting Object"<<endl;
     }
   cout <<">> Calling Slots again (does nothing)"<<endl;
   slot1.call();
   slot2.call();
   slot3.call(1);
   slot4.call(2);
   cout <<">> Success"<<endl;

   return 0;  // compiling is passing
  }
