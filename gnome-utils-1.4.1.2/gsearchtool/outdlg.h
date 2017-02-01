/* Gnome Search Tool 
 * (C) 1998 the Free Software Foundation
 *
 * Author:   George Lebl
 */

#ifndef _OUTDLG_H_
#define _OUTDLG_H_

#include <gtk/gtk.h>

#include "gsearchtool.h"


gboolean outdlg_makedlg(char name[], gboolean clear);
void outdlg_additem(char item[]);
void outdlg_showdlg(void);
void outdlg_freeze(void);
void outdlg_thaw(void);

#endif
