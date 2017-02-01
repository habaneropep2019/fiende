/* Include the gtk-- header files
 * Include stdio.h, we need that for the printf() function
 */
#include        <vector>
using std::vector;

#include        <gtk--/main.h>
#include        <gtk--/frame.h>
#include        <gtk--/button.h>
#include        <gtk--/window.h>
#include        <gtk--/separator.h>
#include        <gtk--/eventbox.h>
#include        <gtk--/box.h>
#include        <gtk--/list.h>
#include        <gtk--/listitem.h>
#include        <gtk--/scrolledwindow.h>
#include        <gtk--/tooltips.h>
#include        <gtk--/label.h>
#include        <stdio.h>

using SigC::slot;

class ListWindow : public Gtk::Window {
  Gtk::Frame *frame;
  Gtk::List *gtklist;
  Gtk::Tooltips tips;
public:
  ListWindow();
  ~ListWindow() {}
  void sigh_print_selection();
  int sigh_button_event(GdkEventButton *event);
};


/* prototypes for signal handler that we are going to connect
 * to the GtkList widget
 */

/* This is the signal handler that got connected to button
 * press/release events of the GtkList
 */
int ListWindow::sigh_button_event(GdkEventButton *event)
{
    /* We only do something if the third (rightmost mouse button
     * was released
     */
    if (event->type==GDK_BUTTON_RELEASE &&
	event->button==3) {
	Gtk::ListItem    *new_prisoner;
	
	/* Fetch the currently selected list item which
	 * will be our next prisoner ;)
	 */
	if (!gtklist->selection().empty())
          new_prisoner = *gtklist->selection().begin();
        else
          new_prisoner = 0;
	
	/* Look for already imprisoned list items, we
	 * will put them back into the list.
	 */
	Gtk::Widget *freeitem = frame->get_child();
	if (freeitem) freeitem->reparent(*gtklist);
	
	/* If we have a new prisoner, remove him from the
	 * GtkList and put him into the frame "Prison".
	 * We need to unselect the item first.
	 */
	if (new_prisoner) {
	    gtklist->unselect_child(*new_prisoner);
	    new_prisoner->reparent(*frame);
	}
    }
    return 0;
}

/* This is the signal handler that gets called if GtkList
 * emits the "selection_changed" signal
 */
void ListWindow::sigh_print_selection()
{
    Gtk::List::SelectionList &dlist=gtklist->selection();
    Gtk::List::SelectionList::iterator iter;
    
    // If there are no selected items there is nothing more
    // to do than just telling the user so
    if (dlist.empty()) {
	g_print("Selection cleared\n");
	return;
    }

    // Ok, we got a selection and so we print it
    g_print("The selection is a ");
    
    // Get the list item from the selection list and grab string from label 
    for (iter=dlist.begin();iter!=dlist.end();++iter) 
	g_print("%s ", dynamic_cast<Gtk::Label*>((*iter)->get_child())->get().c_str());
    g_print("\n");
}

ListWindow::ListWindow() : Gtk::Window(GTK_WINDOW_TOPLEVEL) {
    Gtk::HSeparator *separator;
    Gtk::EventBox *eventbox;
    Gtk::VBox *vbox;
    Gtk::ScrolledWindow *scrolled_window;
    Gtk::Button *button;
    Gtk::ListItem *list_item;
    GList *dlist;
    guint i;
    gchar buffer[64];
    
    /* Create a window to put all the widgets in
     * connect quit() to the "destroy" event of
     * the window to handle window manager close-window-events
     */
    set_title("GtkList Example");
    destroy.connect(Gtk::Main::quit.slot());
    
    /* Create the Gtk::List widget.
     * Connect the sigh_print_selection() signal handler
     * function to the "selection_changed" signal of the GtkList
     * to print out the selected items each time the selection
     * has changed */
    gtklist = manage(new Gtk::List());
    gtklist->selection_changed.connect(slot(this, &ListWindow::sigh_print_selection));

    /* Connect the sigh_button_event() signal handler to the Gtk::List
     * which will handle the "arresting" of list items
     */
    gtklist->button_release_event.connect(slot(this, &ListWindow::sigh_button_event));
    
    /* We create a "Prison" to put a list item in ;) */
    frame = manage(new Gtk::Frame("Prison"));
    frame->set_usize(200, 50);
    frame->set_border_width(5);
    frame->set_shadow_type(GTK_SHADOW_OUT);

    // frames can't capture tips so we use an eventbox
    eventbox = manage( new Gtk::EventBox());
    eventbox->add(*frame);
    //eventbox->set_events(GDK_ENTER_NOTIFY_MASK);
    
    
    /* Finally create a button and connect it's "clicked" signal
     * to the destruction of the window */
    button= manage(new Gtk::Button("Close"));
    button->clicked.connect(destroy.slot());

    /* Now we create 5 list items, each having it's own
     * label and add them to the GtkList using gtk_container_add()
     */
    for (i=0; i<5; i++) {
	Gtk::Label *label;
	
	sprintf(buffer, "ListItemContainer with Label #%d", i);
	label = manage(new Gtk::Label(buffer));

	list_item = manage(new Gtk::ListItem());
	list_item->add(*label);

	gtklist->add(*list_item);
    }

    /* Here, we are creating another 5 labels, this time
     * we use Gtk::ListItem(const string&) for the creation
     */
    vector<Gtk::ListItem*> items(5);
    for (i=0; i<5; i++) {
	sprintf(buffer, "List Item with Label %d", i+5);
	items[i]= manage(new Gtk::ListItem(buffer));
    }
    gtklist->append_items(items.begin(),items.end());

    // This is the scrolled window to put the Gtk::List widget inside 
    scrolled_window = manage(new Gtk::ScrolledWindow());
    scrolled_window->set_usize(250, 150);
    scrolled_window->add_with_viewport(*gtklist);

    // Collect things in a vertical box
    vbox = manage(new Gtk::VBox(false, 5));
    vbox->set_border_width(5);
    vbox->add(*scrolled_window);
    vbox->add(*eventbox);
    vbox->add(*manage(new Gtk::HSeparator()));
    vbox->add(*button);

    // Add some cues for the user
    tips.set_tip(*eventbox,"Click right button on list to capture an item.");
    tips.set_tip(*button,"Quits demo.");

    /* Finally we want to see the window, don't we? ;) */
    add(*vbox);
    show_all();
}    


/* Main function to set up the user interface */

int main (int    argc,
           gchar *argv[])
{                                  
    Gtk::Main kit(argc, argv, true);
    ListWindow l;
    kit.run();
    return 0;
}
