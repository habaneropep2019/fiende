<sect> Miscellaneous Widgets
<!-- ***************************************************************** -->

<!-- ----------------------------------------------------------------- -->
<sect1> Labels <label id="sec_Labels">

<p> Labels are used a lot in GTK--; they're the main method of placing
text in windows. Labels emit no signals, because they do not have an
associated X window, and therefore cannot receive X events.  If you
need to catch signals or do clipping for labels, you can use an Event
Box; see <ref id="sec_EventBox" name="Event Boxes">.

The constructors for <tt/Gtk::Label/ are 

<tscreen><verb>
Gtk::Label(const nstring &amp;label = 0);
</verb></tscreen>

The sole argument is the string you wish the label to display.

<tt/nstring/ is a special GTK-- version of the C++ string, which
can also be a NULL pointer. It's internal to GTK--; all you need to
know about it is that you can pass 0 as an <tt/nstring/ argument value
whenever you would want to pass a <tt/NULL/ ptr - something you can't do
with a standard C++ string.  This is analogous to passing a NULL
pointer to a C function taking a <tt/char*/ argument:

<tscreen><verb>
void a_c_func(char *arg);
...
a_c_func(0);

void a_cxx_func(nstring &amp;arg);
...
a_cxx_func(0);
</verb></tscreen>

To change the label's text after creation, use the method:

<tscreen><verb>
void Gtk::Label::set_text(const string &amp;str);
</verb></tscreen>

where <tt/&amp;str/ is the new string.

The space needed for the new string will be automatically adjusted if
needed.  You can produce multi-line labels by putting line breaks in
the label string.

To retrieve the current string, use:

<tscreen><verb>
string Gtk::Label::get_text();
</verb></tscreen>

The label text can be justified using:

<tscreen><verb>
void Gtk::Label::set_justify(GtkJustification jtype);
</verb></tscreen>

Values for <tt/jtype/ are:
<itemize>
<item><tt/GTK_JUSTIFY_LEFT/
<item><tt/GTK_JUSTIFY_RIGHT/
<item><tt/GTK_JUSTIFY_CENTER/ (the default)
<item><tt/GTK_JUSTIFY_FILL/
</itemize>

The label widget is also capable of word-wrapping the text
automatically. This can be activated using:

<tscreen><verb>
void Gtk::Label::set_line_wrap (bool wrap);
</verb></tscreen>

If you want your label underlined, then you can set a <em/pattern/ on the
label:

<tscreen><verb>
void Gtk::Label::set_pattern (const nstring &amp;pattern);
</verb></tscreen>

The pattern argument indicates how the underlining should look. It
consists of a string of underscore and space characters. An underscore
indicates that the corresponding character in the label should be
underlined. For example, the string <verb/"__     __"/ would underline the
first two characters, and the eigth and ninth characters.

Below is a short example to illustrate these functions. This example
makes use of the Frame widget to better demonstrate the label styles.
It also gives some indication of what GTK-- developers do in their
off-hours.  (The Frame widget is explained in the <ref id="sec_Frames"
name="Frame"> section.)

example_incl_src(label/label.cc)

<!-- ----------------------------------------------------------------- -->
<sect1> Arrows <label id="sec_Arrows">
<p>
The Arrow widget draws an arrowhead, facing in a number of possible
directions and having a number of possible styles. It can be very
useful when placed on a button in many applications.

The constructor for <tt/Gtk::Arrow/ is

<tscreen><verb>
Gtk::Arrow( GtkArrowType   arrow_type,
           GtkShadowType  shadow_type );
</verb></tscreen>

and you can manipulate it with

<tscreen><verb>
void Gtk::Arrow::set(GtkArrowType   arrow_type,
                     GtkShadowType  shadow_type );
</verb></tscreen>

The <tt/arrow_type/ argument may take one of the following values:

<itemize>
<item><tt/GTK_ARROW_UP/
<item><tt/GTK_ARROW_DOWN/
<item><tt/GTK_ARROW_LEFT/
<item><tt/GTK_ARROW_RIGHT/
</itemize>

These values indicate the direction in which the arrow will
point. The <tt/shadow_type/ argument may take one of these values:

<itemize>
<item><tt/GTK_SHADOW_IN/
<item><tt/GTK_SHADOW_OUT/ (the default)
<item><tt/GTK_SHADOW_ETCHED_IN/
<item><tt/GTK_SHADOW_ETCHED_OUT/
</itemize>

Here's a brief example illustrating the use of arrows:

example_incl_src(arrow/arrow.cc)

<!-- ----------------------------------------------------------------- -->
<sect1>The Tooltips Widget <label id="sec_Tooltips">
<p>
These are the little text strings that pop up when you leave your
pointer over a button or other widget for a few seconds. They are easy
to use, so we will just explain them without giving a full example; if you
want to see them in action, take a look at the <tt/eventbox.cc/ example.

Widgets that do not receieve events (widgets that do not have their
own window) will not work with tooltips.

The constructor is extremely simple:

<tscreen><verb>
Gtk::Tooltips();
</verb></tscreen>

Once you have created a new tooltip, and the widget you wish to use it
on, simply use this call to set it:

<tscreen><verb>
void Gtk::Tooltips::set_tip(const Gtk::Widget &amp;widget,
                            const nstring &amp;tip_text,
                            const nstring &amp;tip_private );
</verb></tscreen>

The first argument is the widget you wish to have this tooltip pop up
for, and the next is the text you want to display. The last argument is a text
string that can be used as an identifier when using <tt/Gtk::TipsQuery/ to
implement context sensitive help; it should, for the present, be set to 0.

<!-- TODO: sort out what how to do the context sensitive help -->

Here's a short example:

<tscreen><verb>
Gtk::Tooltips tooltips;
Gtk::Widget button("button1");
.
.
.
tooltips.set_tip (button, "This is button 1", 0);
</verb></tscreen>

There are other calls that can be used with tooltips:

<tscreen><verb>
void Gtk::Tooltips::enable();
</verb></tscreen>

Enables a disabled set of tooltips.

<tscreen><verb>
void Gtk::Tooltips::disable();
</verb></tscreen>

Disables an enabled set of tooltips.

<tscreen><verb>
void Gtk::Tooltips::set_delay(gint delay);
</verb></tscreen>

Sets how many milliseconds you have to hold your pointer over the
widget before the tooltip will pop up. The default is 500
milliseconds (half a second).

<tscreen><verb>
void gtk_tooltips::set_colors(const Gdk_Color &amp;background,
                              const Gdk_Color &amp;foreground );
</verb></tscreen>

Sets the foreground and background color of the tooltips.

<!-- ----------------------------------------------------------------- -->
<sect1> Progress Bars <label id="sec_ProgressBar">

<p> Progress bars are used to show the status of an operation.  There
are two ways to create a progress bar: with or without an associated
<tt/Gtk::Adjustment/.  If you don't provide an adjustment, one will be
created for you:

<tscreen><verb>
Gtk::ProgressBar();
Gtk::ProgressBar(GtkAdjustment &amp;adjustment);
</verb></tscreen>

The second method has the advantage that we can use the adjustment
object to specify our own range parameters for the progress bar.

The adjustment object of a progress bar can be changed using:

<tscreen><verb>
void Gtk::Progress::set_adjustment(GtkAdjustment &amp;adjustment);
</verb></tscreen>

(<tt/Gtk::ProgressBar/ derives from <tt/Gtk::Progress/).

After creating the progress bar, you can set its value using:

<tscreen><verb>
void Gtk::Progress::bar_update(gfloat percentage);
</verb></tscreen>

where <tt/percentage/ is a number, from 0 to 1, indicating what
fraction of the bar should be filled up.

A progress bar may be set to one of four orientations using the
method

<tscreen><verb>
void Gtk::ProgressBar::set_orientation( GtkProgressBarOrientation orientation );
</verb></tscreen>

where the <tt/orientation/ argument may take one of the following
values to indicate the direction in which the progress bar should move:

<itemize>
<item><tt/GTK_PROGRESS_LEFT_TO_RIGHT/
<item><tt/GTK_PROGRESS_RIGHT_TO_LEFT/
<item><tt/GTK_PROGRESS_BOTTOM_TO_TOP/
<item><tt/GTK_PROGRESS_TOP_TO_BOTTOM/
</itemize>

When used as a measure of how far a process has progressed, the
<tt/Gtk::ProgressBar/ can be set to display its value in either a continuous
or discrete mode. In continuous mode, the progress bar is updated for
each value. In discrete mode, the progress bar is updated in a number
of discrete blocks. The number of blocks is also configurable.

The style of a progress bar can be set using the following method:

<tscreen><verb>
void Gtk::ProgressBar::set_bar_style( GtkProgressBarStyle  style );
</verb></tscreen>

The <tt/style/ parameter can take one of two values:

<itemize>
<item><tt/GTK_PROGRESS_CONTINUOUS/
<item><tt/GTK_PROGRESS_DISCRETE/
</itemize>

The number of discrete blocks can be set by calling

<tscreen><verb>
void Gtk::ProgressBar::set_discrete_blocks(guint blocks);
</verb></tscreen>

Besides indicating the amount of progress that has occured, the
progress bar can also be used to indicate that there is some activity;
this is done by placing the progress bar in <em/activity mode/.  In
this mode, the progress bar displays a small rectangle which moves
back and forth.  Activity mode is useful in situations where the
progress of an operation cannot be calculated as a value range (e.g.,
receiving a file of unknown length).

Activity mode is selected by passing a non-zero value to the following
method:

<tscreen><verb>
void Gtk::Progress::set_activity_mode(guint activity_mode);
</verb></tscreen>

The step size of the activity indicator, and the number of blocks, are
set using the following methods:

<tscreen><verb>
void Gtk::ProgressBar::set_activity_step(guint step);
void Gtk::ProgressBar::set_activity_blocks(guint blocks);
</verb></tscreen>

When in continuous mode, the progress bar can also display a
configurable text string within its trough, using the following
method:

<tscreen><verb>
void Gtk::Progress::set_format_string(const string &amp;format);
</verb></tscreen>

The <tt/format/ argument is similiar to one that would be used in a C
<tt/printf/ statement. The following directives may be used within the
format string:

<itemize>
<item> %p - percentage
<item> %v - value
<item> %l - lower range value
<item> %u - upper range value
</itemize>

You can select whether or not to display this string using this method:

<tscreen><verb>
void Gtk::Progress::set_show_text(bool show_text);
</verb></tscreen>

The <tt/show_text/ argument is a boolean true/false value. The
appearance of the text can be modified further using:

<tscreen><verb>
void Gtk::Progress::set_text_alignment(gfloat x_align,
                                       gfloat y_align);
</verb></tscreen>

The <tt/x_align/ and <tt/y_align/ arguments take a value between 0.0
and 1.0. Their value indicates the position of the text string within
the trough. Values of 0.0 for both place the string in the top
left hand corner; values of 0.5 (the default) centres the text, and
values of 1.0 place the text in the lower right hand corner.

The current text setting of a progress object can be retrieved using
the current or a specified adjustment value using the following two
methods. They return the formatted string that would be displayed
within the trough:

<tscreen><verb>
string Gtk::Progress::get_current_text();
string Gtk::Progress::get_text_from_value(gfloat value);
</verb></tscreen>

There is also another way to change the range and value of a progress
object using the following method:

<tscreen><verb>
void Gtk::Progress::configure(gfloat value,
                              gfloat min,
                              gfloat max );
</verb></tscreen>

This function provides a convenient way to set the range and value
of a progress object.

The remaining functions can be used to get and set the current value
of a progess object in various types and formats:

<tscreen><verb>
void Gtk::Progress::set_percentage(gfloat percentage);
void Gtk::Progress::set_value(gfloat value);
gfloat Gtk::Progress::get_value();
gfloat Gtk::Progress::get_current_percentage();
gfloat Gtk::Progress::get_percentage_from_value(gfloat value);
</verb></tscreen>

The last of these takes a number falling between the minimum and
maximum values of the progress object's adjustment, and returns the 
percentage to pass to <tt/set_percentage/.

Progress bars are often used with <em/idle timeouts/,
described in the section <ref id="sec_timeouts" name="Timeouts, I/O and Idle
Functions">, to give the illusion of multitasking.

Here is an example using a progress bar, updated using timeouts. This
code also shows you how to reset the progress bar.

example_incl_src(progressbar/progressbar.cc)

<!-- ----------------------------------------------------------------- -->
<sect1> Dialogs <label id="sec_Dialogs"

<p> The Dialog widget is a window with a few widgets pre-packed into
it for you.  It consists of a VBox, a horizontal separator, and an
HBox, stacked on top of each other.  The HBox is at the bottom and
comprises the <em/action area/, which generally contains buttons such
as "OK", "Cancel", etc.  The Dialog widget is useful for
displaying pop-up messages and other similar tasks. There is only one
constructor for it:

<tscreen><verb>
Gtk::Dialog();
</verb></tscreen>

You can pack a button in the action area by doing something like this:

<tscreen><verb>
Gtk::Dialog dialog;
Gtk::Button button("click me");
Gtk::HBox *dialogHBox = dialog.get_action_area();
dialogHBox->pack_start (button, true, true, 0);
button.show();
</verb></tscreen>

You can add to the VBox area by packing, for instance, a label 
into it:

<tscreen><verb>
Gtk::Label label("Dialogs are groovy");
Gtk::VBox *vbox = dialog.get_vbox();
vbox->pack_start (label, true, true, 0);
label.show();
</verb></tscreen>

If the simple functionality provided by the default vertical and
horizontal boxes in the two areas does't give you enough control for
your application, then you can simply pack another layout widget into
the boxes provided. For example, you could pack a table into the
vertical box.

<!-- ----------------------------------------------------------------- -->
<sect1> Pixmaps <label id="sec_Pixmaps">

<p> <em/Pixmaps/ are data structures that contain pictures. These pictures
can be used in various places; they're often used as icons on the
X-Windows desktop, or as cursors.  A pixmap which only has 2 colours is
called a <em/bitmap/; there are a few additional routines for handling
this common special case.

To understand pixmaps, it might help to understand a bit about how
X-windows works. Under X-windows, applications do not need to be
running on the same computer that is interacting with the
user. Instead, the various applications, called <em/clients/, all
communicate with a program which displays the graphics and handles the
keyboard and mouse. This program which interacts directly with the
user is called a <em/display server/ or <em/X server/.  Since the
communication might take place over a network, it's important to keep
some information within the X server.  Pixmaps, for example, are
stored in the memory of the X server. This means that once pixmap
values are set, they don't need to be re-transmitted over the network;
instead, a command is sent to the X server which says something like
"display pixmap number XYZ here."

Note that even if you're using GTK-- on a non-X-windows platform, you
still need to understand pixmaps; using them will keep your code
portable, and they're used in many places in GTK--.

There are several ways to create pixmaps in GTK--.  Most simply, you
can use one of the following constructors:

<tscreen><verb>
Gtk::Pixmap(Gtk::Widget &amp;w, const nstring &amp;xpmfilename);
Gtk::Pixmap(Gtk::Widget &amp;w, const gchar *const *data);
</verb></tscreen>

In both cases, the <tt/Gtk::Widget/ passed as the first argument
should be the one in which the pixmap will eventually be displayed
(e.g. a container, like a <tt/Gtk::Window/ or <tt/Gtk::Button/); it's
called the <em/reference widget/.

The first constructor builds a pixmap from an <em/XPM file/.  XPM
format is a human-readable representation of a pixmap; it is widely
used for storing pixmap data on X-windows systems.  The
<tt/xpmfilename/ parameter of the first constructor is a path to an
XPM file.

The second constructor builds the pixmap from raw XPM-format data;
the <tt/data/ parameter of the second constructor is a pointer to the
data. This data must not be freed until the reference widget is
realised (i.e., displayed).

Another, slightly more complicated, way to construct a
<tt/Gtk::Pixmap/ is to first build a <tt/Gdk_Pixmap/ structure using
routines from GDK--.  As with the first two constructors we showed
you, you can do this either with in-memory data, or from data read
from a file.  Here is how to create a pixmap from data in memory:

<tscreen><verb>
Gdk_Bitmap( Gdk_Window &amp;window,
            gchar     *data,
            gint       width,
            gint       height );
</verb></tscreen>

This routine is used to create a single-plane pixmap (2 colours). Each
bit of the data represents whether that pixel is off or on. Width and
height are in pixels. <tt/Gdk_Window/ is the current window;
pixmap resources are meaningful only in the context of the screen
where they are to be displayed.

<tscreen><verb>
Gdk_Pixmap ( GdkWindow *window,
             gchar     *data,
             gint       width,
             gint       height,
             gint       depth,
             const Gdk_Color &amp;fg,
             const Gdk_Color &amp;bg );
</verb></tscreen>

This is used to create a pixmap of the given depth (number of colours)
from the bitmap data specified. <tt/fg/ and <tt/bg/ are the foreground
and background colours to use.

<tscreen><verb>
Gdk_Pixmap( Gdk_Window   &amp;window,
            Gdk_Bitmap   &amp;mask,
            const Gdk_Color &amp;transparent_color,
            const string &amp;filename );
</verb></tscreen>

As before, <tt/filename/ is a path to an XPM file, which will be
loaded into the new pixmap structure. The mask specifies which bits of
the pixmap are to be opaque; all other bits are coloured using the
colour specified by <tt/transparent_color/.  We'll give an example of
this technique later on.

<tscreen><verb>
Gdk_Pixmap::create_from_xpm_d( const Gdk_Drawable &amp;drawable,
                               Gdk_Bitmap &amp;mask,
                               const Gdk_Color &amp;transparent_color,
                               gchar *const *data );
</verb></tscreen>

This constructor is used to create a pixmap from XPM data incorporated
into the program's code.  An example:

<tscreen><verb>
/* XPM */
static const char *const xpm_data[] = {
"16 16 3 1",
"       c None",
".      c #000000000000",
"X      c #FFFFFFFFFFFF",
"                ",
"   ......       ",
"   .XXX.X.      ",
"   .XXX.XX.     ",
"   .XXX.XXX.    ",
"   .XXX.....    ",
"   .XXXXXXX.    ",
"   .XXXXXXX.    ",
"   .XXXXXXX.    ",
"   .XXXXXXX.    ",
"   .XXXXXXX.    ",
"   .XXXXXXX.    ",
"   .XXXXXXX.    ",
"   .........    ",
"                ",
"                "};
</verb></tscreen>

If you've got a keen eye, you'll recognise the famous 'document' icon
(a dog-eared sheet of paper) in the above code.

When you're done using a pixmap and not likely to reuse it again soon,
it is a good idea to delete it. Pixmaps should be considered a
precious resource.

Once you've created a pixmap, you can display it as a GTK--
widget. The appropriate constructor is

<tscreen><verb>
Gtk::Pixmap(const Gdk_Pixmap &amp;pixmap,
            const Gdk_Bitmap &amp;mask);
</verb></tscreen>

The other pixmap widget calls are:

<tscreen><verb>
void  Gtk::Pixmap::set(const Gdk_Pixmap &amp;newpixmap,
	              const Gdk_Bitmap &amp;mask );

void  Gtk::Pixmap::get(Gdk_Pixmap &amp;pixmap,
	              Gdk_Bitmap &amp;mask);
</verb></tscreen>

<tt/Gtk::Pixmap::set/ is used to change the pixmap that the widget is
currently managing; <tt/newpixmap/ is the pixmap created using GDK.

In the following example, we make a button which is marked with a pixmap:

example_incl_src(pixmap/pixmap.cc)

We could, in the example above, have loaded the pixmap data from a
file instead.  For example, to load the pixmap 
from an XPM data file called <tt/icon0.xpm/ in the current
directory, we would have created the pixmap using:

<tscreen><verb>
Gtk::Pixmap pixmap(window, "./icon0.xpm" );
window.add(pixmap);
</verb></tscreen>

One disadvantage of using pixmaps is that the displayed object is
always rectangular, regardless of the image; sometimes you might want
to use icons that have other shapes.  This can be done using
<em/shaped windows/.  A shaped window is a pixmap where the background
pixels are transparent. This way, when the background image is
multi-coloured, we don't overwrite it with a rectangular, non-matching
border around our icon. The following example displays a picture of a 
wheelbarrow on the screen:

example_incl_src(wheelbarrow/wheelbarrow.cc)

To make the wheelbarrow image sensitive to mouse-clicks, we can
attach a callback to the <tt/button_press_event/ signal to make it do
something. The following code will make the picture sensitive to a
mouse button being pressed, which makes the application terminate:

<tscreen><verb>
window.set_events(window.get_events() | GDK_BUTTON_PRESS_MASK );
window.button_press_event.connect( slot(&amp;close_application) );
</verb></tscreen>

<!-- ----------------------------------------------------------------- -->
<sect1>Rulers <label id="sec_Rulers">

<p> Ruler widgets are used to indicate the location of the mouse
pointer in a given window; they're often found in drawing programs. A
window can have a vertical ruler spanning across the width and a
horizontal ruler spanning down the height. A small triangular
indicator on the ruler shows the exact location of the pointer
relative to the ruler.

Horizontal and vertical rulers have one constructor each:

<tscreen><verb>
Gtk::HRuler();    // horizontal ruler
Gtk::VRuler();    // vertical ruler
</verb></tscreen>

Once a ruler is created, you can define the unit of measurement. Units
of measure for rulers can be <tt/GTK_PIXELS/, <tt/GTK_INCHES/ or
<tt/GTK_CENTIMETERS/. This is set using

<tscreen><verb>
void Gtk::Ruler::set_metric(GtkMetricType metric);
</verb></tscreen>

The default measurement is <tt/GTK_PIXELS/ (note that this avoids
showing favouritism for other systems of measurement!).

<tscreen><verb>
ruler.set_metric(GTK_PIXELS);
</verb></tscreen>

Other important characteristics of a ruler are how to mark the units
of scale, and where the position indicator is initially placed. These
are set for a ruler using

<tscreen><verb>
void Gtk::Ruler::set_range(gfloat lower,
                           gfloat upper,
                           gfloat position,
                           gfloat max_size );
</verb></tscreen>

The arguments <tt/lower/ and <tt/upper/ define the extent of the
ruler, and <tt/max_size/ is the largest possible number that will be
displayed.  <tt/position/ defines the initial position of the pointer
indicator within the ruler.

A vertical ruler can be made to span an 800 pixel wide window using:

<tscreen><verb>
vruler.set_range(0, 800, 0, 800);
</verb></tscreen>

The markings displayed on the ruler will be from 0 to 800, with a
number for every 100 pixels. If instead we wanted the ruler to range
from 7 to 16, we could write

<tscreen><verb>
vruler.set_range(7, 16, 0, 20);
</verb></tscreen>

The indicator on the ruler is a small triangular mark that indicates
the position of the pointer relative to the ruler. If you want the
indicator to follow the mouse pointer, you need to connect the ruler's
<tt/motion_notify_event()/ callback to the <tt/motion_notify_event/
signal.  For example:

<tscreen><verb>
m_area.motion_notify_event.connect(m_hrule.motion_notify_event.slot());
</verb></tscreen>

This statement appears in the example below.

The following example creates a drawing area with a horizontal ruler
above it and a vertical ruler to the left of it. The size of the
drawing area is 600 pixels wide by 400 pixels high. The horizontal
ruler spans from 7 to 13 with a mark every 100 pixels, while the
vertical ruler spans from 0 to 400 with a mark every 100 pixels.
Placement of the drawing area and the rulers is done using a table.

example_incl_src(rulers/rulers.cc)

<!-- ----------------------------------------------------------------- -->
<sect1>Statusbars <label id="sec_Statusbars">
<p>
Statusbars are used to display status messages. They
keep a stack of the messages pushed onto them, so that popping the
current message will redisplay the previous text message.

To make it easier for different parts of an application to use the same
statusbar to display messages, the statusbar widget issues <em/context
identifiers/ which are used to identify different 'users'.  These
context identifiers are then be used to manipulate individual messages
on the stack.  The use of context identifiers is not optional; you
have to obtain one to display messages on a statusbar.

Gtk::Statusbar has a very simple constructor:

<tscreen><verb>
Gtk::StatusBar();
</verb></tscreen>

A new context identifier is requested using a call to the following
method.  <tt/context_description/ is a short textual description of
the context:

<tscreen><verb>
guint Gtk::StatusBar::get_context_id(const nstring &amp;context_description);
</verb></tscreen>

There are three methods that can operate on statusbars:

<tscreen><verb>
guint Gtk::StatusBar::push(guint context_id,
                           const nstring &amp;text );

void Gtk::StatusBar::pop(guint context_id );

void Gtk::StatusBar::remove(guint context_id,
                            guint message_id ); 
</verb></tscreen>

The first, <tt/Gtk::StatusBar::push()/, is used to add a new message to the
statusbar.  It returns a <tt/message identifier/, which can be passed later
to the function <tt/Gtk::StatusBar::remove/ to remove the message with the
given message and context identifiers from the statusbar's stack.
<tt/Gtk::StatusBar::pop()/ removes the message highest in the
stack having the given context identifier.

This may seem like an awfully complicated mechanism for such a seemingly
simple task, but if you've done much GUI programming, you'll quickly 
appreciate what this does for you.  GUI interfaces often have multiple
things going on at once (whether they're multi-threaded or not), and
it can sometimes be more trouble than it's worth to give each task a
way to display messages to the user.  <tt/Gtk::StatusBar/ makes it
easy (well, easier, at any rate).

The following example creates a statusbar and two buttons: one for
pushing items onto the statusbar, and one for popping the last item
back off.

example_incl_src(statusbar/statusbar.cc)

<!-- ----------------------------------------------------------------- -->
<sect1>Text Entries <label id="sec_TextEntries">

<p> Text-entry widgets allow the user to enter text (surprisingly
enough); they're called <tt/Gtk::Entry/ in GTK--.  They have two
constructors:

<tscreen><verb>
Gtk::Entry();
Gtk::Entry(guint16 max);
</verb></tscreen>

The first creates an empty text-entry widget; the second creates an
empty text-entry widget with a fixed limit on the length of its text.

There are several functions for altering the text in a <tt/Gtk::Entry/
widget:

<tscreen><verb>
void Gtk::Entry::set_text(const string &amp;text);
void Gtk::Entry::append_text(const string &amp;text);
void Gtk::Entry::prepend_text(const string &amp;text);
</verb></tscreen>

<tt/Gtk::Entry::set_text()/ replaces the current text in the entry
widget with the string in <tt/text/; <tt/Gtk::Entry::append_text()/
and <tt/Gtk::Entry::prepend_text()/ add the contents of <tt/text/ to
the end and beginning of the current text, respectively.

You can change the position of the insertion point using

<tscreen><verb>
void Gtk::Entry::set_position(gint position);
</verb></tscreen>

The way the <tt/position/ argument is interpreted is as follows.  If
the current contents of the widget are

<tscreen>
hello world
</tscreen>

and you call <tt/set_position()/ with an argument of 5, and then type
the letter X, the contents will be

<tscreen>
helloX world
</tscreen>

You can retrieve the current contents of a text-entry widget using

<tscreen><verb>
string Gtk::Entry::get_text();
</verb></tscreen>

Occasionally you might want to make a text-entry widget read-only.
This can be done by passing <tt/false/ to the following method:

<tscreen><verb>
void Gtk::Entry::set_editable(bool  editable);
</verb></tscreen>

A text-entry widget whose <tt/editable/ flag is turned off won't
respond to keypresses; however, the text in it can still be selected.

Another fairly common use for text-entry widgets is for the input of
passwords, passphrases and other information you don't want echoed
on the screen.  <tt/Gtk::Entry/ provides the <tt/visibility/ flag to
handle these cases:

<tscreen><verb>
void Gtk::Entry::set_visibility(bool  visible);
</verb></tscreen>

When the <tt/visibility/ flag is set to <tt/false/, the letters typed
into the text-entry widget show up as asterisks.  (This can be changed
on the fly, as you'll see in the example below.)

<tt/Gtk::Entry/ supports a selection region for copying text, and
other purposes.  You can change the selection region using

<tscreen><verb>
void Gtk::Entry::select_region(gint start,
                               gint end );
</verb></tscreen>

<tt/start/ and <tt/end/ are interpreted just as <tt/position/ is for
<tt/set_position()/ (see above).

You might want to be notified whenever the user types in a text entry
widget.  <tt/Gtk::Entry/ provides two signals, <tt/activate/ and
<tt/changed/, for just this purpose.  <tt/activate/ is emitted when
the user presses the enter key in a text-entry widget; <tt/changed/ is
emitted when the text in the widget changes.  You can use these to
validate or filter the text the user types, among other things.

Here is an example using <tt/Gtk::Entry/.  It provides, besides a
<tt/Gtk::Entry/ widget, two checkboxes, with which you can toggle the
editable and visible flags.

example_incl_src(entry/entry.cc)

<!-- ----------------------------------------------------------------- -->
<sect1>Spinbuttons <label id="sec_Spinbuttons">

<p> Spinbuttons allow the user to select a value from a range of
numeric values. It consists of a text entry box with up and down arrow
buttons attached to the side. Selecting one of the buttons causes the
value to 'spin' up and down across the range of possible values. The
entry box may also be used to enter a value directly.

A spinbutton's value can have an adjustable number of decimal places,
and the step size is configurable.  Spinbuttons have an 'auto-repeat'
feature as well: holding down one of the arrows can optionally cause
the value to change more quickly the longer the arrow is held down.

Spinbuttons use an <ref id="sec_Adjustment" name="Adjustment">
object to hold information about the range of values they can have.
Recall that an adjustment widget is created with the following
constructor:

<tscreen><verb>
Gtk::Adjustment( gfloat value,
                 gfloat lower,
                 gfloat upper,
                 gfloat step_increment,
                 gfloat page_increment,
                 gfloat page_size );
</verb></tscreen>

These attributes of an Adjustment are used by the Spin Button in the
following way:

<itemize>
<item> <tt/value/: initial value for the Spin Button
<item> <tt/lower/: lower range value
<item> <tt/upper/: upper range value
<item> <tt/step_increment/: value to increment/decrement when pressing
mouse button 1 on a button
<item> <tt/page_increment/: value to increment/decrement when pressing
mouse button 2 on a button
<item> <tt/page_size/: unused
</itemize>

Additionally, mouse button 3 can be used to jump directly to the
<tt/upper/ or <tt/lower/ values.

Here is the <tt/Gtk::SpinButton/ constructor:

<tscreen><verb>
Gtk::SpinButton( GtkAdjustment  &amp;adjustment,
                 gfloat         climb_rate,
                 guint          digits );
</verb></tscreen>

<tt/adjustment/ is the adjustment object that this spinbutton will
use.  The <tt/climb_rate/ argument is an acceleration factor for when
the arrows are held down; a value of 0.0 turns acceleration off, and
1.0 is the maximum.  The <tt/digits/ argument specifies the number of
decimal places in the spinbutton's value.

The adjustment object can be set and retrieved using the following
functions:

<tscreen><verb>
void Gtk::SpinButton::set_adjustment(Gtk::Adjustment&amp; adjustment);
Gtk::Adjustment* Gtk::SpinButton::get_adjustment() const;
</verb></tscreen>

The number of decimal places can be altered using:

<tscreen><verb>
void Gtk::SpinButton::set_digits(guint digits) ;
</verb></tscreen>

You can set the spinbutton's value using:

<tscreen><verb>
void Gtk::SpinButton::set_value(gfloat value);
</verb></tscreen>

The value can be retrieved as either a floating point or integer value
with the following functions:

<tscreen><verb>
gfloat Gtk::SpinButton::get_value_as_float();
gint Gtk::SpinButton::get_value_as_int();
</verb></tscreen>

The <tt/spin()/ method allows you to 'spin' the spinbutton, just as
though its arrows had been clicked.  Its prototype is

<tscreen><verb>
void Gtk::SpinButton::spin( GtkSpinType direction,
                            gfloat      increment );
</verb></tscreen>

where the <tt/direction/ parameter can take one of the following values:

<descrip>
<tag><tt/GTK_SPIN_STEP_FORWARD/, <tt/GTK_SPIN_STEP_BACKWARD/</tag>

Move the value up or down, respectively, by <tt/increment/.  If
<tt/increment/ is 0, change the value by the value of
<tt/step_increment/ in the adjustment object.

<tag><tt/GTK_SPIN_PAGE_FORWARD/, <tt/GTK_SPIN_PAGE_BACKWARD/</tag>

Change the value by <tt/increment/.

<tag><tt/GTK_SPIN_HOME/</tag>

Set the value to the minimum value specified in the adjustment.

<tag><tt/GTK_SPIN_END/</tag>

Set the value to the maximum value specified in the adjustment.

<tag><tt/GTK_SPIN_USER_DEFINED/</tag>

Change the value by <tt/increment/.

</descrip>

To prevent the user from typing non-numeric characters into the entry
box, pass <tt/true/ to

<tscreen><verb>
void Gtk::SpinButton::set_numeric(bool numeric);
</verb></tscreen>

You can cause the spinbutton to 'wrap' between its upper and lower
bounds using

<tscreen><verb>
void Gtk::SpinButton::set_wrap(bool wrap);
</verb></tscreen>

To force the spinbutton's value to be 'snapped' to the nearest
<tt/step_increment/ in the adjustment, use

<tscreen><verb>
void Gtk::SpinButton::set_snap_to_ticks(bool snap_to_ticks);
</verb></tscreen>

You can modify the update policy of a spinbutton using

<tscreen><verb>
void Gtk::SpinButton::set_update_policy(GtkSpinButtonUpdatePolicy policy);
</verb></tscreen>

where <tt/policy/ is one of <tt/GTK_UPDATE_ALWAYS/ or
<tt/GTK_UPDATE_IF_VALID/.  <tt/GTK_UPDATE_ALWAYS/ causes the
spinbutton to ignore errors encountered while converting the text in
the entry box to a numeric value.  You can use this setting to allow
the spinbutton to accept non-numeric values.  <tt/GTK_UPDATE_IF_VALID/
causes the spinbutton to reject values that do not parse correctly, or
that are outside the valid numeric range specified in the adjustment
object.

You can change the spinbutton's shadow style using

<tscreen><verb>
void Gtk::SpinButton::set_shadow_type(GtkShadowType shadow_type);
</verb></tscreen>

where <tt/shadow_type/ is one of

<itemize>
<item><tt/GTK_SHADOW_IN/
<item><tt/GTK_SHADOW_OUT/
<item><tt/GTK_SHADOW_ETCHED_IN/
<item><tt/GTK_SHADOW_ETCHED_OUT/
</itemize>

You can force an immediate update using

<tscreen><verb>
void Gtk::SpinButton::update();
</verb></tscreen>

Here's an example of a spinbutton in action:

example_incl_src(spinbutton/spinbutton.cc)

<!-- ----------------------------------------------------------------- -->
<sect1>Combo boxes <label id="sec_ComboBoxes">

<p>
A combo box is like a cross between a flip-menu and a text-entry
widget.  It consists of a text-entry box with an arrow on the side;
clicking the arrow reveals a menu of entries, and clicking on one of the
entries enters it into the entry box.  The entry box otherwise works
just like a regular text-entry widget.

The constructor for the combo box is

<tscreen><verb>
Gtk::Combo()
</verb></tscreen>

A <tt/Gtk::Combo/ contains a <tt/Gtk::Entry/ widget, which is used to
implement the entry box.  You can obtain the <tt/Gtk::Entry/ using the
member function

<tscreen><verb>
Gtk::Entry* Gtk::Combo::get_entry() const;
</verb></tscreen>

To set the values in the flip-menu, use

<tscreen><verb>
void Gtk::Combo::set_popdown_strings(const Gtk::SArray&amp; strings);
</verb></tscreen>

where <tt/strings/ is a list of the strings you want to appear in the
list.  A <tt/Gtk::SArray/ is a converter object which can take any
kind of STL vector container; this means that you can pass vectors or
lists to this function, and things will work as you expect.  For
example, the following is legal:


<tscreen><verb>
list&lt;string&gt; gl;

gl.push_back("String 1");
gl.push_back("String 2");
gl.push_back("String 3"); 
gl.push_back("String 4");

combo.set_popdown_strings(gl);
</verb></tscreen>

<tt/Gtk::Combo/ has a number of key-bindings which make the widget a
bit easier to use.  You can allow the values in the list to be
selected using the up-arrow and down-arrow keys by passing <tt/true/
to

<tscreen><verb>
void Gtk::Combo::set_use_arrows(bool val);
</verb></tscreen>

Normally, when you press (for example) the down-arrow in the
entry-box, but you're on the last item in the flip-menu, the focus
will change.  You can make the value wrap around instead by passing
<tt/true/ to

<tscreen><verb>
void Gtk::Combo::set_use_arrows_always(bool val);
</verb></tscreen>

<tt/Gtk::Combo/ also features completion; hitting ALT-TAB in the entry
box will cause the widget to attempt to complete the text in it using
the values in the flip-menu.  You can set whether the completion
search is case-sensitive or not by calling

<tscreen><verb>
void Gtk::Combo::set_case_sensitive(gint val);
</verb></tscreen>

<!-- There are also a function to set the string on a particular item, void
gtk_combo_set_item_string(GtkCombo *combo, GtkItem *item, const gchar
*item_value), but this requires that you have a pointer to the
appropriate GtkItem.  Frankly, I have no idea how to do that.
-->

<!-- ----------------------------------------------------------------- -->
<sect1> Colour Selection Widget <label id="sec_ColourSelection">

<p>
The colour selection widget is used for
interactively selecting colours. This composite widget lets the user
select a colour by manipulating RGB (Red, Green, Blue) and HSV (Hue,
Saturation, Value) triples.  This is done either by adjusting single
values with sliders or entries, or by picking the desired colour from a
hue-saturation wheel/value bar.  Optionally, the opacity of the colour
can also be set.

The colour selection widget currently emits only one signal,
<tt/color_changed/, which is emitted whenever the current colour in the
widget changes, either when the user changes it, or if it's set
explicitly through <tt/Gtk::ColorSelection::set_color()/.

The colour selection widget can be used on its own, as part of a
dialog, for example, or you can use the
<tt/Gtk::ColorSelectionDialog/, which is a complete colour-selection
dialog using <tt/Gtk::ColorSelection/.

The constructor for <tt/Gtk::ColorSelection/ looks like this:

<tscreen><verb>
Gtk::ColorSelection();
</verb></tscreen>
	
You'll probably not be using this constructor directly. It creates an
orphan <tt/Gtk::ColorSelection/ widget which you'll have to parent
yourself.

<tt/Gtk::ColorSelectionDialog/ has the following constructor:

<tscreen><verb> 
Gtk::ColorSelectionDialog(const nstring &amp;title);
</verb></tscreen>

<tt/Gtk::ColorSelectionDialog/ inherits from <tt/Gtk::Dialog/. It
consists of a <tt/Gtk::Frame/ containing a <tt/Gtk::ColorSelection/
widget, a <tt/Gtk::HSeparator/, and a <tt/Gtk::HBox/ with three
buttons in it, marked "Ok", "Cancel" and "Help". (You can obtain the
button widgets using the <tt/get_ok_button()/,
<tt/get_cancel_button()/ and <tt/get_help_button()/ methods.)

<tt/Gtk::ColorSelection/ has an adjustable update policy, which can be
set using

<tscreen><verb>
void Gtk::ColorSelection::set_update_policy(GtkUpdateType policy);
</verb></tscreen>

where <tt/policy/ is one of <tt/GTK_UPDATE_CONTINUOUS/,
<tt/GTK_UPDATE_DISCONTINUOUS/, or <tt/GTK_UPDATE_DELAYED/.  The
default policy is <tt/GTK_UPDATE_CONTINUOUS/, which means that the
current colour is updated continuously when the user drags the sliders
or presses the mouse and drags in the hue-saturation wheel or value
bar. If you experience performance problems, you may want to set the
policy to <tt/GTK_UPDATE_DISCONTINUOUS/ or <tt/GTK_UPDATE_DELAYED/.

The colour selection widget supports the adjustment of the opacity of
a colour (also known as the colour's <em/alpha channel/). This is
disabled by default; to activate this feature, pass <tt/true/ to

<tscreen><verb>
void Gtk::ColorSelection::set_opacity(gint use_opacity);
</verb></tscreen>

You can set the colour using

<tscreen><verb>
void Gtk::ColorSelection::set_color(gdouble *color);
</verb></tscreen>

where <tt/color/ is an array of <tt/gdouble/s.  The array should
contain four doubles, representing the red, green, blue, and alpha
channels, in that order.  (If opacity is not enabled, the alpha
component is ignored.)  The values of each channel must be between 0
and 1.

You can get the currently selected colour using

<tscreen><verb>
void Gtk::ColorSelection::get_color(gdouble *color);
</verb></tscreen>

<tt/color/ should be a pointer to an array of <tt/gdouble/; the function
will fill in this array with the currently selected colour.

<!-- Need to do a whole section on DnD - TRG
Drag and drop
-------------

The colour sample areas (right under the hue-saturation wheel) supports
drag and drop. The type of drag and drop is "application/x-color". The
message data consists of an array of 4 (or 5 if opacity is enabled)
gdouble values, where the value at position 0 is 0.0 (opacity on) or
1.0 (opacity off) followed by the red, green and blue values at
positions 1,2 and 3 respectively.  If opacity is enabled, the opacity
is passed in the value at position 4.
-->

divert(-1)
Here's a simple example demonstrating the use of 
<tt/Gtk::ColorSelectionDialog/. The program displays a window containing a
drawing area. Clicking on it opens a colour selection dialog, and
changing the colour in the colour selection dialog changes the
background colour of the drawing area.

<tscreen><verb>
dnl this example apparently hasn't been written yet
dnlexample_include(colorsel/colorsel.cc)
</verb></tscreen>
divert(0)
<!-- ----------------------------------------------------------------- -->
<sect1> File Selection Widget <label id="sec_FileSelection">

<p>
The file selection widget is a complete file-selection dialog.
Its constructor is:

<tscreen><verb>
Gtk::FileSelection(const nstring &amp;title);
</verb></tscreen>

The <tt/title/ will be used in the window's title bar.

To set the selected file, use

<tscreen><verb>
void Gtk::FileSelection::set_filename(const string &amp;filename);
</verb></tscreen>

This is useful for bringing the user to a specific directory, or to
set a default filename.

To get the currently selected filename, call

<tscreen><verb>
string Gtk::FileSelection::get_filename();
</verb></tscreen>

There are also accessor functions for the widgets contained within the file 
selection widget. These are:

<itemize>
<item><tt/Gtk::HBox* get_action_area();/
<item><tt/Gtk::Entry* get_selection_entry();/
<item><tt/Gtk::Label* get_selection_text();/
<item><tt/Gtk::Button* get_ok_button();/
<item><tt/Gtk::Button* get_cancel_button();/
</itemize>

You'll primarily use these for connecting signals; usually you'll
connect something to at least the OK and Cancel buttons.

Here is an example of using the file selection widget:

example_incl_src(filesel/filesel.cc)

<!-- ***************************************************************** -->
