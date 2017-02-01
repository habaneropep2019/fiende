/* $Id: gtk--.h,v 1.18 2000/01/10 15:02:59 kenelson Exp $ */
/* GTK-- - a C++ wrapper for the Gtk toolkit
 *
 * Copyright (C) 1997 Elliot Lee <sopwith@redhat.com>      
 *                    Phil Dawes
 *                    Tero Pulkkinen
 *                    Guillaume Laurent
 *                  
 * Currently maintained by Tero Pulkkinen. <terop@modeemi.cs.tut.fi>
 *                                                        
 * New Callback stuff written by Phil Dawes (1997)        
 *  using ideas from code by Tero Pulkkinen.
 * 
 * REMEMBER: class_init fixed for C objects (make parser for this)
 *           get_data removed for optimization
 *           virtual functions to nonvirtual where virtual not needed(ok)
 *           check if forgot some signals from widgets
 *           check if friends are needed still(ok)
 *           Gtk_Main to thing that has no instances (hmm, ok)
 * 
 * 20 Aug 1997 Additions to the new callback stuffs by Tero Pulkkinen
 * 20 Aug 1997 Added border_width function to Gtk_Container -class.
 * 20 Aug 1997 Fixed Slot1's to SLOTNAME from Phil's code :)
 * 
 * 22 Aug 1997 Added cbdata support
 * 22 Aug 1997 Added Deletion support
 * 10 Oct 1997 Added proper static typing for builtin widgets
 * 17 Nov 1997 Applied patch from Guillaume Laurent about widget flags.
 * 19 Nov 1997 I messed up the source :) Now it requires egcs to compile :)
 *             How are we going to fix that? (tp)
 * 28 Nov 1997 Return types of signals implemented and now works
 * 29 Nov 1997 Deriving from C-language implementation class starts
 *             to look implementable now. Also now it works properly
 *             with gcc2.7.2 again.
 * 13 Dec 1997 Removed alot of virtual words from the library!
 *             It'd be cheating people to let them think they can override
 *             those methods! The C version calls directly the C functions
 *             and cannot be overriden. Thanks to nether a.k.a Lauri Alanko
 *             for pointing this out.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#define GTK_0_99_4   /* yes we share the future */

#ifndef GTK_MINUS_MINUS_H
#define GTK_MINUS_MINUS_H 1
extern "C" {
#include <gtk/gtk.h>
}

/* Gtkmm version.  *  */
extern const guint gtkmm_major_version;
extern const guint gtkmm_minor_version;
extern const guint gtkmm_micro_version;

#include <assert.h>
#include <gtk--/base.h>
#include <gtk--/proxy.h>

#include <gtk--/object.h>
#include <gtk--/accelgroup.h>
#include <gtk--/adjustment.h>
#include <gtk--/alignment.h>
#include <gtk--/arrow.h>
#include <gtk--/aspectframe.h>
#include <gtk--/base.h>
#include <gtk--/bin.h>
#include <gtk--/box.h>
#include <gtk--/button.h>
#include <gtk--/buttonbox.h>
#include <gtk--/checkbutton.h>
#include <gtk--/checkmenuitem.h>
#include <gtk--/clist.h>
#include <gtk--/colorselection.h>
#include <gtk--/combo.h>
#include <gtk--/container.h>
#include <gtk--/curve.h>
#include <gtk--/data.h>
#include <gtk--/dialog.h>
#include <gtk--/drawingarea.h>
#include <gtk--/editable.h>
#include <gtk--/entry.h>
#include <gtk--/eventbox.h>
#include <gtk--/fileselection.h>
#include <gtk--/fixed.h>
#include <gtk--/frame.h>
//#include <gtk--/rc.h>
#include <gtk--/handlebox.h>
#include <gtk--/image.h>
#include <gtk--/inputdialog.h>
#include <gtk--/item.h>
#include <gtk--/calendar.h>
#include <gtk--/ctree.h>
#include <gtk--/invisible.h>
#include <gtk--/label.h>
#include <gtk--/list.h>
#include <gtk--/listitem.h>
#include <gtk--/main.h>
#include <gtk--/menu.h>
#include <gtk--/menubar.h>
#include <gtk--/menuitem.h>
#include <gtk--/menushell.h>
#include <gtk--/misc.h>
#include <gtk--/notebook.h>
#include <gtk--/object.h>
#include <gtk--/optionmenu.h>
#include <gtk--/paned.h>
#include <gtk--/pixmap.h>
#include <gtk--/preview.h>
#include <gtk--/progressbar.h>
#include <gtk--/radiobutton.h>
#include <gtk--/radiomenuitem.h>
#include <gtk--/range.h>
#include <gtk--/ruler.h>
#include <gtk--/scale.h>
#include <gtk--/scrollbar.h>
#include <gtk--/scrolledwindow.h>
#include <gtk--/separator.h>
#include <gtk--/spinbutton.h>
#include <gtk--/statusbar.h>
#include <gtk--/style.h>
#include <gtk--/table.h>
#include <gtk--/text.h>
#include <gtk--/tipsquery.h>
#include <gtk--/togglebutton.h>
#include <gtk--/toolbar.h>
#include <gtk--/tooltips.h>
#include <gtk--/tree.h>
#include <gtk--/treeitem.h>
#include <gtk--/viewport.h>
#include <gtk--/widget.h>
#include <gtk--/window.h>

#endif /* #ifndef GTK_MINUS_MINUS_H */
