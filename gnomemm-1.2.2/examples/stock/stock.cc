#include <gnome--.h>


/************************************************************/

typedef const char* StockType;

class StockPixmapTable : public Gtk::Table
  {
    int r,c;
    public:
      void set_active(bool active);
      void add(const string &,StockType pix,StockType menu=0);
      StockPixmapTable();
  };

class StockButtonTable : public Gtk::Table
  {
    int r,c;
    public:
      void set_active(bool active);
      void add(StockType button);
      StockButtonTable();
  };

/************************************************************/

StockPixmapTable::StockPixmapTable() : Gtk::Table(8*3,10)
  {
    r=c=0; 
    for (int i=0;i<9;i++)    set_col_spacing(i,20);
    for (int i=2;i<8*3-1;i+=3) set_row_spacing(i,20);
  }

void set_active_cb(Gtk::Widget& w, bool active)
  {
     w.set_sensitive(active); 
  }

void StockPixmapTable::set_active(bool active) 
  {
    foreach(bind(slot(&set_active_cb),active));
  }

void StockPixmapTable::add(const string &s,StockType pix,StockType menu=0)
  {
    Gnome::Pixmap *p1,*p2;
    Gtk::Label* l;
    l=manage(new Gtk::Label(s));
    l->show();
    attach(*l,c,c+1,r+2,r+3);

    p1=manage(new Gnome::StockPixmap(pix));
    p1->show();
    attach(*p1,c,c+1,r,r+1);
 
    if (menu)
      {
        p2=manage(new Gnome::StockPixmap(menu));
        p2->show();
        attach(*p2,c,c+1,r+1,r+2);
      }
 
    c+=1;
    if (c==10)
      {
        c=0;
        r+=3;
      }
  }


/************************************************************/
StockButtonTable::StockButtonTable() : Gtk::Table(4*2,3)
  {
    r=c=0;
    for (int i=0;i<2;i++) set_col_spacing(i,20);
    for (int i=1;i<4*2-1;i+=2) set_row_spacing(i,20);
  }

void StockButtonTable::set_active(bool active)
  {
    foreach(bind(slot(&set_active_cb),active));
  }

void StockButtonTable::add(StockType button)
  {
    Gtk::Button *p1;
    Gtk::Label* l;
    l=manage(new Gtk::Label(button));
    l->show();
    attach(*l,c,c+1,r+1,r+2);

    p1=manage(new Gnome::StockButton(button));
    p1->show();
    attach(*p1,c,c+1,r,r+1);

    c+=1;
    if (c==3)
      {
        c=0;
        r+=2;
      }
  }

/************************************************************/

class StockApp : public Gnome::App
  {
    public:
      StockApp();
      void toggle_sensitive();

    protected:

      //overrides:
      gint delete_event_impl(GdkEventAny*);

      //Member widgets:
      StockPixmapTable *pixmaps;
      StockButtonTable *buttons;
      Gtk::ToggleButton *button;
  };

StockApp::StockApp() : Gnome::App("StockDemo","StockDemo")
  {
    pixmaps=manage(new StockPixmapTable);

// stock icons
    pixmaps->add("New",GNOME_STOCK_PIXMAP_NEW,GNOME_STOCK_MENU_NEW);
    pixmaps->add("Open",GNOME_STOCK_PIXMAP_OPEN, GNOME_STOCK_MENU_OPEN);
    pixmaps->add("Close",GNOME_STOCK_PIXMAP_CLOSE,GNOME_STOCK_MENU_CLOSE);
    pixmaps->add("Revert",GNOME_STOCK_PIXMAP_REVERT, GNOME_STOCK_MENU_REVERT);
    pixmaps->add("Save",GNOME_STOCK_PIXMAP_SAVE, GNOME_STOCK_MENU_SAVE);
    pixmaps->add("Save As",GNOME_STOCK_PIXMAP_SAVE_AS, GNOME_STOCK_MENU_SAVE_AS);
    pixmaps->add("Cut",GNOME_STOCK_PIXMAP_CUT,GNOME_STOCK_MENU_CUT);
    pixmaps->add("Copy",GNOME_STOCK_PIXMAP_COPY,GNOME_STOCK_MENU_COPY);
    pixmaps->add("Paste",GNOME_STOCK_PIXMAP_PASTE, GNOME_STOCK_MENU_PASTE);
    pixmaps->add("Clear",GNOME_STOCK_PIXMAP_CLEAR);
    pixmaps->add("Properties",GNOME_STOCK_PIXMAP_PROPERTIES, GNOME_STOCK_PIXMAP_PROPERTIES);
    pixmaps->add("Preferences",GNOME_STOCK_PIXMAP_PREFERENCES, GNOME_STOCK_PIXMAP_PREFERENCES);
    pixmaps->add("Help",GNOME_STOCK_PIXMAP_HELP);
    pixmaps->add("Scores",GNOME_STOCK_PIXMAP_SCORES, GNOME_STOCK_MENU_SCORES);
    pixmaps->add("Print",GNOME_STOCK_PIXMAP_PRINT,GNOME_STOCK_MENU_PRINT);
    pixmaps->add("Search",GNOME_STOCK_PIXMAP_SEARCH,GNOME_STOCK_MENU_SEARCH);
    pixmaps->add("Search/Replace",GNOME_STOCK_PIXMAP_SRCHRPL,GNOME_STOCK_MENU_SRCHRPL);
    pixmaps->add("Back",GNOME_STOCK_PIXMAP_BACK,GNOME_STOCK_MENU_BACK);
    pixmaps->add("Forward",GNOME_STOCK_PIXMAP_FORWARD,GNOME_STOCK_MENU_FORWARD);
    pixmaps->add("First",GNOME_STOCK_PIXMAP_FIRST,GNOME_STOCK_MENU_FIRST);
    pixmaps->add("Last",GNOME_STOCK_PIXMAP_LAST,GNOME_STOCK_MENU_LAST);
    pixmaps->add("Home",GNOME_STOCK_PIXMAP_HOME,GNOME_STOCK_MENU_HOME);
    pixmaps->add("Refresh",GNOME_STOCK_PIXMAP_REFRESH,GNOME_STOCK_MENU_REFRESH);
    pixmaps->add("Undo",GNOME_STOCK_PIXMAP_UNDO,GNOME_STOCK_MENU_UNDO);
    pixmaps->add("Redo",GNOME_STOCK_PIXMAP_REDO,GNOME_STOCK_MENU_REDO);
    pixmaps->add("Timer",GNOME_STOCK_PIXMAP_TIMER,GNOME_STOCK_MENU_TIMER);
    pixmaps->add("Timer Stop",GNOME_STOCK_PIXMAP_TIMER_STOP,GNOME_STOCK_MENU_TIMER_STOP);
    pixmaps->add("Mail",GNOME_STOCK_PIXMAP_MAIL,GNOME_STOCK_MENU_MAIL);
    pixmaps->add("Receive Mail",GNOME_STOCK_PIXMAP_MAIL_RCV,GNOME_STOCK_MENU_MAIL_RCV);
    pixmaps->add("Send Mail",GNOME_STOCK_PIXMAP_MAIL_SND,GNOME_STOCK_MENU_MAIL_SND);
    pixmaps->add("Reply Mail",GNOME_STOCK_PIXMAP_MAIL_RPL,GNOME_STOCK_MENU_MAIL_RPL);
    pixmaps->add("Forward Mail",GNOME_STOCK_PIXMAP_MAIL_FWD,GNOME_STOCK_MENU_MAIL_FWD);
    pixmaps->add("New Mail",GNOME_STOCK_PIXMAP_MAIL_NEW,GNOME_STOCK_MENU_MAIL_NEW);
    pixmaps->add("Trash",GNOME_STOCK_PIXMAP_TRASH,GNOME_STOCK_MENU_TRASH);
    pixmaps->add("Trash Full",GNOME_STOCK_PIXMAP_TRASH_FULL,GNOME_STOCK_MENU_TRASH_FULL);
    pixmaps->add("Undelete",GNOME_STOCK_PIXMAP_UNDELETE,GNOME_STOCK_MENU_UNDELETE);
    pixmaps->add("Spellchecker",GNOME_STOCK_PIXMAP_SPELLCHECK);
    pixmaps->add("Microphone",GNOME_STOCK_PIXMAP_MIC,GNOME_STOCK_MENU_MIC);
    pixmaps->add("Line In",GNOME_STOCK_PIXMAP_LINE_IN,GNOME_STOCK_MENU_LINE_IN);
    pixmaps->add("Cdrom",GNOME_STOCK_PIXMAP_CDROM,GNOME_STOCK_MENU_CDROM);
    pixmaps->add("Volume",GNOME_STOCK_PIXMAP_VOLUME,GNOME_STOCK_MENU_VOLUME);
    pixmaps->add("Midi",GNOME_STOCK_PIXMAP_MIDI,GNOME_STOCK_MENU_MIDI);
    pixmaps->add("Book Red",GNOME_STOCK_PIXMAP_BOOK_RED,GNOME_STOCK_MENU_BOOK_RED);
    pixmaps->add("Book Green",GNOME_STOCK_PIXMAP_BOOK_GREEN,GNOME_STOCK_MENU_BOOK_GREEN);
    pixmaps->add("Book Blue",GNOME_STOCK_PIXMAP_BOOK_BLUE,GNOME_STOCK_MENU_BOOK_BLUE);
    pixmaps->add("Book Yellow",GNOME_STOCK_PIXMAP_BOOK_YELLOW,GNOME_STOCK_MENU_BOOK_YELLOW);
    pixmaps->add("Book Open",GNOME_STOCK_PIXMAP_BOOK_OPEN,GNOME_STOCK_MENU_BOOK_OPEN);
    pixmaps->add("About",GNOME_STOCK_PIXMAP_ABOUT,GNOME_STOCK_MENU_ABOUT);
    pixmaps->add("Quit",GNOME_STOCK_PIXMAP_QUIT,GNOME_STOCK_MENU_QUIT);
    pixmaps->add("Multiple",GNOME_STOCK_PIXMAP_MULTIPLE);
    pixmaps->add("Not",GNOME_STOCK_PIXMAP_NOT);
    pixmaps->add("Convert",GNOME_STOCK_PIXMAP_CONVERT,GNOME_STOCK_MENU_CONVERT);
    pixmaps->add("Jump To",GNOME_STOCK_PIXMAP_JUMP_TO,GNOME_STOCK_MENU_JUMP_TO);
    pixmaps->add("Up",GNOME_STOCK_PIXMAP_UP,GNOME_STOCK_MENU_UP);
    pixmaps->add("Down",GNOME_STOCK_PIXMAP_DOWN,GNOME_STOCK_MENU_DOWN);
    pixmaps->add("Top",GNOME_STOCK_PIXMAP_TOP,GNOME_STOCK_MENU_TOP);
    pixmaps->add("Bottom",GNOME_STOCK_PIXMAP_BOTTOM,GNOME_STOCK_MENU_BOTTOM);
    pixmaps->add("Attach",GNOME_STOCK_PIXMAP_ATTACH,GNOME_STOCK_MENU_ATTACH);
    pixmaps->add("Index",GNOME_STOCK_PIXMAP_INDEX,GNOME_STOCK_MENU_INDEX);
    pixmaps->add("Font",GNOME_STOCK_PIXMAP_FONT,GNOME_STOCK_MENU_FONT);
    pixmaps->add("Exec",GNOME_STOCK_PIXMAP_EXEC,GNOME_STOCK_MENU_EXEC);
    pixmaps->add("Left",GNOME_STOCK_PIXMAP_ALIGN_LEFT,GNOME_STOCK_MENU_ALIGN_LEFT);
    pixmaps->add("Right",GNOME_STOCK_PIXMAP_ALIGN_RIGHT,GNOME_STOCK_MENU_ALIGN_RIGHT);
        pixmaps->add("Center",GNOME_STOCK_PIXMAP_ALIGN_CENTER,GNOME_STOCK_MENU_ALIGN_CENTER);
    pixmaps->add("Justify",GNOME_STOCK_PIXMAP_ALIGN_JUSTIFY,GNOME_STOCK_MENU_ALIGN_JUSTIFY);
    pixmaps->add("Bold",GNOME_STOCK_PIXMAP_TEXT_BOLD,GNOME_STOCK_MENU_TEXT_BOLD);
    pixmaps->add("Italic",GNOME_STOCK_PIXMAP_TEXT_ITALIC,GNOME_STOCK_MENU_TEXT_ITALIC);
    pixmaps->add("Underline",GNOME_STOCK_PIXMAP_TEXT_UNDERLINE,GNOME_STOCK_MENU_TEXT_UNDERLINE);
    pixmaps->add("Strikeout",GNOME_STOCK_PIXMAP_TEXT_STRIKEOUT,GNOME_STOCK_MENU_TEXT_STRIKEOUT);
    pixmaps->add("Text Indent",GNOME_STOCK_PIXMAP_TEXT_INDENT);
    pixmaps->add("Text Unindent",GNOME_STOCK_PIXMAP_TEXT_UNINDENT);
    pixmaps->add("Color Selector",GNOME_STOCK_PIXMAP_COLORSELECTOR);
    pixmaps->add("Add Pixmap",GNOME_STOCK_PIXMAP_ADD);
    pixmaps->add("Remove Pixmap",GNOME_STOCK_PIXMAP_REMOVE);
    pixmaps->add("Table Borders",GNOME_STOCK_PIXMAP_TABLE_BORDERS);
    pixmaps->add("Table Fill",GNOME_STOCK_PIXMAP_TABLE_FILL);
    pixmaps->add("Text Bullet",GNOME_STOCK_PIXMAP_TEXT_BULLETED_LIST);
    pixmaps->add("Text Number",GNOME_STOCK_PIXMAP_TEXT_NUMBERED_LIST);


    buttons=manage( new StockButtonTable );
    buttons->add(GNOME_STOCK_BUTTON_OK);
    buttons->add(GNOME_STOCK_BUTTON_CANCEL);
    buttons->add(GNOME_STOCK_BUTTON_YES);
    buttons->add(GNOME_STOCK_BUTTON_NO);
    buttons->add(GNOME_STOCK_BUTTON_CLOSE);
    buttons->add(GNOME_STOCK_BUTTON_APPLY);
    buttons->add(GNOME_STOCK_BUTTON_HELP);
    buttons->add(GNOME_STOCK_BUTTON_NEXT);
    buttons->add(GNOME_STOCK_BUTTON_PREV);
    buttons->add(GNOME_STOCK_BUTTON_UP);
    buttons->add(GNOME_STOCK_BUTTON_DOWN);
    buttons->add(GNOME_STOCK_BUTTON_FONT);


    Gtk::Box *box;
    Gtk::Box *tools;
    Gtk::Frame *frame;
    Gtk::ScrolledWindow *area;

    box=manage( new Gtk::VBox );
    area=manage( new Gtk::ScrolledWindow );
    area->add_with_viewport(*pixmaps);
    area->show_all();
    area->set_usize(400,300);
    frame=manage( new Gtk::Frame("Pixmaps") );
    frame->add(*area);
    box->pack_start(*frame);

    frame=manage( new Gtk::Frame("Buttons") );
    area=manage( new Gtk::ScrolledWindow );
    area->add_with_viewport(*buttons);
    area->show_all();
    area->set_usize(400,150);
    frame->add(*area);
    box->pack_start(*frame);

    tools=manage( new Gtk::HBox );
    button=manage( new Gtk::ToggleButton("Sensitive"));
    button->set_active(true);
    button->toggled.connect(slot(*this,&StockApp::toggle_sensitive));

    Gtk::Button *quit=manage( new Gtk::Button("Quit") );
    quit->clicked.connect(Gtk::Kit::quit.slot());
    
    tools->pack_start(*button,false,false,10);
    tools->pack_end(*quit,false,false,10);
    tools->show_all();

    box->pack_start(*tools,false,false,20);
    box->show_all();

    set_contents(*box);
    show();
  }

void StockApp::toggle_sensitive()
  {
    pixmaps->set_active(button->get_active());
    buttons->set_active(button->get_active());
  }

gint StockApp::delete_event_impl(GdkEventAny*)
  { 
    Gtk::Main::quit();
    return FALSE; 
  }

int main (int argc, char* argv[])
{
    Gnome::Main kit("stock-demo", "0.0", argc, argv);
    StockApp app;
    kit.run();
    return 0;
  }

