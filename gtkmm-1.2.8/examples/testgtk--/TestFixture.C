/*
** ===========================================================================
** $RCSfile: TestFixture.C,v $
** $Revision: 1.4 $
** $Date: 1999/10/19 09:30:42 $
** $Author: kenelson $
** ===========================================================================
*/

#include "TestFixture.h"

static char * pc_rcs_h = TestFixture_h;
static char * pc_rcs = "$Id: TestFixture.C,v 1.4 1999/10/19 09:30:42 kenelson Exp $";

#define USE(var) static void * use_##var = (void *) var
USE( pc_rcs_h);
USE( pc_rcs);

TestFixture::TestFixture () :
  vbox ( false, 0 ),
  actionArea ( false, 10 ),
  controlArea ( false, 10 ), 
  closeButton ( "close" )
{
  add ( vbox );
  vbox . set_border_width ( 0 );
  vbox . pack_start ( actionArea );
  actionArea . set_border_width ( 10 );
  destroy.connect(bind(finished.slot(),this));
 
};

TestFixture::~TestFixture () 
{
}

void
TestFixture::packControlArea () 
{
  vbox . pack_start ( separator, false, true, 0 );
  controlArea . set_border_width ( 10 );
  vbox . pack_start ( controlArea, false, true, 0 );
  controlArea . show ();
  closeButton.clicked.connect(bind(finished.slot(),this));

  controlArea . pack_start ( closeButton, true, true, 0 );
  closeButton . set_flags ( GTK_CAN_DEFAULT );
  closeButton . grab_default ();
  closeButton . show ();
};
