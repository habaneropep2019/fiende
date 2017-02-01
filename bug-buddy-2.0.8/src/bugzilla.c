/* bug-buddy bug submitting program
 *
 * Copyright (C) 2001 Jacob Berkman
 * Copyright 2001 Ximian, Inc.
 *
 * Author:  jacob berkman  <jacob@bug-buddy.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 */

#include <config.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <utime.h>
#include <errno.h>

#include <gnome.h>

#include <libgnomevfs/gnome-vfs.h>

#include "bug-buddy.h"
#include "libglade-buddy.h"
#include "util.h"
#include <dirent.h>

#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xmlmemory.h>

/* define to x for some debugging output */
#define d(x)

static int
prod_cmp (BugzillaProduct *a, BugzillaProduct *b)
{
	return strcasecmp (a->name, b->name);
}

static int
comp_cmp (BugzillaComponent *a, BugzillaComponent *b)
{
	return strcasecmp (a->name, b->name);
}

static void
bugzilla_bts_insert_product (BugzillaBTS *bts, BugzillaProduct *prod)
{
	bts->products = g_slist_insert_sorted (bts->products, prod, (gpointer)prod_cmp);
}

static void
bugzilla_product_insert_component (BugzillaProduct *prod, BugzillaComponent *comp)
{
	prod->components = g_slist_insert_sorted (prod->components, comp, (gpointer)comp_cmp);
}

static void
load_config_xml (BugzillaBTS *bts, xmlDoc *doc)
{
	xmlNode *node, *cur;
	char *s;

	for (node = doc->root->childs; node; node = node->next) {
		if (!strcmp (node->name, bts->severity_node)) {
			for (cur = node->childs; cur; cur = cur->next) {
				if (strcmp (cur->name, bts->severity_item))
					continue;
				s = xmlNodeGetContent (cur);
				bts->severities = g_slist_append (bts->severities,
								  g_strdup (s));
				xmlFree (s);
			}
		} else if (!strcmp (node->name, "opsys_list")) {
			for (cur = node->childs; cur; cur = cur->next) {
				if (strcmp (cur->name, "opsys"))
					continue;
				s = xmlNodeGetContent (cur);
				bts->opsys = g_slist_append (bts->opsys,
							     g_strdup (s));
				xmlFree (s);
			}
		}
	}

	xmlFreeDoc (doc);
}

static void
load_products_xml (BugzillaBTS *bts, xmlDoc *doc)
{
	BugzillaProduct *prod;
	BugzillaComponent *comp;
	xmlNode *node, *cur;
	char *s;

	for (node = doc->root->childs; node; node = node->next) {
		if (!strcmp (node->name, "product")) {
			prod = g_new0 (BugzillaProduct, 1);
			prod->bts = bts;

			s = xmlGetProp (node, "name");
			prod->name = g_strdup (s);
			xmlFree (s);

			s = xmlGetProp (node, "description");
			prod->description = g_strdup (s);
			xmlFree (s);

			bugzilla_bts_insert_product (bts, prod);
			bugzilla_bts_insert_product (druid_data.all_products, prod);

			for (cur = node->childs; cur; cur = cur->next) {
				if (strcmp (cur->name, "component"))
					continue;
				
				comp = g_new0 (BugzillaComponent, 1);
				comp->product = prod;
				
				s = xmlGetProp (cur, "value");
				comp->name = g_strdup (s);
				xmlFree (s);

				s = xmlGetProp (cur, "description");
				comp->description = g_strdup (s);
				xmlFree (s);

				bugzilla_product_insert_component (prod, comp);
			}
		}
	}

	xmlFreeDoc (doc);
}

#if 0
static void
print_comp (gpointer data, gpointer user_data)
{
	BugzillaComponent *comp = data;

	g_print ("\t\t%s (%s)\n", comp->name, comp->description);
}

static void
print_product (gpointer data, gpointer user_data)
{
	BugzillaProduct *prod = data;

	g_print ("\t%s (%s)\n", prod->name, prod->description);

	g_slist_foreach (prod->components, print_comp, NULL);
}

static void
print_item (gpointer data, gpointer user_data)
{
	g_print ("\t%s\n", (char *)data);
}
#endif

static void
goto_product_page (void)
{
	druid_set_sensitive (TRUE, TRUE, TRUE);
	druid_set_state (STATE_PRODUCT);
}

static int
async_update (GnomeVFSAsyncHandle *handle, GnomeVFSXferProgressInfo *info, gpointer data)
{
	d(g_print ("%lu\n", info->bytes_copied));
	if (info->source_name) {
		d(g_print ("source: %s\n", info->source_name));
		gtk_label_set_text (GTK_LABEL (GET_WIDGET ("progress-source")), info->source_name);
	}

	if (info->target_name) {
		d(g_print ("target: %s\n", info->target_name));
		gtk_label_set_text (GTK_LABEL (GET_WIDGET ("progress-dest")), info->target_name);
	}

	if (info->bytes_total)
		gtk_progress_set_percentage (GTK_PROGRESS (GET_WIDGET ("progress-progress")), (gfloat)info->total_bytes_copied / info->bytes_total);

	if (info->phase == GNOME_VFS_XFER_PHASE_COMPLETED) {
		GList *li;
		char *remote, *local;

		/* until gnome-vfs will touch the file for us */
		for (li = druid_data.dldests; li; li=li->next) {
			remote = gnome_vfs_uri_to_string (li->data, GNOME_VFS_URI_HIDE_NONE);
			local = gnome_vfs_get_local_path_from_uri (remote);
			g_free (remote);
			if (!local)
				continue;
			d(g_print ("touching: %s\n", local));
			utime (local, NULL);
			g_free (local);
		}

		goto_product_page ();
		d(g_print ("w00t!\n"));
		return FALSE;
	}

	return TRUE;
}

/**
 * e_mkdir_hier:
 * @path: a directory path
 * @mode: a mode, as for mkdir(2)
 *
 * This creates the named directory with the given @mode, creating
 * any necessary intermediate directories (with the same @mode).
 *
 * Return value: 0 on success, -1 on error, in which case errno will
 * be set as for mkdir(2).
 **/
static int
e_mkdir_hier(const char *path, mode_t mode)
{
        char *copy, *p;

        p = copy = g_strdup (path);
        do {
                p = strchr (p + 1, '/');
                if (p)
                        *p = '\0';
                if (access (copy, F_OK) == -1) {
                        if (mkdir (copy, mode) == -1) {
                                g_free (copy);
                                return -1;
                        }
                }
                if (p)
                        *p = '/';
        } while (p);

        g_free (copy);
        return 0;
}

/* 
 * this is probably the scariest function in bug-buddy.
 *
 * here is what it is supposed to do:
 *
 * we should download a new file into our local cache if:
 *
 *  * the cached file doesn't exist
 *  * the cached file is older than the system one
 *  * the cached file is older than a week
 *  * the cached file isn't XML
 *
 */

static BugzillaXMLFile *
get_xml_file (BugzillaBTS *bts, const char *key, XMLFunc parse_func)
{
	BugzillaXMLFile *xmlfile;
	char *localdir, *tmppath, *src_uri;
	char *err;
	struct stat sys_stat, local_stat;
	gboolean sys_is_newer, cache_is_old;
	xmlDoc *doc;

	src_uri = gnome_config_get_string (key);
	if (!src_uri) {
		d(g_warning ("could not read: %s\n", key));
		return NULL;
	}

	xmlfile = g_new0 (BugzillaXMLFile, 1);
	xmlfile->xml_func = parse_func;

	xmlfile->system_path = g_strdup_printf (BUDDY_DATADIR "/bugzilla/%s/%s", bts->subdir, key);
	tmppath = gnome_util_home_file ("bug-buddy.d/bugzilla/");
	localdir = g_concat_dir_and_file (tmppath, bts->subdir);
	xmlfile->cache_path = g_concat_dir_and_file (localdir, key);

	g_free (tmppath);

	if (stat (xmlfile->cache_path, &local_stat)) {
		err = g_strerror (errno);
		d(g_message ("could not stat local file: `%s': %s\n", xmlfile->cache_path, err));
		if (e_mkdir_hier (localdir, S_IRWXU)) {
			err = g_strerror (errno);
			d(g_warning ("could not create local dir: `%s': %s\n", localdir, err));
			g_free (localdir);			
			return xmlfile;
		}
		g_free (localdir);
		goto append_uris;
	}

	g_free (localdir);

	if (stat (xmlfile->system_path, &sys_stat)) {
		err = g_strerror (errno);
		d(g_warning ("could not stat sys file: `%s': %s\n", xmlfile->system_path, err));
		goto append_uris;
	}
       
#define A_DAY (24 * 60 * 60)

	sys_is_newer = sys_stat.st_mtime > local_stat.st_mtime;
	cache_is_old = (time (NULL) - local_stat.st_mtime) > (7 * A_DAY);


	d(g_print ("sys_is_newer (%d): %d - %d = %d (%f)\n",
		   sys_is_newer, sys_stat.st_mtime, local_stat.st_mtime,
		   sys_stat.st_mtime - local_stat.st_mtime,
		   (sys_stat.st_mtime - local_stat.st_mtime) / (float)A_DAY));

	d(g_print ("cache_is_old (%d): %d - %d = %d (%f)\n",
		   cache_is_old, time (NULL), local_stat.st_mtime,
		   time (NULL) - local_stat.st_mtime,
		   (time (NULL) - local_stat.st_mtime) / (float)A_DAY));

	if (sys_is_newer || cache_is_old)
		goto append_uris;

	doc = xmlParseFile (xmlfile->cache_path);
	if (!doc)
		goto append_uris;
	
	parse_func (bts, doc);
		
	xmlfile->done = TRUE;
	
	return xmlfile;

 append_uris:

	d(g_print ("wanting to save: %s\nto %s\n", src_uri, xmlfile->cache_path));

	xmlfile->source_uri = gnome_vfs_uri_new (src_uri);
	g_free (src_uri);

	if (!xmlfile->source_uri) {
		d(g_error ("could not parse source"));
		return xmlfile;
	}
		
	xmlfile->dest_uri = gnome_vfs_uri_new (xmlfile->cache_path);
	if (!xmlfile->dest_uri) {
		gnome_vfs_uri_unref (xmlfile->source_uri);
		xmlfile->source_uri = NULL;
		d(g_error ("could not parse dest"));
		return xmlfile;
	}

	druid_data.dlsources = g_list_prepend (druid_data.dlsources, xmlfile->source_uri);
	druid_data.dldests = g_list_prepend (druid_data.dldests, xmlfile->dest_uri);

	xmlfile->download = xmlfile->read_from_cache = TRUE;
	return xmlfile;
}

static BugzillaBTS *
load_bugzilla (const char *filename)
{
	BugzillaBTS *bts;
	char *path, *pixmap;
	gboolean def;
	GdkPixbuf *pb;
	char *s;
	
	d(g_print ("loading `%s'...\n", filename));

	bts = g_new0 (BugzillaBTS, 1);

	path = g_strdup_printf ("=%s=/Bugzilla/", filename);

	gnome_config_push_prefix (path);

	bts->name = gnome_config_get_string ("name");

	if (!bts->name) {
		g_free (bts);
		return NULL;
	}

	bts->subdir = gnome_config_get_string ("subdir");
	if (!bts->subdir)
		bts->subdir = g_strdup (bts->name);

	pixmap = gnome_config_get_string_with_default ("icon="BUDDY_ICONDIR"/bug-buddy.png", &def);

	if (pixmap[0] == '/')
		bts->icon = g_strdup (pixmap);
	else
		bts->icon = g_concat_dir_and_file (BUDDY_DATADIR, pixmap);
	g_free (pixmap);

	s = gnome_config_get_string ("submit_type=freitag");
	if (!strcasecmp (s, "freitag"))
		bts->submit_type = BUGZILLA_SUBMIT_FREITAG;
	else if (!strcasecmp (s, "debian"))
		bts->submit_type = BUGZILLA_SUBMIT_DEBIAN;
	g_free (s);
	
	d(g_print ("icon: %s\n", bts->icon));

	bts->email = gnome_config_get_string ("email");

	pb = gdk_pixbuf_new_from_file (bts->icon);
	if (pb) {
		GdkPixbuf *pb2 = gdk_pixbuf_scale_simple (pb, CLIST_HEIGHT, CLIST_HEIGHT, GDK_INTERP_BILINEAR);
		gdk_pixbuf_render_pixmap_and_mask (pb2, &bts->pixmap, &bts->mask, 127);
		gdk_pixbuf_unref (pb);
		gdk_pixbuf_unref (pb2);
	}

	bts->severity_node   = gnome_config_get_string ("severity_node=severities");
	bts->severity_item   = gnome_config_get_string ("severity_item=severity");
	bts->severity_header = gnome_config_get_string ("severity_header=Severity");

	bts->products_xml = get_xml_file (bts, "products", load_products_xml);
	bts->config_xml   = get_xml_file (bts, "config",   load_config_xml);

	gnome_config_pop_prefix ();

	if ((bts->products_xml && bts->products_xml->download) ||
	    (bts->config_xml   && bts->config_xml->download))
		druid_data.need_to_download = TRUE;

	return bts;
}

static int
start_xfer (gpointer null)
{
	/*gtk_widget_show (GET_WIDGET ("progress-cancel"));*/

	while (gtk_events_pending ())
		gtk_main_iteration ();

	if (GNOME_VFS_OK != gnome_vfs_async_xfer (	    
		    &druid_data.vfshandle,
		    druid_data.dlsources,
		    druid_data.dldests,
		    GNOME_VFS_XFER_DEFAULT,
		    GNOME_VFS_XFER_ERROR_MODE_ABORT,
		    GNOME_VFS_XFER_OVERWRITE_MODE_REPLACE,
		    async_update, NULL, NULL, NULL))
		goto_product_page ();

	return FALSE;
}

#if 0
/* 
 * GNOME VFS has no way to pause the progress, which is what we want to do, really 
 * so just let it cancel for now 
 */
static gint
check_yoself (void)
{
	GtkWidget *m;

	m = gnome_question_dialog (
		_("Are you sure you want to cancel this update?"), NULL, NULL);
	
	if (gnome_dialog_run_and_close (GNOME_DIALOG (m))) {
		gnome_vfs_async_cancel (druid_data.vfshandle);
		goto_product_page ();
		return TRUE;
	}
	return FALSE;
}
#endif

void
on_progress_cancel_clicked (GtkWidget *w, gpointer data)
{
	d(g_print ("clicked!!\n"));
	gnome_vfs_async_cancel (druid_data.vfshandle);
	d(g_print ("shaggy?\n"));
	goto_product_page ();
	d(g_print ("scooby dooby doo!\n"));
}

static void
download_stuff (void)
{
	gtk_idle_add (start_xfer, NULL);
	gtk_main ();
	/*gtk_widget_hide (GET_WIDGET ("progress-cancel"));*/
}

static void
show_products (GtkWidget *w, gpointer data)
{
	bugzilla_bts_add_products_to_clist ((BugzillaBTS *)data);
}

void
load_bugzilla_xml (void)
{
	static gboolean loaded;
	xmlDoc *doc;
	BugzillaBTS *bts;
	GSList *item;
	GtkWidget *m;
	GtkWidget *w;

	if (loaded) return;
	loaded = TRUE;

	d(g_print ("loading xml..\n"));

	m = gtk_menu_new ();

	for (item = druid_data.bugzillas; item; item = item->next) {
		bts = (BugzillaBTS *)item->data;
		if (bts->products_xml && !bts->products_xml->done) {
			doc = NULL;

			if (bts->products_xml->read_from_cache)
				doc = xmlParseFile (bts->products_xml->cache_path);

			if (!doc)
				doc = xmlParseFile (bts->products_xml->system_path);

			if (doc) 
				load_products_xml (bts, doc);
			bts->products_xml->done = TRUE;
		}

		if (bts->config_xml && !bts->config_xml->done) {
			doc = NULL;

			if (bts->config_xml->read_from_cache)
				doc = xmlParseFile (bts->config_xml->cache_path);

			if (!doc)
				doc = xmlParseFile (bts->config_xml->system_path);

			if (doc) 
				load_config_xml (bts, doc);
			bts->config_xml->done = TRUE;
		}

		w = gtk_menu_item_new_with_label (bts->name);
		gtk_signal_connect (GTK_OBJECT (w), "activate",
				    GTK_SIGNAL_FUNC (show_products),
				    bts);
		gtk_widget_show (w);
		gtk_menu_append (GTK_MENU (m), w);
	}
	w = GET_WIDGET ("bts-menu");
	gtk_option_menu_set_menu (GTK_OPTION_MENU (w), m);
	gtk_option_menu_set_history (GTK_OPTION_MENU (w), 0);
	bugzilla_bts_add_products_to_clist (druid_data.all_products);	
}

static void
p_string (GnomeVFSURI *uri, gpointer data)
{
	char *s;
	
	s = gnome_vfs_uri_to_string (uri, GNOME_VFS_URI_HIDE_NONE);
	g_print ("\t%s\n", s);
	g_free (s);
}

void
load_bugzillas (void)
{
	BugzillaBTS *bts;
	GtkWidget *w;
	DIR *dir;
	struct dirent *dent;
	char *s, *p;

	dir = opendir (BUDDY_DATADIR"/bugzilla/");
	if (!dir) {
		s = g_strdup_printf (_("Could not open '%s'.\n"
				       "Please make sure Bug Buddy was "
				       "installed correctly."),
				     BUDDY_DATADIR "/bugzilla/");
		w = gnome_error_dialog (s);
		g_free (s);
		gnome_dialog_run_and_close (GNOME_DIALOG (w));
		exit (0);
	}

	druid_data.all_products = g_new0 (BugzillaBTS, 1);
	druid_data.all_products->name = _("All");

	druid_data.bugzillas = g_slist_append (druid_data.bugzillas, druid_data.all_products);

	while ((dent = readdir (dir))) {
		if (dent->d_name[0] == '.')
			continue;

		p = strrchr (dent->d_name, '.');
		if (!p || strcmp (p, ".bugzilla")) 
			continue;	
		
		p = g_concat_dir_and_file (BUDDY_DATADIR"/bugzilla", dent->d_name);
		d(g_print ("trying to load `%s'\n", p));
		bts = load_bugzilla (p);
		g_free (p);
		if (bts) {
			d(g_print ("bugzilla loaded: %s\n", bts->name));
			druid_data.bugzillas = g_slist_append (druid_data.bugzillas, bts);
		}
	}

	if (druid_data.need_to_download) {
		GtkWidget *w = gnome_question_dialog (
			_("Bug Buddy has determined that some of its information about\n"
			  "the various bug tracking systems may need to be updated.\n\n"
			  "Should Bug Buddy try to update these files now?"), NULL, NULL);

		gnome_dialog_set_default (GNOME_DIALOG (w), GNOME_YES);

		d(g_print ("downloading:\n"));
		d(g_list_foreach (druid_data.dlsources, (GFunc)p_string, NULL));
		d(g_print ("to:\n"));
		d(g_list_foreach (druid_data.dldests, (GFunc)p_string, NULL));

		if (!gnome_dialog_run_and_close (GNOME_DIALOG (w)))
			if (GNOME_VFS_OK == gnome_vfs_async_xfer (	    
				    &druid_data.vfshandle,
				    druid_data.dlsources,
				    druid_data.dldests,
				    GNOME_VFS_XFER_DEFAULT,
				    GNOME_VFS_XFER_ERROR_MODE_ABORT,
				    GNOME_VFS_XFER_OVERWRITE_MODE_REPLACE,
				    async_update, NULL, NULL, NULL))
				return;
	}
	goto_product_page ();
}

static void
add_product (BugzillaProduct *p, GtkCList *w)
{
	char *entry[3] = { NULL };
	gint row;

	entry[0] = p->name;
	entry[1] = p->description;

	row = gtk_clist_append (w, entry);
	if (p->bts->pixmap)
		gtk_clist_set_pixtext (w, row, 0, p->name, GNOME_PAD_SMALL, 
				       p->bts->pixmap, p->bts->mask);

	gtk_clist_set_row_data (w, row, p);
}

void
bugzilla_bts_add_products_to_clist (BugzillaBTS *bts)
{
	GtkCList *w;

	w = GTK_CLIST (GET_WIDGET ("product_list"));
	gtk_clist_freeze (w);
	gtk_clist_clear (w);
	on_product_list_unselect_row (GTK_WIDGET (w), 0, 0, NULL);

	g_slist_foreach (bts->products, (GFunc)add_product, w);
	gtk_clist_columns_autosize (w);

	gtk_clist_thaw (w);
}

static void
add_component (BugzillaComponent *comp, GtkCList *w)
{
	char *entry[3] = { NULL };
	gint row;

	entry[0] = comp->name;
	entry[1] = comp->description;

	row = gtk_clist_append (w, entry);
	gtk_clist_set_row_data (w, row, comp);
}

static void
update_severity (GtkWidget *w, gpointer data)
{
	druid_data.severity = (char *)data;
}

static void
add_severity (char *s, GtkMenu *m)
{
	GtkWidget *w;

	w = gtk_menu_item_new_with_label (s);
	gtk_signal_connect (GTK_OBJECT (w), "activate",
			    GTK_SIGNAL_FUNC (update_severity),
			    s);
	gtk_widget_show (w);
	gtk_menu_append (m, w);
	if (!strcasecmp (s, "normal") || !strcasecmp (s, "unknown")) {
		gtk_menu_item_activate (GTK_MENU_ITEM (w));
		gtk_option_menu_set_history (GTK_OPTION_MENU (GET_WIDGET ("severity-list")), 
					     g_slist_index (druid_data.product->bts->severities, s));
	}
}

void
bugzilla_product_add_components_to_clist (BugzillaProduct *prod)
{
	GtkWidget *m, *c;
	GtkCList *w;

	w = GTK_CLIST (GET_WIDGET ("component_list"));
	gtk_clist_freeze (w);
	gtk_clist_clear (w);
	on_component_list_select_row (GTK_WIDGET (w), 0, 0, NULL);

	g_slist_foreach (prod->components, (GFunc)add_component, w);
	gtk_clist_columns_autosize (w);

	if (w->rows == 1)
		gtk_clist_select_row (w, 0, 0);

	gtk_clist_thaw (w);

	m = gtk_menu_new ();
	c = GET_WIDGET ("severity-list");
	gtk_option_menu_set_menu (GTK_OPTION_MENU (c), m);
	g_slist_foreach (prod->bts->severities, (GFunc)add_severity, m);
}

/*
 * from bugmail_help.html:
 * 
 * Example Mail
 * See the example of the mail body (Dont forget to specify the short description in the mail subject):
 *
 * @product      = Bugzilla
 * @component    = general
 * @version      = All
 * @groupset     = ReadWorld ReadPartners
 * @op_sys       = Linux
 * @priority     = P3
 * @rep_platform = i386
 *
 *
 * This is the description of the bug I found. It is not neccessary to start
 * it with a keyword. 
 *
 * Note: The short_description is neccessary and may be given with the keyword
 * @short_description or will be retrieved from the mail subject.
 */ 

gchar *
generate_email_text (void)
{	
	char *subject, *product,  *component, *version;
	char *opsys,   *platform, *severity,  *body, *tmp_body;
	char *sysinfo, *debug_info, *text_file;
	char *email, *email1;

	subject    = gtk_editable_get_chars (GTK_EDITABLE (GET_WIDGET ("desc-subject")), 0, -1);
	product    = druid_data.product->name;
	component  = druid_data.component->name;
	version    = gtk_editable_get_chars (GTK_EDITABLE (GET_WIDGET ("the-version-entry")), 0, -1);
	opsys      = "Linux";
	platform   = "Debian";
	severity   = druid_data.severity ? druid_data.severity : "Normal";
	/* sysinfo    = generate_sysinfo (); */
	tmp_body   = gtk_editable_get_chars (GTK_EDITABLE (GET_WIDGET ("desc-text")), 0, -1);
	body = format_for_width (tmp_body);
	g_free (tmp_body);

	debug_info = gtk_editable_get_chars (GTK_EDITABLE (GET_WIDGET ("gdb-text")), 0, -1);

	if (druid_data.product->bts->submit_type == BUGZILLA_SUBMIT_FREITAG) {
		email1 = g_strdup_printf (
			"Subject: %s\n"
			"\n"
			"@product = %s\n"
			"@component = %s\n"
			"@version = %s\n"
#if 0
			"@op_sys = %s\n"
			"@rep_platform = %s\n"
#endif
			"@severity = %s\n"
			"\n"
			"%s\n"
			"\n",
			subject,
			product,
			component,
			version,
#if 0
			opsys,
			platform,
#endif
			severity,
			body);
	} else if (druid_data.product->bts->submit_type == BUGZILLA_SUBMIT_DEBIAN) {
		email1 = g_strdup_printf (
			"Subject: %s\n"
			"\n"
			"Package: %s\n"
			"%s: %s\n"
			"Version: %s\n"
			"Synopsis: %s\n"
			"Bugzilla-Product: %s\n"
			"Bugzilla-Component: %s\n"
			"\n"
			"Description:\n"
			"%s\n"
			"\n",
			subject,
			product,
			druid_data.product->bts->severity_header,
			severity,
			version,
			subject,
			product,
			component,
			body);
	} else 
		email1 = g_strdup ("Unkown format.");
	
	if (druid_data.crash_type != CRASH_NONE && debug_info[0]) {
		email = g_strconcat (email1, "\nDebugging Information:\n\n", debug_info, NULL);
		g_free (email1);
	} else
		email = email1;       
	
	g_free (subject);
	g_free (version);
	g_free (body);
	g_free (debug_info);

	return email;
}
