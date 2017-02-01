#include <gnome--.h>

class Prefs;

class MDIWindow : public Gnome::MDI
{
public:
  MDIWindow();
  gint child_number;
  void prefs_window_closed();

protected:
  virtual void app_created_impl(Gnome::App& app);
  virtual gint delete_event_impl(GdkEventAny *event);
  void cleanup();

  Prefs *properties;

  void preferences();
  void new_child();
  void new_view();
  void close_child();
  void quit_cb();
  void nothing_cb();
  void dump_children();
};

class Prefs : public Gnome::PropertyBox
{
public:
  Prefs();
  ~Prefs();
  void start();
  MDIWindow *mdi_object;
private:
  Gtk::RadioButton *defaultMDI;
  Gtk::RadioButton *toplevelMDI;
  Gtk::RadioButton *notebookMDI;
  Gtk::RadioButton *modalMDI;

  void apply_prefs(int page);

};

class CustomMDIChild : public Gnome::MDIChild
{
public:
  CustomMDIChild(gint number);
  /* This method just forwards the call to the super-classes method. We need
   * it for connecting signals to it.
   */
  void create_view() { Gnome::MDIChild::create_view(); }

protected:
  gint child_number;
  gint view_number;
  /* Gnome::MDIChild is a virtual class that must be inherited by a custom
   * class, implementing at least create_view_impl(). This method returns a
   * pointer to a widget that represents the document contents.
   */
  virtual Gtk::Widget *create_view_impl();
  virtual Gtk::Widget* create_title_impl();
  virtual void update_title_impl(Gtk::Widget &old_label);
  void nothing_cb();
};

class CustomMDIView : public Gtk::VBox
{
public:
  CustomMDIView(gint child, gint view);
  SigC::Signal0<void> add_view;

protected:
  Gtk::Label label;
};

class CustomMDILabel : public Gtk::HBox
{
public:
  CustomMDILabel(const string &name);
  void set_name(const string &name) { label.set_text(name); }
protected:
  Gnome::Pixmap *pixmap;
  Gtk::Label label;
};
