// Copyright 1999 Karl Nelson.
// 
// Okay here is a complete primer on basic use of signals.

#include <iostream>
// (1) Include the signal system in headers file.
#include <sigc++/signal_system.h>


// (2) If your compiler supports name spaces, you should use namespace SigC
//  It is not necessary to include the entire space.  However, if you 
//  include all you should do so in each source file.   
#ifdef SIGC_CXX_NAMESPACES
using namespace std;
using namespace SigC;
#endif

// Some procedures to connect to. 
int foo1(int i)   {cout<<"f("<<i<<");"<<endl;        return 1;}
int foo2(int i)   {cout<<"sig2::f("<<i<<");"<<endl;  return 1;}
void foo1v(int i) {cout<<"fv("<<i<<");"<<endl;}
void foo2v(int i) {cout<<"sig2::fv("<<i<<");"<<endl;}

// (3) Objects which are to be connected must be derived from SigC::Object.
struct A:public Object
  {
   int foo(int i)   {cout<<"A::f("<<i<<");"<<endl;   return 1;}
   void foov(int i) {cout<<"A::fv("<<i<<");"<<endl;}
   A() {}
  };


main()
  {
   A a;

   // (4) Signals can be declared anywhere, including as class members
   // Their size is about that of 2 pointers.
   // Signals contain their callback signature as template parameters.
   // The number following it is the number of parameters, and the
   // first argument is the return type.
   //  
   // So to declare a signal called like int foo(int) would be 
   //    Signal1< int, int> foo;
  
   // Lets declare a few signals.
   Signal1<int,int> sig1;    // int sig1(int);
   Signal1<int,int> sig2;    // int sig2(int);

   // The return type is allowed to be void.
   Signal1<void,int> sig1v;  // void sig(int);
   Signal1<void,int> sig2v;  // void sig2(int);

   // (5) After the signals are declared you can establish
   // connections between them and functions and methods.
   cout << ">> Connect to signals "<< endl;

   // Connect to function foo.
   sig1.connect(slot(foo1));

   // Connect to method foo of object a.
   sig1.connect(slot(a,&A::foo));

   // Connect to signal 1 to signal 2.  Thus all things in signal2
   // are also called.
   sig1.connect(sig2.slot());

   // We can do the same for the void signals.
   sig1v.connect(slot(foo1v));
   sig1v.connect(slot(a,&A::foov));
   sig1v.connect(sig2v.slot());

   sig2.connect(slot(foo2));
   sig2v.connect(slot(foo2v));

   // (6) After connection the signals can be "emitted".
   // This calls all the callbacks stored within the signal.
   // (Emits are generally called reverse of the order connected,
   // however this is implementation specific.)
   cout << ">> Emit signals "<< endl;

   // Explicitly calling the emits.
   sig1.emit(1);
   sig1v.emit(2);

   // Or alternatively they can be called as a function object
   // with operator()
   sig1(1);
   sig1v(2);
  }
