<sect> Text Widget (draft)
<!-- ***************************************************************** -->
<p>
The Text widget allows multiple lines of text to be displayed and
edited.  It supports both multi-coloured and multi-font text, allowing
them to be mixed in any combination. It also has a wide set of key
based text editing commands, which are compatible with Emacs.

The text widget supports full cut-and-paste facilities, including the
use of double- and triple-click to select a word and a whole line,
respectively.

<!-- ----------------------------------------------------------------- -->
<sect1>Creating and Configuring a Text box
<p>
The constructor for creating a new Text widget is:

<tscreen><verb>
Gtk::Text(Gtk::Adjustment &amp;hadj,
          Gtk::Adjustment &amp;vadj);
</verb></tscreen>

The arguments allow us to give the Text widget pointers to Adjustments
that can be used to track the viewing position of the widget. Leaving
out the arguments will cause the constructor to create its own Adjustments.

<tscreen><verb>
void Gtk::Text::set_adjustments(Gtk::Adjustment &amp;hadj,
                                Gtk::Adjustment &amp;vadj);
</verb></tscreen>

The above function allows the horizontal and vertical adjustments of a
text widget to be changed at any time.

The text widget will not automatically create its own scrollbars when
the amount of text to be displayed is too long for the display
window. We therefore have to create and add them to the display layout
ourselves.

<tscreen><verb>
  Gtk::VScrollbar vscrollbar = Gtk::VScrollbar(text.get_vadjustment());
  hbox.pack_start(vscrollbar, false, false, 0);
  vscrollbar.show();
</verb></tscreen>

The above code snippet creates a new vertical scrollbar, and attaches
it to the vertical adjustment of the text widget, <tt/text/. It then
packs it into a box, <tt/hbox/, in the normal way.

Unfortunately, <tt/Gtk::Text/ does not currently support horizontal
scrollbars.

There are two main ways in which a Text widget can be used: to allow
the user to edit a body of text, or to allow us to display multiple
lines of text to the user. In order for us to switch between these
modes of operation, the text widget has the following function:

<tscreen><verb>
void Gtk::Editable::set_editable(gboolean editable);
</verb></tscreen>

The <tt/editable/ argument is a TRUE or FALSE value that specifies
whether the user is permitted to edit the contents of the Text
widget. When the text widget is editable, it will display a cursor at
the current insertion point.

You are not, however, restricted to just using the text widget in
these two modes. You can toggle the editable state of the text widget
at any time, and can insert text at any time.

The text widget wraps lines of text that are too long to fit onto a
single line of the display window. Its default behaviour is to break
words across line breaks. This can be changed using the next function:

<tscreen><verb>
void Gtk::Text::set_line_wrap(gboolean line_wrap);
</verb></tscreen>

Using this function allows us to specify that the text widget should
wrap long lines on word boundaries. The <tt/word_wrap/ argument is a
TRUE or FALSE value.

<!-- ----------------------------------------------------------------- -->
<sect1>Text Manipulation
<P>
The current insertion point of a Text widget can be set using
<tscreen><verb>
void Gtk::Text::set_point(guint index);
</verb></tscreen>

where <tt/index/ is the position to set the insertion point.

Analogous to this is the function for getting the current insertion
point:

<tscreen><verb>
guint Gtk::Text::get_point();
</verb></tscreen>

A function that is useful in combination with the above two functions
is

<tscreen><verb>
guint Gtk::Text::get_length();
</verb></tscreen>

which returns the current length of the text in the Text widget. The
length is the number of characters that are within the text block of
the widget, including characters such as newlines, which mark
the ends of lines.

In order to insert text at the current insertion point of a Text
widget, the <tt/insert()/ method is used; it also allows us to
specify background and foreground colors and a font for the text.

<tscreen><verb>
void Gtk::Text::insert(const Gdk_Font&amp; font,
                       const Gdk_Color&amp;   fore,
                       const Gdk_Color&amp;   back,
                       nstring&amp;    chars,
                       gint        length);
</verb></tscreen>

The font and colours of the text can also be specified using a
<em/context/ object:

<tscreen><verb>
void Gtk::Text_Helpers::Context();
</verb></tscreen>

The font and colours of the context can be set using:

<tscreen><verb>
void Gtk::Text_Helpers::Context::set_foreground(const Gdk_Color&amp; color)
void Gtk::Text_Helpers::Context::set_background(const Gdk_Color&amp; color)
void Gtk::Text_Helpers::Context::set_font(const Gdk_Font&amp; font)
</verb></tscreen>

These values can be cleared by calling the same methods without
arguments. Corresponding "get" methods also exist for retrieving the
values.

The default context of the text widget can be set using:

<tscreen><verb>
void Gtk::Text::set_context(const Context&amp; gc);
</verb></tscreen>

Again, the value can be cleared by calling with no argument. To insert
text using a context, use:

<tscreen><verb>
void Gtk::Text::insert(const Context&amp; gc, const string&amp; text);
</verb></tscreen>

or simply:

<tscreen><verb>
void Gtk::Text::insert(const string&amp; text);
</verb></tscreen>

to insert using the text widget's current context. If the widget's
context is cleared, text with no specified style is inserted. So
if you want a plain text widget you only have to use this member.

The text widget is one of the few within GTK-- that redraws itself
dynamically. This means that all changes to the contents of the text
widget take effect immediately. This may be undesirable when
performing multiple changes to the text widget. In order to allow us
to perform multiple updates to the text widget without it continuously
redrawing, we can "freeze" the widget, which temporarily stops it from
automatically redrawing itself every time it is changed. We can then
"thaw" the widget after our updates are complete.

The following two functions perform this freeze and thaw action:

<tscreen><verb>
void Gtk::Text::freeze();
void Gtk::Text::thaw();
</verb></tscreen>

Text is deleted from the text widget relative to the current insertion
point by the following two functions. The return value is a TRUE or
FALSE indicator of whether the operation was successful.

<tscreen><verb>
gint Gtk::Text::backward_delete(guint nchars);

gint Gtk::Text::forward_delete(guint nchars);
</verb></tscreen>

To retrieve blocks of text from the text widget, we can use the
function

<tscreen><verb>
string Gtk::Editable::get_chars(gint start_pos,
                                gint end_pos);   
</verb></tscreen>

This is a member of the parent class of the text widget. A value of
-1 as <tt/end_pos/ signifies the end of the text. The index of the
text starts at 0.

<!-- ----------------------------------------------------------------- -->
<sect1>Keyboard Shortcuts

<p>
The text widget has a number of pre-installed keyboard shortcuts for
common editing, motion and selection functions. These are accessed
using Control and Alt key combinations.

In addition to these, holding down the Control key whilst using cursor
key movement will move the cursor by words rather than
characters. Holding down Shift whilst using cursor movement will
extend the selection.

Any resemblance to the Emacs keybindings is purely coincidental. :)

<sect2>Motion Shortcuts
<p>
<itemize>
<item> Ctrl-A   Beginning of line
<item> Ctrl-E   End of line
<item> Ctrl-N   Next Line
<item> Ctrl-P   Previous Line
<item> Ctrl-B   Backward one character
<item> Ctrl-F   Forward one character
<item> Alt-B    Backward one word
<item> Alt-F    Forward one word
</itemize>

<sect2>Editing Shortcuts
<p>
<itemize>
<item> Ctrl-H   Delete Backward Character (Backspace)
<item> Ctrl-D   Delete Forward Character (Delete)
<item> Ctrl-W   Delete Backward Word
<item> Alt-D    Delete Forward Word
<item> Ctrl-K   Delete to end of line
<item> Ctrl-U   Delete line
</itemize>

<sect2>Selection Shortcuts
<p>
<itemize>
<item> Ctrl-X   Cut to clipboard
<item> Ctrl-C   Copy to clipboard
<item> Ctrl-V   Paste from clipboard
</itemize>

<!-- ----------------------------------------------------------------- -->
<sect1>An example

<p>
example_incl_src(text/text.cc)

<!-- ***************************************************************** -->
