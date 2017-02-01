/* Author: Tim P. Gerla <timg@rrv.net> */ 
#include <config.h>
#include <gnome.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <dirent.h>
#include <signal.h>
#include <errno.h>
#include <pwd.h>
#include <libgnomeui/gnome-window-icon.h>

#include "gtcd_public.h"
#include "cddb_props.h"
#include "prefs.h"

/* prototypes */
static void edit_cb(GtkWidget *widget, GtkWidget *clist);
static GtkWidget *create_local_db(void);
static gchar *get_file_name(gchar *append);
static gchar *get_dtitle(const gchar *filename);
static void select_row_cb(GtkCList *clist,
			  gint row,
			  gint column,
			  GdkEventButton *event,
			  GtkWidget *editbutton);
static void fill_list(GtkWidget *clist);
static void edit_destroy_cb(GtkWidget *widget, EditWindow *w);
static EditWindow *create_edit_window(GtkWidget *clist);
static void refresh_cb(GtkWidget *widget, GtkWidget *clist);
static void remove_cb(GtkWidget *widget, GtkWidget *clist);
static void msg_callback(gint reply, GtkCList *clist);
static void entry_cb(GtkWidget *widget, gpointer data);
static void port_cb(GtkObject *adj, gpointer data);
static void use_http_cb(GtkWidget *widget, GtkWidget *entry);
static void httpproxy_cb(GtkWidget *widget, gpointer data);
static void use_httpproxy_auth_cb(GtkWidget *widget, GtkWidget *entry);
static void use_httpproxy_auth_cb2(GtkWidget *widget, GtkWidget *entry);
static void use_httpproxy_auth_cb3(GtkWidget *widget, GtkWidget *entry);
static void httpproxyauthname_cb(GtkWidget *widget, gpointer data);
static void httpproxyauthpasswd_cb(GtkWidget *widget, gpointer data);
static void use_socks_cb(GtkWidget *widget, GtkWidget *entry);
static void set_socks_cb(GtkWidget *widget, GtkWidget *entry);

/* code */
static void select_row_cb(GtkCList *clist,
			  gint row,
			  gint column,
			  GdkEventButton *event,
			  GtkWidget *editbutton)
{
    static gchar *filename=NULL;
    gchar *tmp;

    gtk_widget_set_sensitive(editbutton, TRUE);

    if(filename)
	g_free(filename);

    gtk_clist_get_text(clist, row, 0, &tmp);
    filename = get_file_name(tmp);
    gtk_object_set_data(GTK_OBJECT(clist), "filename", filename);
    gtk_object_set_data(GTK_OBJECT(clist), "row", GINT_TO_POINTER(row));
}

static GtkWidget *create_local_db(void)
{
    gchar *titles[] = {N_("Disc ID"), N_("Disc Title")};
    GtkWidget *clist, *refresh_button, *edit_button, *hsep;
    GtkWidget *hbox, *bbox, *scw, *remove_button;

#ifdef ENABLE_NLS
    titles[0] = _(titles[0]);
    titles[1] = _(titles[1]);
#endif /* ENABLE_NLS */
    
    hbox = gtk_hbox_new(FALSE, GNOME_PAD_SMALL);

    /* buttons */
    bbox = gtk_vbutton_box_new();
    gtk_button_box_set_layout(GTK_BUTTON_BOX(bbox), GTK_BUTTONBOX_START);

    edit_button = gtk_button_new_with_label(_("Edit"));
    gtk_widget_set_sensitive(edit_button, FALSE);
    gtk_box_pack_start_defaults(GTK_BOX(bbox), edit_button);

    remove_button = gtk_button_new_with_label(_("Remove"));
    gtk_box_pack_start_defaults(GTK_BOX(bbox), remove_button);

    hsep = gtk_hseparator_new();
    gtk_box_pack_start_defaults(GTK_BOX(bbox), hsep);

    refresh_button = gtk_button_new_with_label(_("Refresh List"));
    gtk_box_pack_start_defaults(GTK_BOX(bbox), refresh_button);

    gtk_box_pack_end(GTK_BOX(hbox), bbox, FALSE, FALSE, GNOME_PAD_SMALL);

    /* clist */
    clist = gtk_clist_new_with_titles(2, titles);
    gtk_clist_column_titles_show(GTK_CLIST(clist));
    gtk_clist_set_selection_mode(GTK_CLIST(clist), 
                                 GTK_SELECTION_BROWSE); 
    gtk_clist_column_titles_passive(GTK_CLIST(clist));
    gtk_clist_set_column_auto_resize(GTK_CLIST(clist), 0, TRUE);
    gtk_clist_set_column_auto_resize(GTK_CLIST(clist), 1, TRUE);
    gtk_clist_set_sort_column(GTK_CLIST(clist), 1);
    gtk_clist_set_auto_sort(GTK_CLIST(clist), TRUE);

    scw = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scw),
                                   GTK_POLICY_AUTOMATIC,
                                   GTK_POLICY_AUTOMATIC);
    gtk_clist_set_column_width(GTK_CLIST(clist), 0, 64);

    gtk_container_add(GTK_CONTAINER(scw), clist);
    gtk_box_pack_start(GTK_BOX(hbox), scw, TRUE, TRUE, GNOME_PAD_SMALL);
    gtk_signal_connect(GTK_OBJECT(clist), "select_row",
		       GTK_SIGNAL_FUNC(select_row_cb), edit_button);
    fill_list(clist);
    
    /* now that we have a clist to pass... */
    gtk_signal_connect(GTK_OBJECT(edit_button), "clicked",
		       GTK_SIGNAL_FUNC(edit_cb), clist);
    gtk_signal_connect(GTK_OBJECT(refresh_button), "clicked",
		       GTK_SIGNAL_FUNC(refresh_cb), clist);
    gtk_signal_connect(GTK_OBJECT(remove_button), "clicked",
		       GTK_SIGNAL_FUNC(remove_cb), clist);

    gtk_widget_show_all(hbox);
    return hbox;
}

static GtkWidget *portw;

GtkWidget *create_cddb_page(void)
{
    GtkWidget *vbox, *frame, *label, *cbutton, *pbutton, *sbutton;
    GtkWidget *table, *entry, *socksentry;
    GtkObject *adj;

    vbox = gtk_vbox_new(FALSE, GNOME_PAD_SMALL);

    /* server settings */
    frame = gtk_frame_new(_("Server Settings"));
    gtk_container_border_width(GTK_CONTAINER(frame), GNOME_PAD_SMALL);

    gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 0);

    table = gtk_table_new(5, 4, FALSE);
    gtk_table_set_row_spacings(GTK_TABLE(table), GNOME_PAD_SMALL);
    gtk_table_set_col_spacings(GTK_TABLE(table), GNOME_PAD_SMALL);

    label = gtk_label_new(_("Address"));
    entry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(entry), prefs->cddb_server);
    gtk_table_attach_defaults(GTK_TABLE(table), label, 0, 1, 0, 1);
    gtk_table_attach_defaults(GTK_TABLE(table), entry, 1, 3, 0, 1);
    gtk_signal_connect(GTK_OBJECT(entry), "changed",
		       GTK_SIGNAL_FUNC(entry_cb), NULL);

    adj = gtk_adjustment_new(8880, 0, 65536, 1, 100, 10);
    gtk_adjustment_set_value(GTK_ADJUSTMENT(adj), prefs->cddb_port);
    label = gtk_label_new(_("Port"));
    portw = gtk_spin_button_new(GTK_ADJUSTMENT(adj), 1, 0);
    gtk_table_attach_defaults(GTK_TABLE(table), label, 3, 4, 0, 1);
    gtk_table_attach_defaults(GTK_TABLE(table), portw, 4, 5, 0, 1);
    gtk_signal_connect(GTK_OBJECT(adj), "value_changed",
		       GTK_SIGNAL_FUNC(port_cb), NULL);
    gtk_signal_connect(GTK_OBJECT(adj), "changed",
		       GTK_SIGNAL_FUNC(port_cb), NULL);

    cbutton = gtk_check_button_new_with_label(_("Use HTTP"));
    gtk_object_set_data(GTK_OBJECT(cbutton), "server-entry", entry);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(cbutton), prefs->cddb_http);
    gtk_table_attach_defaults(GTK_TABLE(table), cbutton, 0,1, 1,2);

    label = gtk_label_new(_("Proxy"));
    gtk_table_attach_defaults(GTK_TABLE(table), label, 1,2, 1,2);
    entry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(entry), prefs->cddb_httpproxy);
    gtk_table_attach_defaults(GTK_TABLE(table), entry, 2,5, 1,2);
    if (!prefs->cddb_http) gtk_widget_set_sensitive(entry, FALSE);
    gtk_signal_connect(GTK_OBJECT(entry), "changed",
		       GTK_SIGNAL_FUNC(httpproxy_cb), NULL);

#ifndef WITH_LIBGHTTP
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(cbutton), FALSE);
    gtk_widget_set_sensitive(cbutton, FALSE);
    gtk_widget_set_sensitive(entry, FALSE);
#endif
	      
    gtk_signal_connect(GTK_OBJECT(cbutton), "toggled",
		       GTK_SIGNAL_FUNC(use_http_cb), entry);

    pbutton = gtk_check_button_new_with_label(_("Use proxy authentication"));
    gtk_object_set_data(GTK_OBJECT(cbutton), "server-entry", entry);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pbutton), prefs->cddb_httpproxy_need_auth);
    gtk_table_attach_defaults(GTK_TABLE(table), pbutton, 0,1, 2,3);
    if (!prefs->cddb_http) gtk_widget_set_sensitive(pbutton, FALSE);
    gtk_signal_connect(GTK_OBJECT(cbutton), "toggled",
		       GTK_SIGNAL_FUNC(use_httpproxy_auth_cb2), pbutton);

    label = gtk_label_new(_("Username"));
    gtk_table_attach_defaults(GTK_TABLE(table), label, 1,2, 2,3);
    entry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(entry), prefs->cddb_httpproxy_auth_name);
    gtk_table_attach_defaults(GTK_TABLE(table), entry, 2,3, 2,3);
    if (!prefs->cddb_http || !prefs->cddb_httpproxy_need_auth)
        gtk_widget_set_sensitive(entry, FALSE);
    gtk_signal_connect(GTK_OBJECT(entry), "changed",
		       GTK_SIGNAL_FUNC(httpproxyauthname_cb), NULL);

#ifndef WITH_LIBGHTTP
    gtk_widget_set_sensitive(pbutton, FALSE);
    gtk_widget_set_sensitive(entry, FALSE);
#endif
	      
    gtk_signal_connect(GTK_OBJECT(pbutton), "toggled",
		       GTK_SIGNAL_FUNC(use_httpproxy_auth_cb), entry);
    gtk_signal_connect(GTK_OBJECT(cbutton), "toggled",
		       GTK_SIGNAL_FUNC(use_httpproxy_auth_cb3), entry);

    label = gtk_label_new(_("Password"));
    gtk_table_attach_defaults(GTK_TABLE(table), label, 1,2, 3,4);
    entry = gtk_entry_new();
    gtk_entry_set_visibility(GTK_ENTRY(entry), FALSE);
    gtk_entry_set_text(GTK_ENTRY(entry), prefs->cddb_httpproxy_auth_passwd);
    gtk_table_attach_defaults(GTK_TABLE(table), entry, 2,3, 3,4);
    if (!prefs->cddb_http || !prefs->cddb_httpproxy_need_auth)
        gtk_widget_set_sensitive(entry, FALSE);
    gtk_signal_connect(GTK_OBJECT(entry), "changed",
		       GTK_SIGNAL_FUNC(httpproxyauthpasswd_cb), NULL);
    
#ifndef WITH_LIBGHTTP
    gtk_widget_set_sensitive(entry, FALSE);
#endif

    gtk_signal_connect(GTK_OBJECT(pbutton), "toggled",
		       GTK_SIGNAL_FUNC(use_httpproxy_auth_cb), entry);
    gtk_signal_connect(GTK_OBJECT(cbutton), "toggled",
		       GTK_SIGNAL_FUNC(use_httpproxy_auth_cb3), entry);

    label = gtk_label_new(_("SOCKS Server"));
    gtk_table_attach_defaults(GTK_TABLE(table), label, 1,2, 4,5);

    socksentry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(socksentry), prefs->socks_server);
    gtk_table_attach_defaults(GTK_TABLE(table), socksentry, 2,5, 4,5);

    sbutton = gtk_check_button_new_with_label(_("Use SOCKS"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(sbutton), prefs->use_socks);
    gtk_table_attach_defaults(GTK_TABLE(table), sbutton, 0,1, 4,5);

    if (!prefs->use_socks) gtk_widget_set_sensitive(socksentry, FALSE);
    gtk_signal_connect(GTK_OBJECT(socksentry), "changed",
		       GTK_SIGNAL_FUNC(set_socks_cb), socksentry);
    gtk_signal_connect(GTK_OBJECT(sbutton), "toggled",
		       GTK_SIGNAL_FUNC(use_socks_cb), socksentry);

    gtk_container_add(GTK_CONTAINER(frame), table);

    /* local db */
    frame = gtk_frame_new(_("Local Database"));
    gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_NONE);
    gtk_container_border_width(GTK_CONTAINER(frame), GNOME_PAD_SMALL);

    gtk_box_pack_start(GTK_BOX(vbox), frame, TRUE, TRUE, 0);
    
    gtk_container_add(GTK_CONTAINER(frame), create_local_db());

    return vbox;
}

/* EDIT STUFF */
static void edit_destroy_cb(GtkWidget *widget, EditWindow *w)
{
    if(strncmp(gtk_object_get_data(GTK_OBJECT(widget), "save"), "y", 1) == 0)
	gnome_less_write_file(GNOME_LESS(w->gl), w->filename);

    gtk_widget_destroy(w->window);
    g_free(w);
}

static void edit_cb(GtkWidget *widget, GtkWidget *clist)
{
    EditWindow *w;

    w = create_edit_window(clist);
}

static EditWindow *create_edit_window(GtkWidget *clist)
{
    EditWindow *w;
    GtkWidget *frame;
    GtkWidget *button;
    GtkWidget *vbox, *bbox;

    w = g_new0(EditWindow, 1);

    /* vertical box */
    vbox = gtk_vbox_new(FALSE, GNOME_PAD_SMALL);
    gtk_container_border_width(GTK_CONTAINER(vbox), GNOME_PAD_SMALL);
    
    /* frame */
    frame = gtk_frame_new(_("CDDB Data"));
    gtk_box_pack_start(GTK_BOX(vbox), frame, TRUE, TRUE, 0);

    /* TEXT WINDOW */
 
    /* gnome-less widget */
    w->gl = gnome_less_new();
    w->filename = gtk_object_get_data(GTK_OBJECT(clist), "filename");
    gnome_less_show_file(GNOME_LESS(w->gl), w->filename);
    gtk_text_set_editable(GNOME_LESS(w->gl)->text, TRUE);
    
    gtk_container_add(GTK_CONTAINER(frame), w->gl);

    /* BUTTONS */
    bbox = gtk_hbutton_box_new();
    gtk_button_box_set_layout(GTK_BUTTON_BOX(bbox), GTK_BUTTONBOX_SPREAD);

    button = gnome_stock_button(GNOME_STOCK_BUTTON_OK);
    gtk_object_set_data(GTK_OBJECT(button), "save", "y");
    gtk_box_pack_start_defaults(GTK_BOX(bbox), button);
    gtk_signal_connect(GTK_OBJECT(button), "clicked",
      		       GTK_SIGNAL_FUNC(edit_destroy_cb), w);
    button = gnome_stock_button(GNOME_STOCK_BUTTON_CANCEL);
    gtk_object_set_data(GTK_OBJECT(button), "save", "n");
    gtk_box_pack_start_defaults(GTK_BOX(bbox), button);
    gtk_signal_connect(GTK_OBJECT(button), "clicked",
      		       GTK_SIGNAL_FUNC(edit_destroy_cb), w);
    gtk_box_pack_start(GTK_BOX(vbox), bbox, FALSE, FALSE, 0);

    /* main window */
    w->window = gtk_window_new(GTK_WINDOW_DIALOG);
    gnome_window_icon_set_from_default (GTK_WINDOW (w->window));
    gtk_container_border_width(GTK_CONTAINER(w->window), GNOME_PAD_SMALL);
    gtk_window_set_title(GTK_WINDOW(w->window), _("CDDB Editor"));
    gtk_window_set_wmclass(GTK_WINDOW(w->window), "cddb_editor","gtcd");   
    gtk_signal_connect(GTK_OBJECT(w->window), "destroy_event",
		       GTK_SIGNAL_FUNC(edit_destroy_cb), w);

    gtk_container_add(GTK_CONTAINER(w->window), vbox);
    gtk_widget_show_all(w->window);

    return w;
}

/* fill a clist with cddb entries */
static void fill_list(GtkWidget *clist)
{
    gchar *error_item[] = {"0", N_("Error reading $HOME/.cddbslave.")};
    char *dname;
    gchar *tmp[2];
    DIR *d;
    struct dirent *de;

#ifdef ENABLE_NLS
    error_item[1] = _(error_item[1]);
#endif /* ENABLE_NLS */

    dname = get_file_name("");
    if(!dname)
	goto error;

    d = opendir(dname);
    if(!d)
	goto error;

    gtk_clist_freeze(GTK_CLIST(clist));
    while((de = readdir(d)))
    {
	gchar *filename;

	if(strlen(de->d_name) != 8) /* erm, sort of ugly hack but it should work just fine :) */
	    continue;

        filename = get_file_name(de->d_name);
	tmp[0] = de->d_name;
	tmp[1] = get_dtitle(filename);

	gtk_clist_append(GTK_CLIST(clist), tmp);

	g_free(filename);
	g_free(tmp[1]);
    }
    gtk_clist_thaw(GTK_CLIST(clist));
    
    closedir(d);
    g_free(dname);
    return;
 error:;
    gtk_clist_append(GTK_CLIST(clist), error_item);
    g_free(dname);
    return;
}

/* return a path formatted like: $HOME/.cddbslave/append */
static gchar *get_file_name(gchar *append)
{
    char *fname;
    char *homedir=NULL;
    struct passwd *pw=NULL;

    homedir = getenv("HOME");

    if(homedir == NULL)
    {
        pw = getpwuid(getuid());

        if(pw != NULL)
            homedir = pw->pw_dir;
        else
            homedir = "/";
    }

    fname = g_malloc(512);

    g_snprintf(fname, 511, "%s/.cddbslave/%s", homedir, append);
    return fname;
}

/* open a cddb file in specified dir, and parse out the dtitle. */
static gchar *get_dtitle(const gchar *filename)
{
    FILE *fp;
    char string[256];

    if(!(fp = fopen(filename, "r")))
    {
	perror("fopen");
	return g_strdup("Cannot open file");
    }
    while(fgets(string, 255, fp)!=NULL)
    {
	string[strlen(string)-1] = 0;
	
	if( strncmp( string, "DTITLE", 6 ) == 0)
	{
	    fclose(fp);
	    return g_strdup(string+7);
	}
    }
    fclose(fp);
    return g_strdup("Invalid CDDB File");  
}

static void refresh_cb(GtkWidget *widget, GtkWidget *clist)
{
    gtk_clist_clear(GTK_CLIST(clist));

    fill_list(clist);
}

static void msg_callback(gint reply, GtkCList *clist)
{
    gchar *filename;
    gint row;

    filename = gtk_object_get_data(GTK_OBJECT(clist), "filename");
    row = GPOINTER_TO_INT(gtk_object_get_data(GTK_OBJECT(clist), "row"));

    if(reply == 0)
    {
	if(remove(filename))
	{
	    gchar tmp[256];
	    g_snprintf(tmp, 255, _("Couldn't remove file: %s\n"), strerror(errno));
	    gtk_widget_show(gnome_error_dialog(tmp));
	}
	else
	    gtk_clist_remove(clist, row);
    }
}

static void remove_cb(GtkWidget *widget, GtkWidget *clist)
{
    GtkWidget *msg;

    msg = gnome_question_dialog(_("Delete CDDB Entry?"),
				(GnomeReplyCallback)msg_callback,
				clist);
    gtk_widget_show(msg);
}

static void entry_cb(GtkWidget *widget, gpointer data)
{
    prefs->cddb_server = g_strdup(gtk_entry_get_text(GTK_ENTRY(widget)));
    changed_cb(NULL, NULL);
}

static void port_cb(GtkObject *adj, gpointer data)
{
    prefs->cddb_port = (gint)GTK_ADJUSTMENT(adj)->value;
    changed_cb(NULL, NULL);
}

static void use_http_cb(GtkWidget *widget, GtkWidget *entry) {
    gint val;

    prefs->cddb_http = GTK_TOGGLE_BUTTON(widget)->active;
    gtk_widget_set_sensitive(entry, prefs->cddb_http);
    changed_cb(NULL, NULL);

    val = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(portw));
    /* if the port number is set at the default for the opposite setting of
     * use_http, set it to the default for this setting.  When switching off
     * http usage, try to guess the cddbp port.  Freedb uses 888, while
     * cddb.com uses 8880 */
    if (prefs->cddb_http && (val == 8880 || val == 888))
      gtk_spin_button_set_value(GTK_SPIN_BUTTON(portw), 80);
    if (!prefs->cddb_http && val == 80) {
      gchar *text = gtk_entry_get_text(GTK_ENTRY(gtk_object_get_data(
				GTK_OBJECT(widget), "server-entry")));
      if (!g_strcasecmp((text + strlen(text) - 10), "freedb.org"))
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(portw), 888);
      else
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(portw), 8880);
    }
}
static void httpproxy_cb(GtkWidget *widget, gpointer data) {
    prefs->cddb_httpproxy = g_strdup(gtk_entry_get_text(GTK_ENTRY(widget)));
    changed_cb(NULL, NULL);
}
static void set_socks_cb(GtkWidget *widget, GtkWidget *entry) {
    prefs->socks_server = g_strdup(gtk_entry_get_text(GTK_ENTRY(entry)));
    changed_cb(NULL, NULL);
}
static void use_socks_cb(GtkWidget *widget, GtkWidget *entry) {
    prefs->use_socks = GTK_TOGGLE_BUTTON(widget)->active;
    gtk_widget_set_sensitive(entry, prefs->use_socks);
    changed_cb(NULL, NULL);
}
static void use_httpproxy_auth_cb(GtkWidget *widget, GtkWidget *entry) {

    prefs->cddb_httpproxy_need_auth = GTK_TOGGLE_BUTTON(widget)->active;
    gtk_widget_set_sensitive(entry, prefs->cddb_httpproxy_need_auth);
    changed_cb(NULL, NULL);
}
static void use_httpproxy_auth_cb2(GtkWidget *widget, GtkWidget *entry) {

    gtk_widget_set_sensitive(entry, prefs->cddb_http);
    changed_cb(NULL, NULL);
}
static void use_httpproxy_auth_cb3(GtkWidget *widget, GtkWidget *entry) {

    gtk_widget_set_sensitive(entry, prefs->cddb_http && prefs->cddb_httpproxy_need_auth);
    changed_cb(NULL, NULL);
}
static void httpproxyauthname_cb(GtkWidget *widget, gpointer data) {
    prefs->cddb_httpproxy_auth_name = g_strdup(gtk_entry_get_text(GTK_ENTRY(widget)));
    changed_cb(NULL, NULL);
}
static void httpproxyauthpasswd_cb(GtkWidget *widget, gpointer data) {
    prefs->cddb_httpproxy_auth_passwd = g_strdup(gtk_entry_get_text(GTK_ENTRY(widget)));
    changed_cb(NULL, NULL);
}

