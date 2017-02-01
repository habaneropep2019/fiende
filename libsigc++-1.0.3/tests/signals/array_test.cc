#include <iostream>
#include <string>
#include <sigc++/signal_system.h>

/*
Copyright 1999, Karl Nelson

(contributions by eyal.ben-david@aks.com)

This program shows how to use slots in STL maps to form callback
maps for menus and such.

*/
#ifdef SIGC_CXX_NAMESPACES
using namespace std;
using namespace SigC;
#endif

typedef Slot1<int,int> MenuSlot;

struct MenuItem
  {
    string   name;
    int      id;
    MenuSlot call;
    MenuItem(const char* s, int ID, MenuSlot acall) 
      : name(s), id(ID), call(acall) {}
    MenuItem() 
      : name(), id(0), call() {}
  };

int fred(int)
  {cout << "call fred"<<endl; return 1;}
int bob(int)
  {cout << "call bob"<<endl; return 1;}
int george(int)
  {cout << "call george"<<endl; return 1;}


MenuItem menu[]=
  { 
    MenuItem("fred",   1, slot(fred)),
    MenuItem("bob",    1, slot(bob)),
    MenuItem("george", 1, slot(george)),
    MenuItem()
  };


int main(int argc,char **argv)
  {
    int i;
    string command;
    while (command!="quit")
      {
        cout << ">>";
        cin >> command;
        for (i=0;menu[i].id!=0;i++)
        if (menu[i].name==command) 
        menu[i].call(i);
      }
    return 0; //compiling is passing
  }


