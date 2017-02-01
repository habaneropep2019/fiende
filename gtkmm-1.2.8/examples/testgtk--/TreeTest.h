#ifdef JUNK

#include "TestFixture.h"

/*
 * Gtk::Tree
 */

class TreeTest : public TestFixture 
{
public:
  // functions
  static TestFixture * create ();
  virtual ~TreeTest () {};
  virtual void destroyTest () { delete theTest; theTest = 0;}
  //  data
  static TreeTest * theTest;
private:
  // ctor
  TreeTest ();
  // functions
  void addNewItem ();
  void remove_item ( Gtk::Tree * tree);
  void remove_subtree ( Gtk::Tree * tree);
  void tree_changed ( Gtk::Tree * tree);
  void create_subtree ( guint level, guint, nb_item_max, guint recursion_level_max);
  void create_tree_sample ( guint selection_mode, 
			    guint draw_line, guint view_line, guint no_root_item,
			    guint nb_item_max, guint recursion_level_max);
  void create_tree ();
  void create_tree_mode_window ();
  // data
  Gtk::VBox       box1;
  Gtk::VBox       box2;
  Gtk::VBox       box3;
  Gtk::HSeparator separator;

  GSList          *selectionModeGroup;
  Gtk::RadioButton  singleButton;
  Gtk::RadioButton  browseButton;
  Gtk::RadioButton  multipleButton;
  Gtk::CheckButton  drawLineButton;
  Gtk::CheckButton  viewLineButton;
  Gtk::CheckButton  noRootItemButton;
  Gtk::SpinButton  *nbItemSpinner;
  Gtk::SpinButton  *recursionSpinner;

  guint            nb_item_add;
  GtkWidget       *add_button;
  GtkWidget       *remove_button;
  GtkWidget       *subtree_button;

  Gtk::Tree        *tree;

};

#endif
