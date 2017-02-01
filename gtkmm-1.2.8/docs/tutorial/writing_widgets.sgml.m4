<sect>Writing Your Own Widgets 
<!-- ***************************************************************** -->

<!-- ----------------------------------------------------------------- -->
<sect1> Overview
<p>
Although the Gtk-- distribution comes with many types of widgets that
should cover most basic needs, there may come a time when you need to
create your own new widget type. Since Gtk-- uses widget inheritance
extensively, and there is already a widget that is close to what you want,
it is often possible to make a useful new widget type in
just a few lines of code. But before starting work on a new widget, check
around first to make sure that someone has not already written
it. This will prevent duplication of effort and keep the number of
Gtk-- widgets out there to a minimum, which will help keep both the code
and the interface of different applications consistent. As a flip side
to this, once you finish your widget, announce it to the world so
other people can benefit. The best place to do this is probably the
<tt>gtk-list</tt>.

Complete sources for the example widgets are available at the place you 
got this tutorial, or from:

<htmlurl url="http://www.gtk.org/~otaylor/gtk/tutorial/"
name="http://www.gtk.org/~otaylor/gtk/tutorial/">


<!-- ----------------------------------------------------------------- -->
<sect1> The Anatomy Of A Widget
<p>
In order to create a new widget, it is important to have an
understanding of how Gtk-- objects work. This section is just meant as a
brief overview. See the reference documentation for the details. 

Gtk-- widgets are implemented in an object oriented fashion. However,
they are implemented in standard C. This greatly improves portability
and stability over using current generation C++ compilers; however,
it does mean that the widget writer has to pay attention to some of
the implementation details. The information common to all instances of
one class of widgets (e.g., to all Button widgets) is stored in the 
<em>class structure</em>. There is only one copy of this in
which is stored information about the class's signals
(which act like virtual functions in C). To support inheritance, the
first field in the class structure must be a copy of the parent's
class structure. The declaration of the class structure of GtkButtton
looks like:

<tscreen><verb>
struct _GtkButtonClass
{
  GtkContainerClass parent_class;

  void (* pressed)  (GtkButton *button);
  void (* released) (GtkButton *button);
  void (* clicked)  (GtkButton *button);
  void (* enter)    (GtkButton *button);
  void (* leave)    (GtkButton *button);
};
</verb></tscreen>

When a button is treated as a container (for instance, when it is
resized), its class structure can be cast to GtkContainerClass, and
the relevant fields used to handle the signals.

There is also a structure for each widget that is created on a
per-instance basis. This structure has fields to store information that
is different for each instance of the widget. We'll call this
structure the <em>object structure</em>. For the Button class, it looks
like:

<tscreen><verb>
struct _GtkButton
{
  GtkContainer container;

  GtkWidget *child;

  guint in_button : 1;
  guint button_down : 1;
};
</verb></tscreen>

Note that, similar to the class structure, the first field is the
object structure of the parent class, so that this structure can be
cast to the parent class's object structure as needed.

<!-- ----------------------------------------------------------------- -->
<sect1> Creating a Composite widget

<!-- ----------------------------------------------------------------- -->
<sect2> Introduction
<p>
One type of widget that you may be interested in creating is a
widget that is merely an aggregate of other Gtk-- widgets. This type of
widget does nothing that couldn't be done without creating new
widgets, but provides a convenient way of packaging user interface
elements for reuse. The FileSelection and ColorSelection widgets in
the standard distribution are examples of this type of widget.

The example widget that we'll create in this section is the Tictactoe
widget, a 3x3 array of toggle buttons which triggers a signal when all
three buttons in a row, column, or on one of the diagonals are
depressed. 

<!-- ----------------------------------------------------------------- -->
<sect2> Choosing a parent class
<p>
The parent class for a composite widget is typically the container
class that holds all of the elements of the composite widget. For
example, the parent class of the FileSelection widget is the
Dialog class. Since our buttons will be arranged in a table, it
might seem natural to make our parent class the GtkTable
class. Unfortunately, this turns out not to work. The creation of a
widget is divided among two functions - a <tt/WIDGETNAME_new()/
function that the user calls, and a <tt/WIDGETNAME_init()/ function
which does the basic work of initializing the widget which is
independent of the arguments passed to the <tt/_new()/
function. Descendent widgets only call the <tt/_init/ function of
their parent widget. But this division of labor doesn't work well for
tables, which when created, need to know the number of rows and
columns in the table. Unless we want to duplicate most of the
functionality of <tt/gtk_table_new()/ in our Tictactoe widget, we had
best avoid deriving it from GtkTable. For that reason, we derive it
from GtkVBox instead, and stick our table inside the VBox.

<!-- ----------------------------------------------------------------- -->
<sect2> The header file
<p>
Each widget class has a header file which declares the object and
class structures for that widget, along with public functions. 
A couple of features are worth pointing out. To prevent duplicate
definitions, we wrap the entire header file in:

<tscreen><verb>
#ifndef __TICTACTOE_H__
#define __TICTACTOE_H__
.
.
.
#endif /* __TICTACTOE_H__ */
</verb></tscreen>

And to keep C++ programs that include the header file happy, in:

<tscreen><verb>
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
.
.
.
#ifdef __cplusplus
}
#endif /* __cplusplus */
</verb></tscreen>

Along with the functions and structures, we declare three standard
macros in our header file, <tt/TICTACTOE(obj)/,
<tt/TICTACTOE_CLASS(klass)/, and <tt/IS_TICTACTOE(obj)/, which cast a
pointer into a pointer to the object or class structure, and check
if an object is a Tictactoe widget respectively.

Here is the complete header file:

<tscreen><verb>
/* tictactoe.h */

#ifndef __TICTACTOE_H__
#define __TICTACTOE_H__

#include <gdk/gdk.h>
#include <gtk/gtkvbox.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define TICTACTOE(obj)          GTK_CHECK_CAST (obj, tictactoe_get_type (), Tictactoe)
#define TICTACTOE_CLASS(klass)  GTK_CHECK_CLASS_CAST (klass, tictactoe_get_type (), TictactoeClass)
#define IS_TICTACTOE(obj)       GTK_CHECK_TYPE (obj, tictactoe_get_type ())


typedef struct _Tictactoe       Tictactoe;
typedef struct _TictactoeClass  TictactoeClass;

struct _Tictactoe
{
  GtkVBox vbox;
  
  GtkWidget *buttons[3][3];
};

struct _TictactoeClass
{
  GtkVBoxClass parent_class;

  void (* tictactoe) (Tictactoe *ttt);
};

guint          tictactoe_get_type        (void);
GtkWidget*     tictactoe_new             (void);
void	       tictactoe_clear           (Tictactoe *ttt);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __TICTACTOE_H__ */

</verb></tscreen>

<!-- ----------------------------------------------------------------- -->
<sect2> The <tt/_get_type()/ function.
<p>
We now continue on to the implementation of our widget. A core
function for every widget is the function
<tt/WIDGETNAME_get_type()/. This function, when first called, tells
Gtk-- about the widget class, and gets an ID that uniquely identifies
the widget class. Upon subsequent calls, it just returns the ID.

<tscreen><verb>
guint
tictactoe_get_type ()
{
  static guint ttt_type = 0;

  if (!ttt_type)
    {
      GtkTypeInfo ttt_info =
      {
	"Tictactoe",
	sizeof (Tictactoe),
	sizeof (TictactoeClass),
	(GtkClassInitFunc) tictactoe_class_init,
	(GtkObjectInitFunc) tictactoe_init,
	(GtkArgSetFunc) NULL,
        (GtkArgGetFunc) NULL
      };

      ttt_type = gtk_type_unique (gtk_vbox_get_type (), &amp;ttt_info);
    }

  return ttt_type;
}
</verb></tscreen>

The GtkTypeInfo structure has the following definition:

<tscreen><verb>
struct _GtkTypeInfo
{
  gchar *type_name;
  guint object_size;
  guint class_size;
  GtkClassInitFunc class_init_func;
  GtkObjectInitFunc object_init_func;
  GtkArgSetFunc arg_set_func;
  GtkArgGetFunc arg_get_func;
};
</verb></tscreen>

The fields of this structure are pretty self-explanatory. We'll ignore
the <tt/arg_set_func/ and <tt/arg_get_func/ fields here: they have an important, 
but as yet largely
unimplemented, role in allowing widget options to be conveniently set
from interpreted languages. Once Gtk-- has a correctly filled in copy of
this structure, it knows how to create objects of a particular widget
type. 

<!-- ----------------------------------------------------------------- -->
<sect2> The <tt/_class_init()/ function
<p>
The <tt/WIDGETNAME_class_init()/ function initializes the fields of
the widget's class structure, and sets up any signals for the
class. For our Tictactoe widget it looks like:

<tscreen><verb>

enum {
  TICTACTOE_SIGNAL,
  LAST_SIGNAL
};

static gint tictactoe_signals[LAST_SIGNAL] = { 0 };

static void
tictactoe_class_init (TictactoeClass *class)
{
  GtkObjectClass *object_class;

  object_class = (GtkObjectClass*) class;
  
  tictactoe_signals[TICTACTOE_SIGNAL] = gtk_signal_new ("tictactoe",
					 GTK_RUN_FIRST,
					 object_class->type,
					 GTK_SIGNAL_OFFSET (TictactoeClass, tictactoe),
					 gtk_signal_default_marshaller, GTK_TYPE_NONE, 0);


  gtk_object_class_add_signals (object_class, tictactoe_signals, LAST_SIGNAL);

  class->tictactoe = NULL;
}
</verb></tscreen>

Our widget has just one signal, the <tt/tictactoe/ signal that is
invoked when a row, column, or diagonal is completely filled in. Not
every composite widget needs signals, so if you are reading this for
the first time, you may want to skip to the next section now, as
things are going to get a bit complicated.

The function:

<tscreen><verb>
gint gtk_signal_new( const gchar         *name,
                     GtkSignalRunType     run_type,
                     GtkType              object_type,
                     gint                 function_offset,
                     GtkSignalMarshaller  marshaller,
                     GtkType              return_val,
                     guint                nparams,
                     ...);
</verb></tscreen>

Creates a new signal. The parameters are:

<itemize>
<item> <tt/name/: The name of the signal.
<item> <tt/run_type/: Whether the default handler runs before or after
user handlers. Usually this will be <tt/GTK_RUN_FIRST/, or <tt/GTK_RUN_LAST/,
although there are other possibilities.
<item> <tt/object_type/: The ID of the object that this signal applies
to. (It will also apply to that objects descendents)
<item> <tt/function_offset/: The offset within the class structure of
a pointer to the default handler.
<item> <tt/marshaller/: A function that is used to invoke the signal
handler. For signal handlers that have no arguments other than the
object that emitted the signal and user data, we can use the
pre-supplied marshaller function <tt/gtk_signal_default_marshaller/.
<item> <tt/return_val/: The type of the return val.
<item> <tt/nparams/: The number of parameters of the signal handler
(other than the two default ones mentioned above)
<item> <tt/.../: The types of the parameters.
</itemize>

When specifying types, the <tt/GtkType/ enumeration is used:

<tscreen><verb>
typedef enum
{
  GTK_TYPE_INVALID,
  GTK_TYPE_NONE,
  GTK_TYPE_CHAR,
  GTK_TYPE_BOOL,
  GTK_TYPE_INT,
  GTK_TYPE_UINT,
  GTK_TYPE_LONG,
  GTK_TYPE_ULONG,
  GTK_TYPE_FLOAT,
  GTK_TYPE_DOUBLE,
  GTK_TYPE_STRING,
  GTK_TYPE_ENUM,
  GTK_TYPE_FLAGS,
  GTK_TYPE_BOXED,
  GTK_TYPE_FOREIGN,
  GTK_TYPE_CALLBACK,
  GTK_TYPE_ARGS,

  GTK_TYPE_POINTER,

  /* it'd be great if the next two could be removed eventually */
  GTK_TYPE_SIGNAL,
  GTK_TYPE_C_CALLBACK,

  GTK_TYPE_OBJECT

} GtkFundamentalType;
</verb></tscreen>

<tt/gtk_signal_new()/ returns a unique integer identifier for the
signal, that we store in the <tt/tictactoe_signals/ array, which we
index using an enumeration. (Conventionally, the enumeration elements
are the signal name, uppercased, but here there would be a conflict
with the <tt/TICTACTOE()/ macro, so we called it <tt/TICTACTOE_SIGNAL/
instead.

After creating our signals, we need to tell Gtk-- to associate our
signals with the Tictactoe class. We do that by calling
<tt/gtk_object_class_add_signals()/. We then set the pointer which
points to the default handler for the `tictactoe' signal to NULL,
indicating that there is no default action.

<!-- ----------------------------------------------------------------- -->
<sect2> The <tt/_init()/ function.
<p>
Each widget class also needs a function to initialize the object
structure. Usually, this function has the fairly limited role of
setting the fields of the structure to default values. For composite
widgets, however, this function also creates the component widgets.

<tscreen><verb>
static void
tictactoe_init (Tictactoe *ttt)
{
  GtkWidget *table;
  gint i,j;
  
  table = gtk_table_new (3, 3, TRUE);
  gtk_container_add (GTK_CONTAINER(ttt), table);
  gtk_widget_show (table);

  for (i=0;i<3; i++)
    for (j=0;j<3; j++)
      {
	ttt->buttons[i][j] = gtk_toggle_button_new ();
	gtk_table_attach_defaults (GTK_TABLE(table), ttt->buttons[i][j], 
				   i, i+1, j, j+1);
	gtk_signal_connect (GTK_OBJECT (ttt->buttons[i][j]), "toggled",
			    GTK_SIGNAL_FUNC (tictactoe_toggle), ttt);
	gtk_widget_set_usize (ttt->buttons[i][j], 20, 20);
	gtk_widget_show (ttt->buttons[i][j]);
      }
}
</verb></tscreen>

<!-- ----------------------------------------------------------------- -->
<sect2> And the rest...
<p>
There is one more function that every widget (except for base widget
types like GtkBin that cannot be instantiated) needs to have - the
function that the user calls to create an object of that type. This is
conventionally called <tt/WIDGETNAME_new()/. In some
widgets, though not for the Tictactoe widgets, this function takes
arguments, and does some setup based on the arguments. The other two
functions are specific to the Tictactoe widget. 

<tt/tictactoe_clear()/ is a public function that resets all the
buttons in the widget to the up position. Note the use of
<tt/gtk_signal_handler_block_by_data()/ to keep our signal handler for
button toggles from being triggered unnecessarily.

<tt/tictactoe_toggle()/ is the signal handler that is invoked when the
user clicks on a button. It checks to see if there are any winning
combinations that involve the toggled button, and if so, emits
the "tictactoe" signal.

<tscreen><verb>  
GtkWidget*
tictactoe_new ()
{
  return GTK_WIDGET ( gtk_type_new (tictactoe_get_type ()));
}

void	       
tictactoe_clear (Tictactoe *ttt)
{
  int i,j;

  for (i=0;i<3;i++)
    for (j=0;j<3;j++)
      {
	gtk_signal_handler_block_by_data (GTK_OBJECT(ttt->buttons[i][j]), ttt);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (ttt->buttons[i][j]),
				     FALSE);
	gtk_signal_handler_unblock_by_data (GTK_OBJECT(ttt->buttons[i][j]), ttt);
      }
}

static void
tictactoe_toggle (GtkWidget *widget, Tictactoe *ttt)
{
  int i,k;

  static int rwins[8][3] = { { 0, 0, 0 }, { 1, 1, 1 }, { 2, 2, 2 },
			     { 0, 1, 2 }, { 0, 1, 2 }, { 0, 1, 2 },
			     { 0, 1, 2 }, { 0, 1, 2 } };
  static int cwins[8][3] = { { 0, 1, 2 }, { 0, 1, 2 }, { 0, 1, 2 },
			     { 0, 0, 0 }, { 1, 1, 1 }, { 2, 2, 2 },
			     { 0, 1, 2 }, { 2, 1, 0 } };

  int success, found;

  for (k=0; k<8; k++)
    {
      success = TRUE;
      found = FALSE;

      for (i=0;i<3;i++)
	{
	  success = success &amp;&amp; 
	    GTK_TOGGLE_BUTTON(ttt->buttons[rwins[k][i]][cwins[k][i]])->active;
	  found = found ||
	    ttt->buttons[rwins[k][i]][cwins[k][i]] == widget;
	}
      
      if (success &amp;&amp; found)
	{
	  gtk_signal_emit (GTK_OBJECT (ttt), 
			   tictactoe_signals[TICTACTOE_SIGNAL]);
	  break;
	}
    }
}
</verb></tscreen>

And finally, an example program using our Tictactoe widget:

<tscreen><verb>
#include <gtk/gtk.h>
#include "tictactoe.h"

/* Invoked when a row, column or diagonal is completed */
void
win (GtkWidget *widget, gpointer data)
{
  g_print ("Yay!\n");
  tictactoe_clear (TICTACTOE (widget));
}

int 
main (int argc, char *argv[])
{
  GtkWidget *window;
  GtkWidget *ttt;
  
  gtk_init (&amp;argc, &amp;argv);

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  
  gtk_window_set_title (GTK_WINDOW (window), "Aspect Frame");
  
  gtk_signal_connect (GTK_OBJECT (window), "destroy",
		      GTK_SIGNAL_FUNC (gtk_exit), NULL);
  
  gtk_container_set_border_width (GTK_CONTAINER (window), 10);

  /* Create a new Tictactoe widget */
  ttt = tictactoe_new ();
  gtk_container_add (GTK_CONTAINER (window), ttt);
  gtk_widget_show (ttt);

  /* And attach to its "tictactoe" signal */
  gtk_signal_connect (GTK_OBJECT (ttt), "tictactoe",
		      GTK_SIGNAL_FUNC (win), NULL);

  gtk_widget_show (window);
  
  gtk_main ();
  
  return 0;
}

</verb></tscreen>

<!-- ----------------------------------------------------------------- -->
<sect1> Creating a widget from scratch.

<!-- ----------------------------------------------------------------- -->
<sect2> Introduction
<p>
In this section, we'll learn more about how widgets display themselves
on the screen and interact with events. As an example of this, we'll
create an analog dial widget with a pointer that the user can drag to
set the value.

<!-- ----------------------------------------------------------------- -->
<sect2> Displaying a widget on the screen
<p>
There are several steps that are involved in displaying on the screen.
After the widget is created with a call to <tt/WIDGETNAME_new()/,
several more functions are needed:

<itemize>
<item> <tt/WIDGETNAME_realize()/ is responsible for creating an X
window for the widget if it has one.
<item> <tt/WIDGETNAME_map()/ is invoked after the user calls
<tt/gtk_widget_show()/. It is responsible for making sure the widget
is actually drawn on the screen (<em/mapped/). For a container class,
it must also make calls to <tt/map()/> functions of any child widgets.
<item> <tt/WIDGETNAME_draw()/ is invoked when <tt/gtk_widget_draw()/
is called for the widget or one of its ancestors. It makes the actual
calls to the drawing functions to draw the widget on the screen. For
container widgets, this function must make calls to
<tt/gtk_widget_draw()/ for its child widgets.
<item> <tt/WIDGETNAME_expose()/ is a handler for expose events for the
widget. It makes the necessary calls to the drawing functions to draw
the exposed portion on the screen. For container widgets, this
function must generate expose events for its child widgets which don't
have their own windows. (If they have their own windows, then X will
generate the necessary expose events)
</itemize>

You might notice that the last two functions are quite similar - each
is responsible for drawing the widget on the screen. In fact many
types of widgets don't really care about the difference between the
two. The default <tt/draw()/ function in the widget class simply
generates a synthetic expose event for the redrawn area. However, some
types of widgets can save work by distinguishing between the two
functions. For instance, if a widget has multiple X windows, then
since expose events identify the exposed window, it can redraw only
the affected window, which is not possible for calls to <tt/draw()/.

Container widgets, even if they don't care about the difference for
themselves, can't simply use the default <tt/draw()/ function because
their child widgets might care about the difference. However,
it would be wasteful to duplicate the drawing code between the two
functions. The convention is that such widgets have a function called
<tt/WIDGETNAME_paint()/ that does the actual work of drawing the
widget, that is then called by the <tt/draw()/ and <tt/expose()/
functions.

In our example approach, since the dial widget is not a container
widget, and only has a single window, we can take the simplest
approach and use the default <tt/draw()/ function and only implement
an <tt/expose()/ function.

<!-- ----------------------------------------------------------------- -->
<sect2> The origins of the Dial Widget
<p>
Just as all land animals are just variants on the first amphibian that
crawled up out of the mud, Gtk widgets tend to start off as variants
of some other, previously written widget.  Thus, although this section
is entitled `Creating a Widget from Scratch', the Dial widget really
began with the source code for the Range widget. This was picked as a
starting point because it would be nice if our Dial had the same
interface as the Scale widgets which are just specialized descendents
of the Range widget. So, though the source code is presented below in
finished form, it should not be implied that it was written, <em>deus
ex machina</em> in this fashion. Also, if you aren't yet familiar with
how scale widgets work from the application writer's point of view, it
would be a good idea to look them over before continuing.

<!-- ----------------------------------------------------------------- -->
<sect2> The Basics
<p>
Quite a bit of our widget should look pretty familiar from the
Tictactoe widget. First, we have a header file:

<tscreen><verb>
/* Gtk-- - The GIMP Toolkit
 * Copyright (C) 1995-1997 Peter Mattis, Spencer Kimball and Josh MacDonald
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef __GTK_DIAL_H__
#define __GTK_DIAL_H__

#include <gdk/gdk.h>
#include <gtk/gtkadjustment.h>
#include <gtk/gtkwidget.h>


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#define GTK_DIAL(obj)          GTK_CHECK_CAST (obj, gtk_dial_get_type (), GtkDial)
#define GTK_DIAL_CLASS(klass)  GTK_CHECK_CLASS_CAST (klass, gtk_dial_get_type (), GtkDialClass)
#define GTK_IS_DIAL(obj)       GTK_CHECK_TYPE (obj, gtk_dial_get_type ())


typedef struct _GtkDial        GtkDial;
typedef struct _GtkDialClass   GtkDialClass;

struct _GtkDial
{
  GtkWidget widget;

  /* update policy (GTK_UPDATE_[CONTINUOUS/DELAYED/DISCONTINUOUS]) */
  guint policy : 2;

  /* Button currently pressed or 0 if none */
  guint8 button;

  /* Dimensions of dial components */
  gint radius;
  gint pointer_width;

  /* ID of update timer, or 0 if none */
  guint32 timer;

  /* Current angle */
  gfloat angle;

  /* Old values from adjustment stored so we know when something changes */
  gfloat old_value;
  gfloat old_lower;
  gfloat old_upper;

  /* The adjustment object that stores the data for this dial */
  GtkAdjustment *adjustment;
};

struct _GtkDialClass
{
  GtkWidgetClass parent_class;
};


GtkWidget*     gtk_dial_new                    (GtkAdjustment *adjustment);
guint          gtk_dial_get_type               (void);
GtkAdjustment* gtk_dial_get_adjustment         (GtkDial      *dial);
void           gtk_dial_set_update_policy      (GtkDial      *dial,
						GtkUpdateType  policy);

void           gtk_dial_set_adjustment         (GtkDial      *dial,
						GtkAdjustment *adjustment);
#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __GTK_DIAL_H__ */
</verb></tscreen>

Since there is quite a bit more going on in this widget, than the last
one, we have more fields in the data structure, but otherwise things
are pretty similar.

Next, after including header files, and declaring a few constants,
we have some functions to provide information about the widget
and initialize it:

<tscreen><verb>
#include <math.h>
#include <stdio.h>
#include <gtk/gtkmain.h>
#include <gtk/gtksignal.h>

#include "gtkdial.h"

#define SCROLL_DELAY_LENGTH  300
#define DIAL_DEFAULT_SIZE 100

/* Forward declarations */

[ omitted to save space ]

/* Local data */

static GtkWidgetClass *parent_class = NULL;

guint
gtk_dial_get_type ()
{
  static guint dial_type = 0;

  if (!dial_type)
    {
      GtkTypeInfo dial_info =
      {
	"GtkDial",
	sizeof (GtkDial),
	sizeof (GtkDialClass),
	(GtkClassInitFunc) gtk_dial_class_init,
	(GtkObjectInitFunc) gtk_dial_init,
	(GtkArgSetFunc) NULL,
        (GtkArgGetFunc) NULL,
      };

      dial_type = gtk_type_unique (gtk_widget_get_type (), &amp;dial_info);
    }

  return dial_type;
}

static void
gtk_dial_class_init (GtkDialClass *class)
{
  GtkObjectClass *object_class;
  GtkWidgetClass *widget_class;

  object_class = (GtkObjectClass*) class;
  widget_class = (GtkWidgetClass*) class;

  parent_class = gtk_type_class (gtk_widget_get_type ());

  object_class->destroy = gtk_dial_destroy;

  widget_class->realize = gtk_dial_realize;
  widget_class->expose_event = gtk_dial_expose;
  widget_class->size_request = gtk_dial_size_request;
  widget_class->size_allocate = gtk_dial_size_allocate;
  widget_class->button_press_event = gtk_dial_button_press;
  widget_class->button_release_event = gtk_dial_button_release;
  widget_class->motion_notify_event = gtk_dial_motion_notify;
}

static void
gtk_dial_init (GtkDial *dial)
{
  dial->button = 0;
  dial->policy = GTK_UPDATE_CONTINUOUS;
  dial->timer = 0;
  dial->radius = 0;
  dial->pointer_width = 0;
  dial->angle = 0.0;
  dial->old_value = 0.0;
  dial->old_lower = 0.0;
  dial->old_upper = 0.0;
  dial->adjustment = NULL;
}

GtkWidget*
gtk_dial_new (GtkAdjustment *adjustment)
{
  GtkDial *dial;

  dial = gtk_type_new (gtk_dial_get_type ());

  if (!adjustment)
    adjustment = (GtkAdjustment*) gtk_adjustment_new (0.0, 0.0, 0.0, 0.0, 0.0, 0.0);

  gtk_dial_set_adjustment (dial, adjustment);

  return GTK_WIDGET (dial);
}

static void
gtk_dial_destroy (GtkObject *object)
{
  GtkDial *dial;

  g_return_if_fail (object != NULL);
  g_return_if_fail (GTK_IS_DIAL (object));

  dial = GTK_DIAL (object);

  if (dial->adjustment)
    gtk_object_unref (GTK_OBJECT (dial->adjustment));

  if (GTK_OBJECT_CLASS (parent_class)->destroy)
    (* GTK_OBJECT_CLASS (parent_class)->destroy) (object);
}
</verb></tscreen>

Note that this <tt/init()/ function does less than for the Tictactoe
widget, since this is not a composite widget, and the <tt/new()/
function does more, since it now has an argument. Also, note that when
we store a pointer to the Adjustment object, we increment its
reference count, (and correspondingly decrement when we no longer use
it) so that Gtk-- can keep track of when it can be safely destroyed.

<p>
Also, there are a few function to manipulate the widget's options:

<tscreen><verb>
GtkAdjustment*
gtk_dial_get_adjustment (GtkDial *dial)
{
  g_return_val_if_fail (dial != NULL, NULL);
  g_return_val_if_fail (GTK_IS_DIAL (dial), NULL);

  return dial->adjustment;
}

void
gtk_dial_set_update_policy (GtkDial      *dial,
			     GtkUpdateType  policy)
{
  g_return_if_fail (dial != NULL);
  g_return_if_fail (GTK_IS_DIAL (dial));

  dial->policy = policy;
}

void
gtk_dial_set_adjustment (GtkDial      *dial,
			  GtkAdjustment *adjustment)
{
  g_return_if_fail (dial != NULL);
  g_return_if_fail (GTK_IS_DIAL (dial));

  if (dial->adjustment)
    {
      gtk_signal_disconnect_by_data (GTK_OBJECT (dial->adjustment), (gpointer) dial);
      gtk_object_unref (GTK_OBJECT (dial->adjustment));
    }

  dial->adjustment = adjustment;
  gtk_object_ref (GTK_OBJECT (dial->adjustment));

  gtk_signal_connect (GTK_OBJECT (adjustment), "changed",
		      (GtkSignalFunc) gtk_dial_adjustment_changed,
		      (gpointer) dial);
  gtk_signal_connect (GTK_OBJECT (adjustment), "value_changed",
		      (GtkSignalFunc) gtk_dial_adjustment_value_changed,
		      (gpointer) dial);

  dial->old_value = adjustment->value;
  dial->old_lower = adjustment->lower;
  dial->old_upper = adjustment->upper;

  gtk_dial_update (dial);
}
</verb></tscreen>

<sect2> <tt/gtk_dial_realize()/

<p>
Now we come to some new types of functions. First, we have a function
that does the work of creating the X window. Notice that a mask is
passed to the function <tt/gdk_window_new()/ which specifies which fields of
the GdkWindowAttr structure actually have data in them (the remaining
fields will be given default values). Also worth noting is the way the
event mask of the widget is created. We call
<tt/gtk_widget_get_events()/ to retrieve the event mask that the user
has specified for this widget (with <tt/gtk_widget_set_events()/, and
add the events that we are interested in ourselves.

<p>
After creating the window, we set its style and background, and put a
pointer to the widget in the user data field of the GdkWindow. This
last step allows Gtk-- to dispatch events for this window to the correct
widget.

<tscreen><verb>
static void
gtk_dial_realize (GtkWidget *widget)
{
  GtkDial *dial;
  GdkWindowAttr attributes;
  gint attributes_mask;

  g_return_if_fail (widget != NULL);
  g_return_if_fail (GTK_IS_DIAL (widget));

  GTK_WIDGET_SET_FLAGS (widget, GTK_REALIZED);
  dial = GTK_DIAL (widget);

  attributes.x = widget->allocation.x;
  attributes.y = widget->allocation.y;
  attributes.width = widget->allocation.width;
  attributes.height = widget->allocation.height;
  attributes.wclass = GDK_INPUT_OUTPUT;
  attributes.window_type = GDK_WINDOW_CHILD;
  attributes.event_mask = gtk_widget_get_events (widget) | 
    GDK_EXPOSURE_MASK | GDK_BUTTON_PRESS_MASK | 
    GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK |
    GDK_POINTER_MOTION_HINT_MASK;
  attributes.visual = gtk_widget_get_visual (widget);
  attributes.colormap = gtk_widget_get_colormap (widget);

  attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL | GDK_WA_COLORMAP;
  widget->window = gdk_window_new (widget->parent->window, &amp;attributes, attributes_mask);

  widget->style = gtk_style_attach (widget->style, widget->window);

  gdk_window_set_user_data (widget->window, widget);

  gtk_style_set_background (widget->style, widget->window, GTK_STATE_ACTIVE);
}
</verb></tscreen>

<sect2> Size negotiation

<p>
Before the first time that the window containing a widget is
displayed, and whenever the layout of the window changes, Gtk-- asks
each child widget for its desired size. This request is handled by the
function, <tt/gtk_dial_size_request()/. Since our widget isn't a
container widget, and has no real constraints on its size, we just
return a reasonable default value.

<tscreen><verb>
static void 
gtk_dial_size_request (GtkWidget      *widget,
		       GtkRequisition *requisition)
{
  requisition->width = DIAL_DEFAULT_SIZE;
  requisition->height = DIAL_DEFAULT_SIZE;
}
</verb></tscreen>

<p>
After all the widgets have requested an ideal size, the layout of the
window is computed and each child widget is notified of its actual
size. Usually, this will at least as large as the requested size, but
if for instance, the user has resized the window, it may occasionally
be smaller than the requested size. The size notification is handled
by the function <tt/gtk_dial_size_allocate()/. Notice that as well as
computing the sizes of some component pieces for future use, this
routine also does the grunt work of moving the widgets X window into
the new position and size.

<tscreen><verb>
static void
gtk_dial_size_allocate (GtkWidget     *widget,
			GtkAllocation *allocation)
{
  GtkDial *dial;

  g_return_if_fail (widget != NULL);
  g_return_if_fail (GTK_IS_DIAL (widget));
  g_return_if_fail (allocation != NULL);

  widget->allocation = *allocation;
  if (GTK_WIDGET_REALIZED (widget))
    {
      dial = GTK_DIAL (widget);

      gdk_window_move_resize (widget->window,
			      allocation->x, allocation->y,
			      allocation->width, allocation->height);

      dial->radius = MAX(allocation->width,allocation->height) * 0.45;
      dial->pointer_width = dial->radius / 5;
    }
}
</verb></tscreen>.

<!-- ----------------------------------------------------------------- -->
<sect2> <tt/gtk_dial_expose()/

<p>
As mentioned above, all the drawing of this widget is done in the
handler for expose events. There's not much to remark on here except
the use of the function <tt/gtk_draw_polygon/ to draw the pointer with
three dimensional shading according to the colors stored in the
widget's style.

<tscreen><verb>
static gint
gtk_dial_expose (GtkWidget      *widget,
		 GdkEventExpose *event)
{
  GtkDial *dial;
  GdkPoint points[3];
  gdouble s,c;
  gdouble theta;
  gint xc, yc;
  gint tick_length;
  gint i;

  g_return_val_if_fail (widget != NULL, FALSE);
  g_return_val_if_fail (GTK_IS_DIAL (widget), FALSE);
  g_return_val_if_fail (event != NULL, FALSE);

  if (event->count > 0)
    return FALSE;
  
  dial = GTK_DIAL (widget);

  gdk_window_clear_area (widget->window,
			 0, 0,
			 widget->allocation.width,
			 widget->allocation.height);

  xc = widget->allocation.width/2;
  yc = widget->allocation.height/2;

  /* Draw ticks */

  for (i=0; i<25; i++)
    {
      theta = (i*M_PI/18. - M_PI/6.);
      s = sin(theta);
      c = cos(theta);

      tick_length = (i%6 == 0) ? dial->pointer_width : dial->pointer_width/2;
      
      gdk_draw_line (widget->window,
		     widget->style->fg_gc[widget->state],
		     xc + c*(dial->radius - tick_length),
		     yc - s*(dial->radius - tick_length),
		     xc + c*dial->radius,
		     yc - s*dial->radius);
    }

  /* Draw pointer */

  s = sin(dial->angle);
  c = cos(dial->angle);


  points[0].x = xc + s*dial->pointer_width/2;
  points[0].y = yc + c*dial->pointer_width/2;
  points[1].x = xc + c*dial->radius;
  points[1].y = yc - s*dial->radius;
  points[2].x = xc - s*dial->pointer_width/2;
  points[2].y = yc - c*dial->pointer_width/2;

  gtk_draw_polygon (widget->style,
		    widget->window,
		    GTK_STATE_NORMAL,
		    GTK_SHADOW_OUT,
		    points, 3,
		    TRUE);
  
  return FALSE;
}
</verb></tscreen>

<!-- ----------------------------------------------------------------- -->
<sect2> Event handling
<p>
The rest of the widget's code handles various types of events, and
isn't too different from what would be found in many Gtk--
applications. Two types of events can occur - either the user can
click on the widget with the mouse and drag to move the pointer, or
the value of the Adjustment object can change due to some external
circumstance. 

When the user clicks on the widget, we check to see if the click was
appropriately near the pointer, and if so, store then button that the
user clicked with in the <tt/button/ field of the widget
structure, and grab all mouse events with a call to
<tt/gtk_grab_add()/. Subsequent motion of the mouse causes the
value of the control to be recomputed (by the function
<tt/gtk_dial_update_mouse/). Depending on the policy that has been
set, "value_changed" events are either generated instantly
(<tt/GTK_UPDATE_CONTINUOUS/), after a delay in a timer added with
<tt/gtk_timeout_add()/ (<tt/GTK_UPDATE_DELAYED/), or only when the
button is released (<tt/GTK_UPDATE_DISCONTINUOUS/).

<tscreen><verb>
static gint
gtk_dial_button_press (GtkWidget      *widget,
		       GdkEventButton *event)
{
  GtkDial *dial;
  gint dx, dy;
  double s, c;
  double d_parallel;
  double d_perpendicular;

  g_return_val_if_fail (widget != NULL, FALSE);
  g_return_val_if_fail (GTK_IS_DIAL (widget), FALSE);
  g_return_val_if_fail (event != NULL, FALSE);

  dial = GTK_DIAL (widget);

  /* Determine if button press was within pointer region - we 
     do this by computing the parallel and perpendicular distance of
     the point where the mouse was pressed from the line passing through
     the pointer */
  
  dx = event->x - widget->allocation.width / 2;
  dy = widget->allocation.height / 2 - event->y;
  
  s = sin(dial->angle);
  c = cos(dial->angle);
  
  d_parallel = s*dy + c*dx;
  d_perpendicular = fabs(s*dx - c*dy);
  
  if (!dial->button &amp;&amp;
      (d_perpendicular < dial->pointer_width/2) &amp;&amp;
      (d_parallel > - dial->pointer_width))
    {
      gtk_grab_add (widget);

      dial->button = event->button;

      gtk_dial_update_mouse (dial, event->x, event->y);
    }

  return FALSE;
}

static gint
gtk_dial_button_release (GtkWidget      *widget,
			  GdkEventButton *event)
{
  GtkDial *dial;

  g_return_val_if_fail (widget != NULL, FALSE);
  g_return_val_if_fail (GTK_IS_DIAL (widget), FALSE);
  g_return_val_if_fail (event != NULL, FALSE);

  dial = GTK_DIAL (widget);

  if (dial->button == event->button)
    {
      gtk_grab_remove (widget);

      dial->button = 0;

      if (dial->policy == GTK_UPDATE_DELAYED)
	gtk_timeout_remove (dial->timer);
      
      if ((dial->policy != GTK_UPDATE_CONTINUOUS) &amp;&amp;
	  (dial->old_value != dial->adjustment->value))
	gtk_signal_emit_by_name (GTK_OBJECT (dial->adjustment), "value_changed");
    }

  return FALSE;
}

static gint
gtk_dial_motion_notify (GtkWidget      *widget,
			 GdkEventMotion *event)
{
  GtkDial *dial;
  GdkModifierType mods;
  gint x, y, mask;

  g_return_val_if_fail (widget != NULL, FALSE);
  g_return_val_if_fail (GTK_IS_DIAL (widget), FALSE);
  g_return_val_if_fail (event != NULL, FALSE);

  dial = GTK_DIAL (widget);

  if (dial->button != 0)
    {
      x = event->x;
      y = event->y;

      if (event->is_hint || (event->window != widget->window))
	gdk_window_get_pointer (widget->window, &amp;x, &amp;y, &amp;mods);

      switch (dial->button)
	{
	case 1:
	  mask = GDK_BUTTON1_MASK;
	  break;
	case 2:
	  mask = GDK_BUTTON2_MASK;
	  break;
	case 3:
	  mask = GDK_BUTTON3_MASK;
	  break;
	default:
	  mask = 0;
	  break;
	}

      if (mods &amp; mask)
	gtk_dial_update_mouse (dial, x,y);
    }

  return FALSE;
}

static gint
gtk_dial_timer (GtkDial *dial)
{
  g_return_val_if_fail (dial != NULL, FALSE);
  g_return_val_if_fail (GTK_IS_DIAL (dial), FALSE);

  if (dial->policy == GTK_UPDATE_DELAYED)
    gtk_signal_emit_by_name (GTK_OBJECT (dial->adjustment), "value_changed");

  return FALSE;
}

static void
gtk_dial_update_mouse (GtkDial *dial, gint x, gint y)
{
  gint xc, yc;
  gfloat old_value;

  g_return_if_fail (dial != NULL);
  g_return_if_fail (GTK_IS_DIAL (dial));

  xc = GTK_WIDGET(dial)->allocation.width / 2;
  yc = GTK_WIDGET(dial)->allocation.height / 2;

  old_value = dial->adjustment->value;
  dial->angle = atan2(yc-y, x-xc);

  if (dial->angle < -M_PI/2.)
    dial->angle += 2*M_PI;

  if (dial->angle < -M_PI/6)
    dial->angle = -M_PI/6;

  if (dial->angle > 7.*M_PI/6.)
    dial->angle = 7.*M_PI/6.;

  dial->adjustment->value = dial->adjustment->lower + (7.*M_PI/6 - dial->angle) *
    (dial->adjustment->upper - dial->adjustment->lower) / (4.*M_PI/3.);

  if (dial->adjustment->value != old_value)
    {
      if (dial->policy == GTK_UPDATE_CONTINUOUS)
	{
	  gtk_signal_emit_by_name (GTK_OBJECT (dial->adjustment), "value_changed");
	}
      else
	{
	  gtk_widget_draw (GTK_WIDGET(dial), NULL);

	  if (dial->policy == GTK_UPDATE_DELAYED)
	    {
	      if (dial->timer)
		gtk_timeout_remove (dial->timer);

	      dial->timer = gtk_timeout_add (SCROLL_DELAY_LENGTH,
					     (GtkFunction) gtk_dial_timer,
					     (gpointer) dial);
	    }
	}
    }
}
</verb></tscreen>

Changes to the Adjustment by external means are communicated to our
widget by the `changed' and `value_changed' signals. The handlers
for these functions call <tt/gtk_dial_update()/ to validate the
arguments, compute the new pointer angle, and redraw the widget (by
calling <tt/gtk_widget_draw()/).

<tscreen><verb>
static void
gtk_dial_update (GtkDial *dial)
{
  gfloat new_value;
  
  g_return_if_fail (dial != NULL);
  g_return_if_fail (GTK_IS_DIAL (dial));

  new_value = dial->adjustment->value;
  
  if (new_value < dial->adjustment->lower)
    new_value = dial->adjustment->lower;

  if (new_value > dial->adjustment->upper)
    new_value = dial->adjustment->upper;

  if (new_value != dial->adjustment->value)
    {
      dial->adjustment->value = new_value;
      gtk_signal_emit_by_name (GTK_OBJECT (dial->adjustment), "value_changed");
    }

  dial->angle = 7.*M_PI/6. - (new_value - dial->adjustment->lower) * 4.*M_PI/3. /
    (dial->adjustment->upper - dial->adjustment->lower);

  gtk_widget_draw (GTK_WIDGET(dial), NULL);
}

static void
gtk_dial_adjustment_changed (GtkAdjustment *adjustment,
			      gpointer       data)
{
  GtkDial *dial;

  g_return_if_fail (adjustment != NULL);
  g_return_if_fail (data != NULL);

  dial = GTK_DIAL (data);

  if ((dial->old_value != adjustment->value) ||
      (dial->old_lower != adjustment->lower) ||
      (dial->old_upper != adjustment->upper))
    {
      gtk_dial_update (dial);

      dial->old_value = adjustment->value;
      dial->old_lower = adjustment->lower;
      dial->old_upper = adjustment->upper;
    }
}

static void
gtk_dial_adjustment_value_changed (GtkAdjustment *adjustment,
				    gpointer       data)
{
  GtkDial *dial;

  g_return_if_fail (adjustment != NULL);
  g_return_if_fail (data != NULL);

  dial = GTK_DIAL (data);

  if (dial->old_value != adjustment->value)
    {
      gtk_dial_update (dial);

      dial->old_value = adjustment->value;
    }
}
</verb></tscreen>

<!-- ----------------------------------------------------------------- -->
<sect2> Possible Enhancements
<p>
The Dial widget as we've described it so far runs about 670 lines of
code. Although that might sound like a fair bit, we've really
accomplished quite a bit with that much code, especially since much of
that length is headers and boilerplate. However, there are quite a few
more enhancements that could be made to this widget:

<itemize>
<item> If you try this widget out, you'll find that there is some
flashing as the pointer is dragged around. This is because the entire
widget is erased every time the pointer is moved before being
redrawn. Often, the best way to handle this problem is to draw to an
offscreen pixmap, then copy the final results onto the screen in one
step. (The ProgressBar widget draws itself in this fashion.)

<item> The user should be able to use the up and down arrow keys to
increase and decrease the value.

<item> It would be nice if the widget had buttons to increase and
decrease the value in small or large steps. Although it would be
possible to use embedded Button widgets for this, we would also like
the buttons to auto-repeat when held down, as the arrows on a
scrollbar do. Most of the code to implement this type of behavior can
be found in the GtkRange widget.

<item> The Dial widget could be made into a container widget with a
single child widget positioned at the bottom between the buttons
mentioned above. The user could then add their choice of a label or
entry widget to display the current value of the dial.

</itemize>

<!-- ----------------------------------------------------------------- -->
<sect1> Learning More

<p>
Only a small part of the many details involved in creating widgets
could be described above. If you want to write your own widgets, the
best source of examples is the Gtk-- source itself. Ask yourself some
questions about the widget you want to write: is it a Container
widget? does it have its own window? is it a modification of an
existing widget? Then find a similar widget, and start making changes.
Good luck!

<!-- ***************************************************************** -->
