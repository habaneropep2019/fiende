/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */
#include "corba-glue.h"
#include "capplet-manager.h"
#include <orb/orbit.h>
#include "gnome.h"
#include <libgnorba/gnorba.h>
#include <gdk/gdkx.h>

/* prototypes */
#if 0
static void orb_add_connection(GIOPConnection *cnx);
static void orb_remove_connection(GIOPConnection *cnx);
static void orb_handle_connection(GIOPConnection *cnx, gint source, GdkInputCondition cond);
#endif
void control_center_corba_gtk_init(gint *argc, char **argv);
int server_register_capplet(PortableServer_Servant servant,
                        CORBA_long id,
                        GNOME_capplet cap,
                        CORBA_Environment * env);
void server_state_changed(PortableServer_Servant servant, CORBA_long id, CORBA_boolean undoable, CORBA_Environment * ev);
void server_changes_are_immediate(PortableServer_Servant servant, CORBA_long id, CORBA_Environment * ev);
void server_register_capplet_new (PortableServer_Servant servant,
                                  const GNOME_capplet cap,
                                  const CORBA_char * name,
                                  const CORBA_long capid,
                                  CORBA_unsigned_long * xid,
                                  CORBA_long * newid,
                                  CORBA_Environment * ev);
void server_show_control_center(PortableServer_Servant servant, CORBA_Environment * ev);
extern gchar *init_cap;
extern GtkWidget *tree;
extern GtkWidget *main_window;
/* Variables */
CORBA_ORB orb = NULL;
CORBA_Environment ev;
GNOME_control_center control_center = NULL;

gchar *ior;
PortableServer_ServantBase__epv base_epv = {
        NULL,
        NULL,
        NULL
};
POA_GNOME_control_center__epv control_center_epv = 
{  
        NULL, 
        (gpointer)&server_register_capplet,
        (gpointer)&server_state_changed,
        (gpointer)&server_register_capplet_new,
        (gpointer)&server_show_control_center,
	(gpointer)&server_changes_are_immediate
};
POA_GNOME_control_center__vepv poa_control_center_vepv = { &base_epv, &control_center_epv };
POA_GNOME_control_center poa_control_center_servant = { NULL, &poa_control_center_vepv };

#if 0
static void
orb_add_connection(GIOPConnection *cnx)
{
         cnx->user_data = (gpointer)gtk_input_add_full(GIOP_CONNECTION_GET_FD(cnx),
                                                      GDK_INPUT_READ|GDK_INPUT_EXCEPTION,
                                                      (GdkInputFunction)orb_handle_connection,
                                                      NULL, cnx, NULL);
}

static void
orb_remove_connection(GIOPConnection *cnx)
{
        gtk_input_remove((guint)cnx->user_data);
        cnx->user_data = (gpointer)-1;
}
#endif

void
control_center_corba_gtk_main_quit(void)
{
       CORBA_ORB_shutdown(orb, CORBA_FALSE, &ev);
       gtk_main_quit();
}

static struct poptOption cap_options[] = {
        {"capplet", '\0', POPT_ARG_STRING, &init_cap, 0, N_("capplet-command to be run."), N_("CAPPLET")},
        {NULL, '\0', 0, NULL, 0}
};

void
control_center_corba_gtk_init(gint *argc, char **argv)
{
        PortableServer_ObjectId objid = {0, sizeof("control_center_interface"), "control_center_interface"};
        PortableServer_POA poa;
        CORBA_exception_init(&ev);
        orb = gnome_CORBA_init_with_popt_table (g_basename(argv[0]), VERSION, argc, argv, cap_options, 0, NULL, GNORBA_INIT_SERVER_FUNC, &ev);

        poa = (PortableServer_POA)CORBA_ORB_resolve_initial_references(orb, "RootPOA", &ev);
        POA_GNOME_control_center__init(&poa_control_center_servant, &ev);

        PortableServer_POA_activate_object_with_id(poa, 
                                                   &objid, &poa_control_center_servant, &ev);
        control_center = PortableServer_POA_servant_to_reference((PortableServer_POA)orb->root_poa, 
                                                      &poa_control_center_servant, &ev);

        if (!control_center) {
                g_error ("Unable to initialize CORBA.\naborting...\n");
                exit (1);
        }
        ior = CORBA_ORB_object_to_string(orb, control_center, &ev);
        switch(goad_server_register (CORBA_OBJECT_NIL,
                                     control_center,
                                     "control_center",
                                     "server",
                                     &ev)) {
        case -2:
                /* There is a control-center running; bring it forward and quit */
                control_center = goad_server_activate_with_repo_id (NULL,
                                                                    "IDL:GNOME/control_center:1.0",
                                                                    0, NULL);
                GNOME_control_center_show_control_center (control_center, &ev);
                exit (0);
        case -1:
                g_warning ("There was trouble with the name server.\naborting...\n");
                exit (1);
        }
}

void
control_center_corba_gtk_main (gint *argc, char **argv)
{
        if(!orb) {
                control_center_corba_gtk_init( argc, argv);
        }
        PortableServer_POAManager_activate(PortableServer_POA__get_the_POAManager(CORBA_ORB_resolve_initial_references(orb, "RootPOA", &ev), &ev), &ev);

        gtk_main();
}

#if 0
static void
orb_handle_connection(GIOPConnection *cnx, gint source, GdkInputCondition cond)
{
        switch(cond) {
        case GDK_INPUT_EXCEPTION:
                giop_main_handle_connection_exception(cnx);
                break;
        default:
                giop_main_handle_connection(cnx);
        }
}
#endif

int
server_register_capplet(PortableServer_Servant servant,
                        CORBA_long id,
                        GNOME_capplet cap,
                        CORBA_Environment * env)
{
        node_data *nd = find_node_by_id (id);
        if (nd == NULL || nd->id == -1) {
                /* It's already been killed */
                return -1;
        }
        nd->capplet = cap;
        CORBA_Object_duplicate (cap, env);
        nd->state = CAPPLET_ACTIVE;
        return 0;
}
void
server_state_changed(PortableServer_Servant servant, CORBA_long id, CORBA_boolean undoable, CORBA_Environment * ev)
{
        node_data *nd = find_node_by_id (id);
        GdkColor col;
        col.red = 56000;
        col.green = 0;
        col.blue = 0;
        if (nd == NULL){
                g_warning ("couldn't find node %d\n",id);
                g_warning ("this capplet will not function correctly...\n");
                return;
        }
        nd->modified = TRUE;
        gtk_widget_set_sensitive (nd->try_button, TRUE);
        if (undoable)
                gtk_widget_set_sensitive (nd->revert_button, TRUE);
        gtk_widget_set_sensitive (nd->ok_button, TRUE);
        
        gtk_ctree_node_set_foreground (nd->ctree, nd->node, &col);
}

void
server_changes_are_immediate(PortableServer_Servant servant, CORBA_long id, CORBA_Environment * ev)
{
        node_data *nd = find_node_by_id (id);
        if (nd == NULL){
                g_warning ("couldn't find node %d\n",id);
                g_warning ("this capplet will not function correctly...\n");
                return;
        }
        gtk_widget_set_sensitive (nd->try_button, FALSE);
        gtk_widget_set_sensitive (nd->revert_button, FALSE);
        gtk_widget_set_sensitive (nd->cancel_button, FALSE);
        gtk_widget_set_sensitive (nd->ok_button, TRUE);        
}

static void
find_capplet (GtkCTree     *ctree,
              GtkCTreeNode *node,
              gpointer      list)
{
        node_data **data_pointer = (node_data **) (((GSList *)list)->data);
        node_data *nd = (node_data *) gtk_ctree_node_get_row_data (GTK_CTREE (ctree),
                                                                   node); 
        gchar *name = (gchar*) (((GSList *)list)->next->data);
        gint capid = (gint) (((GSList *)list)->next->next->data);
        gint i;

        if (*data_pointer != NULL)
                return;
        if (nd->gde->exec && nd->gde->exec[0]
            && !strcmp (nd->gde->exec[0], name)) {
                if (capid != -1) {
                        for (i = 1;nd->gde->exec[i];i++) { 
                                if (strstr (nd->gde->exec[i], "--cap-id=")) {
                                       if ((nd->gde->exec[i] + 9) &&
                                           (atoi (nd->gde->exec[i] + 9) ==
                                            capid)) {
                                               *data_pointer = nd;
                                       }
                                }
                        }
                } else
                        *data_pointer = nd;
        }
}
static void
show_self ()
{
        gint workspace;
        gint x, y;

        workspace = gnome_win_hints_get_current_workspace ();
        if (workspace != gnome_win_hints_get_workspace (main_window))
                gnome_win_hints_set_workspace (main_window, workspace);
        
        gdk_window_get_position (main_window->window, &x, &y);
        if (x > gdk_screen_width () || y > gdk_screen_height () || x < 0 || y < 0) {
                gtk_widget_unmap (main_window);
                gtk_widget_map (main_window);
        }
        gdk_window_show (main_window->window);

}
void server_register_capplet_new (PortableServer_Servant servant,
                                  const GNOME_capplet cap,
                                  const CORBA_char * name,
                                  const CORBA_long capid,
                                  CORBA_unsigned_long * xid,
                                  CORBA_long * newid,
                                  CORBA_Environment *ev)
{
        node_data *nd = NULL;
        GSList *ugly_list = NULL;

        ugly_list = g_slist_prepend (ugly_list, (gpointer) capid);
        ugly_list = g_slist_prepend (ugly_list, (gpointer) name);
        ugly_list = g_slist_prepend (ugly_list, &nd);
        
        /* First thing we want to do is find the entry, if possible */
        gtk_ctree_post_recursive (GTK_CTREE (tree),NULL, find_capplet, ugly_list);
        if ((nd == NULL) || (nd->id != -1)) {
                /* Abort, Abort */
                *xid = 0;
                *newid = -1;

                return;
        }
        nd->capplet = CORBA_Object_duplicate (cap, ev);
        launch_capplet (nd, FALSE);
        *xid = GDK_WINDOW_XWINDOW (nd->socket->window);
        *newid = nd->id;
        nd->state = CAPPLET_ACTIVE;
        show_self ();
}
void
server_show_control_center(PortableServer_Servant servant, CORBA_Environment * ev)
{
        show_self ();
}

