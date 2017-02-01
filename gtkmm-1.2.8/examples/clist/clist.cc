/* example-start clist clist.c */

#include <vector>
#include <string>
#include <gtk--/window.h>
#include <gtk--/scrolledwindow.h>
#include <gtk--/clist.h>
#include <gtk--/box.h>
#include <gtk--/button.h>
#include <gtk--/main.h>

using std::vector;

// User clicked the "Add List" button.
void button_add_clicked(Gtk::CList *clist)
{
    int indx;
 
    // Something silly to add to the list. 4 rows of 2 columns each
    static const char *drink[4][2] = { 
             { "Milk",    "3 Oz" },
             { "Water",   "6 l" },
             { "Carrots", "2" },
             { "Snakes",  "55" } };

    // Here we do the actual adding of the text. It's done once for
    // each row.
    for ( indx=0 ; indx < 4 ; indx++ )
	clist->rows().push_back( drink[indx] );

    vector<Gtk::string> v_less;
    v_less.push_back("hello");

    vector<Gtk::string> v_more;
    v_more.push_back("hello");
    v_more.push_back("brave");
    v_more.push_back("new");
    v_more.push_back("world");

    clist->rows().push_back(v_less);
    clist->rows().push_back(v_more);

    return;
}

// User clicked the "Clear List" button.
void button_clear_clicked( Gtk::CList *clist )
{
    // Clear the list using gtk_clist_clear. This is much faster than
    // calling gtk_clist_remove once for each row.
    clist->clear();

    return;
}

// The user clicked the "Hide/Show titles" button.
void button_hide_show_clicked( Gtk::CList *clist )
{
    // Just a flag to remember the status. 0 = currently visible
    static short int flag = 0;

    if (flag == 0)
    {
        // Hide the titles and set the flag to 1
	clist->column_titles_hide();
	flag++;
    }
    else
    {
        // Show the titles and reset flag to 0 
	clist->column_titles_show();
	flag--;
    }

    return;
}

// If we come here, then the user has selected a row in the list.
void selection_made(gint            row,
                    gint            column,
                    GdkEvent       *event,
                    Gtk::CList      *clist)
{
    // Get the text that is stored in the selected row and column
    // which was clicked in. We will receive it as a pointer in the
    // argument text.
    Gtk::string text = clist->cell(row,column).get_text();

    // Just prints some information about the selected row
    g_print("You selected row %d. More specifically you clicked in "
            "column %d, and the text in this cell is %s\n\n",
            row, column, text.c_str());

    return;
}

int main( int    argc,
          gchar *argv[] )
{                                  
    Gtk::Main main(argc, argv);


    // Create the CList. For this example we use 2 columns
    static const gchar *titles[] = { "Ingredients", "Amount", NULL };
    Gtk::CList *clist = manage(new Gtk::CList(titles));

    // When a selection is made, we want to know about it. The callback
    // used is selection_made, and its code can be found further down
    clist->select_row.connect(bind(slot(selection_made), clist));

    // It isn't necessary to shadow the border, but it looks nice :)
    clist->set_shadow_type(GTK_SHADOW_OUT);

    // What however is important, is that we set the column widths as
    // they will never be right otherwise. Note that the columns are
    // numbered from 0 and up (to 1 in this case).
    clist->set_column_width (0, 150);


    // Create a scrolled window to pack the CList widget into */
    Gtk::ScrolledWindow *scrolled_window = manage(new Gtk::ScrolledWindow());
    scrolled_window->set_policy(GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
    scrolled_window->add(*clist);


    Gtk::Button *button_add        = manage(new Gtk::Button("Add List"));
    Gtk::Button *button_clear      = manage(new Gtk::Button("Clear List"));
    Gtk::Button *button_hide_show  = manage(new Gtk::Button("Hide/Show titles"));

    button_add       ->clicked.connect(bind(slot(button_add_clicked),       clist));
    button_clear     ->clicked.connect(bind(slot(button_clear_clicked),     clist));
    button_hide_show ->clicked.connect(bind(slot(button_hide_show_clicked), clist));


    Gtk::HBox *hbox = manage(new Gtk::HBox(false, 0));
    hbox->pack_start(*button_add);
    hbox->pack_start(*button_clear);
    hbox->pack_start(*button_hide_show);


    Gtk::VBox *vbox = manage(new Gtk::VBox(false, 5));
    vbox->set_border_width(5);
    vbox->pack_start(*scrolled_window);
    vbox->pack_start(*hbox, false);


    Gtk::Window *window = manage(new Gtk::Window(GTK_WINDOW_TOPLEVEL));
    window->destroy.connect(Gtk::Main::quit.slot());
    window->set_usize(300, 150);
    window->set_title("GtkCList Example");
    window->add(*vbox);
    

    // The interface is completely set up so we show the window and
    // enter the gtk_main loop.
    window->show_all();
    main.run();
    
    return(0);
}
