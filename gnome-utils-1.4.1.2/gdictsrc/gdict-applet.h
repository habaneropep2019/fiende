#ifndef __GDICT_APPLET_H_
#define __GDICT_APPLET_H_

/* $Id: gdict-applet.h,v 1.4 2001/04/19 01:15:12 hovinen Exp $ */

/*
 *  Papadimitriou Spiros <spapadim@cs.cmu.edu>
 *
 *  This code released under the GNU GPL.
 *  Read the file COPYING for more information.
 *
 *  GDict panel applet
 *
 */

/* This structure contains the internal state of the gdict applet.
 * Everything the functions and signal handlers need to be able to
 * manipulate the applet should be contained within. */
typedef struct _GDictApplet {
	GtkWidget *applet_widget;
	GtkWidget *button_widget;
	GtkWidget *vbox_widget;
	GtkWidget *entry_widget;
	GtkWidget *handlebox_widget;
	gboolean handle;
} GDictApplet;

extern gboolean gdict_applet_toggle;

extern GDictApplet * gdict_applet_create (void);
extern void gdict_applet_delete (GDictApplet * applet);


#endif /* __GDICT_APPLET_H_ */
