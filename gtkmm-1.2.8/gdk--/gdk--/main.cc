#include <gdk--/main.h>

void Gdk::init(int &argc,char** &argv)
  {
   gdk_init(&argc,&argv);
  }

void Gdk::exit(int error_code)
  {
   gdk_exit(error_code);
  }

Gtk::string Gdk::set_locale()
  {
   gchar* locale=gdk_set_locale();
   return Gtk::string(locale);
  }

gint Gdk::events_pending()
  {
   return gdk_events_pending();
  }

GdkEvent* Gdk::event_get(void)
  {
   return gdk_event_get();
  }

void Gdk::event_put(GdkEvent &event)
  {
   gdk_event_put(&event);
  }

GdkEvent* Gdk::event_copy(GdkEvent* event)
  {
   return gdk_event_copy(event);
  }

void Gdk::event_free(GdkEvent* event)
  {
   gdk_event_free(event);
  }

#if GDK_VERSION_GT(1,0)
void Gdk::event_get_time(GdkEvent* event)
  {
   gdk_event_get_time(event);
  }

void Gdk::set_sm_client_id(const Gtk::string &sm_client_id)
  {
    // This one is declared in gdk.h, but never actually defined
    // gdk_set_sm_client_id(sm_client_id.c_str());
  }

#endif
