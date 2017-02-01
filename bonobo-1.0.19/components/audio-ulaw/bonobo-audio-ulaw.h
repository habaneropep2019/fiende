#include <gnome.h>

#include <bonobo.h>

#ifndef __BONOBO_AUDIO_ULAW_H__
#define __BONOBO_AUDIO_ULAW_H__

/*
 * Component data
 */
typedef struct {
  BonoboControl *bonobo_object;

  gchar *sound;
  guint sound_len;
  
  gint update_callback_id;  

  GtkWidget *canvas;
  GtkWidget *vbox;
  GtkWidget *hscroll;
  GnomeCanvasItem *canvas_item;
} bonobo_object_data_t;

#endif /* __BONOBO_AUDIO_ULAW_H__ */
