
#ifndef __PROCVIEW_H__
#define __PROCVIEW_H__

#include <summary.h>

typedef struct	_GTopProcViewData	GTopProcViewData;
typedef enum	_GTopProcViewType	GTopProcViewType;
typedef enum	_GTopProcViewFlags	GTopProcViewFlags;

typedef struct	_GTopProcViewDetails	GTopProcViewDetails;
typedef struct	_GTopProcMapRow		GTopProcMapRow;

typedef struct	_ProcProcData		ProcProcData;
typedef struct	_GTopProcField		GTopProcField;

/* There is some reason why this is starting with 4, this is the number
 * of entries in the view_type_menu in gtop-procview.c including the
 * separator - the view_changed_cb () in mdi.c uses something like
 * gtk_menu_shell_activate_item (g_list_nth (ftype)).
 */

enum _GTopProcViewType {
	GTOP_PROCVIEW_ALL = 0,
	GTOP_PROCVIEW_USER,
};

enum _GTopProcViewFlags {
	GTOP_PROCVIEW_TTY = 1,
	GTOP_PROCVIEW_IDLE,
	GTOP_PROCVIEW_SYSTEM
};

struct _GTopProcMapRow {
	unsigned long		VMstart;
	unsigned long		VMend;
	char			flags[5];
	unsigned long		VMoffset;
	short			dev_major;
	short			dev_minor;
	unsigned long		inode;
	gchar			*filename;
};

struct _GTopProcViewDetails
{
	gint			pid;
	GtkWidget		*dwin;
	gint			x, y, w, h;
	GtkWidget		*icl_sw, *mcl_sw;
	GtkWidget		*nb, *icl, *gswin, *mcl;
	GTopProcMapRow		**rows;
	Graph			*gg;
};

struct _GTopProcViewData
{
	GTopProcViewType	ftype;
	GTopProcViewDetails	details;
	GTopSummaryData		*summary_data;
	gulong			proc_selection_flags;
	GtkWidget		*renice_dialog, *renice_msg;
	GtkAdjustment		*renice_adj;
	GtkWidget		*bin, *vbox, *sw, *clist, *clist_menu;
	gint			select_pid;
	unsigned		*proc_tab;
	int			prev_count;
	GList			*field_list, *geometry_list;
	gint			field_list_length;
	GtkWidget		*sort_asc, *sort_dsc;
	int			sort_field, sort_order;
	gint			cmd_field_index;
	GTopProcField		*p_fields;
	ProcProcData		**proc_data;
};

void procview_destroy (GtkWidget *);
void procview_new (GTopProcViewData *, GtkWidget *, gint);
void procview_type_set (GTopProcViewData *, gint);
void procview_update (GTopProcViewData *);
void procview_map (GTopProcViewData *);
void procview_unmap (GTopProcViewData *);

typedef enum _p_fmt p_fmt;

enum _p_fmt {
	PROCVIEW_USER,
	PROCVIEW_CMD,
	PROCVIEW_STATE,
	PROCVIEW_TTYC,
	PROCVIEW_ENVIRON,
	PROCVIEW_CMDLINE,
	PROCVIEW_UID,
	PROCVIEW_PID,
	PROCVIEW_PPID,
	PROCVIEW_PGRP,
	PROCVIEW_SESSION,
	PROCVIEW_TTY,
	PROCVIEW_TPGID,
	PROCVIEW_PRIORITY,
	PROCVIEW_NICE,
	PROCVIEW_SIGNAL,
	PROCVIEW_BLOCKED,
	PROCVIEW_SIGIGNORE,
	PROCVIEW_SIGCATCH,
	PROCVIEW_STARTTIME,
	PROCVIEW_UTIME,
	PROCVIEW_STIME,
	PROCVIEW_CUTIME,
	PROCVIEW_CSTIME,
	PROCVIEW_SIZE,
	PROCVIEW_RESIDENT,
	PROCVIEW_SHARE,
	PROCVIEW_TRS,
	PROCVIEW_LRS,
	PROCVIEW_DRS,
	PROCVIEW_DT,
	PROCVIEW_PCPU,
	PROCVIEW_PMEM,
	PROCVIEW_TIME,
	PROCVIEW_VSIZE,
	PROCVIEW_RSS,
	PROCVIEW_RSS_RLIM,
	PROCVIEW_TIMEOUT,
	PROCVIEW_IT_REALVALUE,
	PROCVIEW_FLAGS,
	PROCVIEW_MIN_FLT,
	PROCVIEW_MAJ_FLT,
	PROCVIEW_CMIN_FLT,
	PROCVIEW_CMAJ_FLT,
	PROCVIEW_START_CODE,
	PROCVIEW_END_CODE,
	PROCVIEW_START_STACK,
	PROCVIEW_KSTK_ESP,
	PROCVIEW_KSTK_EIP,
	PROCVIEW_WCHAN,
};

struct save_hist {
    int ticks;
    int pid;
    int pcpu;
    int utime;
    int stime;
};

struct _ProcProcData {
	GTopProcViewData *d;
	gtop_proc_t _p, *p;

	gint pcpu;
	gint pmem;
};

struct _GTopProcField {
	gchar *label;
	gchar *long_info;
	p_fmt fmt;
	int (*compare) (const ProcProcData **, const ProcProcData **);
	gint order;
	gint flag;
	GtkJustification justification;
	GtkWidget *hb;
};

extern GnomePropertyDescriptor ProcViewProperty_Descriptor;
extern GnomePropertyDescriptor ProcFieldsProperty_Descriptor;
extern GnomePropertyDescriptor SummaryProperty_Descriptor;
extern GnomePropertyDescriptor SummaryColorsProperty_Descriptor;

extern GTopProcField gtop_proc_fields [];

extern gint cfg_has_swap;

void *addProcessesView ();
gchar *sprint_fmt (ProcProcData *d, p_fmt fmt);
        
#endif
