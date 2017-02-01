<!-- ***************************************************************** -->
<sect>List & CList
<!-- ***************************************************************** -->

<p> The List and CList ("columnar list") display information in rows,
and, in the case of CList, columns.  The List widget is the older, and
slower, of the two; its primary advantage over CList is that any
widget can be placed in its rows.  The CList widget is restricted to
text, but is much faster, and can handle very large lists; the List
widget bogs down rather dramatically when handling a large list.

The current word around the campfire seems to be that CList will
eventually go away, to be replaced by a new "Super-List" widget, which
will combine the best features from List and CList, along with an
updated API.  Until then, however, you'll probably want to use CList
for most things (or perhaps think about writing a "Super-List" widget
yourself!).

<!-- ----------------------------------------------------------------- -->
<sect1>The List widget




The Gtk::CList widget has replaced the Gtk::List widget (which is still
available).

The Gtk::CList widget is a multi-column list widget that is capable of
handling literally thousands of rows of information. Each column can
optionally have a title, which itself is optionally active, allowing
us to bind a function to its selection.

<!-- ----------------------------------------------------------------- -->
<sect1>Creating a Gtk::CList widget
<p>
Creating a Gtk::CList is quite straightforward, once you have learned
about widgets in general. It provides the almost standard two ways,
that is the hard way, and the easy way. But before we create it, there
is one thing we should figure out beforehand: how many columns should
it have?

Not all columns have to be visible and can be used to store data that
is related to a certain cell in the list.

<tscreen><verb>
!!!!!!!!!! Correct this API
Gtk::CList( gint   columns,
           gchar *titles[] );
</verb></tscreen>

The first form is very straight forward, the second might require some
explanation. Each column can have a title associated with it, and this
title can be a label or a button that reacts when we click on it. If
we use the second form, we must provide pointers to the title texts,
and the number of pointers should equal the number of columns
specified. Of course we can always use the first form, and manually
add titles later.

Note: the Gtk::CList widget does not have its own scrollbars and should
be placed within a Gtk::ScrolledWindow widget if your require this
functionality.

<!-- ----------------------------------------------------------------- -->
<sect1>Modes of operation
<p>
There are several attributes that can be used to alter the behaviour of
a Gtk::CList. First there is

<tscreen><verb>
void Gtk::CList::set_selection_mode( GtkSelectionMode  mode );
</verb></tscreen>

which, as the name implies, sets the selection mode of the
Gtk::CList. The first argument is the Gtk::CList widget, and the second
specifies the cell selection mode (they are defined in gtkenums.h). At
the time of this writing, the following modes are available to us:

<itemize>
<item> GTK_SELECTION_SINGLE - The selection is either NULL or contains
a GList pointer for a single selected item.

<item> GTK_SELECTION_BROWSE - The selection is NULL if the list
contains no widgets or insensitive ones only, otherwise it contains a
GList pointer for one GList structure, and therefore exactly one list
item.

<item> GTK_SELECTION_MULTIPLE - The selection is NULL if no list items
are selected or a GList pointer for the first selected item. That in
turn points to a GList structure for the second selected item and so
on. This is currently the <bf>default</bf> for the GtkCList widget.

<item> GTK_SELECTION_EXTENDED - The selection is always NULL.
</itemize>

Others might be added in later revisions of Gtk--.

We can also define what the border of the GtkCList widget should look
like. It is done through

<tscreen><verb>
void Gtk::CList::set_shadow_type( GtkShadowType  border );
</verb></tscreen>

And the possible values for the second argument are

<itemize>
<item> GTK_SHADOW_NONE

<item> GTK_SHADOW_IN

<item> GTK_SHADOW_OUT

<item> GTK_SHADOW_ETCHED_IN

<item> GTK_SHADOW_ETCHED_OUT
</itemize>

<!-- ----------------------------------------------------------------- -->
<sect1>Working with titles
<p>
When you create a Gtk::CList widget, you will also get a set of title
buttons automatically. They live in the top of the CList window, and
can act either as normal buttons that respond to being pressed, or
they can be passive, in which case they are nothing more than a
title. There are four different calls that aid us in setting the
status of the title buttons.

<tscreen><verb>
void Gtk::CList::column_title_active( gint     column );

void Gtk::CList::column_title_passive( gint      column );

void Gtk::CList::column_titles_active();

void Gtk::CList::column_titles_passive();
</verb></tscreen>

An active title is one which acts as a normal button, a passive one is
just a label. The first two calls above will activate/deactivate the
title button above the specific column, while the last two calls
activate/deactivate all title buttons in the supplied clist widget.

But of course there are those cases when we don't want them at all,
and so they can be hidden and shown at will using the following two
calls.

<tscreen><verb>
void Gtk::CList::column_titles_show();

void Gtk::CList::column_titles_hide();
</verb></tscreen>

For titles to be really useful we need a mechanism to set and change
them, and this is done using

<tscreen><verb>
void Gtk::CList::set_column_title(gint      column,
                                 gchar    *title );
</verb></tscreen>

Note that only the title of one column can be set at a time, so if all
the titles are known from the beginning, then I really suggest using
Gtk::CList::new_with_titles (as described above) to set them. Saves you
coding time, and makes your program smaller. There are some cases
where getting the job done the manual way is better, and that's when
not all titles will be text. GtkCList provides us with title buttons
that can in fact incorporate whole widgets, for example a pixmap. It's
all done through

<tscreen><verb>
void Gtk::CList::set_column_widget(gint       column,
                                  GtkWidget *widget );
</verb></tscreen>

which should require no special explanation.

<!-- ----------------------------------------------------------------- -->
<sect1>Manipulating the list itself
<p>
It is possible to change the justification for a column, and it is
done through

<tscreen><verb>
void Gtk::CList::set_column_justification(gint              column,
                                         GtkJustification  justification );
</verb></tscreen>

The GtkJustification type can take the following values:

<itemize>
<item>GTK_JUSTIFY_LEFT - The text in the column will begin from the
left edge.

<item>GTK_JUSTIFY_RIGHT - The text in the column will begin from the
right edge.

<item>GTK_JUSTIFY_CENTER - The text is placed in the center of the
column.

<item>GTK_JUSTIFY_FILL - The text will use up all available space in
the column. It is normally done by inserting extra blank spaces
between words (or between individual letters if it's a single
word). Much in the same way as any ordinary WYSIWYG text editor.
</itemize>

The next function is a very important one, and should be standard in
the setup of all GtkCList widgets. When the list is created, the width
of the various columns are chosen to match their titles, and since
this is seldom the right width we have to set it using

<tscreen><verb>
void Gtk::CList::set_column_width(gint      column,
                                 gint      width );
</verb></tscreen>

Note that the width is given in pixels and not letters. The same goes
for the height of the cells in the columns, but as the default value
is the height of the current font this isn't as critical to the
application. Still, it is done through

<tscreen><verb>
void Gtk::CList::set_row_height( gint      height );
</verb></tscreen>

Again, note that the height is given in pixels.

We can also move the list around without user interaction, however, it
does require that we know what we are looking for. Or in other words,
we need the row and column of the item we want to scroll to.

<tscreen><verb>
void Gtk::CList::moveto(gint      row,
                       gint      column,
                       gfloat    row_align,
                       gfloat    col_align );
</verb></tscreen>

The gfloat row_align is pretty important to understand. It's a value
between 0.0 and 1.0, where 0.0 means that we should scroll the list so
the row appears at the top, while if the value of row_align is 1.0,
the row will appear at the bottom instead. All other values between
0.0 and 1.0 are also valid and will place the row between the top and
the bottom. The last argument, gfloat col_align works in the same way,
though 0.0 marks left and 1.0 marks right instead.

Depending on the application's needs, we don't have to scroll to an
item that is already visible to us. So how do we know if it is
visible? As usual, there is a function to find that out as well.

<tscreen><verb>
GtkVisibility Gtk::CList::row_is_visible( gint      row );
</verb></tscreen>

The return value is is one of the following:

<itemize>
<item>GTK_VISIBILITY_NONE

<item>GTK_VISIBILITY_PARTIAL

<item>GTK_VISIBILITY_FULL
</itemize>

Note that it will only tell us if a row is visible. Currently there is
no way to determine this for a column. We can get partial information
though, because if the return is GTK_VISIBILITY_PARTIAL, then some of
it is hidden, but we don't know if it is the row that is being cut by
the lower edge of the listbox, or if the row has columns that are
outside.

We can also change both the foreground and background colors of a
particular row. This is useful for marking the row selected by the
user, and the two functions that is used to do it are

<tscreen><verb>
void Gtk::CList::set_foreground(gint      row,
                               GdkColor *color );

void Gtk::CList::set_background(gint      row,
                               GdkColor *color );
</verb></tscreen>

Please note that the colors must have been previously allocated.

<!-- ----------------------------------------------------------------- -->
<sect1>Adding rows to the list
<p>
We can add rows in three ways. They can be prepended or appended to
the list using

<tscreen><verb>
gint Gtk::CList::prepend(const gchar    *text[] );
gint Gtk::CList::prepend(const vector&lt;string&gt; text);

gint Gtk::CList::append(gchar    *text[] );
gint Gtk::CList::append(const vector&lt;string&gt; text);
</verb></tscreen>

The return value of these two functions indicate the index of the row
that was just added. We can insert a row at a given place using

<tscreen><verb>
void Gtk::CList::insert(gint      row,
                       gchar    *text[] );
void Gtk::CList::insert(gint      row,
                       vector&lt;string&gt; text);
</verb></tscreen>

In these calls we have to provide a collection of pointers that are
the texts we want to put in the columns. The number of pointers should
equal the number of columns in the list. If the text[] argument is
NULL, then there will be no text in the columns of the row. This is
useful, for example, if we want to add pixmaps instead (something that
has to be done manually).

Also, please note that the numbering of both rows and columns start at 0.

To remove an individual row we use

<tscreen><verb>
void Gtk::CList::remove( gint      row );
</verb></tscreen>

There is also a call that removes all rows in the list. This is a lot
faster than calling Gtk::CList::remove once for each row, which is the
only alternative.

<tscreen><verb>
void Gtk::CList::clear();
</verb></tscreen>

There are also two convenience functions that should be used when a
lot of changes have to be made to the list. This is to prevent the
list flickering while being repeatedly updated, which may be highly
annoying to the user. So instead it is a good idea to freeze the list,
do the updates to it, and finally thaw it which causes the list to be
updated on the screen.

<tscreen><verb>
void Gtk::CList::freeze();

void Gtk::CList::thaw();
</verb></tscreen>

<!-- ----------------------------------------------------------------- -->
<sect1>Setting text and pixmaps in the cells
<p>
A cell can contain a pixmap, text or both. To set them the following
functions are used.

<tscreen><verb>
void Gtk::CList::set_text(gint      row,
                         gint      column,
                         const string &amp;text );

void Gtk::CList::set_pixmap(gint       row,
                           gint       column,
                           const Gdk_Pixmap &amp;pixmap,
                           const Gdk_Bitmap &amp;mask );

void Gtk::CList::set_pixtext(gint       row,
                            gint       column,
                            const string &amp;text,
                            guint8     spacing,
                            const Gdk_Pixmap &amp;pixmap,
                            const Gdk_Bitmap &amp;mask );
</verb></tscreen>

It's quite straightforward. All the calls have the GtkCList as the
first argument, followed by the row and column of the cell, followed
by the data to be set. The <tt/spacing/ argument in
Gtk::CList::set_pixtext is the number of pixels between the pixmap and
the beginning of the text.

To read back the data, we instead use

<tscreen><verb>
gint Gtk::CList::get_text(gint       row,
                         gint       column,
                         string    &amp;text );

gint Gtk::CList::get_pixmap(gint        row,
                           gint        column,
                           Gdk_Pixmap  &amp;pixmap,
                           Gdk_Bitmap  &amp;mask );

gint Gtk::CList::get_pixtext(gint        row,
                            gint        column,
                            string     &amp;text,
                            guint8     *spacing,
                            Gdk_Pixmap &amp;pixmap,
                            Gdk_Bitmap &amp;mask );
</verb></tscreen>

There is one more call that is related to what's inside a cell in the
clist, and that's

<tscreen><verb>
GtkCellType Gtk::CList::get_cell_type(gint      row,
                                     gint      column );
</verb></tscreen>

which returns the type of data in a cell. The return value is one of

<itemize>
<item>GTK_CELL_EMPTY

<item>GTK_CELL_TEXT

<item>GTK_CELL_PIXMAP

<item>GTK_CELL_PIXTEXT

<item>GTK_CELL_WIDGET
</itemize>

There is also a method that will let us set the indentation, both
vertical and horizontal, of a cell. The indentation value is of type
gint, given in pixels, and can be both positive and negative.

<tscreen><verb>
void Gtk::CList::set_shift(gint      row,
                          gint      column,
                          gint      vertical,
                          gint      horizontal );
</verb></tscreen>

<!-- ----------------------------------------------------------------- -->
<sect1>Storing data pointers
<p>
With a Gtk::CList it is possible to set a data pointer for a row. This
pointer will not be visible for the user, but is merely a convenience
for the programmer to associate a row with a pointer to some
additional data.

The functions should be fairly self-explanatory by now

<tscreen><verb>
void Gtk::CList::set_row_data( GtkCList *clist,
                             gint      row,
                             gpointer  data );

void Gtk::CList::set_row_data_full( GtkCList         *clist,
                                  gint              row,
                                  gpointer          data,
                                  GtkDestroyNotify  destroy );

gpointer Gtk::CList::get_row_data( GtkCList *clist,
                                 gint      row );

gint Gtk::CList::find_row_from_data( GtkCList *clist,
                                   gpointer  data );
</verb></tscreen>

<!-- ----------------------------------------------------------------- -->
<sect1>Working with selections
<p>
There are also functions available that let us force the (un)selection
of a row. These are

<tscreen><verb>
void Gtk::CList::select_row(gint      row,
                           gint      column );

void Gtk::CList::unselect_row(gint      row,
                             gint      column );
</verb></tscreen>

And also a function that will take x and y coordinates (for example,
read from the mousepointer), and map that onto the list, returning the
corresponding row and column.

<tscreen><verb>
gint Gtk::CList::get_selection_info(gint      x,
                                   gint      y,
                                   gint     *row,
                                   gint     *column );
</verb></tscreen>

When we detect something of interest, it might be movement of the
pointer, a click somewhere in the list, we can read the pointer
coordinates and find out where in the list the pointer is. Cumbersome?
Luckily, there is a simpler way...

<!-- ----------------------------------------------------------------- -->
<sect1>The signals that bring it together
<p>
As with all other widgets, there are a few signals that can be
used. The Gtk::CList widget is derived from the Gtk::Container widget,
and so has all the same signals, but also the adds following:

<itemize>
<item>select_row - This signal will send the following information, in
order: gint row, gint column, GtkEventButton *event

<item>unselect_row - When the user unselects a row, this signal is
activated. It sends the same information as select_row

<item>click_column - Send gint column
</itemize>

So if we want to connect a callback to select_row, the callback
function would be declared like this

<tscreen><verb>
!!!!!!!!!!
void select_row_callback(GtkWidget *widget,
                         gint row,
			 gint column,
                         GdkEventButton *event,
			 gpointer data);
</verb></tscreen>

The callback is connected as usual with

<tscreen><verb>
gtk_signal_connect(GTK_OBJECT( clist),
		   "select_row"
		   GTK_SIGNAL_FUNC(select_row_callback),
		   NULL);
</verb></tscreen>

<!-- ----------------------------------------------------------------- -->
<sect1>A GtkCList example
<p>

<tscreen><verb>
dnl example_include(clist/clist.cc)
</verb></tscreen>
