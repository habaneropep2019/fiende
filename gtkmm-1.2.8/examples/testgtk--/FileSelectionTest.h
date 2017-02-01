#include "TestFixture.h"

/*
 * GtkFileSelection
 */

class FileSelectionTest : public TestFixture 
{
public:

  static TestFixture * create ();
  virtual             ~FileSelectionTest () { fileSelection.hide(); };
  virtual void         destroyTest ();
private:
                       FileSelectionTest ();
  void                 hide_fileops ();
  void                 show_fileops ();
  void                 ok ();
  //  void                 finished () { TestFixture::finished ( this ); };

  // data
  static FileSelectionTest * theTest;
  Gtk::FileSelection          fileSelection;

};
