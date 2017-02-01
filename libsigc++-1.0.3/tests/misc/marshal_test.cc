#include <iostream>
#include <sigc++/marshal.h>

#ifdef SIGC_CXX_NAMESPACES
using namespace std;
using namespace SigC;
#endif

int foo1()
  {
   return 5;
  }

int foo2()
  {
   RetCode::ignore();
   return 10;
  }

int main()
  {
   {int i=1000;
   }
   Marshal<int> v;
   cout << v.value() <<endl;
   v.marshal(foo1());
   v.marshal(foo2());

   FixedMarshal<int,-1> f;

   if (v.value()==5&&f.value()==-1) 
     return 0;
   else 
     return 1;
  }
