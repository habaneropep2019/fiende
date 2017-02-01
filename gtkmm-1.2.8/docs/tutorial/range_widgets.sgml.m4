<sect> Range Widgets<label id="sec_Range_Widgets">
<!-- ***************************************************************** -->

<p>
The category of range widgets includes the ubiquitous scrollbar widget
and the less common "scale" widget. Though these two types of widgets
are generally used for different purposes, they are quite similar in
function and implementation. All range widgets share a set of common
graphic elements, each of which has its own X window and receives
events. They all contain a "trough" and a "slider" (what is sometimes
called a "thumbwheel" in other GUI environments). Dragging the slider
with the pointer moves it back and forth within the trough, while
clicking in the trough advances the slider towards the location of the
click, either completely, or by a designated amount, depending on
which mouse button is used.

As mentioned in <ref id="sec_Adjustment" name="Adjustments"> above,
all range widgets are associated with an adjustment object, from which
they calculate the length of the slider and its position within the
trough. When the user manipulates the slider, the range widget will
change the value of the adjustment.

<!-- ----------------------------------------------------------------- -->
<sect1> Scrollbar Widgets
<p>
These are your standard, run-of-the-mill scrollbars. These should be
used only for scrolling some other widget, such as a list, a text box,
or a viewport (and it's generally easier to use the scrolled window
widget in most cases).  For other purposes, you should use scale
widgets, as they are friendlier and more featureful.

There are separate types for horizontal and vertical scrollbars.
There really isn't much to say about these. You create them with the
following constructors:

<tscreen><verb>
Gtk::HScrollbar( Gtk::Adjustment &amp;adjustment );
Gtk::HScrollbar();

Gtk::VScrollbar( GtkAdjustment &amp;adjustment );
Gtk::VScrollbar();
</verb></tscreen>

and that's about it (if you don't believe me, look in the header
files!).  You can either pass a reference to an existing
<tt/adjustment/ as argument, or no argument at all, in which case one
will be created for you. Not specifying an argument might actually be
useful in this case, if you wish to pass the newly-created adjustment
to the constructor function of some other widget which will configure
it for you, such as a text widget. You can fetch the object's
adjustment using the following method:

<tscreen><verb>
Gtk::Adjustment* GtkRange::get_adjustment()
</verb></tscreen>

<tt/Gtk::Scrollbar/ derives from <tt/Gtk::Range/; it's defined in
<tt>&lt;gtk--/scrollbar.h&gt;</tt>.


</sect1>

<!-- ----------------------------------------------------------------- -->
<sect1> Scale Widgets

<p> Scale widgets (or "sliders") are used to allow the user to
visually select and manipulate a value within a specific range. You
might want to use a scale widget, for example, to adjust the
magnification level on a zoomed preview of a picture, or to control
the brightness of a colour, or to specify the number of minutes of
inactivity before a screensaver takes over the screen.

<!-- ----------------------------------------------------------------- -->
<sect2>Creating a Scale Widget
<p>
As with scrollbars, there are separate widget types for horizontal and
vertical scale widgets. (Most programmers seem to favour horizontal
scale widgets). Since they work essentially the same way, there's no
need to treat them separately here. Here are the constructors for vertical
and horizontal scale widgets, respectively:

<tscreen>
<verb>
Gtk::VScale(GtkAdjustment &amp;adjustment);
Gtk::VScale();

Gtk::HScale(GtkAdjustment &amp;adjustment);
Gtk::HScale();
</verb>
</tscreen>

As with scrollbars, you can either pass an existing <tt/adjustment/ as
argument, or no argument at all, in which case an anonymous
<tt/Gtk::Adjustment/ is created with all of its values set to <tt/0.0/
(which isn't very useful in this case). In order to avoid confusing
yourself, you probably want to create your adjustment with a
<tt/page_size/ of <tt/0.0/ so that its <tt/upper/ value actually
corresponds to the highest value the user can select.  (If you're
already thoroughly confused, go back and read the section on <ref
id="sec_Adjustment" name="Adjustments"> again (you <em/did/ read it
already, didn't you?) for an explanation of what exactly adjustments
do and how to create and manipulate them).

<!-- ----------------------------------------------------------------- -->
<sect2> Methods and Signals (well, methods, at least)
<p>
Scale widgets can display their current value as a number beside the
trough. The default behaviour is to show the value, but you can change
this with this function:

<tscreen><verb>
void Gtk::Scale::set_draw_value(bool draw_value);
</verb></tscreen>

The value displayed by a scale widget is rounded to one decimal point
by default, as is the <tt/value/ field in its <tt/Gtk::Adjustment/. You can
change this with:

<tscreen>
<verb>
void Gtk::Scale::set_digits(gint digits);
</verb>
</tscreen>

where <tt/digits/ is the number of decimal places you want. You can
set <tt/digits/ to anything you like, but no more than 13 decimal
places will actually be drawn.

Finally, the value can be drawn in different positions
relative to the trough:

<tscreen>
<verb>
void Gtk::Scale::set_value_pos(GtkPositionType pos);
</verb>
</tscreen>

The argument <tt/pos/ can be one of the following:

<itemize>
<item><tt/GTK_POS_LEFT/
<item><tt/GTK_POS_RIGHT/
<item><tt/GTK_POS_TOP/
<item><tt/GTK_POS_BOTTOM/
</itemize>

If you position the value on the "side" of the trough (e.g. on the top
or bottom of a horizontal scale widget), then it will follow the
slider up and down the trough.

The <tt/Gtk::Scale/ class is defined in <tt>&lt;gtk--/scale.h&gt;</tt>.

</sect2>
</sect1>

<!-- ----------------------------------------------------------------- -->
<sect1> Common Functions <label id="sec_Range_Functions">
<p>
The <tt/Gtk::Range/ widget class is fairly complicated internally, but like
all the "base class" widgets, most of its complexity is only
interesting if you want to hack on it. Also, almost all of the
methods and signals it defines are only really used in writing
derived widgets. There are, however, a few useful methods that are
defined in <tt>&lt;gtk--/range.h&gt;</tt> and will work on all range
widgets.

<!-- ----------------------------------------------------------------- -->
<sect2> Setting the Update Policy
<p>
The <em/update policy/ of a range widget defines at what points during
user interaction it will change the <tt/value/ field of its
<tt/Gtk::Adjustment/ and emit the <tt/value_changed/ signal on this
<tt/Gtk::Adjustment/. The update policies, defined in
<tt>&lt;gtk/gtkenums.h&gt;</tt> as type <tt>enum GtkUpdateType</tt>,
are:

<itemize>
<item><tt/GTK_UPDATE_POLICY_CONTINUOUS/ - This is the default. The
<tt/value_changed/ signal is emitted continuously, i.e. whenever the
slider is moved by even the tiniest amount.
</item>
<item><tt/GTK_UPDATE_POLICY_DISCONTINUOUS/ - The <tt/value_changed/ signal is
only emitted once the slider has stopped moving and the user has
released the mouse button.
</item>
<item><tt/GTK_UPDATE_POLICY_DELAYED/ - The <tt/value_changed/ signal is emitted
when the user releases the mouse button, or if the slider stops moving
for a short period of time.
</item>
</itemize>

The update policy of a range widget can be set using this method:

<tscreen><verb>
void Gtk::Range::set_update_policy(GtkUpdateType policy);
</verb></tscreen>

Getting and setting the adjustment for a range widget "on the fly" is
done, predictably, with:

<tscreen><verb>
Gtk::Adjustment* Gtk::Range::get_adjustment();
void Gtk::Range::set_adjustment(Gtk::Adjustment *adjustment);
</verb></tscreen>

</sect2>
</sect1>

<!-- ----------------------------------------------------------------- -->
<sect1> Key and Mouse bindings

<p>
All of the GTK-- range widgets react to mouse clicks in more or less
the same way.  Clicking button 1 in the trough will cause its
adjustment's <tt/page_increment/ to be added or subtracted from its
<tt/value/, and the slider to be moved accordingly. Clicking mouse
button 2 in the trough will jump the slider to the point at which the
button was clicked. Clicking any button on a scrollbar's arrows will
cause its adjustment's value to change <tt/step_increment/ at a time.

It may take a little while to get used to, but by default, scrollbars
as well as scale widgets can take the keyboard focus in GTK--. If you
think your users (or you!) will find this too confusing, you can
always disable this by unsetting the <tt/GTK_CAN_FOCUS/ flag on the
scrollbar, like this:

<tscreen><verb>
scrollbar.unset_flag(GTK_CAN_FOCUS);
</verb></tscreen>

The key bindings (which are, of course, only active when the widget
has focus) are slightly different between horizontal and vertical
range widgets, for obvious reasons. They are also not quite the same
for scale widgets as they are for scrollbars, for somewhat less
obvious reasons (possibly to avoid confusion between the keys for
horizontal and vertical scrollbars in scrolled windows, where both
operate on the same area).

<sect2> Vertical Range Widgets
<p>
All vertical range widgets can be operated with the up and down arrow
keys, as well as with the <tt/Page Up/ and <tt/Page Down/ keys. The
arrows move the slider up and down by <tt/step_increment/, while
<tt/Page Up/ and <tt/Page Down/ move it by <tt/page_increment/.

The user can also move the slider all the way to one end or the other
of the trough using the keyboard. With the <tt/Gtk::VScale/ widget, this is
done with the <tt/Home/ and <tt/End/ keys, whereas with the
<tt/Gtk::VScrollbar/ widget, it's done by typing <tt>Control-Page Up</tt>
and <tt>Control-Page Down</tt>.

<!-- ----------------------------------------------------------------- -->
<sect2> Horizontal Range Widgets
<p>
The left and right arrow keys work as you might expect in these
widgets, moving the slider back and forth by <tt/step_increment/. The
<tt/Home/ and <tt/End/ keys move the slider to the ends of the trough.
For the <tt/Gtk::HScale/ widget, moving the slider by <tt/page_increment/ is
accomplished with <tt>Control-Left</tt> and <tt>Control-Right</tt>,
while for <tt/Gtk::HScrollbar/, it's done with <tt>Control-Home</tt> and
<tt>Control-End</tt>.
</sect2>
</sect1>

<!-- ----------------------------------------------------------------- -->
<sect1>Example: Range widgets<label id="sec_Range_Example"> 

<p>
This example displays a window with three range widgets all connected
to the same adjustment, along with a couple of controls for adjusting
some of the parameters mentioned above and in the section on
adjustments, so you can see how they affect the way these widgets work
for the user.

Notice the use of subclassing in the example.  We've made a
"mini-widget" called <tt/LabeledOptionMenu/ which simplifies the
creation of the two labelled option menus.  Although, in this small
example, we don't really gain anything in terms of code size, for
large programs the technique can quickly prove its worth.

example_incl_src(rangewidgets/rangewidgets.cc)

</sect1>
</sect>

<!-- ***************************************************************** -->
