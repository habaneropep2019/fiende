<!-- ***************************************************************** -->
<sect>Container widgets in GTK-- <label id="sec_ContainerWidgets">
<!-- ***************************************************************** -->

<p>
Some GUI toolkits focus on the ability to place arbitrary widgets on a
display area.  Container widgets are often something of an
afterthought in these toolkits; they require special handling and
special, sometimes obtuse, programming techniques.

GTK takes a different approach.  Not only does it give you container
widgets, almost <em/every/ widget in it is a container of some sort.
Widget containment and hierarchy are fundamental parts of the GTK
design; understanding how GTK containment widgets work is therefore
essential to understanding the GTK-- toolkit.

We've already covered many things about this aspect of GTK/GTK--.  Now
it's time to discuss GTK--'s container widget system in detail.

<sect1>The GTK-- container widget system<label id="sec_ContainerOverview">

<p>
Building a hierarchical widget system is not easy.  Making it simple
to use is downright difficult.  GTK-- breaks the problem down by
defining two basic types of containers:

<itemize>

<item><bf/Single-item containers/ can contain at most one widget.
They all descend from the <tt/Gtk::Bin/ class.

<item><bf/Multiple-item containers/ can contain zero or more widgets.
They all descend from the <tt/Gtk::Container/ class.

</itemize>

It's common for newcomers to GTK-- to wonder what the purpose of the
<tt/Gtk::Bin/ class is.  Why not use multi-item containers for
everything?  Knowing the answer to this question is an important key
to understanding GTK/GTK--.

First, let's consider the alternative: making all containers
multi-item.  This approach is taken by most toolkits that implement
some sort of hierarchy for their widgets.  This is possible because,
in general, such toolkits only implement a single <em/placement
policy/ for their widgets: what, in GTK--, is called the <em/fixed/
policy, which leaves it up to the programmer to place widgets at the
proper screen coordinates.

GTK--, by contrast, provides multiple placement policies, implemented
primarily by its multiple-item container widgets.  There are many of
these: GTK-- provides horizontal boxes, vertical boxes, tables,
packboxes, fixed layout areas, and others.  This helps the programmer
to easily construct clean-looking interfaces, without even using a
form-painter.

Now, if all GTK-- containers were multiple-item, we'd have to
answer some questions about each one, such as:

<itemize>
<item>What should the placement policy be?
<item>Should the policy be selectable?
<item>Should we make it possible to define new policies?  What's the best
way to do this?
<item>What widgets should be containers?
</itemize>

.. and so forth.  These are just a few of the problems that might
arise if we tried to dispense with the concept of single-item
containers.  The existence of the <tt/Gtk::Bin/ class enables GTK to
sidestep these thorny issues.

A single-item container in GTK-- is not a second-class citizen, and
it's not really handicapped in any way.  In fact, a <tt/Gtk::Bin/
container isn't really limited to containing just one widget.  If we
place, in a <tt/Gtk::Bin/, one of the multiple-item containers,
our <tt/Gtk::Bin/ can contain more than one widget after all.
Furthermore, we can select the placement policy for those widgets by
merely using the proper multi-item container widget.

Making a <tt/Gtk::Bin/ widget is much easier than making a multi-item
container.  We need deal with only one child, so we don't have to
write code to handle multiple children, or provide any sort of access
to them.  We don't need to worry about placing or displaying the
children; that will be taken care of by a multi-item child.

This means that it's practical to make many widgets containers which
in other toolkits wouldn't be.  Take the lowly button widget, for
example.  Very few - if any - toolkits consider buttons to be a
container widget.  GTK--, however, does, and it's almost too easy not
to: by simply making a button a derivative of <tt/Gtk::Bin/, it
doesn't even have to provide code for displaying a label or picture;
this can be done by a child pixmap or label widget.  A
<tt/Gtk::Button/ can even contain both at the same time, using any
placement policy the programmer desires.  Consider how impractical
this would be without <tt/Gtk::Bin/!

Of course, sometimes you don't want multiple children in a single-item
container widget; one, or even zero, is enough.  Most buttons only
contain one child, that being a label or a picture.  Because it
doesn't require its container widgets to support multiple children,
GTK lets you avoid paying for what you don't use.

Now consider the GTK-- window widget.  We've already seen that GTK
windows can contain at most one child.  Now you know that, far from
being a limitation, this is actually a <em/strength/ of GTK, thanks to
the <tt/Gtk::Bin/ concept.  

We summarise this section with a profound statement that composer
Brian Eno once made:

<tscreen>
<em/"Let your limitations be your secret strengths."/
</tscreen>

... which GTK certainly does, in this case.

<sect1>Using <tt/Gtk::Bin/ widgets

<p>
<tt/Gtk::Bin/ derives from a class called <tt/Gtk::Container/, which
is a base class for all widgets which contain other widgets.  It
provides virtual methods for operations such as adding and removing
children and obtaining contained widgets, among other things.

Unlike human parents, <tt/Gtk::Bin/-derived widgets don't have much to
take care of when it comes to their only children.  <tt/Gtk::Bin/
widgets use the following two methods to add and remove their child
widgets:

<tscreen><verb>
void Gtk::Bin::add(Gtk::Widget&amp; p0);
void Gtk::Bin::remove();
</verb></tscreen>

<tt/Gtk::Bin/ also provides some convenience functions for adding
labels and pixmaps (a common thing to do with <tt/Gtk::Bin/ widgets):

<tscreen><verb>
void add_label(const string &amp;label, gfloat x=0.0, gfloat y=0.5);
void add_pixmap(const Gdk_Pixmap &amp;pixmap, const Gdk_Bitmap &amp;bitmap);
void add_pixlabel(const Gdk_Pixmap &amp;pixmap, const Gdk_Bitmap &amp;bitmap,
                  const string &amp;label, gfloat x=0.0, gfloat y=0.5);
void add_pixlabel(const string &amp;pixfile, 
                  const string &amp;label, gfloat x=0.0, gfloat y=0.5);
</verb></tscreen>

These are similar to the constructors for the <ref id="sec_Pixmaps" 
name="Pixmap"> and <ref id="sec_Labels" name="Label"> widgets; see 
those sections for details.

<sect1>Understanding the GTK-- multiple-item widgets

<p> Multiple-item widgets inherit from <tt/Gtk::Container/; just as
with <tt/Gtk::Bin/, you use the <tt/add()/ and <tt/remove()/ methods
to add and remove contained widgets.  Unlike <tt/Gtk::Bin::remove()/,
however, the <tt/remove()/ method for <tt/Gtk::Container/ takes an
argument:

<tscreen><verb>
void Gtk::Container::add(Gtk::Widget&amp; p0);
void Gtk::Container::remove(Gtk::Widget&amp; p0);
</verb></tscreen>

The argument for <tt/remove()/ specifies which widget to remove, since
there can be more than one.

Multiple-item containers have a bit more to do than <tt/Gtk::Bin/
widgets, since they can have more than one child (something which
those of you with families can no doubt appreciate).  The <tt/add()/
and <tt/remove()/ methods give you a way to put things in and take
things out, but what if you need to access the contained widgets
(which you probably will)?

If you're an accomplished C++ programmer, you'll be happy to hear that
most of the GTK-- widgets which maintain lists of things do so
STL-style (if you don't know STL, learn it!).  They don't necessarily
use actual STL containers (there are good reasons for this), but the
lists they use look, feel, and act, for the most part, just like STL
container classes.

There is, however, a major difference between GTK-- containers and
STL containers.  Normally, when you use a <tt/vector/, for example,
you expect that whatever you put in, you'll get out, unmodified.  You
wouldn't make a <tt/vector&lt;int&gt;/ and expect to get <tt/double/s
out of it.  Strangely enough, GTK-- containers don't always work like
that.  In fact, it's perfectly normal to put one kind of object into a
GTK-- container, and to later get a different kind out.  Why this odd
behaviour?

Consider a menu widget, which must maintain a hierarchical list of
menus and menu items.  Menus can't just contain any widget at all;
they can only contain certain objects, like menu items, separators,
and submenus.  To ensure consistency, what's needed is a "filter" to
keep out illegal objects.  Also, since only a few types of objects are
allowed, convenience functions can be provided to make it easy to
build up menus from scratch.

GTK-- takes care of both requirements in one stroke using special
<em/helper objects/, or <em/elements/.  Helper objects are
temporary.  They're constructed and passed to a list insertion function
(typically in the same call); the list insertion function uses
the information in the helper object to construct the <em/real/
object, which is then inserted into the list.

As an example, let's look at the Notebook widget (explained in the
section on <ref id="sec_Notebooks" name="Notebook widgets">).  As
we'll see later on, notebook widgets consist of a series of "pages",
which are "stacked" on top of each other, so that only one is visible
at a time.  The pages are selected by clicking on marked "tabs".

Each page in a notebook requires, at minimum, the following
information:

<itemize>

<item>A child widget (zero or one), to be placed in the page

<item>A label for the page's tab

</itemize>

(The GTK-- notebook widget keeps other data for each page as well.)

To insert a new page in a notebook, we can use one of the notebook
helper classes, like this:

<tscreen><verb>
notebook->pages().push_back(
          Gtk::Notebook_Helpers::TabElem(*frame,bufferl));
</verb></tscreen>

(This line comes from the notebook example program.)  Let's see what's
going on here.  Assume we have a pointer to a Notebook widget called
<tt/notebook/; we go from that to a member function called
<tt/pages()/, which returns an STL-like list object.  On this we call
the function <tt/push_back()/ (this should be familiar to those of you
who know STL).

The object that the <tt/pages()/ method returns is called a
<tt/Notebook_Helpers::PageList/.  It's one of the STL-like containers
that we keep referring to.  Let's take a look at some of the parts of
this class (this has been heavily edited for clarity; see
<tt>&lt;gtk--/notebook.h&gt;</tt> for the actual definition):

<tscreen><verb>
namespace Notebook_Helpers
{
    class PageList
    {
    public:
             . . .
        void push_back(const Element& e);
             . . .
        Page* operator[](size_type l);
    };
};
</verb></tscreen>

There are two important things to notice here:

<itemize>
<item>The <tt/push_back()/ method takes as argument an <tt/Element/
object (helper);
<item>The overloaded <tt/[]/ operator returns .. a pointer to a
<tt/Page/!
</itemize>

So what does a <tt/PageList/ actually hold?  It holds (as far as the
programmer is concerned) <tt/Page/s, objects which contain all the
necessary data about a notebook page.  The twist with using the
Notebook widget (and most other GTK-- multi-item containers) is that
you aren't allowed to construct <tt/Page/ objects, and then insert
them; you have to insert <tt/Element/ objects instead.  The reasons
for this are numerous, and mainly involve the technical aspects of
wrapping the GTK+ lists in C++ classes; the specifics don't concern us
here.

This scheme of storing different objects than we put in has some
important advantages:

<itemize>

<item>We can provide as many different Helper objects as desired,
making it simple to construct complex widgets like Menus.

<item>Construction of the actual objects can be performed by the list
at any appropriate time, enabling us to sidestep certain difficulties
with GTK+.

<item>The definitions of the objects contained in the list can change;
their interfaces need not concern the programmer.  For example, even
if the <tt/Page/ object changes drastically, the programmer need not
be concerned; the <tt/Element/s need not change, and will continue to
work as expected.

<item>New <tt/Element/ objects can be added at any time to support new
features, without breaking existing code.

</itemize>

Not bad, eh?  Perhaps the design isn't so odd after all!

<sect1>Using the GTK-- multiple-item widgets <label id="sec_UsingMultiWidgets">

<p>
Each multiple-item container in GTK-- (with a few exceptions) contains
a GTK-- list, which holds information for the contained elements.
These lists work almost exactly like ordered STL containers - so much
so, in fact, that rather than explaining them in detail, we can refer
you to the STL documentation for most of their methods (saving
ourselves a lot of typing in the process!).

At minimum, GTK-- container lists support iterators and the usual
complement of insertion, deletion, and addition methods.  You can
always expect the following methods to be available for GTK-- lists:

<itemize>
<item><tt/begin()/ returns a <tt/begin/ iterator
<item><tt/end()/ returns an <tt/end/ iterator
<item><tt/rbegin()/ returns a reverse <tt/begin/ iterator
<item><tt/rend()/ returns a reverse <tt/end/ iterator
<item><tt/size()/
<item><tt/max_size()/
<item><tt/empty()/
<item><tt/insert()/
<item><tt/push_front()/
<item><tt/push_back()/
<item><tt/pop_front()/
<item><tt/pop_back()/
<item><tt/clear()/
<item><tt/erase()/
<item><tt/remove()/
<item><tt/find()/
<item><tt/front()/
<item><tt/back()/
</itemize>

Also, the <tt/[]/ operator is overloaded; note that it's usually order
N, so if performance is a consideration, or the list has a large
number of elements, think carefully before using it.

The element objects and list objects are defined, for each container,
in a namespace whose name ends in <tt/_Helpers/.  For example, the
helper namespace for the notebook widget is called
<tt/Notebook_Helpers/.  Several element types are usually provided;
the Notebook widget provides three:

<itemize>
<item><tt/Gtk::Notebook_Helpers::Element/
<item><tt/Gtk::Notebook_Helpers::TabElem/
<item><tt/Gtk::Notebook_Helpers::MenuElem/
</itemize>

<tt/TabElem/ and <tt/MenuElem/ inherit from <tt/Element/; the
insertion functions take an <tt/Element/ reference as argument.  All
multi-item containers have an <tt/Element/ object in their helper
namespaces; usually there are additional "sub-element" objects
available (like <tt/TabElem/ and <tt/MenuElem/) which derive from
<tt/Element/.  <tt/Element/ classes vary from container to container,
since each container contains different kinds of objects; we'll
document these as we come to them.

A common point of confusion with new GTK-- users is over the nature of
<tt/Element/s.  It's very important to remember that <tt/Element/s are
<em/not/ "real" objects; they exist only temporarily, and they're
never stored by multi-item widgets.  They are used <em/only/ as
temporary "parameter-holders".  Therefore, the following segment of
code is illegal:

<tscreen><verb>
MenuElem *m= new MenuElem("hello");
m->right_justify();
items().push_back(*m);
</verb></tscreen>

We constructed a new <tt/MenuElem/ helper object, and then
tried to invoke <tt/right_justify()/ on it before adding it to the
menu.  The trouble is that there is no <tt/right_justify()/ method
in the <tt/MenuElem/ class!  The correct way to accomplish this would
be:

<tscreen><verb>
items().push_back(MenuElem("hello"));
items().back()->right_justify();
</verb></tscreen>

Here, we've constructed a <tt/MenuElem/ and inserted it into the menu
by passing it to <tt/push_back()/, causing the <em/real/ menu item to
be created.  We've then called <tt/right_justify()/ on the object
retrieved from the list.  This is correct; the object retrieved from
the list is <em/not/ a <tt/MenuElem/, but a <tt/MenuItem/, which is a
"real" menu item object, and therefore supports the
<tt/right_justify()/ method as expected.  (Fortunately, the right way
is actually shorter than the wrong way, in this case!)

<sect1>Container widgets: a rogues' gallery

<p>
In the next two chapters, we'll be looking at each of GTK--'s
container widgets in detail.  The <tt/Gtk::Bin/ container widgets we'll be
discussing are:

<itemize>
<item>Frames
<item>Aspect frames
<item>Event boxes
<item>The Alignment widget
<item>Viewports
<item>Scrolled windows
<item>Panes
</itemize>

We've already encountered some <tt/Gtk::Bin/ widgets: buttons (all
types), and windows.

Panes are a special case; they actually contain <em/two/ child
widgets, but since they don't really maintain a list of children,
we'll discuss them along with the single-child widgets.

The multiple-child containers we'll discuss are:

<itemize>
<item>VBoxes and HBoxes
<item>Packers
<item>Tables
<item>The Layout & Fixed widgets
<item>Button boxes
<item>Trees & CTrees
<item>Lists & CLists
<item>Toolbars
<item>Menus
<item>Notebooks
</itemize>

Of that list, there are three widgets that do not use STL-like
container lists: button boxes, the Layout widget, and the Fixed
widget.
