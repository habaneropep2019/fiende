#include <gnome--.h>

using SigC::slot;

class myApp : public Gtk::Window
{
public:
  myApp();

protected:
  Gtk::Button *button1;
  Gtk::ToggleButton *button2;
  Gnome::Animator *animator1, *animator2, *animator3;

  void toggle_start_stop();
  void toggle_speed();
  static gint quit_cb(GdkEventAny *ev);
};

/**********************************************************/

myApp::myApp()
{
  Gtk::Box *box;
  set_title("The Great Animator");

  /***************************/
  string s = "gnome-default.png";

  animator1 = manage(new Gnome::Animator(100, 100));
  // Warning: Adding frames using a file name repeatedly is 
  // is slow, and should be avoided in real programs with long animations.
  animator1->append_frame(s, 36, 36, 200, 24, 24);
  animator1->append_frame(s, 30, 30, 50, 36, 36);
  animator1->append_frame(s, 28, 28, 50, 40, 40);
  animator1->append_frame(s, 24, 24, 50, 48, 48);
  animator1->append_frame(s, 23, 23, 50, 51, 51);
  animator1->append_frame(s, 17, 17, 50, 62, 62);
  animator1->append_frame(s, 9, 9, 50, 78, 78);
  animator1->append_frame(s, 3, 3, 50, 90, 90);
  animator1->append_frame(s, -2, -2, 50, 100, 100);
  animator1->append_frame(s, -12, -12, 50, 120, 120);
  animator1->append_frame(s, -27, -27, 50, 150, 150);
  animator1->append_frame(s, -52, -52, 50, 200, 200);
  animator1->append_frame(s, -102, -102, 100, 300, 300);
  // make the animation loop forward to back and then reverse
  animator1->set_loop_type(GNOME_ANIMATOR_LOOP_PING_PONG);
  animator1->start();

  /***************************/

  animator2 = manage(new Gnome::Animator(100, 100));
  // This is the mandatory "jumping Tux" animation.  
  // The animation is extracted from a large image, made up by 
  // tiling the frames horizontally.  The frames are then magnified
  // to match the desired size.
  animator2->append_frames("tux-jump.png", 0, 0, 300, 48, 100, 100);
  animator2->set_loop_type(GNOME_ANIMATOR_LOOP_RESTART);
  // Here we double the rate of the animation.  This reduces
  // the interval.  It is useful to increase or decrease the rate
  // of an animation without recreating it.
  animator2->set_playback_speed(2.0);
  animator2->start();

  /***************************/

  // And this is another animation, similiar to the Tux one, but
  //   without magnification and no shape. 
  animator3 = manage(new Gnome::Animator(48, 48));
  animator3->append_frames("email.png", 0, 0, 150, 48);
  animator3->set_loop_type(GNOME_ANIMATOR_LOOP_PING_PONG);
  animator3->set_playback_speed(1.0);
  animator3->start();

  /***************************/

  // This puts the foot animation into a Gtk::Button, to demonstrate how
  // the animator can behave (almost) like a shaped windows without
  // actually being one.  If we used a real shaped window, the
  // animation would be a lot less smooth.  Also notice that the window 
  // size is never larger than the specified size: the foot is always 
  // clipped to fit into this size, even if the button becomes larger. 
  button1 = manage(new Gtk::Button());
  button1->set_border_width(4);
  button1->add(*animator1);
  // Clicking on the foot starts/stops its animation. 
  button1->clicked.connect(slot(this, &myApp::toggle_start_stop));

  button2 = manage(new Gtk::ToggleButton());
  button2->set_border_width(4);
  button2->add(*animator2);
  // Toggling the button alters the playback speed
  button2->toggled.connect(slot(this, &myApp::toggle_speed));

  box = manage(new Gtk::HBox(true, 0));
  box->pack_start(*button1, false, true, 0);
  box->pack_start(*button2, false, true, 0);
  box->pack_start(*animator3, false, false, 0);
  add(*box);

  delete_event.connect(slot(&myApp::quit_cb));

  show_all();
}

void
myApp::toggle_start_stop()
{
  if (animator1->get_status() == GNOME_ANIMATOR_STATUS_STOPPED)
    animator1->start();
  else
    animator1->stop();
}

void
myApp::toggle_speed()
{
  if (button2->get_active())
    animator2->set_playback_speed(0.5);
  else
    animator2->set_playback_speed(2.0);
}


int
myApp::quit_cb(GdkEventAny *ev)
{
  Gnome::Main::quit();
  return 0;
}

/**********************************************************/

int main (int argc, char* argv[])
{
  Gnome::Main application("animator", "0.0", argc, argv);
  myApp window;
  application.run();

  return 0;
}

