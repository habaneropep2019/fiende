// -*- c++ -*-
#include <sigc++/signal_system.h>
#include <sigc++/bind.h>
#include <iostream>

/*
Copyright 1999, Karl Nelson

This program shows how to bind an argument to a value using
bind().

*/

#ifdef SIGC_CXX_NAMESPACES
using namespace std;
using namespace SigC;
#endif

int k=0;

void foo1(int i)       {k+=i;cout<<"foo1 "<< i <<endl;}
int  foo2(int i)       {k+=i;cout<<"foo2 "<< i <<endl;return 1;}
void foo3(int i,int j) {k+=i+j;cout<<"foo3 "<< i <<" "<< j <<endl;}
int  foo4(int i,int j) {k+=i+j;cout<<"foo4 "<< i <<" "<< j <<endl;return 1;}

int main(int argc,char **argv)
  {
   cout << ">> Sizes "<<endl;
   cout << "sizeof(Slot1<void,int>)                    = "
        <<  sizeof(Slot1<void,int>)<<endl;
   cout << "sizeof(AdaptorBindSlot0_1<void,int>)       = "
        <<  sizeof(AdaptorBindSlot0_1<void,int>) << endl;
   cout << "sizeof(AdaptorBindSlot0_1<void,int>::Node) = "
        <<  sizeof(AdaptorBindSlot0_1<void,int>::Node)<<endl;

   cout << ">> Connecting Slots"<<endl;
   Slot0<void>      slot1=bind(slot(foo1),1);
   Slot0<int>       slot2=bind(slot(foo2),2);
   Slot1<void,int>  slot3=bind(slot(foo3),3);
   Slot1<int,int>   slot4=bind(slot(foo4),4);
   cout << ">> Calling Slots"<<endl;
   slot1.call();
   slot2.call();
   slot3.call(5);
   slot4.call(6);

   return (k!=21);
  }

