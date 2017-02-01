/*
    test-libglade.c: a test program for the libglade library
    Copyright (C) 1998, 1999, 2000  James Henstridge <james@daa.com.au>
      GNOME option parsing code by Miguel de Icaza.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA


  At your option, you may use this alternative X style licence:

    Permission is hereby granted, free of charge, to any person obtaining
    a copy of this software and associated documentation files (the
    "Software"), to deal in the Software without restriction, including
    without limitation the rights to use, copy, modify, merge, publish,
    distribute, sublicense, and/or sell copies of the Software, and to
    permit persons to whom the Software is furnished to do so, subject to
    the following conditions:

    The above copyright notice and this permission notice shall be
    included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
    IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES
    OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
    OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
    THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/


#include <config.h>
#include <string.h>
#ifdef ENABLE_GNOME
#    include <gnome.h>
#else
#    include <gtk/gtk.h>
#endif
#include <glade/glade.h>

const char *filename = NULL, *rootnode = NULL;
gboolean no_connect = FALSE;

#ifdef ENABLE_GNOME
static poptContext ctx;

static const struct poptOption options [] = {
        { "no-connect", '\0', POPT_ARG_INT, &no_connect, 0,
          N_("Do not connect signals") },
        { NULL, '\0', 0, NULL, 0 }
};
#endif

int main (int argc, char **argv)
{
  int i;
  GladeXML *xml;

#ifdef ENABLE_GNOME
  const char **list = NULL;
  
  gnome_init_with_popt_table ("test-libglade", VERSION, argc, argv, options, 0, &ctx);
  glade_gnome_init();

  list = poptGetArgs (ctx);
  if (list){
	  for (i = 0; list [i]; i++){
		  if (filename == NULL)
			  filename = list [i];
		  else if (rootnode == NULL)
			  rootnode = list [i];
	  }
  }
  if (filename == NULL){
      g_print("Usage: %s [--no-connect] filename [rootnode]\n", argv[0]);
      return 1;
  }
#else
  gtk_init(&argc, &argv);
  glade_init();

  for (i = 1; i < argc; i++) {
    if (!strcmp(argv[i], "--no-connect"))
      no_connect = TRUE;
    else if (filename == NULL)
      filename = argv[i];
    else if (rootnode == NULL)
      rootnode = argv[i];
    else {
      g_print("Usage: %s [--no-connect] filename [rootnode]\n", argv[0]);
      return 1;
    }
  }
  if (filename == NULL) {
    g_print("Usage: %s [--no-connect] filename [rootnode]\n", argv[0]);
    return 1;
  }
#endif
  
  xml = glade_xml_new(filename, rootnode);

  if (!xml) {
    g_warning("something bad happened while creating the interface");
    return 1;
  }

  if (rootnode) {
    GtkWidget *wid = glade_xml_get_widget(xml, rootnode);
    if (!GTK_IS_WINDOW(wid)) {
      GtkWidget *win, *frame;

      win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
      gtk_signal_connect(GTK_OBJECT(win), "destroy",
			 GTK_SIGNAL_FUNC(gtk_main_quit), NULL);
      gtk_container_set_border_width(GTK_CONTAINER(win), 5);
      frame = gtk_frame_new(NULL);
      gtk_container_set_border_width(GTK_CONTAINER(frame), 5);
      gtk_container_add(GTK_CONTAINER(win), frame);
      gtk_widget_show(frame);
      gtk_container_add(GTK_CONTAINER(frame), wid);
      gtk_widget_show(win);
    }
  }

  if (!no_connect)
    glade_xml_signal_autoconnect(xml);

  gtk_object_unref(GTK_OBJECT(xml));

  gtk_main();

  return 0;
}
