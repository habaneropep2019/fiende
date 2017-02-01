<!-- ***************************************************************** -->
<sect>The Button Widget
<!-- ***************************************************************** -->
<p>
We've already seen quite a bit of the button widget.  We've been using
it rather heavily in our examples so far, but the chances are that
you'll use it rather heavily throughout your GTK-- career, so it's a
useful thing to introduce early on.  This chapter provides a more
thorough discussion of this important widget.

GTK-- provides four basic types of buttons:

<descrip>

<tag/Pushbuttons/ (class: <tt/Gtk::Button/) Standard buttons, usually
marked with a label or picture; usually they cause something to happen
when you press them.  See <ref id="sec_Pushbuttons" name="Pushbuttons">.

<tag/Toggle buttons/
(class: <tt/Gtk::ToggleButton/)
These look like pushbuttons, but act differently: when you press a
Pushbutton, it "springs" back up; a toggle button stays down until you
press it again.  Useful when you need an on/off switch.  See <ref
id="sec_Toggle_Buttons" name="Toggle Buttons">.

<tag/Checkboxes/
(class: <tt/Gtk::CheckButton/)
These are buttons which act like toggle buttons, but are much
smaller.  They are usually marked with a label off to the side.
They're useful in all sorts of situations where you need an on/off
setting.  Whether you use this or a toggle button is a matter of the
situation, the amount of space you have, personal preference, etc.
See <ref id="sec_Checkboxes" name="Checkboxes">.

<tag/Radio buttons/
(class: <tt/Gtk::RadioButton/)
Named after the station-selectors on old car
radios, these buttons travel in groups; pressing one causes all the
others in its group to turn off.  They are similar in form to
Checkboxes (i.e., small control with a label to the side), but usually
look different.  Radio buttons are a good way to control
options which are mutually exclusive.
See <ref id="sec_Radio_Buttons" name="Radio Buttons">.
</descrip>

Note that, because of GTK's theming system, the appearance of these
widgets will vary.  In the case of checkboxes and radio buttons, they
may vary considerably.

<!-- ----------------------------------------------------------------- -->
<sect1>Pushbuttons <label id="sec_Pushbuttons">

<p> There are two ways to create a pushbutton: you can pass a string to
the <tt/Gtk::Button/ constructor to create a button with a label, or
pass no argument to it to create a blank button.  If you make a blank
button, you can pack a label or pixmap (icon or picture) into it.  You
can also put another container widget into it, and place more widgets
inside that.  (This makes it possible to construct rather unusual
layouts - imagine packing a button inside another button!)

Here's an example of creating a button with a picture and a label in
it.  We've broken up the code to make it easy for you to use 
in your own programs.

example_incl_src(buttons/buttons.cc)

Note that the <tt/XPMLabelBox/ class can be used to place XPMs and
labels into any widget that can be a container.

The <tt/Gtk::Button/ widget has the following signals:

<descrip>

<tag><tt/pressed/</tag> Emitted when the button is pressed.

<tag><tt/released/</tag> Emitted when the button is released.

<tag><tt/clicked/</tag> Emitted when the button is pressed and released.

<tag><tt/enter/</tag> Emitted when the mouse pointer moves over the
button's window.

<tag><tt/leave/</tag> Emitted when the mouse pointer leaves the button's window.

</descrip>

<!-- ----------------------------------------------------------------- -->
<sect1> Toggle Buttons <label id="sec_Toggle_Buttons">

<p>
The <tt/Gtk::ToggleButton/ constructors are:

<tscreen><verb>
Gtk::ToggleButton();
Gtk::ToggleButton(const string &amp;label);
</verb></tscreen>

These work exactly the same as the <tt/Gtk::Button/ constructors.

To retrieve the state of the toggle button, you can use the method
<tt/Gtk::ToggleButton::get_active()/; this returns true if the button
is "down".  You can also set the toggle button's state; the member
function to use for this is:

<tscreen><verb>
void Gtk::ToggleButton::set_active(bool state);
</verb></tscreen>

Pass <tt/true/ to this method to force the button "down".  Note that
if you do this, and the state is actually changed, it causes the
"clicked" signal to be emitted.  This is usually what you want.

You can use the following function to toggle the button, rather than
forcing it to be up or down:

<tscreen><verb>
void Gtk::ToggleButton::toggled ();
</verb></tscreen>

This switches the button's state, and causes the <tt/toggled/ signal
to be emitted.

<!-- ----------------------------------------------------------------- -->
<sect1> Checkboxes <label id="sec_Checkboxes">
<p>
<tt/Gtk::CheckButton/ inherits from <tt/Gtk::ToggleButton/.  The only
real difference between the two is <tt/Gtk::CheckButton/'s
appearance; you can check, set, and toggle a checkbox using the same
member functions as you would for <tt/Gtk::ToggleButton/.

The two <tt/Gtk::CheckButton/ constructors are 

<tscreen><verb>
Gtk::CheckButton();
Gtk::CheckButton(const string &amp;label);
</verb></tscreen>

and they work exactly as the ones for <tt/Gtk::Button/.

<!-- ----------------------------------------------------------------- -->
<sect1> Radio Buttons <label id="sec_Radio_Buttons">
<p>
Like checkboxes, radio buttons also inherit from
<tt/Gtk::ToggleButton/, but there are more substantial differences
than mere appearance, due to their travelling in groups (you could
have a radio button by itself, but they're very social beings, and get
lonely rather easily.  Also, it doesn't make much sense).

The constructors for <tt/Gtk::RadioButton/ are:

<tscreen><verb>
  Gtk::RadioButton();
  Gtk::RadioButton(const string &amp;label, gfloat xalign=0.5, gfloat yalign=0.5);
  Gtk::RadioButton(Group &amp;groupx);
  Gtk::RadioButton(Group &amp;groupx,
                   const string &amp;label, gfloat xalign=0.5, gfloat yalign=0.5);
</verb></tscreen>

The first constructor makes a new radio button without a label; the
second lets you specify a label, and also allows you to tweak the
positioning of the label text, if you want.  The third and fourth
constructors are like the first two, except you also specify a group
to put the new buttons in.  (Of course, the group must exist first;
read on to see how this works.)

There are two ways to set up a group of radio buttons.  The first way
is to create the buttons, and set up their groups afterwards.  Only
the first two constructors are used.  In the following example, we
make a new window class called <tt/RadioButtons/, and then put three
radio buttons in it:

<tscreen><verb>
  class RadioButtons : public Gtk::Window
  {
      Gtk::RadioButton m_rb1, m_rb2, m_rb3;
      RadioButtons();
  };     

  RadioButtons::RadioButtons()
    : Window(),  
      m_rb1("button1"),
      m_rb2("button2"),
      m_rb3("button3")
  {
      m_rb2.set_group(m_rb1.group());
      m_rb3.set_group(m_rb2.group());
  }
</verb></tscreen>

When a radio button is created without a group, a new group is made
for it; this group can be retrieved through the radio button's
<tt/group()/ method.  A radio button's group can be changed using the
<tt/set_group()/ method.  In the example, we created three radio
buttons without specifying groups for them.  We then used
<tt/set_group()/ on radio button <tt/m_rb2/ to set its group to the
same as <tt/m_rb1/, and did likewise with <tt/m_rb3/.  The result is
that all three buttons belong to the same group.

The second way to set up radio buttons is to make a group first, and
then add radio buttons to it.  Here's an example:

<tscreen><verb>
  class RadioButtons : public Gtk::Window
  {
      RadioButtons();
  };     

  RadioButtons::RadioButtons()
    : Window()
  {
      Gtk::RadioButton::Group gr;
      Gtk::RadioButton *m_rb1=manage( new Gtk::RadioButton(gr,"button1"));
      Gtk::RadioButton *m_rb2=manage( new Gtk::RadioButton(gr,"button2"));
      Gtk::RadioButton *m_rb3=manage( new Gtk::RadioButton(gr,"button3"));
  }
</verb></tscreen>

We made a new group by simply declaring a variable, <tt/gr/, of type
<tt/Gtk::RadioButton::Group/.  Then we made three radio buttons, using
the third constructor to make each of them part of <tt/gr/.

Radio buttons are "off" when created; this means that when you first
make a group of them, they'll all be off.  Often this is not what you
want (or what the user expects), so don't forget to turn one of them 
on using <tt/set_active()/:

<tscreen><verb>
void Gtk::ToggleButton::set_active(bool state);
</verb></tscreen>

This function is inherited from <tt/Gtk::ToggleButton/, and works the
same.

The following is an example of using radio buttons:

example_incl_src(radiobuttons/radiobuttons.cc)


