<sect>Gtk-- Specific stuff <label id="sec_specific">
Gtk::SArray is meant to be a transfer-class only, i.e.
constructed as a tempory object with implicit type-conversion 
from the types below. it will handle memory for it's char_data.

i.e. whenever you need a Gtk::SArray argument you can pass a 
argument of the follwoing type ...

o const char** data   -> Gtk::SArray does not copy char-data,
                         your app is responsible for freeing it.

o const Gtk::SArray&amp; c -> the new Gtk::SArray does not take
                         ownership of char-data.

o const list&lt;string&gt;amp; c
  const vector&lt;string&gt;amp; c
  const list&lt;gtkmm_string&gt;amp; c
  const vector&lt;gtkmm_string&gt;amp; c
  const list&lt;const char*&gt;amp; c
  const vector&lt;const char*&gt;amp; c

  -> Gtk::SArray copies the char-data and frees it after 
     being destroyed. your with new allocated Gtk::SArray
     never gets destroyed. and to allocated it is not 
     neccessary.


i do it like this e.g. ....

void function(Gtk::CList *clist)
{
  vector&lt;string&gt; v;
  v.push_back("string1");
  v.push_back("string2");
  v. ....

  static const char *titles[] = 
     { "Ingredients", "Amount", "Test", NULL };

  clist->append(v);
  clist->append(titles);
}



--------------------------------------------------
On gtkmm_wrap :

All the gtk+ cast constructors became private to the wrapper system
in order to satisfy the rule that every gtk+ object can have one and
only one Gtk-- wrapper at a time.  

If the users can pick what sort of wrapper they may pick the wrong one.

For example:
  GtkButton *b= gtk_button_new();
  add(new Gtk::Widget(b)); // b wrapper is now fixed at a widget

This meant that the item remained at the lower wrapper for its
lifetime and that the user had to handle the memory for the wrapper.  
We could have handled this by destroying wrappers and making a 
more derived one when we caught the mistake, but that would
envolve forcing the user to use some sort of handle class so we
could clean up the references (bad.)  Or we could have let a widget
have many wrappers, but that would mean that signal connections would
die when one of the wrapper did, leading to strange object fragmentation
problems.

Instead a factory called gtkmm_wrap was created.  It servers two
purposes.  It creates wrappers at the most derived class known
to the system and it binds the lifetime of that wrapper to that of
the gtk+ system.  Thus eliminating ambiguity of the user naming
the wrapper and the effort of cleaning up the mess.

Thus you can now do this...

  widget = glade_xml_get_widget( xml, name );
  Gtk::Widget* cppwidget = gtkmm_wrap(widget);  // this knows if it is a
                                               // button, checkbutton, etc!
  if (Gtk::Button::isA(cppwidget)) return dynamic_cast<Gtk::Button*>(cppwidget);

(looks a lot more like C++, doesn't it.)

This is much cleaner and the wrapper will die with the gtk+ object.
You can keep wrappers arround longer than their gtk+ lifetime by
using Handles.

Handle<Gtk::Widget> w=gtkmm_wrap(widget);  // references it so that gtk+
                                          // doesn't destroy it.

This leads to much cleaner code and make factory building interfaces
like yours much easier to write and mantain.

Another big change which is closely tied to this is that objects
can no long become disembouled like they did before.  This was 
perfectly legal code in 1.0...

  foo(Gtk::Container &amp;c)
    {
     Gtk::Button b("hello");
     b.show();
     c.add(b);
    }  // b is destroyed here, but its gtk+ half lived on!

Now b attempts to kill all references when it dies.  This includes
attempting to remove itself from containers or unparenting itself.

Thus code now needs to be written as.

  foo(Gtk::Container &amp;c)
    {
     Gtk::Button *b=manage(new Gtk::Button("hello"));
     b->show();
     c->add(*b);
    }

Just like gtk+ we don't have to use a handle here because the
object is "floating" until added to a container (or handle).  You could put
it into a handle if it was possible that it wouldn't get added
into the container.  (Manage serves the same function as 
before of tagging an object as controlled by gtk+, but the
handle part isn't necessary.)
   
Thus our code now "works" the way C++ users would expect.  Objects
stay together and remained wrapped with proper wrappers at all
times.  C++ scopes are strictly enforced.  A strong 1 to 1 
correspondence is established.

--------------------------------------------------

On the internals of a Gtk-- object

Agreed. I thought there were no way to do that kind of code, but guess
it is possible. :)

The problem is that the gtk--'s _impl methods work by deriving from gtk+
widget and overriding all the wrapped callbacks and then giving them to
C++ object. The real structure of gtk-- objects is the following:


    GtkObject                 Gtk::Object
       ^                          ^
       |                          |
    GtkWidget                 Gtk::Widget
       ^                          ^
       |                          |
    GtkButton                     |
       ^                          |
       |                          |
    GtkCppButton <>----------> Gtk::Button
                                  ^
                                  |
                               My_Cpp_Button

This is the structure of gtk-- objects versus gtk+ objects. gtk--
derives from every gtk+ widget using C's inheritance and overrides
wrapped methods and makes them call the _impl methods. _impl methods
then call (by default) the old implementation via function pointers
from GtkButton's virtual table.

This structure causes two bugs to gtk--:
1) you cannot call GtkWidget's implementation of virtual method in
   My_Cpp_Button using Gtk::Widget::delete_event_impl(args);
2) conversions from GtkButton to Gtk::Button does not attach
   *_impl methods at all, because gtk+ object is of type GtkButton
   and not GtkCppButton like it should be.

These two bugs have been in gtk-- for LONG time, and they've been something
we never planned to fix :) If someone figures out good way to fix them, nice - but I'm not sure how important they really are.. :)
