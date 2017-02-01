/* Control applet ("capplet") for the gnome-pilot address conduit,        */
/* based on                                                                 */
/* gpilotd control applet ('capplet') for use with the GNOME control center */

#include <pwd.h>
#include <sys/types.h>
#include <signal.h>
#include <ctype.h>
#include <gnome.h>

#include <config.h>
#include <capplet-widget.h>

#include <libgpilotdCM/gnome-pilot-conduit-management.h>
#include <libgpilotdCM/gnome-pilot-conduit-config.h>
#include <gpilotd/gnome-pilot-client.h>

#include "address-conduit.h"

/* tell changes callbacks to ignore changes or not */
static gboolean ignore_changes=FALSE;

/* capplet widget */
static GtkWidget *capplet=NULL;

/* host/device/pilot configuration windows */
GtkWidget *cfgOptionsWindow=NULL;
GtkWidget *cfgStateWindow=NULL;
GtkWidget *dialogWindow=NULL;

GnomePilotConduitManagement *conduit;
GnomePilotConduitConfig *conduit_config;

ConduitCfg *origState = NULL;
ConduitCfg *curState = NULL;

static void doTrySettings(GtkWidget *widget, gpointer whatever);
static void doRevertSettings(GtkWidget *widget, gpointer whatever);
static void doSaveSettings(GtkWidget *widget, gpointer whatever);
static void doCancelSettings(GtkWidget *widget, gpointer whatever);

static void readStateCfg(GtkWidget *w, ConduitCfg *state);
static void setStateCfg(GtkWidget *w, ConduitCfg *state);

static void readOptionsCfg(GtkWidget *w, ConduitCfg *state);
static void setOptionsCfg(GtkWidget *w, ConduitCfg *state);

gint pilotId;
static GnomePilotClient *gpc;


static void 
load_configuration(ConduitCfg **c,guint32 pilotId) 
{
	gchar prefix[256];
	g_return_if_fail(c!=NULL);
	
	g_snprintf(prefix,255,CONFIG_PREFIX,pilotId);
 
	*c = g_new0(ConduitCfg,1);
	gnome_config_push_prefix(prefix);
	(*c)->sync_type = GnomePilotConduitSyncTypeCustom; /* this will be reset by capplet */
	(*c)->filename = gnome_config_get_string("filename");
	(*c)->open_secret = gnome_config_get_bool("open_secret=FALSE");
	gnome_config_pop_prefix();
	
	if((*c)->filename==NULL) {
		(*c)->filename=g_strdup_printf("%s/.gnome/GnomeCard.gcrd",
		g_get_home_dir ());
	} 
	(*c)->pilotId = pilotId;
}

static void 
save_configuration(ConduitCfg *c) 
{
	gchar prefix[256];

	g_return_if_fail(c!=NULL);
	g_snprintf(prefix,255,CONFIG_PREFIX,c->pilotId);
	
	gnome_config_push_prefix(prefix);
	gnome_config_set_string("filename",c->filename);
	gnome_config_set_bool("open_secret",c->open_secret);
	gnome_config_pop_prefix();
	
	gnome_config_sync();
	gnome_config_drop_all();
}

static void 
copy_configuration(ConduitCfg *d, ConduitCfg *c)
{
	g_return_if_fail(c!=NULL);
	g_return_if_fail(d!=NULL);
	d->sync_type=c->sync_type;
	if(d->filename) g_free(d->filename);
	d->filename = g_strdup(c->filename);
	d->open_secret = c->open_secret;
	d->pilotId = c->pilotId;
}

static ConduitCfg*
dupe_configuration(ConduitCfg *c) 
{
	ConduitCfg *d;
	g_return_val_if_fail(c!=NULL,NULL);
	d = g_new0(ConduitCfg,1);
	copy_configuration(d,c);
	return d;
}


/** this method frees all data from the conduit config */
static void 
destroy_configuration(ConduitCfg **c) 
{
	g_return_if_fail(c!=NULL);
	if((*c)->filename) g_free((*c)->filename);
	g_free(*c);
	*c = NULL;
}

/* This array must be in the same order as enumerations
   in GnomePilotConduitSyncType as they are used as index.
   Custom type implies Disabled state.
*/
static gchar* sync_options[] ={ N_("Disabled"),
				N_("Synchronize"),
				N_("Copy From Pilot"),
				N_("Copy To Pilot"),
				N_("Merge From Pilot"),
				N_("Merge To Pilot")};
#define SYNC_OPTIONS_COUNT 6

static void 
setSettings(ConduitCfg* conduitCfg)
{
	if(conduitCfg->sync_type!=GnomePilotConduitSyncTypeCustom)
		gnome_pilot_conduit_config_enable_with_first_sync(conduit_config,
								  conduitCfg->sync_type,
								  conduitCfg->sync_type,
								  TRUE);
	else
		gnome_pilot_conduit_config_disable(conduit_config);

	save_configuration(conduitCfg);
}

static void
doTrySettings(GtkWidget *widget, gpointer whatever)
{
	readStateCfg(cfgStateWindow,curState);
	readOptionsCfg(cfgOptionsWindow,curState);
	setSettings(curState);
}

static void
doSaveSettings(GtkWidget *widget, gpointer whatever)
{
	doTrySettings(widget,whatever);
}

static void
doCancelSettings(GtkWidget *widget, gpointer whatever)
{
	setSettings(origState);
}

static void
doRevertSettings(GtkWidget *widget, gpointer whatever)
{
	copy_configuration(curState,origState);
	setOptionsCfg(cfgOptionsWindow, curState);
	setStateCfg(cfgStateWindow,curState);
	setSettings(curState);
}


static void
insert_ignore_space (GtkEditable    *editable, const gchar    *text,
		     gint len, gint *position, void *data)
{
	gint i;
	gchar *curname;

	curname = gtk_entry_get_text(GTK_ENTRY(editable));
	if (*curname == '\0' && len > 0) {
		if (isspace(text[0])) {
			gtk_signal_emit_stop_by_name(GTK_OBJECT(editable), "insert_text");
			return;
		}
	} else { 
		for (i=0; i<len; i++) {
			if (isspace(text[i])) {
				gtk_signal_emit_stop_by_name(GTK_OBJECT(editable),"insert_text");
				return;
			}
		}
	}
}

static void
statechange_cb(GtkEditable *editable, const gchar *text,
	       gint  length, gint *position, void *data)
{
	if (!ignore_changes)
		capplet_widget_state_changed(CAPPLET_WIDGET(capplet), TRUE);
}

static void 
about_cb (GtkWidget *widget, gpointer data) 
{
	GtkWidget *about;
	const gchar *authors[] = {_("Vadim Strizhevsky <vadim@optonline.net>"),NULL};
  
	about = gnome_about_new(_("Gpilotd address conduit"), VERSION,
				_("(C) 1998 the Free Software Foundation"),
				authors,
				_("Configuration utility for the address conduit.\n"
				  "The address conduit syncronizes addresses from the "
				  "pilot with GnomeCard file on the local host"),
				_("gnome-unknown.xpm"));
	gtk_widget_show (about);
  
	return;
}

/* called by the sync_type GtkOptionMenu */
static void
sync_action_selection(GtkMenuShell *widget, gpointer unused) 
{
	if (!ignore_changes) {
		capplet_widget_state_changed(CAPPLET_WIDGET (capplet), TRUE);
	}
}

/* called by the sync_type GtkOptionMenu */
static void
activate_sync_type(GtkMenuItem *widget, gpointer data)
{
	curState->sync_type = GPOINTER_TO_INT(data);
	gtk_widget_set_sensitive(cfgOptionsWindow,curState->sync_type!=GnomePilotConduitSyncTypeCustom);
	if(!ignore_changes)
		capplet_widget_state_changed(CAPPLET_WIDGET(capplet), TRUE);
}

static GtkWidget
*createStateCfgWindow(void)
{
	GtkWidget *vbox, *table;
	GtkWidget *label;
	GtkWidget *optionMenu,*menuItem;
	GtkMenu   *menu;
	gint i;
	
	vbox = gtk_vbox_new(FALSE, GNOME_PAD);

	table =  gtk_hbox_new(FALSE, 0); 
	gtk_box_pack_start(GTK_BOX(vbox), table, FALSE, FALSE, GNOME_PAD);

	label = gtk_label_new(_("Synchronize Action"));
	gtk_box_pack_start(GTK_BOX(table), label, FALSE, FALSE, GNOME_PAD);    

	optionMenu=gtk_option_menu_new();
	gtk_object_set_data(GTK_OBJECT(vbox), "conduit_state", optionMenu);
	menu = GTK_MENU(gtk_menu_new());

	for (i=0; i<SYNC_OPTIONS_COUNT;i++) {
		sync_options[i]=_(sync_options[i]);
		menuItem = gtk_menu_item_new_with_label(sync_options[i]);
		gtk_widget_show(menuItem);
		gtk_signal_connect(GTK_OBJECT(menuItem),"activate",
				   GTK_SIGNAL_FUNC(activate_sync_type),
				   GINT_TO_POINTER(i));
		gtk_menu_append(menu,menuItem);
	}

	gtk_option_menu_set_menu(GTK_OPTION_MENU(optionMenu),GTK_WIDGET(menu));
	gtk_signal_connect(GTK_OBJECT(menu), "selection-done",
			   GTK_SIGNAL_FUNC(sync_action_selection),
			   NULL);
  
	gtk_box_pack_start(GTK_BOX(table), optionMenu, FALSE, FALSE, 0);    
	
	return vbox;
}

static void
setStateCfg(GtkWidget *widget,ConduitCfg *cfg)
{
	GtkOptionMenu *optionMenu;
	GtkMenu *menu;

	optionMenu = gtk_object_get_data(GTK_OBJECT(widget), "conduit_state");
	g_assert(optionMenu!=NULL);
	menu = GTK_MENU(gtk_option_menu_get_menu(optionMenu));

  
	ignore_changes = TRUE;
	/* Here were are relying on the items in menu being the same 
	   order as in GnomePilotConduitSyncType. */
	gtk_option_menu_set_history(optionMenu,(int)cfg->sync_type);
	gtk_widget_set_sensitive(cfgOptionsWindow,cfg->sync_type!=GnomePilotConduitSyncTypeCustom);
	ignore_changes = FALSE;
}


static void
readStateCfg(GtkWidget *widget,ConduitCfg *cfg)
{}

typedef struct _FieldInfo FieldInfo;
struct _FieldInfo
{
	gchar    *name;
	gchar    *label_data;
	gchar    *obj_data;
	gpointer  insert_func;
};


FieldInfo fields[] =
{ { N_("GnomeCard File:"),NULL,"filename",insert_ignore_space},
  { NULL,NULL,NULL}
};

static GtkWidget
*createCfgWindow(void)
{
	GtkWidget *vbox, *table;
	GtkWidget *entry, *label;
	int i,count=0;

	/* how many fields do we have */
	while(fields[count].name!=0) count++;

	vbox = gtk_vbox_new(FALSE, GNOME_PAD);

	table = gtk_table_new(count, 3, FALSE);
	gtk_table_set_row_spacings(GTK_TABLE(table), 4);
	gtk_table_set_col_spacings(GTK_TABLE(table), 10);
	gtk_box_pack_start(GTK_BOX(vbox), table, FALSE, FALSE, GNOME_PAD);

	for(i=0;i<count;i++) {
		label = gtk_label_new(_(fields[i].name));
		gtk_table_attach(GTK_TABLE(table), label, 1, 2, i, i+1, 0,0,0,0);
		if(fields[i].label_data!=NULL) {
			gtk_object_set_data(GTK_OBJECT(vbox), fields[i].label_data, label);
		}
		entry = gtk_entry_new_with_max_length(128);
		gtk_object_set_data(GTK_OBJECT(vbox), fields[i].obj_data, entry);
		gtk_table_attach_defaults(GTK_TABLE(table), entry, 2, 3, i, i+1); //, 0,0,0,0);
		gtk_signal_connect(GTK_OBJECT(entry), "insert_text",
				   GTK_SIGNAL_FUNC(fields[i].insert_func),
				   NULL);
		gtk_signal_connect_after(GTK_OBJECT(entry), "insert_text",
					 GTK_SIGNAL_FUNC(statechange_cb),
					 NULL);
		gtk_signal_connect_after(GTK_OBJECT(entry), "delete_text",
					 GTK_SIGNAL_FUNC(statechange_cb),
					 NULL);
	}
	
	return vbox;
}

static void
setOptionsCfg(GtkWidget *pilotcfg, ConduitCfg *state)
{
	GtkWidget *filename;
	filename  = gtk_object_get_data(GTK_OBJECT(pilotcfg), "filename");
	g_assert(filename!=NULL);

	ignore_changes = TRUE;
	gtk_entry_set_text(GTK_ENTRY(filename), state->filename);
	ignore_changes = FALSE;
}


static void
readOptionsCfg(GtkWidget *pilotcfg, ConduitCfg *state)
{
	GtkWidget *entry; 

	entry  = gtk_object_get_data(GTK_OBJECT(pilotcfg), "filename");
	if(state->filename) g_free(state->filename);
	state->filename = g_strdup(gtk_entry_get_text(GTK_ENTRY(entry)));

}

static void
pilot_capplet_setup(void)
{
	GtkWidget *frame, *box;

	capplet = capplet_widget_new();

	box =  gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(capplet), box); 

	frame = gtk_frame_new(_("Address Conduit State"));
	gtk_box_pack_start(GTK_BOX(box), frame, FALSE, FALSE,GNOME_PAD);
	
	cfgStateWindow = createStateCfgWindow();
	gtk_container_add(GTK_CONTAINER(frame), cfgStateWindow);

	frame = gtk_frame_new(_("Address Conduit Options"));
	gtk_box_pack_start(GTK_BOX(box), frame, FALSE, FALSE,GNOME_PAD);

	cfgOptionsWindow = createCfgWindow();
	gtk_container_add(GTK_CONTAINER(frame), cfgOptionsWindow);

	gtk_signal_connect(GTK_OBJECT(capplet), "try",
			   GTK_SIGNAL_FUNC(doTrySettings), NULL);
	gtk_signal_connect(GTK_OBJECT(capplet), "revert",
			   GTK_SIGNAL_FUNC(doRevertSettings), NULL);
	gtk_signal_connect(GTK_OBJECT(capplet), "ok",
			   GTK_SIGNAL_FUNC(doSaveSettings), NULL);
	gtk_signal_connect(GTK_OBJECT(capplet), "cancel",
			   GTK_SIGNAL_FUNC(doCancelSettings), NULL);
	gtk_signal_connect(GTK_OBJECT(capplet), "help",
			   GTK_SIGNAL_FUNC(about_cb), NULL);


	setStateCfg(cfgStateWindow,curState);
	setOptionsCfg(cfgOptionsWindow,curState);
    
	gtk_widget_show_all(capplet);
}

static void 
run_error_dialog(gchar *mesg,...) 
{
	char tmp[80];
	va_list ap;

	va_start(ap,mesg);
	vsnprintf(tmp,79,mesg,ap);
	dialogWindow = gnome_message_box_new(mesg,GNOME_MESSAGE_BOX_ERROR,GNOME_STOCK_BUTTON_OK,NULL);
	gnome_dialog_run_and_close(GNOME_DIALOG(dialogWindow));
	va_end(ap);
}


static gint 
get_pilot_id_from_gpilotd() 
{
	GList *pilots = NULL;
	gint pilot=0;
	int i,err;
  
	i=0;
	/* we don't worry about leaking here, so pilots isn't freed */
	switch(err = gnome_pilot_client_get_pilots(gpc,&pilots)) {
	case GPILOTD_OK: {
		if(pilots) {
			i=g_list_length(pilots);
			if(i==0) {
				run_error_dialog(_("No pilot configured, please choose the\n'Pilot Link Properties' capplet first."));
				return -1;
			} else {
				gnome_pilot_client_get_pilot_id_by_name(gpc,
									pilots->data, /* this is the first pilot */
									&pilot);
				if(i>1) {
					g_message("too many pilots...");
					/* need a choose here */
				}
				return pilot;
			}
		} else {
			run_error_dialog(_("No pilot configured, please choose the\n'Pilot Link Properties' capplet first."));
			return -1;
		}    
		break;
	}
	case GPILOTD_ERR_NOT_CONNECTED:
		run_error_dialog(_("Not connected to the gnome-pilot daemon"));
		return -1;
		break;
	default:
		g_warning("gnome_pilot_client_get_pilot_ids(...) = %d",err);
		run_error_dialog(_("An error occured when trying to fetch\npilot list from the gnome-pilot daemon"));
		return -1;
		break;
	}
}

int
main( int argc, char *argv[] )
{
	bindtextdomain (PACKAGE, GNOMELOCALEDIR);
	textdomain (PACKAGE);

	gnome_capplet_init ("calendar conduit control applet", NULL, argc, argv, 
			    NULL,
			    0, NULL);

    
	gpc = gnome_pilot_client_new();
	gnome_pilot_client_connect_to_daemon(gpc);
	pilotId = get_pilot_id_from_gpilotd();
	if(!pilotId) return -1;

	/* put all code to set things up in here */
	conduit = gnome_pilot_conduit_management_new("gnomecard",GNOME_PILOT_CONDUIT_MGMT_ID);
	if (conduit==NULL) {
		g_warning("Could not create object for conduit. The .conduit file may be missing.");
		run_error_dialog(_("Could not create object for conduit.\n"
				   "Perhaps the conduit isn't properly installed ?"));
		return -1;
	}
	conduit_config = gnome_pilot_conduit_config_new(conduit,pilotId);
	load_configuration(&origState,pilotId);
	gnome_pilot_conduit_config_is_enabled(conduit_config,&origState->sync_type);
	curState = dupe_configuration(origState);
    
	pilot_capplet_setup();
	
	/* done setting up, now run main loop */
	capplet_gtk_main();
    
	destroy_configuration(&origState);
	destroy_configuration(&curState);
    
	return 0;
}    
