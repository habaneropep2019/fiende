
#ifndef __SUMMARY_H__
#define __SUMMARY_H__

#include <properties.h>
#include <graph.h>
#include <proc.h>

typedef struct	_GTopStatusBarData	GTopStatusBarData;
typedef struct	_GTopSummaryData	GTopSummaryData;

struct _GTopStatusBarData
{
	GtkWidget		*container;
	GtkWidget		*status_bar;
	GtkWidget		*status_cpu, *status_cpu_label;
	GtkWidget		*status_mem, *status_mem_label;
	GtkWidget		*status_uptime, *status_uptime_label;
	GtkWidget		*status_loadavg, *status_loadavg_label;
	guint			status_cpu_timeout;
	guint			status_mem_timeout;
	guint			status_uptime_timeout;
	guint			status_loadavg_timeout;
};

struct _GTopSummaryData
{
	GtkWidget		*summary, *box;
	GtkWidget		*summary_cpu, *summary_mem, *summary_swap;
	GtkWidget		*summary_load;
#ifdef HAVE_LIBGTOP_SMP
	GtkWidget		*summary_xcpu [4];
#endif
	ProcInfo		*summary_info;
	GtkWidget		*xpm_cpu, *xpm_mem, *xpm_swap, *xpm_load;
#ifdef HAVE_LIBGTOP_SMP
	GtkWidget		*xpm_xcpu [4];
#endif
};

GTopStatusBarData *gtop_statusbar_new (void);
void gtop_statusbar_update (GTopStatusBarData *);

GTopSummaryData *gtop_summary_new (void);
void gtop_summary_map (GTopSummaryData *);
void gtop_summary_unmap (GTopSummaryData *);
        
#endif
