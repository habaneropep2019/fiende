<!-- ***************************************************************** -->
<sect>Moving On
<!-- ***************************************************************** -->

<!-- -----------------------------------------------------------------
--> <sect1>Data Types

<p> There are a few types you've probably noticed in the previous
examples that might need explaining.  The <tt/gint/, <tt/gchar/ and
other "g" types that you saw are <tt/typedef/s to <tt/int/ and <tt/char/,
respectively. This is done to get around that nasty dependency on the
sizes of simple data types when doing calculations.

A good example is <tt/gint32/, which will be <tt/typedef/ed to a 32 bit
integer for any given platform, whether it be the 64 bit alpha, or the
32 bit i386. The typedefs are very straightforward and intuitive. They
are all defined in <tt>glib/glib.h</tt>, which gets included from
<tt/gtk.h/.

<!-- ----------------------------------------------------------------- -->
<sect1>Using X events

<p>
In addition to the signal mechanism described above, there are a set
of <em>events</em> that reflect the X event mechanism. Callbacks may
also be attached to these events, just like regular signals (in fact,
from the application programmer's point of view, there is no
difference). These events are:

<itemize>
<item> event
<item> button_press_event
<item> button_release_event
<item> motion_notify_event
<item> delete_event
<item> destroy_event
<item> expose_event
<item> key_press_event
<item> key_release_event
<item> enter_notify_event
<item> leave_notify_event
<item> configure_event
<item> focus_in_event
<item> focus_out_event
<item> map_event
<item> unmap_event
<item> property_notify_event
<item> selection_clear_event
<item> selection_request_event
<item> selection_notify_event
<item> proximity_in_event
<item> proximity_out_event
<item> drag_begin_event
<item> drag_request_event
<item> drag_end_event
<item> drop_enter_event
<item> drop_leave_event
<item> drop_data_available_event
<item> other_event
</itemize>

Connecting callbacks to these events is done exactly as it is for any
other signal.  For example:

<tscreen><verb>
gint button_press_callback(GdkEventButton *event);
Gtk::Button button("label");
button.button_press_event.connect( slot(&amp;button_press_callback) );
</verb></tscreen>

When the mouse is over the button and a mouse button is pressed,
<tt/button_press_callback()/ will be called.  The value returned from
<tt/button_press_callback()/ indicates whether the callback "handled"
the event.  If the value is zero (false), then the callback did not
handle the event, and GTK-- will pass the event on to the widget's
parent.  If the value is non-zero (i.e., true), the callback handled
the event, and the event will not be seen by any other widget.  (See
Chapter <ref id="sec_Adv_Events_and_Signals" name="Advanced Event and Signal
Handling"> for more information on this mechanism.)

<tt/GdkEventButton/ is a structure containing the event's parameters,
such as the coordinates of the mouse pointer at the time the button
was pressed.  There are several different types of <tt/GdkEvent/
structures; which one is used in the callback depends on the event
type.  For details on the GdkEvent data types, see Appendix
<ref id="sec_GDK_Event_Types" name="GDK Event Types">.

You'll find it useful to handle X events when there's something you
can't accomplish with a widget's signals alone.  <tt/Gtk::Button/, for
example, does not send mouse-pointer coordinates with the <tt/clicked/
signal; you could handle <tt/button_pressed_event/ for
<tt/Gtk::Button/ if you needed this information.  X events are also
often used to handle key-presses.

Handling an X event doesn't affect the operation of a widget's other
signals.  If you handle <tt/button_pressed_event/ for
<tt/Gtk::Button/, you'll still be able to get the <tt/clicked/ signal,
if you want them both; they are emitted at (nearly) the same time.

<!-- ----------------------------------------------------------------- -->
<sect1>More on Signal Handlers
<p>

Let's take another look at the declaration for <tt/SigC::Signal1/:

<tscreen><verb>
SigC::Connection Signal1&lt;void,int&gt;::connect( Slot1&lt;void,int&gt;&amp; );
</verb></tscreen>

Notice that the return value is of type <tt/SigC::Connection/.  This
is an object which you can use to control a connection to a callback.
By keeping a copy of this object, you can disconnect its associated
callback using the method <tt/SigC::Connection::disconnect()/.

divert(-1)
Another function to remove all the signal handers from an object is:

!!!!!!!!! <-- I am not sure that we have such a beast right now -->
<tscreen><verb>
void gtk_signal_handlers_destroy( GtkObject *object );
</verb></tscreen>

This call is fairly self explanatory. It simply removes all the
current signal handlers from the object passed in as the first
argument.
!!!!!!!!

divert(0)

<!-- -----------------------------------------------------------------
--> <sect1>An Upgraded Hello World 

<p> Let's take a look at a slightly improved <tt>helloworld</tt> with
better examples of callbacks.  Here we also introduce <em>packing
widgets</em>, which is our next topic.

example_incl_src(helloworld2/helloworld2.cc)

Compile this program using the same linking arguments as our first
example.  This time, there is no easy way to exit the program; you
have to use your window manager or command line to kill it.
(Exercise: add a "Quit" button that will exit the program.)  Try
playing with the options to <tt/pack_start()/ while reading the next
section.  Also resize the window, and observe the behaviour.

Note that there is another useful constant for the <tt/Gtk::Window/
constructor: <tt/GTK_WINDOW_DIALOG/. This causes the window manager to
treat the window as a transient dialog window.

