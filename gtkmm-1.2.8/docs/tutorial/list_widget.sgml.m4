<!-- ***************************************************************** -->
<sect> List Widget
<!-- ***************************************************************** -->
<p>
NOTE: The Gtk::List widget has been superseded by the GtkCList
widget. It is detailed here just for completeness.

The Gtk::List widget is designed to act as a vertical container for
widgets that should be of the type Gtk::ListItem.

A Gtk::List widget has its own window to receive events and its own
background color which is usually white. As it is directly derived
from a GtkContainer it can be treated as such by using the
GTK_CONTAINER(List) macro, see the GtkContainer widget for more on
this. One should already be familiar with the usage of a GList and
its related functions g_list_*() to be able to use the Gtk::List widget
to it full extent.

There is one field inside the structure definition of the Gtk::List
widget that will be of greater interest to us, this is:

<tscreen><verb>
struct _Gtk::List
{
  ...
  GList *selection;
  guint selection_mode;
  ...
}; 
</verb></tscreen>

The selection field of a Gtk::List points to a linked list of all items
that are currently selected, or NULL if the selection is empty.  So to
learn about the current selection we read the GTK_LIST()->selection
field, but do not modify it since the internal fields are maintained
by the gtk_list_*() functions.

The selection_mode of the Gtk::List determines the selection facilities
of a Gtk::List and therefore the contents of the GTK_LIST()->selection
field. The selection_mode may be one of the following:

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
                        on.

<item> GTK_SELECTION_EXTENDED - The selection is always NULL.
</itemize>

The default is GTK_SELECTION_MULTIPLE.

<!-- ----------------------------------------------------------------- -->
<sect1> Signals
<p>
<tscreen><verb>
void selection_changed( Gtk::List *list );
</verb></tscreen>

This signal will be invoked whenever the selection field of a Gtk::List
has changed. This happens when a child of the Gtk::List got selected or
deselected.

<tscreen><verb>
void select_child( Gtk::List   *list,
                   GtkWidget *child);
</verb></tscreen>

This signal is invoked when a child of the Gtk::List is about to get
selected. This happens mainly on calls to gtk_list_select_item(),
gtk_list_select_child(), button presses and sometimes indirectly
triggered on some else occasions where children get added to or
removed from the Gtk::List.

<tscreen><verb>
void unselect_child( Gtk::List   *list,
                     GtkWidget *child );
</verb></tscreen>

This signal is invoked when a child of the Gtk::List is about to get
deselected. This happens mainly on calls to gtk_list_unselect_item(),
gtk_list_unselect_child(), button presses and sometimes indirectly
triggered on some else occasions where children get added to or
removed from the Gtk::List.

<!-- ----------------------------------------------------------------- -->
<sect1> Functions
<p>
<tscreen><verb>
guint gtk_list_get_type( void );
</verb></tscreen>

Returns the `Gtk::List' type identifier.

<tscreen><verb>
GtkWidget *gtk_list_new( void );
</verb></tscreen>

Create a new Gtk::List object. The new widget is returned as a pointer
to a GtkWidget object. NULL is returned on failure.

<tscreen><verb>
void gtk_list_insert_items( Gtk::List *list,
                            GList   *items,
                            gint     position );
</verb></tscreen>

Insert list items into the list, starting at <tt/position/.
<tt/items/ is a doubly linked list where each nodes data pointer is
expected to point to a newly created Gtk::ListItem.  The GList nodes of
<tt/items/ are taken over by the list.

<tscreen><verb>
void gtk_list_append_items( Gtk::List *list,
                            GList   *items);
</verb></tscreen>

Insert list items just like gtk_list_insert_items() at the end of the
list. The GList nodes of <tt/items/ are taken over by the list.

<tscreen><verb>
void gtk_list_prepend_items( Gtk::List *list,
                             GList   *items);
</verb></tscreen>

Insert list items just like gtk_list_insert_items() at the very
beginning of the list. The GList nodes of <tt/items/ are taken over by
the list.

<tscreen><verb>
void gtk_list_remove_items( Gtk::List *list,
                            GList   *items);
</verb></tscreen>

Remove list items from the list. <tt/items/ is a doubly linked list
where each nodes data pointer is expected to point to a direct child
of list. It is the callers responsibility to make a call to
g_list_free(items) afterwards. Also the caller has to destroy the list
items himself.

<tscreen><verb>
void gtk_list_clear_items( Gtk::List *list,
                           gint start,
                           gint end );
</verb></tscreen>

Remove and destroy list items from the list. A widget is affected if
its current position within the list is in the range specified by
<tt/start/ and <tt/end/.

<tscreen><verb>
void gtk_list_select_item( Gtk::List *list,
                           gint     item );
</verb></tscreen>

Invoke the select_child signal for a list item specified through its
current position within the list.

<tscreen><verb>
void gtk_list_unselect_item( Gtk::List *list,
                             gint     item);
</verb></tscreen>

Invoke the unselect_child signal for a list item specified through its
current position within the list.

<tscreen><verb>
void gtk_list_select_child( Gtk::List *list,
                            GtkWidget *child);
</verb></tscreen>

Invoke the select_child signal for the specified child.

<tscreen><verb>
void gtk_list_unselect_child( Gtk::List   *list,
                              GtkWidget *child);
</verb></tscreen>

Invoke the unselect_child signal for the specified child.

<tscreen><verb>
gint gtk_list_child_position( Gtk::List *list,
                              GtkWidget *child);
</verb></tscreen>

Return the position of <tt/child/ within the list. "-1" is returned on
failure.

<tscreen><verb>
void gtk_list_set_selection_mode( Gtk::List         *list,
                                  GtkSelectionMode mode );
</verb></tscreen>

Set the selection mode MODE which can be of GTK_SELECTION_SINGLE,
GTK_SELECTION_BROWSE, GTK_SELECTION_MULTIPLE or
GTK_SELECTION_EXTENDED.

<tscreen><verb>
Gtk::List *GTK_LIST( gpointer obj );
</verb></tscreen>

Cast a generic pointer to `Gtk::List *'. *Note Standard Macros::, for
more info.

<tscreen><verb>
Gtk::ListClass *GTK_LIST_CLASS( gpointer class);
</verb></tscreen>

Cast a generic pointer to `Gtk::ListClass*'. *Note Standard Macros::,
for more info.

<tscreen><verb>
gint GTK_IS_LIST( gpointer obj);
</verb></tscreen>

Determine if a generic pointer refers to a `Gtk::List' object. *Note
Standard Macros::, for more info.

<!-- ----------------------------------------------------------------- -->
<sect1> Example
<p>
Following is an example program that will print out the changes of the
selection of a Gtk::List, and lets you "arrest" list items into a prison
by selecting them with the rightmost mouse button.

example_incl_scr(`list/list.cc')

<!-- ----------------------------------------------------------------- -->
<sect1> List Item Widget
<p>
The Gtk::ListItem widget is designed to act as a container holding up to
one child, providing functions for selection/deselection just like the
Gtk::List widget requires them for its children.

A Gtk::ListItem has its own window to receive events and has its own
background color which is usually white.

As it is directly derived from a GtkItem it can be treated as such by
using the GTK_ITEM(ListItem) macro, see the GtkItem widget for more on
this. Usually a Gtk::ListItem just holds a label to identify e.g. a
filename within a Gtk::List -- therefore the convenience function
gtk_list_item_new_with_label() is provided. The same effect can be
achieved by creating a GtkLabel on its own, setting its alignment to
xalign=0 and yalign=0.5 with a subsequent container addition to the
Gtk::ListItem.

As one is not forced to add a GtkLabel to a Gtk::ListItem, you could
also add a GtkVBox or a GtkArrow etc. to the Gtk::ListItem.

<!-- ----------------------------------------------------------------- -->
<sect1> Signals
<p>
A Gtk::ListItem does not create new signals on its own, but inherits
the signals of a GtkItem. *Note GtkItem::, for more info.

<!-- ----------------------------------------------------------------- -->
<sect1> Functions
<p>
<tscreen><verb>
guint gtk_list_item_get_type( void );
</verb></tscreen>

Returns the `Gtk::ListItem' type identifier.

<tscreen><verb>
GtkWidget *gtk_list_item_new( void );
</verb></tscreen>

Create a new Gtk::ListItem object. The new widget is returned as a
pointer to a GtkWidget object. NULL is returned on failure.

<tscreen><verb>
GtkWidget *gtk_list_item_new_with_label( gchar *label );
</verb></tscreen>

Create a new Gtk::ListItem object, having a single GtkLabel as the sole
child. The new widget is returned as a pointer to a GtkWidget
object. NULL is returned on failure.

<tscreen><verb>
void gtk_list_item_select( Gtk::ListItem *list_item );
</verb></tscreen>

This function is basically a wrapper around a call to gtk_item_select
(GTK_ITEM (list_item)) which will emit the select signal.  *Note
GtkItem::, for more info.

<tscreen><verb>
void gtk_list_item_deselect( Gtk::ListItem *list_item );
</verb></tscreen>

This function is basically a wrapper around a call to
gtk_item_deselect (GTK_ITEM (list_item)) which will emit the deselect
signal.  *Note GtkItem::, for more info.

<tscreen><verb>
Gtk::ListItem *GTK_LIST_ITEM( gpointer obj );
</verb></tscreen>

Cast a generic pointer to `Gtk::ListItem*'. *Note Standard Macros::, for
more info.

<tscreen><verb>
Gtk::ListItemClass *GTK_LIST_ITEM_CLASS( gpointer class );
</verb></tscreen>

Cast a generic pointer to Gtk::ListItemClass*. *Note Standard Macros::,
for more info.

<tscreen><verb>
gint GTK_IS_LIST_ITEM( gpointer obj );
</verb></tscreen>

Determine if a generic pointer refers to a `Gtk::ListItem' object.
*Note Standard Macros::, for more info.
 
<!-- ----------------------------------------------------------------- -->
<sect1> Example
<p>
Please see the Gtk::List example on this, which covers the usage of a
Gtk::ListItem as well.
