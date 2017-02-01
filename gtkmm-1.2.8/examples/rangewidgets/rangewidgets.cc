#include <gtk--/main.h>
#include <gtk--/adjustment.h>
#include <gtk--/scale.h>
#include <gtk--/box.h>
#include <gtk--/label.h>
#include <gtk--/window.h>
#include <gtk--/separator.h>
#include <gtk--/scrollbar.h>
#include <gtk--/checkbutton.h>
#include <string>
#include <gtk--/menu.h>
#include <gtk--/optionmenu.h>

// Gtk-- version of the range widgets example from the gtk+ tutorial

class LabeledOptionMenu : public Gtk::HBox
{
  Gtk::Label m_label;

public:
    LabeledOptionMenu(const Gtk::string &menutitle, Gtk::Menu *menu,
		      bool homogeneous=false,
		      gint spacing=10);
    Gtk::Menu *m_menu;
};

LabeledOptionMenu::LabeledOptionMenu(const Gtk::string &menutitle,
				     Gtk::Menu *menu,
				     bool homogeneous,
				     gint spacing) :
    Gtk::HBox(homogeneous, spacing),
    m_label(menutitle),
    m_menu(menu)
{
  pack_start(m_label, false, false, 0);
  Gtk::OptionMenu *om=manage(new Gtk::OptionMenu);
  om->set_menu(m_menu);
  pack_start(*om);
}

class RangeControls : public Gtk::Window
{

  Gtk::VBox m_vbox1, m_vbox2, m_vbox3;
  Gtk::HBox m_hbox1, m_hbox2;

  Gtk::Adjustment m_adj1;

  Gtk::VScale m_vscale;
  Gtk::HScale m_hscale;

  Gtk::HSeparator m_separator;
  
  Gtk::Button m_buttonQuit;
  Gtk::CheckButton m_checkbutton;
  
  Gtk::HScrollbar m_scrollbar;

public:

    RangeControls();

    // callbacks
    void draw_value(Gtk::CheckButton *button);
    void menu_pos_select_cb(GtkPositionType type);
    void menu_update_select_cb(GtkUpdateType type);
    void digits_cb(Gtk::Adjustment *adj);
    void psize_cb(Gtk::Adjustment *adj);
    gint delete_event_impl(GdkEventAny*) { 
	Gtk::Main::quit(); return 0; 
    }
};

RangeControls::RangeControls() :
  m_vbox1(false, 0),
  m_vbox2(false, 20),
  m_vbox3(false, 10),

  m_hbox1(false, 10),
  m_hbox2(false, 10),

  // value, lower, upper, step_increment, page_increment, page_size
  // note that the page_size value only makes a difference for
  // scrollbar widgets, and the highest value you'll get is actually
  // (upper - page_size).
  m_adj1(0.0, 0.0, 101.0, 0.1, 1.0, 1.0),

  m_vscale(m_adj1),

  m_hscale(m_adj1),

  m_buttonQuit("Quit"),
  // a checkbutton to control whether the value is displayed or not
  m_checkbutton("Display value on scale widgets",0),

  // reuse the same adjustment again
  m_scrollbar(m_adj1)
  // notice how this causes the scales to always be update
  // continuously when the scrollbar is moved
{
  set_title("range controls");

  m_vscale.set_update_policy(GTK_UPDATE_CONTINUOUS);
  m_vscale.set_digits(1);
  m_vscale.set_value_pos(GTK_POS_TOP);
  m_vscale.set_draw_value(true);
  m_hscale.set_update_policy(GTK_UPDATE_CONTINUOUS);
  m_hscale.set_digits(1);
  m_hscale.set_value_pos(GTK_POS_TOP);
  m_hscale.set_draw_value(true);

  add(m_vbox1);
  m_vbox1.pack_start(m_vbox2);
  m_vbox2.set_border_width(10);
  m_vbox2.pack_start(m_hbox1);
  m_hbox1.pack_start(m_vscale);  
  m_hbox1.pack_start(m_vbox3);
  
  m_hscale.set_usize(200, 30);
  m_vbox3.pack_start(m_hscale);
  
  m_scrollbar.set_update_policy(GTK_UPDATE_CONTINUOUS);
  m_vbox3.pack_start(m_scrollbar);
  
  m_checkbutton.set_active(true);
  m_checkbutton.toggled.connect(
      bind(slot(this, &RangeControls::draw_value), &m_checkbutton));
  m_vbox2.pack_start(m_checkbutton);

  {
      using namespace Gtk::Menu_Helpers;

      Gtk::Menu *menu_vpos=manage(new Gtk::Menu);
      MenuList& list_vpos=menu_vpos->items();
      list_vpos.push_back(
	  MenuElem("Top",bind(
	      slot(this,&RangeControls::menu_pos_select_cb),GTK_POS_TOP)));
      list_vpos.push_back(
	  MenuElem("Bottom",bind(
	      slot(this,&RangeControls::menu_pos_select_cb),GTK_POS_BOTTOM)));
      list_vpos.push_back(
	  MenuElem("Left",bind(
	      slot(this,&RangeControls::menu_pos_select_cb),GTK_POS_LEFT)));
      list_vpos.push_back(
	  MenuElem("Right",bind(
	      slot(this,&RangeControls::menu_pos_select_cb),GTK_POS_RIGHT)));

      Gtk::Menu *menu_upd=manage(new Gtk::Menu);
      MenuList& list_upd=menu_upd->items();
      list_upd.push_back(
	  MenuElem("Continuous",bind(
	      slot(this,&RangeControls::menu_update_select_cb),GTK_UPDATE_CONTINUOUS)));
      list_upd.push_back(
	  MenuElem("Discontinuous",bind(
	      slot(this,&RangeControls::menu_update_select_cb),GTK_UPDATE_DISCONTINUOUS)));
      list_upd.push_back(
	  MenuElem("Delayed",bind(
	      slot(this,&RangeControls::menu_update_select_cb),GTK_UPDATE_DELAYED)));

      m_vbox2.pack_start(
	  *manage(new LabeledOptionMenu("Scale Value Position:",menu_vpos)));
      m_vbox2.pack_start(
	  *manage(new LabeledOptionMenu("Scale Update Policy:",menu_upd)));
  }

  Gtk::HBox *lsbox1=manage(new Gtk::HBox(false,10));
  lsbox1->pack_start(*manage(new Gtk::Label("Scale Digits:",0)),false,false);
  Gtk::Adjustment *adj1=manage(new Gtk::Adjustment(1.0, 0.0, 5.0));
  Gtk::HScale *digits=manage(new Gtk::HScale(*adj1));
  digits->set_digits(0);
  adj1->value_changed.connect(bind(slot(this,&RangeControls::digits_cb),adj1));
  lsbox1->pack_start(*digits,true,true);

  Gtk::HBox *lsbox2=manage(new Gtk::HBox(false,10));
  lsbox2->pack_start(*manage(new Gtk::Label("Scrollbar Page Size:",0)),false,false);
  Gtk::Adjustment *adj2=manage(new Gtk::Adjustment(1.0, 1.0, 101.0));
  Gtk::HScale *pgsize=manage(new Gtk::HScale(*adj2));
  pgsize->set_digits(0);
  adj2->value_changed.connect(bind(slot(this,&RangeControls::psize_cb),adj2));
  lsbox2->pack_start(*pgsize,true,true);
  
  m_vbox2.pack_start(*lsbox1);
  m_vbox2.pack_start(*lsbox2);
  m_vbox1.pack_start(m_separator, false, true, 0);
  m_vbox1.pack_start(m_buttonQuit,false,false);
  m_buttonQuit.set_flags(GTK_CAN_DEFAULT);
  m_buttonQuit.grab_default();
  m_buttonQuit.clicked.connect(Gtk::Main::quit.slot());
  m_buttonQuit.set_border_width(10);
  show_all();
}

void RangeControls::draw_value(Gtk::CheckButton *button)
{
    m_vscale.set_draw_value(button->get_active());
    m_hscale.set_draw_value(button->get_active());
}

void RangeControls::menu_pos_select_cb(GtkPositionType postype)
{
    m_vscale.set_value_pos(postype);
    m_hscale.set_value_pos(postype);
}

void RangeControls::menu_update_select_cb(GtkUpdateType type)
{
    m_vscale.set_update_policy(type);
    m_hscale.set_update_policy(type);
}

void RangeControls::digits_cb(Gtk::Adjustment *adj)
{
    gfloat val=adj->get_value();
    m_vscale.set_digits(val);
    m_hscale.set_digits(val);
}

void RangeControls::psize_cb(Gtk::Adjustment *adj)
{
    Gtk::Adjustment *swadj=m_scrollbar.get_adjustment();
    gfloat val=adj->get_value();
    swadj->set_page_size(val);
    swadj->set_page_increment(val);

    // note that we don't have to emit the "changed" signal;
    // GTK-- does this for us
}

int main(int argc, char *argv[])
{
  Gtk::Main myapp(&argc, &argv);
  RangeControls rangecontrols;

  myapp.run();
  return 0;
}


