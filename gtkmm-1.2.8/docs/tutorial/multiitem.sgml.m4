<!-- ***************************************************************** -->
<sect>Multiple-item widgets <label id="sec_MultiItemWidgets">
<!-- ***************************************************************** -->

<p> In this chapter, we discuss some of the multiple-item widgets in
GTK--.  The <ref id="sec_Notebooks" name="notebooks"> and <ref
id="sec_Toolbars" name="toolbars"> sections in this chapter assume a
decent understanding of the material in the <ref
id="sec_ContainerWidgets" name="container widgets"> chapter; go back
and read it if you haven't already.

<!-- ----------------------------------------------------------------- -->   
<sect1> Fixed Container <label id="sec_FixedContainer">

<p>
The Fixed widget places its children at relative coordinates, which you
specify.  The widgets can also be moved.

The constructor for <tt/Gtk::Fixed/ is:

<tscreen><verb>
Gtk::Fixed();
</verb></tscreen>

Place children in the Fixed container using

<tscreen><verb>
void Gtk::Fixed::put(const Gtk::Widget &amp;widget,
                     gint16            x,
                     gint16            y);
</verb></tscreen>

You can then move them using

<tscreen><verb>
void Gtk::Fixed::move(const Gtk::Widget &amp;widget,
                      gint16            x,
                      gint16            y);
</verb></tscreen>

Although the Fixed container is a multiple-item widget, it doesn't
provide STL-like access to its child widgets.  If you'll need access
to the widgets you put in the Fixed container, it will probably be a
good idea for you to maintain your own list of them.

The following example illustrates how to use <tt/Gtk::Fixed/:

example_incl_src(fixed/fixed.cc)

<!-- ----------------------------------------------------------------- -->
<sect1> Layout Container <label id="sec_LayoutContainer">

<p>
The Layout container is similar to the Fixed container except that it
implements an infinite (where infinity is less than 2^32) scrolling
area. Xwindows has a limitation where windows can be at most 32767
pixels wide or tall. The Layout container gets around this limitation
by doing some exotic stuff using window and bit gravities, so that you
can have smooth scrolling even when you have many child widgets in
your scrolling area.

The Layout container operates similarly to the <ref id="sec_Viewports"
name="Viewport">, but it contains multiple widgets instead of just
one.  If you'll be placing a large number of widgets in the viewing
area, the Layout container is a good choice.

A Layout container is created using:

<tscreen><verb>
Gtk::Layout(Gtk::Adjustment &amp;hadjustment,
            Gtk::Adjustment &amp;vadjustment);
Gtk::Layout();
</verb></tscreen>

These are exactly like the Viewport constructors, and work the same.

You can add and move widgets in the Layout container using the
following two methods, which work the same as their counterparts in
<tt/Gtk::Fixed/:

<tscreen><verb>
void Gtk::Layout::put(const Gtk::Widget &amp;widget,
                      gint              x,
                      gint              y);

void Gtk::Layout::move(const Gtk::Widget &amp;widget,
                       gint              x,
                       gint              y);
</verb></tscreen>

Unlike the Viewport and Fixed widgets, the maximum size of the Layout
widget can be changed.  This is done using:

<tscreen><verb>
void Gtk::Layout::set_size(guint width,
                           guint height);
</verb></tscreen>

Layout containers are one of the very few widgets in GTK-- that
actively repaint themselves as they are being changed (the vast
majority of the GTK-- widgets queue redrawing requests, which are
processed when control returns to the <tt/gtk_main()/ function).
Therefore, when you're making a large number of changes in a Layout
container, it will repaint itself for each one.  This is inefficient
and unnecessary.  To temporarily stop the Layout container from
repainting itself, call

<tscreen><verb>
void Gtk::Layout::freeze();
</verb></tscreen>

Once you're done making your changes, call

<tscreen><verb>
void Gtk::Layout::thaw();
</verb></tscreen>

Calling <tt/thaw()/ after a <tt/freeze()/ causes the Layout widget to
repaint itself, making all your changes visible in one go.

You can use the following four methods to get and set the Layout
container's adjustment widgets:

<tscreen><verb>
Gtk::Adjustment* Gtk::Layout::get_hadjustment();
Gtk::Adjustment* Gtk::Layout::get_vadjustment();
void Gtk::Layout::set_hadjustment(Gtk::Adjustment &amp;adjustment);
void Gtk::Layout::set_vadjustment(Gtk::Adjustment &amp;adjustment);
</verb></tscreen>

As with the Fixed container, Layout has no STL-like list associated
with it; if you need a list of its child widgets, you're on your
own.

<!-- ----------------------------------------------------------------- -->   
<sect1>Button boxes <label id="sec_ButtonBoxes">

<p>
Button boxes are a convenient way to quickly arrange a group of
buttons. They come in both horizontal (<tt/Gtk::HButtonBox/) and
vertical (<tt/Gtk::VButtonBox/) flavours; the two types are exactly
alike, except in name and orientation.

One interesting feature of button boxes is that all of them in a
given program can share certain settings, such as inter-button
spacing.  The idea is that you can impose consistency on your
interface this way.  Methods which change a given setting for all
button boxes have <tt/default/ in their names.

The constructors for button boxes are:

<tscreen><verb>
Gtk::HButtonBox();
Gtk::VButtonBox();
</verb></tscreen>

Buttons are added to a button box using the <tt/add()/ method:

<tscreen><verb>
Gtk::ButtonBox::add(GtkWidget &amp;child);
</verb></tscreen>

You can set the spacing between the buttons using

<tscreen><verb>
void Gtk::ButtonBox::set_spacing_default(gint spacing);
gint Gtk::ButtonBox::get_spacing_default();
gint get_spacing();
void set_spacing(gint spacing);
</verb></tscreen>

<tt/set_spacing_default()/ and <tt/get_spacing_default()/ operate on
<em/every/ button box in your program.  Most of the button box
parameters have <tt/default/ methods like this; we'll only point
them out here.

You can set and get a minimum size for the buttons using

<tscreen><verb>
gint Gtk::ButtonBox::get_child_size_default_width();
gint Gtk::ButtonBox::get_child_size_default_height();
void Gtk::ButtonBox::set_child_size_default(gint min_width,gint min_height);
gint Gtk::ButtonBox::get_child_size_width();
gint Gtk::ButtonBox::get_child_size_height();
void Gtk::ButtonBox::set_child_size(gint min_width,gint min_height);
</verb></tscreen>

You can also change the padding for each button in the button box
using

<tscreen><verb>
gint Gtk::ButtonBox::get_child_ipadding_default_x();
gint Gtk::ButtonBox::get_child_ipadding_default_y();
void Gtk::ButtonBox::set_child_ipadding_default(gint ipad_x,gint ipad_y);
gint Gtk::ButtonBox::get_child_ipadding_x();
gint Gtk::ButtonBox::get_child_ipadding_y();
void Gtk::ButtonBox::set_child_ipadding(gint ipad_x,gint ipad_y);
</verb></tscreen>

Button boxes support several layout styles.  These can be retrieved
and changed using:

<tscreen><verb>
GtkButtonBoxStyle Gtk::ButtonBox::get_layout_default();
void Gtk::ButtonBox::set_layout_default(GtkButtonBoxStyle layout);
GtkButtonBoxStyle Gtk::ButtonBox::get_layout();
void Gtk::ButtonBox::set_layout(GtkButtonBoxStyle layout_style);
</verb></tscreen>

where <tt/layout/ is one of

<itemize>
<item><tt/GTK_BUTTONBOX_DEFAULT_STYLE/
<item><tt/GTK_BUTTONBOX_SPREAD/
<item><tt/GTK_BUTTONBOX_EDGE/
<item><tt/GTK_BUTTONBOX_START/
<item><tt/GTK_BUTTONBOX_END/
</itemize>

Run the example to see what these styles do.

There's also a function to set the layout and spacing at the same
time:

<tscreen><verb>
void set_layout_spacing(GtkButtonBoxStyle layout,
                        gint spacing);
</verb></tscreen>

(There is no <tt/default/ version of <tt/set_layout_spacing()/.)

Once again, button boxes don't provide an STL-like list container.

This example illustrates all the different layout settings
for button boxes.

example_incl_src(buttonbox/buttonbox.cc)

<!-- ----------------------------------------------------------------- -->   
<sect1>Toolbars <label id="sec_Toolbars">

<p>
Toolbars are those rows of buttons one often sees just beneath the
menubars of GUI applications.  They're meant to give the user quick
access to often-used commands, some of which might be buried deep in
the menubar hierarchy.

Most people think of toolbars as a row of little square buttons with
all-but-meaningless icons on them, such as those found in the products
of certain very large software vendors.  While GTK can't force your
icons to have obvious meanings, it can help you make your toolbars
less opaque, because GTK toolbars can contain almost <em/any/ type of
widget.

Still, toolbars do generally contain picture-buttons, so the GTK
toolbar provides special support for this GUI idiom.  In GTK--, you
can use the toolbar helper objects to create special toolbar items
having icons, labels, and tooltips.  You can then let your users
decide whether they want to see just the icons, or the icons and
labels, or just the labels, as certain web-browsers let you do.  This
way, once your users have finally learnt the meaning of your
pictograms, they can dispense with the captions, saving a bit of room
in the window.  (Picture-only toolbars also tend to look slicker in
screenshots, as certain very large software vendors know all too
well.)

For those of you interested in making meaningful icons - which I hope
includes nearly all of you that will be releasing GUI software to the
public - there's a classic work on the subject of symbols by the great
Adrain Frutiger (designer of the typefaces Univers and Frutiger),
entitled <em/Signs and Symbols: Their Design and Meaning/ (English
translation by Andrew Bluhm, published in the US by Watson-Guptill
Publications, ISBN 0823048268), which probably everybody who designs
anything graphic should read.

At any rate, here's the constructor for <tt/Gtk::Toolbar/:

<tscreen><verb>
Gtk::Toolbar(GtkOrientation orientation,
             GtkToolbarStyle style );
</verb></tscreen>

where <tt/orientation/ may be one of

<itemize>
<item><tt/GTK_ORIENTATION_HORIZONTAL/
<item><tt/GTK_ORIENTATION_VERTICAL/
</itemize>

and <tt/style/ may be one of

<itemize>
<item><tt/GTK_TOOLBAR_TEXT/
<item><tt/GTK_TOOLBAR_ICONS/
<item><tt/GTK_TOOLBAR_BOTH/
</itemize>

As previously noted, you can insert two types of elements into a
toolbar: toolbar items, and regular widgets.  (Note that <tt/style/
applies only to the specially made toolbar items and not to button
widgets inserted as widgets.)  Both types of elements are inserted the
same way, using classes from the <tt/Gtk::Toolbar_Helpers/ namespace.
The various helper objects are:

<itemize>
<item><tt/Element/ - used for inserting arbitrary widgets
<item><tt/Space/ - a blank spot, used to separate groups of elements
<item><tt/ButtonElem/ - a regular button element
<item><tt/ToggleElem/ - a toggle-button element
<item><tt/RadioElem/ - a radio-button element
</itemize>

Here's the constructor for <tt/Element/:

<tscreen><verb>
Element(Widget& w,
        const nstring &amp;tooltip_text=0,
        const nstring &amp;tooltip_private_text=0);
</verb></tscreen>

<tt/w/ is the widget to insert, and <tt/tooltip_text/ is the text for the
element's tooltip.  You can ignore <tt/tooltip_private_text/.

The constructors for <tt/ButtonElem/ and <tt/ToggleElem/ are exactly
alike; each has three forms.  Here are the <tt/ButtonElem/
constructors:

<tscreen><verb>
// text + icon
ButtonElem(const nstring &amp;text,
           Widget         &amp;content,
           SigC::Slot0<void>        callback,
           const nstring &amp;tooltip_text=0,
           const nstring &amp;tooltip_private_text=0);

// icon only
ButtonElem(Widget         &amp;content,
           SigC::Slot0<void>        callback,
           const nstring &amp;tooltip_text=0,
           const nstring &amp;tooltip_private_text=0);

// text only
ButtonElem(const nstring &amp;text,
           SigC::Slot0<void>        callback,
           const nstring &amp;tooltip_text=0,
           const nstring &amp;tooltip_private_text=0);
</verb></tscreen>

The only difference between these is whether they take an icon, text,
or both as arguments. <tt/text/ is the text to display below the
icon.  <tt/content/ is the icon; note that any widget can be inserted
here, but generally this will be a pixmap or other display widget.
<tt/callback/ is the callback to use for the button.
<tt/tooltip_text/ will be displayed in the button's tooltip, and you
can safely ignore <tt/tooltip_private_text/.

The <tt/RadioElem/ constructors are the same as those for
<tt/ButtonElem/ and <tt/RadioElem/, but they take an additional
argument specifying the group for the radio button.  Here they are:

<tscreen><verb>
// text + icon
RadioElem(Gtk::RadioButton_Helpers::Group&amp; group,
          const nstring&amp;      text,
          Widget&amp;             content,
          SigC::Slot0<void>   callback=0,
          const nstring&amp;      tooltip_text=0,
          const nstring&amp;      tooltip_private_text=0);

// icon only
RadioElem(Gtk::RadioButton_Helpers::Group&amp; group,
          Widget&amp;             content,
          SigC::Slot0<void>   callback=0,
          const nstring&amp;      tooltip_text=0,
          const nstring&amp;      tooltip_private_text=0);

// text only
RadioElem(Gtk::RadioButton_Helpers::Group&amp; group,
          const nstring&amp;      text,
          SigC::Slot0<void>   callback=0,
          const nstring&amp;      tooltip_text=0,
          const nstring&amp;      tooltip_private_text=0);
</verb></tscreen>

The <tt/group/ argument is the only addition here; it works exactly
like the <tt/group/ argument for normal radio buttons.  See the 
<ref id="sec_Radio_Buttons" name="Radio Buttons"> section for details.

The toolbar's contents are manipulated through an STL-like list, which
you can obtain using the <tt/tools()/ method:

<tscreen><verb>
ToolList&amp; tools();
</verb></tscreen>

For example, to add a text-only button tool to the toolbar, we could write

<tscreen><verb>
toolbar.tools().push_back(Gtk::Toolbar_Helpers::ButtonElem(
        "Crash",slot(&amp;crash_cb),"Causes the program to dump core");
</verb></tscreen>

Since it's inconvenient to have to type <tt/Gtk::Toolbar_Helpers/ all
the time, you might want to add a <tt/using/ declaration.  However,
<em/don't/ add a global <tt/using namespace Gtk::Toolbar_Helpers/
declaration; place this only in some localised scope, to avoid clashes
with other <tt/Helpers/ namespaces.

Here's an example program which displays a toolbar:

example_incl_src(toolbar/toolbar.cc)

<!-- ----------------------------------------------------------------- -->
<sect1> Notebooks <label id="sec_Notebooks">

<p>
A notebook consists of a set of stacked "pages", each of which can
contain one widget.  Labelled "tabs" are provided to allow the user to
select a page.  These are useful in a large number of situations; you
can make a single dialog do a lot of work using a notebook.

The <tt/Gtk::Notebook/ widget uses an STL-like list to contain the
pages; these are constructed using helper objects.  See the section on
<ref id="sec_UsingMultiWidgets" name="using multiple-item widgets">
for information on how GTK-- lists and helper objects work; the
notebook widget is used there as an example.

The constructor for <tt/Gtk::Notebook/ takes no arguments:

<tscreen><verb>
Gtk::Notebook();
</verb></tscreen>

To insert pages into a notebook, use the <tt/TabElem/ helper:

<tscreen><verb>
Gtk::Notebook_Helpers::TabElem(Widget&amp; child,Widget&amp; tab);
Gtk::Notebook_Helpers::TabElem(Widget&amp; child,const string&amp; s);
</verb></tscreen>

In both constructors, <tt/child/ is the child widget to place in the
page itself.  <tt/tab/, in the first constructor, is a widget to be
placed in the tab; this will generally be a display widget such as an
icon or a label.  Most often, you'll be placing labels in a tab, and
the second constructor lets you pass a string for argument <tt/s/;
using this constructor will cause a label to be built from the string.

You can access the notebook's page list through the method

<tscreen><verb>
Gtk::Notebook_Helpers::PageList&amp; Gtk::Notebook::pages();
</verb></tscreen>

A <tt/PageList/ contains a list of <tt/Page/s, which provide some
useful methods (shown unqualified):

<tscreen><verb>
Widget* get_child();
Widget* get_tab();

void set_tab (Widget* tab=0);
void set_tab (Widget&amp; tab);
void set_tab_text(const nstring&amp; str=0);

bool get_expand();
bool get_fill();  
GtkPackType get_pack();
void set_tab_packing(bool expand,bool fill,GtkPackType pack_type);
</verb></tscreen>

<tt/get_child()/ and <tt/get_tab()/ return the child and tab widgets
for the <tt/Page/, respectively.

<tt/set_tab()/ lets you change a tab's widget.  You can pass either a
pointer to the widget, or the widget itself.  <tt/set_tab_text/ sets
the tab to contain a new label, the text of which will be <tt/str/.

<tt/set_tab_packing/ lets you change the packing parameters of the
tab-list.  If <tt/expand/ is set, then the tabs will be spaced out
evenly across the edge they're on; if <tt/fill/ is set, the tabs
themselves will be sized such that there will be no space between
them.  <tt/pack_type/ can be either <tt/GTK_PACK_START/ or
<tt/GTK_PACK_END/.

There are a number of things you can do to the notebook widget itself.
GTK notebooks allow their tabs to be placed along any edge of the
notebook's display area; you can set or retrieve the placement using

<tscreen><verb>
void Gtk::Notebook::set_tab_pos(GtkPositionType pos);
GtkPositionType Gtk::Notebook::get_tab_pos();
</verb></tscreen>

where <tt/pos/ is one of

<itemize>
<item><tt/GTK_POS_LEFT/
<item><tt/GTK_POS_RIGHT/
<item><tt/GTK_POS_TOP/
<item><tt/GTK_POS_BOTTOM/
</itemize>

Here are some other useful <tt/Gtk::Notebook/ methods (shown
unqualified):

<tscreen><verb>
gint get_current_page_num();
</verb></tscreen>

Returns the index of the current page.

<tscreen><verb>
Gtk::Widget* get_nth_page(gint page_number);
</verb></tscreen>

Returns the child widget of page <tt/page_number/.

<tscreen><verb>
gint page_num(const Gtk::Widget&amp; child);
</verb></tscreen>

Returns the page number of the page containing the given child widget.

<tscreen><verb>
void set_page(gint page_number);
void next_page();
void prev_page();
</verb></tscreen>

<tt/set_page/ selects page number <tt/page_number/.  <tt/next_page()/
and <tt/prev_page()/ cause the next and previous pages to be selected,
respectively.  Both functions wrap around if they are on the first or
last pages.

<tscreen><verb>
Page* get_current();
</verb></tscreen>

Returns the <tt/Page/ object for the currently selected page.

<tscreen><verb>
bool get_show_tabs();
void set_show_tabs(bool show_tabs);
</verb></tscreen>

Set or retrieve the visibility of the tab-list.

<tscreen><verb>
bool get_show_border();
void set_show_border(bool show_border);
</verb></tscreen>

Notebooks have a border, which includes the tab-list; use
<tt/set_show_border()/ to change its visibility.

<tscreen><verb>
void set_homogeneous_tabs(bool homogeneous);
</verb></tscreen>

Forces all of the tabs to be the same size.

<tscreen><verb>
void set_tab_border(gint border_width);
void set_tab_hborder(guint tab_hborder);
void set_tab_vborder(guint tab_vborder);
</verb></tscreen>

<tscreen><verb>
void set_scrollable(bool scrollable);
</verb></tscreen>

Sets whether the tab-list will be scrollable if there are more tabs
than can be displayed all at once.

<tscreen><verb>
void popup_enable();
void popup_disable();
</verb></tscreen>

The notebook can have an optional popup menu which will display when
the user right-clicks on the tab-list; the menu will contain a list of
all the pages.  This can be useful if there are a large number of
pages in the notebook.  You can enable or disable the popup menu with
these functions.

As you can see, there's rather a lot to the notebook widget.  We
haven't been exhaustive here; see the GTK-- reference manual and
source code for more information.

It's that time again.  This example program demonstrates the use of
the notebook widget.

example_incl_src(notebook/notebook.cc)
