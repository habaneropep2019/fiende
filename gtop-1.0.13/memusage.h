
#ifndef __MEMUSAGE_H__
#define __MEMUSAGE_H__

#include <properties.h>

#include <gtop-graph.h>

typedef struct	_GTopMemUsageData	GTopMemUsageData;

typedef enum	_GTopMemUsageType	GTopMemUsageType;

enum _GTopMemUsageType {
	GTOP_MEMUSAGE_RESIDENT = 0,
	GTOP_MEMUSAGE_SHARED,
	GTOP_MEMUSAGE_SIZE,
	GTOP_MEMUSAGE_VIRTUAL,
	GTOP_MEMUSAGE_SWAP
};

struct _GTopMemUsageData
{
	GTopMemUsageType	ftype;
	GtkWidget		*sw, *graph;
	gchar			*graph_head;
	gchar			*graph_tail;
	gint64			value_total;
};

extern GnomePropertyDescriptor MemUsageProperty_Descriptor;

void memusage_destroy (GtkWidget *);
void memusage_new (GTopMemUsageData *, GtkWidget *, gint);
void memusage_type_set (GTopMemUsageData *, gint);
gint memusage_update (GTopMemUsageData *);

#endif
