{
<sect>Memory management (draft) <label id="sec_Memory">
<!-- ***************************************************************** -->
<p>
Widget Memory management
 
Gtk-- makes the programmer responsible for deciding how every widget will be 
destroyed.  In addition to the standard memory management tools provided by C++, Gtk-- 
allows a widget to be managed by the system, using manage() and add().  A widget that 
is created using manage() and then packed into a container using add() will be destroyed
whenever the container is destroyed.  Standard C++ memory management methods,
whether new() and delete() or automatic variables, remain appropriate in many instances.
These unmanaged widgets are destroyed according to C++ requirements (new() widgets 
must be destroyed by calling delete() and automatic widgets are destroyed when out of
scope).
 
With the addition of manage() and add(), there are four ways to manage widgets:
1) Dynamic allocation with manage() and add()
2) Dynamic allocation with new() and delete()
3) Class scope data members
4) Function scope (automatic) data
 
<sect1>1) Dynamic allocation with manage() and add()
<p> 
Gtk-- provides the manage() and add() methods to create and destroy widgets. 
Every widget except a top-level window must be added or packed into a container in 
order to be displayed.  The manage() function marks a packed widget so that when the 
widget is added to a container, the container becomes responsible for deleting the 
widget.

<tscreen><verb>
	MyWidget::MyWidget()
	{
		Gtk::Button &amp;aButton = manage(new Gtk::Button("Test"));
		add(aButton); //add aButton to MyWidget
	}
</verb></tscreen> 
Now, when MyWidget is destroyed, aButton will also be destroyed.  It is not
necessary to delete() aButton to free aButton's memory.
 
Gtk-- also provides the set_dynamic() method for all widgets.  set_dynamic() 
can be used to generate the same result as manage(), but is laborious:
 
foo.add( (w=new Gtk::Label("Hello"), w-&gt;set_dynamic(), &amp;w) );
 
is the same as

foo.add(&amp;manage(new Gtk::Label("Hello"));
 
Of course, a top level container will not be added to another container.  The
programmer is responsible for destroying the top level container using one of 
the traditional C++ tools.
 
<sect2>2) Dynamic allocation with new and delete
<p> 
Although in most cases, the programmer will prefer to allow containers to 
automatically destroy their children using manage(), the programmer is not 
required to use manage(). The traditional new() and delete() functions may also be 
used.

<tscreen><verb>
	Gtk::Button &amp;aButtonptr = new Gtk::Button("Test");
	
	// do something useful with aButtonptr
	
	delete aButtonptr;
</verb></tscreen>
Here, the programmer deletes aButtonptr to prevent a memory leak.
 
<sect3>3) Class Scope widgets
<p>
If a programmer does not need dynamic memory allocation, automatic widgets in class 
scope may be used.  One advantage of autmoatic widgets in class scope is that
memory management is grouped in one place.  The programmer does not 
risk memory leaks from failing to delete() a widget.
 
The primary disadvantages of using class scope widgets are revealing
the class implementation rather than the class interface in the class header.  Class
scope widgets also require Automatic widgets in class scope suffer the same disadvantages as 
any other class scope automatic variable.  

<tscreen><verb>
	#include &lt;gtk--/button.h&gt;
	class Foo {
	private:
		Gtk::Button theButton;
		// will be destroyed when the Foo object is destroyed
	};
</verb></tscreen>

<sect4>4) Function scope widgets
<p>
If a programmer does not need a class scope widget, a function scope widget 
may also be used.  The advantages to function scope over class scope are the 
increased data hiding and reduced dependencies.
<tscreen><verb>
	{
		Gtk::Button aButton;
		aButton.show();
		...
		kit.run();
	}
</verb></tscreen> 
Other resources:
This discussion is based upon email from the gtk-- mailing list.  You can 
subscribe to the list at http://lists.sourceforge.net/mailman/listinfo/gtkmm-main.  You 
can review archives at http://www.geocrawler.com/lists/3/SourceForge/1110/0/ and 
are encouraged to do so.


<!-- ***************************************************************** -->

