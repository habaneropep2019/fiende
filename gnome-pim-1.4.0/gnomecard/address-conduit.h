#ifndef __ADDRESS_CONDUIT_H__
#define __ADDRESS_CONDUIT_H__

#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <pi-appinfo.h>
#include <pi-address.h>
#include <glib.h>
#include <gnome.h>
#include <errno.h>

#include <gpilotd/gnome-pilot-conduit.h>
#include <gpilotd/gnome-pilot-client.h>
#include <gpilotd/gnome-pilot-conduit-standard-abs.h>

#include "card.h"


#define OBJ_DATA_CONDUIT "conduit_data"
#define OBJ_DATA_CONFIG  "conduit_config"
#define CONFIG_PREFIX    "/gnome-pilot.d/address-conduit/Pilot_%u/"


typedef struct _GCardLocalRecord GCardLocalRecord;

struct _GCardLocalRecord {
	LocalRecord      local;
	gboolean         ignore;
	GCardLocalRecord *next;
	Card             *gcard;
	struct Address   *addr;
	gint             category;
};

typedef struct _ConduitData ConduitData;
struct _ConduitData {
	struct AddressAppInfo ai;
	GList *records;
	GnomePilotDBInfo *dbi;
};

#define GET_CONDUIT_CFG(s) ((ConduitCfg*)gtk_object_get_data(GTK_OBJECT(s),OBJ_DATA_CONFIG))
#define GET_CONDUIT_DATA(s) ((ConduitData*)gtk_object_get_data(GTK_OBJECT(s),OBJ_DATA_CONDUIT))

typedef struct IterateData {
	int flag;
	int archived;
	GCardLocalRecord *prev;
	GCardLocalRecord *first;
} IterateData;

typedef struct LoadInfo {
	gint id;
	gint secret;
	time_t mtime;
} LoadInfo;


typedef struct _ConduitCfg ConduitCfg;
struct _ConduitCfg 
{
	GnomePilotConduitSyncType  sync_type;   /* only used by capplet */
	guint32  pilotId;
	gchar   *filename;
	gboolean open_secret;
};


#endif
