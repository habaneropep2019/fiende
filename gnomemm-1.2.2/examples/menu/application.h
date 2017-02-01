#include <gnome--/app.h>
#include <gnome--/appbar.h>
#include <gtk--/box.h>
#include <gtk--/button.h>
#include <gtk--/main.h>
#include <vector>
#include <list>

class AppExample : public Gnome::App
{
public:
  AppExample();
  virtual ~AppExample();

protected:

  virtual void init();

  virtual void install_menus();
  virtual void install_toolbars();
  virtual void close();

  virtual int delete_event_impl(GdkEventAny *event);

  
  //Signal handlers:
  virtual void on_menu_generic();
  virtual void on_menu_file_exit();

  virtual void on_button_insert();
  virtual void on_button_disable();
  virtual void on_button_enable();


  typedef std::vector<Gnome::UI::SubTree> type_vecGnome_UI_SubTree;
  typedef std::vector<Gnome::UI::Info> type_vecGnome_UI_Info;
  
  //Member widgets:
  Gnome::AppBar m_Status;
  Gtk::VBox m_VBox;
  Gtk::HBox m_HBox;
  Gtk::Button m_Button_Insert, m_Button_Disable, m_Button_Enable;

  Gtk::Widget* m_pMenuItem;
  Gtk::Widget* m_pToolbarItem;
};

