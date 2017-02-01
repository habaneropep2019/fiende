#ifdef JUNK

#include "TreeTest.h"

TreeTest * TreeTest::theTest = 0;

TreeTest::TreeTest () :
  box2 ( false, 5 ),
  box3 ( false, 5 ),
  frame ( new Gtk::Frame ( "Selection Mode" ) ),
  selectionModeGroup ( 0 ),
  singleButton ( selectionModeGroup, "SINGLE" ),
  browseButton ( selectionModeGroup, "BROWSE" ),
  multipleButton ( selectionModeGroup, "MULTIPLE" ),
  drawLineButton ( "Draw Line" ),
  viewLineButton ( "View Line mode" ),
  noRootItemButton ( "Without Root item" ),
  tree ( 0 )
{
  childrenToDelete . push_back ( box4 );

  set_title ( "Set Tree Parameters");
  action_area . pack_start ( box1 );    
      /* create upper box - selection box */
  box1 . pack_start ( box2 );
  box2 . set_border_width ( 5 );
  box2 . pack_start ( box3 );
      /* create selection mode frame */
  box3 . pack_start ( frame );
  Gtk::VBox * vbox = new Gtk::VBox ( false, 0 );
  frame -> add ( *vbox );
  vbox -> set_border_width ( 5 );
  vbox -> pack_start ( singleButton );
  vbox -> pack_start ( browseButton );
  vbox -> pack_start ( multipleButton );
  /* create option mode frame */
  frame = new Gtk::Frame ( "Options" );
  childrenToDelete . push_back ( frame );
  box3 . pack_start ( *frame );
  vbox = new GtkVBox ( false, 0 );
  childrenToDelete . push_back ( vbox );
  frame -> add ( vbox );
  vbox -> set_border_width ( 5 );
  /* create check button */
  vbox -> pack_start ( drawLineButton );
  drawLineButton . set_state ( true );
  vbox -> pack_start ( viewLineButton );
  viewLineButton . set_state ( true );
  vbox -> pack_start ( noRootItemButton );
  noRootItemButton . set_state ( true );
  /* create recursion parameter */
  frame = new GtkFrame ( "Size Parameters" );
  childrenToDelete . push_back ( frame );
  box2 . pack_start ( *frame );
  Gtk::HBox * hbox = new GtkHBox ( false, 5 );
  childrenToDelete . push_back ( hbox );
  frame -> add ( hbox );
  hbox -> set_border_width( 5 );
  /* create number of item spin button */
  Gtk::HBox * hbox2 = new Gtk::HBox ( false, 5 );
  childrenToDelete . push_back ( hbox2 );
  hbox -> pack_start ( *hbox2 );
  Gtk::Label * label = new Gtk::Label ( "Number of items : " );
  childrenToDelete . push_back ( label );
  label . set_alignment ( 0, 0.5 );
  hbox2 -> pack_start ( label );
  Gtk::Adjustment * adj = new Gtk::Adjustment ( (gfloat)DEFAULT_NUMBER_OF_ITEM,  // value
					      1.0,                             // lower
					      255.0,                           // upper
					      1.0,                             // step_increment
					      5.0,                             // page_increment
					      0.0);                            // page_size
  childrenToDelete . push_back ( adj );
  nbItemSpinner = new Gtk::SpinButton ( adj, 0, 0 );
  childrenToDelete . push_back ( nbItemSpinner );
  hbox2 -> pack_start ( *nbItemSpinner );
  /* create recursion level spin button */
  hbox2 = new Gtk::HBox ( FALSE, 5 );
  childrenToDelete . push_back ( hbox2 );
  hbox -> pack_start ( hbox2 );
  label = new Gtk::Label ( "Depth : " );
  childrenToDelete . push_back ( label );
  label -> set_alignment ( 0, 0.5 );
  hbox2 -> pack_start ( *label );
  adj = new Gtk::Adjustment ( static_cast < gfloat > ( DEFAULT_RECURSION_LEVEL ) , 
			     0.0, 255.0, 1.0, 5.0, 0.0 );
  childrenToDelete . push_back ( adj );
  recursionSpinner = new Gtk::SpinButton ( adj, 0, 0 );
  childrenToDelete . push_back ( recursionSpinner );
  hbox2 -> pack_start ( *recursionSpinner );
  /* create horizontal separator */
  box1 . pack_start ( separator );
  /* create bottom button box */
  hbox = new Gtk::HBox ( true, 10 );
  childrenToDelete . push_back ( hbox );
  box1 . pack_start ( *hbox );
  hbox -> set_border_width ( 5 );
  Gtk::Button *button = new Gtk::Button( "Create Tree" );
  childrenToDelete . push_back ( button );
  box2 . pack_start ( *button );
  button->clicked.connect(bind(slot(this,cb_create_tree),0));

  button = new Gtk::Button ( "Close" );
  childrenToDelete . push_back ( button );
  box2 . pack_start ( *button );
  button->clicked.connect(bind(finished.slot(),this));

  show_all ();
}

static void
TreeTest::addNewItem ()
{
  /*
  sTreeButtons* tree_buttons;
  GList* selected_list;
  GtkWidget* selected_item;
  GtkWidget* subtree;
  GtkWidget* item_new;
  char buffer[255];
  */
  Gtk::Tree * subtree;
  Gtk::TreeItem * selectedItem;
  
  if( tree -> beginselection () == tree -> endselection () )
    {
      /* there is no item in tree */
      subtree = tree;
    }
  else
    {
      /* list can have only one element */
      selectedItem = beginselection ();
      subtree = selectedItem -> get_subtree ();
      if( subtree == NULL )
	{
	  /* current selected item have not subtree ... create it */
	  subtree = new Gtk::Tree;
	  childrenToDelete . push_back ( subtree );
	  selectedItem -> set_subtree ( subtree );
	}
    }
  /* at this point, we know which subtree will be used to add new item */
  /* create a new item */
  strstream ss;
  ss << "item add " << nb_item_add << ends;
  
  Gtk::TreeItem *itemNew = new Gtk::TreeItem ( ss . str () );
  childrenToDelete . push_back ( itemNew );

  subtree -> append ( *itemNew);
  itemNew -> show ();

  nb_item_add++;
}

void
TreeTest::removeItem ()
{

  G_list < Gtk::TreeItem > clear_list;

  for ( Gtk::Tree::selectioniterator iter = tree -> beginselection (); 
	iter != tree -> endselection ();
	iter++ )
    {
      clear_list . prepend ( *iter );
    }

  tree -> remove_items ( clear_list );
}


void
TreeTest::removeSubtree ()
{
  
  //  GList* selected_list;
  //  GtkTreeItem *item;
  
  //  selected_list = GTK_TREE_SELECTION(tree);
  //  if (selected_list)
  if ( tree -> beginselection () != tree -> endselection () )
    {
      Gtk::TreeItem * treeItem = *( tree -> beginselection () )
      if ( tree -> beginselection () 
      item = GTK_TREE_ITEM (selected_list->data);
      if (item->subtree)
	gtk_tree_item_remove_subtree (item);
    }
}

static void
cb_tree_changed(GtkTree* tree)
{
  sTreeButtons* tree_buttons;
  GList* selected_list;
  guint nb_selected;

  tree_buttons = gtk_object_get_user_data(GTK_OBJECT(tree));

  selected_list = GTK_TREE_SELECTION(tree);
  nb_selected = g_list_length(selected_list);

  if(nb_selected == 0) 
    {
      if(tree->children == NULL)
	gtk_widget_set_sensitive(tree_buttons->add_button, TRUE);
      else
	gtk_widget_set_sensitive(tree_buttons->add_button, FALSE);
      gtk_widget_set_sensitive(tree_buttons->remove_button, FALSE);
      gtk_widget_set_sensitive(tree_buttons->subtree_button, FALSE);
    } 
  else 
    {
      gtk_widget_set_sensitive(tree_buttons->remove_button, TRUE);
      gtk_widget_set_sensitive(tree_buttons->add_button, (nb_selected == 1));
      gtk_widget_set_sensitive(tree_buttons->subtree_button, (nb_selected == 1));
    }  
}

static void 
create_subtree(GtkWidget* item, guint level, guint nb_item_max, guint recursion_level_max)
{
  GtkWidget* item_subtree;
  GtkWidget* item_new;
  guint nb_item;
  char buffer[255];
  int no_root_item;

  if(level == recursion_level_max) return;

  if(level == -1)
    {
      /* query with no root item */
      level = 0;
      item_subtree = item;
      no_root_item = 1;
    }
  else
    {
      /* query with no root item */
      /* create subtree and associate it with current item */
      item_subtree = gtk_tree_new();
      no_root_item = 0;
    }
  
  for(nb_item = 0; nb_item < nb_item_max; nb_item++)
    {
      sprintf(buffer, "item %d-%d", level, nb_item);
      item_new = gtk_tree_item_new_with_label(buffer);
      gtk_tree_append(GTK_TREE(item_subtree), item_new);
      create_subtree(item_new, level+1, nb_item_max, recursion_level_max);
      gtk_widget_show(item_new);
    }

  if(!no_root_item)
    gtk_tree_item_set_subtree(GTK_TREE_ITEM(item), item_subtree);
}

static void
create_tree_sample(guint selection_mode, 
		   guint draw_line, guint view_line, guint no_root_item,
		   guint nb_item_max, guint recursion_level_max) 
{
  GtkWidget* window;
  GtkWidget* box1;
  GtkWidget* box2;
  GtkWidget* separator;
  GtkWidget* button;
  GtkWidget* scrolled_win;
  GtkWidget* root_tree;
  GtkWidget* root_item;
  sTreeButtons* tree_buttons;

  /* create tree buttons struct */
  if ((tree_buttons = g_malloc (sizeof (sTreeButtons))) == NULL)
    {
      g_error("can't allocate memory for tree structure !\n");
      return;
    }
  tree_buttons->nb_item_add = 0;

  /* create top level window */
  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(window), "Tree Sample");
  gtk_signal_connect(GTK_OBJECT(window), "destroy",
		     (GtkSignalFunc) cb_tree_destroy_event, NULL);
  gtk_object_set_user_data(GTK_OBJECT(window), tree_buttons);

  box1 = gtk_vbox_new(FALSE, 0);
  gtk_container_add(GTK_CONTAINER(window), box1);
  gtk_widget_show(box1);

  /* create tree box */
  box2 = gtk_vbox_new(FALSE, 0);
  gtk_box_pack_start(GTK_BOX(box1), box2, TRUE, TRUE, 0);
  gtk_container_set_border_width(GTK_CONTAINER(box2), 5);
  gtk_widget_show(box2);

  /* create scrolled window */
  scrolled_win = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_win),
				  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_box_pack_start (GTK_BOX (box2), scrolled_win, TRUE, TRUE, 0);
  gtk_widget_set_usize (scrolled_win, 200, 200);
  gtk_widget_show (scrolled_win);
  
  /* create root tree widget */
  root_tree = gtk_tree_new();
  gtk_signal_connect(GTK_OBJECT(root_tree), "selection_changed",
		     (GtkSignalFunc)cb_tree_changed,
		     (gpointer)NULL);
  gtk_object_set_user_data(GTK_OBJECT(root_tree), tree_buttons);
  gtk_container_add(GTK_CONTAINER(scrolled_win), root_tree);
  gtk_tree_set_selection_mode(GTK_TREE(root_tree), selection_mode);
  gtk_tree_set_view_lines(GTK_TREE(root_tree), draw_line);
  gtk_tree_set_view_mode(GTK_TREE(root_tree), !view_line);
  gtk_widget_show(root_tree);

  if ( no_root_item )
    {
      /* set root tree to subtree function with root item variable */
      root_item = GTK_WIDGET(root_tree);
    }
  else
    {
      /* create root tree item widget */
      root_item = gtk_tree_item_new_with_label("root item");
      gtk_tree_append(GTK_TREE(root_tree), root_item);
      gtk_widget_show(root_item);
     }
  create_subtree(root_item, -no_root_item, nb_item_max, recursion_level_max);

  box2 = gtk_vbox_new(FALSE, 0);
  gtk_box_pack_start(GTK_BOX(box1), box2, FALSE, FALSE, 0);
  gtk_container_set_border_width(GTK_CONTAINER(box2), 5);
  gtk_widget_show(box2);

  button = gtk_button_new_with_label("Add Item");
  gtk_widget_set_sensitive(button, FALSE);
  gtk_signal_connect(GTK_OBJECT (button), "clicked",
		     (GtkSignalFunc) cb_add_new_item, 
		     (gpointer)root_tree);
  gtk_box_pack_start(GTK_BOX(box2), button, TRUE, TRUE, 0);
  gtk_widget_show(button);
  tree_buttons->add_button = button;

  button = gtk_button_new_with_label("Remove Item(s)");
  gtk_widget_set_sensitive(button, FALSE);
  gtk_signal_connect(GTK_OBJECT (button), "clicked",
		     (GtkSignalFunc) cb_remove_item, 
		     (gpointer)root_tree);
  gtk_box_pack_start(GTK_BOX(box2), button, TRUE, TRUE, 0);
  gtk_widget_show(button);
  tree_buttons->remove_button = button;

  button = gtk_button_new_with_label("Remove Subtree");
  gtk_widget_set_sensitive(button, FALSE);
  gtk_signal_connect(GTK_OBJECT (button), "clicked",
		     (GtkSignalFunc) cb_remove_subtree, 
		     (gpointer)root_tree);
  gtk_box_pack_start(GTK_BOX(box2), button, TRUE, TRUE, 0);
  gtk_widget_show(button);
  tree_buttons->subtree_button = button;

  /* create separator */
  separator = gtk_hseparator_new();
  gtk_box_pack_start(GTK_BOX(box1), separator, FALSE, FALSE, 0);
  gtk_widget_show(separator);

  /* create button box */
  box2 = gtk_vbox_new(FALSE, 0);
  gtk_box_pack_start(GTK_BOX(box1), box2, FALSE, FALSE, 0);
  gtk_container_set_border_width(GTK_CONTAINER(box2), 5);
  gtk_widget_show(box2);

  button = gtk_button_new_with_label("Close");
  gtk_box_pack_start(GTK_BOX(box2), button, TRUE, TRUE, 0);
  gtk_signal_connect_object(GTK_OBJECT (button), "clicked",
			    (GtkSignalFunc) gtk_widget_destroy, 
			    GTK_OBJECT(window));
  gtk_widget_show(button);

  gtk_widget_show(window);
}

static void
cb_create_tree(GtkWidget* w)
{
  guint selection_mode = GTK_SELECTION_SINGLE;
  guint view_line;
  guint draw_line;
  guint no_root_item;
  guint nb_item;
  guint recursion_level;

  /* get selection mode choice */
  if(GTK_TOGGLE_BUTTON(sTreeSampleSelection.single_button)->active)
    selection_mode = GTK_SELECTION_SINGLE;
  else
    if(GTK_TOGGLE_BUTTON(sTreeSampleSelection.browse_button)->active)
      selection_mode = GTK_SELECTION_BROWSE;
    else
      selection_mode = GTK_SELECTION_MULTIPLE;

  /* get options choice */
  draw_line = GTK_TOGGLE_BUTTON(sTreeSampleSelection.draw_line_button)->active;
  view_line = GTK_TOGGLE_BUTTON(sTreeSampleSelection.view_line_button)->active;
  no_root_item = GTK_TOGGLE_BUTTON(sTreeSampleSelection.no_root_item_button)->active;
    
  /* get levels */
  nb_item = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(sTreeSampleSelection.nb_item_spinner));
  recursion_level = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(sTreeSampleSelection.recursion_spinner));

  if (pow (nb_item, recursion_level) > 10000)
    {
      g_print ("%g total items? That will take a very long time. Try less\n",
	       pow (nb_item, recursion_level));
      return;
    }

  create_tree_sample(selection_mode, draw_line, view_line, no_root_item, nb_item, recursion_level);
}

#endif

