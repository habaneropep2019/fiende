/* 
 *  This is a demo of gtkpacker. 
 */

#include <gtk--/main.h>
#include <gtk--/togglebutton.h>
#include <gtk--/frame.h>
#include <gtk--/table.h>
#include <gtk--/window.h>
#include <gtk--/packer.h>
#include <stdio.h>

class SideArea;
class AnchorArea;
class OptionArea;

class AppWindow : public Gtk::Window
{
  Gtk::Packer *packer;
  Gtk::Packer_Helpers::Child *current;

  SideArea* side_area;
  AnchorArea* anchor_area;
  OptionArea* option_area;

  bool changing;
  
  virtual void destroy_impl()
    { Gtk::Main::quit(); }

public:  
  AppWindow();
  ~AppWindow();
  void add_widget();
  void remove_widget();
  void set_widget(Gtk::Widget* );
  Gtk::Packer_Helpers::Child* get_current() {return current; }
};

class SideArea : public Gtk::Frame
{
  AppWindow *app;
  Gtk::ToggleButton* button_top;
  Gtk::ToggleButton* button_bottom;
  Gtk::ToggleButton* button_left;
  Gtk::ToggleButton* button_right;
  bool changing;
public:
  SideArea(AppWindow* a);
  void set_side (GtkSideType side);
  void toggle_side (GtkSideType side);
};

class AnchorArea : public Gtk::Frame
{
  AppWindow *app;
  Gtk::ToggleButton* button_n;
  Gtk::ToggleButton* button_s;
  Gtk::ToggleButton* button_w;
  Gtk::ToggleButton* button_e;
  Gtk::ToggleButton* button_ne;
  Gtk::ToggleButton* button_nw;
  Gtk::ToggleButton* button_se;
  Gtk::ToggleButton* button_sw;
  Gtk::ToggleButton* button_center;
  bool changing;

public:
  AnchorArea(AppWindow* a);
  void set_anchor (GtkAnchorType anchor);
  void toggle_anchor (GtkAnchorType anchor);
};

class OptionArea : public Gtk::Frame
{
  AppWindow *app;
  Gtk::ToggleButton*  button_fillx;
  Gtk::ToggleButton*  button_filly;
  Gtk::ToggleButton*  button_expand;
  bool changing;
public:
  OptionArea(AppWindow* a);
  void set_options (guint o);
  void toggle_options ();
};

AppWindow::~AppWindow() {}

AppWindow::AppWindow()
  : Gtk::Window(GTK_WINDOW_TOPLEVEL)
{
    Gtk::Packer *window_pack;
    Gtk::Packer *top_pack;
    Gtk::Packer *bottom_pack;
    Gtk::Packer *button_pack;
    Gtk::Frame  *frame;
    Gtk::Button *button;

    changing=false;

    window_pack = manage(new Gtk::Packer());
    add (*window_pack);
    set_border_width (4);

    top_pack = manage( new Gtk::Packer() );
    window_pack->add ( *top_pack,
		       GTK_SIDE_TOP,
		       GTK_ANCHOR_CENTER,
		       GTK_FILL_X | GTK_FILL_Y | GTK_EXPAND);

  

    // Create the demo area
    frame = manage( new Gtk::Frame("Packing Area") );
    frame->set_usize(400, 400);
    top_pack->add( *frame,
                   GTK_SIDE_LEFT,
                   GTK_ANCHOR_CENTER,
                   GTK_FILL_X | GTK_FILL_Y | GTK_EXPAND,     
                   0, 8, 8, 0, 0);

    packer = manage( new Gtk::Packer() );
    frame->add(*packer);


    // Create the control buttons
    button_pack =  manage( new Gtk::Packer() );
    top_pack->add( *button_pack, 
                   GTK_SIDE_LEFT,
                   GTK_ANCHOR_N,
                   0, 
                   0, 0, 0, 0, 0);

    button = manage( new Gtk::Button("Add Button") );

    top_pack->add( *button, 
                   GTK_SIDE_TOP,
                   GTK_ANCHOR_CENTER,
                   GTK_FILL_X, 
                   0, 8, 8, 8, 0);

    button->clicked.connect(slot(this,&AppWindow::add_widget));

    button = manage( new Gtk::Button("Remove Button") );

    top_pack->add( *button, 
                   GTK_SIDE_TOP,
                   GTK_ANCHOR_CENTER,
                   GTK_FILL_X, 
                   0, 8, 8, 8, 0);

    button->clicked.connect(slot(this,&AppWindow::remove_widget));

    button = manage( new Gtk::Button("Quit") );
    top_pack->add( *button, 
                   GTK_SIDE_TOP,
                   GTK_ANCHOR_CENTER,
                   GTK_FILL_X, 
                   0, 8, 8, 0, 0);

    button->clicked.connect( Gtk::Main::quit.slot() );

    
    // Create the options
    bottom_pack = manage( new Gtk::Packer() );
    window_pack->add( *bottom_pack, GTK_SIDE_TOP, GTK_ANCHOR_CENTER, GTK_FILL_X);
  
    side_area = manage( new SideArea(this) );
    bottom_pack->add( *side_area, GTK_SIDE_LEFT, GTK_ANCHOR_W, GTK_FILL_Y,
                   0, 10, 10, 0, 0);

    anchor_area = manage( new AnchorArea(this) );
    bottom_pack->add( *anchor_area, GTK_SIDE_LEFT, GTK_ANCHOR_W, GTK_FILL_Y,
                   0, 10, 10, 0, 0);

    option_area = manage( new OptionArea(this) );
    bottom_pack->add( *option_area, GTK_SIDE_LEFT, GTK_ANCHOR_W, GTK_FILL_Y,
                   0, 10, 10, 0, 0);

    current=0;
    add_widget();

    show_all ();

};


SideArea::SideArea(AppWindow* a)
  : Gtk::Frame("Side"), app(a)
{
    Gtk::Packer* pack = manage( new Gtk::Packer() );
    add(*pack);
    
    button_top    = manage( new Gtk::ToggleButton("Top") );
    button_bottom = manage( new Gtk::ToggleButton("Bottom") );
    button_left   = manage( new Gtk::ToggleButton("Left") );
    button_right  = manage( new Gtk::ToggleButton("Right") );

    button_top->set_usize( 50, -1);
    button_bottom->set_usize( 50, -1);
    button_left->set_usize( 50, -1);
    button_right->set_usize( 50, -1);

    pack->add(*button_top, GTK_SIDE_TOP, GTK_ANCHOR_CENTER,
                   0, 0, 5, 5, 0, 0);
    pack->add(*button_bottom, GTK_SIDE_BOTTOM, GTK_ANCHOR_CENTER,
                   0, 0, 5, 5, 0, 0);
    pack->add(*button_left, GTK_SIDE_LEFT, GTK_ANCHOR_CENTER,
                  0, 0, 10, 5, 0, 0);
    pack->add(*button_right, GTK_SIDE_RIGHT, GTK_ANCHOR_CENTER,
                   0, 0, 10, 5, 0, 0);

    button_top->clicked.connect(bind(slot(this,&SideArea::toggle_side),GTK_SIDE_TOP));
    button_bottom->clicked.connect(bind(slot(this,&SideArea::toggle_side),GTK_SIDE_BOTTOM));
    button_left->clicked.connect(bind(slot(this,&SideArea::toggle_side),GTK_SIDE_LEFT));
    button_right->clicked.connect(bind(slot(this,&SideArea::toggle_side),GTK_SIDE_RIGHT));

    changing=false;
};


AnchorArea::AnchorArea(AppWindow* a)
  : Gtk::Frame("Anchor"), app(a)
{
    Gtk::Packer* pack = manage( new Gtk::Packer() );
    add(*pack);

    Gtk::Table* table = manage( new Gtk::Table(3,3,true) );
    pack->add( *table, GTK_SIDE_TOP, GTK_ANCHOR_CENTER,
               GTK_FILL_Y | GTK_FILL_X | GTK_PACK_EXPAND,
               0, 10, 5, 0, 0);

    button_n  = manage( new Gtk::ToggleButton("N") );
    button_s  = manage( new Gtk::ToggleButton("S") );
    button_w  = manage( new Gtk::ToggleButton("W") );
    button_e  = manage( new Gtk::ToggleButton("E") );
    button_ne = manage( new Gtk::ToggleButton("NE") );
    button_nw = manage( new Gtk::ToggleButton("NW") );
    button_se = manage( new Gtk::ToggleButton("SE") );
    button_sw = manage( new Gtk::ToggleButton("SW") );
    button_center = manage( new Gtk::ToggleButton("") );

    button_center->set_state(GTK_STATE_NORMAL);
//    button_center->set_sensitive(0);

    button_n->clicked.connect(bind(slot(this,&AnchorArea::toggle_anchor),GTK_ANCHOR_N));
    button_e->clicked.connect(bind(slot(this,&AnchorArea::toggle_anchor),GTK_ANCHOR_E));
    button_w->clicked.connect(bind(slot(this,&AnchorArea::toggle_anchor),GTK_ANCHOR_W));
    button_s->clicked.connect(bind(slot(this,&AnchorArea::toggle_anchor),GTK_ANCHOR_S));
    button_ne->clicked.connect(bind(slot(this,&AnchorArea::toggle_anchor),GTK_ANCHOR_NE));
    button_se->clicked.connect(bind(slot(this,&AnchorArea::toggle_anchor),GTK_ANCHOR_SE));
    button_nw->clicked.connect(bind(slot(this,&AnchorArea::toggle_anchor),GTK_ANCHOR_NW));
    button_sw->clicked.connect(bind(slot(this,&AnchorArea::toggle_anchor),GTK_ANCHOR_SW));
    button_center->clicked.connect(bind(slot(this,&AnchorArea::toggle_anchor),GTK_ANCHOR_CENTER));

    table->attach( *button_nw, 0, 1, 0, 1);
    table->attach( *button_n, 1, 2, 0, 1);
    table->attach( *button_ne, 2, 3, 0, 1);
    table->attach( *button_w, 0, 1, 1, 2);
    table->attach( *button_center, 1, 2, 1, 2);
    table->attach( *button_e, 2, 3, 1, 2);
    table->attach( *button_sw, 0, 1, 2, 3);
    table->attach( *button_s, 1, 2, 2, 3);
    table->attach( *button_se, 2, 3, 2, 3);

    changing=false;
}


OptionArea::OptionArea(AppWindow* a)
  : Gtk::Frame("Options"), app(a)
{
    Gtk::Packer* pack = manage( new Gtk::Packer() );
    add(*pack);

    button_fillx = manage( new Gtk::ToggleButton("Fill X") );
    button_filly = manage( new Gtk::ToggleButton("Fill Y") );
    button_expand = manage( new Gtk::ToggleButton("Expand") );

    pack->add( *button_fillx, GTK_SIDE_TOP, GTK_ANCHOR_N,
                   GTK_FILL_X | GTK_PACK_EXPAND,
                   0, 10, 5, 0, 0);
    pack->add( *button_filly, GTK_SIDE_TOP, GTK_ANCHOR_CENTER,
                   GTK_FILL_X | GTK_PACK_EXPAND,
                   0, 10, 5, 0, 0);
    pack->add( *button_expand, GTK_SIDE_TOP, GTK_ANCHOR_S,
                   GTK_FILL_X | GTK_PACK_EXPAND,
                   0, 10, 5, 0, 0);

    button_fillx->toggled.connect (slot(this,&OptionArea::toggle_options));
    button_filly->toggled.connect (slot(this,&OptionArea::toggle_options));
    button_expand->toggled.connect (slot(this,&OptionArea::toggle_options));

    changing=false;
}


void OptionArea::set_options (guint option)
{
   button_fillx->set_active(option&GTK_FILL_X);
   button_filly->set_active(option&GTK_FILL_Y);
   button_expand->set_active(option&GTK_PACK_EXPAND);
}

void OptionArea::toggle_options ()
{
   if (changing) return;
   changing=true;
   gint option = 0;

   if (button_fillx->get_active())   option |= GTK_FILL_X;
   if (button_filly->get_active())   option |= GTK_FILL_Y;
   if (button_expand->get_active())  option |= GTK_PACK_EXPAND;

   if (app->get_current())
     app->get_current()->set_options(option);
   changing=false;
}

void AnchorArea::set_anchor (GtkAnchorType anchor)
{
   button_n->set_active(anchor==GTK_ANCHOR_N);
   button_e->set_active(anchor==GTK_ANCHOR_E);
   button_w->set_active(anchor==GTK_ANCHOR_W);
   button_s->set_active(anchor==GTK_ANCHOR_S);
   button_ne->set_active(anchor==GTK_ANCHOR_NE);
   button_se->set_active(anchor==GTK_ANCHOR_SE);
   button_nw->set_active(anchor==GTK_ANCHOR_NW);
   button_sw->set_active(anchor==GTK_ANCHOR_SW);
   button_center->set_active(anchor==GTK_ANCHOR_CENTER);

   button_n->set_sensitive(anchor!=GTK_ANCHOR_N);
   button_e->set_sensitive(anchor!=GTK_ANCHOR_E);
   button_w->set_sensitive(anchor!=GTK_ANCHOR_W);
   button_s->set_sensitive(anchor!=GTK_ANCHOR_S);
   button_ne->set_sensitive(anchor!=GTK_ANCHOR_NE);
   button_se->set_sensitive(anchor!=GTK_ANCHOR_SE);
   button_nw->set_sensitive(anchor!=GTK_ANCHOR_NW);
   button_sw->set_sensitive(anchor!=GTK_ANCHOR_SW);
   button_center->set_sensitive(anchor!=GTK_ANCHOR_CENTER);
}

void AnchorArea::toggle_anchor (GtkAnchorType anchor)
{
   if (changing) return;
   changing=true;
   if (app->get_current())
     {
       app->get_current()->set_anchor(anchor);
       set_anchor(anchor);
     }
   changing=false;
}

void SideArea::set_side (GtkSideType side)
{
   button_top->set_active(side==GTK_SIDE_TOP);
   button_bottom->set_active(side==GTK_SIDE_BOTTOM);
   button_right->set_active(side==GTK_SIDE_RIGHT);
   button_left->set_active(side==GTK_SIDE_LEFT);

   button_top->set_sensitive(side!=GTK_SIDE_TOP);
   button_bottom->set_sensitive(side!=GTK_SIDE_BOTTOM);
   button_right->set_sensitive(side!=GTK_SIDE_RIGHT);
   button_left->set_sensitive(side!=GTK_SIDE_LEFT);
}

void SideArea::toggle_side (GtkSideType side)
{
   if (changing) return;
   changing=true;
   if (app->get_current())
     {
       app->get_current()->set_side(side);
       set_side(side);
     }
   changing=false;
}

void AppWindow::set_widget (Gtk::Widget *w) 
{
   if (changing) return;
   changing=true;
   Gtk::Packer::PackerList &pl=packer->children();

   Gtk::Packer::iterator  i=pl.end(),j;
   if (w) i=pl.find(*w);
   for (j=pl.begin();j!=pl.end();++j)
     {
       if (j==i) continue;
       ((Gtk::ToggleButton*)(*j)->get_widget())->set_active(0);
       ((Gtk::ToggleButton*)(*j)->get_widget())->set_sensitive(1); 
     }

   if (i==packer->children().end())
     { 
       current=0;
       changing=false;
       return;
     }

   w->set_sensitive(0); 
  
   current=*i;

   side_area->set_side(current->get_side());
   anchor_area->set_anchor(current->get_anchor());
   option_area->set_options(current->get_options());

   changing=false;
}

void AppWindow::add_widget ()
{  
   static gint n = 0;
   gchar str[255];
   Gtk::ToggleButton *widget;

   sprintf(str, "%d", n);
   widget = manage( new Gtk::ToggleButton(str) );
   widget->set_usize(50, 50);
   packer->add(*widget,GTK_SIDE_TOP,GTK_ANCHOR_CENTER,0);
   widget->show();


   widget->set_active(GTK_STATE_ACTIVE);
   widget->set_sensitive(0);
   set_widget(widget);

   widget->toggled.connect(bind(slot(this,&AppWindow::set_widget),(Gtk::Widget*) widget));

   n++;
}

void AppWindow::remove_widget ()
{  
   packer->children().remove(current);

   current=0;
   set_widget(0);
}

int main (int argv, char **argc)
{
    Gtk::Main kit(argv, argc);
    AppWindow app;

    kit.run();

    return 0;
}


