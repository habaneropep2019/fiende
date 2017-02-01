<!-- ***************************************************************** -->
<sect>Single-item widgets <label id="sec_SingleItemWidgets">
<!-- ***************************************************************** -->

<p> In this chapter, we discuss the single-item (i.e., derived from
<tt/Gtk::Bin/) container widgets, with the exception of the Button and
Window widgets, which we've already covered.

We also discuss the <tt/Gtk::Paned/ widget, which allows you to divide
a window into two separate "panes".  This widget actually contains two
child widgets, but it doesn't maintain a list, so we've included it
here.

<!-- ----------------------------------------------------------------- -->
<sect1> Frames <label id="sec_Frames">
<p>
Frames can be used to enclose one or a group of widgets with a box
which can optionally be labelled. The position of the label and the
style of the box can be altered to suit.

The <tt/Gtk::Frame/ constructor is:

<tscreen><verb>
Gtk::Frame(const nstring &amp;label);
</verb></tscreen>

The label is by default placed in the upper left-hand corner of the
frame. A value of <tt/NULL/ for the <tt/label/ argument will result in no
label being displayed. The text of the label can be changed using:

<tscreen><verb>
void Gtk::Frame::set_label(const nstring &amp;label);
</verb></tscreen>

The position of the label can be changed using:

<tscreen><verb>
void Gtk::Frame::set_label_align(gfloat xalign,
                                 gfloat yalign);
</verb></tscreen>

<tt/xalign/ and <tt/yalign/ take values between 0.0 and
1.0. <tt/xalign/ indicates the position of the label along the top
horizontal of the frame.  A value of 0.0 will left-justify the label,
1.0 right-justifies it, and 0.5 centres it.  Values in-between also
work; for example, 0.25 will place the label a quarter of the way
across the top.  <tt/yalign/ is ignored. The default value of
<tt/xalign/ is 0.0.

You can alter the frame's appearance using:

<tscreen><verb>
void Gtk::Frame::set_shadow_type(GtkShadowType type);
</verb></tscreen>

where <tt/type/ can is one of

<itemize>
<item><tt/GTK_SHADOW_NONE/
<item><tt/GTK_SHADOW_IN/
<item><tt/GTK_SHADOW_OUT/
<item><tt/GTK_SHADOW_ETCHED_IN/ (the default)
<item><tt/GTK_SHADOW_ETCHED_OUT/
</itemize>

The following code example illustrates the use of the Frame widget.

example_incl_src(frame/frame.cc)

<!-- ----------------------------------------------------------------- -->   
<sect1> Aspect Frames <label id="sec_AspectFrames">
<p>
The aspect frame widget is like a frame widget, except that it also
enforces the <em/aspect ratio/ (the ratio of the width to the
height) of the child widget to have a certain value, adding extra
space if necessary. This is useful, for instance, if you want to
display a photograph.
  
To create a new aspect frame, use:
   
<tscreen><verb>
Gtk::AspectFrame(const string label,
                 gfloat       xalign,
                 gfloat       yalign,
                 gfloat       ratio,
                 gint         obey_child);
</verb></tscreen>

<tt/xalign/ and <tt/yalign/ specify the alignment factors.
If <tt/obey_child/ is true, the aspect ratio of a child
widget will match the aspect ratio of the ideal size it requests.
Otherwise, it is given by <tt/ratio/.
   
To change the options of an existing aspect frame, you can use:
   
<tscreen><verb>
void Gtk::AspectFrame::set(gfloat xalign,
                           gfloat yalign,
                           gfloat ratio,
                           gint   obey_child);
</verb></tscreen>
   
The following program uses a <tt/Gtk::AspectFrame/ to
present a drawing area whose aspect ratio will always be 2:1, no
matter how the user resizes the top-level window.

example_incl_src(aspectframe/aspectframe.cc)

<!-- ----------------------------------------------------------------- -->   
<sect1>The EventBox <label id="sec_EventBox">

<p> Some Gtk-- widgets don't have associated X windows; they draw on
their parents' windows.  (Of course, you should <em/not/ let your own
children do this.)  Because of this, they cannot receive events.
Also, if they are incorrectly sized, they don't clip, so you can get
messy overwriting etc. If you require more from these widgets, the
EventBox is for you.  Although the name EventBox emphasises the
event-handling function, the widget can also be used for clipping
(and more; see the example below).

The constructor for <tt/Gtk::EventBox/ is:

<tscreen><verb>
Gtk::EventBox();
</verb></tscreen>

A child widget can be added to the EventBox using:

<tscreen><verb>
event_box.add(child_widget);
</verb></tscreen>

The following example demonstrates both uses of an EventBox - a label
is created that is clipped to a small box, and set up so that a
mouse-click on the label causes the program to exit. Resizing the
window reveals varying amounts of the label.

example_incl_src(eventbox/eventbox.cc)

<!-- ----------------------------------------------------------------- -->   
<sect1>The Alignment widget <label id="sec_Alignment">

<p>
The Alignment widget allows you to place a widget within its window at
a position and size relative to the size of the Alignment widget
itself.  It's useful for centering a widget
within the window.

There is only one method associated with <tt/Gtk::Alignment/, other
than the constructor:

<tscreen><verb>
Gtk::Alignment(gfloat xalign,
               gfloat yalign,
               gfloat xscale,
               gfloat yscale);

void Gtk::Alignment::set(GtkAlignment *alignment,
                         gfloat        xalign,
                         gfloat        yalign,
                         gfloat        xscale,
                         gfloat        yscale);
</verb></tscreen>

<tt/set()/ allows the alignment parameters of an
exisiting Alignment widget to be altered.

All four alignment parameters are floating point numbers which can
range from 0.0 to 1.0. The <tt/xalign/ and <tt/yalign/ arguments
affect the position of the widget placed within the Alignment
widget. The <tt/xscale/ and <tt/yscale/ arguments affect the amount of
space allocated to the widget.

A child widget can be added to the Alignment widget using the usual
<tt/add()/ method:

<tscreen><verb>
alignment.add(child_widget);
</verb></tscreen>

For an example of using the Alignment widget, refer to the example for
the <ref id="sec_ProgressBar" name="Progress Bar"> widget.

<!-- ----------------------------------------------------------------- -->
<sect1>Viewports <label id="sec_Viewports">

<p>
A viewport widget allows you to place a larger widget within it such
that you can view a part of it at a time. It uses the ubiquitous
<ref id="sec_Adjustment" name="adjustment object"> to define the area that
is currently in view.

It's fairly unlikely that you will ever need to use the Viewport widget
directly. You are much more likely to use the <ref
id="sec_ScrolledWindows" name="Scrolled Windows"> widget, which itself
uses the Viewport, and gives you scrollbars.

Here are the <tt/Gtk::Viewport/ constructors:

<tscreen><verb>
Gtk::Viewport(Gtk::Adjustment &amp;hadjustment,
              Gtk::Adjustment &amp;vadjustment);
Gtk::Viewport();
</verb></tscreen>

You can specify the horizontal and vertical adjustment objects
that the widget is to use when you create the widget, or you can let
the viewport create its own, which it will do if you don't pass any
arguments to the constructor.

You can get and set the adjustments after the widget has been created
using the following four methods:

<tscreen><verb>
Gtk::Adjustment* Gtk::Viewport::get_hadjustment();
Gtk::Adjustment* Gtk::Viewport::get_vadjustment();
void Gtk::Viewport::set_hadjustment(GtkAdjustment &amp;adjustment);
void Gtk::Viewport::set_vadjustment(GtkAdjustment &amp;adjustment);
</verb></tscreen>

The only other viewport function is used to alter its appearance:

<tscreen><verb>
void Gtk::Viewport::set_shadow_type(GtkShadowType  type );
</verb></tscreen>

Possible values for the <tt/type/ parameter are:

<itemize>
<item><tt/GTK_SHADOW_NONE/
<item><tt/GTK_SHADOW_IN/
<item><tt/GTK_SHADOW_OUT/
<item><tt/GTK_SHADOW_ETCHED_IN/ (the default)
<item><tt/GTK_SHADOW_ETCHED_OUT/
</itemize>

<!-- ----------------------------------------------------------------- -->
<sect1>Scrolled Windows <label id="sec_ScrolledWindows">

<p>
Scrolled windows are used to create a scrollable area inside a real
window.  You can insert any type of widget into a scrolled window, and
it will be accessible regardless of its size by using the scrollbars.

<tt/Gtk::ScrolledWindow/ has the following constructors:

<tscreen><verb>
Gtk::ScrolledWindow(Gtk::Adjustment *hadjustment,
                    Gtk::Adjustment *vadjustment);
Gtk::ScrolledWindow();
</verb></tscreen>

As with the Viewport, you can supply Adjustments at creation time, or
you can let the widget supply its own by passing no arguments.

Scrolled windows have <em/scrollbar policies/ which determine whether
the scrollbars will be displayed or not; the policy can be set for
each scrollbar using:

<tscreen><verb>
void Gtk::ScrolledWindow::set_policy(GtkPolicyType hscrollbar_policy,
                                     GtkPolicyType vscrollbar_policy);
</verb></tscreen>

The policy may be one of <tt/GTK_POLICY_AUTOMATIC/ or <tt/GTK_POLICY_ALWAYS/.
<tt/GTK_POLICY_AUTOMATIC/ will cause the scrolled window to display
the scrollbar only if the contained widget is larger than the visible area;
<tt/GTK_POLICY_ALWAYS/ will cause the scrollbar to always be displayed.

Use the following method to add a child widget to the scrolled window:

<tscreen><verb>
void Gtk::ScrolledWindow::add_with_viewport(const Gtk::Widget &amp;child);
</verb></tscreen>

Note that this differs from the usual <tt/add()/ method.

Here is a simple example that packs 100 toggle buttons into a scrolled
window.  Try resizing the window, and notice how the scrollbars react.

example_incl_src(scrolledwin/scrolledwin.cc)

<!-- ----------------------------------------------------------------- -->
<sect1> Paned Window Widgets

<p>
Panes are used to divide a window into halves, separated by a moveable
divider.  GTK-- provides two of these widgets: <tt/Gtk::HPaned/ adds a
horizontal divider, and <tt/Gtk::VPaned/ adds a vertical one.  Other
than the names and the orientations, there's no difference between the
two, so we'll describe them together.

Unlike the other widgets in this chapter, pane widgets contain not
one, but two child widgets, one in each pane.  Otherwise, they're no
different from the other single-item widgets; they do, however, have
different <tt/add()/ methods.

The pane constructors are:
   
<tscreen><verb>
Gtk::HPaned();
Gtk::VPaned();
</verb></tscreen>

To add child widgets to the panes, call
   
<tscreen><verb>
void Gtk::Paned::add1 (const Gtk::Widget &amp;child);
void Gtk::Paned::add2 (const Gtk::Widget &amp;child);
</verb></tscreen>

For <tt/HPaned/, <tt/add1()/ adds the child to the upper pane; for
<tt/VPaned/, <tt/add1()/ adds to the left-hand pane.  <tt/add2()/ adds
to the opposite pane.
   
The appearance of the divider is adjustable.  Dividers have a small,
square <em/handle/ which the user can use to move the divider; the size
of this handle is adjustable.  The <em/gutter/ is a separation area
between the two panes; the width of the gutter is also adjustable.
These two parameters can be set using:

<tscreen><verb>
void Gtk::Paned::set_handle_size(guint16 size);
void Gtk::Paned::set_gutter_size(guint16 size);
</verb></tscreen>

where <tt/size/ is in pixels.  Typical values for these are 10 and 16,
respectively.

You can adjust the position of the divider using:

<tscreen><verb>
void Gtk::Paned::set_position(gint position);
</verb></tscreen>

where <tt/position/ is in pixels.  It's a good idea to set the initial
position using this method when you create the pane, since the default
position isn't always what you want.

It's example time again.  For the example, we'll create part of the
user interface of an imaginary E-mail program. A window is divided into
two portions vertically, with the top portion being a list of email
messages and the bottom portion the text of the email message.

A couple of things to notice: Text can't be added to a <tt/Gtk::Text/
widget until it is realized (actually displayed on the screen). This
could be done by calling <tt/Gtk::Widget::realize()/, but as a
demonstration of an alternate technique, we connect a handler to the
<tt/realize/ signal to add the text. Also, we need to add the
<tt/GTK_SHRINK/ option to some of the items in the table containing
the text window and its scrollbars, so that when the bottom portion is
made smaller, the correct portions shrink instead of being pushed off
the bottom of the window.

example_incl_src(paned/paned.cc)

