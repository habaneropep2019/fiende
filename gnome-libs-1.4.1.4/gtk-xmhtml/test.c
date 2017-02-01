/* These are defined by the Makefile for gtk-xmhtml */
#undef LIBRARY
#undef _LIBRARY

#include <string.h> 
#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <X11/Xatom.h>		/* property defines */
#include <gtk-xmhtml/gtk-xmhtml.h>

/* Prototypes */

void click (GtkWidget *widget, gpointer data);
void frame (GtkWidget *widget, gpointer data);


char *urls [] = {
	"unknown", "named (...)", "jump (#...)",
	"file_local (file.html)", "file_remote (file://foo.bar/file)",
	"ftp", "http", "gopher", "wais", "news", "telnet", "mailto",
	"exec:foo_bar", "internal"
};

char *test_string2 =
"<html>\n"
"<head><title>The Gtk/XmHTML test</title></head>\n"
"This is the Gtk/XmHTML test program<p>\n"
"You can invoke this program with a command line argument, like this:\n"
"<hr>"
"<tt>./xtest filename.html</tt>"
"<hr>"
"Click here to load a different <a href=\"nothing\">test message</a>"
"</html>";

char *test_string =
"<html><head><title>h</title></head>"
"<body>Item: %s<p>Frame: %s<p>"
"We want all the people in the world to use free software, because"
"free software is a very nice way of sharing code and learning new"
"things you had never thought of before"
"</body>"
"</html>";

char *test_string3 =
"<html><head><title>h</title></head>"
"<body>I love you world"
"</body>"
"</html>";

void
click (GtkWidget *widget, gpointer data)
{
	XmHTMLAnchorCallbackStruct *cbs = (XmHTMLAnchorCallbackStruct *) data;
	
	printf ("click!\n");
	printf ("URLtype: %s\n", urls [cbs->url_type]);
	printf ("line:    %d\n", cbs->line);
	printf ("href:    %s\n", cbs->href);
	printf ("target:  %s\n", cbs->target);
	printf ("rel:     %s\n", cbs->rel);
	printf ("rev:     %s\n", cbs->rev);
	printf ("title:   %s\n", cbs->title);
	printf ("doit:    %d\n", cbs->doit);
	printf ("visited: %d\n", cbs->visited);
	gtk_xmhtml_source (GTK_XMHTML (widget), test_string3);
}

void
frame (GtkWidget *widget, gpointer data)
{
	XmHTMLFrameCallbackStruct *cbs = (void *) data;

	printf ("Frame callback: ");
	if (cbs->reason == XmCR_HTML_FRAME){
		char buffer [1024];
		GtkXmHTML *html = GTK_XMHTML (cbs->html);

		sprintf (buffer, test_string, cbs->src, cbs->name);
		printf ("frame: %s\n", buffer);
		gtk_xmhtml_source (html, buffer);
		return;
	}

	if (cbs->reason == XmCR_HTML_FRAMECREATE){
		printf ("create\n");
		return;
	}
	if (cbs->reason == XmCR_HTML_FRAMEDESTROY){
		printf ("destroy\n");
		return;
	}
}

int
main (int argc, char *argv [])
{
	GtkWidget *window, *html;
	GString *file_contents;
	char aline[1024];
	FILE *afile = NULL;

	gtk_set_locale();
	gtk_init (&argc, &argv);
	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

	file_contents = g_string_new(NULL);
	if(argc == 2) {
		afile = fopen(argv[1], "rt");
		if(afile != NULL) {
		  while(fgets(aline, sizeof(aline), afile))
			file_contents = g_string_append(file_contents, aline);
		  fclose(afile);
		}
	}
	if(strlen(file_contents->str) <= 0)
		file_contents = g_string_append(file_contents, test_string2);
	gtk_signal_connect(GTK_OBJECT(window), "delete_event",
		GTK_SIGNAL_FUNC(gtk_true), NULL);
	gtk_signal_connect(GTK_OBJECT(window), "destroy",
		GTK_SIGNAL_FUNC(gtk_main_quit), NULL);

	html = gtk_xmhtml_new ();
	gtk_xmhtml_source (GTK_XMHTML (html), file_contents->str);
	gtk_container_add (GTK_CONTAINER (window), html);
	gtk_widget_show (html);

	gtk_signal_connect (GTK_OBJECT(html), "activate", (GtkSignalFunc) click, html);
	gtk_signal_connect (GTK_OBJECT(html), "frame", (GtkSignalFunc) frame, html);

	gtk_widget_show (window);
	

	gtk_main ();
	return 0;
}
