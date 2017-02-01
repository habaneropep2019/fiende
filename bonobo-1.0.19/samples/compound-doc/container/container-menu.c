#include "config.h"
#include "container-menu.h"

#include "container-io.h"
#include "container-print.h"
#include "container-filesel.h"

static void
verb_AddEmbeddable_cb (BonoboUIComponent *uic, gpointer user_data, const char *cname)
{
	SampleApp *inst = user_data;
	char *required_interfaces [2] =
	    { "IDL:Bonobo/Embeddable:1.0", NULL };
	char *obj_id;

	/* Ask the user to select a component. */
	obj_id = bonobo_selector_select_id (
		_("Select an embeddable Bonobo component to add"),
		(const gchar **) required_interfaces);

	if (!obj_id)
		return;

	/* Activate it. */
	sample_app_add_component (inst, obj_id);

	g_free (obj_id);
}

static void
load_ok_cb (GtkWidget *caller, SampleApp *app)
{
	GtkWidget *fs = app->fileselection;
	gchar *filename = gtk_file_selection_get_filename
		(GTK_FILE_SELECTION (fs));

	if (filename)
		sample_container_load (app, filename);

	gtk_widget_destroy (fs);
}

static void
save_ok_cb (GtkWidget *caller, SampleApp *app)
{
	GtkWidget *fs = app->fileselection;
	gchar *filename = gtk_file_selection_get_filename
	    (GTK_FILE_SELECTION (fs));

	if (filename)
		sample_container_save (app, filename);

	gtk_widget_destroy (fs);
}

static void
verb_FileSaveAs_cb (BonoboUIComponent *uic, gpointer user_data, const char *cname)
{
	SampleApp *app = user_data;

	container_request_file (app, TRUE, save_ok_cb, app);
}

static void
verb_FileLoad_cb (BonoboUIComponent *uic, gpointer user_data, const char *cname)
{
	SampleApp *app = user_data;

	container_request_file (app, FALSE, load_ok_cb, app);
}

static void
verb_PrintPreview_cb (BonoboUIComponent *uic, gpointer user_data, const char *cname)
{
	SampleApp *app = user_data;

	sample_app_print_preview (app);
}

static void
verb_XmlDump_cb (BonoboUIComponent *uic, gpointer user_data, const char *cname)
{
	SampleApp *app = user_data;

	bonobo_window_dump (BONOBO_WINDOW (app->app), "On request");
}

static void
verb_HelpAbout_cb (BonoboUIComponent *uic, gpointer user_data, const char *cname)
{
	static const gchar *authors[] = {
		"ÉRDI Gergõ <cactus@cactus.rulez.org>",
		"Michael Meeks <michael@helixcode.com>",
		NULL
	};

	GtkWidget *about = gnome_about_new ("sample-container", VERSION,
					    "(C) 2000 ÉRDI Gergõ, Helix Code, Inc",
					    authors,
					    _("Bonobo sample container"), NULL);
	gtk_widget_show (about);
}

static void
verb_FileExit_cb (BonoboUIComponent *uic, gpointer user_data, const char *cname)
{
	SampleApp *app = user_data;

	sample_app_exit (app);
}

/*
 * The menus.
 */
static char ui_commands [] =
"<commands>\n"
"	<cmd name=\"AddEmbeddable\" _label=\"A_dd Embeddable\"/>\n"
"	<cmd name=\"FileOpen\" _label=\"_Open\"\n"
"		pixtype=\"stock\" pixname=\"Open\" _tip=\"Open a file\"/>\n"
"	<cmd name=\"FileSaveAs\" _label=\"Save _As...\"\n"
"		pixtype=\"stock\" pixname=\"Save\"\n"
"		_tip=\"Save the current file with a different name\"/>\n"
"	<cmd name=\"XmlDump\" _label=\"Xml dump\"/>\n"
"	<cmd name=\"PrintPreview\" _label=\"Print Pre_view\"/>\n"
"	<cmd name=\"FileExit\" _label=\"E_xit\" _tip=\"Exit the program\"\n"
"		pixtype=\"stock\" pixname=\"Quit\" accel=\"*Control*q\"/>\n"
"	<cmd name=\"HelpAbout\" _label=\"_About...\" _tip=\"About this application\"\n"
"		pixtype=\"stock\" pixname=\"About\"/>\n"
"</commands>";

static char ui_data [] =
"<menu>\n"
"	<submenu name=\"File\" _label=\"_File\">\n"
"		<menuitem name=\"AddEmbeddable\" verb=\"\"/>\n"
"		<separator/>"
"		<menuitem name=\"FileOpen\" verb=\"\"/>\n"
"\n"
"		<menuitem name=\"FileSaveAs\" verb=\"\"/>\n"
"\n"
"		<placeholder name=\"Placeholder\"/>\n"
"\n"
"		<menuitem name=\"XmlDump\" verb=\"\"/>\n"
"		<separator/>\n"
"		<menuitem name=\"PrintPreview\" verb=\"\"/>\n"
"		<separator/>\n"
"		<menuitem name=\"FileExit\" verb=\"\"/>\n"
"	</submenu>\n"
"\n"
"	<submenu name=\"Help\" _label=\"_Help\">\n"
"		<menuitem name=\"HelpAbout\" verb=\"\"/>\n"
"	</submenu>\n"
"</menu>";

static BonoboUIVerb sample_app_verbs[] = {
	BONOBO_UI_VERB ("AddEmbeddable", verb_AddEmbeddable_cb),
	BONOBO_UI_VERB ("FileOpen", verb_FileLoad_cb),
	BONOBO_UI_VERB ("FileSaveAs", verb_FileSaveAs_cb),
	BONOBO_UI_VERB ("PrintPreview", verb_PrintPreview_cb),
	BONOBO_UI_VERB ("XmlDump", verb_XmlDump_cb),
	BONOBO_UI_VERB ("FileExit", verb_FileExit_cb),
	BONOBO_UI_VERB ("HelpAbout", verb_HelpAbout_cb),
	BONOBO_UI_VERB_END
};

void
sample_app_fill_menu (SampleApp *app)
{
	Bonobo_UIContainer corba_container;
	BonoboUIComponent *uic;

	uic = bonobo_ui_component_new ("sample");
	corba_container = BONOBO_OBJREF (app->ui_container);
	bonobo_ui_component_set_container (uic, corba_container);

	bonobo_ui_component_set_translate (uic, "/", ui_commands, NULL);
	bonobo_ui_component_set_translate (uic, "/", ui_data, NULL);

	bonobo_ui_component_add_verb_list_with_data (uic, sample_app_verbs, app);

#if 0
	BonoboUIHandlerMenuItem *menu_list;

	/* Load the menu bar with the container-specific base menus */
	menu_list = bonobo_ui_handler_menu_parse_uiinfo_list_with_data
		(sample_app_menu, app);

	bonobo_ui_handler_menu_add_list  (app->ui_handler, "/", menu_list);
	bonobo_ui_handler_menu_free_list (menu_list);
#endif
}
