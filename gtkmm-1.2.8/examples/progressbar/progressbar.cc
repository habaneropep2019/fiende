#include <gtk--/progressbar.h>
#include <gtk--/table.h>
#include <gtk--/label.h>
#include <gtk--/button.h>
#include <gtk--/main.h>
#include <gtk--/window.h>

// Gtk-- version of the "progress bar" example from the gtk+ tutorial

class ProgressBar : public Gtk::Window
{
  bool m_pstat;
  
  Gtk::Button m_bReset, m_bCancel;
  Gtk::Label m_label;
  Gtk::ProgressBar m_pbar;
  Gtk::Table m_table;

  gint delete_event_impl(GdkEventAny*) { 
    Gtk::Main::quit(); return 0; 
  }
  
public:
  int progress();
  void progress_r() { m_pstat = false; }
  
  ProgressBar();
};

ProgressBar::ProgressBar() :
  m_pstat(true),
  m_bReset("Reset"), m_bCancel("Cancel"),
  m_label("Progress Bar Example"),
  m_table(3,2, true)
{
  set_border_width(10);
  add(m_table);

  // You don't need attach_default(), Gtk::Table::attach() already has
  // default args
  m_table.attach(m_label, 0,2,0,1);
  m_table.attach(m_pbar,  0,2,0,2);

  // Set the timeout to handle automatic updating of the progress bar
  Gtk::Main::timeout.connect(slot(this, &ProgressBar::progress),50);

  m_bReset.clicked.connect(slot(this, &ProgressBar::progress_r));
  m_table.attach(m_bReset, 0,1,2,3);
  
  m_bCancel.clicked.connect(Gtk::Main::quit.slot());

  m_table.attach(m_bCancel, 1,2,2,3);

  show_all();
}

int
ProgressBar::progress()
{
  gfloat pvalue = m_pbar.get_current_percentage();

  pvalue += 0.01;

  if ((pvalue >= 1.0) || (m_pstat == false)) {
    pvalue = 0.0;
    m_pstat = true;
  }

  m_pbar.set_percentage(pvalue);

  return true;
}


int main (int argc, char *argv[])
{
  Gtk::Main myapp(argc, argv);

  ProgressBar progressbar;

  myapp.run();
  return 0;
}
