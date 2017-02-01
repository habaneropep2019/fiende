/* example-start progressbar progressbar.c */

#include <gtk--.h>

class ProgressWindow: public Gtk::Window
{
    /* Create a centering alignment object */
    Gtk::Alignment align;
    Gtk::Adjustment adj;
    Gtk::ProgressBar pbar;
    Gtk::VBox vbox;
    Gtk::HSeparator hseparator1, hseparator2;
    Gtk::VSeparator vseparator;
    Gtk::Table table;
    Gtk::Button button;
    Gtk::CheckButton text, activity;
    Gtk::RadioButton cont, disc;
public:
    ProgressWindow();
private:
    void toggle_text();
    void toggle_activity();
    void set_continuous();
    void set_discrete();
    gint progress_timeout();
    gint delete_event_impl(GdkEventAny*) { 
        Gtk::Main::instance()->quit(); return 0; 
    }
};

ProgressWindow::ProgressWindow():
    align(0.5, 0.5, 0, 0),
    adj(0, 1, 150),
    pbar(adj),
    vbox(false, 5),
    table(2, 3, false),
    button("close"),
    text("Show text"),
    activity("Activity mode"),
    cont(0, "Continuous"),
    disc(cont.group(), "Discrete")
{
    set_title("Gtk::ProgressBar");
    set_border_width(0);
    set_policy(false, false, true);

    /* Set the format of the string that can be displayed in the
     * trough of the progress bar:
     * %p - percentage
     * %v - value
     * %l - lower range value
     * %u - upper range value */    
    pbar.set_format_string ("%v from [%l-%u] (=%p%%)");
    align.add(pbar);

    table.attach(vseparator, 1, 2, 0, 2,
		 GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL,
		 5, 5);
    connect_to_method(text.clicked, this, &toggle_text);
    table.attach(text, 0, 1, 0, 1,
		 GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL,
		 5, 5);
    connect_to_method(activity.clicked, this, &toggle_activity);
    table.attach(activity, 0, 1, 1, 2,
		 GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL,
		 5, 5);
    connect_to_method(cont.clicked, this, &set_continuous);
    table.attach(cont, 2, 3, 0, 1,
		 GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL,
		 5, 5);
    connect_to_method(disc.clicked, this, &set_discrete);
    table.attach(disc, 2, 3, 1, 2,
		 GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL,
		 5, 5);

    connect_to_method(button.clicked, Gtk::Main::instance(),
		      &Gtk::Main::quit);
    
    vbox.set_border_width(10);
    vbox.pack_start(align, false, false, 5);
    vbox.pack_start(hseparator1, false, false, 0);
    vbox.pack_start(table, false, true, 0);
    vbox.pack_start(hseparator2, false, false, 0);
    vbox.pack_start(button, false, false, 0);
    
    vbox.show_all();
    add(vbox);

    connect_to_method(Gtk::Main::timeout(100), this, &progress_timeout);

    /* This makes it so the button is the default. */
    button.set_flags(GTK_CAN_DEFAULT);
    /* This grabs this button to be the default button. Simply hitting
     * the "Enter" key will cause this button to activate. */
    button.grab_default();
}

/* Callback that toggles the text display within the progress
 * bar trough */
void ProgressWindow::toggle_text()
{
    pbar.set_show_text(text.get_active());
}

/* Callback that toggles the activity mode of the progress
 * bar */
void ProgressWindow::toggle_activity()
{
    pbar.set_activity_mode(activity.get_active());
}

/* Callback that toggles the discrete mode of the progress
 * bar */
void ProgressWindow::set_discrete()
{
    pbar.set_bar_style(GTK_PROGRESS_DISCRETE);
}

/* Callback that toggles the continuous mode of the progress
 * bar */
void ProgressWindow::set_continuous()
{
    pbar.set_bar_style(GTK_PROGRESS_CONTINUOUS);
}

/* Update the value of the progress bar so that we get
 * some movement */
gint ProgressWindow::progress_timeout()
{
    gfloat new_val;

    /* Calculate the value of the progress bar using the
     * value range set in the adjustment object */

    new_val = pbar.get_value() + 1;

    if (new_val > adj.gtkobj()->upper)
      new_val = adj.gtkobj()->lower;

    /* Set the new value */
    pbar.set_value(new_val);

    /* As this is a timeout function, return true so that it
     * continues to get called */
    return(true);
} 

#if 0
/* Clean up allocated memory and remove the timer */
void destroy_progress( GtkWidget     *widget,
		       ProgressData *pdata)
{
    gtk_timeout_remove (pdata->timer);
    pdata->timer = 0;
    pdata->window = NULL;
    g_free(pdata);
    gtk_main_quit();
}

int main( int   argc,
          char *argv[])
{
    /* Add a timer callback to update the value of the progress bar */
    pdata->timer = gtk_timeout_add (100, progress_timeout, pdata->pbar);

}
#endif

int main(int argc, char *argv[])
{
    Gtk::Main m(&argc, &argv);
    ProgressWindow window;

    window.show();
    m.run();

    return 0;
}
/* example-end */
