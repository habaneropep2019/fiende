/*
** ===========================================================================
** $RCSfile: TooltipsTest.h,v $
** $Revision: 1.4 $
** $Date: 2000/03/07 14:21:41 $
** $Author: kenelson $
** ===========================================================================
*/

/*
** Gtk::Button
*/

#include "TestFixture.h"

#ifndef TooltipsTest_h
#define TooltipsTest_h "$Id: TooltipsTest.h,v 1.4 2000/03/07 14:21:41 kenelson Exp $"


class TooltipsTest : public TestFixture 
{
public:
  static TestFixture *  create ();
  virtual              ~TooltipsTest () { };
  virtual void          destroyTest ();
private:
                        TooltipsTest ();
  // functions
  void                  toggleShowButton ( int buttonIndex );
  // data
  static TooltipsTest * theTest;

  Gtk::Tooltips     tooltips;
  Gtk::VBox         vbox1;
  Gtk::VBox         vbox2;
  Gtk::ToggleButton toggleButton1;
  Gtk::ToggleButton toggleButton2;
  Gtk::ToggleButton toggleButton3;
  Gtk::VBox         vbox3;
  Gtk::TipsQuery    tipsQuery;
  Gtk::Button       button;
  Gtk::Frame        frame;

  void widget_entered ( Gtk::Widget      *widget,
			const gchar     *tip_text,
			const gchar     *tip_private );

  gint widget_selected ( Gtk::Widget      *widget,
			 const gchar    *tip_text,
			 const gchar    *tip_private,
			 GdkEventButton *event );

};

#endif
