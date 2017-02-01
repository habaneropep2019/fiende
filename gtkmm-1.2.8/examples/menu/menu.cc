/* example-start menu menu.c */

#include <stdio.h>
#include <gtk--.h>

using namespace Gtk;
using SigC::slot;

struct MyMenu : public Menu
  {
    gint button_press (GdkEvent *);
  };

static void menuitem_response (gchar *);

static gint quit( GdkEventAny *)
{
  Kit::quit();
  return 0;
}

int main (int argc, char *argv[])
{

    Window *window;
    MyMenu *menu;
    MenuBar *menu_bar;
    MenuItem *root_menu;
    MenuItem *menu_items;
    VBox *vbox;
    Button *button;
    char buf[128];
    int i;

    Kit kit(argc,argv);

    /* create a new window */
    window = manage ( new Window() );
    window->set_usize( 200, 100);
    window->set_title("GTK Menu Test");
    window->delete_event.connect( slot(&quit));

    /* Init the menu-widget, and remember -- never
     * show() the menu widget!! 
     * This is the menu that holds the menu items, the one that
     * will pop up when you click on the "Root Menu" in the app */
    menu = manage( new MyMenu() );

    /* Next we make a little loop that makes three menu-entries for "test-menu".
     * Notice the call to gtk_menu_append.  Here we are adding a list of
     * menu items to our menu.  Normally, we'd also catch the "clicked"
     * signal on each of the menu items and setup a callback for it,
     * but it's omitted here to save space. */

    for(i = 0; i < 3; i++)
        {
            /* Copy the names to the buf. */
            sprintf(buf, "Test-undermenu - %d", i);

            /* Create a new menu-item with a name... */
            menu_items = manage( new MenuItem(buf) );

            /* ...and add it to the menu. */
            menu->append(*menu_items);

	    /* Do something interesting when the menuitem is selected */
//	    gtk_signal_connect_object(GTK_OBJECT(menu_items), "activate",
//		(GtkSignalFunc)menuitem_response, (gpointer) g_strdup(buf));

            /* Show the widget */
        }

    /* This is the root menu, and will be the label
     * displayed on the menu bar.  There won't be a signal handler attached,
     * as it only pops up the rest of the menu when pressed. */
    root_menu = manage( new MenuItem("Root Menu") );


    /* Now we specify that we want our newly created "menu" to be the menu
     * for the "root menu" */
    root_menu->set_submenu(*menu);

    /* A vbox to put a menu and a button in: */
    vbox = manage( new VBox(false, 0) );
    window->add(*vbox);

    /* Create a menu-bar to hold the menus and add it to our main window */
    menu_bar = manage( new MenuBar() );
    vbox->pack_start(*menu_bar, false, false, 2);

    /* Create a button to which to attach menu as a popup */
    button = manage( new Button("press me") );
    button->event.connect( slot(menu,&MyMenu::button_press) );

//    gtk_signal_connect_object(GTK_OBJECT(button), "event",
//	GTK_SIGNAL_FUNC (button_press), GTK_OBJECT(menu));
    vbox->pack_end(*button, true, true, 2);

    /* And finally we append the menu-item to the menu-bar -- this is the
     * "root" menu-item I have been raving about =) */
    menu_bar->append(*root_menu);

    /* always display the window as the last step so it all splashes on
     * the screen at once. */
    window->show_all();

    kit.run ();

    return 0;
}

/* Respond to a button-press by posting a menu passed in as widget.
 *
 * Note that the "widget" argument is the menu being posted, NOT
 * the button that was pressed.
 */

gint MyMenu::button_press (GdkEvent *event)
{

    if (event->type == GDK_BUTTON_PRESS) {
        GdkEventButton *bevent = (GdkEventButton *) event; 
        popup(bevent->button, bevent->time);

        /* Tell calling code that we have handled this event; the buck
         * stops here. */
        return true;
    }

    /* Tell calling code that we have not handled this event; pass it on. */
    return false;
}


/* Print a string when a menu item is selected */

static void menuitem_response (gchar *s)
{
    printf("%s\n", s);
}
/* example-end */
