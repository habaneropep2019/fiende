/*
 * bonobo-ui-util.h: Bonobo UI utility functions
 *
 * Author:
 *	Michael Meeks (michael@helixcode.com)
 *
 * Copyright 2000 Helix Code, Inc.
 */
#ifndef _BONOBO_UI_XML_UTIL_H_
#define _BONOBO_UI_XML_UTIL_H_

#include <gtk/gtkwidget.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <bonobo/bonobo-ui-component.h>

char      *bonobo_ui_util_pixbuf_to_xml        (GdkPixbuf    *pixbuf);

GdkPixbuf *bonobo_ui_util_xml_to_pixbuf  (const char *xml);

GdkPixbuf *bonobo_ui_util_xml_get_icon_pixbuf         (BonoboUINode *node, gboolean prepend_menu);
GtkWidget *bonobo_ui_util_xml_get_icon_pixmap_widget  (BonoboUINode *node, gboolean prepend_menu);

void  bonobo_ui_util_xml_set_pixbuf     (BonoboUINode  *node,
					 GdkPixbuf     *pixbuf);
void  bonobo_ui_util_xml_set_pix_xpm    (BonoboUINode  *node,
					 const char   **xpm);
void  bonobo_ui_util_xml_set_pix_stock  (BonoboUINode  *node,
					 const char    *name);
void  bonobo_ui_util_xml_set_pix_fname  (BonoboUINode  *node,
					 const char    *name);

void       bonobo_ui_util_build_help_menu   (BonoboUIComponent *listener,
					     const char        *app_prefix,
					     const char        *app_name,
					     BonoboUINode      *parent);

BonoboUINode   *bonobo_ui_util_build_accel  (guint              accelerator_key,
					     GdkModifierType    accelerator_mods,
					     const char        *verb);

BonoboUINode   *bonobo_ui_util_new_menu     (gboolean           submenu,
					     const char        *name,
					     const char        *label,
					     const char        *tip,
					     const char        *verb);

BonoboUINode   *bonobo_ui_util_new_placeholder   (const char        *name,
					     gboolean           top,
					     gboolean           bottom);

void       bonobo_ui_util_set_radiogroup    (BonoboUINode           *node,
					     const char        *group_name);

void       bonobo_ui_util_set_toggle        (BonoboUINode           *node,
					     const char        *id,
					     const char        *init_state);

BonoboUINode   *bonobo_ui_util_new_std_toolbar   (const char        *name,
					     const char        *label,
					     const char        *tip,
					     const char        *verb);
					     
BonoboUINode   *bonobo_ui_util_new_toggle_toolbar(const char        *name,
					     const char        *label,
					     const char        *tip,
					     const char        *id);

char      *bonobo_ui_util_get_ui_fname      (const char        *component_prefix,
					     const char        *file_name);

void       bonobo_ui_util_translate_ui      (BonoboUINode      *node);

void       bonobo_ui_util_fixup_help        (BonoboUIComponent *component,
					     BonoboUINode      *node,
					     const char        *app_prefix,
					     const char        *app_name);

void       bonobo_ui_util_fixup_icons       (BonoboUINode      *node);


/*
 * Does all the translation & other grunt.
 */
BonoboUINode   *bonobo_ui_util_new_ui       (BonoboUIComponent *component,
					     const char        *fname,
					     const char        *app_prefix,
					     const char        *app_name);

void            bonobo_ui_util_set_ui       (BonoboUIComponent *component,
					     const char        *app_prefix,
					     const char        *file_name,
					     const char        *app_name);

void            bonobo_ui_util_set_pixbuf   (BonoboUIComponent *component,
					     const char        *path,
					     GdkPixbuf         *pixbuf);

gchar          *bonobo_ui_util_accel_name   (guint              accelerator_key,
					     GdkModifierType    accelerator_mods);

void            bonobo_ui_util_accel_parse  (char              *name,
					     guint             *accelerator_key,
					     GdkModifierType   *accelerator_mods);

char           *bonobo_ui_util_decode_str   (const char *str, gboolean *err);

char           *bonobo_ui_util_encode_str   (const char *str);

#endif /* _BONOBO_UI_XML_UTIL_H_ */
