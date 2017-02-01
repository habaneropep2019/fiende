<!-- ***************************************************************** -->
<sect>Widget Overview
<!-- ***************************************************************** -->

<p>
In this chapter, we briefly introduce the concept of <em/packing/,
which is the primary mechanism for window layout in GTK--.  We also
present a brief catalog of the most commonly used GTK-- widgets.

<!-- ----------------------------------------------------------------- -->
<sect1>Packing (light)

<p>
If you've played with GTK applications, you've probably noticed that
GTK windows seem "elastic"; they can usually be stretched in many
different ways.  This ability is due to the <em/widget packing/
system, a concept borrowed (with several other concepts) from TK.

Most GUI toolkits allow you to place widgets on a window in arbitrary
positions.  In Delphi or Visual C++, for example, you place a button
on a form by specifying its coordinates relative to the upper
left-hand corner of the window.  While this is easy to understand, and
gives you quite a bit of flexibility, it also has some disadvantages:

<itemize>
<item>It's possible for widgets to overlap.  This is very seldom what
you want.

<item>It's difficult to make a window resizable.

<item>It's easy to make a window with unaligned widgets; this makes
your program look disorganised.  To avoid this problem, one must spend
time laying out and aligning each widget.  Changing the layout of a
window can be a tedious process.

<item>Changing the layout of a window "on the fly" can be a
non-trivial operation.

<item>Using a form painter to lay out your windows is all but a
requirement.
</itemize>

<p>
GTK uses the packing system to solve these problems.  Rather than
making you specify the position and size of each widget in the window,
you can instead tell GTK to arrange your widgets in rows, columns,
and/or tables.  GTK can size your window automatically, based on the
sizes of the widgets it contains.  You can further tune your layout by
specifying padding distance, centering values, and minimum and maximum
sizes for each of your widgets, among other things.  GTK then uses
all this information to do the right thing when the user resizes a
window.

GTK arranges windows hierarchically, using <em/containers/.  A
container is a widget which contains other widgets.  Most GTK widgets
are containers; windows, borders, notebook tabs, and buttons are all
container widgets.  There are two flavours of containers in GTK:
single-child containers, which are all descendants of <tt/Gtk::Bin/,
and multiple-child containers, which are descendants of
<tt/Gtk::Container/.  Most widgets in GTK are descendants of
<tt/Gtk::Bin/, including <tt/Gtk::Window/.

Yes, that's correct: a GTK window can contain at most one widget.
How, then, can we use a window for anything useful?  By placing a
multiple-child container in the window.  GTK-- offers several basic
multi-child containers:

<itemize>
<item><tt/Gtk::VBox/ places its widgets one on top of another.

<item><tt/Gtk::HBox/ arranges its widgets side-by-side.

<item><tt/Gtk::Packer/ packs its widgets in TK-style.

<item><tt/Gtk::Fixed/ allows you to place widgets inside it the
old-fashioned way, using coordinates.

<item><tt/Gtk::Table/ arranges its widgets in a grid.
</itemize>

<p> There are quite a few other containers in GTK--; see <ref
id="sec_ContainerOverview" name="Container Widgets"> for a complete
discussion of them.

If you've never used a packing toolkit before, it can take a bit
of getting used to.  You'll probably find, however, that you don't
need to rely on form-painters with GTK quite as much as you did with
other toolkits.  For a complete discussion of packing and packing
theory, see the section <ref id="sec_Packing_Widgets" name="Packing
Widgets">.  For now, we'll just mention a few of the most common
packing-related calls, because we'll be using them in our upcoming
examples:

<itemize>
<item><tt/Gtk::Box::pack_start()/ places a widget near the top for
<tt/VBox/es, and near the left for <tt/HBox/es.

<item><tt/Gtk::Box::pack_end()/ places a widget near the bottom for
<tt/VBox/es, and near the right for <tt/HBox/es.
</itemize>

Essentially, successive calls to <tt/pack_start()/ will result in your
widgets being "packed in" in a row or column.

<!-- ----------------------------------------------------------------- -->
<sect1>Using a GTK-- widget

<p>
The steps for using a GTK-- widget are:

<enum>
<item> Declare a variable of the type of the widget you wish to use.

<item> Connect the signals and events you wish to use to the
appropriate handlers, and/or subclass the widget and override the
appropriate <tt/_impl/ methods.

<item> Set the attributes of the widget.

<item> Pack the widget into a container using the appropriate call,
e.g. <tt/Gtk::Container::add()/ or <tt/Gtk::Box::pack_start()/.

<item> Call <tt/Gtk::Widget::show()/ to display the widget.
</enum>

<tt/Gtk::Widget::show()/ lets GTK-- know that we are done setting the
attributes of the widget, and that it is ready to be displayed. You
can use <tt/Gtk::Widget::hide()/ to make it disappear again. The order
in which you show the widgets is not important, but we do suggest
that you show the window last; this way, the whole window
will appear with its contents already drawn.  Otherwise, the user will
first see a blank window, and then the widgets will begin to appear
in it, which doesn't look quite as nice.

<!-- ----------------------------------------------------------------- -->
<sect1>Widget Hierarchy
<p>
Here is the class hierarchy for GTK-- widgets:

<tscreen><verb>
 Gtk::Object
  +Gtk::Widget
  | +Gtk::Misc
  | | +Gtk::Label
  | | | +Gtk::AccelLabel
  | | | `Gtk::TipsQuery
  | | +Gtk::Arrow
  | | +Gtk::Image
  | | `Gtk::Pixmap
  | +Gtk::Container
  | | +Gtk::Bin
  | | | +Gtk::Alignment
  | | | +Gtk::Frame
  | | | | `Gtk::AspectFrame
  | | | +Gtk::Button
  | | | | +Gtk::ToggleButton
  | | | | | `Gtk::CheckButton
  | | | | |   `Gtk::RadioButton
  | | | | `Gtk::OptionMenu
  | | | +Gtk::Item
  | | | | +Gtk::MenuItem
  | | | | | +Gtk::CheckMenuItem
  | | | | | | `Gtk::RadioMenuItem
  | | | | | `Gtk::TearoffMenuItem
  | | | | +Gtk::ListItem
  | | | | `Gtk::TreeItem
  | | | +Gtk::Window
  | | | | +Gtk::ColorSelectionDialog
  | | | | +Gtk::Dialog
  | | | | | `Gtk::InputDialog
  | | | | +Gtk::DrawWindow
  | | | | +Gtk::FileSelection
  | | | | +Gtk::FontSelectionDialog
  | | | | `Gtk::Plug
  | | | +Gtk::EventBox
  | | | +Gtk::HandleBox
  | | | +Gtk::ScrolledWindow
  | | | `Gtk::Viewport
  | | +Gtk::Box
  | | | +Gtk::ButtonBox
  | | | | +Gtk::HButtonBox
  | | | | `Gtk::VButtonBox
  | | | +Gtk::VBox
  | | | | +Gtk::ColorSelection
  | | | | `Gtk::GammaCurve
  | | | `Gtk::HBox
  | | |   +Gtk::Combo
  | | |   `Gtk::Statusbar
  | | +Gtk::CList
  | | | `Gtk::CTree
  | | +Gtk::Fixed
  | | +Gtk::Notebook
  | | | `Gtk::FontSelection
  | | +Gtk::Paned
  | | | +Gtk::HPaned
  | | | `Gtk::VPaned
  | | +Gtk::Layout
  | | +Gtk::List
  | | +Gtk::MenuShell
  | | | +Gtk::MenuBar
  | | | `Gtk::Menu
  | | +Gtk::Packer
  | | +Gtk::Socket
  | | +Gtk::Table
  | | +Gtk::Toolbar
  | | `Gtk::Tree
  | +Gtk::Calendar
  | +Gtk::DrawingArea
  | | `Gtk::Curve
  | +Gtk::Editable
  | | +Gtk::Entry
  | | | `Gtk::SpinButton
  | | `Gtk::Text
  | +Gtk::Ruler
  | | +Gtk::HRuler
  | | `Gtk::VRuler
  | +Gtk::Range
  | | +Gtk::Scale
  | | | +Gtk::HScale
  | | | `Gtk::VScale
  | | `Gtk::Scrollbar
  | |   +Gtk::HScrollbar
  | |   `Gtk::VScrollbar
  | +Gtk::Separator
  | | +Gtk::HSeparator
  | | `Gtk::VSeparator
  | +Gtk::Preview
  | `Gtk::Progress
  |   `Gtk::ProgressBar
  +Gtk::Data
  | +Gtk::Adjustment
  | `Gtk::Tooltips
  `Gtk::ItemFactory
</verb></tscreen>

<!-- ----------------------------------------------------------------- -->
<sect1>Widgets Without Windows

<p> The following widgets do not have an associated window; they
therefore do not receive events.  If you want to capture events for
these widgets, you can use a special container called
<tt/Gtk::EventBox/, which is described in the section <ref
id="sec_EventBox" name="EventBox">.

<tscreen><verb>
Gtk::Alignment
Gtk::Arrow
Gtk::Bin
Gtk::Box
Gtk::Image
Gtk::Item
Gtk::Label
Gtk::Pixmap
Gtk::ScrolledWindow
Gtk::Separator
Gtk::Table
Gtk::AspectFrame
Gtk::Frame
Gtk::VBox
Gtk::HBox
Gtk::VSeparator
Gtk::HSeparator
</verb></tscreen>

These widgets are mainly used for decoration, so you won't often need
to capture events on them.

