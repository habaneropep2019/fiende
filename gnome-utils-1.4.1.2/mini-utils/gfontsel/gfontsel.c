/* Gnome Font Selector Tool
 * (C) 1998 the Free Software Foundation
 *
 * This is an xfontsel replacement.
 *
 * Uses the GTK font selection widget, so this is nice and simple.
 *
 * Author: Andrew Veliath */

#include <config.h>
#include <gnome.h>
#include <libgnomeui/gnome-window-icon.h>
#include <gdk/gdkkeysyms.h>

#include <stdlib.h>
#include <string.h>
#include <signal.h>

#define CONFIG_PREFIX		"/gfontsel"
#define CONFIG_PROPERTIES	CONFIG_PREFIX "/Properties"
#define CONFIG_LASTFONT		CONFIG_PROPERTIES "/LastFont"

typedef struct {
	GtkWidget *dialog;
	GtkWidget *action_area;
	GtkWidget *fontsel;
	GtkWidget *combo;
	GtkWidget *entry;
	gchar *prev_fontname;

	gchar *font_load;
	gboolean print_on_exit;
	gboolean remember_font;
	gboolean load_last;
} gfontsel_cfg_t;

static void
copy_and_activate_cb (GtkWidget * widget, gpointer user_data)
{
	gfontsel_cfg_t *cfg = (gfontsel_cfg_t *) user_data;
	GtkWidget *list = GTK_COMBO (cfg->combo)->list;
	gchar *fontname;

	fontname = gtk_font_selection_get_font_name (
		GTK_FONT_SELECTION (cfg->fontsel));

	if (fontname && *fontname) {
		GtkWidget *new_item;
		GList *item;

                /* FIXME: this doesn't properly set an item already in
                the list as the top (and shown option) */
		for (item = GTK_LIST (list)->children;
		     item; item = item->next) {
			GtkLabel *label = GTK_LABEL (
				GTK_BIN (item->data)->child);
			if (strcmp (GTK_LABEL (label)->label,
				    fontname) == 0) {
                                gtk_entry_set_text(
                                    GTK_ENTRY(GTK_COMBO(cfg->combo)->entry),
                                    GTK_LABEL(label)->label);
				gtk_entry_select_region (
					GTK_ENTRY (cfg->entry),
					0, GTK_ENTRY (cfg->entry)->text_length);
				return;
			}
		}

		new_item = gtk_list_item_new_with_label (fontname);
		gtk_widget_show (new_item);
		gtk_container_add (GTK_CONTAINER (list), new_item);
		gtk_entry_set_text (GTK_ENTRY (cfg->entry), fontname);
		g_free (fontname);
	}
	gtk_entry_select_region (GTK_ENTRY (cfg->entry), 0,
				 GTK_ENTRY (cfg->entry)->text_length);
}

static void
update_fontsel_cb (GtkWidget * widget, gpointer user_data)
{
	gfontsel_cfg_t *cfg = (gfontsel_cfg_t *) user_data;
	gchar *fontname;

	fontname = gtk_entry_get_text (GTK_ENTRY (cfg->entry));

	if (fontname && cfg->prev_fontname &&
	    strcmp (cfg->prev_fontname, fontname) == 0)
		return;

	if (fontname && *fontname) {
		gtk_font_selection_set_font_name
			(GTK_FONT_SELECTION (cfg->fontsel), fontname);
		if (cfg->prev_fontname)
			g_free (cfg->prev_fontname);
		cfg->prev_fontname = g_strdup (fontname);
	}
}

static void
cancel_cb (GtkWidget * widget, gpointer user_data)
{
	gtk_widget_destroy (widget);
	gtk_main_quit ();
}

static void
help_cb (GtkWidget * widget, gpointer user_data)
{
    GnomeHelpMenuEntry ref = {"gfontsel", "index.html"};
			gnome_help_display (NULL, &ref);
			

  
    
}

static void
close_cb (GtkWidget * widget, gpointer user_data)
{
	gfontsel_cfg_t *cfg = (gfontsel_cfg_t *) user_data;
	gchar *fontname;

	fontname = gtk_entry_get_text (GTK_ENTRY (cfg->entry));

	if (!(fontname && *fontname))
		fontname = gtk_font_selection_get_font_name (
			GTK_FONT_SELECTION (cfg->fontsel));

	if (fontname && *fontname) {
		if (cfg->remember_font) {
			gnome_config_set_string (CONFIG_LASTFONT, fontname);
			gnome_config_sync ();
		}
		if (cfg->print_on_exit)
			g_print ("%s", fontname);
	}
	gtk_widget_destroy (widget);
	gtk_main_quit ();
}

static GtkWidget *
get_logo (void)
{
	GtkWidget *pixmap = NULL;
	gchar *data = gnome_pixmap_file ("gnome-default.png");
	
	if (data) {
		pixmap = gnome_pixmap_new_from_file (data);
		g_free (data);
	}
	
	return pixmap;
}

static void
gfontsel_create_dialog (gfontsel_cfg_t * cfg)
{
	GtkWidget *box, *label, *sep, *pixmap;
	gchar *on_load_font = NULL;

	cfg->dialog = gnome_dialog_new (_("Font Selector"),
					GNOME_STOCK_BUTTON_APPLY,
					GNOME_STOCK_BUTTON_CLOSE,
					GNOME_STOCK_BUTTON_HELP,
					NULL);

	gtk_signal_connect (GTK_OBJECT (cfg->dialog), "close",
			    GTK_SIGNAL_FUNC (cancel_cb), cfg);
	gnome_dialog_button_connect (GNOME_DIALOG (cfg->dialog), 0,
				     GTK_SIGNAL_FUNC (copy_and_activate_cb),
				     cfg);
	gnome_dialog_button_connect (GNOME_DIALOG (cfg->dialog), 1,
				     GTK_SIGNAL_FUNC (close_cb), cfg);
	gnome_dialog_button_connect (GNOME_DIALOG (cfg->dialog), 2,
				     GTK_SIGNAL_FUNC (help_cb), cfg);
	gnome_dialog_set_default (GNOME_DIALOG (cfg->dialog), 0);
	gnome_dialog_set_accelerator (GNOME_DIALOG (cfg->dialog), 0,
				      GDK_space, 0);

	box = gtk_hbox_new (FALSE, GNOME_PAD_SMALL);
	gtk_widget_show (box);

	pixmap = get_logo ();
	if (pixmap) {
		gtk_widget_show (pixmap);
		gtk_box_pack_start (GTK_BOX (box), pixmap, FALSE, FALSE, 0);
	}
	
	label = gtk_label_new (_("Selection:"));
	gtk_widget_show (label);
	gtk_box_pack_start (GTK_BOX (box), label, FALSE, FALSE, 0);

	cfg->combo = gtk_combo_new ();
	gtk_widget_show (cfg->combo);
	gtk_box_pack_start (GTK_BOX (box), cfg->combo,
			    TRUE, TRUE, GNOME_PAD_SMALL);

	gtk_container_add (GTK_CONTAINER (
		GNOME_DIALOG (cfg->dialog)->vbox), box);

	cfg->entry = GTK_COMBO (cfg->combo)->entry;
	gtk_signal_connect (GTK_OBJECT (cfg->entry), "changed",
			    GTK_SIGNAL_FUNC (update_fontsel_cb), cfg);

	sep = gtk_hseparator_new ();
	gtk_widget_show (sep);
	gtk_container_add (GTK_CONTAINER (
		GNOME_DIALOG (cfg->dialog)->vbox), sep);

	cfg->fontsel = gtk_font_selection_new ();
	gtk_widget_show (cfg->fontsel);
	gtk_container_add (GTK_CONTAINER (
		GNOME_DIALOG (cfg->dialog)->vbox), cfg->fontsel);

	if (cfg->font_load)
		on_load_font = cfg->font_load;
	else if (cfg->load_last)
		on_load_font = gnome_config_get_string (CONFIG_LASTFONT);

	if (on_load_font) {
		GtkWidget *item;

		gtk_font_selection_set_font_name (
			GTK_FONT_SELECTION (cfg->fontsel), on_load_font);
		item = gtk_list_item_new_with_label (on_load_font);
		gtk_widget_show (item);
		gtk_container_add (GTK_CONTAINER (
			GTK_COMBO (cfg->combo)->list), item);
		g_free (on_load_font);
	}
}

static void
handle_signal (int sig)
{
	exit (EXIT_SUCCESS);
}

static void
parse_an_arg (poptContext ctx,
	      enum poptCallbackReason reason,
	      const struct poptOption *opt,
	      const char *arg, void *data)
{
	gfontsel_cfg_t *cfg = (gfontsel_cfg_t *) data;

	switch (opt->val) {
#define FONT_KEY			'f'
	case FONT_KEY:
		cfg->font_load = g_strdup (arg);
		break;

#define PRINT_KEY			'p'
	case PRINT_KEY:
		cfg->print_on_exit = TRUE;
		break;

#define NO_REMEMBER_FONT_KEY		-1
	case NO_REMEMBER_FONT_KEY:
		cfg->remember_font = FALSE;
		break;

#define NO_LOAD_LAST_FONT_KEY		-2
	case NO_LOAD_LAST_FONT_KEY:
		cfg->load_last = FALSE;
		break;
	}
}

static struct poptOption options[] = {
  {NULL, '\0', POPT_ARG_CALLBACK, &parse_an_arg, 0, NULL, NULL},
  {"font", 'f', POPT_ARG_STRING, NULL, FONT_KEY, N_("Font to load"), N_("FONTSPEC")},
  {"print", 'p', POPT_ARG_NONE, NULL, PRINT_KEY, N_("Print selected font name on exit"), NULL},
  {"noremember", '\0', POPT_ARG_NONE, NULL, NO_REMEMBER_FONT_KEY, N_("Inhibit remembering selected font for next run"), NULL},
  {"nolast", '\0', POPT_ARG_NONE, NULL, NO_LOAD_LAST_FONT_KEY, N_("Inhibit loading of last font"), NULL},
  {NULL, '\0', 0, NULL, 0}
};

int
main (int argc, char *argv[])
{
	static gfontsel_cfg_t cfg;
	
	bindtextdomain (PACKAGE, GNOMELOCALEDIR);
	textdomain (PACKAGE);

	cfg.font_load = NULL;
	cfg.print_on_exit = FALSE;
	cfg.remember_font = TRUE;
	cfg.load_last = TRUE;
	options[0].descrip = (char *) &cfg;
	gnome_init_with_popt_table ("gfontsel", VERSION,
				    argc, argv,
				    options, 0, NULL);
	gnome_window_icon_set_default_from_file (GNOME_ICONDIR"/gnome-fontsel.png");
	
	signal (SIGHUP, handle_signal);
	signal (SIGINT, handle_signal);
	signal (SIGQUIT, handle_signal);
	signal (SIGTERM, handle_signal);

	gfontsel_create_dialog (&cfg);
	gtk_widget_show (cfg.dialog);
	gtk_main ();

	return EXIT_SUCCESS;
}
