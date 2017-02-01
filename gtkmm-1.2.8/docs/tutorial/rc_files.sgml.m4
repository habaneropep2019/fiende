<sect>Gtk--'s rc Files
<!-- ***************************************************************** -->
<p>
Gtk-- has its own way of dealing with application defaults, by using rc
files. These can be used to set the colors of just about any widget, and
can also be used to tile pixmaps onto the background of some widgets.  

<!-- ----------------------------------------------------------------- -->
<sect1>Functions For rc Files 
<p>
When your application starts, you should include a call to:

<tscreen><verb>
void gtk_rc_parse( char *filename );
</verb></tscreen>

Passing in the filename of your rc file. This will cause Gtk-- to parse
this file, and use the style settings for the widget types defined
there.

If you wish to have a special set of widgets that can take on a
different style from others, or any other logical division of widgets,
use a call to:

<tscreen><verb>
void gtk_widget_set_name( GtkWidget *widget,
                          gchar     *name );
</verb></tscreen>

Passing your newly created widget as the first argument, and the name
you wish to give it as the second. This will allow you to change the
attributes of this widget by name through the rc file.

If we use a call something like this:

<tscreen><verb>
button = gtk_button_new_with_label ("Special Button");
gtk_widget_set_name (button, "special button");
</verb></tscreen>

Then this button is given the name "special button" and may be addressed by
name in the rc file as "special button.GtkButton".  [<--- Verify ME!]

The example rc file below, sets the properties of the main window, and lets
all children of that main window inherit the style described by the "main
button" style.  The code used in the application is:

<tscreen><verb>
window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
gtk_widget_set_name (window, "main window");
</verb></tscreen>

And then the style is defined in the rc file using:

<tscreen><verb>
widget "main window.*GtkButton*" style "main_button"
</verb></tscreen>

Which sets all the GtkButton widgets in the "main window" to the
"main_buttons" style as defined in the rc file.

As you can see, this is a fairly powerful and flexible system.  Use your
imagination as to how best to take advantage of this.

<!-- ----------------------------------------------------------------- -->
<sect1>Gtk--'s rc File Format
<p>
The format of the Gtk-- file is illustrated in the example below. This is
the testgtkrc file from the Gtk-- distribution, but I've added a
few comments and things. You may wish to include this explanation
your application to allow the user to fine tune his application.

There are several directives to change the attributes of a widget.

<itemize>
<item>fg - Sets the foreground color of a widget.
<item>bg - Sets the background color of a widget.
<item>bg_pixmap - Sets the background of a widget to a tiled pixmap.
<item>font - Sets the font to be used with the given widget.
</itemize>

In addition to this, there are several states a widget can be in, and you
can set different colors, pixmaps and fonts for each state. These states are:

<itemize>
<item>NORMAL - The normal state of a widget, without the mouse over top of
it, and not being pressed etc.
<item>PRELIGHT - When the mouse is over top of the widget, colors defined
using this state will be in effect.
<item>ACTIVE - When the widget is pressed or clicked it will be active, and
the attributes assigned by this tag will be in effect.
<item>INSENSITIVE - When a widget is set insensitive, and cannot be
activated, it will take these attributes.
<item>SELECTED - When an object is selected, it takes these attributes.
</itemize>

When using the "fg" and "bg" keywords to set the colors of widgets, the
format is:

<tscreen><verb>
fg[<STATE>] = { Red, Green, Blue }
</verb></tscreen>

Where STATE is one of the above states (PRELIGHT, ACTIVE etc), and the Red,
Green and Blue are values in the range of 0 - 1.0,  { 1.0, 1.0, 1.0 } being
white. They must be in float form, or they will register as 0, so a straight 
"1" will not work, it must be "1.0".  A straight "0" is fine because it 
doesn't matter if it's not recognized.  Unrecognized values are set to 0.

bg_pixmap is very similar to the above, except the colors are replaced by a
filename.

pixmap_path is a list of paths separated by ":"'s.  These paths will be
searched for any pixmap you specify.

The font directive is simply:
<tscreen><verb>
font = "<font name>"
</verb></tscreen>

Where the only hard part is figuring out the font string. Using xfontsel or
similar utility should help.

The "widget_class" sets the style of a class of widgets. These classes are
listed in the widget overview on the class hierarchy.

The "widget" directive sets a specifically named set of widgets to a
given style, overriding any style set for the given widget class.
These widgets are registered inside the application using the
gtk_widget_set_name() call. This allows you to specify the attributes of a
widget on a per widget basis, rather than setting the attributes of an
entire widget class. I urge you to document any of these special widgets so
users may customize them.

When the keyword <tt>parent</> is used as an attribute, the widget will take on
the attributes of its parent in the application.

When defining a style, you may assign the attributes of a previously defined
style to this new one.

<tscreen><verb>
style "main_button" = "button"
{
  font = "-adobe-helvetica-medium-r-normal--*-100-*-*-*-*-*-*"
  bg[PRELIGHT] = { 0.75, 0, 0 }
}
</verb></tscreen>

This example takes the "button" style, and creates a new "main_button" style
simply by changing the font and prelight background color of the "button"
style.

Of course, many of these attributes don't apply to all widgets. It's a
simple matter of common sense really. Anything that could apply, should.

<!-- ----------------------------------------------------------------- -->
<sect1>Example rc file
<p>

<tscreen><verb>
# pixmap_path "<dir 1>:<dir 2>:<dir 3>:..."
#
pixmap_path "/usr/include/X11R6/pixmaps:/home/imain/pixmaps"
#
# style <name> [= <name>]
# {
#   <option>
# }
#
# widget <widget_set> style <style_name>
# widget_class <widget_class_set> style <style_name>


# Here is a list of all the possible states.  Note that some do not apply to
# certain widgets.
#
# NORMAL - The normal state of a widget, without the mouse over top of
# it, and not being pressed etc.
#
# PRELIGHT - When the mouse is over top of the widget, colors defined
# using this state will be in effect.
#
# ACTIVE - When the widget is pressed or clicked it will be active, and
# the attributes assigned by this tag will be in effect.
#
# INSENSITIVE - When a widget is set insensitive, and cannot be
# activated, it will take these attributes.
#
# SELECTED - When an object is selected, it takes these attributes.
#
# Given these states, we can set the attributes of the widgets in each of
# these states using the following directives.
#
# fg - Sets the foreground color of a widget.
# fg - Sets the background color of a widget.
# bg_pixmap - Sets the background of a widget to a tiled pixmap.
# font - Sets the font to be used with the given widget.
#

# This sets a style called "button".  The name is not really important, as
# it is assigned to the actual widgets at the bottom of the file.

style "window"
{
  #This sets the padding around the window to the pixmap specified.
  #bg_pixmap[<STATE>] = "<pixmap filename>"
  bg_pixmap[NORMAL] = "warning.xpm"
}

style "scale"
{
  #Sets the foreground color (font color) to red when in the "NORMAL"
  #state.
  
  fg[NORMAL] = { 1.0, 0, 0 }
  
  #Sets the background pixmap of this widget to that of its parent.
  bg_pixmap[NORMAL] = "<parent>"
}

style "button"
{
  # This shows all the possible states for a button.  The only one that
  # doesn't apply is the SELECTED state.
  
  fg[PRELIGHT] = { 0, 1.0, 1.0 }
  bg[PRELIGHT] = { 0, 0, 1.0 }
  bg[ACTIVE] = { 1.0, 0, 0 }
  fg[ACTIVE] = { 0, 1.0, 0 }
  bg[NORMAL] = { 1.0, 1.0, 0 }
  fg[NORMAL] = { .99, 0, .99 }
  bg[INSENSITIVE] = { 1.0, 1.0, 1.0 }
  fg[INSENSITIVE] = { 1.0, 0, 1.0 }
}

# In this example, we inherit the attributes of the "button" style and then
# override the font and background color when prelit to create a new
# "main_button" style.

style "main_button" = "button"
{
  font = "-adobe-helvetica-medium-r-normal--*-100-*-*-*-*-*-*"
  bg[PRELIGHT] = { 0.75, 0, 0 }
}

style "toggle_button" = "button"
{
  fg[NORMAL] = { 1.0, 0, 0 }
  fg[ACTIVE] = { 1.0, 0, 0 }
  
  # This sets the background pixmap of the toggle_button to that of its
  # parent widget (as defined in the application).
  bg_pixmap[NORMAL] = "<parent>"
}

style "text"
{
  bg_pixmap[NORMAL] = "marble.xpm"
  fg[NORMAL] = { 1.0, 1.0, 1.0 }
}

style "ruler"
{
  font = "-adobe-helvetica-medium-r-normal--*-80-*-*-*-*-*-*"
}

# pixmap_path "~/.pixmaps"

# These set the widget types to use the styles defined above.
# The widget types are listed in the class hierarchy, but could probably be
# just listed in this document for the users reference.

widget_class "GtkWindow" style "window"
widget_class "GtkDialog" style "window"
widget_class "GtkFileSelection" style "window"
widget_class "*Gtk*Scale" style "scale"
widget_class "*GtkCheckButton*" style "toggle_button"
widget_class "*GtkRadioButton*" style "toggle_button"
widget_class "*GtkButton*" style "button"
widget_class "*Ruler" style "ruler"
widget_class "*GtkText" style "text"

# This sets all the buttons that are children of the "main window" to
# the main_button style.  These must be documented to be taken advantage of.
widget "main window.*GtkButton*" style "main_button"
</verb></tscreen>

<!-- ***************************************************************** -->
