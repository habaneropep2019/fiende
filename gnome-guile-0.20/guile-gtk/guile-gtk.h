/*	Copyright (C) 1997, 1998, 1999 Marius Vollmer
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef GUILE_GTK_H
#define GUILE_GTK_H

#include <libguile.h>
#ifndef GTK_2_0
#include <gtk/gtk.h>
#else
#include <gtk-2.0/gtk/gtk.h>
#endif

typedef struct _sgtk_type_info {
  char *name;
  GtkType type;
  SCM (*conversion) (SCM);
} sgtk_type_info;

typedef struct _sgtk_enum_literal {
  SCM symbol;
  char *name;
  int value;
} sgtk_enum_literal;

typedef struct _sgtk_enum_info {
  sgtk_type_info header;
  int n_literals;
  sgtk_enum_literal *literals;
} sgtk_enum_info;

/* This is like an _sgtk_enum_literal, but the values are strings.
   This is used in Gnome.  */
typedef struct _sgtk_senum_literal {
  char *name;
  char *value;
} sgtk_senum_literal;

typedef struct _sgtk_senum_info {
  sgtk_type_info header;
  int n_literals;
  sgtk_senum_literal *literals;
} sgtk_senum_info;

typedef struct _sgtk_boxed_info {
  sgtk_type_info header;
  void *(*copy) (void *);
  void (*destroy) (void *);
  size_t size;
} sgtk_boxed_info;

typedef struct _sgtk_object_info {
  sgtk_type_info header;
  GtkType (*init_func) ();

  struct _sgtk_object_info *parent;
  guint n_args;
  GtkArg *args;
  guint *args_flags;
  char **args_short_names;
} sgtk_object_info;

void sgtk_register_type_infos (sgtk_type_info **infos);
sgtk_type_info *sgtk_get_type_info (guint type_seqno);

SCM sgtk_wrap_gtkobj (GtkObject *obj);
int sgtk_is_a_gtkobj (guint type, SCM obj);
GtkObject *sgtk_get_gtkobj (SCM obj);

void sgtk_enum_flags_init (sgtk_enum_info*);
int sgtk_enum_flags_bin_search (SCM key, sgtk_enum_info *info, int *rval);

int sgtk_valid_enum (SCM obj, sgtk_enum_info*);
SCM sgtk_enum2scm (gint val, sgtk_enum_info*);
gint sgtk_scm2enum (SCM obj, sgtk_enum_info*, int pos, char *sname);

int sgtk_valid_flags (SCM obj, sgtk_enum_info*);
SCM sgtk_flags2scm (gint val, sgtk_enum_info*);
gint sgtk_scm2flags (SCM obj, sgtk_enum_info*, int pos, char *sname);

int sgtk_valid_senum (SCM obj, sgtk_senum_info*);
SCM sgtk_senum2scm (char *val, sgtk_senum_info*);
char *sgtk_scm2senum (SCM obj, sgtk_senum_info*);

SCM sgtk_boxed2scm (gpointer ptr, sgtk_boxed_info*, int copyp);
void *sgtk_scm2boxed (SCM obj);
int sgtk_valid_boxed (SCM obj, sgtk_boxed_info*);

int sgtk_valid_float (SCM obj);
gfloat sgtk_scm2float (SCM obj);
SCM sgtk_float2scm (gfloat f);

int sgtk_valid_double (SCM obj);
double sgtk_scm2double (SCM obj);
SCM sgtk_double2scm (double f);

int sgtk_valid_point (SCM obj);
GdkPoint sgtk_scm2point (SCM obj);
SCM sgtk_point2scm (GdkPoint p);

int sgtk_valid_rect (SCM obj);
GdkRectangle sgtk_scm2rect (SCM obj);
SCM sgtk_rect2scm (GdkRectangle r);

int sgtk_port2fileno (SCM port);
SCM sgtk_fileno2port (int fd);

GdkAtom sgtk_scm2atom (SCM symbol);
SCM sgtk_atom2scm (GdkAtom atom);

int sgtk_valid_type (SCM obj);
GtkType sgtk_scm2type (SCM obj);
SCM sgtk_type2scm (GtkType t);

int sgtk_valid_composite (SCM obj, int (*predicate)(SCM));
int sgtk_valid_complen (SCM obj, int (*predicate)(SCM), int len);
SCM sgtk_composite_inconversion (SCM obj, SCM (*conversion)(SCM));
SCM sgtk_composite_outconversion (SCM obj, SCM (*conversion)(SCM));

SCM sgtk_slist2scm (GSList *list, SCM (*toscm)(void*));
GSList *sgtk_scm2slist (SCM obj, void (*fromscm)(SCM, void*));
void sgtk_slist_finish (GSList *list, SCM obj, SCM (*toscm)(void*));

SCM sgtk_list2scm (GList *list, SCM (*toscm)(void*));
GList *sgtk_scm2list (SCM obj, void (*fromscm)(SCM, void*));
void sgtk_list_finish (GList *list, SCM obj, SCM (*toscm)(void*));

typedef struct {
  int count;
  void *vec;
} sgtk_cvec;

sgtk_cvec sgtk_scm2cvec (SCM obj, void (*fromscm)(SCM, void*), size_t sz);
void sgtk_cvec_finish (sgtk_cvec *, SCM obj, SCM (*toscm)(void*), size_t sz);

typedef struct sgtk_protshell sgtk_protshell;

sgtk_protshell *sgtk_protect (SCM protector, SCM obj);
void sgtk_unprotect (sgtk_protshell *);

void sgtk_callback_marshal (GtkObject *,
			    gpointer data,
			    guint n_args,
			    GtkArg *args);
void sgtk_callback_destroy (gpointer data);
SCM sgtk_callback_trampoline (SCM new_trampoline);

int sgtk_valid_arg (GtkArg *, SCM val);
SCM sgtk_arg2scm (GtkArg *a, int free_mem);
void sgtk_scm2arg (GtkArg *a, SCM obj, SCM protector);
void sgtk_scm2ret (GtkArg *a, SCM obj);

sgtk_object_info *sgtk_find_object_info_from_type (GtkType type);
sgtk_object_info *sgtk_find_object_info (char *name);
GtkArg *sgtk_build_args (sgtk_object_info *info, int *n_argsp,
			 SCM scm_args, SCM protector, char *subr);

SCM sgtk_color_conversion (SCM color);
SCM sgtk_font_conversion (SCM color);
SCM sgtk_string_conversion (SCM string);

void sgtk_set_standalone (int flag);
int sgtk_is_standalone ();
SCM sgtk_standalone_p ();

void sgtk_register_glue (char *name, void (*func)(void));
#define SGTK_REGISTER_GLUE(func) sgtk_register_glue (#func, func)

void sgtk_init ();
void sgtk_init_with_args (int *argcp, char ***argvp);

void sgtk_shell (int argc, char **argv);

/* Additional useful Gdk routines. */

/* The following two do their magic with type conversions that are
   automatically generated by build-guile-gtk. */

GdkColor *gdk_color_intern (GdkColor *);
GdkFont *gdk_font_intern (GdkFont *);

GdkEventType gdk_event_type (GdkEvent *event);
GdkWindow *gdk_event_window (GdkEvent *event);
gboolean gdk_event_send_event (GdkEvent *event);
GdkRectangle gdk_event_area (GdkEvent *event);
GdkVisibilityState gdk_event_visibility_state (GdkEvent *event);
guint32 gdk_event_time (GdkEvent *event);
gdouble gdk_event_x (GdkEvent *event);
gdouble gdk_event_y (GdkEvent *event);
gdouble gdk_event_pressure (GdkEvent *event);
gdouble gdk_event_xtilt (GdkEvent *event);
gdouble gdk_event_ytilt (GdkEvent *event);
gint gdk_event_button (GdkEvent *event);
guint gdk_event_state (GdkEvent *event);
gboolean gdk_event_is_hint (GdkEvent *event);
GdkInputSource gdk_event_source (GdkEvent *event);
guint32 gdk_event_deviceid (GdkEvent *event);
gdouble gdk_event_x_root (GdkEvent *event);
gdouble gdk_event_y_root (GdkEvent *event);
guint gdk_event_keyval (GdkEvent *event);
gchar *gdk_event_string (GdkEvent *event);
GdkWindow *gdk_event_subwindow (GdkEvent *event);
GdkNotifyType gdk_event_notify_detail (GdkEvent *event);
gboolean gdk_event_in (GdkEvent *event);
gint16 gdk_event_configure_x (GdkEvent *event);
gint16 gdk_event_configure_y (GdkEvent *event);
gint16 gdk_event_configure_width (GdkEvent *event);
gint16 gdk_event_configure_height (GdkEvent *event);

guint32 gdk_get_leader_window_id ();

void gdk_draw_text_scm (GdkDrawable  *drawable,
			GdkFont      *font,
			GdkGC	       *gc,
			gint		x,
			gint		y,
			const gchar  *text);

GdkGC *gtk_style_fg_gc (GtkStyle *style, GtkStateType state);
GdkGC *gtk_style_bg_gc (GtkStyle *style, GtkStateType state);

/* Gtk stuff that wouldn't be here in an ideal world. */

#if GTK_MAJOR_VERSION == 1 && GTK_MINOR_VERSION == 0
typedef int GtkWidgetFlags;
#endif

gchar *gtk_label_get_interp (GtkLabel *label);
void gtk_menu_popup_interp (GtkMenu *menu,
			    GtkWidget *parent_menu_shell,
			    GtkWidget *parent_menu_item,
			    gint button,
			    guint32 activate_time);

GtkWidget*
gtk_radio_menu_item_new_with_label_from_widget (GtkRadioMenuItem *group,
						gchar            *label);
GtkWidget* gtk_radio_menu_item_new_from_widget (GtkRadioMenuItem *group);
GtkWidget* gtk_pixmap_new_interp (char *file, GtkWidget *intended_parent);

GdkColor *gdk_color_parse_interp (char *spec);
GdkColor *gtk_style_get_white_interp (GtkStyle *style);

GdkColormap *gtk_widget_peek_colormap (void);

void gtk_list_append_item (GtkList *list, GtkListItem *item);

gboolean gtk_type_get_info (GtkType type, GtkTypeInfo *info);
GtkType gtk_class_new (GtkType parent_type, gchar *name);
guint
gtk_signal_new_generic (const gchar     *name,
			GtkSignalRunType signal_flags,
			GtkType          type,
			GtkType          return_type,
			guint            nparams,
			GtkType         *params);
void sgtk_signal_emit (GtkObject *obj, char *name, SCM scm_args);
void gtk_signal_set_class_function_full (GtkType            type,
					 const gchar       *signal,
					 GtkSignalFunc      func,
					 GtkCallbackMarshal marshal,
					 gpointer           data,
					 GtkDestroyNotify   destroy_func);

void gtk_color_selection_set_color_interp (GtkColorSelection *sel, GdkColor *color);
GdkColor *gtk_color_selection_get_color_interp (GtkColorSelection *sel);

int gtk_widget_allocation_width_scm (GtkWidget *widget);
int gtk_widget_allocation_height_scm (GtkWidget *widget);
int gtk_widget_allocation_x_scm (GtkWidget *widget);
int gtk_widget_allocation_y_scm (GtkWidget *widget);

void* g_timer_fake_copy (void *);

void *gtk_no_copy (void *);
void gtk_no_free (void *);

GdkGC *gtk_style_fg_gc (GtkStyle *style, GtkStateType state);
GdkGC *gtk_style_bg_gc (GtkStyle *style, GtkStateType state);
GdkGC *gtk_style_light_gc (GtkStyle *style, GtkStateType state);
GdkGC *gtk_style_dark_gc (GtkStyle *style, GtkStateType state);
GdkGC *gtk_style_mid_gc (GtkStyle *style, GtkStateType state);
GdkGC *gtk_style_text_gc (GtkStyle *style, GtkStateType state);
GdkGC *gtk_style_base_gc (GtkStyle *style, GtkStateType state);

#endif /* !GUILE_GTK_H */
