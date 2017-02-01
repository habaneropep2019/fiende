/* bug-buddy bug submitting program
 *
 * Copyright (C) 1999 - 2001 Jacob Berkman
 * Copyright 2000 Ximian, Inc.
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

#ifndef __BUG_BUDDY_H__
#define __BUG_BUDDY_H__

#include <glade/glade.h>
#include <libgnomeui/gnome-canvas.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include <sys/types.h>
#include "distro.h"
#include "bugzilla.h"

typedef enum {
	CRASH_DIALOG,
	CRASH_CORE,
	CRASH_NONE
} CrashType;

typedef enum {
	SUBMIT_REPORT,
	SUBMIT_TO_SELF,
	SUBMIT_FILE
} SubmitType;

typedef enum {
	BUG_NEW,
	BUG_EXISTING
} BugType;

typedef enum {
	STATE_INTRO,
	STATE_GDB,
	STATE_DESC,
	STATE_UPDATE,
	STATE_PRODUCT,
	STATE_COMPONENT,
	STATE_SYSTEM,
	STATE_EMAIL,
	STATE_FINISHED,
	STATE_LAST
} BuddyState;

#define GET_WIDGET(name) (glade_xml_get_widget (druid_data.xml, (name)))

typedef struct {
	/* contact page */
	gchar *name;
	gchar *email;
	
	/* package page */
	gchar *package;
	gchar *package_ver;
	
	/* dialog page */
	gchar *app_file;
	gchar *pid;
	
	/* core page */
	gchar *core_file;

	/* file to include */
	gchar *include_file;
} PoptData;
extern PoptData popt_data;

typedef struct {
	GladeXML  *xml;
	BuddyState state;

	/* canvas stuff */
	GnomeCanvasItem *title_box;
	GnomeCanvasItem *banner;
	GnomeCanvasItem *logo;

	GnomeCanvasItem *side_box;
	GnomeCanvasItem *side_image;

	/* throbber */
	GnomeCanvasItem *throbber;
	GdkPixbuf *throbber_pb;
	guint            throbber_id;

	gboolean already_run;

	Distribution      *distro;
	char              *bts_file;

	CrashType  crash_type;

	int selected_row;
	int progress_timeout;

	/* gdb stuff */
	pid_t       app_pid;
	pid_t       gdb_pid;
	GIOChannel *ioc;
	int         fd;
	gboolean    explicit_dirty;

	BugzillaBTS *all_products;
	GSList      *bugzillas;

	/* Debian BTS stuff */
	SubmitType    submit_type;
#if 0
	BugType       bug_type; 
	SeverityType  severity;
	BugClassType  bug_class;
#endif
	GSList       *packages;

	/* Bugzilla BTS stuff */
	BugzillaProduct   *product;
	BugzillaComponent *component;
	char              *severity;

	GList *dlsources;
	GList *dldests;

	GnomeVFSAsyncHandle *vfshandle;
	gboolean need_to_download;
} DruidData;

extern DruidData druid_data;

extern const gchar *severity[];
extern const gchar *bug_class[][2];

void druid_set_sensitive (gboolean prev, gboolean next, gboolean cancel);
void druid_set_state (BuddyState state);

void get_trace_from_core (const gchar *core_file);
void get_trace_from_pair (const gchar *app, const gchar *extra);
void stop_gdb (void);
void start_gdb (void);

void stop_progress (void);

void append_packages (void);

void load_config (void);
void save_config (void);

#endif /* __bug_buddy_h__ */
