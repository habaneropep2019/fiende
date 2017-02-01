/* G Calendar
 * Copyright (C) 1998 Cesar Miquel, Shawn T. Amundson, Mattias Grönlund
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <gtk--/fontselection.h>
#include <gtk--/window.h>
#include <gtk--/calendar.h>
#include <gtk--/style.h>
#include <gtk--/label.h>
#include <gtk--/main.h>
#include <gtk--/checkbutton.h>
#include <gtk--/frame.h>
#include <gtk--/box.h>
#include <gtk--/separator.h>
#include <gtk--/buttonbox.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

using namespace SigC;

#define DEF_PAD 10
#define DEF_PAD_SMALL 5

#define TM_YEAR_BASE 1900

class Calendar : public Gtk::Window {
  Gtk::CheckButton *flag_checkboxes[5];
  gboolean  settings[5];
  gchar     *font;
  Gtk::FontSelectionDialog *font_dialog;
  Gtk::Calendar *calendar;
  Gtk::Label *selected;
  Gtk::Label *selected_double_click;
  Gtk::Label *month;

public:
  Calendar();
  ~Calendar();

  void set_flags();
  void font_selection_ok();
  void select_font();
  void destroy_widget(Gtk::FontSelectionDialog *w);
  void toggle_flag(Gtk::CheckButton *toggle);

  void month_changed ();
  void day_selected ();
  void day_selected_double_click ();
};

int destroy_handler(GdkEventAny *) {
	Gtk::Main::quit();
	return 0;
}

Calendar::~Calendar() {
  int i;

  for (i=0; i<5; i++) {
	delete flag_checkboxes[i];
  }
  delete calendar;
  delete selected;
  delete selected_double_click;
  delete month;
}

/*
 * GtkCalendar
 */

void Calendar::month_changed ()
{
  char buffer[256];
  struct tm tm;
  time_t time;
  guint year, mon, day;

  memset (&tm, 0, sizeof (tm));
  calendar->get_date (&year, &mon, &day);
  tm.tm_year = year - TM_YEAR_BASE;
  tm.tm_mon = mon;
  tm.tm_mday = day;

  time = mktime(&tm);
  strftime (buffer, 255, "%x", gmtime(&time));
  month->set(buffer);
}

void Calendar::day_selected ()
{
  char buffer[256];
  struct tm tm;
  time_t time;
  guint year, mon, day;

  memset (&tm, 0, sizeof (tm));
  calendar->get_date (&year, &mon, &day);
  tm.tm_year = year - TM_YEAR_BASE;
  tm.tm_mon = mon;
  tm.tm_mday = day;

  time = mktime(&tm);
  strftime (buffer, 255, "%x", gmtime(&time));
  selected->set(buffer);
}

void Calendar::day_selected_double_click ()
{
  char buffer[256];
  struct tm tm;
  time_t time;
  guint year, mon, day;

  memset (&tm, 0, sizeof (tm));
  calendar->get_date (&year, &mon, &day);
  tm.tm_year = year - TM_YEAR_BASE;
  tm.tm_mon = mon;
  tm.tm_mday = day;

  time = mktime(&tm);
  strftime (buffer, 255, "%x", gmtime(&time));
  selected_double_click->set(buffer);
}

void
Calendar::set_flags()
{
  gint i;
  gint options=0;

  for (i=0; i<5; i++) 
    if (settings[i])
    {
      options = options + (1<<i);
    }
  if (calendar)
    gtk_calendar_display_options(calendar->gtkobj(), (GtkCalendarDisplayOptions)options);
}

void 
Calendar::toggle_flag(Gtk::CheckButton *toggle)
{
  gint i;
  gint j;

  j = 0;
  for (i=0; i<5; i++)
    if (flag_checkboxes[i] == (Gtk::CheckButton *)toggle)
      j = i;

  settings[j] = !settings[j];
  set_flags();
}

void Calendar::font_selection_ok()
{
  GtkStyle *style;
  GdkFont  *font0;
  GtkWidget *w;

  font = (gchar *)(font_dialog->get_font_name().gc_str());
  if (calendar)
    {
      w = (GtkWidget *)calendar->gtkobj();
      font0 = font_dialog->get_font();
      if (font0) 
	{
          style = gtk_style_copy (gtk_widget_get_style(w));
	  gdk_font_unref (style->font);
	  style->font = font0;
	  gdk_font_ref (style->font);
	  gtk_widget_set_style (w, style);
	}
    }
}
void Calendar::destroy_widget(Gtk::FontSelectionDialog *w)
{
    gtk_widget_destroy((GtkWidget *)w->gtkobj());
}

void Calendar::select_font()
{
  Gtk::FontSelectionDialog *window;

  if (!font_dialog) {
    window = manage(new Gtk::FontSelectionDialog("Font Selection Dialog"));
    g_return_if_fail(Gtk::FontSelectionDialog::isA(window));
    font_dialog = window;
    
    window->set_position(GTK_WIN_POS_MOUSE);
    window->destroy.connect(bind(slot(this, &Calendar::destroy_widget), font_dialog));
    window->get_ok_button()->clicked.connect(slot(this, &Calendar::font_selection_ok));
    window->get_cancel_button()->clicked.connect(bind(slot(this, &Calendar::destroy_widget), font_dialog));
  }
  window=font_dialog;
  if (!window->is_visible())
    window->show();
  else
    window->destroy();

}

Calendar::Calendar() : Gtk::Window(GTK_WINDOW_TOPLEVEL) {
  Gtk::VBox *vbox, *vbox2, *vbox3;
  Gtk::HBox *hbox;
  Gtk::HButtonBox *hbbox;
  Gtk::CheckButton *toggle;
  Gtk::Button *button;
  Gtk::Frame *frame;
  Gtk::VSeparator *separator;
  Gtk::Label *label;
  Gtk::HButtonBox *bbox;
  gint i;
  
  struct {
    char *label;
  } flags[] =
    {
      { "Show Heading" },
      { "Show Day Names" },
      { "No Month Change" },
      { "Show Week Numbers" },
      { "Week Start Monday" }
    };

  
  font = NULL;
  font_dialog = NULL;

  for (i=0; i<5; i++) {
    settings[i]=0;
  }

  set_border_width (5);
  destroy.connect(Gtk::Main::quit.slot());
  delete_event.connect(slot(&destroy_handler));

  set_policy(false, false, true);

  vbox = manage(new Gtk::VBox(false, DEF_PAD));
  add(*vbox);

  /*
   * The top part of the Calendar, flags and fontsel.
   */

  hbox = manage(new Gtk::HBox(false, DEF_PAD));
  vbox->pack_start(*hbox, true, true, DEF_PAD);
  hbbox = manage(new Gtk::HButtonBox());
  hbox->pack_start(*hbbox, false, false, DEF_PAD);
  hbbox->set_layout(GTK_BUTTONBOX_SPREAD);
  hbbox->set_spacing(5);

  /* Calendar widget */
  frame = manage(new Gtk::Frame("Calendar"));
  hbbox->pack_start(*frame, true, true, DEF_PAD);
  calendar = new Gtk::Calendar();
  set_flags();
  calendar->mark_day (19);	
  frame->add(*calendar);
  calendar->month_changed.connect(slot(this, &Calendar::month_changed));
  calendar->day_selected.connect(slot(this, &Calendar::day_selected));
  calendar->day_selected_double_click.connect(slot(this, &Calendar::day_selected_double_click));

  separator = manage(new Gtk::VSeparator());
  hbox->pack_start (*separator, false, true, 0);

  vbox2 = manage(new Gtk::VBox(false, DEF_PAD));
  hbox->pack_start(*vbox2, false, false, DEF_PAD);
  
  /* Build the Right frame with the flags in */ 

  frame = manage(new Gtk::Frame("Flags"));
  vbox2->pack_start(*frame, true, true, DEF_PAD);
  vbox3 = manage(new Gtk::VBox(true, DEF_PAD_SMALL));
  frame->add(*vbox3);

  for (i = 0; i < 5; i++)
    {
      toggle = new Gtk::CheckButton(flags[i].label);
      toggle->toggled.connect(bind(slot(this, &Calendar::toggle_flag), toggle));
      vbox3->pack_start (*toggle, true, true, 0);
      flag_checkboxes[i] = toggle;
    }
  /* Build the right font-button */ 
  button = manage(new Gtk::Button("Font..."));
  button->clicked.connect(slot(this, &Calendar::select_font));
  vbox2->pack_start (*button, false, false, 0);

  /*
   *  Build the Signal-event part.
   */

  frame = manage(new Gtk::Frame("Signal events"));
  vbox->pack_start(*frame, true, true, DEF_PAD);
  vbox2 = manage(new Gtk::VBox(true, DEF_PAD_SMALL));
  frame->add(*vbox2);
  
  hbox = manage(new Gtk::HBox(false, 5));
  vbox2->pack_start (*hbox, false, true, 0);
  label = manage(new Gtk::Label("Day selected:"));
  hbox->pack_start (*label, false, true, 0);
  selected = new Gtk::Label("");
  hbox->pack_start (*selected, false, true, 0);

  hbox = manage(new Gtk::HBox(false, 5));
  vbox2->pack_start (*hbox, false, true, 0);
  label = manage(new Gtk::Label("Day selected double click:"));
  hbox->pack_start (*label, false, true, 0);
  selected_double_click = new Gtk::Label("");
  hbox->pack_start (*selected_double_click, false, true, 0);

  hbox = manage(new Gtk::HBox(false, 5));
  vbox2->pack_start (*hbox, false, true, 0);
  label = manage(new Gtk::Label("Month change:")); 
  hbox->pack_start (*label, false, true, 0);
  month = new Gtk::Label("");
  hbox->pack_start(*month, false, true, 0);
  
  bbox = manage(new Gtk::HButtonBox());
  vbox->pack_start(*bbox, false, false, 0);
  bbox->set_layout(GTK_BUTTONBOX_END);

  button = manage(new Gtk::Button("Close"));
  button->clicked.connect(Gtk::Main::quit.slot()); 
  bbox->add(*button);
  button->set_flags(GTK_CAN_DEFAULT);
  button->grab_default();

  show_all();
}


int
main(int argc, char *argv[])
{
  Gtk::Main myapp(&argc, &argv, true);
  Calendar calendar;
  myapp.run();
  return 0;
}
