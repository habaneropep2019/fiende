#include <iostream>
#include <sigc++/thread.h>

#ifndef SIGC_PTHREADS
int main()
  {
   cout << "SigC not compiled with pthreads."<<endl;
  } 
#else

#ifdef SIGC_CXX_NAMESPACES
using namespace std;
using namespace SigC::Threads;
#endif

char buffer;
Private<int> items;
Semaphore done(-1);

struct Consumer :public Thread
  {  
   Semaphore turn;

   virtual void* main(void*);
   Consumer():Thread(),turn(0)
     {}
  };


struct Producer :public Thread
  { 
   Semaphore turn;
   virtual void* main(void*);

   Producer():Thread(),turn(1)
     {}
  };

Consumer consumer;
Producer producer;
int main()
  {
   consumer.start();
   producer.start();
   done.down();
   cout <<"Done!"<<endl;
   return 0;
  }

void* Consumer::main(void*)
  {
   for (items=0;items<10;items++)
     {
      turn.down();
      cout <<items<< " Consume "<<buffer <<endl;
      producer.turn.up();
     }
   done.up();
   return 0;
  }

void* Producer::main(void*)
  {
   char c='a';
   for (items=0;items<10;items++)
     {
      turn.down();
      buffer = (c++);
      cout <<items<<" Produce "<<buffer<<endl; 
      consumer.turn.up();
     }
   done.up();
   return 0;
  }

#endif  



