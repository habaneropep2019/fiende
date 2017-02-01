#include <iostream>
#include <sigc++/object.h>
#include <sigc++/scope.h>
#include <sigc++/handle.h>

#ifdef SIGC_CXX_NAMESPACES
using namespace SigC;
#endif

template <class T>
  class Ref:public Handle<T,Scopes::Extend>
    {
     HANDLE_CTORS(Ref,T,Scopes::Extend)
    };

int main()
  {
   Ref<Object> o=manage(new Object());
   return 0;  // Not crashing is a pass.
  }
