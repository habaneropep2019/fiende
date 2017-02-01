#ifdef JUNK

#include "TestFixture.h"

/*
 * Saved Position
 */
gint upositionx = 0;
gint upositiony = 0;

class SavedPosition : public TestFixture 
{
public:
  static TestFixture * create ();
  virtual             ~SavedPosition () {} ;
  virtual void         destroyTest ();
private:
  static SavedPosition * theTest;
  static gint            upositionx;
  static gint            upositiony;

                         SavedPosition ();
  bool                   upositionConfigure ();

};
#endif
