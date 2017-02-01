/*
 *  Gnome Character Map
 *  main.c: what do you think this file is?
 *
 *  Copyright (C) 2000 Hongli Lai
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef _MAIN_C_
#define _MAIN_C_

#include <config.h>
#include <gnome.h>
#include "interface.h"

int
main (int argc, char *argv[])
{
    bindtextdomain (PACKAGE, GNOMELOCALEDIR);
    textdomain (PACKAGE);

    gnome_init ("gcharmap", VERSION, argc, argv);

    gtk_widget_show (GTK_WIDGET (main_app_new ()->window));
    gtk_main ();
    return 0;
}

#endif /* _MAIN_C_ */
