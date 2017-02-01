__BEGIN__

dnl
dnl  These variables affect the generation of the list
dnl 
define(GP_LIST_HELPER,`define(`__LISTPREFIX__',__NAMESPACE__::$1_Helpers::__LISTNAME__)')
define(GP_LIST_ELEM,`define(`__LISTELEM__',`$*')')
define(GP_LIST_ITER,`define(`__LISTITER__',`$*')')
define(GP_LIST_READONLY,`define(`__LISTRO__')')
define(GP_LIST_NOINSERT,`define(`__LISTEO__')')

dnl
dnl GP_LIST(ListName, Parent, TypeName, FieldName)
dnl
dnl  List with most of the trimmings
dnl  You should define
dnl    - iterator insert(iterator,ElementType&)
dnl    - iterator erase(iterator)
dnl    - void remove(const_reference)
dnl
dnl  Fieldname assumed to be children if not specified
define(GP_LIST,`
__PUSHDIV__

define(`__LISTNAME__',$1)
define(`__LISTPARENT__',$2)
define(`__LISTTYPE__',$3*)
define(`__LISTELEM__',const Element)
define(`__LISTITER__',G_List_Iterator<value_type>)
define(`__LISTPREFIX__',__NAMESPACE__::$2_Helpers::$1)
define(`__LISTFIELD__',ifelse($4,,children,$4))

GTKMM_SECTION(USR)
')

dnl
dnl  GP_LIST_END()
dnl
dnl   Closes a list
define(GP_LIST_END,`dnl
__POPDIV__
  
GLINE()
  class __LISTNAME__
    { 
      public:
        typedef __LISTTYPE__                       value_type;
        typedef value_type &                       reference;
        typedef const value_type &                 const_reference;

        typedef __LISTITER__        iterator;
        typedef G_List_ConstIterator<iterator>     const_iterator;
        typedef G_List_ReverseIterator<iterator>   reverse_iterator;
        typedef G_List_ConstIterator<reverse_iterator>   const_reverse_iterator;

        typedef size_t                             difference_type;
        typedef size_t                             size_type;

      private:
        friend class __NAMESPACE__`'::__LISTPARENT__;
        __NAMESPACE__`'::__LISTPARENT__ *parent_;
        explicit __LISTNAME__`'(__LISTPARENT__* parent): parent_(parent) {}

        GList*& glist() const;      // front of list

        iterator begin_() const;
        iterator end_() const;
      
      public:
        ~__LISTNAME__`'() {}

        inline iterator begin() 
          {return begin_();}
        inline iterator end()   
          {return end_();}

        inline const_iterator begin() const
          { return const_iterator(begin_()); }
        inline const_iterator end() const
          { return const_iterator(end_()); }

        inline reverse_iterator rbegin()
          { return reverse_iterator(end_()); }
        inline reverse_iterator rend()
          { return reverse_iterator(begin_()); }

        inline const_reverse_iterator rbegin() const
          { return const_reverse_iterator(reverse_iterator(end_())); }
        inline const_reverse_iterator rend() const
          { return const_reverse_iterator(reverse_iterator(begin_())); }

        size_type size(void) const;
        inline size_type max_size(void) { return size_type(-1); }
        inline bool empty(void) { return glist() == 0; }

ifdef(`__LISTRO__',,`dnl
ifdef(`__LISTEO__',,`dnl
        iterator insert(iterator position, __LISTELEM__& e);
        template <class InputIterator>
        inline void insert(iterator position, InputIterator first, InputIterator last)
          { for (;first!=last;++first) position=insert(position,*first); }

        inline void push_front(__LISTELEM__& e) { insert(begin(), e); }
        inline void push_back(__LISTELEM__& e)  { insert(end(), e); }
        inline void pop_front()                  { erase(begin()); }
        inline void pop_back()                   { erase(--end()); }
')dnl

        void clear();

        iterator erase(iterator);
        void erase(iterator start, iterator stop);
        void remove(const_reference);
')dnl
GTKMM_IMPORT(USR)
   };

__PUSHDIV__
GTKMM_SECTION(METHOD)
GList*& __LISTPREFIX__::glist() const
{ return parent_->gtkobj()->__LISTFIELD__; }

__LISTPREFIX__::iterator __LISTPREFIX__::begin_() const
{return iterator(glist(),glist());}

__LISTPREFIX__::iterator __LISTPREFIX__::end_() const
{return iterator(glist(),(GList*)0);}

__LISTPREFIX__::size_type __LISTPREFIX__::size() const
  { return g_list_length(glist()); }

ifdef(`__LISTRO__',,`dnl
void __LISTPREFIX__::clear()
{ erase(begin(),end()); }

void __LISTPREFIX__::erase(iterator start, iterator stop)
{ while(start != stop) start=erase(start); }
')

undefine(`__LISTNAME__')dnl
undefine(`__LISTTYPE__')dnl
undefine(`__LISTPARENT__')dnl
undefine(`__LISTPREFIX__')dnl
undefine(`__LISTELEM__')dnl
undefine(`__LISTFIELD__')dnl
undefine(`__LISTRO__')dnl
__POPDIV__
')

dnl
dnl  GP_LIST_FIND(access)
dnl
dnl    Defines  find(containertype) and find(Widget&)
dnl    access is the name of method returning a Widget* 
define(GP_LIST_FIND,`
GLINE()
        iterator find(const_reference c);
        iterator find(Widget&);
__PUSHDIV__
GTKMM_SECTION(METHOD)
__LISTPREFIX__::iterator __LISTPREFIX__::find(const_reference w)
{
  iterator i=begin();
  for (i=begin();i!=end()&&*i!=w;i++);
  return i;
}

__LISTPREFIX__::iterator __LISTPREFIX__::find(Widget& w)
{
  iterator i;
  for (i=begin();i!=end()&&(*i)->$1()!=&w;i++);
  return i;
}

__POPDIV__
')

dnl
dnl  GP_LIST_CONTAINER_REMOVE(access)
dnl
dnl    Implements remove(const_reference), erase(iterator)
dnl    and defines remove(Widget&)
dnl    (assumes that the widget uses gtk+ container methods)
dnl    access is the name of method returning a Widget* 
define(GP_LIST_CONTAINER_REMOVE,`
GLINE()
        void remove(Widget& w);
__PUSHDIV__
GTKMM_SECTION(METHOD)
void __LISTPREFIX__::remove(const_reference child)
{
  g_return_if_fail(child!=0);
  gtk_container_remove(parent_->Container::gtkobj(),
                       (GtkWidget*)(child->ifelse($1,,,$1()->)gtkobj()));
}

void __LISTPREFIX__::remove(Widget &w)
{
  gtk_container_remove(parent_->Container::gtkobj(),(GtkWidget*)(w.gtkobj()));
}

__LISTPREFIX__::iterator __LISTPREFIX__::erase(iterator position)
{
  if (!position.node||position==end()) return end();
  iterator next=position;
  next++;

  gtk_container_remove(parent_->Container::gtkobj(),
      (GtkWidget*)((*position)->ifelse($1,,,$1()->)gtkobj()));
  return next;
}

__POPDIV__
')

dnl
dnl This provides a slow non-standard front, back, operator[] which may
dnl be useful for some containers.
dnl  Ie erase 4th page -  tools().remove(tools()[3]);
define(GP_LIST_VECTOR,`
GLINE()
        value_type front() const     { return *begin(); }
        value_type back() const      { return *(--end()); }

        // This is order n. (use at own risk)
        value_type operator[](size_type l) const;
__PUSHDIV__
GTKMM_SECTION(METHOD)
__LISTPREFIX__::value_type __LISTPREFIX__::operator[](size_type l) const
          { size_type j; iterator i; for (i=begin(),j=0;i!=end(),j<l;++i,++j);
            return (*i);
          }

__POPDIV__
')

__END__
