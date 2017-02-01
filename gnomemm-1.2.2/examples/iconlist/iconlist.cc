#include <gnome--.h>


class TestIconList
    : public Gnome::IconList
{
public:
    TestIconList ();

protected:
    virtual void select_icon_impl (gint icon, GdkEvent* event);
    virtual void unselect_icon_impl (gint icon, GdkEvent* event);
};


TestIconList::TestIconList ()
    : Gnome::IconList (80, 0, true)
{
    set_selection_mode (GTK_SELECTION_EXTENDED);
    set_separators ("icon1.png");

    append ("icon1.png", "Icon1");
    append ("icon2.png", "Icon2");
}


void 
TestIconList::select_icon_impl (gint icon, GdkEvent* event)
{
    Gnome::IconList::select_icon_impl (icon, event);
    cerr << "icon select: " << icon << endl;
}


void 
TestIconList::unselect_icon_impl (gint icon, GdkEvent* event)
{
    Gnome::IconList::unselect_icon_impl (icon, event);
    cerr << "icon unselect: " << icon << endl;
}


class IconWindow
    : public Gtk::Window
{
public:
    IconWindow ();

protected:
    virtual gint delete_event_impl (GdkEventAny*);

private:
    TestIconList m_iconlist;
};


IconWindow::IconWindow ()
{
    set_title ("Iconlist Example");
    set_default_size (300, 300);

    add (m_iconlist);

    show_all ();
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

