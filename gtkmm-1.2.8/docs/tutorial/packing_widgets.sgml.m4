<!-- ***************************************************************** -->
<sect>Packing widgets <label id="sec_Packing_Widgets">
<!-- ***************************************************************** -->

<p> When creating an application, you'll want to put more than one
widget inside a window. Our first <em>helloworld</em> example only
used one widget, so we could simply use a <tt/Gtk::Container::add()/
call to "pack" the widget into the window. But when you want to put
more than one widget into a window, how do you control where the
widgets are positioned?  This is where packing comes in.

<!-- ----------------------------------------------------------------- -->
<sect1>Theory of Packing Boxes

<p>
Most packing is done by creating boxes as in the example above. These
are invisible widget containers that we can pack our widgets into
which come in two forms, a horizontal box, and a vertical box. When
packing widgets into a horizontal box, the objects are inserted
horizontally from left to right or right to left depending on the call
used. In a vertical box, widgets are packed from top to bottom or vice
versa. You may use any combination of boxes inside or beside other
boxes to create the desired effect.

To create a new horizontal box, we use a <tt/Gtk::HBox/, and for
vertical boxes, a <tt/Gtk::VBox/.  The <tt/Gtk::Box::pack_start()/ and
<tt/Gtk::Box::pack_end()/ methods are used to place objects inside
these containers. The <tt/Gtk::Box::pack_start()/ method will start at
the top and work its way down in a vbox, and pack left to right in an
hbox.  <tt/Gtk::Box::pack_end()/ will do the opposite, packing from
bottom to top in a vbox, and right to left in an hbox. Using these
methods allows us to right justify or left justify our widgets; the
methods may be mixed in any way to achieve the desired effect. We will
use <tt/Gtk::Box::pack_start()/ in most of our examples. An object may
be another container or a widget. In fact, many widgets are actually
containers themselves, including the button, but we usually only place
a label inside a button.

By using these calls, GTK-- knows where you want to place your widgets
so it can do automatic resizing and other nifty things. There are also
a number of options covering how your widgets should be packed. As you
can imagine, this gives us a quite a bit of flexibility when placing
and creating widgets.

<!-- ----------------------------------------------------------------- -->
<sect1>Details of Boxes

<p>
Because of this flexibility, packing boxes in GTK-- can be confusing at
first. There are a lot of options, and it's not immediately obvious how
they all fit together. In the end, however, there are basically five
different styles, as shown in this picture:

<? <CENTER> >
<?
<IMG SRC="gtk_tut_packbox1.png" VSPACE="15" HSPACE="10"
ALT="Box Packing Example Image">
>
<? </CENTER> >

Each line contains one horizontal box (hbox) with several
buttons. Each of the buttons is packed into the hbox the same way
(i.e. same arguments to the <tt/Gtk::Box::pack_start()/ method).

This is the declaration of <tt/Gtk::Box::pack_start()/ method:

<tscreen><verb>
void Gtk::Box::pack_start(const Gtk::Widget &amp;child,
                          bool              expand = FALSE,
                          bool              fill = FALSE,
                          gint              padding = 0);
</verb></tscreen>

The first argument is the object you're packing. The objects will all
be buttons for now, so we'll be packing buttons into boxes.

If <tt/expand/ is set, the packed widgets will be spaced out evenly
across the width/length of the hbox/vbox, but their sizes won't change
(there will be empty space between the widgets).  <tt/fill/ has an
effect only if <tt/expand/ is set; if it's <tt/true/, then the packed
widgets will be resized so that there will be no space between them.
<tt/padding/ specifies the width of a border-area to leave around the
packed widget.

Here's the constructor for the box widgets:

<tscreen><verb>
Gtk::Box(bool homogeneous,
         gint spacing);
</verb></tscreen>

Passing <tt/true/ for <tt/homogeneous/ will force all of the contained
widgets to be the same size.  <tt/spacing/ is a (minimum) number of
pixels to leave between each widget.

What's the difference between spacing (set when the box is created)
and padding (set when elements are packed)? Spacing is added between
objects; padding is added on either side of an object. The following
figure should make it clearer:

<? <CENTER> >
<?
<IMG ALIGN="center" SRC="gtk_tut_packbox2.png"
VSPACE="15" HSPACE="10"
ALT="Box Packing Example Image">
>
<? </CENTER> >

Here is the code used to create the above images:

example_incl_src(packbox/packbox.cc)

<!-- ----------------------------------------------------------------- -->
<sect1>Packing Using Tables

<p> Let's take a look at another way of packing: tables. These can be
extremely useful in certain situations.  Using tables, we create a
grid that we can place widgets in. The widgets may take up as many
spaces as we specify.

Here is the constructor for <tt/Gtk::Table/:

<tscreen><verb>
Gtk::Table(gint rows,
           gint columns,
           bool homogeneous);
</verb></tscreen>

The first argument is the number of rows to make in the table, while
the second, obviously, is the number of columns.  If <tt/homogeneous/
is <tt/true/, the table boxes are forced to all be the same size
(i.e. the size of the largest widget in the table).

The rows and columns are indexed starting at 0.  If you specify
<tt/rows/ = 2 and <tt/columns/ = 2, the layout would look something
like this:

<tscreen><verb>
 0          1          2
0+----------+----------+
 |          |          |
1+----------+----------+
 |          |          |
2+----------+----------+
</verb></tscreen>

Note that the coordinate system starts in the upper left hand corner.
To place a widget into a box, use the following function:

<tscreen><verb>
void Gtk::Table::attach(Gtk::Widget &amp;child,
                        guint left_attach,
                        guint right_attach,
                        guint top_attach,
                        guint bottom_attach,
                        guint xoptions=(GTK_FILL|GTK_EXPAND),
                        guint yoptions=(GTK_FILL|GTK_EXPAND),
                        guint xpadding=0,
                        guint ypadding=0);
</verb></tscreen>				       

The first argument is the widget you wish to place in the table.

The <tt/left_attach/ and <tt/right_attach/ arguments specify where to
place the widget, and how many boxes to use.  For example, if you want
a button in the lower-right cell of a 2x2 table, and want it to occupy
that cell <em/only/, then <tt/left_attach/ would be 1,
<tt/right_attach/ 2, <tt/top_attach/ 1, and <tt/bottom_attach/ 2.  If,
on the other hand, you wanted a widget to take up the entire top row
of our 2x2 table, you'd set <tt/left_attach/ = 0, <tt/right_attach/ =
2, <tt/top_attach/ = 0, and <tt/bottom_attach/ = 1.

<tt/xoptions/ and <tt/yoptions/ are used to specify packing options
and may be bitwise ORed together to allow multiple options.
These options are:

<descrip>

<tag><tt/GTK_FILL/</tag>
If the table box is larger than the widget, and
<tt/GTK_FILL/ is specified, the widget will expand to use all the room
available.

<tag><tt/GTK_SHRINK/</tag>
If the table widget was allocated less
space than was requested (usually by the user resizing the window),
then the widgets would normally just be pushed off the bottom of the
window and disappear. If <tt/GTK_SHRINK/ is specified, the widgets
will shrink with the table.

<tag><tt/GTK_EXPAND/</tag>
This will cause the table to expand to use up any
remaining space in the window.

</descrip>

The padding arguments work just as they do for boxes.

<tt/Gtk::Table::set_row_spacing()/ and
<tt/Gtk::Table::set_col_spacing()/ set the spacing between the rows at
the specified row or column:

<tscreen><verb>
void Gtk::Table::set_row_spacing(gint row,
                                 gint spacing);
void Gtk::Table::set_col_spacing(gint column,
                                 gint spacing);
</verb></tscreen>

Note that for columns, the space goes to the right of the column, and
for rows, the space goes below the row.

You can also set a consistent spacing of all rows and/or columns with:

<tscreen><verb>
void Gtk::Table::set_row_spacings(gint spacing );
void Gtk::Table::set_col_spacings(gint spacing );
</verb></tscreen>

Note that with these calls, the last row and last column do not get
any spacing.

In the following example, we make a window with three buttons in a 2x2
table.  The first two buttons will be placed in the upper row.  A
third button is placed in the lower row, spanning both columns.  It
should look something like this:

<? <CENTER> >
<?
<IMG SRC="gtk_tut_table.png" VSPACE="15" HSPACE="10" 
ALT="Table Packing Example Image">
>
<? </CENTER> >

example_incl_src(table/table.cc)
