#include <gtk--/main.h>
#include <gtk--/window.h>
#include <gtk--/scrolledwindow.h>
#include <gtk--/label.h>
#include <gtk--/tree.h>
#include <gtk--/treeitem.h>

using SigC::slot;
using SigC::bind;

static int cb_quit(GdkEventAny*)
{
  Gtk::Main::quit();
  return true;
}

/* for all the Gtk::Item:: and Gtk::TreeItem:: signals */
static void cb_itemsignal (Gtk::TreeItem *item, gchar *signame)
{
  /* It's a GtkBin, so it has one child, which we know to be a
     label, so get that */
  Gtk::Label *label = dynamic_cast<Gtk::Label*>(item->get_child());

  /* Get the text of the label */
  Gtk::string name=label->get();

  /* Get the level of the tree which the item is in */
  g_print ("%s called for item %s->%p, level %d\n", signame, name.c_str(),
	   item, dynamic_cast<Gtk::Tree*>(item->get_parent())->get_level());
}

/* Note that this is never called */
static void cb_unselect_child (Gtk::Widget& child,Gtk::Tree *root_tree,
			       Gtk::Tree *subtree)
{
  g_print ("unselect_child called for root tree %p, subtree %p, child %p\n",
	   root_tree, subtree, &child);
}

/* Note that this is called every time the user clicks on an item,
   whether it is already selected or not. */
static void cb_select_child (Gtk::Widget& child,Gtk::Tree *root_tree,
			       Gtk::Tree *subtree)
{
  g_print ("select_child called for root tree %p, subtree %p, child %p\n",
	   root_tree, subtree, &child);
}

static void cb_selection_changed (Gtk::Tree *tree)
{
  g_print ("selection_change called for tree %p\n", tree);
  g_print ("selected objects are:\n");

  Gtk::Tree::SelectionList &selection=tree->selection();
  Gtk::Tree::SelectionList::iterator i=selection.begin();

  while (i!=selection.end())
    {
      /* Get a GtkWidget pointer from the list node */
      Gtk::TreeItem *item = (*i);
      Gtk::Label *label = dynamic_cast<Gtk::Label*>(item->get_child());
      Gtk::string name=label->get();
      g_print ("\t%s on level %d\n", 
                name.c_str(),
                dynamic_cast<Gtk::Tree*>(item->get_parent())->get_level());
      //g_print ("%p\n",item);
      ++i;
    }
}

int main (int argc, char *argv[])
{
  Gtk::Window *window;
  Gtk::ScrolledWindow *scrolled_win;
  Gtk::Tree *tree;
  static gchar *itemnames[] = {"Foo", "Bar", "Baz", "Quux",
			       "Maurice"};
  gint i;

  Gtk::Main myapp (argc, argv);

  /* a generic toplevel window */
  window = manage( new Gtk::Window (GTK_WINDOW_TOPLEVEL) );
  window->delete_event.connect(slot(&cb_quit));
  window->set_border_width(5);

  /* A generic scrolled window */
  scrolled_win = manage( new Gtk::ScrolledWindow () );
  scrolled_win->set_policy (GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  scrolled_win->set_usize (150, 200);
  window->add (*scrolled_win);
  scrolled_win->show();
  
  /* Create the root tree */
  tree = manage( new Gtk::Tree() );
  g_print ("root tree is %p\n", tree);

  /* connect all GtkTree:: signals */
  tree->select_child.connect(bind(slot(&cb_select_child), tree, tree));
  tree->unselect_child.connect(bind(slot(&cb_unselect_child), tree, tree));
  tree->selection_changed.connect(bind(slot(&cb_selection_changed),tree));

  /* Add it to the scrolled window */
  scrolled_win->add_with_viewport(*tree);

  /* Set the selection mode */
  tree->set_selection_mode (GTK_SELECTION_MULTIPLE);

  /* Show it */
  tree->show ();

  for (i = 0; i < 5; i++){
    Gtk::Tree *subtree;
    Gtk::TreeItem *item;
    gint j;

    /* Create a tree item */
    item = manage( new Gtk::TreeItem(itemnames[i]) );

    /* Connect all GtkItem:: and GtkTreeItem:: signals */
    item->select  .connect( 
      bind<Gtk::TreeItem*,char*>(slot(&cb_itemsignal),item,"select"));
    item->deselect.connect(
      bind<Gtk::TreeItem*,char*>(slot(&cb_itemsignal),item,"deselect"));
    item->toggle  .connect(
      bind<Gtk::TreeItem*,char*>(slot(&cb_itemsignal),item,"toggle"));
    item->expand  .connect(
      bind<Gtk::TreeItem*,char*>(slot(&cb_itemsignal),item,"expand"));
    item->collapse.connect(
      bind<Gtk::TreeItem*,char*>(slot(&cb_itemsignal),item,"collapse"));

    /* Add it to the parent tree */
    tree->append(*item);

    /* Show it - this can be done at any time */
    item->show();

    /* Create this item's subtree */
    subtree = manage( new Gtk::Tree() );
    g_print ("-> item %s->%p, subtree %p\n", itemnames[i], item,
	     subtree);

    /* This is still necessary if you want these signals to be called
       for the subtree's children.  Note that selection_change will be 
       signalled for the root tree regardless. */
    subtree->select_child.connect(bind(slot(&cb_select_child),tree,subtree));
    subtree->unselect_child.connect(bind(slot(&cb_unselect_child),tree,subtree));

    /* This has absolutely no effect, because it is completely ignored 
       in subtrees */
    subtree->set_selection_mode (GTK_SELECTION_SINGLE);

    /* Neither does this, but for a rather different reason - the
       view_mode and view_line values of a tree are propagated to
       subtrees when they are mapped.  So, setting it later on would
       actually have a (somewhat unpredictable) effect */
    subtree->set_view_mode (GTK_TREE_VIEW_ITEM);

    /* Set this item's subtree - note that you cannot do this until
       AFTER the item has been added to its parent tree! */
    item->set_subtree (*subtree);

    for (j = 0; j < 5; j++){
      Gtk::TreeItem *subitem;

      /* Create a subtree item, in much the same way */
      subitem = manage( new Gtk::TreeItem(itemnames[j]) );

      /* Connect all GtkItem:: and GtkTreeItem:: signals */
      subitem->select  .connect(
        bind<Gtk::TreeItem*,char*>(slot(&cb_itemsignal), subitem, "select"));
      subitem->deselect.connect(
        bind<Gtk::TreeItem*,char*>(slot(&cb_itemsignal), subitem, "deselect"));
      subitem->toggle  .connect(
        bind<Gtk::TreeItem*,char*>(slot(&cb_itemsignal), subitem, "toggle"));
      subitem->expand  .connect(
        bind<Gtk::TreeItem*,char*>(slot(&cb_itemsignal), subitem, "expand"));
      subitem->collapse.connect(
        bind<Gtk::TreeItem*,char*>(slot(&cb_itemsignal), subitem, "collapse"));

      g_print ("-> -> item %s->%p\n", itemnames[j], subitem);
      /* Add it to its parent tree */
      subtree->append (*subitem);

      /* Show it */
      subitem->show();
    }
  }

  /* Show the window and loop endlessly */
  window->show ();

  Gtk::Main::run();
  return 0;
}


