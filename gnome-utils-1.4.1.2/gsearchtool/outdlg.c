/* Gnome Search Tool 
 * (C) 1998,2000 the Free Software Foundation
 *
 * Author:   George Lebl
 *
 */

#include "config.h"
#include <gnome.h>
#include <gdk/gdkprivate.h>
#include <stdio.h>
#include <string.h>

#include "gsearchtool.h"
#include "outdlg.h"

static GtkWidget *outdlg = NULL;
static GtkWidget *outlist;
static int list_width = 0;

extern GtkWidget *app;

static char *filename = NULL;

static gboolean
save_file(char *fname)
{
	FILE *fp;
	int i;
	fp = fopen(fname, "w");
	if(!fp) return FALSE;
	for(i=0; i<GTK_CLIST(outlist)->rows; i++) {
		char *text = "";
		gtk_clist_get_text(GTK_CLIST(outlist), i, 0, &text);
		fprintf(fp, "%s\n", text);
	}
	fclose(fp);
	return TRUE;
}

static void
save_ok(GtkWidget *widget, GtkFileSelection *fsel)
{
	char *fname;
	g_return_if_fail(GTK_IS_FILE_SELECTION(fsel));

	fname = gtk_file_selection_get_filename(fsel);
	if(!fname ||
	   !save_file(fname)) {
		GtkWidget *dlg;
		dlg = gnome_error_dialog_parented(_("Cannot save file"),
						  GTK_WINDOW(fsel));
		gtk_window_set_modal(GTK_WINDOW(dlg), TRUE);
	} else {
		g_free(filename);
		filename = g_strdup(fname);
		gtk_widget_destroy(GTK_WIDGET(fsel));
	}
}



static void
outdlg_clicked(GtkWidget * widget, int button, gpointer data)
{
	GtkFileSelection *fsel;
	switch(button) {
	case 0:
		fsel = GTK_FILE_SELECTION(gtk_file_selection_new(_("Save Results")));
		if(filename)
			gtk_file_selection_set_filename(fsel, filename);

		gtk_signal_connect (GTK_OBJECT (fsel->ok_button), "clicked",
				    GTK_SIGNAL_FUNC (save_ok), fsel);
		gtk_signal_connect_object
			(GTK_OBJECT (fsel->cancel_button), "clicked",
			 GTK_SIGNAL_FUNC (gtk_widget_destroy), 
			 GTK_OBJECT(fsel));

		gtk_window_position (GTK_WINDOW (fsel), GTK_WIN_POS_MOUSE);

		gtk_window_set_transient_for(GTK_WINDOW(fsel),
					     GTK_WINDOW(outdlg));

		gtk_signal_connect_object_while_alive(GTK_OBJECT(widget),
						      "destroy",
						      GTK_SIGNAL_FUNC(gtk_widget_destroy),
						      GTK_OBJECT(fsel));

		gtk_widget_show (GTK_WIDGET (fsel));

		break;
	case 1:
		gtk_clist_clear(GTK_CLIST(outlist));
		list_width = 0;
		break;
	case 2:
	default:
		gtk_widget_destroy(outdlg);
		list_width = 0;
		break;
	}
}

static gint 
outdlg_double_click(GtkWidget *widget, GdkEventButton *event,
		    gpointer func_data)
{
	GtkCList *clist=(GtkCList *)widget;
	gint row, col;
	gchar *fileName;
	const gchar *program;
	const gchar *mimeType;

	if (event->type==GDK_2BUTTON_PRESS) {
		if (!gtk_clist_get_selection_info(clist, event->x, event->y, &row, &col))
			return FALSE;
		
		gtk_clist_get_text(clist, row, col, &fileName);
		mimeType=gnome_mime_type_of_file(fileName);
		program=gnome_mime_program(mimeType);
		
		if (program) {
			char **argv;
			int argc, i;
			/* we use a popt function as it does exactly
			   what we want to do and
			   gnome already uses popt */
			if(poptParseArgvString(program, &argc, &argv) != 0) {
				gnome_error_dialog_parented(_("Command mangled"),
							    GTK_WINDOW(outdlg));
				return FALSE;
			}

			for (i = 0; i<argc; i++) {
				if (strcmp(argv[i], "%f") == 0 ||
				    strcmp(argv[i], "%s") == 0) {
					argv[i]=fileName;
					break;
				}
			}

			if (gnome_mime_needsterminal(mimeType, NULL)) {
				char **bigargv;
				bigargv = g_new0(char *, argc+3);
				bigargv[0] = "gnome-terminal";
				bigargv[1] = "-x";
				for(i=2; i<argc+2; i++)
					bigargv[i] = argv[i-2];
				bigargv[i] = NULL;

				gnome_execute_async(NULL, argc+2, bigargv);
				g_free(bigargv);
			} else {
				gnome_execute_async(NULL, argc, argv);
			}
			free(argv);
		} else {
			gnome_error_dialog_parented(_("No command for this mime type"),
						    GTK_WINDOW(outdlg));
			return FALSE;
		}
		return TRUE;
	}
	return FALSE;
}

static void
outdlg_closedlg(GtkWidget * widget, gpointer data)
{
	list_width = 0;
	outdlg = NULL;
}

gboolean
outdlg_makedlg(char name[], gboolean clear)
{
	GtkWidget *w;

	/*we already have a dialog!!!*/
	if(outdlg!=NULL) {
		if(clear) {
			list_width = 0;
			gtk_clist_clear(GTK_CLIST(outlist));
		}
		return FALSE;
	}
	list_width = 0;

	outdlg = gnome_dialog_new(name,
				  "Save",
				  "Clear",
				  GNOME_STOCK_BUTTON_CLOSE,
				  NULL);
	gtk_window_set_policy(GTK_WINDOW(outdlg), FALSE, TRUE, FALSE);
	gtk_window_set_default_size(GTK_WINDOW(outdlg), 200, 400);
	gnome_dialog_set_parent(GNOME_DIALOG(outdlg), GTK_WINDOW(app));
	gtk_signal_connect(GTK_OBJECT(outdlg), "destroy",
			   GTK_SIGNAL_FUNC(outdlg_closedlg), NULL);

	gtk_signal_connect(GTK_OBJECT(outdlg), "clicked",
			   GTK_SIGNAL_FUNC(outdlg_clicked), NULL);

	w = gtk_scrolled_window_new(NULL,NULL);
	/*gtk_widget_set_usize(w,200,350);*/
	gtk_box_pack_start(GTK_BOX(GNOME_DIALOG(outdlg)->vbox),w,TRUE,TRUE,0);
	outlist = gtk_clist_new(1);
	gtk_signal_connect(GTK_OBJECT(outlist), "button_press_event",
				GTK_SIGNAL_FUNC(outdlg_double_click),NULL);
	gtk_container_add(GTK_CONTAINER(w),outlist);

	gtk_widget_ensure_style(outlist);

	return TRUE;
}

void
outdlg_additem(char item[])
{
	int width;
	
	if(!outdlg)
		return;

	gtk_clist_append(GTK_CLIST(outlist), &item);

	width = gdk_string_width(GTK_WIDGET(outlist)->style->font, item);
	if(list_width < width) {
		list_width = width;
		gtk_clist_set_column_width(GTK_CLIST(outlist), 0, list_width);
	}
}

void
outdlg_freeze(void)
{
	gtk_clist_freeze(GTK_CLIST(outlist));
}

void
outdlg_thaw(void)
{
	gtk_clist_thaw(GTK_CLIST(outlist));
}

void
outdlg_showdlg(void)
{
	gtk_widget_show_all(outdlg);
}

