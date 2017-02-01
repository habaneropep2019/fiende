#ifndef __MOTIFP_H__
#define __MOTIFP_H__

#include <Xm/XmP.h>		/* index defines */
#include <Xm/ManagerP.h>
#include <X11/IntrinsicP.h>	/* fast macros */
#include <X11/Xmu/Atoms.h>	/* must be below, it includes X11/Intrinsic.h */

/*****
* Class pointer and extension record definition
*****/
typedef struct {
  XtPointer		extension;	/* Pointer to extension record */
}XmHTMLClassPart;

typedef struct _XmHTMLClassRec
{
	CoreClassPart		core_class;
	CompositeClassPart	composite_class;
	ConstraintClassPart	constraint_class;
	XmManagerClassPart	manager_class;
	XmHTMLClassPart		html_class;
}XmHTMLClassRec;

#endif
