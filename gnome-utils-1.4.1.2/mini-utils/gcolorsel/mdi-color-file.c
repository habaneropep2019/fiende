#include "mdi-color-file.h"
#include "mdi-color-generic.h"
#include "utils.h"

#include <gnome.h>
#include <glade/glade.h>

static void       mdi_color_file_class_init  (MDIColorFileClass *class);
static void       mdi_color_file_init        (MDIColorFile *mcf);

static gpointer   
mdi_color_generic_get_control (MDIColorGeneric *vcg, GtkVBox *box,
			       void (*changed_cb)(gpointer data), 
			       gpointer change_data);
static void mdi_color_generic_apply (MDIColorGeneric *mcg, gpointer data);
static void mdi_color_generic_close (MDIColorGeneric *mcg, gpointer data);
static void mdi_color_generic_sync  (MDIColorGeneric *mcg, gpointer data);

static void mdi_color_generic_load  (MDIColorGeneric *mcg);
static void mdi_color_generic_save  (MDIColorGeneric *mcg);

static MDIColorGenericClass *parent_class = NULL;

guint 
mdi_color_file_get_type()
{
  static guint mdi_gen_child_type = 0;
  
  if (!mdi_gen_child_type) {
    GtkTypeInfo mdi_gen_child_info = {
      "MDIColorFile",
      sizeof (MDIColorFile),
      sizeof (MDIColorFileClass),
      (GtkClassInitFunc) mdi_color_file_class_init,
      (GtkObjectInitFunc) mdi_color_file_init,
      (GtkArgSetFunc) NULL,
      (GtkArgGetFunc) NULL,
    };
    
    mdi_gen_child_type = gtk_type_unique (mdi_color_generic_get_type (),
					  &mdi_gen_child_info);
  }
  
  return mdi_gen_child_type;
}

static void 
mdi_color_file_class_init (MDIColorFileClass *class)
{
  GnomeMDIChildClass   *mdi_child_class;
  GtkObjectClass       *object_class;
  MDIColorGenericClass *mcg_class;
  
  object_class    = GTK_OBJECT_CLASS (class);
  mdi_child_class = GNOME_MDI_CHILD_CLASS (class);
  parent_class    = gtk_type_class(mdi_color_generic_get_type());
  mcg_class       = (MDIColorGenericClass *)class;

  mcg_class->get_control = mdi_color_generic_get_control;
  mcg_class->apply       = mdi_color_generic_apply;
  mcg_class->close       = mdi_color_generic_close;
  mcg_class->sync        = mdi_color_generic_sync;
  mcg_class->load        = mdi_color_generic_load;
  mcg_class->save        = mdi_color_generic_save;
}

static void
mdi_color_file_init (MDIColorFile *mcf)
{
  mcf->filename = NULL;
  mcf->create   = FALSE;
  mcf->header   = NULL;
  mcf->comments_begin = NULL;
  mcf->comments_end   = NULL;
 
  MDI_COLOR_GENERIC (mcf)->monitor_modified = TRUE;

  MDI_COLOR_GENERIC (mcf)->flags = CHANGE_APPEND | CHANGE_REMOVE | 
    CHANGE_NAME | CHANGE_POS | CHANGE_RGB | CHANGE_CLEAR;
}

MDIColorFile *
mdi_color_file_new (void)
{
  MDIColorFile *mcf; 

  mcf = gtk_type_new (mdi_color_file_get_type ()); 

  return mcf;
}

void
mdi_color_file_set_filename (MDIColorFile *mcf, const char *filename,
                             gboolean create)
{
  if (mcf->filename)
    g_free (mcf->filename);

  mcf->filename = g_strdup (filename);  
  mcf->create = create;
}

gboolean
mdi_color_file_load (MDIColorFile *mcf, GnomeMDI *mdi)
{
  FILE *fp;
  int r, g, b;
  char tmp[256], name[256];
  gboolean ok = TRUE;
  int nb = 0;
  long size, pos = 0, last_pos = 0;

  if (mcf->create) 
    fp = fopen (mcf->filename, "a+");
  else 
    fp = fopen (mcf->filename, "r");
    
  if (!fp) 
    return FALSE;

  fseek (fp, 0, SEEK_END);
  size = ftell (fp);
  fseek (fp, 0, SEEK_SET);

  if (mcf->header) {
    g_free (mcf->header);
    mcf->header = NULL;       
  }

  mdi_color_generic_freeze (MDI_COLOR_GENERIC (mcf));

  while (1) {
    fgets(tmp, 255, fp);    

    if (feof (fp)) break;

    if ((tmp[0] == '!') || (tmp[0] == '#')) continue;

    name[0] = 0;
    if (sscanf(tmp, "%d %d %d\t\t%255[^\n]\n", &r, &g, &b, name) < 3) {
      if (nb) {
	ok = FALSE;
	break;
      } else {
	if (mcf->header) {
	  ok = FALSE;
	  break;
	} else { /* For GIMP Palette ... */
	  {
	    int i = strlen (tmp);
	    while ((i) && (tmp[i - 1] == '\n')) {
	      tmp [i - 1] = 0; i--;
	    }
	  }

	  mcf->header = g_strdup (tmp);
	  continue;
	}
      }
    }

    pos = ftell (fp);

    if (pos > last_pos) {    
      if (mdi)
	progress_set (mdi, ((float)pos / (float)size));

      last_pos += size / 150;
    }

    mdi_color_generic_append_new (MDI_COLOR_GENERIC (mcf), 
				  r, g, b, name);
    nb++;
  }

  if (mdi)
    progress_set (mdi, 0);

  fclose (fp);

/*  if (!nb) ok = FALSE; */
  mdi_color_generic_thaw (MDI_COLOR_GENERIC (mcf));
  mdi_color_generic_set_modified (MDI_COLOR_GENERIC (mcf), FALSE);
  
  return ok;
}

gboolean
mdi_color_file_save (MDIColorFile *mcf)
{
  FILE *fp;
  GList *list;
  MDIColor *col;
  char *buf;
  gboolean ok = TRUE;

  fp = fopen (mcf->filename, "w");
  if (!fp) return FALSE;

  if (mcf->header) {
    if (fputs (mcf->header, fp) < 0) 
      ok = FALSE;
    else 
      if (mcf->header[strlen (mcf->header) - 1] != '\n')
	if (fputc ('\n', fp) < 0) 
	  ok = FALSE;    
  }

  if (ok) {
    list = MDI_COLOR_GENERIC (mcf)->col;
    while (list) {
      col = list->data;
      
      buf = g_strdup_printf ("%3d %3d %3d\t\t%s\n", 
			     col->r, col->g, col->b, col->name);
      if (fputs (buf, fp) < 0) {
	ok = FALSE;
	g_free (buf);
	break;
      }
      
      g_free (buf);
      
      list = g_list_next (list);
    }

    if (ok) 
      mdi_color_generic_set_modified (MDI_COLOR_GENERIC (mcf), FALSE);
  }

  fclose (fp);

  return ok;
}

/************************* PROPERTIES ************************************/

typedef struct prop_t {
  GladeXML *gui;

  gpointer parent_data;

  void (*changed_cb)(gpointer data);
  gpointer change_data;

  GtkWidget *entry_file;
  GtkWidget *combo_header;
  GtkWidget *text_comments_begin;
  GtkWidget *text_comments_end;
} prop_t;

static void
entry_changed (GtkWidget *widget, prop_t *prop)
{
  prop->changed_cb (prop->change_data);
}

static gpointer
mdi_color_generic_get_control (MDIColorGeneric *mcg, GtkVBox *box,
			       void (*changed_cb)(gpointer data), 
			       gpointer change_data)
{
  GtkWidget *frame;
  prop_t *prop = g_new0 (prop_t, 1);
  
  prop->parent_data = parent_class->get_control (mcg, box, 
						 changed_cb, change_data);

  prop->changed_cb   = changed_cb;
  prop->change_data = change_data;

  prop->gui = glade_xml_new (GCOLORSEL_GLADEDIR "mdi-color-file-properties.glade", "frame");
  if (!prop->gui) {
    printf ("Could not find mdi-color-file-properties.glade\n");
    return NULL;
  }

  frame = glade_xml_get_widget (prop->gui, "frame");
  if (!frame) {
    printf ("Corrupt file mdi-color-file-properties.glade");
    return NULL;
  }

  gtk_box_pack_start_defaults (GTK_BOX (box), frame);

  prop->entry_file = glade_xml_get_widget (prop->gui, "entry-file");

  prop->combo_header = glade_xml_get_widget (prop->gui, "combo-header");
  gtk_signal_connect (GTK_OBJECT (GTK_COMBO (prop->combo_header)->entry),
		      "changed",
		      GTK_SIGNAL_FUNC (entry_changed), prop);

  prop->text_comments_begin = 
    glade_xml_get_widget (prop->gui, "text-comments-begin");
  prop->text_comments_end = 
    glade_xml_get_widget (prop->gui, "text-comments-end");

  gtk_signal_connect (GTK_OBJECT (prop->text_comments_begin), "changed",
		      GTK_SIGNAL_FUNC (entry_changed), prop);
  gtk_signal_connect (GTK_OBJECT (prop->text_comments_end), "changed",
		      GTK_SIGNAL_FUNC (entry_changed), prop);
  
  return prop;
}

static void
mdi_color_generic_sync (MDIColorGeneric *mcg, gpointer data)
{
  prop_t *prop = data;
  MDIColorFile *mcf = MDI_COLOR_FILE (mcg);

  if (mcf->filename)
    gtk_entry_set_text (GTK_ENTRY (prop->entry_file), mcf->filename);
  else
    gtk_entry_set_text (GTK_ENTRY (prop->entry_file), "");
  
  entry_set_text (GTK_ENTRY (GTK_COMBO (prop->combo_header)->entry),
		  mcf->header, prop);

  parent_class->sync (mcg, prop->parent_data);
}

static void
mdi_color_generic_apply (MDIColorGeneric *mcg, gpointer data)
{
  prop_t *prop = data;  
  MDIColorFile *mcf = MDI_COLOR_FILE (mcg);
  char *text;

  text = gtk_entry_get_text (GTK_ENTRY (GTK_COMBO (prop->combo_header)->entry));
  text = g_strchomp (g_strdup (text));

  if (!text[0]) {
    g_free (text);
    text = NULL;
  }

  if (my_strcmp (text, mcf->header)) {
    if (mcf->header) 
      g_free (mcf->header);

    mcf->header = text;

    mdi_color_generic_set_modified (mcg, TRUE);
  }

  parent_class->apply (mcg, prop->parent_data);
}

static void
mdi_color_generic_close (MDIColorGeneric *mcg, gpointer data)
{
  prop_t *prop = data;

  parent_class->close (mcg, prop->parent_data);

  gtk_object_unref (GTK_OBJECT (prop->gui));
  g_free (prop);
}

/******************************** Config *********************************/

static void 
mdi_color_generic_save  (MDIColorGeneric *mcg)
{
  gnome_config_set_string ("FileName", MDI_COLOR_FILE (mcg)->filename);
  gnome_config_set_bool ("Create", MDI_COLOR_FILE (mcg)->create);

  parent_class->save (mcg);
}

static void 
mdi_color_generic_load  (MDIColorGeneric *mcg)
{
  char *str;

  str = gnome_config_get_string ("FileName");
  if (! strcmp (str, "")) {
    g_free (str);
    str = NULL;
  }
  
  MDI_COLOR_FILE (mcg)->create = gnome_config_get_bool ("Create");
    
  MDI_COLOR_FILE (mcg)->filename = str;

  parent_class->load (mcg);
}

