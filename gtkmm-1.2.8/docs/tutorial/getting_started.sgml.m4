<!-- ***************************************************************** -->
<sect>Getting Started
<!-- ***************************************************************** -->

<p>
The first thing to do, of course, is to obtain GTK--.  You can get the 
latest version from <url url="http://gtkmm.sourceforge.net/">.
If you don't already have it, you will also need to 
install the companion library libsigc++, available from 
<url url="http://libsigc.sourceforge.net/">.

The GTK-- source distribution also contains the complete source to 
the examples used in this tutorial, along with Makefiles to aid
compilation.  The binary package distributions include it as well;
the example sources are included with the gtkmm-devel RPM, which 
installs them to /usr/doc.  You'll need to copy the examples subdirectory 
to your home directory, or some other place where you have write access,
to compile the examples, if you're using the RPM distribution.

To begin our introduction to GTK--, we'll start with the simplest
program possible. This program will create an empty 200x200 pixel window.
It has no way of exiting except to be killed using the shell.

example_incl_src(base/base.cc)

You can compile the above program with gcc using:
<tscreen><verb>
g++ base.cc -o base `gtkmm-config --cflags --libs`
</verb></tscreen>

The meaning of the unusual compilation options is explained below.

All GTK-- programs must include certain GTK-- headers; gtk--.h
includes the entire GTK-- kit, which is usually not a good idea, since
it includes a meg or so of headers, but for simple programs, it
suffices.

The next line:

<tscreen><verb>
Gtk::Main kit(argc, argv);
</verb></tscreen>

creates a <tt/Gtk::Main/ object.  This is needed in all GTK--
applications. The constructor for this object initializes the GTK--
library for use, sets up default signal handlers, and checks the
arguments passed to your application on the command line, looking for
the following options:

<itemize>
<item> <tt/--gtk-module/
<item> <tt/--g-fatal-warnings/
<item> <tt/--gtk-debug/
<item> <tt/--gtk-no-debug/
<item> <tt/--gdk-debug/
<item> <tt/--gdk-no-debug/
<item> <tt/--display/
<item> <tt/--sync/
<item> <tt/--no-xshm/
<item> <tt/--name/
<item> <tt/--class/
</itemize>

It removes these from the argument list, leaving anything it does not
recognize for your application to parse or ignore.  This ensures 
that all GTK-- applications accept the same set of standard arguments.

The next two lines of code create and display a window:

<tscreen><verb>
  Gtk::Window window (GTK_WINDOW_TOPLEVEL);
  window.show();
</verb></tscreen>

The <tt/GTK_WINDOW_TOPLEVEL/ argument specifies that we want the window to
undergo window manager decoration and placement. Rather than create a
window of 0x0 size, a window without children is set to 200x200 by
default so you can still manipulate it.

The <tt/show()/ method lets GTK-- know that we are done setting
the attributes of this widget, and that it can display it.

The last line enters the GTK-- main processing loop.

<tscreen><verb>
  kit.run();
</verb></tscreen>

This is another call you will see in every (working) GTK-- application
(the <tt/run()/ method of the <tt/Gtk::Main/ class).  When control
reaches this point, GTK-- will sleep waiting for X events (such as
button or key presses), timeouts, or file I/O notifications to
occur. In this simple example, however, events are ignored.

To compile this, use this command (assuming you've put the source code
in <tt/simple.cc/):

<tscreen><verb>
g++ -Wall -g simple.cc -o simple `gtkmm-config --cflags --libs`
</verb></tscreen>

To simplify compilation, we use the program <tt>gtkmm-config</>, which
is present in all (properly installed) Gtk-- installations.  This
program 'knows' what compiler switches are needed to compile programs
that use GTK--.  The <tt>--cflags</> option causes
<tt>gtkmm-config</tt> to output a list of include directories for the
compiler to look in; the <tt>--libs</> option causes it to output the
list of libraries for the compiler to link with and the directories to
find them in (try running it from your shell-prompt to see what it's
printing on your system).

For the above compilation command to work, note that you <em>must</em> surround 
the <tt>gtkmm-config</tt> invocation with backquotes.  
Backquotes cause the shell to execute the command inside them, and to use 
the command's output as part of the command line.

Some of the libraries that are usually linked in are:
<itemize>
<item>The GTK-- library (-lgtkmm)
<item>The Gdk-- library (-lgdkmm)
<item>The GTK library (-lgtk), the widget library, based on top of GDK.
<item>The GDK library (-lgdk), the Xlib wrapper.
<item>The gmodule library (-lgmodule), which is used to load run time
extensions.
<item>The Xlib library (-lX11) which is used by GDK.
<item>The Xext library (-lXext). This contains code for shared memory
pixmaps and other X extensions.
<item>The math library (-lm). This is used by GTK for various purposes.
</itemize>

This list will vary from installation to installation; 
run <tt>gtkmm-config --libs</tt> to see it for your system.

<!-- ----------------------------------------------------------------- -->
<sect1>Theory of Signals and Callbacks
<p>
GTK is an <em>event driven</em> toolkit, which means it will sleep
until an event occurs and control is passed to the appropriate
function. This is done using a callback mechanism called a
<em>signal</em>. When an event occurs, such as the press of a mouse
button, the appropriate signal will be <em>emitted</em> by the widget
that was pressed.  This is how GTK does most of its useful work. There
are a set of signals that all widgets inherit, such as "destroy", and
there are also signals that are widget specific, such as "toggled" for
toggle buttons.  You can also create your own signals.  To make a
widget - a button for example - perform an action, we set up a
<em>signal handler</em> to catch the signal(s) the button emits; the
signal handler will perform whatever actions we want to be triggered
by the button-press.

GTK-- objects emit signals using special signal-emitting objects which
we will call <em>signallers</em> (they're also known as "signal
objects").  A good way to understand this is to think of a traffic
light.  In most countries, a traffic light is a box with three lights
on it, one red, one green, and one yellow.  Most people know what
these lights mean: red means "stop", yellow means "slow down", and
green means "go".  If a traffic light were a GTK-- object, it would
have three "lamps" (signallers), which we might call Red, Green, and
Yellow.  In the same way, a (simple) GTK-- button has only one "lamp"
(signaller) on it: it's called <tt/clicked/, and it "blinks" (emits a
signal) whenever somebody presses it.

Naturally, signals aren't much good unless somebody's watching them; a
traffic light could blink red all day, but it would just be showing
pretty colours if nobody stopped for it.  Fortunately, most people
know about traffic lights, but computers are a bit duller; we have to
tell them explicitly what signals to watch out for, and what to do
when they see them.  We do this by <em>connecting</em> a signaller to
a <em>callback function</em> (or simply <em>callback</em>); the
callback function is what does something in response to the signal.

Thanks to the flexibility of libsigc++, the callback library used by
GTK--, the callback can be almost any kind of function: it can be a
method of some object, or a standalone static function - it can even
be a signaller.

There is a point of potential confusion here which you should watch
out for.  We've given the name "signaller" to the libsigc++ objects
which emit signals, but that's not what these objects are called by
the library; libsigc++ signal objects are all of type <tt/SigC::Signal/,
as we'll see later.  Remember that these objects, even though
they're called "signals", are simply the "light bulbs" that do the
signalling.  In the traffic light example, the traffic light's light
bulbs would be called <tt/SigC::Signal/s in libsigc++ parlance; keep that
in mind, and you should be alright.

Here's an example of a callback being connected to a signal:

<tscreen><verb>
#include <libsigc++>
#include <gtk--/button.h>

void hello()
{
    cout << "Hello World" << endl;
}

main()
{
    Gtk::Button button("Hello World");
    button.clicked.connect(slot(&amp;hello));
}
</verb></tscreen>

There's rather a lot to think about in this (non-functional) code.
First let's identify the parties involved:

<itemize>
<item>The callback function is <tt/hello()/.
<item>We're hooking it up to the <tt/Gtk::Button/ object called <tt/button/.
<item>When the Button emits its <tt/clicked/ signal, <tt/hello()/ will be
called.
</itemize>

Now that we've got that straight, how exactly does <tt/hello()/ get called?
The answer is that we <em>connected</em> <tt/hello()/ to one of the
<tt/Gtk::Button/'s signallers, in this case the one called <tt/clicked/.  A
signaller is an object of one of the template classes <tt/SigC::Signal/#,
where # is a number from 0 to 7 (as we'll show later, this number
determines how many arguments the callback should take).  A signaller's
sole purpose in life is to call callback functions.

(An aside: GTK calls this scheme "signalling"; the sharp-eyed reader
with GUI toolkit experience will note that this same design is oft
seen under the name of "broadcaster-listener" (e.g., in Metrowerks'
PowerPlant framework for the Macintosh).  It works in much the same
way: one sets up <em>broadcasters</em>, and then connects
<em>listeners</em> to them; the broadcaster keeps a list of the
objects listening to it, and when someone gives the broadcaster a
message, it calls all of its objects in its list with the message.  In
GTK--, signal objects play the role of broadcasters, and slots
play the role of listeners - sort of.  More on this later.)

On first hearing of the concept of connecting callback functions to
signal objects, one might well suppose that the signal object's
<tt/connect/ method takes as argument a function pointer.  But C++ is
a strongly typed language, so this won't work, unless we're certain
that we will only ever need one type of callback function.  But we
won't; we need lots of different types of callback functions, with
varying numbers of arguments; and those arguments need to be of any
type we want, and oh yes, we also need to be able to have any return
value we want, and also we want complete compile-time type-checking,
and for our beer to be free.

Believe it or not, C++ can handle all of these requirements but the
last one.  (Sorry.)  Not only that, we get added flexibility.  The
particular technique by which we achieve all these things does work,
but like so many things in life, it involves a tradeoff; it can be
a bit difficult to get your head 'round at first, because it involves
what looks at first like an odd bit of syntactical gymnastics, and it
makes very heavy use of templates (the magic behind most of the
weirdness and wonderfulness of C++).  But it works, and it works
beautifully; so well, in fact, that although you may have come to
GTK-- for other reasons, you might well stick around because of the
ingenious signalling system.

Now let's get back to the code.  Here again is the line where we
hooked up the callback:

<tscreen><verb>
    ...
    button.clicked.connect(slot(&amp;hello));
    ...
</verb></tscreen>

Keep in mind that we're trying to set things up so that <tt/hello()/
gets called when we click the button.  Now, note again that we don't
pass a pointer to <tt/hello()/ directly to the button's signal's
<tt/connect()/ function (remember, we can't).  Instead, we pass it to
something called <tt/slot()/, and pass the result of <em>that</em> to
<tt/connect()/.  What's that <tt/slot()/ function for?

slot() is a <em>factory function</em> which generates, unsurprisingly,
<em>slots</em>.  A slot is an object which looks and feels like a
function, but is actually an object.  Such beasts are known as
<em>function objects</em>, or <em>functors</em>, and they're the key
to the GTK-- operation.  They're used instead of pointers because a pointer
can't really know what kind of thing it's pointing at.  Compilers can
give you the illusion of this, using typed pointers, but the illusion
breaks down in this situation.

Slots don't have that problem.  Unlike function pointers, slots "know"
what kind of function they're pointing at.  You can make slots that
point to member functions, static functions, normal functions -
indeed, any kind of function at all - by invoking the proper
slot-building factory function.  Slots not only know what flavour of
function they are pointing to, but they know where it is; if a slot is
pointing to a method of some dynamically allocated object - which it
can - the user of the slot <em>doesn't have to know what the type of
that object is</em>.  This is what makes slots so much better than
pointers-to-methods: if we used those, signal objects would have to
know what the type of those objects was, and since signal objects
often come as part of a library (e.g., GTK--), they simply can't
(unless you insist that the GTK-- authors <tt/#include/ your headers
in the GTK-- source, which they won't).

Why is a slot called a "slot"?  We don't know exactly; but we've got a
good guess.  You've probably seen a postal box with a narrow
rectangular opening in it just big enough for letters to be put
through.  English speakers tend to refer to narrow rectangular
openings as "slots", and slots are often used for passing messages
(bits of mail) about, just as they are in GTK--.  If the Button in the
previous example were telling you about its having been pressed by
sending you a letter ("Dear developer: At eight o'clock, I was clicked
.."), it would very likely post that letter by pushing it through a
slot.  In the world of GTK--, the button does indeed tell you of its
clickedness by passing a "message" through a slot (object).

Incidentally, the <tt/slot()/ functions are all declared as part of the
namespace <tt/SigC/.  The fully qualified name of <tt/slot()/ is <tt/SigC::slot()/;
we've omitted it in these examples for brevity, but you need to know
about this, in case you find your compiler telling you that it doesn't
know where <tt/slot()/ is.  All libsigc++ types and static functions are
declared in the <tt/SigC/ namespace.

Here's a slightly larger example of slots in action:

<tscreen><verb>
void callback();

class callback_class : public SigC::Object
{
    void method();
};

callback_class callback_object;

main()
{
    Gtk::Button button;
    button.clicked.connect( slot(&amp;callback) );
    button.clicked.connect( slot(callback_object, &amp;callback_class::method) );
}
</verb></tscreen>

The first call to <tt/connect()/ is just like the one we saw last
time; nothing new here.  The next is more interesting.  <tt/slot()/ is
now called with two arguments (it's overloaded).  The first argument
is "f", which is the object that our new slot will be pointing at; the
second argument is a pointer to one of its methods.  This particular
version of <tt/slot()/ creates a slot which will, when "called", call
the pointed-to method of the specified object, in this case
f.mymethod().

Another thing to note about this example is that we placed the call to
<tt/connect()/ twice for the same signal object.  This is perfectly
fine; the result will be that when the button is clicked,
<em>both</em> slots will be called.  You can make buttons do double,
triple, or n-duty this way, without modifying much code.

We just told you that the button's <tt/clicked/ signaller is expecting
to call a function with no arguments.  All signallers have
requirements like this; you can't hook a function with two arguments
to a signaller expecting to not have to provide any (unless you use an
adapter, of course).  Therefore, it's important to know what type of
slot you'll be expected to provide to a given signaller.

To find out what type of slot you can connect to a signaller, you can 
look it up in the documentation, or you can look at the signaller's
declaration - which you might have to do, since there might not be 
any documentation.  Here's an example of a signaller declaration you
might see in the GTK-- headers:

<tscreen><verb>
Gtk::EmitProxySignal1&lt;gint,GtkDirectionType,
                 CppObjectType,BaseObjectType,3,&amp;gtk_container_focus&gt; focus;
</verb></tscreen>

This looks rather a mess; but fortunately, you can ignore most all of
it.  Other than the signaller's name (<tt/focus/), two things are
important to note here: the number following the word <tt/Signal/ at
the beginning (1, in this case), and the first two types in the list
(<tt/gint/ and <tt/GtkDirectionType/).  The number indicates how many
arguments the signaller will be giving out; the first type, <tt/gint/,
is the type that the callback ought to return; and the next type,
<tt/GtkDirectionType/, is the type of this signaller's single
argument.  Don't let the remaining types (<tt/CppObjectType/,
<tt/BaseObjectType/, etc.) worry you; they're used internally by
GTK--.

The same principles apply for signallers which send out more
arguments.  Here's one that sends three (taken from
<tt/&lt;gtk--/editable.h&gt;/):

<tscreen><verb>
Gtk::EmitProxySignal3<void,const gchar*,gint,gint*,
       CppObjectType,BaseObjectType,2,&amp;gtk_editable_insert_text> insert_text;
</verb></tscreen>

Granted, it looks even scarier, but it follows the same form.  The
number 3 at the end of the type's name indicates that our callback
will need three arguments.  The first type in the type list is
<tt/void/, so that should be our callback's return type.  The
following three types should be the argument types, in order, of our
callback.  Our callback function's prototype could look like this:

<tscreen><verb>
void insert_text_cb(const gchar* foo, gint bar, gint* foobar);
</verb></tscreen>

The names of the formal parameters aren't important at all, because
the signaller will never see them.  Of course, it <em>is</em>
important to know what <tt/insert_text/ will be sending you in those
argument, but for that you'll have to consult the documentation.

<!-- -----------------------------------------------------------------
--> <sect1>Overriding Member Functions 

<p>
(Note: In the following section, we often use the terms "function",
"member function" and "method" interchangeably.  If we refer to a
"function" which belongs to a class, we really mean "member function"
or "method".  We apologise for any confusion.)

So far, we've been telling you that the way to perform actions in
response to button-presses and the like is to watch for signals.
That's certainly a good way to do things, and the only practical way
to do it in straight GTK+.  In GTK--, though, it's not the <em/only/
way.

With GTK--, instead of laboriously connecting callbacks to signals,
you can simply make a new class which inherits from a widget - say, a
button - and then override one of its member functions, such as the
one that gets called when somebody presses the button.  This can be a
lot simpler than hooking up signals for each thing you want to do.
You can do this in the straight-C world of GTK+ too; that's what GTK's
object system is for.  But in GTK+, you have to go through some
complicated procedures to get object-oriented features like
inheritance and overloading.  In C++, it's simple, since those
features are supported in the language itself; you can let the
compiler do the dirty work.

This is one of the places where the beauty of C++ really comes out.
One wouldn't think of subclassing a GTK+ widget simply to override its
action method; it's just too much trouble.  In GTK+, you almost always
use signals to get things done, unless you're writing a new widget.
But because overriding methods is so easy in C++, it's entirely
practical - and sensible - to subclass a button for that purpose.

Overriding functions this way has a number of advantages.  The first
is that you can get a lot done with only a few lines of code; in some
situations, you might find that you never need to use signals at all.
It also means that you'll rarely have to write an entirely new widget -
you can almost always do what you want by subclassing.  And because
overriding is so simple, you're a lot less likely to make the sort of
silly mistakes you'll probably make if you're writing a new GTK+
widget in C.  Also, overriding will usually result in reduced
execution time; emitting and distributing signals takes longer than
simply calling a virtual function.

So why did we spend all that time explaining signals?  There are two
main reasons why you need to be familiar with them.  Firstly, the
GTK-- library relies on them.  Even if you don't use them much
yourself, you need to understand what's going on when you see signals
used.  Secondly, subclassing isn't always the best way to accomplish
things.  Making a new class adds a symbol to your program, and that
requires additional resources; and when you subclass a widget, its
actions are fixed at compile time - signals allow you to change the
behaviour of objects as the program is running.  You can also make a
signal easily affect multiple unrelated objects; overriding is not
meant for that purpose.  The power of signals shouldn't be ignored;
you need them in your toolbox to be truly effective in GTK--.

To summarise: both techniques are valid tools for different
situations.  Which one you tend to use will help define your personal
GTK-- programming style, but you need to know about them both.

GTK-- classes are designed with overriding in mind; they contain
virtual member functions specifically intended to be overridden.
These functions have <tt/_impl/ at the end of their names.  All of the
signals in GTK-- classes have corresponding overridable methods.
<tt/_impl/ methods are declared <tt/protected/, so they can only be
used by derived classes.

Let's look at an example of overriding.  Here is the first example
from the section on signals, rewritten to use overriding:

<tscreen><verb>
#include <gtk--/button.h>

class OverriddenButton : public Gtk::Button
{
protected:
    virtual void clicked_impl();
}

void OverriddenButton::clicked_impl()
{
    cout << "Hello World" << endl;
// call the parent's version of the function
    Gtk::Button::clicked_impl();
}

main()
{
    OverriddenButton button("Hello World");
}
</verb></tscreen>

Here, instead of making a <tt/Gtk::Button/ object, which we then
connect a signal to, we define a new class called
<tt/OverriddenButton/, which <em/inherits/ from <tt/Gtk::Button/.  The
only thing we change is the <tt/clicked_impl/ function, which is
called whenever <tt/Gtk::Button/ emits the <tt/clicked/ signal.  We
define this function to print "Hello World" to <tt/stdout/, and then
we call the original, overridden function, to let <tt/Gtk::Button/ do
what it would have done had we not overridden the <tt/clicked/
function.  Note that we declared our personal <tt/clicked_impl/
function to be <tt/protected/; you should always do this when you
override a <tt/protected/ function.

You don't always have to call the parent's function; there are times
when you might not want to.  Note that we called the parent function
<em/after/ writing "Hello World", but we could have called it before.
In this simple example, it hardly matters much, but there are times
when it will.  With signals, it's not quite so easy to change details
like this, and you can do something here which you can't do at all
with signals: you can call the parent function in the <em/middle/ of
your custom code.

On the other hand, to do this we had to make a new class, and this
particular example came out a little longer, line-by-line, than the
original.  There are situations, though, where this extra work of
making a new class can be extremely helpful.  You might, for example,
make a button which you want, say, 16 of, and you might want all of
them to do roughly the same thing when pressed.  Using signals, you'd
have to manually create each button and then connect each one to a
callback of some sort.  If you make a new class, though, you don't
have to connect anything - you can make each button behave correctly
as soon as it's created.  Not only that, you can make a new
constructor for your class which takes extra parameters; you could
then use these values to affect the behaviour of each object.  The
possibilities are virtually limitless.

We noted that the <tt/clicked_impl/ function is provided by the
<tt/Gtk::Button/ widget for the <tt/clicked/ signal; this same
principle holds for all signals that a widget supports.
<tt/Gtk::Button/ gives you <tt/_impl/ functions called
<tt/pressed_impl/, <tt/released_impl/, <tt/clicked_impl/,
<tt/enter_impl/, and <tt/leave_impl/ (these all correspond to
<tt/Gtk::Button/ signals; see the chapter on buttons for details).  It
also inherits some <tt/_impl/ functions from <tt/Gtk::Widget/, which
are, of course, available to subclasses of <tt/Gtk::Button/.

<!-- ----------------------------------------------------------------- -->
<sect1>Hello World in GTK--
<p>
We've now learned enough to look at a <em/real/ example, instead of
the silly stuff we've been analysing.  In accordance with an ancient
tradition of computer science, we now introduce Hello World, a la GTK--:

example_incl_src(helloworld/helloworld.cc)

Try to compile and run it before going on.

Pretty thrilling, eh?  Let's examine the code.  First, the
<tt/HelloWorld/ class:

<tscreen><verb>
class HelloWorld : public Gtk::Window
{
  Gtk::Button m_button;

public:
  HelloWorld();
  
  void hello();
  virtual int delete_event_impl(GdkEventAny *event);
  
};
</verb></tscreen>

This class implements the "Hello World" window.  It's derived from
<tt/Gtk::Window/, and has a single <tt/Gtk::Button/ as a member.
We'll be using two signals, and we also override the <tt/_impl/ method
for the widget's <tt/delete_event/ signal.  We've chosen to use the
constructor to do all of the initialisation work for the window,
including setting up the signals.  Here it is, with the comments
omitted:

<tscreen><verb>
HelloWorld::HelloWorld()
  : Gtk::Window(GTK_WINDOW_TOPLEVEL),
    m_button("Hello World")
{
  set_border_width(10);

  destroy.connect(slot(&amp;destroy_handler));
  m_button.clicked.connect(slot(this, &amp;HelloWorld::hello));
  m_button.clicked.connect(destroy.slot());

  add(m_button);
  m_button.show();
  show();
}
</verb></tscreen>

We've placed two initialiser statements in the declaration.  The first
one provides an argument to our parent's constructor; the argument
we've provided here simply tells GTK-- to make us a full-fledged
application window, instead of a transient dialog window or something
of that sort.  The next initialiser initialises our <tt/m_button/
object; we give it the label "Hello World".

Next we run the window's <tt/set_border_width()/ method.  This sets
the amount of space between the sides of the window and the widget it
contains (windows can only contain a single widget, but this isn't
really a limitation, as you'll see later on).

Then we hook up some signals.  We need to handle three signals:
<tt/delete_event/ and <tt/destroy/ for the window, and <tt/clicked/
for the button.  <tt/delete_event/ and <tt/destroy/ are defined in
<tt/Gtk::Widget/, and are therefore common to all GTK-- widgets.
(<tt/delete_event/ is one of a special class of signals which
correspond to X events.  We talk about those in Chapter 2.)  A window
receives a <tt/delete_event/ when someone clicks its close box; when
it receives a <tt/destroy/, it disappears (the way this mechanism
works is explained below).

First we hook up the <tt/destroy/ signal to a callback,
<tt/destroy_handler()/, which looks like this:

<tscreen><verb>
void destroy_handler()
{
  Gtk::Main::quit();
}
</verb></tscreen>

When <tt/destroy_handler()/ is called, it invokes
<tt/Gtk::Main::quit()/, which, surprisingly enough, quits the
program.  This is what we want; when our only window disappears, we
shouldn't keep running.

We next hook up two callbacks to <tt/m_button/'s <tt/clicked/ signal.
The first of these runs one of our member functions, <tt/hello()/,
which prints our friendly greeting to <tt/stdout/.  The other one may
be a bit odd-looking at first, because we didn't use <tt/slot()/ here;
instead, we used a member function of <tt/destroy/ called
<tt/slot()/.  This function, which is part of every libsigc++
<tt/Signal/ object, returns a slot which, when called, will emit the
signal it came from.  So, when the <tt/clicked/ signal occurs, and it
calls this slot, that slot will emit the <tt/destroy/ signal, which
will call the <tt/destroy_handler()/ function.  (You can chain signals
up this way as much as you please.)  Therefore, clicking the button
causes the program to quit.

Next, we use the window's <tt/add()/ method to put <tt/m_button/ in
the window.  (<tt/add()/ comes from <tt/Gtk::Container/, which is
described in the chapter on container widgets.)  The <tt/add()/ method
places the widget you give it in the window, but it doesn't display
the widget.  GTK-- widgets are always invisible when you create them;
to get them to display, you must call their <tt/show()/ method, which
is what we do in the next line.  The same principle applies to the
window itself (remember, it's a widget too), so we <tt/show()/ it
next.  We could also have used the window's <tt/show_all()/ method,
instead of the two calls to <tt/show()/.  <tt/show_all()/ shows not
only the widget, but all of the widgets it contains.

That's it for the constructor.  Now what about the <tt/_impl/ function
we overrode?  We did this to handle the window's <tt/delete_event/
signal, as we mentioned earlier.  X doesn't destroy windows when it
sends them a <tt/delete/ event; in fact, it doesn't do anything to
them except send them the event.  This is useful when you have, for
example, a window that contains a document; catching the <tt/delete/
event allows you to ask the user whether he or she wants to save the
document, if it hasn't been.

GTK-- handles this event specially.  A signal handler for
<tt/delete_event/ is expected to return <tt/false/ if the window
should really be destroyed, and <tt/true/ if it should stick around.
To demonstrate this, we return <tt/true/ from our
<tt/delete_event_impl()/:

<tscreen><verb>
int HelloWorld::delete_event_impl(GdkEventAny *event)
{
  cout << "delete event occured" << endl;
  return true;
}
</verb></tscreen>

Therefore, when you click the window's close box, it doesn't go away;
it merely prints "delete event occured" on <tt/stdout/.  Had we
returned <tt/false/ instead, GTK-- would cause our window to emit the
<tt/destroy/ signal, and the program would quit.

Incidentally, this is one situation where it's perhaps a little bit
silly to use signals.  This is a case of a widget catching one of its
own signals; instead of bothering to hook up a signal we'd be sending
to ourselves, we override the <tt/_impl/ method.  This makes sense,
because we're subclassing <tt/Gtk::Window/ anyway.

Now let's look at our program's <tt/main()/ function.  Here it is,
sans comments:

<tscreen><verb>
int main (int argc, char *argv[])
{
  Gtk::Main kit(argc, argv);

  HelloWorld helloworld;

  kit.run();
  return 0;
}
</verb></tscreen>

First we instantiate an object called <tt/kit/; this is of type
<tt/Gtk::Main/.  Every GTK-- program must have one of these.  We pass
our command-line arguments to its constructor; it takes the arguments
it wants, and leaves you the rest, as we described earlier.

Next we make an object of our <tt/HelloWorld/ class, whose constructor
takes no arguments.  At this point our window is on the screen; it
only remains to invoke the <tt/run()/ method of our <tt/Gtk::Main/
object.  This starts up the <em/event loop/; every GTK-- program has
one of these too.  During the event loop, GTK-- idles, waiting for
events to come in from the X server.  When it gets them, it does with
them whatever is appropriate.

Note that if you interrupt the event loop, or don't run it, your
program will effectively be "hung", because it won't be able to
respond to events.  The same can happen if, when you receive a signal,
you execute an operation that takes a really long time.  You've
probably seen X programs hang like this.  GTK-- does provide a way for
you to give time to the event loop while you're busy doing something;
you can also use threads (but that's <em/way/ beyond the scope of this
chapter).

The <tt/kit.run()/ call will return when we invoke the
<tt/Gtk::Main::quit()/ function.  At that point we can consider
ourselves finished, so we return from <tt/main()/ with a successful
result code, having successfully spread a little cheer to the world.

If you understood all that, then congratulations: you are now advanced
to Journeyman Class, and may proceed to the next chapter.  Add four to
your maximum hit-points.  Go forth and code!
