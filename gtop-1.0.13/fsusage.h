
#ifndef __FSUSAGE_H__
#define __FSUSAGE_H__

#include <properties.h>

#include <gtop-graph.h>

typedef struct	_GTopFsUsageData	GTopFsUsageData;
typedef enum	_GTopFsUsageType	GTopFsUsageType;

enum _GTopFsUsageType {
	GTOP_FSUSAGE_TOTAL = 0,
	GTOP_FSUSAGE_USED,
	GTOP_FSUSAGE_FREE
};

struct _GTopFsUsageData
{
	GTopFsUsageType	ftype;
	GtkWidget	*sw, *graph;
	gchar		*graph_head;
	gchar		*graph_tail;
	gint64		value_total;
};

extern GnomePropertyDescriptor FsUsageProperty_Descriptor;

void fsusage_destroy (GtkWidget *);
void fsusage_new (GTopFsUsageData *, GtkWidget *, gint);
void fsusage_type_set (GTopFsUsageData *, gint);
gint fsusage_update (GTopFsUsageData *);

#endif
