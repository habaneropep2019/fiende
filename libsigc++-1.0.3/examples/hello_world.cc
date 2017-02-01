// Copyright 1999 Karl Nelson.

#include <iostream>
#include <string>
#include <sigc++/signal_system.h>

#ifdef SIGC_CXX_NAMESPACES
using namespace SigC;
#endif

void print(const string &str) 
  {cout << str;}

main()
  {
   Signal1<void,const string &> printer;

   printer.connect(slot(print));

   printer("hello world\n");
  }
