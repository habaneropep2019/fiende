<!-- ***************************************************************** -->
<sect> Adjustments <label id="sec_Adjustment">
<!-- ***************************************************************** -->
<p>
GTK-- has various widgets that can be visually adjusted by the user
using the mouse or the keyboard, such as the range widgets (described
in the section <ref id="sec_Range_Widgets" name="Range Widgets">).
There are also a few widgets that display some adjustable
portion of a larger area of data, such as the text widget and the
viewport widget.

Obviously, an application needs to be able to react to changes the
user makes in range widgets. One way to do this would be to have each
widget emit its own type of signal when its adjustment changes, and
either pass the new value to the signal handler, or require it to look
inside the widget's data structure in order to ascertain the value.
But you may also want to connect the adjustments of several widgets
together, so that adjusting one adjusts the others. The most obvious
example of this is connecting a scrollbar to a panning viewport or a
scrolling text area. If each widget has its own way of setting or
getting the adjustment value, then the programmer may have to write
his own signal handlers to translate between the output of one
widget's signal and the "input" of another's adjustment setting
function.

GTK-- solves this problem using the <tt/Gtk::Adjustment/ object, which is a
way for widgets to store and pass adjustment information in an
abstract and flexible form. The most obvious use of <tt/Gtk::Adjustment/ is
to store the configuration parameters and values of range widgets,
such as scrollbars and scale controls. However, since <tt/Gtk::Adjustment/s
are derived from <tt/Gtk::Object/, they have some special powers beyond
those of normal classes. Most importantly, they can emit signals, just
like widgets, and these signals can be used not only to allow your
program to react to user input on adjustable widgets, but also to
propagate adjustment values transparently between adjustable widgets.

<sect1> Creating an Adjustment
<p>
The <tt/Gtk::Adjustment/ constructor is as follows:

<tscreen><verb>
Gtk::Adjustment( gfloat value,
                gfloat lower,
                gfloat upper,
                gfloat step_increment=1,
                gfloat page_increment=10,
                gfloat page_size=0);
</verb></tscreen>

The <tt/value/ argument is the initial value you want to give to the
adjustment, usually corresponding to the topmost or leftmost position
of an adjustable widget. The <tt/lower/ argument specifies the lowest
value which the adjustment can hold. The <tt/step_increment/ argument
specifies the "smaller" of the two increments by which the user can
change the value, while the <tt/page_increment/ is the "larger" one.
The <tt/page_size/ argument usually corresponds somehow to the visible
area of a panning widget. The <tt/upper/ argument is used to represent
the bottom most or right most coordinate in a panning widget's
child. Therefore it is <em/not/ always the largest number that
<tt/value/ can take, since the <tt/page_size/ of such widgets is
usually non-zero.

<!-- ----------------------------------------------------------------- -->
<sect1> Using Adjustments the Easy Way
<p>
The adjustable widgets can be roughly divided into those which use and
require specific units for these values, and those which treat them as
arbitrary numbers. The group which treats the values as arbitrary
numbers includes the range widgets (scrollbars and scales, the
progress bar widget, and the spin-button widget). These widgets are
all the widgets which are typically "adjusted" directly by the user
with the mouse or keyboard. They will treat the <tt/lower/ and
<tt/upper/ values of an adjustment as a range within which the user
can manipulate the adjustment's <tt/value/. By default, they will only
modify the <tt/value/ of an adjustment.

The other group includes the text widget, the viewport widget, the
compound list widget, and the scrolled window widget. All of these
widgets use pixel values for their adjustments. These are also all
widgets which are typically "adjusted" indirectly using scrollbars.
While all widgets which use adjustments can either create their own
adjustments or use ones you supply, you'll generally want to let this
particular category of widgets create its own adjustments. Usually,
they will eventually override all the values except the <tt/value/
itself in whatever adjustments you give them, but the results are, in
general, undefined (meaning, you'll have to read the source code to
find out, and it may be different from widget to widget).

Now, you're probably thinking that since text widgets and viewports insist
on setting everything except the <tt/value/ of their adjustments,
while scrollbars will <em/only/ touch the adjustment's <tt/value/, if
you <em/share/ an adjustment object between a scrollbar and a text
widget, will manipulating the scrollbar will automagically adjust the text
widget?  Of course it will!  You can set it up like this:

<tscreen><verb>
  // creates its own adjustments
  Gtk::Text text(0, 0);
  // uses the newly-created adjustment for the scrollbar as well
  Gtk::VScrollbar vscrollbar (*(text.get_vadjustment()));
</verb></tscreen>

</sect1>
<!-- ----------------------------------------------------------------- -->
<sect1> Adjustment Internals
<p>
OK, you say, that's nice, but what if I want to create my own handlers
to respond when the user adjusts a range widget or a spin button, and
how do I get at the value of the adjustment in these handlers?
To access the value of a <tt>Gtk::Adjustment</tt>, you can use the
following method:

<tscreen><verb>
gfloat Gtk::Adjustment::get_value()
</verb></tscreen>

and its counterpart to set the value:

<tscreen><verb>
void Gtk::Adjustment::set_value(gfloat value);
</verb></tscreen>

As mentioned earlier, <tt/Gtk::Adjustment/ is a subclass of <tt/Gtk::Object/ just
like all the other GTK-- widgets, and thus it is able to emit signals.
This is, of course, why updates happen automagically when you share an
adjustment object between a scrollbar and another adjustable widget;
all adjustable widgets connect signal handlers to their adjustment's
<tt/value_changed/ signal, as can your program. Here's the definition
of this signal in <tt/struct _GtkAdjustmentClass/:

<tscreen><verb>
  void value_changed ();
</verb></tscreen>

The various widgets that use the Gtk::Adjustment object will emit this
signal on an adjustment whenever they change its value. This happens
both when user input causes the slider to move on a range widget, and
when the program explicitly changes the value with
<tt/Gtk::Adjustment::set_value()/. So, for example, if you have a scale
widget, and you want to change the rotation of a picture whenever its
value changes, you would create a callback like this:

<tscreen><verb>
void cb_rotate_picture (Gtk::Widget *picture)
{
  picture->set_rotation (adj->value);
...
</verb></tscreen>

and connect it to the scale widget's adjustment like this:

<tscreen><verb>
adj.value_changed.connect(bind<Widget*>(slot(&amp;cb_rotate_picture),picture));
</verb></tscreen>
<!-- connect_to_function (adj, "value_changed",
	             cb_rotate_picture, picture); -->

What about when a widget reconfigures the <tt/upper/ or <tt/lower/
fields of its adjustment, such as when a user adds more text to a text
widget?  In this case, it emits the <tt/changed/ signal, which looks
like this:

<tscreen><verb>
  void changed();
</verb></tscreen>

Range widgets typically connect a handler to this signal, which
changes their appearance to reflect the change - for example, the size
of the slider in a scrollbar will grow or shrink in inverse proportion
to the difference between the <tt/lower/ and <tt/upper/ values of its
adjustment.

You probably won't ever need to attach a handler to this signal,
unless you're writing a new type of range widget.  However, if you
change any of the values in a Gtk::Adjustment directly, you should emit
this signal on it to reconfigure whatever widgets are using it, like
this:

<tscreen><verb>
adjustment->changed();
</verb></tscreen>

Now go forth and adjust!
</sect1>
</sect>

