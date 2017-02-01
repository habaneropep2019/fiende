/*   Gnome Help Window - Michael Fulbright <msf@redhat.com>
 *   A help widget based on a help widget from:
 *
 *   GTimeTracker - a time tracker
 *   Copyright (C) 1997,98 Eckehard Berns
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <config.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/wait.h>

#ifdef HAVE_ALLOCA_H
#include <alloca.h>
#endif

#include <glib.h>
#include <gnome.h>
#include "gnome-helpwin.h"

#ifdef HELP_USE_GTKHTML
static GtkHTMLClass *parent_class;
#else
static GtkXmHTMLClass *parent_class;
#endif

/****************/
/* widget stuff */
/****************/

#if 0
static void
gnome_helpwin_destroy(GtkObject *object)
{
	GnomeHelpWin *help;
	
	g_return_if_fail (object != NULL);
	g_return_if_fail (GNOME_HELPWIN_IS_HELP (object));
	
	help = GNOME_HELPWIN (object);
	
	if (help->html_source)
		g_free(help->html_source);
	
	/* probably want to turn this on at some point */
/* 	if (GTK_OBJECT_CLASS(parent_class)->destroy)
 		(* GTK_OBJECT_CLASS(parent_class)->destroy)(GTK_OBJECT(help));
*/		
}
#endif

static void
gnome_helpwin_class_init(GnomeHelpWinClass *helpclass)
{
    /* may need to enable this */
/*	GtkObjectClass *object_class = GTK_OBJECT_CLASS(helpclass);
	object_class->destroy = gnome_helpwin_destroy;
*/

	/* ugly C magic, doubt its required */
#ifdef HELP_USE_GTKHTML
	parent_class = gtk_type_class (GTK_TYPE_HTML);
#else
	parent_class = gtk_type_class (gtk_xmhtml_get_type ()); 
#endif
}

static void
gnome_helpwin_init(GnomeHelpWin *help)
{
	/* init struct _GnomeHelpWin */
#ifdef HELP_USE_GTKHTML
	gtk_html_construct (GTK_WIDGET (help));
	gtk_html_load_empty (GTK_HTML (help));
#endif
	help->document_path[0] = 0;
	help->html_source = NULL;
}

guint
gnome_helpwin_get_type(void)
{
	static guint GnomeHelpWin_type = 0;
	if (!GnomeHelpWin_type) {
		GtkTypeInfo GnomeHelpWin_info = {
			"GnomeHelpWin",
			sizeof(GnomeHelpWin),
			sizeof(GnomeHelpWinClass),
			(GtkClassInitFunc) gnome_helpwin_class_init,
			(GtkObjectInitFunc) gnome_helpwin_init,
			NULL,
			NULL,
			(GtkClassInitFunc) NULL
		};

#ifdef HELP_USE_GTKHTML
		GnomeHelpWin_type = gtk_type_unique(GTK_TYPE_HTML,
#else
		GnomeHelpWin_type = gtk_type_unique(gtk_xmhtml_get_type(),
#endif
						    &GnomeHelpWin_info);
	}
	return GnomeHelpWin_type;
}

GtkWidget *
gnome_helpwin_new(void)
{
	return GTK_WIDGET(gtk_type_new(gnome_helpwin_get_type()));
}




guint
gnome_helpwin_close(GnomeHelpWin *w)
{
        gtk_widget_hide(GTK_WIDGET(w));
        return FALSE;
}

void
gnome_helpwin_jump_to_anchor(GnomeHelpWin *w, gchar *anchor)
{
	gchar *a;
#ifdef HELP_USE_GTKHTML
	g_warning ("jumping...");
#else
	g_return_if_fail( w != NULL );
	g_return_if_fail( anchor != NULL );

	if (*anchor != '#') {
		a = alloca(strlen(anchor)+5);
		strcpy(a, "#");
		strcat(a, anchor);
	} else {
		a = anchor;
	}
		
        XmHTMLAnchorScrollToName(GTK_WIDGET(w), a);
#endif
}

void
gnome_helpwin_jump_to_line(GnomeHelpWin *w, gint line)
{
#ifdef HELP_USE_GTKHTML
	g_warning ("jumping to line...");
#else
	g_return_if_fail( w != NULL );

	/*        XmHTMLTextScrollToLine(GTK_WIDGET(w), line); */
	gtk_xmhtml_set_topline(GTK_XMHTML(w), line);
#endif
}

gint
gnome_helpwin_get_line(GnomeHelpWin *w)
{
#ifdef HELP_USE_GTKHTML
	return 0;
#else
        return gtk_xmhtml_get_topline(GTK_XMHTML(w));
#endif
}

#ifdef HELP_USE_GTKHTML
void
gtk_html_source (GtkHTML *html, char *url, char *source)
{
	GtkHTMLStream *s;

	/* we need to set writing to TRUE so that the url_requested
	 * callback won't try to load it, since we already have the
	 * source in source, and url_requested doesn't know how to
	 * handle ghelp: or toc: or the other funky "protocols".
	 */
	GNOME_HELPWIN (html)->writing = TRUE;
	s = gtk_html_begin (html);
	gtk_html_write (html, s, source, strlen (source));

	GNOME_HELPWIN (html)->writing = FALSE;
	/*gtk_html_end (html, handle, GTK_HTML_STREAM_OK);*/
}
#endif /* HELP_USE_GTKHTML */

/*
 * gnome_helpwin_goto()
 *
 * Loads contents of file 'filename' in the help widget object 'help'
 *
 * NOTE - fairly obsolete, left around for reference mostly
 *        wouldn't recommend using it
 *      
 *        Still figuring out what goes into the widget and
 *        what needs to be external routines (like filtering) 
 *
 */
void
gnome_helpwin_goto(GnomeHelpWin *help, const char *filename)
{
	FILE *f;
	char *tmp, s[BUFSIZ], *anchor, *path;
	GString *str;

	g_return_if_fail(help != NULL);
	g_return_if_fail(GNOME_HELPWIN_IS_HELP(help));
	g_return_if_fail(filename != NULL);

	path = help->document_path;
	
	/* TODO: parse filename for '..' */
	if (filename[0] == '/') {
		strcpy(path, filename);
	} else if (strrchr(path, '/')) {
		strcpy(strrchr(path, '/') + 1, filename);
	} else {
		strcpy(path, filename);
	}
	/* check for '#' */
	anchor = NULL;
	if (NULL != (tmp = strrchr(path, '#'))) {
		anchor = alloca(strlen(tmp)+2);
		strcpy(anchor, tmp);
		*tmp = '\0';
	}

#ifndef HELP_USE_GTKHTML
        /* TODO: jump to a "#anchor" in the same document should work */
	if ((path[strlen(path) - 1] == '/') ||
	    (path[0] == 0)) {
		if (!help->html_source)
		    return;

		/* just jump to the anchor */
		gnome_helpwin_jump_to_anchor( help, anchor);
		return;
	}
#endif

	errno = 0;
	f = fopen(path, "rt");
	if ((errno) || (!f)) {
		if (f) fclose(f);

		if (help->html_source)
		    g_free(help->html_source);

		help->html_source= g_strdup(_("<body><h2>Error: file not "
					    "found</h2></body>"));
#ifdef HELP_USE_GTKHTML
		gtk_html_source (GTK_HTML (help), filename, help->html_source);
#else
		gtk_xmhtml_source( GTK_XMHTML(help), help->html_source );
#endif
		return;
	}

	str = g_string_new (NULL);
	while ((!feof(f)) && (!errno)) {
		if (!fgets(s, sizeof (s)-1, f))
		    continue;
		g_string_append (str, s);
	}
	fclose(f);
	if (errno) {
		g_warning("gnome_helpwin_goto: error reading file "
			  "\"%s\".\n", path);
		errno = 0;
	}

#ifdef HELP_USE_GTKHTML
	gtk_html_source (GTK_HTML (help), path, str->str);
#else
	gtk_xmhtml_source( GTK_XMHTML(help), str->str);
	gnome_helpwin_jump_to_anchor( help, anchor);
#endif
	if (help->html_source)
	    g_free(help->html_source);

	help->html_source = str->str;
	g_string_free (str, FALSE);

}
