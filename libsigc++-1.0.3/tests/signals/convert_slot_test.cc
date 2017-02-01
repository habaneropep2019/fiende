#include <string>
#include <iostream>
#include <sigc++/signal_system.h>
#include <sigc++/convert.h>

/*
Copyright 1999, Karl Nelson

This program shows how to alter the argument list of a slot
to match a signal using convert().

*/

#ifdef SIGC_CXX_NAMESPACES
using namespace std;
using namespace SigC;
#endif

int fail=1;

int my_string_to_char(Callback1<int,const char*> *d,const string &s)
  {
   return d->call(s.c_str());
  }

int foobar(const char* f)
  {
    if (strcmp(f,"hello")==0) fail=0;
    cout << f<<endl;
    return 1;
  }

int main(int argc,char **argv)
  {
   Slot1<int,const string &>  s=convert(slot(foobar),my_string_to_char);
   s("hello");
   return fail;
  }


