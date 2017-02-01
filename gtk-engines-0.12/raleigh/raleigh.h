#include <gtk/gtk.h>

#define OPTION_INDICATOR_WIDTH 7
#define OPTION_INDICATOR_LEFT_SPACING 7
#define OPTION_INDICATOR_RIGHT_SPACING 5

typedef struct _RaleighEngineData RaleighEngineData;

struct _RaleighEngineData 
{
  GdkColor aa[5];
  GdkGC *aa_gc[5];
};

void raleigh_initialize_style (GtkStyleClass *klass,
			       GtkStyleClass *parent);

