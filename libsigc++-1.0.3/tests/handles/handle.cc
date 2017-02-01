
#include <iostream>

#include <sigc++/object.h>
#include <sigc++/scope.h>
#include <sigc++/handle.h>
#include <sigc++/generator.h>

#ifdef SIGC_CXX_NAMESPACES
using namespace std;
using namespace SigC;
#endif

int global;

/********************************************************/
class A:public Object  
  {
   public: 
     A() 
       {cout<<"A()"<<endl;}
     ~A() 
       {global=1;
        cout<<"~A()"<<endl;
       }
  };

int main()
  {
   int i;
   int fail=0;

   global=0;
   cout << "Test contained"<<endl;
   cout << "begin"<<endl;
     {
      Handle<A,Scopes::Extend> o=gen<A>();
      i=1;
     }
   if (global!=1) fail=1;
   cout << "end"<<endl;
   cout << endl;

   global=0;
   cout << "Test extend:"<<endl;
   cout << "begin outer"<<endl;
     {
      Handle<A,Scopes::Extend> o2;
      cout << "begin inter"<<endl;
        {
         Handle<A,Scopes::Extend> o1=gen<A>();
         o2=o1;
         i=1;
        }
      if (global==1) fail=1;
      cout << "end inter"<<endl;
     }
   if (global!=1) fail=1;
   cout << "end outer"<<endl; 
   cout << endl;

   cout << "Test limit:"<<endl;
   cout << "begin outer"<<endl;
     {
      Handle<A,Scopes::Extend> o2=gen<A>();
      cout << "begin inter"<<endl;
        {
         Handle<A,Scopes::Limit> o1=o2;
         cout<< "In inter"<<endl;
         i=1;
        }
      if (global!=1) fail=1;
      cout << "end inter"<<endl;
     }
   if (global!=1) fail=1;
   cout << "end outer"<<endl; 
   cout << endl;

   if (fail) 
     return 1;
   else
     return 0;
  }

