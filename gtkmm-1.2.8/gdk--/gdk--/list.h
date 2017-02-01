#ifndef _GDKMM_LIST_H_
#define _GDKMM_LIST_H_

#ifdef GTKMM_CXX_HAVE_PARTIAL_SPECIALIZATION
// Dummy class to make it appear as though the user has the list.
/*
template<class Parent,class Iterator,class Access>
  class Gdk_List 
  {
   public:
     typedef Gdk_List<Parent,Iterator,Access> List;
   private:
     Parent* parent;
   public:
     Gdk_List(Parent& p):parent(p) {}
     Gdk_List():parent(0) {}
     Gdk_List(const List& list):parent(list.parent) {}

     Iterator begin() 
       {
        if (parent)
          return Access::begin(parent);
        return Iterator();
       }
     Iterator end();
       {
        if (parent)
          return Access::end(parent);
        return Iterator();
       }
  };
*/

// An iterator that caches the current object to C++ for speed
template<class C_Obj, class Cpp_Obj>
  class Gdk_List_Iterator 
  {
   public:
     typedef Gdk_List_Iterator<C_Obj,Cpp_Obj> self;

   private:
     GList *node;
     Cpp_Obj cache;
     
   public:
     self& operator=(const self& x) 
       {
        cache.free();
        node=x.node;
       }

     bool operator==(const self& x) const 
       { return node == x.node; }
     bool operator!=(const self& x) const 
       { return node != x.node; }

     Gdk_List_Iterator(GList *n) : node(n),cache(0) 
       {}
     Gdk_List_Iterator() :node(0),cache(0)
       {}
     Gdk_List_Iterator(const self& x) 
        : node(x.node),cache(0) 
       {}

     Cpp_Obj& operator*() const 
       {
        if (node) 
          {if (cache.gdkobj()!=node->data)
             cache=Cpp_Obj(node->data);
          }
        else
          cache=Cpp_Obj(0);
        cache=0;
        return ;
       }

     Cpp_Obj* operator->() const 
       {
        return &(operator*());
       }

     self&  operator++() 
       { 
        cache.free();
        if (node && node->next) 
          node = node->next;
        return *this;
       }

     self operator++(int) 
       { 
        self tmp = *this;
        ++*this;
        return tmp;
       }

     self& operator--() 
       {
        cache.free();
        if (node && node->prev)
          node=node->prev;
        return *this;
       }

     self operator--(int) 
       { 
        self tmp = *this;
        --*this;
        return tmp;
       }

   };

/*
Gdk_List_Iterator<GdkWidget*,Gdk_Widget> iter;
(*iter)  should be a Gdk_Widget

Example usage:

  class Gdk_Foo()
    {
     public: 
       typedef Gdk_List_Iterator<GdkWidget*,Gdk_Widget> Iterator;
       typedef Gdk_List<Gdk_Foo,Iterator,Child_Access) Child_List; 
     private:
       struct Child_Access
         {
          static Iterator begin(Gdk_Foo& foo)
            {return foo.get_children_begin();}
          static Iterator end(Gdk_Foo& foo)
            {return foo.get_children_end();}
         };
     public:
//       GList* get_children();
       Iterator get_children_begin()
         {return Iterator(gdk_foo_get_children(*this);}
       Iterator get_children_end()
         {
          GList* list=gdk_foo_get_children(*this);
          return Iterator(g_list_last(list);
         }
       Child_List get_children()
         {
          return Child_List(this);
         }
       
    };
*/

#endif

#endif // _GDKMM_LIST_H_
