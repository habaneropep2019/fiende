#include <iostream>
#include <vector>
#include <gtk--/box.h>
#include <gtk--/button.h>
#include <gtk--/ctree.h>
#include <gtk--/window.h>
#include <gtk--/main.h>
#include <cstdio>

using std::cout;
using std::endl;

using std::vector;

class AppWindow: public Gtk::Window
  {
    public:
      AppWindow();
      virtual ~AppWindow();
      virtual gint delete_event_impl (GdkEventAny*);

      Gtk::VBox m_VBox; //Box for tree and HBox.
      Gtk::CTree* tree;
      
      Gtk::HBox m_Box_Buttons; //HBox for buttons.
      Gtk::Button m_Button_SelectAll, m_Button_SelectNone, m_Button_GetSelection;

   protected:
      //Signal handlers:
      virtual void on_Button_SelectAll();
      virtual void on_Button_SelectNone();
      virtual void on_Button_GetSelection();
  };

AppWindow::AppWindow()
    : Gtk::Window(),
      m_Button_SelectAll("Select All"),
      m_Button_SelectNone("SelectNone"),
      m_Button_GetSelection("Get Selection")
  {
    // create a tree with 2 columns (you can not resize it later)
    vector<char*> titles;
    titles.push_back("Items");
    titles.push_back("Column2");
    tree=manage(new Gtk::CTree(titles));
    tree->set_selection_mode(GTK_SELECTION_EXTENDED);
    tree->column(0).set_width(100);

    // Add some items
    { using namespace Gtk::CTree_Helpers;
    RowList::iterator i;
    vector<char*> item;
    item.push_back("fruit");
    item.push_back("1");
    tree->rows().push_back(Element(item));

    i=--tree->rows().end();
    item[0]="peaches";
    i->subtree().push_back(Element(item));
    item[0]="apples";
    i->subtree().push_back(Element(item));
    item[0]="pears";
    i->subtree().push_back(Element(item));

    item[0]="animals";
    tree->rows().push_back(BranchElem(item)); // with no children

    // demonstrate traversing the list
    cout << "traverse row list (forward)"<<endl;
    for (RowList::iterator i1=tree->rows().begin(); 
         i1!=tree->rows().end(); ++i1)
       cout << (*i1)[0].get_text() <<endl; 

    cout << "traverse tree list (forward)"<<endl;
    for (TreeList::iterator i1=tree->tree().begin(); 
         i1!=tree->tree().end(); ++i1)
       cout << (*i1)[0].get_text() <<endl; 

// FIXME reverse is broken
//    cout << "traverse row list (backward)"<<endl;
//    for (RowList::reverse_iterator i2=tree->rows().rbegin(); 
//         i2!=tree->rows().rend(); ++i2)
//       cout << (*i2)[0].get_text() <<endl; 

    } /* using Gtk::CTree_Helpers */

    // set up widget layout
    tree->set_usize(300,300);

    //Put the Buttons in the HBox:
    m_Box_Buttons.pack_start(m_Button_GetSelection);
    m_Box_Buttons.pack_start(m_Button_SelectAll);
    m_Box_Buttons.pack_start(m_Button_SelectNone);

    //Connect signals:
    m_Button_GetSelection.clicked.connect(slot(this, &AppWindow::on_Button_GetSelection));
    m_Button_SelectAll.clicked.connect(slot(this, &AppWindow::on_Button_SelectAll));
    m_Button_SelectNone.clicked.connect(slot(this, &AppWindow::on_Button_SelectNone));
    
    //Put the Buttons under the CTree:
    m_VBox.pack_start(*tree);
    m_VBox.pack_start(m_Box_Buttons);
    add(m_VBox);
    
    show_all();
  }

AppWindow::~AppWindow()
  {} 

gint AppWindow::delete_event_impl (GdkEventAny*)
  {
    Gtk::Main::quit();
    return 0;
  }

void AppWindow::on_Button_SelectAll()
  {
    Gtk::CTree_Helpers::SelectionList selectionList = tree->selection();
    selectionList.all();
  }

void AppWindow::on_Button_SelectNone()
  {
    Gtk::CTree_Helpers::SelectionList selectionList = tree->selection();
    selectionList.clear();
  }

void AppWindow::on_Button_GetSelection()
  {
    Gtk::CTree_Helpers::SelectionList selectionList = tree->selection();
    printf("Selection Count: %d\n", selectionList.size());

    gint iRow = 1;
    for(Gtk::CTree_Helpers::SelectionList::iterator iter = selectionList.begin(); iter != selectionList.end(); iter++)
    {
      //Get Row:
      Gtk::CTree_Helpers::Row& row = *iter;

      printf("  Row: %d\n", iRow);
      iRow++;
      
      //Iterate through Row:
      //(This could be recursive, but let's not complicate things)
      for(Gtk::CTree_Helpers::Row::iterator iterRow = row.begin(); iterRow != row.end(); iterRow++)
      {
        Gtk::CTree_Helpers::Cell& cell = *iterRow;
        printf("    Cell: %s\n", cell.get_text().c_str());
      }
    }
    
   
  }

int main (int argc, char *argv[])
{
  Gtk::Main kit(argc, argv);
  AppWindow arrows;

  kit.run ();
  return 0;
}
