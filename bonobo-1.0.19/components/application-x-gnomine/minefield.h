#ifndef __GTK_MINEFIELD_H__
#define __GTK_MINEFIELD_H__

#include <gdk/gdk.h>
#include <gtk/gtkwidget.h>
#include <glib.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#define GTK_MINEFIELD(obj) GTK_CHECK_CAST(obj, gtk_minefield_get_type(), GtkMineFieldView)
#define GTK_MINEFIELD_CLASS(klass) GTK_CHECK_CLASS_CAST(klass, gtk_minefield_get_type(), GtkMineFieldViewClass);
#define GTK_IS_MINEFIELD(obj) GTK_CHECK_TYPE(obj, gtk_minefield_get_type())

typedef struct _MineField                MineField;
typedef struct _GtkMineFieldView         GtkMineFieldView;
typedef struct _GtkMineFieldViewClass    GtkMineFieldViewClass;

struct _Mine {
	guint mined:1;
	guint shown:1;
	guint marked:1;
	guint down:1;
        guint neighbours;
};

typedef struct _Mine mine;

typedef struct _Sign sign;

struct _Sign {
	GdkPixmap *pixmap;
	GdkBitmap *mask;
	
        gint width;
        gint height;
};

struct _MineField {
/* List of minefield widgets */
	GList     *views;

        guint      xsize;
	guint      ysize;
        guint      mcount;
	mine      *mines;
	guint      flags;
	guint      shown;
        gint       lose;
        gint       win;
};
  
struct _GtkMineFieldView {
        GtkWidget  widget;
	MineField *data;

	gint       cdown;
        guint      cdownx;
        guint      cdowny;
	gint       bdown[3];
	gint       multi_mode;
	guint      minesize;
        GdkFont         *font;
	GdkColorContext *cc;
        sign             flag;
        sign             mine;
	struct {
		char text[2];
		gint dx, dy;
		GdkGC *gc;
	} numstr[9];
};

struct _GtkMineFieldViewClass
{
	GtkWidgetClass parent_class;
	void (*marks_changed) (GtkMineFieldView *mfield);
	void (*explode)       (GtkMineFieldView *mfield);
	void (*look)          (GtkMineFieldView *mfield);
	void (*unlook)        (GtkMineFieldView *mfield);
	void (*win)           (GtkMineFieldView *mfield);
};

MineField *minefield_new       (void);
void       minefield_destroy   (MineField *data);
void       minefield_set_size  (MineField *data,
				guint xsize, guint ysize);
void       minefield_set_mines (MineField *data, guint mcount);
void       minefield_restart   (MineField *data);
void       minefield_set_at    (MineField *data, int x, int y,
				char num, char state);
void       minefield_get_at    (MineField *data, int x, int y,
				char *num, char *state);

guint      gtk_minefield_get_type     (void);
GtkWidget *gtk_minefield_new_view     (MineField *data);
void       gtk_minefield_set_minesize (GtkMineFieldView *data,
				       guint minesize);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __GTK_MINEFIELD_H__ */
