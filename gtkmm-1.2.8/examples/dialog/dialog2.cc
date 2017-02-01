//
// One of my biggest complaints about Gtk+-based programs is that
// popups never popup centered over the correct window.  At best they
// come up in the middle of the display or over the mouse.  More
// commonly, they quietly come up in some random location on the
// display.
// 
// The correct way, IMHO, to popup a dialog is to place it in the
// center of the window responsible for popping it.  The fact that it
// is centered over the window tells me that, yes, this pop up is part
// of the program underneath it.  When a dialog popups up over a
// different program, it is confusing because the program underneath
// the popup has no relationship with the popup itself.
// 
// What follows is an example of how to popup a dialog centered over
// any widget including a GTK_WINDOW widget.  It also shows how to
// raise an iconified popup to the same place where it was originally
// iconified.
// 
// If you find a better way of doing this, please let me know.
//  
//                                            Paul Serice
//                                            September 13, 2000
//
// Changed code a bit, works better but window "flashs" in the wrong
// position.  --Karl
//
#include <gtk--/main.h>
#include <gtk--/window.h>
#include <gtk--/dialog.h>
#include <gtk--/button.h>
#include <gtk--/box.h>
#include <gtk--/label.h>
#include <gtk--/eventbox.h>
#include <iostream>

using namespace Gtk;
using SigC::slot;

class MyDialog : public Dialog {

public:
  // create one dialog only
  static MyDialog* Instance() {
    if( _instance == 0 ) {
      _instance = new MyDialog;
    }
    return _instance;
  }

  /************************************************************/

  // make sure we are on top.
  void raise()
    {
      if (is_realized()) 
         get_window().show();
    }
 
  void center(gint x, gint y, gint w=-1, gint h=-1)
    {
      gint dw,dh;
      if (!is_realized()) realize();
      get_window().get_size(dw, dh);
      if (w==-1) w=x;
      if (h==-1) h=y;
      
      int cx = x + w/2 - dw/2;
      int cy = y + h/2 - dh/2;

      set_position(GTK_WIN_POS_NONE);
      set_uposition(cx, cy);
      raise();
    }

  void center()
    {
      gint w = gdk_screen_width ();
      gint h = gdk_screen_height ();
      center(0,0,w,h);
    }

  void center(Gtk::Widget& widget)
    {
      gint x, y, w, h;   
      if (!widget.is_mapped()) 
        {
          center();
          return;
        }
      Gdk_Window win=widget.get_window();
      win.get_origin(x, y);
      win.get_size(w, h);
      center(x,y,w,h);
    }

  void center_on_mouse()
    {
      if (!is_mapped())
        {
          set_position(GTK_WIN_POS_MOUSE);
          return;
        }
    }

  /************************************************************/

  void WindowManagerDecides()
  {
    show();
  }

  void CenterOverDisplay()
  {
    center();
    show();
  }

  void CenterOverMouse()
  {
    // You have to call set_position() before the map event.
    set_position(GTK_WIN_POS_MOUSE);
    center_on_mouse();
    show();
  }

  void CenterOverWidget(Widget* targetWidget)
  {
    center(*targetWidget);
    show();
  }

protected:
  MyDialog()
  {
    set_usize(100, 100);
    set_title("Little Dialog");
 
    Gtk::Button *button=manage(new Gtk::Button("Ok"));
    get_action_area()->pack_start(*button);
    button->clicked.connect(hide.slot());
    button->show();
  }

  // don't close the window, hide it
  virtual gint delete_event_impl(GdkEventAny* p0)
  {
    hide();
    return 1;
  }

private:
  static MyDialog* _instance;
};

MyDialog* MyDialog::_instance(0);


class MyWindow : public Window {

public:
  MyWindow() {

    set_usize(480, 360);

    Box* mainBox = manage(new HBox);

    Box* leftBox = manage(new VBox);
    mainBox->pack_start(*leftBox);

    Box* centralBox = manage(new VBox);
    mainBox->pack_start(*centralBox, /* expand */ false, /* fill */ false);

    Box* topBox = manage(new HBox);
    centralBox->pack_start(*topBox);

  
    // To center over a widget, it needs its own GdkWindow.  So, wrap
    // the Label in an EventBox to achieve this.
    EventBox* eventBox = new EventBox;
    Label* label = manage(new Label("Center Over Me"));
    eventBox->add(*label);


    //
    // Set up the button callbacks to directly call MyDialog methods.
    //

    MyDialog* dialog = MyDialog::Instance();

    Button* windowManagerDecidesButton
      = manage(new Button("Window Manager Decides"));
    windowManagerDecidesButton
      ->clicked.connect(slot(dialog, &MyDialog::WindowManagerDecides));

    Button* centerOverDisplayButton
      = manage(new Button("Center Over Display"));
    centerOverDisplayButton
      ->clicked.connect(slot(dialog, &MyDialog::CenterOverDisplay));
    Button* centerOverMouseButton
      = manage(new Button("Center Over Mouse"));
    centerOverMouseButton
      ->clicked.connect(slot(dialog, &MyDialog::CenterOverMouse));

    Button* centerOverWindowButton
      = manage(new Button("Center Over Window"));
    centerOverWindowButton
      ->clicked.connect(bind(slot(dialog, &MyDialog::CenterOverWidget),
                             this));

    Button* centerOverWidgetButton
      = manage(new Button("Center Over Widget"));
    centerOverWidgetButton
      ->clicked.connect(bind(slot(dialog, &MyDialog::CenterOverWidget),
                             eventBox));

    centralBox->pack_start(*windowManagerDecidesButton,false,false);
    centralBox->pack_start(*centerOverDisplayButton,false,false);
    centralBox->pack_start(*centerOverMouseButton,false,false);
    centralBox->pack_start(*centerOverWindowButton,false,false);
    centralBox->pack_start(*centerOverWidgetButton,false,false);

    Box* bottomBox = manage(new HBox);
    centralBox->pack_start(*bottomBox);

    centralBox->pack_start(*eventBox);

    Box* rightBox = manage(new VBox);
    mainBox->pack_start(*rightBox);

    add(*mainBox);
  }

protected:
  virtual gint delete_event_impl(GdkEventAny* p0)
  {
    Kit::quit();
    return 0;
  }

};

int main(int argc, char* argv[])
{
  Kit kit(argc, argv);
  MyWindow win;
  win.show_all();
  kit.run();
  return 0;
}

