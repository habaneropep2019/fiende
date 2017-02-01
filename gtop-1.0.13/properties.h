#ifndef __PROPERTIES_H__
#define __PROPERTIES_H__

#include <global.h>

BEGIN_GNOME_DECLS

typedef struct	_GTopProperties			GTopProperties;
typedef struct	_RadioButtonCbData		RadioButtonCbData;

typedef struct	_GTopGlobalProperties		GTopGlobalProperties;
typedef struct	_GTopGraphProperties		GTopGraphProperties;
typedef struct	_GTopMemUsageProperties		GTopMemUsageProperties;
typedef struct	_GTopFsUsageProperties		GTopFsUsageProperties;
typedef struct	_GTopProcViewProperties	       	GTopProcViewProperties;
typedef struct	_GTopProcFieldsProperties	GTopProcFieldsProperties;
typedef struct	_GTopSummaryProperties		GTopSummaryProperties;
typedef struct	_GTopSummaryColorsProperties	GTopSummaryColorsProperties;

typedef enum	_GTopProcSelect			GTopProcSelect;
typedef enum	_GTopPropFsMode			GTopPropFsMode;
typedef enum	_GTopSummaryMode		GTopSummaryMode;
typedef enum	_GTopUpdateTimes		GTopUpdateTimes;
typedef enum	_GTopDetailsFlags		GTopDetailsFlags;

#define GTOP_PROCFIELD_COUNT	15

#define UPDATE_FIELDS		11
#define MEMUSAGE_FIELDS		5
#define GRAPH_DEFAULT_COLORS	4

enum _GTopPropFsMode {
	GTOP_FSMODE_SUBLOCKS = 0,
	GTOP_FSMODE_BLOCKS,
	GTOP_FSMODE_INODES
};

enum _GTopProcSelect {
	GTOP_PROC_SELECT_ALL = 0,
	GTOP_PROC_SELECT_USER,
	GTOP_PROC_SELECT_TTY
};

enum _GTopSummaryMode {
	GTOP_SUMMARY_SHOW_TEXT = 0,
	GTOP_SUMMARY_TEXT_STATUSBAR,
	GTOP_SUMMARY_TEXT_HOSTNAME,
	GTOP_SUMMARY_TEXT_USE_FQDN,
	GTOP_SUMMARY_TEXT_CPU,
	GTOP_SUMMARY_TEXT_MEMORY,
	GTOP_SUMMARY_TEXT_SWAP,
	GTOP_SUMMARY_TEXT_UPTIME,
	GTOP_SUMMARY_TEXT_LOADAVG,
	GTOP_SUMMARY_SHOW_GRAPH,
	GTOP_SUMMARY_GRAPH_CPU,
	GTOP_SUMMARY_GRAPH_XCPU,
	GTOP_SUMMARY_GRAPH_MEM,
	GTOP_SUMMARY_GRAPH_SWAP,
	GTOP_SUMMARY_GRAPH_LOAD,
};

enum _GTopUpdateTimes {
	GTOP_UPDATE_CPU = 0,
	GTOP_UPDATE_MEM,
	GTOP_UPDATE_PROCVIEW,
	GTOP_UPDATE_MEMUSAGE,
	GTOP_UPDATE_FSUSAGE,
	GTOP_UPDATE_LOAD,
	GTOP_UPDATE_STATUS_CPU,
	GTOP_UPDATE_STATUS_MEMORY,
	GTOP_UPDATE_STATUS_UPTIME,
	GTOP_UPDATE_STATUS_LOADAVG,
	GTOP_UPDATE_DETAILS
};

enum _GTopDetailsFlags {
	GTOP_DETAILS_AUTO_UPDATE = 1,
	GTOP_DETAILS_REMEMBER_POSITION,
	GTOP_DETAILS_CUMULATIVE_TIMINGS,
	GTOP_DETAILS_FULL_PATHNAMES
};

struct _RadioButtonCbData
{
	GnomePropertyObject *object;
	GtkWidget *button;
	gint index;
};

struct _GTopGlobalProperties
{
	gint save_session;
	GnomeMDIMode mdi_mode;
	glong update_times [UPDATE_FIELDS];

	gint show_menubar;
	gint show_toolbar;
};

struct _GTopMemUsageProperties
{
	GTopProcSelect proc_select;
	glong thresholds [MEMUSAGE_FIELDS];
};

struct _GTopFsUsageProperties
{
	GTopPropFsMode fsmode;
	gint selected_fs;
	glong selected_fs_mask;
};

struct _GTopProcViewProperties
{
	GdkFont *font;
	gchar *font_name;
	glong details_flags;
};

struct _GTopSummaryProperties
{
	GdkFont *statusbar_font;
	gchar *statusbar_font_name;
	glong summary_mode;
	glong summary_supported;
	gfloat maximum_loadavg;
};

struct _GTopProcFieldsProperties
{
	glong field_mask;
	gint field_width [GTOP_PROCFIELD_COUNT];
};

struct _GTopSummaryColorsProperties
{
	GdkColor cpu [4];
	GdkColor mem [4];
	GdkColor swap [2];
	GdkColor load [2];
};

struct _GTopGraphProperties
{
	GdkFont *font;
	gchar *font_name;
	GdkColor colors [GRAPH_DEFAULT_COLORS+2];
	guint default_width;
	guint default_height;
	guint horizontal_border;
	guint vertical_border;
	guint graph_width;
	guint extra_height;
	guint line_width;
	guint pad_width;
};

struct _GTopProperties {
	GTopGlobalProperties global;
	GTopGraphProperties graph;
	GTopMemUsageProperties memusage;
	GTopFsUsageProperties fsusage;
	GTopProcViewProperties procview;
	GTopProcFieldsProperties procfields;
	GTopSummaryProperties summary;
	GTopSummaryColorsProperties summary_colors;
};

extern GList *gtop_property_object_list;

/* extern GTopProperties gtop_temp_properties; */
extern GTopProperties gtop_properties;

void gtop_properties_apply (void);
void gtop_properties_close (void);
void gtop_properties_changed (void);
void gtop_show_properties (void);
void gtop_init_properties (void);

END_GNOME_DECLS

#endif
