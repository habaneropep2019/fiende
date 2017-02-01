/*
** ===========================================================================
** $RCSfile: CListTest.h,v $
** $Revision: 1.4 $
** $Date: 2000/03/07 14:21:41 $
** $Author: kenelson $
** ===========================================================================
*/

/*
** Gtk::Button
*/

#include "TestFixture.h"

#ifndef CListTest_h
#define CListTest_h "$Id: CListTest.h,v 1.4 2000/03/07 14:21:41 kenelson Exp $"


class CListTest : public TestFixture 
{
public:
  static TestFixture *  create ();
  virtual              ~CListTest () { };
  virtual void          destroyTest ();
private:
                        CListTest ();
  // functions
  void add1000 ();
  void add10000 ();
  void clear ();
  void remove_focus_row ();
  void show_titles ();
  void hide_titles ();
  void select ( gint row_in, gint column_in, GdkEventButton * event_in,  GtkWidget * button_in );
  void insert_row ();
  void remove_selection ();
  void warning_test ();
  void undo_selection ();
  void toggle_sel_mode ();
  void toggle_title_buttons () {}
  void toggle_reorderable () {}
  

  void click_column ( gint column );

  Gtk::OptionMenu * build_option_menu ();
  // data
  static CListTest * theTest;

  Gtk::CList      *_clist;
  gint            _clist_rows;
  Gtk::OptionMenu *_clist_omenu;
  Gtk::Style      *_style1;
  Gtk::Style      *_style2;
  Gtk::Style      *_style3;
};


#endif

