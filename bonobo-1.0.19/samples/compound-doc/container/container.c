#include <gnome.h>
#include <bonobo.h>
#include "config.h"
#include <liboaf/liboaf.h>

#include "container.h"
#include "component.h"
#include "container-io.h"
#include "container-menu.h"

poptContext ctx;

#define ASYNC_MONIKERS

void
sample_app_exit (SampleApp *app)
{
	while (app->components)
		bonobo_object_unref (
			BONOBO_OBJECT (app->components->data));

	bonobo_object_unref (BONOBO_OBJECT (app->container));
	bonobo_object_unref (BONOBO_OBJECT (app->ui_container));
	gtk_widget_destroy (app->app);

	gtk_main_quit ();
}

static gint
delete_cb (GtkWidget *caller, GdkEvent *event, SampleApp *app)
{
	sample_app_exit (app);

	return TRUE;
}

static SampleApp *
sample_app_create (void)
{
	SampleApp *app = g_new0 (SampleApp, 1);
	GtkWidget *app_widget;

	/* Create widgets */
	app_widget = app->app = bonobo_window_new ("sample-container",
						   _("Sample Bonobo container"));

	app->box = gtk_vbox_new (FALSE, 10);

	gtk_signal_connect (GTK_OBJECT (app_widget), "delete_event",
			    (GtkSignalFunc) delete_cb, app);

	/* Do the packing stuff */
	bonobo_window_set_contents (BONOBO_WINDOW (app->app), app->box);
	gtk_widget_set_usize (app_widget, 400, 600);

	app->container = bonobo_item_container_new ();

	app->ui_container = bonobo_ui_container_new ();
	bonobo_ui_container_set_win (app->ui_container, BONOBO_WINDOW (app->app));

	/* Create menu bar */
	sample_app_fill_menu (app);

	gtk_widget_show_all (app_widget);

	return app;
}

static SampleClientSite *
sample_app_add_embeddable (SampleApp          *app,
			   BonoboObjectClient *server,
			   char               *object_id)
{
	SampleClientSite *site;

	/*
	 * The ClientSite is the container-side point of contact for
	 * the Embeddable.  So there is a one-to-one correspondence
	 * between BonoboClientSites and BonoboEmbeddables.
	 */
	site = sample_client_site_new (app->container, app,
				       server, object_id);
	if (!site) {
		g_warning ("Can't create client site");
		return NULL;
	}

	app->components = g_list_append (app->components, site);

	gtk_box_pack_start (GTK_BOX (app->box),
			    sample_client_site_get_widget (site),
			    FALSE, FALSE, 0);

	sample_client_site_add_frame (site);

	gtk_widget_show_all (GTK_WIDGET (app->box));
	
	return site;
}

SampleClientSite *
sample_app_add_component (SampleApp *app,
			  gchar     *object_id)
{
	BonoboObjectClient *server;

	server = bonobo_object_activate (object_id, 0);

	if (!server) {
		gchar *error_msg;

		error_msg =
		    g_strdup_printf (_("Could not launch Embeddable %s!"),
				     object_id);
		gnome_warning_dialog (error_msg);
		g_free (error_msg);

		return NULL;
	}

	return sample_app_add_embeddable (app, server, object_id);
}

static void
do_add_cb (Bonobo_Unknown     embeddable,
	   CORBA_Environment *ev,
	   gpointer           user_data)
{
	SampleApp         *app = user_data;
	SampleClientSite  *site;

	if (BONOBO_EX (ev)) {
		g_warning ("Moniker resolve demarshal exception '%s'\n",
			   bonobo_exception_get_text (ev));
		return;
	}

	if (embeddable == CORBA_OBJECT_NIL) {
		g_warning ("Failed to demarshal embeddable");
		return;
	}

	site = sample_app_add_embeddable (
		app, bonobo_object_client_from_corba (embeddable),
		"Kippers");

	if (!site)
		g_warning ("Failed to add embeddable to app");
}

#ifdef ASYNC_MONIKERS
static void
add_moniker_async (SampleApp  *app,
		   const char *name,
		   const char *interface)
{
	CORBA_Environment ev;

	CORBA_exception_init (&ev);

	bonobo_get_object_async (name, interface, &ev, 5000, do_add_cb, app);

	if (BONOBO_EX (&ev)) {
		g_warning ("bonobo_get_object_async exception '%s'\n",
			 bonobo_exception_get_text (&ev));
		return;
	}

	CORBA_exception_free (&ev);
}
#else
static void
resolve_and_add (SampleApp *app, Bonobo_Moniker moniker, const char *interface)
{
	CORBA_Environment ev;
	char             *name;
	Bonobo_Unknown    object;

	CORBA_exception_init (&ev);

	object = bonobo_moniker_client_resolve_default (
		moniker, interface, &ev);

	do_add_cb (object, &ev, app);

	name = bonobo_moniker_client_get_name (moniker, &ev);
	g_print ("My moniker looks like '%s'\n", name);
	CORBA_free (name);

	bonobo_object_release_unref (moniker, &ev);
}

static Bonobo_Moniker
make_moniker (const char *name, SampleApp *app)
{
	Bonobo_Moniker    moniker = CORBA_OBJECT_NIL;
	CORBA_Environment ev;
	Bonobo_ActivationContext context;

	CORBA_exception_init (&ev);

	context = bonobo_context_get ("Activation", &ev);
	g_return_val_if_fail (context != CORBA_OBJECT_NIL, CORBA_OBJECT_NIL);
		
	moniker = Bonobo_ActivationContext_createFromName (context, name, &ev);

	if (ev._major != CORBA_NO_EXCEPTION) {
		g_warning ("Moniker new exception '%s'\n",
			   bonobo_exception_get_text (&ev));
		return CORBA_OBJECT_NIL;
	}
	
	return moniker;
}
#endif

typedef struct {
	SampleApp *app;
	const char **startup_files;
} setup_data_t;

/*
 *  This is placed after the Bonobo main loop has started
 * and we can handle CORBA properly.
 */
static guint
final_setup (setup_data_t *sd)
{
	const gchar **filenames = sd->startup_files;

	while (filenames && *filenames) {
		sample_container_load (sd->app, *filenames);
		filenames++;
	}

	if (g_getenv ("BONOBO_MONIKER_TEST")) {
#ifdef ASYNC_MONIKERS
		add_moniker_async (sd->app, "file:/demo/a.jpeg", "IDL:Bonobo/Embeddable:1.0");
#else
		{
			Bonobo_Moniker moniker;
		
			moniker = make_moniker ("file:/demo/a.jpeg", sd->app);
			resolve_and_add (sd->app, moniker, "IDL:Bonobo/Embeddable:1.0");
			bonobo_object_release_unref (moniker, NULL);
		}
#endif

/*		moniker = make_moniker ("OAFIID:bonobo_application-x-mines:804d34a8-57dd-428b-9c94-7aa3a8365230");
		resolve_and_add (sd->app, moniker, "IDL:Bonobo/Embeddable:1.0");
		bonobo_object_release_unref (moniker, NULL);*/

/*		moniker = make_moniker ("query:(bonobo:supported_mime_types.has ('image/x-png'))");
		resolve_and_add (sd->app, moniker, "IDL:Bonobo/Embeddable:1.0");
		bonobo_object_release_unref (moniker, NULL);*/
	}

	return FALSE;
}

int
main (int argc, char **argv)
{
	setup_data_t init_data;
	CORBA_Environment ev;
	CORBA_ORB orb;

	free (malloc (8));

 	CORBA_exception_init (&ev);
	gnome_init_with_popt_table ("container", VERSION,
				    argc, argv, oaf_popt_options, 0, &ctx);

	orb = oaf_init (argc, argv);

	if (bonobo_init (orb, NULL, NULL) == FALSE)
		g_error (_("Could not initialize Bonobo!\n"));

	init_data.app = sample_app_create ();

	if (ctx)
		init_data.startup_files = (const char**) poptGetArgs (ctx);
	else
		init_data.startup_files = NULL;

	gtk_idle_add ((GtkFunction) final_setup, &init_data);

	bonobo_main ();

	if (ctx)
		poptFreeContext (ctx);

	return 0;
}
