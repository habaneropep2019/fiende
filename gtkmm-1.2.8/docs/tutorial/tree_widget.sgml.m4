<sect> Tree Widget (draft) <label id="sec_Tree_Widgets">
<!-- ***************************************************************** -->
<p>
The purpose of tree widgets is to display hierarchically-organized
data. The Gtk::Tree widget itself is a vertical container for widgets of
type Gtk::TreeItem. Gtk::Tree itself is not terribly different from
Gtk::List - both are derived directly from Gtk::Container, and the
Gtk::Container methods work in the same way on Gtk::Tree widgets as on
Gtk::List widgets. The difference is that Gtk::Tree widgets can be nested
within other Gtk::Tree widgets. We'll see how to do this shortly.

The Gtk::Tree widget has its own window, and defaults to a white
background, as does Gtk::List. Also, most of the Gtk::Tree methods work in
the same way as the corresponding Gtk::List ones. However, Gtk::Tree is
not derived from Gtk::List, so you cannot use them interchangeably.

<sect1> Creating a Tree
<p>
A Gtk::Tree is created in the usual way, using:

<tscreen><verb>
Gtk::Tree();
</verb></tscreen>

Like the Gtk::List widget, a Gtk::Tree will simply keep growing as more
items are added to it, as well as when subtrees are expanded.  For
this reason, they are almost always packed into a
Gtk::ScrolledWindow. You might want to use Gtk::Widget::set_usize() on the
scrolled window to ensure that it is big enough to see the tree's
items, as the default size for Gtk::ScrolledWindow is quite small.

Now that you have a tree, you'll probably want to add some items to
it.  <ref id="sec_Tree_Item_Widget" name="The Tree Item Widget"> below
explains the gory details of Gtk::TreeItem. For now, it'll suffice to
create one, using:

<tscreen><verb>
Gtk::TreeItem( const string &amp;label );
</verb></tscreen>

You can then add it to the tree using one of the following (see
<ref id="sec_Gtk::Tree_Functions" name="Methods">
below for more options):

<tscreen><verb>
void Gtk::Tree::append( const Gtk::Widget &amp;tree_item );

void Gtk::Tree::prepend( const Gtk::Widget &amp;tree_item );
</verb></tscreen>

Note that you must add items to a Gtk::Tree one at a time - there is no
equivalent to Gtk::List::*_items().

<!-- ----------------------------------------------------------------- -->
<sect1> Adding a Subtree
<p>
A subtree is created like any other Gtk::Tree widget. A subtree is added
to another tree beneath a tree item, using:

<tscreen><verb>
void Gtk::TreeItem::set_subtree(const Gtk::Tree &amp;subtree );
</verb></tscreen>

You do not need to call Gtk::Widget::show() on a subtree before or after
adding it to a Gtk::TreeItem. However, you <em>must</em> have added the
Gtk::TreeItem in question to a parent tree before calling
Gtk::TreeItem::set_subtree(). This is because, technically, the parent
of the subtree is <em>not</em> the Gtk::TreeItem which "owns" it, but
rather the Gtk::Tree which holds that Gtk::TreeItem.

When you add a subtree to a Gtk::TreeItem, a plus or minus sign appears
beside it, which the user can click on to "expand" or "collapse" it,
meaning, to show or hide its subtree. Gtk::TreeItems are collapsed by
default. Note that when you collapse a Gtk::TreeItem, any selected
items in its subtree remain selected, which may not be what the user
expects.

<!-- ----------------------------------------------------------------- -->
<sect1> Handling the Selection List
<p>
As with Gtk::List, the Gtk::Tree type has a <tt>selection</tt> field, and
it is possible to control the behaviour of the tree (somewhat) by
setting the selection type using:

<tscreen><verb>
void Gtk::Tree::set_selection_mode(GtkSelectionMode  mode );
</verb></tscreen>

The selection field of a Gtk::Tree points to a linked list of all items
that are currently selected, or NULL if the selection is empty.  So to
learn about the current selection we use:

<tscreen><verb>
SelectionList& selection()
</verb></tscreen>

The selection_mode of the Gtk::List determines the selection facilities
of a Gtk::List and therefore the contents of the selection. The
selection_mode may be one of the following:

<itemize>
<item> GTK_SELECTION_SINGLE - The selection is either NULL
                        or contains a GList pointer
                        for a single selected item.

<item> GTK_SELECTION_BROWSE -  The selection is NULL if the list
                        contains no widgets or insensitive
                        ones only, otherwise it contains
                        a GList pointer for one GList
                        structure, and therefore exactly
                        one list item.

<item> GTK_SELECTION_MULTIPLE -  The selection is NULL if no list
                        items are selected or a GList pointer
                        for the first selected item. That
                        in turn points to a GList structure
                        for the second selected item and so
                        

<item> GTK_SELECTION_EXTENDED - The selection is always NULL.
</itemize>

The default is GTK_SELECTION_MULTIPLE.

The "select_child", "unselect_child" (not exactly - see <ref
id="sec_Gtk::Tree_Signals" name="Signals"> below for an explanation),
and "selection_changed" signals are emitted when list items are
selected or unselected. However, in order to take advantage of these
signals, you need to know <em>which</em> Gtk::Tree widget they will be
emitted by.

This is a source of potential confusion. All Gtk::Tree widgets have
their own X window, and can therefore receive events such as mouse
clicks (if their Gtk::TreeItems or their children don't catch them
first!). However, to make GTK_SELECTION_SINGLE and GTK_SELECTION_BROWSE
selection types behave in a sane manner, the list of selected items is
specific to the topmost Gtk::Tree widget in a hierarchy, known as the
"root tree".

Thus, accessing the <tt>selection</tt> directly in an arbitrary
Gtk::Tree widget is not a good idea unless you <em>know</em> it's the
root tree.  The GTK_TREE_SELECTION (Tree) macro does not yet have an
equivalent in Gtk--, the program must remember the root tree.

Note that the selection list can include items that are not in the
subtree in question if the selection type is GTK_SELECTION_MULTIPLE.

Finally, the "select_child" (and "unselect_child", in theory) signals
are emitted by all trees, but the "selection_changed" signal is only
emitted by the root tree. Consequently, if you want to handle the
"select_child" signal for a tree and all its subtrees, you will have
to connect it for every subtree.

<sect1> Tree Widget Internals
<p>
The perils associated with obtaining the selection have already been
mentioned. The other important fields of the tree can also be accessed
with member functions, however some Gtk+ macros do not yet have a
Gtk-- equivalent.

GTK_TREE_IS_ROOT_TREE (Tree) and GTK_TREE_ROOT_TREE (Tree) have no Gtk--
equivalents, so remember to save a reference or pointer to the root tree.

The children of a tree are all Gtk::TreeItems, they are accessed from
their parent using:

<tscreen><verb>
ItemList& tree();
</verb></tscreen>

Gtk::TreeHelpers::ItemList is a wrapper around the Glib list of children
which acts like an STL list. So, for example, the list of items can be
iterated over using:

<tscreen><verb>
  Gtk::Tree_Helpers::ItemList::iterator ItemIt;

  for(ItemIt = a_tree->tree().begin(); ItemIt != a_tree->tree().end(); ItemIt++)
  {
    // operate on each item, access as *ItemIt
  }
</verb></tscreen>

The Gtk+ <tt>tree_owner</tt> field is defined only in subtrees, where it
points to the treeitem which holds the tree in question. It currently cannot
be accessed in Gtk--.

The <tt>level</tt> field indicates how deeply nested a particular tree
is; root trees have level 0, and each successive level of subtrees has
a level one greater than the parent level. This field is set only
after a Gtk::Tree widget is actually mapped (i.e. drawn on the screen).
Gtk::Tree::get_level() returns the tree's level.

<sect2> Signals<label id="sec_Gtk::Tree_Signals">
<p>
<tscreen><verb>
void Gtk::tree::selection_changed();
</verb></tscreen>

This signal will be emitted whenever the selection of a
Gtk::Tree has changed. This happens when a child of the Gtk::Tree is
selected or deselected.

<tscreen><verb>
void Gtk::Tree::select_child(Gtk::Widget& child);
</verb></tscreen>

This signal is emitted when a child of the Gtk::Tree is about to get
selected. This happens on calls to Gtk::Tree::select_item(),
Gtk::Tree::select_child(), on <em>all</em> button presses and calls to
Gtk::Item::toggle().  It may sometimes be indirectly triggered on other
occasions where children get added to or removed from the Gtk::Tree.

<tscreen><verb>
void Gtk::Tree::unselect_child (Gtk::Widget& child);
</verb></tscreen>

This signal is emitted when a child of the Gtk::Tree is about to get
deselected. As of Gtk-- 1.0.4, this seems to only occur on calls to
gtk_tree_unselect_item() or gtk_tree_unselect_child(), and perhaps on
other occasions, but <em>not</em> when a button press deselects a
child, nor on emission of the "toggle" signal by gtk_item_toggle().

<sect2> Methods<label id="sec_Gtk::Tree_Functions">
<p>
Methods on ItemList are also given, which are used to manipulate the
items in the tree.

<tscreen><verb>
Gtk::Tree();
</verb></tscreen>

Gtk::Tree ctor

<tscreen><verb>
void Gtk::Tree::tree().push_back(const Gtk::Widget &amp;tree_item );
</verb></tscreen>

Append a tree item to a Gtk::Tree.

<tscreen><verb>
void Gtk::Tree::tree().push_front(const Gtk::Widget &amp;tree_item );
</verb></tscreen>

Prepend a tree item to a Gtk::Tree.

<tscreen><verb>
iterator Gtk::Tree::tree().insert(iterator position,
                      const Gtk::Widget &amp;tree_item );
</verb></tscreen>

Insert a tree item into a Gtk::Tree at the position in the list
specified by <tt>position.</tt>

<tscreen><verb>
template <class iterator>
void Gtk::Tree::remove_items(iterator start,
                           iterator stop);
</verb></tscreen>

Remove a list of items from a Gtk::Tree. The iterator must be to
Gtk_TreeItem* and be forward iteratable. Note that removing an item
from a tree dereferences (and thus usually) destroys it <em>and</em>
its subtree, if it has one, <em>and</em> all subtrees in that subtree.
If you want to remove only one item, you can use gtk_container_remove().
Note that there is no STL equivalent to this function.

<tscreen><verb>
void Gtk::Tree::tree().erase(iterator start,
                           iterator stop );
</verb></tscreen>

Remove the items from position <tt>start</tt> to position <tt>stop</tt>
from a Gtk::Tree.  The same warning about dereferencing applies here.

<tscreen><verb>
void Gtk::Tree::tree()[item]->select();
</verb></tscreen>

Emits the "select_item" signal for the child at position
<tt>item</tt>, thus selecting the child (unless you unselect it in a
signal handler).

<tscreen><verb>
void Gtk::Tree::tree()[item]->unselect();
</verb></tscreen>

Emits the "unselect_item" signal for the child at position
<tt>item</tt>, thus unselecting the child.

<tscreen><verb>
gint Gtk::Tree::child_position(Gtk::TreeItem& child) const;
</verb></tscreen>

Returns the position of <tt>child</tt> in the item list. Can be used
to select, unselect, etc. children by reference. Returns -1 if
<tt>child</tt> is not in the tree.

<tscreen><verb>
void Gtk::Tree::set_selection_mode(GtkSelectionMode  mode );
</verb></tscreen>

Sets the selection mode, which can be one of GTK_SELECTION_SINGLE (the
default), GTK_SELECTION_BROWSE, GTK_SELECTION_MULTIPLE, or
GTK_SELECTION_EXTENDED. This is only defined for root trees, which
makes sense, since the root tree "owns" the selection. Setting it for
subtrees has no effect at all; the value is simply ignored.

<tscreen><verb>
void Gtk::Tree::set_view_mode(GtkTreeViewMode  mode ); 
</verb></tscreen>

Sets the "view mode", which can be either GTK_TREE_VIEW_LINE (the
default) or GTK_TREE_VIEW_ITEM.  The view mode propagates from a tree
to its subtrees, and can't be set exclusively to a subtree (this is
not exactly true - see the example code comments).

The term "view mode" is rather ambiguous - basically, it controls the
way the highlight is drawn when one of a tree's children is selected.
If it's GTK_TREE_VIEW_LINE, the entire Gtk::TreeItem widget is
highlighted, while for GTK_TREE_VIEW_ITEM, only the child widget
(i.e. usually the label) is highlighted.

<tscreen><verb>
void Gtk::Tree::set_view_lines(guint    flag );
</verb></tscreen>

Controls whether connecting lines between tree items are drawn.
<tt>flag</tt> is either TRUE, in which case they are, or FALSE, in
which case they aren't.

The following are Gtk+ macros which have not yet been implemented in Gtk--.
They can be applied using Tree::gtkobj(), however if you really need them
ask the Gtk-- development team, or write them yourself.

<tscreen><verb>
gint GTK_IS_ROOT_TREE (gpointer obj)
</verb></tscreen>

Determine if a generic pointer refers to a `Gtk_Tree'
<em>and</em> is a root tree. Though this will accept any pointer, the
results of passing it a pointer that does not refer to a Gtk_Tree are
undefined and possibly harmful.

<tscreen><verb>
Gtk_Tree *GTK_TREE_ROOT_TREE (gpointer obj)
</verb></tscreen>

Return the root tree of a pointer to a `Gtk_Tree' object. The above
warning applies.

<sect1> Tree Item Widget<label id="sec_Tree_Item_Widget">
<p>
The Gtk::TreeItem widget, like Gtk::ListItem, is derived from Gtk::Item,
which in turn is derived from Gtk::Bin.  Therefore, the item itself is a
generic container holding exactly one child widget, which can be of
any type.  The Gtk::TreeItem widget has a number of extra members, but
the only one we need be concerned with is the <tt>subtree</tt>.

You can always obtain the subtree of a Gtk::TreeItem in a
type-safe manner with the get_subtree() method. Gtk-- does
not have an interface to the other internals of Gtk_TreeItem,
and you shouldn't need to worry about them.

A Gtk::TreeItem usually holds a label, so the constructor
TreeItem(const string &amp;label, gfloat x=0.0, gfloat y=0.5) is
provided. The same effect can be achieved using code like the
following:

<tscreen><verb>
tree_item = new Gtk::TreeItem();
label_widget = new Gtk::Label(label);
label.set_alignment (0.0, 0.5);

tree_item.add (label_widget);
label.show();
</verb></tscreen>

You are not forced to add a GtkLabel to a Gtk::TreeItem, you could
instead add a GtkHBox or a GtkArrow, or even a GtkNotebook (though your
app will likely be quite unpopular in this case).

If you remove all the items from a subtree, it will be destroyed and
unparented, unless you reference it beforehand, and the Gtk::TreeItem
which owns it will be collapsed.  So, if you want it to stick around,
do something like the following:

<tscreen><verb>
tree->ref();
owner = tree->get_owner();  //!!!! No get_owner method
tree->remove (item);
if (tree->get_parent() == NULL){
  owner.expand ();
  owner.set_subtree (tree);
}
else
  tree->unref();
</verb></tscreen>

Finally, drag-n-drop <em>does</em> work with Gtk::TreeItems.  However,
you have to make sure that the Gtk::TreeItem you want to make into a drag
item or a drop site has not only been added to a Gtk::Tree, but that
each successive parent widget has a parent itself, all the way back to
a toplevel or dialog window, when you call Gtk::Widget::drag_source_set()
or Gtk::Widget::drag_dest__set().  Otherwise, strange things will happen.

<sect2> Signals
<p>
Gtk::TreeItem inherits the "select", "deselect", and "toggle" signals
from GtkItem.  In addition, it adds two signals of its own, "expand"
and "collapse".

<tscreen><verb>
void Gtk::Item::select();
</verb></tscreen>

This signal is emitted when an item is about to be selected, either
after it has been clicked on by the user, or when the program calls
Gtk::Tree::item_select(), Gtk::Item::select(), or Gtk::Tree::select_child().

<tscreen><verb>
void Gtk::Item::deselect();
</verb></tscreen>

This signal is emitted when an item is about to be unselected, either
after it has been clicked on by the user, or when the program calls
Gtk::Tree::item_deselect() or Gtk::Item::deselect(). In the case of
Gtk::TreeItems, it is also emitted by Gtk::Tree::unselect_child(), and
sometimes Gtk::Tree::select_child().

<tscreen><verb>
void Gtk::Item::toggle();
</verb></tscreen>

This signal is emitted when the program calls Gtk::Item::toggle().  The
effect it has when emitted on a Gtk::TreeItem is to call
Gtk::Tree::select_child() (and never Gtk::Tree::unselect_child()) on the
item's parent tree, if the item has a parent tree.  If it doesn't,
then the highlight is reversed on the item.

<tscreen><verb>
void Gtk::TreeItem::expand();
</verb></tscreen>

This signal is emitted when the tree item's subtree is about to be
expanded, that is, when the user clicks on the plus sign next to the
item, or when the program calls Gtk::TreeItem::expand().

<tscreen><verb>
void Gtk::TreeItem::collapse();
</verb></tscreen>

This signal is emitted when the tree item's subtree is about to be
collapsed, that is, when the user clicks on the minus sign next to the
item, or when the program calls Gtk::TreeItem::collapse().

<sect2> Functions and Macros
<p>
<tscreen><verb>
static GtkType Gtk::TreeItem::get_type();
</verb></tscreen>

Returns the `Gtk::TreeItem' type identifier.

<tscreen><verb>
Gtk::TreeItem();
</verb></tscreen>

Constructor for Gtk::TreeItem object.

<tscreen><verb>
Gtk::TreeItem(const string &amp;label,gfloat x=0.0,gfloat y=0.5);
</verb></tscreen>

Creates a new Gtk::TreeItem object, having a single GtkLabel as the sole
child.

<tscreen><verb>
void Gtk::Item::select();
</verb></tscreen>

This function emits the select signal.

<tscreen><verb>
void Gtk::Item::deselect();
</verb></tscreen>

This function emits the deselect signal.

<tscreen><verb>
void Gtk::TreeItem::set_subtree(Gtk::Widget&amp; subtree);
</verb></tscreen>

This function adds subtree to the TreeItem, showing it if the TreeItem is
expanded, or hiding it if collapsed. Again, remember that the TreeItem must
have already been added to a tree for this to work.

<tscreen><verb>
void Gtk::TreeItem::remove_subtree();
</verb></tscreen>

This removes all of the TreeItem's subtree's children (thus unreferencing
and destroying it, any of its children's subtrees, and so on...), then
removes the subtree itself, and hides the plus/minus sign.

The members below are not yet implemented in Gtk::TreeItem.

<tscreen><verb>
void gtk_tree_item_expand( Gtk::TreeItem *tree_item );
</verb></tscreen>

This emits the "expand" signal on tree_item, which expands it.

<tscreen><verb>
void gtk_tree_item_collapse( Gtk::TreeItem *tree_item );
</verb></tscreen>

This emits the "collapse" signal on tree_item, which collapses it.

<tscreen><verb>
Gtk::TreeItem *GTK_TREE_ITEM (gpointer obj)
</verb></tscreen>

Cast a generic pointer to `Gtk::TreeItem*'.

<tscreen><verb>
Gtk::TreeItemClass *GTK_TREE_ITEM_CLASS (gpointer obj)
</verb></tscreen>

Cast a generic pointer to `Gtk::TreeItemClass'.

<tscreen><verb>
gint GTK_IS_TREE_ITEM (gpointer obj)
</verb></tscreen>

Determine if a generic pointer refers to a `Gtk::TreeItem' object.
 
<tscreen><verb>
GtkWidget GTK_TREE_ITEM_SUBTREE (gpointer obj)
</verb></tscreen>

Returns a tree item's subtree (obj should point to a `Gtk::TreeItem'
object).

<sect1> Tree Example
<p>
This is somewhat like the tree example in testgtk.c, but a lot less
complete (although much better commented).  It puts up a window with a
tree, and connects all the signals for the relevant objects, so you
can see when they are emitted.

<tscreen><verb>
example_include(tree/tree.cc)
</verb></tscreen>

<!-- ***************************************************************** -->
