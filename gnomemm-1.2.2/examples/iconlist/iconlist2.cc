#include <gnome--.h>

static GtkTargetEntry target_table[] = {
  {"someicon", 1, 2},
};


class TestIconList
    : public Gnome::IconList
{
public:
    TestIconList ();

protected:
    virtual void select_icon_impl (gint icon, GdkEvent* event);
    virtual void unselect_icon_impl (gint icon, GdkEvent* event);
    virtual void drag_data_get_impl (GdkDragContext* context,
                                     GtkSelectionData* data,
                                     guint info,
                                     guint time);
    virtual void drag_data_delete_impl (GdkDragContext* context);

private:
    int m_selected;
};


TestIconList::TestIconList ()
    : m_selected (-1)
{
    int position = append ("icon1.png", "Icon1");
    set_icon_data (position, (gpointer) "Icon1");
    position = append ("icon2.png", "Icon2");
    set_icon_data (position, (gpointer) "Icon2");
}


void 
TestIconList::select_icon_impl (gint icon, GdkEvent* e)
{
    Gnome::IconList::select_icon_impl (icon, e);
    if (m_selected != icon) {
        drag_source_set (GDK_BUTTON1_MASK,
                         target_table, 1,
                         static_cast<GdkDragAction> (GDK_ACTION_COPY 
                                                     | GDK_ACTION_MOVE));
        m_selected = icon;
        
        // send event again to start drag
        // (how can this be done in a better way?)
        event (e);
    }
}


void 
TestIconList::unselect_icon_impl (gint icon, GdkEvent* event)
{
    Gnome::IconList::unselect_icon_impl (icon, event);
    drag_source_unset ();
    m_selected = -1;
}


void 
TestIconList::drag_data_get_impl (GdkDragContext* context,
                                  GtkSelectionData* data,
                                  guint info,
                                  guint time)
{
    Gnome::IconList::drag_data_get_impl (context, data, info, time);
    char* name = (char*) get_icon_data (m_selected);
    gtk_selection_data_set (data,
                            data->target,
                            8, 
                            (const guchar*) name,
                            strlen (name));
}


void 
TestIconList::drag_data_delete_impl (GdkDragContext* context)
{
    // when will this method be called?
    Gnome::IconList::drag_data_delete_impl (context);
    cerr << "TestIconList::drag_data_delete_impl called" << endl;
}


class IconWindow
    : public Gtk::Window
{
public:
    IconWindow ();

protected:
    virtual gint delete_event_impl (GdkEventAny*);

    void get_drag_data_cb (GdkDragContext* context,
                           gint x,
                           gint y,
                           GtkSelectionData* data,
                           guint info,
                           guint time);
private:
    Gtk::VBox m_vbox;
    TestIconList m_iconlist;
    Gtk::Label m_label;
};


IconWindow::IconWindow ()
    : m_label ("Drop icon here!")
{
    set_title ("Iconlist Example");
    set_default_size (300, 300);

    m_vbox.pack_start (m_iconlist);
    m_vbox.pack_start (m_label, false, false);
    add (m_vbox);

    m_label.drag_dest_set (GTK_DEST_DEFAULT_ALL,
                           target_table, 1,
                           static_cast<GdkDragAction> (GDK_ACTION_COPY
                                                       | GDK_ACTION_MOVE));
    m_label.drag_data_received
        .connect (slot (this, &IconWindow::get_drag_data_cb));    
    
    show_all ();
}


void
IconWindow::get_drag_data_cb (GdkDragContext* context,
                              gint x,
                              gint y,
                              GtkSelectionData* data,
                              guint info,
                              guint time)
{
    cerr << "You gave me icon '" << data->data << "'" << endl;
}


gint 
IconWindow::delete_event_impl (GdkEventAny*)
{ 
    Gtk::Main::quit ();
    return 0; 
}


int 
main (int argc, char *argv[])
{
   Gnome::Main kit ("Iconlist", "0.0.0", argc, argv);

   IconWindow window;
   
   kit.run ();

   return 0;
}

