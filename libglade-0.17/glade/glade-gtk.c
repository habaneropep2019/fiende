/* -*- Mode: C; c-basic-offset: 8 -*-
 * libglade - a library for building interfaces from XML files at runtime
 * Copyright (C) 1998-2001  James Henstridge <james@daa.com.au>
 *
 * glade-gtk.c: support for GTK+ widgets in libglade
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the 
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA  02111-1307, USA.
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <string.h>
#include <glade/glade-build.h>
#include <glade/glade-private.h>
#include <gmodule.h>
#include <gtk/gtk.h>

#ifndef ENABLE_NLS
/* a slight optimisation when gettext is off */
#define glade_xml_gettext(xml, msgid) (msgid)
#endif
#undef _
#define _(msgid) (glade_xml_gettext(xml, msgid))

/* functions to actually build the widgets */

static void
button_build_children (GladeXML *xml, GtkWidget *w, GladeWidgetInfo *info,
		       const char *longname)
{
	GtkWidget *child = glade_xml_build_widget(xml,
			(GladeWidgetInfo *)info->children->data, longname);
	guint key = glade_xml_get_parent_accel(xml);

	gtk_container_add(GTK_CONTAINER(w), child);
	if (key)
		gtk_widget_add_accelerator(w, "clicked",
					   glade_xml_ensure_accel(xml),
					   key, GDK_MOD1_MASK, 0);
}

static void
menushell_build_children (GladeXML *xml, GtkWidget *w, GladeWidgetInfo *info,
			  const char *longname)
{
	GList *tmp;
	gboolean uline;

	uline = strcmp(info->class, "GtkMenuBar") != 0;
	if (uline)
		glade_xml_push_uline_accel(xml,
			gtk_menu_ensure_uline_accel_group(GTK_MENU(w)));
	for (tmp = info->children; tmp; tmp = tmp->next) {
		GtkWidget *child = glade_xml_build_widget(xml, tmp->data,
							  longname);
		gtk_menu_shell_append(GTK_MENU_SHELL(w), child);
	}
	if (uline)
		glade_xml_pop_uline_accel(xml);
}

static void
menuitem_build_children (GladeXML *xml, GtkWidget *w, GladeWidgetInfo *info,
			 const char *longname)
{
	GtkWidget *menu;
	if (!info->children) return;
	menu = glade_xml_build_widget(xml, info->children->data, longname);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(w), menu);
	/* wierd things happen if menu is initially visible */
	gtk_widget_hide(menu);
}

static void
box_build_children (GladeXML *xml, GtkWidget *w, GladeWidgetInfo *info,
		    const char *longname)
{
	GList *tmp;

	for (tmp = info->children; tmp; tmp = tmp->next) {
		GladeWidgetInfo *cinfo = tmp->data;
		GtkWidget *child = glade_xml_build_widget (xml,cinfo,longname);
		GList *tmp2;
		gboolean expand = TRUE, fill = TRUE, start = TRUE;
		gint padding = 0;

		for (tmp2 = cinfo->child_attributes; tmp2; tmp2 = tmp2->next) {
			GladeAttribute *attr = tmp2->data;

			switch (attr->name[0]) {
			case 'e':
				if (!strcmp (attr->name, "expand"))
					expand = attr->value[0] == 'T';
				break;
			case 'f':
				if (!strcmp (attr->name, "fill"))
					fill = attr->value[0] == 'T';
				break;
			case 'p':
				if (!strcmp (attr->name, "padding"))
					padding = strtol(attr->value, NULL, 0);
				else if (!strcmp (attr->name, "pack"))
					start = strcmp (attr->value,
							"GTK_PACK_START") == 0;
				break;
			}
		}
		if (start)
			gtk_box_pack_start (GTK_BOX(w), child, expand,
					    fill, padding);
		else
			gtk_box_pack_end (GTK_BOX(w), child, expand,
					  fill, padding);
	}
}

static void
table_build_children (GladeXML *xml, GtkWidget *w, GladeWidgetInfo *info,
		      const char *longname) {
	GList *tmp;

	for (tmp = info->children; tmp; tmp = tmp->next) {
		GladeWidgetInfo *cinfo = tmp->data;
		GtkWidget *child = glade_xml_build_widget (xml,cinfo,longname);
		GList *tmp2;
		gint left_attach=0,right_attach=1,top_attach=0,bottom_attach=1;
		gint xpad=0, ypad=0, xoptions=0, yoptions=0;
    
		for (tmp2 = cinfo->child_attributes; tmp2; tmp2 = tmp2->next) {
			GladeAttribute *attr = tmp2->data;
			switch (attr->name[0]) {
			case 'b':
				if (!strcmp(attr->name, "bottom_attach"))
					bottom_attach = strtol(attr->value, NULL, 0);
				break;
			case 'l':
				if (!strcmp(attr->name, "left_attach"))
					left_attach = strtol(attr->value, NULL, 0);
				break;
			case 'r':
				if (!strcmp(attr->name, "right_attach"))
					right_attach = strtol(attr->value, NULL, 0);
				break;
			case 't':
				if (!strcmp(attr->name, "top_attach"))
					top_attach = strtol(attr->value, NULL, 0);
				break;
			case 'x':
				switch (attr->name[1]) {
				case 'e':
					if (!strcmp(attr->name, "xexpand") &&
					    attr->value[0] == 'T')
						xoptions |= GTK_EXPAND;
					break;
				case 'f':
					if (!strcmp(attr->name, "xfill") &&
					    attr->value[0] == 'T')
						xoptions |= GTK_FILL;
					break;
				case 'p':
					if (!strcmp(attr->name, "xpad"))
						xpad = strtol(attr->value, NULL, 0);
					break;
				case 's':
					if (!strcmp(attr->name, "xshrink") &&
					    attr->value[0] == 'T')
						xoptions |= GTK_SHRINK;
					break;
				}
				break;
			case 'y':
				switch (attr->name[1]) {
				case 'e':
					if (!strcmp(attr->name, "yexpand") &&
					    attr->value[0] == 'T')
						yoptions |= GTK_EXPAND;
					break;
				case 'f':
					if (!strcmp(attr->name, "yfill") &&
					    attr->value[0] == 'T')
						yoptions |= GTK_FILL;
					break;
				case 'p':
					if (!strcmp(attr->name, "ypad"))
						ypad = strtol(attr->value, NULL, 0);
					break;
				case 's':
					if (!strcmp(attr->name, "yshrink") &&
					    attr->value[0] == 'T')
						yoptions |= GTK_SHRINK;
					break;
				}
				break;
			}
		}
		gtk_table_attach(GTK_TABLE(w), child, left_attach,right_attach,
				 top_attach,bottom_attach, xoptions,yoptions,
				 xpad,ypad);
	}  
}

static void
fixed_build_children (GladeXML *xml, GtkWidget *w, GladeWidgetInfo *info,
		      const char *longname)
{
	GList *tmp;

	for (tmp = info->children; tmp; tmp = tmp->next) {
		GladeWidgetInfo *cinfo = tmp->data;
		GtkWidget *child = glade_xml_build_widget(xml, cinfo,longname);
		GList *tmp2;
		gint xpos = 0, ypos = 0;

		for (tmp2 = cinfo->attributes; tmp2; tmp2 = tmp2->next) {
			GladeAttribute *attr = tmp2->data;
			
			if (attr->name[0] == 'x' && attr->name[1] == '\0')
				xpos = strtol(attr->value, NULL, 0);
			else if (attr->name[0] == 'y' && attr->name[1] == '\0')
				ypos = strtol(attr->value, NULL, 0);
		}
		gtk_fixed_put(GTK_FIXED(w), child, xpos, ypos);
	}
}

static void
layout_build_children (GladeXML *xml, GtkWidget *w, GladeWidgetInfo *info,
		       const char *longname)
{
	GList *tmp;

	for (tmp = info->children; tmp; tmp = tmp->next) {
		GladeWidgetInfo *cinfo = tmp->data;
		GtkWidget *child = glade_xml_build_widget(xml, cinfo,longname);
		GList *tmp2;
		gint xpos = 0, ypos = 0;

		for (tmp2 = cinfo->attributes; tmp2; tmp2 = tmp2->next) {
			GladeAttribute *attr = tmp2->data;
			
			if (attr->name[0] == 'x' && attr->name[1] == '\0')
				xpos = strtol(attr->value, NULL, 0);
			else if (attr->name[0] == 'y' && attr->name[1] == '\0')
				ypos = strtol(attr->value, NULL, 0);
		}
		gtk_layout_put(GTK_LAYOUT(w), child, xpos, ypos);
	}
}

static void
clist_build_children (GladeXML *xml, GtkWidget *w, GladeWidgetInfo *info,
		      const char *longname)
{
	GList *tmp;
	gint col = 0;

	for (tmp = info->children; tmp; tmp = tmp->next) {
		GladeWidgetInfo *cinfo = tmp->data;
		GtkWidget *child = NULL;

		/* if it is a GtkLabel child with a "label" attribute,
		 * treat it specially. */
		if (!strcmp(cinfo->class, "GtkLabel")) {
			GList *tmp2;
			gchar *label = NULL;

			for (tmp2 = cinfo->attributes; tmp2; tmp2=tmp2->next) {
				GladeAttribute *attr = tmp2->data;

				if (!strcmp(attr->name, "label")) {
					label = attr->value;
					break;
				}
			}
			if (label) {
				gtk_clist_set_column_title (GTK_CLIST(w), col,
							    label);
				/* get the GtkLabel, which is a child
				 * of the alignment widget used as
				 * title. */
				child = gtk_clist_get_column_widget
					(GTK_CLIST(w), col);
				child = GTK_BIN(child)->child;
				glade_xml_set_common_params(xml, child, cinfo,
							    longname);
			}
		}
		if (!child) {
			child = glade_xml_build_widget (xml, cinfo, longname);

			gtk_clist_set_column_widget (GTK_CLIST(w), col, child);
		}
		col++;
	}
}

static void
paned_build_children (GladeXML *xml, GtkWidget *w, GladeWidgetInfo *info,
		      const char *longname)
{
	GList *tmp, *tmp2;
	GtkWidget *child;
	gboolean resize, shrink;

	tmp = info->children;
	if (!tmp)
		return;
	
	child = glade_xml_build_widget (xml, tmp->data, longname);
	resize = FALSE; shrink = TRUE;
	for (tmp2 = ((GladeWidgetInfo *)tmp->data)->child_attributes;
	     tmp2; tmp2 = tmp2->next) {
		GladeAttribute *attr = tmp2->data;

		if (!strcmp(attr->name, "resize"))
			resize = attr->value[0] == 'T';
		else if (!strcmp(attr->name, "shrink"))
			shrink = attr->value[0] == 'T';
	}
	gtk_paned_pack1 (GTK_PANED(w), child, resize, shrink);

	tmp = tmp->next;
	if (!tmp)
		return;
	
	child = glade_xml_build_widget (xml, tmp->data, longname);
	resize = TRUE; shrink = TRUE;
	for (tmp2 = ((GladeWidgetInfo *)tmp->data)->child_attributes;
	     tmp2; tmp2 = tmp2->next) {
		GladeAttribute *attr = tmp2->data;

		if (!strcmp(attr->name, "resize"))
			resize = attr->value[0] == 'T';
		else if (!strcmp(attr->name, "shrink"))
			shrink = attr->value[0] == 'T';
	}
	gtk_paned_pack2 (GTK_PANED(w), child, resize, shrink);
}

static void
note_change_page(GtkWidget *child, GtkNotebook *notebook)
{
	gint page = gtk_notebook_page_num(notebook, child);

	gtk_notebook_set_page(notebook, page);
}

static void
note_page_mapped (GtkWidget *page, GtkAccelGroup *accel_group)
{
	GtkWidget *dialog = gtk_widget_get_toplevel (GTK_WIDGET (page));

	gtk_window_add_accel_group (GTK_WINDOW (dialog), accel_group);
}

static void
note_page_unmapped (GtkWidget *page, GtkAccelGroup *accel_group)
{
	GtkWidget *dialog = gtk_widget_get_toplevel (GTK_WIDGET (page));

	gtk_window_remove_accel_group (GTK_WINDOW (dialog), accel_group);
}

static void note_page_setup_signals (GtkWidget *page, GtkAccelGroup *accel)
{
	gtk_accel_group_ref(accel);
	gtk_signal_connect_full (GTK_OBJECT (page),
			    "map",
			    GTK_SIGNAL_FUNC (note_page_mapped), NULL, 
			    accel, (GtkDestroyNotify)gtk_accel_group_unref,
			    FALSE, FALSE);
	gtk_accel_group_ref(accel);
	gtk_signal_connect_full (GTK_OBJECT (page),
			    "unmap",
			    GTK_SIGNAL_FUNC (note_page_unmapped), NULL,
			    accel, (GtkDestroyNotify)gtk_accel_group_unref,
			    FALSE, FALSE);
}

static void
notebook_build_children (GladeXML *xml, GtkWidget *w, GladeWidgetInfo *info,
			 const char *longname)
{

	/*
	 * the notebook tabs are listed after the pages, and have
	 * child_name set to Notebook:tab.  We store pages in a GList
	 * while waiting for tabs
	 */
	
	GList *pages = NULL;
	GList *tmp;

	for (tmp = info->children; tmp; tmp = tmp->next) {
		GladeWidgetInfo *cinfo = tmp->data;
		GtkWidget *child;
		GList *tmp2;
		GladeAttribute *attr = NULL;
		GtkAccelGroup *accel;

		accel = glade_xml_push_accel(xml);
		child = glade_xml_build_widget (xml,cinfo,longname);
		note_page_setup_signals(child, accel);
		glade_xml_pop_accel(xml);
		for (tmp2 = cinfo->attributes; tmp2; tmp2 = tmp2->next) {
			attr = tmp2->data;
			if (!strcmp(attr->name, "child_name"))
				break;
		}
		if (tmp2 == NULL || strcmp(attr->value, "Notebook:tab") != 0)
			pages = g_list_append (pages, child);
		else {
			GtkWidget *page;
			gint key = glade_xml_get_parent_accel(xml);

			if (pages) {
				page = pages->data;
				pages = g_list_remove (pages, page);
			} else {
				page = gtk_label_new("Unknown page");
				gtk_widget_show(page);
			}
			gtk_notebook_append_page (GTK_NOTEBOOK(w), page,child);
			if (key) {
				gtk_widget_add_accelerator(page, "grab_focus",
					     glade_xml_ensure_accel(xml),
					     key, GDK_MOD1_MASK, 0);
				gtk_signal_connect(GTK_OBJECT(page),
					"grab_focus",
					GTK_SIGNAL_FUNC(note_change_page), w);
			}
		}
	}
}

static void
packer_build_children (GladeXML *xml, GtkWidget *w, GladeWidgetInfo *info,
		       const char *longname)
{
	GList *tmp;

	for (tmp = info->children; tmp; tmp = tmp->next) {
		GladeWidgetInfo *cinfo = tmp->data;
		GList *tmp2;
		GtkWidget *child = glade_xml_build_widget (xml,cinfo,longname);
		GtkSideType side = GTK_SIDE_TOP;
		GtkAnchorType anchor = GTK_ANCHOR_CENTER;
		GtkPackerOptions options = 0;
		gboolean use_default = TRUE;
		guint border = 0, pad_x = 0, pad_y = 0, ipad_x = 0, ipad_y = 0;

		for (tmp2 = cinfo->child_attributes; tmp2; tmp2 = tmp2->next) {
			GladeAttribute *attr = tmp2->data;

			if (!strcmp(attr->name, "side"))
				side = glade_enum_from_string(
					GTK_TYPE_SIDE_TYPE, attr->value);
			else if (!strcmp(attr->name, "anchor"))
				anchor = glade_enum_from_string(
					GTK_TYPE_ANCHOR_TYPE, attr->value);
			else if (!strcmp(attr->name, "expand")) {
				if (attr->value[0] == 'T')
					options |= GTK_PACK_EXPAND;
			} else if (!strcmp(attr->name, "xfill")) {
				if (attr->value[0] == 'T')
					options |= GTK_FILL_X;
			} else if (!strcmp(attr->name, "yfill")) {
				if (attr->value[0] == 'T')
					options |= GTK_FILL_Y;
			} else if (!strcmp(attr->name, "use_default"))
				use_default = attr->value[0] == 'T';
			else if (!strcmp(attr->name, "border_width"))
				border = strtoul(attr->value, NULL, 0);
			else if (!strcmp(attr->name, "xpad"))
				pad_x = strtoul(attr->value, NULL, 0);
			else if (!strcmp(attr->name, "ypad"))
				pad_y = strtoul(attr->value, NULL, 0);
			else if (!strcmp(attr->name, "xipad"))
				ipad_x = strtoul(attr->value, NULL, 0);
			else if (!strcmp(attr->name, "yipad"))
				ipad_y = strtoul(attr->value, NULL, 0);
		}
		if (use_default)
			gtk_packer_add_defaults(GTK_PACKER(w), child,
						side, anchor, options);
		else
			gtk_packer_add(GTK_PACKER(w), child, side, anchor,
				       options, border, pad_x, pad_y,
				       ipad_x, ipad_y);
	}
}

static void
dialog_build_children (GladeXML *xml, GtkWidget *w, GladeWidgetInfo *info,
		       const char *longname)
{
	GList *tmp;
	char *vboxname;

	vboxname = g_strconcat (longname, ".", info->name, NULL);

	/* all dialog children are inside the main vbox */
	for (tmp = ((GladeWidgetInfo *)info->children->data)->children;
	     tmp; tmp = tmp->next) {
		GladeWidgetInfo *cinfo = tmp->data;
		GtkWidget *child;
		GList *tmp2;
		gboolean is_action_area = FALSE;
		gboolean expand = TRUE, fill = TRUE, start = TRUE;
		gint padding = 0;

		for (tmp2 = cinfo->attributes; tmp2; tmp2 = tmp2->next) {
			GladeAttribute *attr = tmp2->data;
			if (!strcmp(attr->name, "child_name") &&
			    !strcmp(attr->value, "Dialog:action_area")) {
				is_action_area = TRUE;
				break;
			}
		}
				
		if (is_action_area) {
			char *parent_name;

			/* we got the action area -- call box_build_children
			 * on it */
			parent_name = g_strconcat(vboxname, ".", cinfo->name,
						  NULL);
			box_build_children (xml, GTK_DIALOG(w)->action_area,
					    cinfo, parent_name);
			g_free(parent_name);

			continue;
		}

		child = glade_xml_build_widget (xml, cinfo, vboxname);

		for (tmp2 = cinfo->child_attributes; tmp2; tmp2 = tmp2->next) {
			GladeAttribute *attr = tmp2->data;

			switch (attr->name[0]) {
			case 'e':
				if (!strcmp (attr->name, "expand"))
					expand = attr->value[0] == 'T';
				break;
			case 'f':
				if (!strcmp (attr->name, "fill"))
					fill = attr->value[0] == 'T';
				break;
			case 'p':
				if (!strcmp (attr->name, "padding"))
					padding = strtol(attr->value, NULL, 0);
				else if (!strcmp (attr->name, "pack"))
					start = strcmp (attr->value,
							"GTK_PACK_START") == 0;
				break;
			}
		}
		if (start)
			gtk_box_pack_start (GTK_BOX(GTK_DIALOG(w)->vbox),
					    child, expand, fill, padding);
		else
			gtk_box_pack_end (GTK_BOX(GTK_DIALOG(w)->vbox),
					  child, expand, fill, padding);
	}
	g_free (vboxname);
}

static void
toolbar_build_children (GladeXML *xml, GtkWidget *w, GladeWidgetInfo *info,
			const char *longname)
{
	GList *tmp;

	for (tmp = info->children; tmp; tmp = tmp->next) {
		GladeWidgetInfo *cinfo = tmp->data;
		GList *tmp2;
		gboolean is_button = FALSE;
		gchar *group_name = NULL;
		GtkWidget *child;

		for (tmp2 = cinfo->child_attributes; tmp2; tmp2 = tmp2->next) {
			GladeAttribute *attr = tmp2->data;
			if (!strcmp(attr->name, "new_group") &&
			    attr->value[0] == 'T')
				gtk_toolbar_append_space(GTK_TOOLBAR(w));
		}
		
		/* check to see if this is a special Toolbar:button or just
		 * a standard widget we are adding to the toolbar */
		for (tmp2 = cinfo->attributes; tmp2; tmp2 = tmp2->next) {
			GladeAttribute *attr = tmp2->data;
			if (!strcmp(attr->name, "child_name") &&
			    !strcmp(attr->value, "Toolbar:button"))
				is_button = TRUE;
			else if (!strcmp(attr->name, "group"))
				group_name = attr->value;
		}
		if (is_button) {
			char *label = NULL, *icon = NULL;
			gboolean active = FALSE;
			GtkWidget *iconw = NULL;

			for (tmp2 = cinfo->attributes;tmp2;tmp2 = tmp2->next) {
				GladeAttribute *attr = tmp2->data;
				if (!strcmp(attr->name, "label"))
					label = attr->value;
				else if (!strcmp(attr->name, "icon"))
					icon = glade_xml_relative_file(xml,
								attr->value);
				else if (!strcmp(attr->name, "active"))
					active = (attr->value[0] == 'T');
			}
			if (icon) {
				GdkPixmap *pix;
				GdkBitmap *mask = NULL;
				pix = gdk_pixmap_colormap_create_from_xpm(NULL,
					gtk_widget_get_colormap(w), &mask,
					NULL, icon);
				g_free(icon);
				if (pix)
					iconw = gtk_pixmap_new(pix, mask);
				else
					iconw = gtk_type_new(
						gtk_pixmap_get_type());
				if (pix) gdk_pixmap_unref(pix);
				if (mask) gdk_bitmap_unref(mask);
			}
			if (!strcmp(cinfo->class, "GtkToggleButton")) {
				child = gtk_toolbar_append_element(
					GTK_TOOLBAR(w),
					GTK_TOOLBAR_CHILD_TOGGLEBUTTON, NULL,
					_(label), NULL, NULL, iconw, NULL,
					NULL);
				gtk_toggle_button_set_active(
					GTK_TOGGLE_BUTTON(child), active);
			} else if (!strcmp(cinfo->class, "GtkRadioButton")) {
				child = gtk_toolbar_append_element(
					GTK_TOOLBAR(w),
					GTK_TOOLBAR_CHILD_RADIOBUTTON, NULL,
					_(label), NULL, NULL, iconw, NULL,
					NULL);
				if (group_name) {
					GSList *group =
						g_hash_table_lookup(
						    xml->priv->radio_groups,
						    group_name);

					gtk_radio_button_set_group(
						GTK_RADIO_BUTTON(child),
						group);
					if (!group)
						group_name =
							g_strdup(group_name);
					g_hash_table_insert(
						xml->priv->radio_groups,
						group_name,
						gtk_radio_button_group(
						    GTK_RADIO_BUTTON(child)));
				}

			} else
				child = gtk_toolbar_append_item(GTK_TOOLBAR(w),
					_(label), NULL, NULL, iconw, NULL,
					NULL);
			glade_xml_set_common_params(xml, child, cinfo,
						    longname);
		} else {
			child = glade_xml_build_widget(xml, cinfo, longname);
			gtk_toolbar_append_widget(GTK_TOOLBAR(w), child,
						  NULL, NULL);
		}
	}
}

static void
fileselection_build_children (GladeXML *xml, GtkWidget *w,
			      GladeWidgetInfo *info, const char *longname)
{
	GList *tmp;

	for (tmp = info->children; tmp; tmp = tmp->next) {
		GladeWidgetInfo *cinfo = tmp->data;
		GtkWidget *child = NULL;
		GList *tmp2;
		gchar *child_name = NULL;

		for (tmp2 = cinfo->attributes; tmp2; tmp2 = tmp2->next) {
			GladeAttribute *attr = tmp2->data;
			if (!strcmp(attr->name, "child_name")) {
				child_name = attr->value;
				break;
			}
		}
		if (!child_name) continue;
		if (!strcmp(child_name, "FileSel:ok_button"))
			child = GTK_FILE_SELECTION(w)->ok_button;
		else if (!strcmp(child_name, "FileSel:cancel_button"))
			child = GTK_FILE_SELECTION(w)->cancel_button;
		if (child)
			glade_xml_set_common_params(xml, child, cinfo,
						    longname);
	}
}

static void
colorselectiondialog_build_children(GladeXML *xml, GtkWidget *w,
				    GladeWidgetInfo *info,const char *longname)
{
	GList *tmp;

	for (tmp = info->children; tmp; tmp = tmp->next) {
		GladeWidgetInfo *cinfo = tmp->data;
		GtkWidget *child = NULL;
		GList *tmp2;
		gchar *child_name = NULL;

		for (tmp2 = cinfo->attributes; tmp2; tmp2 = tmp2->next) {
			GladeAttribute *attr = tmp2->data;
			if (!strcmp(attr->name, "child_name")) {
				child_name = attr->value;
				break;
			}
		}
		if (!child_name) continue;

		if (!strcmp(child_name, "ColorSel:ok_button"))
			child = GTK_COLOR_SELECTION_DIALOG(w)->ok_button;
		else if (!strcmp(child_name, "ColorSel:cancel_button"))
			child = GTK_COLOR_SELECTION_DIALOG(w)->cancel_button;
		else if (!strcmp(child_name, "ColorSel:help_button"))
			child = GTK_COLOR_SELECTION_DIALOG(w)->help_button;
		if (child)
			glade_xml_set_common_params(xml, child, cinfo,
						    longname);
	}
}

static void
fontselectiondialog_build_children (GladeXML *xml, GtkWidget *w,
				    GladeWidgetInfo *info,const char *longname)
{
	GList *tmp;

	for (tmp = info->children; tmp; tmp = tmp->next) {
		GladeWidgetInfo *cinfo = tmp->data;
		GtkWidget *child = NULL;
		GList *tmp2;
		gchar *child_name = NULL;

		for (tmp2 = cinfo->attributes; tmp2; tmp2 = tmp2->next) {
			GladeAttribute *attr = tmp2->data;
			if (!strcmp(attr->name, "child_name")) {
				child_name = attr->value;
				break;
			}
		}
		if (!child_name) continue;

		if (!strcmp(child_name, "FontSel:ok_button"))
			child = GTK_FONT_SELECTION_DIALOG(w)->ok_button;
		else if (!strcmp(child_name, "FontSel:apply_button"))
			child = GTK_FONT_SELECTION_DIALOG(w)->apply_button;
		else if (!strcmp(child_name, "FontSel:cancel_button"))
			child = GTK_FONT_SELECTION_DIALOG(w)->cancel_button;
		if (child)
			glade_xml_set_common_params(xml, child, cinfo,
						    longname);
	}
}

static void
inputdialog_build_children (GladeXML *xml, GtkWidget *w,
			    GladeWidgetInfo *info,const char *longname)
{
	GList *tmp;

	for (tmp = info->children; tmp; tmp = tmp->next) {
		GladeWidgetInfo *cinfo = tmp->data;
		GtkWidget *child = NULL;
		GList *tmp2;
		gchar *child_name = NULL;

		for (tmp2 = cinfo->attributes; tmp2; tmp2 = tmp2->next) {
			GladeAttribute *attr = tmp2->data;
			if (!strcmp(attr->name, "child_name")) {
				child_name = attr->value;
				break;
			}
		}
		if (!child_name) continue;

		if (!strcmp(child_name, "InputDialog:save_button"))
			child = GTK_INPUT_DIALOG(w)->save_button;
		else if (!strcmp(child_name, "InputDialog:close_button"))
			child = GTK_INPUT_DIALOG(w)->close_button;

		if (child)
			glade_xml_set_common_params(xml, child, cinfo,
						    longname);
	}
}

static void
combo_build_children (GladeXML *xml, GtkWidget *w,
		      GladeWidgetInfo *info, const char *longname)
{
	GList *tmp;
	GladeWidgetInfo *cinfo = NULL;
	GtkEntry *entry;

	for (tmp = info->children; tmp; tmp = tmp->next) {
		GList *tmp2;
		gchar *child_name = NULL;
		cinfo = tmp->data;
		for (tmp2 = cinfo->attributes; tmp2; tmp2 = tmp2->next) {
			GladeAttribute *attr = tmp2->data;
			if (!strcmp(attr->name, "child_name")) {
				child_name = attr->value;
				break;
			}
		}
		if (child_name && !strcmp(child_name, "GtkCombo:entry"))
			break;
	}
	if (!tmp)
		return;
	entry = GTK_ENTRY(GTK_COMBO(w)->entry);
	for (tmp = cinfo->attributes; tmp; tmp = tmp->next) {
		GladeAttribute *attr = tmp->data;
		if (!strcmp(attr->name, "editable"))
			gtk_entry_set_editable(entry, attr->value[0] == 'T');
		else if (!strcmp(attr->name, "text_visible"))
			gtk_entry_set_visibility(entry, attr->value[0] == 'T');
		else if (!strcmp(attr->name, "text_max_length"))
			gtk_entry_set_max_length(entry, strtol(attr->value,
							       NULL, 0));
		else if (!strcmp(attr->name, "text"))
			gtk_entry_set_text(entry, attr->value);
	}
	glade_xml_set_common_params(xml, GTK_WIDGET(entry), cinfo, longname);
}

static void
misc_set (GtkMisc *misc, GladeWidgetInfo *info)
{
	GList *tmp;

	for (tmp = info->attributes; tmp; tmp = tmp->next){
		GladeAttribute *attr = tmp->data;

		switch (attr->name[0]) {
		case 'x':
			if (!strcmp (attr->name, "xalign")){
				gfloat align = g_strtod (attr->value, NULL);
				gtk_misc_set_alignment(misc, align,
						       misc->yalign);
			} else if (!strcmp(attr->name, "xpad")){
				gint pad = strtol(attr->value, NULL, 0);
				gtk_misc_set_padding(misc, pad, misc->ypad);
			} break;
			
		case 'y':
			if (!strcmp(attr->name, "yalign")){
				gfloat align = g_strtod (attr->value, NULL);
				gtk_misc_set_alignment(misc, misc->xalign,
						       align);
			} else if (!strcmp(attr->name, "ypad")){
				gint pad = strtol(attr->value, NULL, 0);
				gtk_misc_set_padding(misc, misc->xpad, pad);
			}
			break;
		}
	}
}

static GtkWidget *
label_new (GladeXML *xml, GladeWidgetInfo *info)
{
	GList *tmp;
	GtkWidget *label;
	guint key;
	gchar *string = NULL, *focus_target = NULL;
	GtkJustification just = GTK_JUSTIFY_CENTER;
	gboolean wrap = FALSE;

	for (tmp = info->attributes; tmp; tmp = tmp->next) {
		GladeAttribute *attr = tmp->data;

		if (!strcmp(attr->name, "label")) {
			string = attr->value;
		} else if (!strcmp(attr->name, "justify"))
			just = glade_enum_from_string(GTK_TYPE_JUSTIFICATION,
						      attr->value);
		else if (!strcmp(attr->name, "default_focus_target")) {
			if (!focus_target) focus_target = attr->value;
		} else if (!strcmp(attr->name, "focus_target"))
			focus_target = attr->value;
		else if (!strcmp(attr->name, "wrap"))
			wrap = attr->value[0] == 'T';
	}

	label = gtk_label_new("");
	if (string){
		char *s = string [0] ? _(string) : "";
		
		key = gtk_label_parse_uline(GTK_LABEL(label), s);
	} else
		key = 0;
	
	if (key)
		glade_xml_handle_label_accel(xml, focus_target, key);
	if (just != GTK_JUSTIFY_CENTER)
		gtk_label_set_justify(GTK_LABEL(label), just);
	if (wrap)
		gtk_label_set_line_wrap(GTK_LABEL(label), wrap);
	misc_set (GTK_MISC(label), info);
	return label;
}

static GtkWidget *
accellabel_new (GladeXML *xml, GladeWidgetInfo *info)
{
	GList *tmp;
	GtkWidget *label;
	gchar *string = NULL;
	GtkJustification just = GTK_JUSTIFY_CENTER;
	gboolean wrap = FALSE;

	for (tmp = info->attributes; tmp; tmp = tmp->next) {
		GladeAttribute *attr = tmp->data;

		if (!strcmp(attr->name, "label")) {
			string = attr->value;
		} else if (!strcmp(attr->name, "justify"))
			just = glade_enum_from_string(GTK_TYPE_JUSTIFICATION,
						      attr->value);
		else if (!strcmp(attr->name, "wrap"))
			wrap = attr->value[0] == 'T';
	}

	label = gtk_accel_label_new(_(string));
	if (just != GTK_JUSTIFY_CENTER)
		gtk_label_set_justify(GTK_LABEL(label), just);
	if (wrap)
		gtk_label_set_line_wrap(GTK_LABEL(label), wrap);
	misc_set(GTK_MISC(label), info);

	return label;
}

static GtkWidget *
entry_new (GladeXML *xml, GladeWidgetInfo *info)
{
	GtkWidget *entry;
	GList *tmp;
	char *text = NULL;
	gint text_max_length = -1;
	gboolean editable = TRUE, text_visible = TRUE;

	for (tmp = info->attributes; tmp; tmp = tmp->next) {
		GladeAttribute *attr = tmp->data;

		switch (attr->name[0]) {
		case 'e':
			if (!strcmp(attr->name, "editable"))
				editable = attr->value[0] == 'T';
			break;
		case 't':
			if (!strcmp(attr->name, "text"))
				text = attr->value;
			else if (!strcmp(attr->name, "text_visible"))
				text_visible = attr->value[0] == 'T';
			else if (!strcmp(attr->name, "text_max_length"))
				text_max_length = strtol(attr->value, NULL, 0);
			break;
		}
	}
	if (text_max_length >= 0)
		entry = gtk_entry_new_with_max_length(text_max_length);
	else
		entry = gtk_entry_new();

	if (text)
		gtk_entry_set_text(GTK_ENTRY(entry), _(text));

	gtk_entry_set_editable(GTK_ENTRY(entry), editable);
	gtk_entry_set_visibility(GTK_ENTRY(entry), text_visible);
	return entry;
}

static GtkWidget *
text_new(GladeXML *xml, GladeWidgetInfo *info)
{
	GtkWidget *wid;
	GList *tmp;
	char *text = NULL;
	gboolean editable = TRUE;

	for (tmp = info->attributes; tmp; tmp = tmp->next) {
		GladeAttribute *attr = tmp->data;
		switch (attr->name[0]) {
		case 'e':
			if (!strcmp(attr->name, "editable"))
				editable = attr->value[0] == 'T';
			break;
		case 't':
			if (!strcmp(attr->name, "text"))
				text = attr->value;
			break;
		}
	}

	wid = gtk_text_new(NULL, NULL);
	if (text) {
		char *tmp = _(text);
		gint pos = 0;
		gtk_editable_insert_text(GTK_EDITABLE(wid), tmp, strlen(tmp),
					 &pos);
	}
	gtk_text_set_editable(GTK_TEXT(wid), editable);
	return wid;
}

static GtkWidget *
button_new(GladeXML *xml, GladeWidgetInfo *info)
{
	GtkWidget *button;
	GList *tmp;
	char *string = NULL;
  
	/*
	 * This should really be a container, but GLADE is weird in this
	 * respect.  If the label property is set for this widget, insert
	 * a label.  Otherwise, allow a widget to be packed
	 */
	for (tmp = info->attributes; tmp; tmp = tmp->next) {
		GladeAttribute *attr = tmp->data;
		if (!strcmp(attr->name, "label"))
			string = attr->value;
	}
	if (string != NULL) {
		guint key;
		
		button = gtk_button_new_with_label("");
		key = gtk_label_parse_uline(GTK_LABEL(GTK_BIN(button)->child),
					    string[0] ? _(string) : "");
		if (key)
			gtk_widget_add_accelerator(button, "clicked",
						   glade_xml_ensure_accel(xml),
						   key, GDK_MOD1_MASK, 0);
	} else
		button = gtk_button_new();
	return button;
}

static GtkWidget *
togglebutton_new(GladeXML *xml, GladeWidgetInfo *info)
{
	GtkWidget *button;
	GList *tmp;
	char *string = NULL;
	gboolean active = FALSE;

	for (tmp = info->attributes; tmp; tmp = tmp->next) {
		GladeAttribute *attr = tmp->data;

		if (!strcmp(attr->name, "label"))
			string = attr->value;
		else if (!strcmp(attr->name, "active"))
			active = attr->value[0] == 'T';
	}
	if (string != NULL) {
		guint key;
		
		button = gtk_toggle_button_new_with_label("");
		key = gtk_label_parse_uline(GTK_LABEL(GTK_BIN(button)->child),
					    string[0] ? _(string) : "");
		if (key)
			gtk_widget_add_accelerator(button, "clicked",
						   glade_xml_ensure_accel(xml),
						   key, GDK_MOD1_MASK, 0);
	} else
		button = gtk_toggle_button_new();
	gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(button), active);
	return button;
}

static GtkWidget *
checkbutton_new (GladeXML *xml, GladeWidgetInfo *info)
{
	GtkWidget *button;
	GList *tmp;
	char *string = NULL;
	gboolean active = FALSE, draw_indicator = TRUE;

	for (tmp = info->attributes; tmp; tmp = tmp->next) {
		GladeAttribute *attr = tmp->data;

		if (!strcmp(attr->name, "label"))
			string = attr->value;
		else if (!strcmp(attr->name, "active"))
			active = attr->value[0] == 'T';
		else if (!strcmp(attr->name, "draw_indicator"))
			draw_indicator = attr->value[0] == 'T';
	}
	if (string != NULL) {
		guint key;
		
		button = gtk_check_button_new_with_label("");
		key = gtk_label_parse_uline(GTK_LABEL(GTK_BIN(button)->child),
					    string[0] ? _(string) : "");
		if (key)
			gtk_widget_add_accelerator(button, "clicked",
						   glade_xml_ensure_accel(xml),
						   key, GDK_MOD1_MASK, 0);
	} else
		button = gtk_check_button_new();

	gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(button), active);
	gtk_toggle_button_set_mode(GTK_TOGGLE_BUTTON(button), draw_indicator);
	return button;
}

static GtkWidget *
radiobutton_new(GladeXML *xml, GladeWidgetInfo *info)
{
	GtkWidget *button;
	GList *tmp;
	char *string = NULL;
	gboolean active = FALSE, draw_indicator = TRUE;
	GSList *group = NULL;
	char *group_name = NULL;
	
	for (tmp = info->attributes; tmp; tmp = tmp->next) {
		GladeAttribute *attr = tmp->data;

		if (!strcmp(attr->name, "label"))
			string = attr->value;
		else if (!strcmp(attr->name, "active"))
			active = attr->value[0] == 'T';
		else if (!strcmp(attr->name, "draw_indicator"))
			draw_indicator = attr->value[0] == 'T';
		else if (!strcmp(attr->name, "group")){
			group_name = attr->value;
			group = g_hash_table_lookup (xml->priv->radio_groups,
						     group_name);
			if (!group)
				group_name = g_strdup(group_name);
		}
	}
	if (string != NULL) {
		guint key;
		
		button = gtk_radio_button_new_with_label(group, "");
		key = gtk_label_parse_uline(GTK_LABEL(GTK_BIN(button)->child),
					    string[0] ? _(string) : "");
		if (key)
			gtk_widget_add_accelerator(button, "clicked",
						   glade_xml_ensure_accel(xml),
						   key, GDK_MOD1_MASK, 0);
	} else
		button = gtk_radio_button_new (group);

	if (group_name) {
		GtkRadioButton *radio = GTK_RADIO_BUTTON (button);
		
		g_hash_table_insert (xml->priv->radio_groups,
				     group_name,
				     gtk_radio_button_group (radio));
	} 

	gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(button), active);
	gtk_toggle_button_set_mode(GTK_TOGGLE_BUTTON(button), draw_indicator);
	return button;
}

static GtkWidget *
optionmenu_new(GladeXML *xml, GladeWidgetInfo *info)
{
	GtkWidget *option = gtk_option_menu_new(), *menu;
	GList *tmp;
	int initial_choice = 0;

	menu = gtk_menu_new();
	for (tmp = info->attributes; tmp; tmp = tmp->next) {
		GladeAttribute *attr = tmp->data;

		if (!strcmp(attr->name, "items")) {
			gchar **items = g_strsplit(attr->value, "\n", 0);
			int i = 0;
			for (i = 0; items[i] != NULL; i++) {
				GtkWidget *menuitem =
					gtk_menu_item_new_with_label(
								_(items[i]));
				gtk_widget_show (menuitem);
				gtk_menu_append (GTK_MENU (menu), menuitem);
			}
			g_strfreev(items);
		} else if (!strcmp(attr->name, "initial_choice"))
			initial_choice = strtol(attr->value, NULL, 0);
	}
	gtk_option_menu_set_menu(GTK_OPTION_MENU(option), menu);
	gtk_option_menu_set_history(GTK_OPTION_MENU(option), initial_choice);
	return option;
}

static GtkWidget *
combo_new (GladeXML *xml, GladeWidgetInfo *info)
{
	GtkWidget *combo = gtk_combo_new();
	GList *tmp;

	for (tmp = info->attributes; tmp; tmp = tmp->next) {
		GladeAttribute *attr = tmp->data;

		switch (attr->name[0]) {
		case 'c':
			if (!strcmp(attr->name, "case_sensitive"))
				gtk_combo_set_case_sensitive(GTK_COMBO(combo),
							attr->value[0] == 'T');
			break;
		case 'i':
			if (!strcmp(attr->name, "items")) {
				GList *item_list = NULL;
				gchar **items = g_strsplit(attr->value,"\n",0);
				int i = 0;
				for (i = 0; items[i] != NULL; i++)
					item_list = g_list_append(item_list,
								  _(items[i]));
				if (item_list)
					gtk_combo_set_popdown_strings(
						GTK_COMBO(combo), item_list);
				g_list_free(item_list);
				g_strfreev(items);
			}
			break;
		case 'u':
			if (!strcmp(attr->name, "use_arrows"))
				gtk_combo_set_use_arrows(GTK_COMBO(combo),
							attr->value[0] == 'T');
			else if (!strcmp(attr->name, "use_arrows_always"))
				gtk_combo_set_use_arrows_always(
					GTK_COMBO(combo), attr->value[0]=='T');
			break;
		}
	}

	return combo;
}

static GtkWidget *
list_new(GladeXML *xml, GladeWidgetInfo *info)
{
	GtkWidget *list = gtk_list_new();
	GList *tmp;

	for (tmp = info->attributes; tmp; tmp = tmp->next) {
		GladeAttribute *attr = tmp->data;

		if (!strcmp(attr->name, "selection_mode"))
			gtk_list_set_selection_mode(GTK_LIST(list),
				glade_enum_from_string(GTK_TYPE_SELECTION_MODE,
						       attr->value));
	}
	return list;
}

static GtkWidget *
clist_new(GladeXML *xml, GladeWidgetInfo *info)
{
	GtkWidget *clist;
	GList *tmp;
	int cols = 1;

	for (tmp = info->attributes; tmp; tmp = tmp->next) {
		GladeAttribute *attr = tmp->data;
		if (!strcmp(attr->name, "columns")) {
			cols = strtol(attr->value, NULL, 0);
			break;
		}
	}

	clist = gtk_clist_new(cols);
	cols = 0;
	for (tmp = info->attributes; tmp; tmp = tmp->next) {
		GladeAttribute *attr = tmp->data;

		switch (attr->name[0]) {
		case 'c':
			if (!strcmp(attr->name, "column_widths")) {
				char *pos = attr->value;
				while (pos && *pos != '\0') {
					int width = strtol(pos, &pos, 0);
					if (*pos == ',') pos++;
					gtk_clist_set_column_width(GTK_CLIST(clist), cols++, width);
				}
			}
			break;
		case 's':
			if (!strcmp(attr->name, "selection_mode"))
				gtk_clist_set_selection_mode(GTK_CLIST(clist),
					glade_enum_from_string(
						GTK_TYPE_SELECTION_MODE,
						attr->value));
			else if (!strcmp(attr->name, "shadow_type"))
				gtk_clist_set_shadow_type(GTK_CLIST(clist),
					glade_enum_from_string(
						GTK_TYPE_SHADOW_TYPE,
						attr->value));
			else if (!strcmp(attr->name, "show_titles")) {
				if (attr->value[0] == 'T')
					gtk_clist_column_titles_show(GTK_CLIST(clist));
				else
					gtk_clist_column_titles_hide(GTK_CLIST(clist));
			}
			break;
		}
	}
	return clist;
}

static GtkWidget *
ctree_new(GladeXML *xml, GladeWidgetInfo *info)
{
	GtkWidget *ctree;
	GList *tmp;
	int cols = 1;

	for (tmp = info->attributes; tmp; tmp = tmp->next) {
		GladeAttribute *attr = tmp->data;
		if (!strcmp(attr->name, "columns")) {
			cols = strtol(attr->value, NULL, 0);
			break;
		}
	}

	ctree = gtk_ctree_new(cols, 0);
	cols = 0;
	for (tmp = info->attributes; tmp; tmp = tmp->next) {
		GladeAttribute *attr = tmp->data;

		switch (attr->name[0]) {
		case 'c':
			if (!strcmp(attr->name, "column_widths")) {
				char *pos = attr->value;
				while (pos && *pos != '\0') {
					int width = strtol(pos, &pos, 0);
					if (*pos == ',') pos++;
					gtk_clist_set_column_width(GTK_CLIST(ctree), cols++, width);
				}
			}
			break;
		case 's':
			if (!strcmp(attr->name, "selection_mode"))
				gtk_clist_set_selection_mode(GTK_CLIST(ctree),
					glade_enum_from_string(
						GTK_TYPE_SELECTION_MODE,
						attr->value));
			else if (!strcmp(attr->name, "shadow_type"))
				gtk_clist_set_shadow_type(GTK_CLIST(ctree),
					glade_enum_from_string(
						GTK_TYPE_SHADOW_TYPE,
						attr->value));
			else if (!strcmp(attr->name, "show_titles")) {
				if (attr->value[0] == 'T')
					gtk_clist_column_titles_show(GTK_CLIST(ctree));
				else
					gtk_clist_column_titles_hide(GTK_CLIST(ctree));
			}
			break;
		}
	}
	return ctree;
}

static GtkWidget *
tree_new(GladeXML *xml, GladeWidgetInfo *info)
{
	GtkWidget *tree = gtk_tree_new();
	GList *tmp;

	for (tmp = info->attributes; tmp; tmp = tmp->next) {
		GladeAttribute *attr = tmp->data;

		if (!strcmp(attr->name, "selection_mode"))
			gtk_tree_set_selection_mode(GTK_TREE(tree),
				glade_enum_from_string(GTK_TYPE_SELECTION_MODE,
						       attr->value));
		else if (!strcmp(attr->name, "view_mode"))
			gtk_tree_set_view_mode(GTK_TREE(tree),
				glade_enum_from_string(GTK_TYPE_TREE_VIEW_MODE,
						       attr->value));
		else if (!strcmp(attr->name, "view_line"))
			gtk_tree_set_view_lines(GTK_TREE(tree),
						attr->value[0] == 'T');
	}
	return tree;
}

static GtkWidget *
spinbutton_new(GladeXML *xml, GladeWidgetInfo *info)
{
	GtkWidget *spinbutton;
	GtkAdjustment *adj = glade_get_adjustment(info);
	GList *tmp;
	int climb_rate = 1, digits = 0;
	gboolean numeric = FALSE, snap = FALSE, wrap = FALSE;
	GtkSpinButtonUpdatePolicy update_policy = GTK_UPDATE_IF_VALID;
  
	for (tmp = info->attributes; tmp; tmp = tmp->next) {
		GladeAttribute *attr = tmp->data;

		switch (attr->name[0]) {
		case 'c':
			if (!strcmp(attr->name, "climb_rate"))
				climb_rate = strtol(attr->value, NULL, 0);
			break;
		case 'd':
			if (!strcmp(attr->name, "digits"))
				digits = strtol(attr->value, NULL, 0);
			break;
		case 'n':
			if (!strcmp(attr->name, "numeric"))
				numeric = attr->value[0] == 'T';
			break;
		case 's':
			if (!strcmp(attr->name, "snap"))
				snap = attr->value[0] == 'T';
			break;
		case 'u':
			if (!strcmp(attr->name, "update_policy"))
				update_policy = glade_enum_from_string(
					GTK_TYPE_SPIN_BUTTON_UPDATE_POLICY,
					attr->value);
			break;
		case 'w':
			if (!strcmp(attr->name, "wrap"))
				wrap = attr->value[0] == 'T';
			break;
		}
	}

	spinbutton = gtk_spin_button_new(adj, climb_rate, digits);
	gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(spinbutton), numeric);
	gtk_spin_button_set_update_policy(GTK_SPIN_BUTTON(spinbutton),update_policy);
	gtk_spin_button_set_snap_to_ticks(GTK_SPIN_BUTTON(spinbutton), snap);
	gtk_spin_button_set_wrap(GTK_SPIN_BUTTON(spinbutton), wrap);
	return spinbutton;
}

static GtkWidget *
hscale_new(GladeXML *xml, GladeWidgetInfo *info)
{
	GtkAdjustment *adj = glade_get_adjustment(info);
	GtkWidget *scale = gtk_hscale_new(adj);
	GList *tmp;

	for (tmp = info->attributes; tmp; tmp = tmp->next) {
		GladeAttribute *attr = tmp->data;

		switch (attr->name[0]) {
		case 'd':
			if (!strcmp(attr->name, "digits"))
				gtk_scale_set_digits(GTK_SCALE(scale),
						     strtol(attr->value,
							    NULL, 0));
			else if (!strcmp(attr->name, "draw_value"))
				gtk_scale_set_draw_value(GTK_SCALE(scale),
							 attr->value[0]=='T');
			break;
		case 'p':
			if (!strcmp(attr->name, "policy"))
				gtk_range_set_update_policy(GTK_RANGE(scale),
					glade_enum_from_string(
						GTK_TYPE_UPDATE_TYPE,
						attr->value));
			break;
		case 'v':
			if (!strcmp(attr->name, "value_pos"))
				gtk_scale_set_value_pos(GTK_SCALE(scale),
					glade_enum_from_string(
						GTK_TYPE_POSITION_TYPE,
						attr->value));
			break;
		}
	}
	return scale;
}

static GtkWidget *
vscale_new (GladeXML *xml, GladeWidgetInfo *info)
{
	GtkAdjustment *adj = glade_get_adjustment(info);
	GtkWidget *scale = gtk_vscale_new(adj);
	GList *tmp;

	for (tmp = info->attributes; tmp; tmp = tmp->next) {
		GladeAttribute *attr = tmp->data;

		switch (attr->name[0]) {
		case 'd':
			if (!strcmp(attr->name, "digits"))
				gtk_scale_set_digits(GTK_SCALE(scale),
						     strtol(attr->value,
							    NULL, 0));
			else if (!strcmp(attr->name, "draw_value"))
				gtk_scale_set_draw_value(GTK_SCALE(scale),
							 attr->value[0]=='T');
			break;
		case 'p':
			if (!strcmp(attr->name, "policy"))
				gtk_range_set_update_policy(GTK_RANGE(scale),
					glade_enum_from_string(
						GTK_TYPE_UPDATE_TYPE,
						attr->value));
			break;
		case 'v':
			if (!strcmp(attr->name, "value_pos"))
				gtk_scale_set_value_pos(GTK_SCALE(scale),
					glade_enum_from_string(
						GTK_TYPE_POSITION_TYPE,
						attr->value));
			break;
		}
	}
	return scale;
}

static GtkWidget *
hruler_new(GladeXML *xml, GladeWidgetInfo *info)
{
	GtkWidget *ruler = gtk_hruler_new();
	GList *tmp;
	gdouble lower = 0, upper = 10, pos = 0, max = 10;

	for (tmp = info->attributes; tmp; tmp = tmp->next) {
		GladeAttribute *attr = tmp->data;

		switch (attr->name[0]) {
		case 'l':
			if (!strcmp(attr->name, "lower"))
				lower = g_strtod(attr->value, NULL);
			break;
		case 'm':
			if (!strcmp(attr->name, "max"))
				max = g_strtod(attr->value, NULL);
			else if (!strcmp(attr->name, "metric"))
				gtk_ruler_set_metric(GTK_RULER(ruler),
					glade_enum_from_string(
						GTK_TYPE_METRIC_TYPE,
						attr->value));
			break;
		case 'p':
			if (!strcmp(attr->name, "pos"))
				pos = g_strtod(attr->value, NULL);
			break;
		case 'u':
			if (!strcmp(attr->name, "upper"))
				upper = g_strtod(attr->value, NULL);
			break;
		}
	}
	gtk_ruler_set_range(GTK_RULER(ruler), lower, upper, pos, max);
	return ruler;
}

static GtkWidget *
vruler_new(GladeXML *xml, GladeWidgetInfo *info)
{
	GtkWidget *ruler = gtk_vruler_new();
	GList *tmp;
	gdouble lower = 0, upper = 10, pos = 0, max = 10;

	for (tmp = info->attributes; tmp; tmp = tmp->next) {
		GladeAttribute *attr = tmp->data;

		switch (attr->name[0]) {
		case 'l':
			if (!strcmp(attr->name, "lower"))
				lower = g_strtod(attr->value, NULL);
			break;
		case 'm':
			if (!strcmp(attr->name, "max"))
				max = g_strtod(attr->value, NULL);
			else if (!strcmp(attr->name, "metric"))
				gtk_ruler_set_metric(GTK_RULER(ruler),
					glade_enum_from_string(
						GTK_TYPE_METRIC_TYPE,
						attr->value));
			break;
		case 'p':
			if (!strcmp(attr->name, "pos"))
				pos = g_strtod(attr->value, NULL);
			break;
		case 'u':
			if (!strcmp(attr->name, "upper"))
				upper = g_strtod(attr->value, NULL);
			break;
		}
	}
	gtk_ruler_set_range(GTK_RULER(ruler), lower, upper, pos, max);
	return ruler;
}

static GtkWidget *
hscrollbar_new(GladeXML *xml, GladeWidgetInfo *info)
{
	GtkAdjustment *adj = glade_get_adjustment(info);
	GtkWidget *scroll = gtk_hscrollbar_new(adj);
	GList *tmp;

	for (tmp = info->attributes; tmp; tmp = tmp->next) {
		GladeAttribute *attr = tmp->data;

		if (!strcmp(attr->name, "policy"))
			gtk_range_set_update_policy(GTK_RANGE(scroll),
				glade_enum_from_string(GTK_TYPE_UPDATE_TYPE,
						       attr->value));
	}
	return scroll;
}

static GtkWidget *
vscrollbar_new(GladeXML *xml, GladeWidgetInfo *info)
{
	GtkAdjustment *adj = glade_get_adjustment(info);
	GtkWidget *scroll = gtk_vscrollbar_new(adj);
	GList *tmp;

	for (tmp = info->attributes; tmp; tmp = tmp->next) {
		GladeAttribute *attr = tmp->data;

		if (!strcmp(attr->name, "policy"))
			gtk_range_set_update_policy(GTK_RANGE(scroll),
				glade_enum_from_string(GTK_TYPE_UPDATE_TYPE,
						       attr->value));
	}
	return scroll;
}

static GtkWidget *
statusbar_new(GladeXML *xml, GladeWidgetInfo *info)
{
	return gtk_statusbar_new();
}

static GtkWidget *
toolbar_new(GladeXML *xml, GladeWidgetInfo *info)
{
	GtkWidget *tool;
	GList *tmp;
	GtkOrientation orient = GTK_ORIENTATION_HORIZONTAL;
	GtkToolbarStyle style = GTK_TOOLBAR_BOTH;
	GtkToolbarSpaceStyle spaces = GTK_TOOLBAR_SPACE_EMPTY;
	int space_size = 5;
	gboolean tooltips = TRUE;

	for (tmp = info->attributes; tmp; tmp = tmp->next) {
		GladeAttribute *attr = tmp->data;

		switch (attr->name[0]) {
		case 'o':
			if (!strcmp(attr->name, "orientation"))
				orient = glade_enum_from_string(
					GTK_TYPE_ORIENTATION, attr->value);
			break;
		case 's':
			if (!strcmp(attr->name, "space_size"))
				space_size = strtol(attr->value, NULL, 0);
			else if (!strcmp(attr->name, "space_style"))
				spaces = glade_enum_from_string(
					GTK_TYPE_TOOLBAR_SPACE_STYLE,
					attr->value);
			break;
		case 't':
			if (!strcmp(attr->name, "type"))
				style = glade_enum_from_string(
					GTK_TYPE_TOOLBAR_STYLE, attr->value);
			else if (!strcmp(attr->name, "tooltips"))
				tooltips = attr->value[0] == 'T';
			break;
		}
	}
	tool = gtk_toolbar_new(orient, style);
	gtk_toolbar_set_space_size(GTK_TOOLBAR(tool), space_size);
	gtk_toolbar_set_space_style(GTK_TOOLBAR(tool), spaces);
	gtk_toolbar_set_tooltips(GTK_TOOLBAR(tool), tooltips);
	return tool;
}

static GtkWidget *
progressbar_new(GladeXML *xml, GladeWidgetInfo *info)
{
	GtkWidget *ret = gtk_progress_bar_new();
	GList *tmp;
	gfloat value = 0, lower = 0, upper = 100;
	gfloat xalign = 0.5, yalign = 0.5;

	for (tmp = info->attributes; tmp; tmp = tmp->next) {
		GladeAttribute *attr = tmp->data;

		if (!strcmp(attr->name, "value"))
			value = g_strtod(attr->value, NULL);
		else if (!strcmp(attr->name, "lower"))
			lower = g_strtod(attr->value, NULL);
		else if (!strcmp(attr->name, "upper"))
			upper = g_strtod(attr->value, NULL);
		else if (!strcmp(attr->name, "activity_mode"))
			gtk_progress_set_activity_mode(GTK_PROGRESS(ret),
						       attr->value[0]=='T');
		else if (!strcmp(attr->name, "bar_style"))
			gtk_progress_bar_set_bar_style(GTK_PROGRESS_BAR(ret),
			    glade_enum_from_string(GTK_TYPE_PROGRESS_BAR_STYLE,
						   attr->value));
		else if (!strcmp(attr->name, "orientation"))
			gtk_progress_bar_set_orientation(GTK_PROGRESS_BAR(ret),
			    glade_enum_from_string(
			      GTK_TYPE_PROGRESS_BAR_ORIENTATION, attr->value));
		else if (!strcmp(attr->name, "show_text"))
			gtk_progress_set_show_text(GTK_PROGRESS(ret),
						   attr->value[0] == 'T');
		else if (!strcmp(attr->name, "text_xalign"))
			xalign = g_strtod(attr->value, NULL);
		else if (!strcmp(attr->name, "text_yalign"))
			yalign = g_strtod(attr->value, NULL);
		else if (!strcmp(attr->name, "format"))
			gtk_progress_set_format_string(GTK_PROGRESS(ret),
						       attr->value);
	}
	gtk_progress_configure(GTK_PROGRESS(ret), value, lower, upper);
	gtk_progress_set_text_alignment(GTK_PROGRESS(ret), xalign, yalign);
	return ret;
}

static GtkWidget *
arrow_new(GladeXML *xml, GladeWidgetInfo *info)
{
	GtkWidget *arrow;
	GList *tmp;
	GtkArrowType arrow_type = GTK_ARROW_RIGHT;
	GtkShadowType shadow_type = GTK_SHADOW_OUT;

	for (tmp = info->attributes; tmp; tmp = tmp->next) {
		GladeAttribute *attr = tmp->data;

		if (!strcmp(attr->name, "arrow_type"))
			arrow_type =glade_enum_from_string(GTK_TYPE_ARROW_TYPE,
							   attr->value);
		else if (!strcmp(attr->name, "shadow_type"))
			shadow_type = glade_enum_from_string(
					GTK_TYPE_SHADOW_TYPE, attr->value);
	}
	arrow = gtk_arrow_new(arrow_type, shadow_type);
	misc_set(GTK_MISC(arrow), info);
	return arrow;
}

static GtkWidget *
pixmap_new(GladeXML *xml, GladeWidgetInfo *info)
{
	GtkWidget *pix;
	GList *tmp;
	GdkPixmap *pixmap;
	GdkBitmap *bitmap = NULL;
	char *filename = NULL;
  
	for (tmp = info->attributes; tmp; tmp = tmp->next) {
		GladeAttribute *attr = tmp->data;

		if (!strcmp(attr->name, "filename")) {
			if (filename) g_free(filename);
			filename = glade_xml_relative_file(xml, attr->value);
			break;
		}
	}
	pixmap = gdk_pixmap_colormap_create_from_xpm(NULL,
		gtk_widget_get_default_colormap(), &bitmap, NULL, filename);
	if (filename)
		g_free(filename);
	if (pixmap)
		pix = gtk_pixmap_new(pixmap, bitmap);
	else
		pix = gtk_type_new(gtk_pixmap_get_type());

	if (pixmap) gdk_pixmap_unref(pixmap);
	if (bitmap) gdk_bitmap_unref(bitmap);
	misc_set(GTK_MISC(pix), info);

	return pix;
}

static GtkWidget *
drawingarea_new (GladeXML *xml, GladeWidgetInfo *info)
{
	return gtk_drawing_area_new();
}

static GtkWidget *
hseparator_new(GladeXML *xml, GladeWidgetInfo *info)
{
	return gtk_hseparator_new();
}

static GtkWidget *
vseparator_new(GladeXML *xml, GladeWidgetInfo *info)
{
	return gtk_vseparator_new();
}

static GtkWidget *
menubar_new(GladeXML *xml, GladeWidgetInfo *info)
{
	return gtk_menu_bar_new();
}

static GtkWidget *
menu_new(GladeXML *xml, GladeWidgetInfo *info)
{
	return gtk_menu_new();
}

static GtkWidget *
menuitem_new(GladeXML *xml, GladeWidgetInfo *info)
{
	GtkWidget *menuitem;
	GList *tmp;
	char *label = NULL;
	gboolean right = FALSE;

	for (tmp = info->attributes; tmp; tmp = tmp->next) {
		GladeAttribute *attr = tmp->data;

		if (!strcmp(attr->name, "label"))
			label = attr->value;
		else if (!strcmp(attr->name, "right_justify"))
			right = attr->value[0] == 'T';
	}
	if (label) {
		GtkAccelGroup *accel;
		guint key;
		char *s = label [0] ? _(label) : "";
		menuitem = gtk_menu_item_new_with_label("");
		key = gtk_label_parse_uline(
				GTK_LABEL(GTK_BIN(menuitem)->child), s);
		if (key) {
			accel = glade_xml_get_uline_accel(xml);
			if (accel)
				gtk_widget_add_accelerator(menuitem,
							   "activate_item",
							   accel, key, 0,
							   0);
			else {
				/* not inside a GtkMenu -- must be on menubar*/
				accel = glade_xml_ensure_accel(xml);
				gtk_widget_add_accelerator(menuitem,
							   "activate_item",
							   accel, key,
							   GDK_MOD1_MASK, 0);
			}
		}
	} else
		menuitem = gtk_menu_item_new();

	if (right)
		gtk_menu_item_right_justify(GTK_MENU_ITEM(menuitem));
	return menuitem;
}

static GtkWidget *
checkmenuitem_new(GladeXML *xml, GladeWidgetInfo *info)
{
	GtkWidget *menuitem;
	GtkAccelGroup *accel;
	guint key;
	GList *tmp;
	char *label = NULL;
	gboolean right = FALSE, active = FALSE, toggle = FALSE;

	for (tmp = info->attributes; tmp; tmp = tmp->next) {
		GladeAttribute *attr = tmp->data;

		if (!strcmp(attr->name, "label"))
			label = attr->value;
		else if (!strcmp(attr->name, "right_justify"))
			right = attr->value[0] == 'T';
		else if (!strcmp(attr->name, "active"))
			active = attr->value[0] == 'T';
		else if (!strcmp(attr->name, "always_show_toggle"))
			toggle = attr->value[0] == 'T';
	}
	menuitem = gtk_check_menu_item_new_with_label("");
	if (label){
		char *s = label [0] ? _(label) : "";
		
		key = gtk_label_parse_uline(
			GTK_LABEL(GTK_BIN(menuitem)->child), s);
	} else
		key = 0;
	
	if (key) {
		accel = glade_xml_get_uline_accel(xml);
		if (accel)
			gtk_widget_add_accelerator(menuitem,
						   "activate_item",
						   accel, key, 0,
						   0);
		else {
			/* not inside a GtkMenu -- must be on menubar*/
			accel = glade_xml_ensure_accel(xml);
			gtk_widget_add_accelerator(menuitem,
						   "activate_item",
						   accel, key,
						   GDK_MOD1_MASK, 0);
		}
	}
	if (right)
		gtk_menu_item_right_justify(GTK_MENU_ITEM(menuitem));
	gtk_check_menu_item_set_state(GTK_CHECK_MENU_ITEM(menuitem), active);
	gtk_check_menu_item_set_show_toggle(GTK_CHECK_MENU_ITEM(menuitem), toggle);

	return menuitem;
}

static GtkWidget *
radiomenuitem_new(GladeXML *xml, GladeWidgetInfo *info)
{
	GtkWidget *menuitem;
	GtkAccelGroup *accel;
	guint key;
	GList *tmp;
	char *label = NULL;
	gboolean right = FALSE, active = FALSE, toggle = FALSE;
	GSList *group = NULL;
	char *group_name = NULL;

	for (tmp = info->attributes; tmp; tmp = tmp->next) {
		GladeAttribute *attr = tmp->data;

		if (!strcmp(attr->name, "label"))
			label = attr->value;
		else if (!strcmp(attr->name, "right_justify"))
			right = attr->value[0] == 'T';
		else if (!strcmp(attr->name, "active"))
			active = attr->value[0] == 'T';
		else if (!strcmp(attr->name, "always_show_toggle"))
			toggle = attr->value[0] == 'T';
		else if (!strcmp(attr->name, "group")) {
			group_name = attr->value;
			group = g_hash_table_lookup(xml->priv->radio_groups,
						    group_name);
			if (!group)
				group_name = g_strdup(group_name);
		}
	}

	menuitem = gtk_radio_menu_item_new_with_label(group, "");
	if (label){
		char *s = label [0] ? _(label) : "";
		
		key = gtk_label_parse_uline(
			GTK_LABEL(GTK_BIN(menuitem)->child), s);
	} else
		key = 0;
	if (key) {
		accel = glade_xml_get_uline_accel(xml);
		if (accel)
			gtk_widget_add_accelerator(menuitem,
						   "activate_item",
						   accel, key, 0,
						   0);
		else {
			/* not inside a GtkMenu -- must be on menubar*/
			accel = glade_xml_ensure_accel(xml);
			gtk_widget_add_accelerator(menuitem,
						   "activate_item",
						   accel, key,
						   GDK_MOD1_MASK, 0);
		}
	}

	if (right)
		gtk_menu_item_right_justify(GTK_MENU_ITEM(menuitem));
	if (group_name) {
		GtkRadioMenuItem *radio = GTK_RADIO_MENU_ITEM(menuitem);
		g_hash_table_insert(xml->priv->radio_groups,
				    group_name,
				    gtk_radio_menu_item_group(radio));
	}
	gtk_check_menu_item_set_state(GTK_CHECK_MENU_ITEM(menuitem), active);
	gtk_check_menu_item_set_show_toggle(GTK_CHECK_MENU_ITEM(menuitem), toggle);

	return menuitem;
}

static GtkWidget *
hbox_new(GladeXML *xml, GladeWidgetInfo *info)
{
	GList *tmp;
	int spacing = 0;
	gboolean homog = FALSE;

	for (tmp = info->attributes; tmp; tmp = tmp->next) {
		GladeAttribute *attr = tmp->data;

		if (!strcmp(attr->name, "homogeneous"))
			homog = attr->value[0] == 'T';
		else if (!strcmp(attr->name, "spacing"))
			spacing = strtol(attr->value, NULL, 0);
	}
	return gtk_hbox_new (homog, spacing);
}

static GtkWidget *vbox_new (GladeXML *xml, GladeWidgetInfo *info)
{
	GList *tmp;
	int spacing = 0;
	gboolean homog = FALSE;

	for (tmp = info->attributes; tmp; tmp = tmp->next) {
		GladeAttribute *attr = tmp->data;
		
		if (!strcmp(attr->name, "homogeneous"))
			homog = attr->value[0] == 'T';
		else if (!strcmp(attr->name, "spacing"))
			spacing = strtol(attr->value, NULL, 0);
	}
	return gtk_vbox_new(homog, spacing);
}

static GtkWidget *
table_new(GladeXML *xml, GladeWidgetInfo *info)
{
	GtkWidget *table;
	GList *tmp;
	int rows = 1, cols = 1, rspace = 0, cspace = 0;
	gboolean homog = FALSE;

	for (tmp = info->attributes; tmp; tmp = tmp->next) {
		GladeAttribute *attr = tmp->data;

		switch (attr->name[0]) {
		case 'c':
			if (!strcmp(attr->name, "columns"))
				cols = strtol(attr->value, NULL, 0);
			else if (!strcmp(attr->name, "column_spacing"))
				cspace = strtol(attr->value, NULL, 0);
			break;
		case 'h':
			if (!strcmp(attr->name, "homogeneous"))
				homog = attr->value[0] == 'T';
			break;
		case 'r':
			if (!strcmp(attr->name, "rows"))
				rows = strtol(attr->value, NULL, 0);
			else if (!strcmp(attr->name, "row_spacing"))
				rspace = strtol(attr->value, NULL, 0);
			break;
		}
	}

	table = gtk_table_new(rows, cols, homog);
	gtk_table_set_row_spacings(GTK_TABLE(table), rspace);
	gtk_table_set_col_spacings(GTK_TABLE(table), cspace);
	return table;
}

static GtkWidget *
fixed_new(GladeXML *xml, GladeWidgetInfo *info)
{
	return gtk_fixed_new();
}

static GtkWidget *
layout_new(GladeXML *xml, GladeWidgetInfo *info)
{
	GtkWidget *wid = gtk_layout_new(NULL, NULL);
	GList *tmp;
	guint width = 400, height = 400;

	for (tmp = info->attributes; tmp; tmp = tmp->next) {
		GladeAttribute *attr = tmp->data;

		if (!strcmp(attr->name, "area_width"))
			width = strtoul(attr->value, NULL, 0);
		else if (!strcmp(attr->name, "area_height"))
			width = strtoul(attr->value, NULL, 0);
		else if (!strcmp(attr->name, "hstep"))
			GTK_ADJUSTMENT(GTK_LAYOUT(wid)->hadjustment)->
				step_increment = g_strtod(attr->value, NULL);
		else if (!strcmp(attr->name, "vstep"))
			GTK_ADJUSTMENT(GTK_LAYOUT(wid)->vadjustment)->
				step_increment = g_strtod(attr->value, NULL);
	}
	gtk_layout_set_size(GTK_LAYOUT(wid), width, height);
	return wid;
}

static GtkWidget *
hbuttonbox_new(GladeXML *xml, GladeWidgetInfo *info)
{
	GtkWidget *bbox = gtk_hbutton_box_new();
	GList *tmp;
	int minw, minh, ipx, ipy;

	gtk_button_box_get_child_size_default(&minw, &minh);
	gtk_button_box_get_child_ipadding_default(&ipx, &ipy);

	for (tmp = info->attributes; tmp; tmp = tmp->next) {
		GladeAttribute *attr = tmp->data;

		switch (attr->name[0]) {
		case 'c':
			if (!strcmp(attr->name, "child_min_width"))
				minw = strtol(attr->value, NULL, 0);
			else if (!strcmp(attr->name, "child_min_height"))
				minh = strtol(attr->value, NULL, 0);
			else if (!strcmp(attr->name, "child_ipad_x"))
				ipx = strtol(attr->value, NULL, 0);
			else if (!strcmp(attr->name, "child_ipad_y"))
				ipy = strtol(attr->value, NULL, 0);
			break;
		case 'l':
			if (!strcmp(attr->name, "layout_style"))
				gtk_button_box_set_layout(GTK_BUTTON_BOX(bbox),
					glade_enum_from_string(
						GTK_TYPE_BUTTON_BOX_STYLE,
						attr->value));
			break;
		case 's':
			if (!strcmp(attr->name, "spacing"))
			       gtk_button_box_set_spacing(GTK_BUTTON_BOX(bbox),
						strtol(attr->value, NULL, 0));
			break;
		}
	}
	gtk_button_box_set_child_size(GTK_BUTTON_BOX(bbox), minw, minh);
	gtk_button_box_set_child_ipadding(GTK_BUTTON_BOX(bbox), ipx, ipy);
	return bbox;
}

static GtkWidget *
vbuttonbox_new(GladeXML *xml, GladeWidgetInfo *info)
{
	GtkWidget *bbox = gtk_vbutton_box_new();
	GList *tmp;
	int minw, minh, ipx, ipy;

	gtk_button_box_get_child_size_default(&minw, &minh);
	gtk_button_box_get_child_ipadding_default(&ipx, &ipy);

	for (tmp = info->attributes; tmp; tmp = tmp->next) {
		GladeAttribute *attr = tmp->data;

		switch (attr->name[0]) {
		case 'c':
			if (!strcmp(attr->name, "child_min_width"))
				minw = strtol(attr->value, NULL, 0);
			else if (!strcmp(attr->name, "child_min_height"))
				minh = strtol(attr->value, NULL, 0);
			else if (!strcmp(attr->name, "child_ipad_x"))
				ipx = strtol(attr->value, NULL, 0);
			else if (!strcmp(attr->name, "child_ipad_y"))
				ipy = strtol(attr->value, NULL, 0);
			break;
		case 'l':
			if (!strcmp(attr->name, "layout_style"))
				gtk_button_box_set_layout(GTK_BUTTON_BOX(bbox),
					glade_enum_from_string(
						GTK_TYPE_BUTTON_BOX_STYLE,
						attr->value));
			break;
		case 's':
			if (!strcmp(attr->name, "spacing"))
			       gtk_button_box_set_spacing(GTK_BUTTON_BOX(bbox),
						strtol(attr->value, NULL, 0));
			break;
		}
	}
	gtk_button_box_set_child_size(GTK_BUTTON_BOX(bbox), minw, minh);
	gtk_button_box_set_child_ipadding(GTK_BUTTON_BOX(bbox), ipx, ipy);
	return bbox;
}

static GtkWidget *
frame_new(GladeXML *xml, GladeWidgetInfo *info)
{
	GtkWidget *frame;
	GList *tmp;
	char *label = NULL;
	gdouble label_xalign = 0;
	GtkShadowType shadow_type = GTK_SHADOW_ETCHED_IN;

	for (tmp = info->attributes; tmp; tmp = tmp->next) {
		GladeAttribute *attr = tmp->data;

		switch (attr->name[0]) {
		case 'l':
			if (!strcmp(attr->name, "label"))
				label = attr->value;
			else if (!strcmp(attr->name, "label_xalign"))
				label_xalign = g_strtod(attr->value, NULL);
			break;
		case 's':
			if (!strcmp(attr->name, "shadow_type"))
				shadow_type = glade_enum_from_string(
					GTK_TYPE_SHADOW_TYPE, attr->value);
			break;
		}
	}
	if (label)
		frame = gtk_frame_new(_(label));
	else
		frame = gtk_frame_new(NULL);
	gtk_frame_set_label_align(GTK_FRAME(frame), label_xalign, 0.5);
	gtk_frame_set_shadow_type(GTK_FRAME(frame), shadow_type);

	return frame;
}

static GtkWidget *
aspectframe_new(GladeXML *xml, GladeWidgetInfo *info)
{
	GtkWidget *frame;
	GList *tmp;
	char *label = NULL;
	gdouble label_xalign = 0, xalign = 0, yalign = 0, ratio = 1;
	GtkShadowType shadow_type = GTK_SHADOW_ETCHED_IN;
	gboolean obey_child = FALSE;

	for (tmp = info->attributes; tmp; tmp = tmp->next) {
		GladeAttribute *attr = tmp->data;

		switch (attr->name[0]) {
		case 'l':
			if (!strcmp(attr->name, "label"))
				label = attr->value;
			else if (!strcmp(attr->name, "label_xalign"))
				label_xalign = g_strtod(attr->value, NULL);
			break;
		case 'o':
			if (!strcmp(attr->name, "obey_child"))
				obey_child = attr->value[0] == 'T';
			break;
		case 'r':
			if (!strcmp(attr->name, "ratio"))
				ratio = g_strtod(attr->value, NULL);
			break;
		case 's':
			if (!strcmp(attr->name, "shadow_type"))
				shadow_type = glade_enum_from_string(
					GTK_TYPE_SHADOW_TYPE, attr->value);
			break;
		case 'x':
			if (!strcmp(attr->name, "xalign"))
				xalign = g_strtod(attr->value, NULL);
			break;
		case 'y':
			if (!strcmp(attr->name, "yalign"))
				yalign = g_strtod(attr->value, NULL);
			break;
		}
	}
	if (label)
		frame = gtk_aspect_frame_new(_(label), xalign, yalign,
					     ratio, obey_child);
	else
		frame = gtk_aspect_frame_new(NULL, xalign, yalign,
					     ratio, obey_child);
	gtk_frame_set_label_align(GTK_FRAME(frame), label_xalign, 0.5);
	gtk_frame_set_shadow_type(GTK_FRAME(frame), shadow_type);
	return frame;
}

static GtkWidget *
hpaned_new(GladeXML *xml, GladeWidgetInfo *info)
{
	GtkWidget *paned = gtk_hpaned_new();
	GList *tmp;

	for (tmp = info->attributes; tmp; tmp = tmp->next) {
		GladeAttribute *attr = tmp->data;

		if (!strcmp(attr->name, "handle_size"))
			gtk_paned_set_handle_size(GTK_PANED(paned),
						  strtoul(attr->value,NULL,0));
		else if (!strcmp(attr->name, "gutter_size"))
			gtk_paned_set_gutter_size(GTK_PANED(paned),
						  strtoul(attr->value,NULL,0));
		else if (!strcmp(attr->name, "position"))
			gtk_paned_set_position(GTK_PANED(paned),
					       strtol(attr->value, NULL, 0));
	}
	return paned;
}

static GtkWidget *
vpaned_new(GladeXML *xml, GladeWidgetInfo *info)
{
	GtkWidget *paned = gtk_vpaned_new();
	GList *tmp;

	for (tmp = info->attributes; tmp; tmp = tmp->next) {
		GladeAttribute *attr = tmp->data;

		if (!strcmp(attr->name, "handle_size"))
			gtk_paned_set_handle_size(GTK_PANED(paned),
						  strtoul(attr->value,NULL,0));
		else if (!strcmp(attr->name, "gutter_size"))
			gtk_paned_set_gutter_size(GTK_PANED(paned),
						  strtoul(attr->value,NULL,0));
		else if (!strcmp(attr->name, "position"))
			gtk_paned_set_position(GTK_PANED(paned),
					       strtol(attr->value, NULL, 0));
	}
	return paned;
}

static GtkWidget *
handlebox_new(GladeXML *xml, GladeWidgetInfo *info)
{
	return gtk_handle_box_new();
}

static GtkWidget *
notebook_new(GladeXML *xml, GladeWidgetInfo *info)
{
	GtkWidget *notebook = gtk_notebook_new();
	GList *tmp;

	for (tmp = info->attributes; tmp; tmp = tmp->next) {
		GladeAttribute *attr = tmp->data;

		if (!strcmp(attr->name, "popup_enable")) {
			if (attr->value[0] == 'T')
				gtk_notebook_popup_enable(GTK_NOTEBOOK(notebook));
			else
				gtk_notebook_popup_disable(GTK_NOTEBOOK(notebook));
		} else if (!strcmp(attr->name, "scrollable"))
			gtk_notebook_set_scrollable(GTK_NOTEBOOK(notebook),
						    attr->value[0] == 'T');
		else if (!strcmp(attr->name, "show_border"))
			gtk_notebook_set_show_border(GTK_NOTEBOOK(notebook),
						     attr->value[0] == 'T');
		else if (!strcmp(attr->name, "show_tabs"))
			gtk_notebook_set_show_tabs(GTK_NOTEBOOK(notebook),
						   attr->value[0] == 'T');
		else if (!strcmp(attr->name, "tab_pos"))
			gtk_notebook_set_tab_pos(GTK_NOTEBOOK(notebook),
				glade_enum_from_string(GTK_TYPE_POSITION_TYPE,
						       attr->value));
	}
	return notebook;
}

static GtkWidget *
alignment_new (GladeXML *xml, GladeWidgetInfo *info)
{
	GList *tmp;
	gdouble xalign = 0.5, yalign = 0.5, xscale = 0, yscale = 0;
  
	for (tmp = info->attributes; tmp; tmp = tmp->next) {
		GladeAttribute *attr = tmp->data;

		if (!strcmp(attr->name, "xalign"))
			xalign = g_strtod(attr->value, NULL);
		else if (!strcmp(attr->name, "xscale"))
			xscale = g_strtod(attr->value, NULL);
		else if (!strcmp(attr->name, "yalign"))
			yalign = g_strtod(attr->value, NULL);
		else if (!strcmp(attr->name, "yscale"))
			yscale = g_strtod(attr->value, NULL);
	}
	return gtk_alignment_new(xalign, yalign, xscale, yscale);
}

static GtkWidget *
eventbox_new(GladeXML *xml, GladeWidgetInfo *info)
{
	return gtk_event_box_new();
}

static GtkWidget *
scrolledwindow_new (GladeXML *xml, GladeWidgetInfo *info)
{
	GtkWidget *win = gtk_scrolled_window_new(NULL, NULL);
	GList *tmp;
	GtkPolicyType hpol = GTK_POLICY_ALWAYS, vpol = GTK_POLICY_ALWAYS;

	for (tmp = info->attributes; tmp; tmp = tmp->next) {
		GladeAttribute *attr = tmp->data;

		if (!strcmp(attr->name, "hscrollbar_policy"))
			hpol = glade_enum_from_string(GTK_TYPE_POLICY_TYPE,
						      attr->value);
		else if (!strcmp(attr->name, "hupdate_policy"))
			gtk_range_set_update_policy(
			       GTK_RANGE(GTK_SCROLLED_WINDOW(win)->hscrollbar),
			       glade_enum_from_string(GTK_TYPE_UPDATE_TYPE,
						      attr->value));
		else if (!strcmp(attr->name, "shadow_type"))
			gtk_viewport_set_shadow_type(GTK_VIEWPORT(win),
				glade_enum_from_string(GTK_TYPE_SHADOW_TYPE,
						       attr->value));
		else if (!strcmp(attr->name, "vscrollbar_policy"))
			vpol = glade_enum_from_string(GTK_TYPE_POLICY_TYPE,
						      attr->value);
		else if (!strcmp(attr->name, "vupdate_policy"))
			gtk_range_set_update_policy(
			       GTK_RANGE(GTK_SCROLLED_WINDOW(win)->vscrollbar),
			       glade_enum_from_string(GTK_TYPE_UPDATE_TYPE,
						      attr->value));
	}
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(win), hpol, vpol);
	return win;
}

static GtkWidget *
viewport_new(GladeXML *xml, GladeWidgetInfo *info)
{
	GtkWidget *port = gtk_viewport_new(NULL, NULL);
	GList *tmp;

	for (tmp = info->attributes; tmp; tmp = tmp->next) {
		GladeAttribute *attr = tmp->data;

		if (!strcmp(attr->name, "shadow_type"))
			gtk_viewport_set_shadow_type(GTK_VIEWPORT(port),
				glade_enum_from_string(GTK_TYPE_SHADOW_TYPE,
						       attr->value));
	}
	return port;
}

static GtkWidget *
packer_new(GladeXML *xml, GladeWidgetInfo *info)
{
	GtkWidget *packer = gtk_packer_new();
	guint border = 0, pad_x = 0, pad_y = 0, ipad_x = 0, ipad_y = 0;
	GList *tmp;

	for (tmp = info->attributes; tmp; tmp = tmp->next) {
		GladeAttribute *attr = tmp->data;

		if (strncmp(attr->name, "default_", 8))
			continue;
		if (!strcmp(&attr->name[8], "border_width"))
			border = strtoul(attr->value, NULL, 0);
		else if (!strcmp(&attr->name[8], "pad_x"))
			pad_x = strtoul(attr->value, NULL, 0);
		else if (!strcmp(&attr->name[8], "pad_y"))
			pad_y = strtoul(attr->value, NULL, 0);
		else if (!strcmp(&attr->name[8], "ipad_x"))
			ipad_x = strtoul(attr->value, NULL, 0);
		else if (!strcmp(&attr->name[8], "ipad_y"))
			ipad_y = strtoul(attr->value, NULL, 0);
	}
	gtk_packer_set_default_border_width(GTK_PACKER(packer), border);
	gtk_packer_set_default_pad(GTK_PACKER(packer), pad_x, pad_y);
	gtk_packer_set_default_ipad(GTK_PACKER(packer), ipad_x, ipad_y);
	return packer;
}

static GtkWidget *
curve_new(GladeXML *xml, GladeWidgetInfo *info)
{
	GtkWidget *curve = gtk_curve_new();
	GList *tmp;
	gdouble minx=0, miny=0, maxx=1, maxy=1;

	for (tmp = info->attributes; tmp; tmp = tmp->next) {
		GladeAttribute *attr = tmp->data;

		if (!strcmp(attr->name, "curve_type"))
			gtk_curve_set_curve_type(GTK_CURVE(curve),
				glade_enum_from_string(GTK_TYPE_CURVE_TYPE,
						       attr->value));
		else if (!strcmp(attr->name, "min_x"))
			minx = g_strtod(attr->value, NULL);
		else if (!strcmp(attr->name, "min_y"))
			miny = g_strtod(attr->value, NULL);
		else if (!strcmp(attr->name, "max_x"))
			maxx = g_strtod(attr->value, NULL);
		else if (!strcmp(attr->name, "max_y"))
			maxy = g_strtod(attr->value, NULL);
	}
	gtk_curve_set_range(GTK_CURVE(curve), minx, maxx, miny, maxy);
	return curve;
}

static GtkWidget *
gammacurve_new(GladeXML *xml, GladeWidgetInfo *info)
{
	GtkWidget *gamma = gtk_gamma_curve_new();
	GtkWidget *curve = GTK_GAMMA_CURVE(gamma)->curve;
	GList *tmp;
	gdouble minx=0, miny=0, maxx=1, maxy=1;

	for (tmp = info->attributes; tmp; tmp = tmp->next) {
		GladeAttribute *attr = tmp->data;

		if (!strcmp(attr->name, "curve_type"))
			gtk_curve_set_curve_type(GTK_CURVE(curve),
				glade_enum_from_string(GTK_TYPE_CURVE_TYPE,
						       attr->value));
		else if (!strcmp(attr->name, "min_x"))
			minx = g_strtod(attr->value, NULL);
		else if (!strcmp(attr->name, "min_y"))
			miny = g_strtod(attr->value, NULL);
		else if (!strcmp(attr->name, "max_x"))
			maxx = g_strtod(attr->value, NULL);
		else if (!strcmp(attr->name, "max_y"))
			maxy = g_strtod(attr->value, NULL);
	}
	gtk_curve_set_range(GTK_CURVE(curve), minx, maxx, miny, maxy);
	return gamma;
}

static GtkWidget *
colorselection_new(GladeXML *xml, GladeWidgetInfo *info)
{
	GtkWidget *csel = gtk_color_selection_new();
	GList *tmp;

	for (tmp = info->attributes; tmp; tmp = tmp->next) {
		GladeAttribute *attr = tmp->data;

		if (!strcmp(attr->name, "policy"))
			gtk_color_selection_set_update_policy(
				GTK_COLOR_SELECTION(csel),
				glade_enum_from_string(GTK_TYPE_UPDATE_TYPE,
						       attr->value));
	}
	return csel;
}

static GtkWidget *
fontselection_new(GladeXML *xml, GladeWidgetInfo *info)
{
	return gtk_font_selection_new();
}

static GtkWidget *
preview_new(GladeXML *xml, GladeWidgetInfo *info)
{
	GtkWidget *preview;
	GList *tmp;
	GtkPreviewType type = GTK_PREVIEW_COLOR;
	gboolean expand = TRUE;

	for (tmp = info->attributes; tmp; tmp = tmp->next) {
		GladeAttribute *attr = tmp->data;

		if (!strcmp(attr->name, "expand"))
			expand = attr->value[0] == 'T';
		else if (!strcmp(attr->name, "type"))
			type = glade_enum_from_string(GTK_TYPE_PREVIEW_TYPE,
						      attr->value);
	}
	preview = gtk_preview_new(type);
	gtk_preview_set_expand(GTK_PREVIEW(preview), expand);
	return preview;
}

static GtkWidget *
calendar_new (GladeXML *xml, GladeWidgetInfo *info)
{
	GtkWidget *wid = gtk_calendar_new();
	GList *tmp;
	GtkCalendarDisplayOptions dopt = 0;

	for (tmp = info->attributes; tmp; tmp = tmp->next) {
		GladeAttribute *attr = tmp->data;

		if (!strcmp(attr->name, "show_heading")) {
			if (attr->value[0] == 'T')
				dopt |= GTK_CALENDAR_SHOW_HEADING;
		} else if (!strcmp(attr->name, "show_day_names")) {
			if (attr->value[0] == 'T')
				dopt |= GTK_CALENDAR_SHOW_DAY_NAMES;
		} else if (!strcmp(attr->name, "no_month_change")) {
			if (attr->value[0] == 'T')
				dopt |= GTK_CALENDAR_NO_MONTH_CHANGE;
		} else if (!strcmp(attr->name, "show_week_numbers")) {
			if (attr->value[0] == 'T')
				dopt |= GTK_CALENDAR_SHOW_WEEK_NUMBERS;
		} else if (!strcmp(attr->name, "week_start_monday")) {
			if (attr->value[0] == 'T')
				dopt |= GTK_CALENDAR_WEEK_START_MONDAY;
		}
	}
	gtk_calendar_display_options(GTK_CALENDAR(wid), dopt);
	return wid;
}

static GtkWidget *
window_new (GladeXML *xml, GladeWidgetInfo *info)
{
	GtkWidget *win;
	GList *tmp;
	GtkWindowType type = GTK_WINDOW_TOPLEVEL;
	char *title = NULL;

	for (tmp = info->attributes; tmp; tmp = tmp->next) {
		GladeAttribute *attr = tmp->data;

		switch (attr->name[0]) {
		case 't':
			if (!strcmp(attr->name, "title"))
				title = attr->value;
			else if (!strcmp(attr->name, "type"))
				type = glade_enum_from_string(
					GTK_TYPE_WINDOW_TYPE, attr->value);
			break;
		}
	}
	win = gtk_window_new(type);
	gtk_window_set_title(GTK_WINDOW(win), _(title));
	glade_xml_set_window_props(GTK_WINDOW(win), info);
	glade_xml_set_toplevel(xml, GTK_WINDOW(win));

	return win;
}

static GtkWidget *
dialog_new(GladeXML *xml, GladeWidgetInfo *info)
{
	GtkWidget *win = gtk_dialog_new();
	GList *tmp;

	for (tmp = info->attributes; tmp; tmp = tmp->next) {
		GladeAttribute *attr = tmp->data;

		switch (attr->name[0]) {
		case 't':
			if (!strcmp(attr->name, "title"))
				gtk_window_set_title(GTK_WINDOW(win),
						     _(attr->value));
			break;
		}
	}

	glade_xml_set_window_props(GTK_WINDOW(win), info);
	glade_xml_set_toplevel(xml, GTK_WINDOW(win));

	return win;
}

static GtkWidget *
fileselection_new (GladeXML *xml, GladeWidgetInfo *info)
{
	GtkWidget *win;
	GList *tmp;
	GtkWindowType type = GTK_WINDOW_TOPLEVEL;
	char *title = NULL;

	for (tmp = info->attributes; tmp; tmp = tmp->next) {
		GladeAttribute *attr = tmp->data;

		switch (attr->name[0]) {
		case 't':
			if (!strcmp(attr->name, "title"))
				title = attr->value;
			else if (!strcmp(attr->name, "type"))
				type = glade_enum_from_string(
					GTK_TYPE_WINDOW_TYPE, attr->value);
			break;
		}
	}
	win = gtk_file_selection_new(_(title));
	glade_xml_set_window_props(GTK_WINDOW(win), info);
	glade_xml_set_toplevel(xml, GTK_WINDOW(win));

	return win;
}

static GtkWidget *
colorselectiondialog_new (GladeXML *xml, GladeWidgetInfo *info)
{
	GtkWidget *win;
	GList *tmp;
	GtkWindowType type = GTK_WINDOW_TOPLEVEL;
	GtkUpdateType policy = GTK_UPDATE_CONTINUOUS;
	char *title = NULL;

	for (tmp = info->attributes; tmp; tmp = tmp->next) {
		GladeAttribute *attr = tmp->data;

		switch (attr->name[0]) {
		case 'p':
			if (!strcmp(attr->name, "policy"))
				policy = glade_enum_from_string
					(GTK_TYPE_UPDATE_TYPE, attr->value);
			break;
		case 't':
			if (!strcmp(attr->name, "title"))
				title = attr->value;
			else if (!strcmp(attr->name, "type"))
				type = glade_enum_from_string(
					GTK_TYPE_WINDOW_TYPE, attr->value);
			break;
		}
	}
	win = gtk_color_selection_dialog_new(_(title));
	gtk_color_selection_set_update_policy(GTK_COLOR_SELECTION(
			GTK_COLOR_SELECTION_DIALOG(win)->colorsel), policy);

	glade_xml_set_window_props(GTK_WINDOW(win), info);
	glade_xml_set_toplevel(xml, GTK_WINDOW(win));

	return win;
}

static GtkWidget *
fontselectiondialog_new (GladeXML *xml, GladeWidgetInfo *info)
{
	GtkWidget *win;
	GList *tmp;
	GtkWindowType type = GTK_WINDOW_TOPLEVEL;
	char *title = NULL;

	for (tmp = info->attributes; tmp; tmp = tmp->next) {
		GladeAttribute *attr = tmp->data;

		switch (attr->name[0]) {
		case 't':
			if (!strcmp(attr->name, "title"))
				title = attr->value;
			else if (!strcmp(attr->name, "type"))
				type = glade_enum_from_string(
					GTK_TYPE_WINDOW_TYPE, attr->value);
			break;
		}
	}
	win = gtk_font_selection_dialog_new(_(title));

	glade_xml_set_window_props(GTK_WINDOW(win), info);
	glade_xml_set_toplevel(xml, GTK_WINDOW(win));

	return win;
}

static GtkWidget *
inputdialog_new (GladeXML *xml, GladeWidgetInfo *info)
{
	GtkWidget *win = gtk_input_dialog_new();
	GList *tmp;

	for (tmp = info->attributes; tmp; tmp = tmp->next) {
		GladeAttribute *attr = tmp->data;

		if (!strcmp(attr->name, "title"))
			gtk_window_set_title(GTK_WINDOW(win), _(attr->value));
	}
	glade_xml_set_window_props(GTK_WINDOW(win), info);
	glade_xml_set_toplevel(xml, GTK_WINDOW(win));

	return win;
}

static GtkWidget *
custom_new (GladeXML *xml, GladeWidgetInfo *info)
{
	GtkWidget *wid = NULL;
	GList *tmp;
	gchar *func_name = NULL, *string1 = NULL, *string2 = NULL;
	gint int1 = 0, int2 = 0;

	for (tmp = info->attributes; tmp; tmp = tmp->next) {
		GladeAttribute *attr = tmp->data;

		if (!strcmp(attr->name, "creation_function"))
			func_name = attr->value;
		else if (!strcmp(attr->name, "string1"))
			string1 = attr->value;
		else if (!strcmp(attr->name, "string2"))
			string2 = attr->value;
		else if (!strcmp(attr->name, "int1"))
			int1 = strtol(attr->value, NULL, 0);
		else if (!strcmp(attr->name, "int2"))
			int2 = strtol(attr->value, NULL, 0);
	}
	wid = glade_create_custom(xml, func_name, info->name, string1,
				  string2, int1, int2);
	/* fallback to prevent segfault */
	if (wid == NULL)
		wid = gtk_label_new("[custom widget creation failed]");
	return wid;
}

static const GladeWidgetBuildData widget_data[] = {
	/* general widgets */
	{"GtkLabel",         label_new,         NULL},
	{"GtkAccelLabel",    accellabel_new,    NULL},
	{"GtkEntry",         entry_new,         NULL},
	{"GtkText",          text_new,          NULL},
	{"GtkButton",        button_new,        button_build_children},
	{"GtkToggleButton",  togglebutton_new,  button_build_children},
	{"GtkCheckButton",   checkbutton_new,   button_build_children},
	{"GtkRadioButton",   radiobutton_new,   button_build_children},
	{"GtkOptionMenu",    optionmenu_new,    NULL},
	{"GtkCombo",         combo_new,         combo_build_children},
	{"GtkList",          list_new,          NULL}, /* XXXX list appends ? */
	{"GtkCList",         clist_new,         clist_build_children},
	{"GtkCTree",         ctree_new,         clist_build_children},
	{"GtkTree",          tree_new,          NULL},
	{"GtkSpinButton",    spinbutton_new,    NULL},
	{"GtkHScale",        hscale_new,        NULL},
	{"GtkVScale",        vscale_new,        NULL},
	{"GtkHRuler",        hruler_new,        NULL},
	{"GtkVRuler",        vruler_new,        NULL},
	{"GtkHScrollbar",    hscrollbar_new,    NULL},
	{"GtkVScrollbar",    vscrollbar_new,    NULL},
	{"GtkStatusbar",     statusbar_new,     NULL},
	{"GtkToolbar",       toolbar_new,       toolbar_build_children},
	{"GtkProgressBar",   progressbar_new,   NULL},
	{"GtkArrow",         arrow_new,         NULL},
	/* {"GtkImage",         image_new,         NULL}, */
	{"GtkPixmap",        pixmap_new,        NULL},
	{"GtkDrawingArea",   drawingarea_new,   NULL},
	{"GtkHSeparator",    hseparator_new,    NULL},
	{"GtkVSeparator",    vseparator_new,    NULL},

	/* menu widgets */
	{"GtkMenuBar",       menubar_new,       menushell_build_children},
	{"GtkMenu",          menu_new,          menushell_build_children},
	{"GtkMenuItem",      menuitem_new,      menuitem_build_children},
	{"GtkCheckMenuItem", checkmenuitem_new, menuitem_build_children},
	{"GtkRadioMenuItem", radiomenuitem_new, menuitem_build_children},

	/* container widgets */
	{"GtkHBox",          hbox_new,          box_build_children},
	{"GtkVBox",          vbox_new,          box_build_children},
	{"GtkTable",         table_new,         table_build_children},
	{"GtkFixed",         fixed_new,         fixed_build_children},
	{"GtkLayout",        layout_new,        layout_build_children},
	{"GtkHButtonBox",    hbuttonbox_new,    glade_standard_build_children},
	{"GtkVButtonBox",    vbuttonbox_new,    glade_standard_build_children},
	{"GtkFrame",         frame_new,         glade_standard_build_children},
	{"GtkAspectFrame",   aspectframe_new,   glade_standard_build_children},
	{"GtkHPaned",        hpaned_new,        paned_build_children},
	{"GtkVPaned",        vpaned_new,        paned_build_children},
	{"GtkHandleBox",     handlebox_new,     glade_standard_build_children},
	{"GtkNotebook",      notebook_new,      notebook_build_children},
	{"GtkAlignment",     alignment_new,     glade_standard_build_children},
	{"GtkEventBox",      eventbox_new,      glade_standard_build_children},
	{"GtkScrolledWindow",scrolledwindow_new,glade_standard_build_children},
	{"GtkViewport",      viewport_new,      glade_standard_build_children},
	{"GtkPacker",        packer_new,        packer_build_children},

  /* other widgets */
	{"GtkCurve",         curve_new,         NULL},
	{"GtkGammaCurve",    gammacurve_new,    NULL},
	{"GtkColorSelection",colorselection_new,NULL},
	{"GtkFontSelection", fontselection_new, NULL},
	{"GtkPreview",       preview_new,       NULL},
	{"GtkCalendar",      calendar_new,      NULL},

  /* toplevel widgets */
	{"GtkWindow",        window_new,        glade_standard_build_children},
	{"GtkDialog",        dialog_new,        dialog_build_children},
	{"GtkFileSelection", fileselection_new, fileselection_build_children},
	{"GtkColorSelectionDialog", colorselectiondialog_new,
				    colorselectiondialog_build_children},
	{"GtkFontSelectionDialog", fontselectiondialog_new,
				   fontselectiondialog_build_children},
	{"GtkInputDialog",   inputdialog_new,   inputdialog_build_children},

  /* the custom widget */
	{"Custom",           custom_new,        NULL},
	{NULL, NULL, NULL}
};

void glade_init_gtk_widgets(void) {
	glade_register_widgets(widget_data);
}


