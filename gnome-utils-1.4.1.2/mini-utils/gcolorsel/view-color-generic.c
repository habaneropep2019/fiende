#include <config.h>
#include <stdio.h>
#include <gnome.h>
#include <glade/glade.h>

#include "view-color-generic.h"
#include "utils.h"

enum {
  DATA_CHANGED,
  LAST_SIGNAL
};

static guint cg_signals [LAST_SIGNAL] = { 0 };

char *ColFormatStr[] = { N_("Decimal 8 bits"),
			 N_("Decimal 16 bits"),
			 N_("Hex 8 bits"),
			 N_("Hex 16 bits"),
			 N_("Float"),
			 NULL };
				 
static void view_color_generic_class_init      (ViewColorGenericClass *class);
static void view_color_generic_init            (ViewColorGeneric *vcl);

static GList *view_color_generic_get_selected    (ViewColorGeneric *vcg);
static void   view_color_generic_remove_selected (ViewColorGeneric *vcg);
static int    view_color_generic_get_insert_pos  (ViewColorGeneric *vcg);

static gpointer view_color_generic_get_control (ViewColorGeneric *vcg,
						GtkVBox *box,
						void (*changed_cb) (gpointer data), gpointer change_data);
static void     view_color_generic_apply       (ViewColorGeneric *vcg,
						gpointer data);
static void     view_color_generic_close       (ViewColorGeneric *vcg,
						gpointer data);
static void     view_color_generic_sync        (ViewColorGeneric *vcg,
						gpointer data);

static void     view_color_generic_save        (ViewColorGeneric *vcg);
static void     view_color_generic_load        (ViewColorGeneric *vcg);

static GList   *str_tab_2_str_list             (char *tab[]);

static GtkObjectClass *parent_class = NULL;

GtkType 
view_color_generic_get_type (void)
{
  static guint cg_type = 0;

  if (!cg_type) {
    GtkTypeInfo cg_info = {
      "ViewColorGeneric",
      sizeof (ViewColorGeneric),
      sizeof (ViewColorGenericClass),
      (GtkClassInitFunc) view_color_generic_class_init,
      (GtkObjectInitFunc) view_color_generic_init,
      NULL,
      NULL,
      (GtkClassInitFunc) NULL
    };

    cg_type = gtk_type_unique (gtk_object_get_type (), &cg_info);
  }

  return cg_type;
}

static void
view_color_generic_class_init (ViewColorGenericClass *class)
{
  GtkWidgetClass *widget_class;
  GtkObjectClass *object_class;

  object_class = GTK_OBJECT_CLASS (class);
  parent_class = gtk_type_class (GTK_TYPE_OBJECT);
  widget_class = (GtkWidgetClass *)class;

  cg_signals [DATA_CHANGED] = 
    gtk_signal_new ("data_changed",
		    GTK_RUN_LAST,
		    object_class->type,
		    GTK_SIGNAL_OFFSET (ViewColorGenericClass, data_changed),
		    gtk_marshal_NONE__POINTER,
		    GTK_TYPE_NONE, 1, GTK_TYPE_POINTER);

  gtk_object_class_add_signals (object_class, cg_signals, LAST_SIGNAL);

  class->remove_selected = view_color_generic_remove_selected;
  class->get_selected    = view_color_generic_get_selected;
  class->get_insert_pos  = view_color_generic_get_insert_pos;
  class->get_control     = view_color_generic_get_control;
  class->apply           = view_color_generic_apply;
  class->close           = view_color_generic_close;
  class->sync            = view_color_generic_sync;
  class->save            = view_color_generic_save;
  class->load            = view_color_generic_load;
}

static void
view_color_generic_init (ViewColorGeneric *vcg)
{
  vcg->key        = get_config_key ();
  vcg->format     = FORMAT_DEC_8;
}

void 
view_color_generic_data_changed (ViewColorGeneric *vcg, GList *changes)
{
  gtk_signal_emit (GTK_OBJECT (vcg), cg_signals[DATA_CHANGED], changes);
}


static GList *
view_color_generic_get_selected (ViewColorGeneric *vcg)
{
  return NULL;
}

static int    
view_color_generic_get_insert_pos (ViewColorGeneric *vcg)
{
  return -1;
}

static void 
view_color_generic_remove_selected (ViewColorGeneric *vcg)
{
  GList *list = VIEW_COLOR_GENERIC_GET_CLASS (vcg)->get_selected (vcg);
  GList *l;
  MDIColor *col;
  GList *freezed = NULL;
  
  if (!list) return;
  l = list;

  while (l) {
    col = mdi_color_generic_get_owner (l->data);

    mdi_color_generic_freeze (col->owner);
    mdi_color_generic_remove (col->owner, col);

    freezed = g_list_append (freezed, col->owner);

    l = g_list_next (l);
  }

  g_list_free (list); 
  
  l = freezed;
  while (l) {
    mdi_color_generic_thaw (MDI_COLOR_GENERIC (l->data));
    
    l = g_list_next (l);
  }

  g_list_free (freezed);
}

/************************* PROPERTIES ********************************/

typedef struct prop_t {
  GladeXML *gui;

  void (*changed_cb) (gpointer data);
  gpointer change_data;

  GtkWidget *combo_format;
  GtkWidget *check_show_control; 
} prop_t;

static GList *
str_tab_2_str_list (char *tab[])
{
  int i = 0;
  GList *list = NULL;

  while (tab[i]) 
    list = g_list_append (list, tab[i++]);
  
  return list;
}

static void
combo_changed (GtkWidget *widget, prop_t *prop)
{
  prop->changed_cb (prop->change_data);
}

static void
check_toggled_cb (GtkWidget *widget, prop_t *prop)
{
  prop->changed_cb (prop->change_data);
}

static gpointer
view_color_generic_get_control (ViewColorGeneric *vcg, GtkVBox *box,
				void (*changed_cb)(gpointer data), 
				gpointer change_data)
{
  GtkWidget *frame;
  GList *list;
  prop_t *prop = g_new0 (prop_t, 1);

  prop->changed_cb  = changed_cb;
  prop->change_data = change_data;

  prop->gui = glade_xml_new (GCOLORSEL_GLADEDIR "view-color-generic-properties.glade", "frame");
  g_return_val_if_fail (prop->gui != NULL, NULL);

  frame = glade_xml_get_widget (prop->gui, "frame");
  g_return_val_if_fail (frame != NULL, NULL);

  gtk_box_pack_start_defaults (GTK_BOX (box), frame);

  /* Format */

  prop->combo_format = glade_xml_get_widget (prop->gui, "combo-format");

  list = str_tab_2_str_list (ColFormatStr);
  gtk_combo_set_popdown_strings (GTK_COMBO (prop->combo_format), list);
  g_list_free (list);

  gtk_signal_connect (GTK_OBJECT (GTK_COMBO (prop->combo_format)->entry), "changed", GTK_SIGNAL_FUNC (combo_changed), prop);

  /* Show control */

  prop->check_show_control = glade_xml_get_widget (prop->gui, 
						   "check-show-control");
  gtk_signal_connect (GTK_OBJECT (prop->check_show_control), "toggled",
		      GTK_SIGNAL_FUNC (check_toggled_cb), prop);

  return prop;
}

static void     
view_color_generic_apply (ViewColorGeneric *vcg, gpointer data)
{
  prop_t *prop = data;
  int i = 0;
  GtkWidget *entry;

  /* Format */

  entry = GTK_COMBO (prop->combo_format)->entry;
  while (strcmp (ColFormatStr[i], gtk_entry_get_text (GTK_ENTRY (entry)))) i++;
  vcg->format = i;

  /* Show control */

  vcg->show_control = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (prop->check_show_control));
  if (vcg->show_control) {
    if (vcg->control) gtk_widget_show (GTK_WIDGET (vcg->control));
  } else 
    if (vcg->control) gtk_widget_hide (GTK_WIDGET (vcg->control));
}

static void 
view_color_generic_close (ViewColorGeneric *vcg, gpointer data)
{
  prop_t *prop = data;

  gtk_object_unref (GTK_OBJECT (prop->gui));
  g_free (prop);
}

static void 
view_color_generic_sync (ViewColorGeneric *vcg, gpointer data)
{
  prop_t *prop = data;
  GtkWidget *entry;

  /* Format */

  entry = GTK_COMBO (prop->combo_format)->entry;

  entry_set_text (GTK_ENTRY (entry), ColFormatStr[vcg->format], prop);

  /* Show control */

  gtk_signal_handler_block_by_data (GTK_OBJECT (prop->check_show_control), prop);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (prop->check_show_control),
				vcg->show_control);
  gtk_signal_handler_unblock_by_data (GTK_OBJECT (prop->check_show_control), prop);
}

/****************************** Config **********************************/

static void 
view_color_generic_save (ViewColorGeneric *vcg)
{
  gnome_config_set_int ("Format", vcg->format);
  gnome_config_set_bool ("ShowControl", vcg->show_control);
}

static void 
view_color_generic_load (ViewColorGeneric *vcg)
{
  vcg->format       = gnome_config_get_int ("Format");
  vcg->show_control = gnome_config_get_bool ("ShowControl");

  if (vcg->show_control) {
    if (vcg->control) gtk_widget_show (GTK_WIDGET (vcg->control));
  } else 
    if (vcg->control) gtk_widget_hide (GTK_WIDGET (vcg->control));
}
