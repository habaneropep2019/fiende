#include <sigc++/signal_system.h>
#include <iostream>

#ifdef SIGC_CXX_NAMESPACES
using namespace std;
using namespace SigC;
#endif

int count_=0;

int foo1(int i) 
  {
   count_++;
   cout<<"f("<<i<<");"<<endl;
   return 1;
  }

Connection c2;
int foo2(int i) 
  {
   count_++;
   cout<<"f2("<<i<<");"<<endl;
   c2.disconnect();  // this should not crash the system.
   return 1;
  }

int main(int argc,char **argv)
  {
   int fail_=0;
   Signal1<int,int> sig1;  // int sig(int);

   Connection c;

   c=sig1.connect(slot(foo1));
   c2=sig1.connect(slot(foo2)); // this is a self remover

   cout << ">> Emit signal "<< endl;
   sig1.emit(1);
   if (count_!=2) fail_=1;

   c.disconnect();
   cout << ">> Emit signal again "<< endl;
   sig1.emit(1);
   if (count_!=2) fail_=1;
   
   return fail_;
  }
