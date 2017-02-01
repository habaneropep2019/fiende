/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "capplet-widget-libs.h"
#include "control-center.h"
#include <libgnorba/gnorba.h>
#include <orb/orbit.h>

static CORBA_ORB orb;
static CORBA_Environment ev;
static GNOME_capplet capplet = NULL;
static GNOME_control_center control_center;
static gchar* cc_ior = NULL;
static gint id = -1;
static guint xid = 0;
static gint capid = -1;
static GList *id_list = NULL;
static gint cap_session_init = 0;
static gint cap_ignore = 0;
/* structs */
typedef struct _keyval keyval;
struct _keyval
{
        gint capid;
        guint32 xid;
        gint id;
};

/* prototypes... */
static void server_try (PortableServer_Servant servant, CORBA_long id, CORBA_Environment * ev);
static void server_revert (PortableServer_Servant servant, CORBA_long id, CORBA_Environment * ev);
static void server_ok (PortableServer_Servant servant, CORBA_long id, CORBA_Environment * ev);
static void server_cancel (PortableServer_Servant servant, CORBA_long id, CORBA_Environment * ev);
static void server_help (PortableServer_Servant servant, CORBA_long id, CORBA_Environment * ev);
static void server_new_multi_capplet(GNOME_capplet _obj, CORBA_long id, CORBA_long newid, CORBA_unsigned_long newxid, CORBA_long newcapid, CORBA_Environment * ev);
static void server_page_hidden (PortableServer_Servant servant, CORBA_long id, CORBA_Environment * ev);
static void server_page_shown (PortableServer_Servant servant, CORBA_long id, CORBA_Environment * ev);
extern void _capplet_widget_server_try(gint id);
extern void _capplet_widget_server_revert(gint id);
extern void _capplet_widget_server_ok(gint id);
extern void _capplet_widget_server_cancel(gint id);
extern void _capplet_widget_server_help(gint id);
extern void _capplet_widget_server_new_multi_capplet(gint id, gint capid);
extern void _capplet_widget_server_page_hidden(gint id);
extern void _capplet_widget_server_page_shown(gint id);

static struct poptOption cap_options[] = {
        {"id", '\0', POPT_ARG_INT, &id, 0, N_("id of the capplet -- assigned by the control-center"), N_("ID")},
        {"cap-id", '\0', POPT_ARG_INT, &capid, 0, N_("Multi-capplet id."), N_("CAPID")},
        {"xid", '\0', POPT_ARG_INT, &xid, 0, N_("X ID of the socket it's plugged into"), N_("XID")},
        {"ior", '\0', POPT_ARG_STRING, &cc_ior, 0, N_("IOR of the control-center"), N_("IOR")},
        {"init-session-settings", '\0', POPT_ARG_NONE, &cap_session_init, 0, N_("Initialize session settings"), NULL},
        {"ignore", '\0', POPT_ARG_NONE, &cap_ignore, 0, N_("Ignore default action.  Used for custom init-session cases"), NULL},
        {NULL, '\0', 0, NULL, 0}
};


/* ORB stuff... */
static PortableServer_ServantBase__epv base_epv = {
        NULL,
        NULL,
        NULL
};
static POA_GNOME_capplet__epv capplet_epv = 
{  
        NULL,
        (gpointer)&server_try, 
        (gpointer)&server_revert,
        (gpointer)&server_ok,
        (gpointer)&server_cancel,
        (gpointer)&server_help,
        (gpointer)&server_new_multi_capplet,
        (gpointer)&server_page_hidden,
        (gpointer)&server_page_shown
};
static POA_GNOME_capplet__vepv poa_capplet_vepv = { &base_epv, &capplet_epv };
static POA_GNOME_capplet poa_capplet_servant = { NULL, &poa_capplet_vepv };

static void
server_try (PortableServer_Servant servant, CORBA_long id, CORBA_Environment * ev)
{
        _capplet_widget_server_try (id);
}
static void
server_revert (PortableServer_Servant servant, CORBA_long id, CORBA_Environment * ev)
{
        _capplet_widget_server_revert (id);
}
static void
server_ok (PortableServer_Servant servant, CORBA_long id, CORBA_Environment * ev)
{
        _capplet_widget_server_ok (id);
}
static void
server_cancel (PortableServer_Servant servant, CORBA_long id, CORBA_Environment * ev)
{
        _capplet_widget_server_cancel (id);
}
static void
server_help (PortableServer_Servant servant, CORBA_long id, CORBA_Environment * ev)
{
        _capplet_widget_server_help (id);
}
static void
server_page_hidden (PortableServer_Servant servant, CORBA_long id, CORBA_Environment * ev)
{
        _capplet_widget_server_page_hidden (id);
}
static void
server_page_shown (PortableServer_Servant servant, CORBA_long id, CORBA_Environment * ev)
{
        _capplet_widget_server_page_shown (id);
}
static void
server_new_multi_capplet(GNOME_capplet _obj, CORBA_long id, CORBA_long newid, CORBA_unsigned_long newxid, CORBA_long newcapid, CORBA_Environment * ev)
{
        GList *temp;
        keyval *nkv;
        for (temp = id_list; temp; temp = temp->next)
                if (((keyval *)temp->data)->capid == capid) {
                        ((keyval *)temp->data)->xid = xid;
                        ((keyval *)temp->data)->id = newid;
                        _capplet_widget_server_new_multi_capplet (id, capid);
                        return;
                };
        nkv = g_malloc (sizeof (nkv));
        nkv->capid = newcapid;
        nkv->id = newid;
        nkv->xid = newxid;
        id_list = g_list_prepend (id_list, nkv);
        _capplet_widget_server_new_multi_capplet (id, newcapid);
}

/* public methods... */
void
capplet_corba_gtk_main (void)
{
        if (!orb) {
                g_warning ("Corba must be initialized before gtk_main can be called\n");
                exit (1);
        }
        gtk_main();
}

static gint
do_quit(gpointer data)
{
        /* This gross hack is here because just doing gtk_main_quit()
           won't exit a modal dialog that has popped up in a capplet,
           but at the same time just doing exit() won't let some
           capplets (e.g. wm-properties) do their preference saving in
           the post-gtk_main() part of main() -ECL */

        gtk_main_quit();

        if(gtk_main_level() > 1)
                gtk_idle_add(do_quit, NULL);
                
	return 0;            
}

void
capplet_corba_gtk_main_quit (void)
{
        CORBA_Object_release (control_center, &ev);
        CORBA_ORB_shutdown(orb, CORBA_FALSE, &ev);

        do_quit(NULL);
}

void
capplet_corba_state_changed (gint id, gboolean undoable)
{
        GNOME_control_center_state_changed(control_center, id, undoable, &ev);
}

void
capplet_corba_changes_are_immediate (gint id)
{
        GNOME_control_center_changes_are_immediate(control_center, id, &ev);
}

guint32
_capplet_int_get_xid (gint cid)
{
        GList *temp;
        if ((cid == -1) || (cid == capid))
                return xid;

        for (temp = id_list; temp; temp = temp->next)
                if (((keyval *)temp->data)->capid == cid)
                        return ((keyval *)temp->data)->xid;
        g_warning ("received an unknown cid: %d\n", cid);
        return 0;
}
gint _capplet_int_get_ccid (gint cid)
{
        GList *temp;
        if ((cid == -1) || (cid == capid)) {
                return id;
        }
        for (temp = id_list; temp; temp = temp->next)
                if (((keyval *)temp->data)->capid == cid) {
                        return ((keyval *)temp->data)->id;
                }
        g_warning ("received an unknown cid: %d\n", cid);
        return id;
}

gint _capplet_int_session_initialization_requested_p(void)
{
        return cap_session_init;
}

gint _capplet_int_session_ignore_requested_p(void)
{
        return cap_ignore;
}

gint _capplet_int_get_capid ()
{
        return capid;
}

void *capplet_widget_corba_init(const char *app_id,
                               const char *app_version,
                               int *argc, char **argv,
                               struct poptOption *options,
                               unsigned int flags,
                               poptContext *return_ctx)
{
        PortableServer_ObjectId objid = {0, sizeof("capplet_interface"), "capplet_interface"};
        PortableServer_POA poa;
        gchar *name;

        CORBA_exception_init(&ev);

        gnomelib_register_popt_table(cap_options, "capplet options");

        orb = gnome_CORBA_init_with_popt_table (app_id, app_version,
                                                argc, argv, options, flags,
                                                return_ctx, GNORBA_INIT_SERVER_FUNC, &ev);
        /* hmmm... will we ever want to init with CORBA at some point? */
        if(cap_session_init)
                return NULL;

        /* sanity check */
        if (!orb) {
                g_warning ("unable to connect to the server.\nexitting...\n");
                exit (1);
        }

        /* setup CORBA fully */
        POA_GNOME_capplet__init(&poa_capplet_servant, &ev);
        poa = (PortableServer_POA)CORBA_ORB_resolve_initial_references(orb, "RootPOA", &ev);
        PortableServer_POAManager_activate(PortableServer_POA__get_the_POAManager(poa, &ev), &ev);

        PortableServer_POA_activate_object_with_id(poa, 
                                                   &objid, &poa_capplet_servant, &ev);
        capplet = PortableServer_POA_servant_to_reference((PortableServer_POA)orb->root_poa, 
                                                      &poa_capplet_servant, &ev);
        if (!capplet) {
                g_warning ("We cannot connect to a control_center\nexitting...\n");
                exit (1);
        }
        ORBit_custom_run_setup(orb, &ev);

        control_center = goad_server_activate_with_repo_id (NULL,
                                                            "IDL:GNOME/control_center:1.0",
                                                            0, NULL);
        
        /* now we get the control center. */
/*        control_center = CORBA_ORB_string_to_object(orb, cc_ior, &ev);*/
        if (! control_center) {
                g_warning ("Unable reach the control-center.\nExiting...");
                exit (1);
        }
        if ((xid == 0) || (id == -1)) {
                name = strrchr (argv[0], '/');
                if (name)
                        name++;
                else
                        name = argv[0];
                GNOME_control_center_register_capplet_new(control_center,
                                                          capplet,
                                                          name,
                                                          capid,
                                                          &xid,
                                                          &id,
                                                          &ev);
                /* if it's still 0, then the control-center doesn't wan't us */
                /* In theory, we should clean up after ourselves at this point */
                /* but I need to think a bit more about it. */
                /* Is there any time that we'd want to stay alive w/ CORBA and */
                /* not be a capplet??? */
                if ((xid == 0) || (id == -1))
                        return NULL;
        }
        if (GNOME_control_center_register_capplet(control_center, id, capplet, &ev) == -1)
                return NULL;
        /* we need to get the error value from gnome_CORBA_init */
        return (gpointer) orb;
}
