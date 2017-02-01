<sect>Menu Widget
<!-- ***************************************************************** -->
<p>
There are two ways to create menus, there's the easy way, and there's
the hard way. Both have their uses, but you can usually use the
itemfactory (the easy way). The "hard" way is to create all the menus
using the calls directly. The easy way is to use the gtk_item_factory
calls. This is much simpler, but there are advantages and
disadvantages to each approach.

The itemfactory is much easier to use, and to add new menus to,
although writing a few wrapper functions to create menus using the
manual method could go a long way towards usability. With the
itemfactory, it is not possible to add images or the character '/' to
the menus.

<!-- ----------------------------------------------------------------- -->
<sect1>Manual Menu Creation
<p>
In the true tradition of teaching, we'll show you the hard way
first. <tt>:)</>

There are three widgets that go into making a menubar and submenus:
<itemize>
<item>a menu item, which is what the user wants to select, e.g. 'Save'
<item>a menu, which acts as a container for the menu items, and
<item>a menubar, which is a container for each of the individual
menus.
</itemize>

This is slightly complicated by the fact that menu item widgets are
used for two different things. They are both the widgets that are
packed into the menu, and the widget that is packed into the menubar,
which, when selected, activates the menu.

Let's look at the functions that are used to create menus and
menubars.  This first function is used to create a new menubar.

<tscreen>
<verb>
GtkWidget *gtk_menu_bar_new( void );
</verb>
</tscreen>

This rather self explanatory function creates a new menubar. You use
gtk_container_add to pack this into a window, or the box_pack
functions to pack it into a box - the same as buttons.

<tscreen><verb>
GtkWidget *gtk_menu_new( void );
</verb></tscreen>

This function returns a pointer to a new menu, it is never actually
shown (with gtk_widget_show), it is just a container for the menu
items. Hopefully this will become more clear when you look at the
example below.

The next two calls are used to create menu items that are packed into
the menu (and menubar).

<tscreen><verb>
GtkWidget *gtk_menu_item_new( void );
</verb></tscreen>

and

<tscreen><verb>
GtkWidget *gtk_menu_item_new_with_label( const char *label );
</verb></tscreen>

These calls are used to create the menu items that are to be
displayed.  Remember to differentiate between a "menu" as created with
gtk_menu_new and a "menu item" as created by the gtk_menu_item_new
functions. The menu item will be an actual button with an associated
action, whereas a menu will be a container holding menu items.

The gtk_menu_new_with_label and gtk_menu_new functions are just as
you'd expect after reading about the buttons. One creates a new menu
item with a label already packed into it, and the other just creates a
blank menu item.

Once you've created a menu item you have to put it into a menu. This
is done using the function gtk_menu_append. In order to capture when
the item is selected by the user, we need to connect to the
<tt/activate/ signal in the usual way. So, if we wanted to create a
standard <tt/File/ menu, with the options <tt/Open/, <tt/Save/ and
<tt/Quit/ the code would look something like:

<tscreen><verb>
    file_menu = gtk_menu_new ();    /* Don't need to show menus */

    /* Create the menu items */
    open_item = gtk_menu_item_new_with_label ("Open");
    save_item = gtk_menu_item_new_with_label ("Save");
    quit_item = gtk_menu_item_new_with_label ("Quit");

    /* Add them to the menu */
    gtk_menu_append (GTK_MENU (file_menu), open_item);
    gtk_menu_append (GTK_MENU (file_menu), save_item);
    gtk_menu_append (GTK_MENU (file_menu), quit_item);

    /* Attach the callback functions to the activate signal */
    gtk_signal_connect_object (GTK_OBJECT (open_items), "activate",
                               GTK_SIGNAL_FUNC (menuitem_response),
                               (gpointer) "file.open");
    gtk_signal_connect_object (GTK_OBJECT (save_items), "activate",
                               GTK_SIGNAL_FUNC (menuitem_response),
                               (gpointer) "file.save");

    /* We can attach the Quit menu item to our exit function */
    gtk_signal_connect_object (GTK_OBJECT (quit_items), "activate",
                               GTK_SIGNAL_FUNC (destroy),
                               (gpointer) "file.quit");

    /* We do need to show menu items */
    gtk_widget_show (open_item);
    gtk_widget_show (save_item);
    gtk_widget_show (quit_item);
</verb></tscreen>

At this point we have our menu. Now we need to create a menubar and a
menu item for the <tt/File/ entry, to which we add our menu. The code
looks like this:

<tscreen><verb>
    menu_bar = gtk_menu_bar_new ();
    gtk_container_add (GTK_CONTAINER (window), menu_bar);
    gtk_widget_show (menu_bar);

    file_item = gtk_menu_item_new_with_label ("File");
    gtk_widget_show (file_item);
</verb></tscreen>

Now we need to associate the menu with <tt/file_item/. This is done
with the function

<tscreen>
void gtk_menu_item_set_submenu( GtkMenuItem *menu_item,
                                GtkWidget   *submenu );
</tscreen>

So, our example would continue with

<tscreen><verb>
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (file_item), file_menu);
</verb></tscreen>

All that is left to do is to add the menu to the menubar, which is
accomplished using the function

<tscreen>
void gtk_menu_bar_append( GtkMenuBar *menu_bar,
                          GtkWidget  *menu_item );
</tscreen>

which in our case looks like this:

<tscreen><verb>
    gtk_menu_bar_append (GTK_MENU_BAR (menu_bar), file_item);
</verb></tscreen>

If we wanted the menu right justified on the menubar, such as help
menus often are, we can use the following function (again on
<tt/file_item/ in the current example) before attaching it to the
menubar.

<tscreen><verb>
void gtk_menu_item_right_justify( GtkMenuItem *menu_item );
</verb></tscreen>

Here is a summary of the steps needed to create a menu bar with menus
attached:

<itemize>
<item> Create a new menu using gtk_menu_new()
<item> Use multiple calls to gtk_menu_item_new() for each item you
wish to have on your menu. And use gtk_menu_append() to put each of
these new items on to the menu.
<item> Create a menu item using gtk_menu_item_new(). This will be the
root of the menu, the text appearing here will be on the menubar
itself.
<item>Use gtk_menu_item_set_submenu() to attach the menu to the root
menu item (the one created in the above step).
<item> Create a new menubar using gtk_menu_bar_new. This step only
needs to be done once when creating a series of menus on one menu bar.
<item> Use gtk_menu_bar_append() to put the root menu onto the menubar.
</itemize>

Creating a popup menu is nearly the same. The difference is that the
menu is not posted `automatically' by a menubar, but explicitly by
calling the function gtk_menu_popup() from a button-press event, for
example.  Take these steps:

<itemize>
<item>Create an event handling function. It needs to have the
prototype
<tscreen>
static gint handler (GtkWidget *widget,
                     GdkEvent  *event);
</tscreen>
and it will use the event to find out where to pop up the menu.
<item>In the event handler, if the event is a mouse button press,
treat <tt>event</tt> as a button event (which it is) and use it as
shown in the sample code to pass information to gtk_menu_popup().
<item>Bind that event handler to a widget with
<tscreen>
    gtk_signal_connect_object (GTK_OBJECT (widget), "event",
                               GTK_SIGNAL_FUNC (handler),
                               GTK_OBJECT (menu));
</tscreen>
where <tt>widget</tt> is the widget you are binding to,
<tt>handler</tt> is the handling function, and <tt>menu</tt> is a menu
created with gtk_menu_new(). This can be a menu which is also posted
by a menu bar, as shown in the sample code.
</itemize>

<!-- ----------------------------------------------------------------- -->
<sect1>Manual Menu Example
<p>
That should about do it. Let's take a look at an example to help clarify.

<tscreen><verb>
example_include(menu/menu.cc)
</verb></tscreen>

You may also set a menu item to be insensitive and, using an accelerator
table, bind keys to menu functions.

<!-- ----------------------------------------------------------------- -->
<sect1>Using GtkItemFactory
<p>
Now that we've shown you the hard way, here's how you do it using the
gtk_item_factory calls.

<!-- ----------------------------------------------------------------- -->
<sect1>Item Factory Example
<p>
Here is an example using the Gtk-- item factory.

<tscreen><verb>
example_include(itemfactory/itemfactory.cc)
</verb></tscreen>


For now, there's only this example. An explanation and lots 'o' comments
will follow later.

<!-- ***************************************************************** -->
