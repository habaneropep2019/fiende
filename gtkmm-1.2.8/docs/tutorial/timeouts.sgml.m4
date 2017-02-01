<sect>Timeouts, I/O and Idle Functions <label id="sec_timeouts">
<!-- ***************************************************************** -->

<!-- ----------------------------------------------------------------- -->
<sect1>Timeouts

<p> You may be wondering how to make GTK-- do useful work while it's
idling along (well, sleeping actually) in <tt/Gtk::Main()/.  Happily,
you have several options. Using the following functions you can create
a timeout function that will be called every few milliseconds.

<tscreen><verb>
Connection Gtk::Main::timeout.connect(const SlotType &amp;sd, guint32 interval);
</verb></tscreen>

The first argument is a slot you wish to have called when the timeout
occurs. The second argument is the number of milliseconds between
calls to your function. You get back a <tt/Connection/ object that can be
used to destroy the connection.  Use

<tscreen><verb>
MyConnection.disconnect();
</verb></tscreen>

to destroy the connection. Another way of destroying the Connection
is your callback function. It has to be of the type
<tt>Slot0&lt;gint&gt;</tt>. As you see from the definition your
callback function has to return a value of the type <tt>gint</tt>.  A
definition of a sample function might look like this:

<tscreen><verb>
gint MyCallback() { cout << "Hello World!"; return true; }
</verb></tscreen>

You can stop the timeout function by returning zero or <tt/false/ from
your callback function.  Therefore, if you want your
function to be called repeatedly, it should return a non-zero value,
or <tt/true/.

Here's an example of this technique:

example_incl_src(timeout/timeout.cc)

<!-- ----------------------------------------------------------------- -->
<sect1>Monitoring I/O

<p> A nifty feature of GDK (one of the libraries that underlying
GTK--) is the ability to have it check for data on a file descriptor
for you.  This is especially useful for networking applications. The
following function is used to do this:

<tscreen><verb>
Connection Gtk::Main::input.connect(const SlotType&amp; sd, gint source,
                                    GdkInputCondition condition);
</verb></tscreen>

The first argument is a slot you wish to have called when then the
specified event (see argument 3) occurs on the file descriptor you
specify using argument two. Argument three may be one or a combination
(using <tt>|</tt>) of:

<itemize>

<item>GDK_INPUT_READ - Call your function when there is data ready for
reading on your file descriptor.

<item>GDK_INPUT_WRITE - Call your function when the file descriptor is
ready for writing.

<item>GDK_INPUT_EXCEPTION - Call your function when an exception happened
on the file descriptor.
</itemize>

The return value is a Connection that may be used to stop monitoring
this file descriptor using the <tt>disconnect</tt> following function.
The callback function should be declared as follows:

<tscreen><verb>
void input_callback(gint source, GdkInputCondition condition);
</verb></tscreen>

where <tt/source/ and <tt/condition/ are as specified above. As usual
the slot is created with <tt>slot()</tt> and can be a member function
of an object.

A little (and somewhat dirty) example follows as usual.  To use
the example just execute it from a terminal; it doesn't create a
window.  It will create a pipe named <tt>testpipe</tt> in the current
directory. Then start another shell and execute <tt>cat
>testpipe</tt>. The example will print each line you enter until you
type <tt>quit</tt>.

example_incl_src(input/input.cc)

<!-- ----------------------------------------------------------------- -->
<sect1>Idle Functions
<p>

What if you have a function you want called when nothing else is
happening?  Hook it up using the following:

<tscreen><verb>
Connection Gtk::Main::idle.connect(Slot0<gint> idlefunc, gint priority);
</verb></tscreen>

This causes GTK-- to call the specified function whenever nothing else
is happening. You can add a priority (lower numbers are higher
priorities).  If you don't supply a priority value, then
GTK_PRIORITY_DEFAULT will be used. There are two ways to remove the
callback: calling <tt>disconnect</tt> on the Connection object, or
returning <tt/false/ (or 0) in the callback function, which should be
declared as follows:

<tscreen><verb>
int idleFunc();
</verb></tscreen>

Since this is very similar to the functions above this explanation should
be sufficient to understand what's going on. However, here's a little example:

example_incl_src([[idle]]/idle.cc)

This example points out the difference of idle and timeout functions a
little.  If you need functions that are called periodically, and speed
is not very important, then you want timeout functions. If
you want functions that are called as often as possible (like
calculating a fractal in background), then use idle functions.

Try executing the example and increasing the system load. The upper
progress bar will increase steadily; the lower one will slow down.

<!-- ***************************************************************** -->

