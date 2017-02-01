#include <global.h>
#include <gtop-page.h>

#include <gtop-fsusage.h>
#include <gtop-memusage.h>
#include <gtop-procview.h>
#include <gtop-procbar.h>

#include <netdb.h>

#include <session.h>

#include <summary.h>

#include "cpu.xpm"
#include "mem.xpm"
#include "swap.xpm"
#include "load.xpm"

#ifdef HAVE_LIBGTOP_SMP
#include "xcpu0.xpm"
#include "xcpu1.xpm"
#include "xcpu2.xpm"
#include "xcpu3.xpm"

static gint ncpu = -1;
#endif

#include <glibtop.h>
#include <glibtop/union.h>

static const gchar *cpu_texts [4] = {
	N_("User"),  N_("Nice"),   N_("System"),  N_("Idle")
};

static const gchar *mem_texts [4] =  {
	N_("Other"), N_("Shared"), N_("Buffers"), N_("Free")
};

static const gchar *swap_texts [2] = {
	N_("Used"), N_("Free")
};

static const gchar *load_texts [2] = {
	N_("Used"), N_("Free")
};

static const GdkColor cpu_defaults [4] = {
	{0, 0xffff, 0xffff, 0x4fff},
	{0, 0xdfff, 0xdfff, 0xdfff},
	{0, 0xafff, 0xafff, 0xafff},
	{0, 0x0000, 0x0000, 0x0000}
};

static const GdkColor mem_defaults [4] = {
	{0, 0xbfff, 0xbfff, 0x4fff},
	{0, 0xefff, 0xefff, 0x4fff},
	{0, 0xafff, 0xafff, 0xafff},
	{0, 0x0000, 0x8fff, 0x0000}
};

static const GdkColor swap_defaults [2] = {
	{0, 0xcfff, 0x5fff, 0x5fff},
	{0, 0x0000, 0x8fff, 0x0000}
};

static const GdkColor load_defaults [2] = {
	{0, 0xcfff, 0x5fff, 0x5fff},
	{0, 0x0000, 0x8fff, 0x0000}
};

#define SUMMARY_MODE_BIT(bit)	\
(gtop_properties.summary.summary_mode & (1 << GTOP_SUMMARY_ ## bit))
#define SUMMARY_SUPPORTED_BIT(bit)	\
(gtop_properties.summary.summary_supported & (1 << GTOP_SUMMARY_ ## bit))
#define SUMMARY_GRAPH_BIT(bit)	\
(SUMMARY_MODE_BIT (SHOW_GRAPH) && SUMMARY_MODE_BIT (bit) && \
 SUMMARY_SUPPORTED_BIT (bit))
#define SUMMARY_TEXT_BIT(bit)	\
(SUMMARY_MODE_BIT (SHOW_TEXT) && SUMMARY_MODE_BIT (bit) && \
 SUMMARY_SUPPORTED_BIT (bit))
#define PIX(x) widget = gnome_pixmap_new_from_xpm_d (x ## _xpm); \
                        gtk_widget_ref (widget);
               
static GtkAdjustment *max_loadavg_adjustment;

#define STATUSBAR_DEFAULT_FONT "-*-*-medium-r-*-*-12-*-*-*-*-*-*-*"

static void update_font (GtkWidget *widget);

static GtkWidget *summary_properties_init (GnomePropertyObject *object);
static void summary_properties_apply (GnomePropertyObject *object);
static void summary_properties_update (GnomePropertyObject *object);
static void summary_properties_load (GnomePropertyObject *object);
static void summary_properties_save (GnomePropertyObject *object);
static void summary_properties_changed (GnomePropertyObject *object);

static GtkWidget *summary_colors_properties_init (GnomePropertyObject *object);
static void summary_colors_properties_update (GnomePropertyObject *object);
static void summary_colors_properties_load (GnomePropertyObject *object);
static void summary_colors_properties_save (GnomePropertyObject *object);
static void summary_colors_properties_changed (GnomePropertyObject *object);

GnomePropertyDescriptor SummaryColorsProperty_Descriptor = {
	sizeof (GTopSummaryColorsProperties),
	N_("Summary Colors"),
	summary_colors_properties_init,
	NULL,
	summary_colors_properties_update,
	summary_colors_properties_load,
	summary_colors_properties_save,
	NULL, NULL, NULL,
	summary_colors_properties_changed,
	NULL
};

GnomePropertyDescriptor SummaryProperty_Descriptor = {
	sizeof (GTopSummaryProperties),
	N_("Summary"),
	summary_properties_init,
	summary_properties_apply,
	summary_properties_update,
	summary_properties_load,
	summary_properties_save,
	NULL, NULL, NULL,
	summary_properties_changed,
	NULL
};

static GList *statusbar_object_list = NULL;
static GList *summary_object_list = NULL;

static void
summary_check_libgtop_support ()
{
	/* This things are always supported. */
	glong summary_supported = (1 << GTOP_SUMMARY_TEXT_STATUSBAR) |
		(1 << GTOP_SUMMARY_TEXT_HOSTNAME) |
		(1 << GTOP_SUMMARY_TEXT_USE_FQDN);

	/* Check whether LibGTop has all the required features. */
	if (!gtop_libgtop_is_summary_supported (GTOP_SUMMARY_TEXT_CPU))
		summary_supported |= (1 << GTOP_SUMMARY_TEXT_CPU);

	if (!gtop_libgtop_is_summary_supported (GTOP_SUMMARY_TEXT_MEMORY))
		summary_supported |= (1 << GTOP_SUMMARY_TEXT_MEMORY);

	if (!gtop_libgtop_is_summary_supported (GTOP_SUMMARY_TEXT_MEMORY))
		summary_supported |= (1 << GTOP_SUMMARY_TEXT_MEMORY);

	if (!gtop_libgtop_is_summary_supported (GTOP_SUMMARY_TEXT_SWAP))
		summary_supported |= (1 << GTOP_SUMMARY_TEXT_SWAP);

	if (!gtop_libgtop_is_summary_supported (GTOP_SUMMARY_TEXT_UPTIME))
		summary_supported |= (1 << GTOP_SUMMARY_TEXT_UPTIME);

	if (!gtop_libgtop_is_summary_supported (GTOP_SUMMARY_TEXT_LOADAVG))
		summary_supported |= (1 << GTOP_SUMMARY_TEXT_LOADAVG);

	if (!gtop_libgtop_is_summary_supported (GTOP_SUMMARY_GRAPH_CPU))
		summary_supported |= (1 << GTOP_SUMMARY_GRAPH_CPU);

	if (!gtop_libgtop_is_summary_supported (GTOP_SUMMARY_GRAPH_XCPU))
		summary_supported |= (1 << GTOP_SUMMARY_GRAPH_XCPU);

	if (!gtop_libgtop_is_summary_supported (GTOP_SUMMARY_GRAPH_MEM))
		summary_supported |= (1 << GTOP_SUMMARY_GRAPH_MEM);

	if (!gtop_libgtop_is_summary_supported (GTOP_SUMMARY_GRAPH_SWAP))
		summary_supported |= (1 << GTOP_SUMMARY_GRAPH_SWAP);

	if (!gtop_libgtop_is_summary_supported (GTOP_SUMMARY_GRAPH_LOAD))
		summary_supported |= (1 << GTOP_SUMMARY_GRAPH_LOAD);

	/* Graphical summary is supported if at least one field in it
	 * is supported. */
	if (summary_supported &
	    ((1 << GTOP_SUMMARY_GRAPH_CPU) | (1 << GTOP_SUMMARY_GRAPH_XCPU) |
	     (1 << GTOP_SUMMARY_GRAPH_MEM) | (1 << GTOP_SUMMARY_GRAPH_SWAP) |
	     (1 << GTOP_SUMMARY_GRAPH_LOAD)))
		summary_supported |= (1 << GTOP_SUMMARY_SHOW_GRAPH);

	gtop_properties.summary.summary_supported = summary_supported;
}

static gint
summary_cpu (GTopSummaryData *d)
{
	gint i;

	if (!d->summary_cpu || !gtop_is_running)
		return TRUE;

	proc_read_cpu (d->summary_info);
	gtop_proc_bar_set_values
		(GTOP_PROC_BAR (d->summary_cpu), d->summary_info->cpu);

#ifdef HAVE_LIBGTOP_SMP
	for (i = 0; i < ncpu; i++)
		gtop_proc_bar_set_values
			(GTOP_PROC_BAR (d->summary_xcpu [i]),
			 d->summary_info->xcpu [i]);
#endif
		
	return TRUE;
}

static gint
summary_mem (GTopSummaryData *d)
{
	if (!d->summary_mem || !gtop_is_running)
		return TRUE;

	proc_read_mem (d->summary_info);
	gtop_proc_bar_set_values
		(GTOP_PROC_BAR (d->summary_mem), d->summary_info->mem);

	return TRUE;
}

static gint
summary_swap (GTopSummaryData *d)
{
	if (!d->summary_swap || !gtop_is_running)
		return TRUE;

	proc_read_mem (d->summary_info);
	gtop_proc_bar_set_values
		(GTOP_PROC_BAR (d->summary_swap), d->summary_info->swap);

	return TRUE;
}

static gint
summary_load (GTopSummaryData *d)
{
	if (!d->summary_load || !gtop_is_running)
		return TRUE;

	proc_read_load (d->summary_info);
	gtop_proc_bar_set_values
		(GTOP_PROC_BAR (d->summary_load), d->summary_info->load);

	return TRUE;
}

static GtkWidget *
summary_create (GTopSummaryData *d)
{
	GtkWidget *hbox = NULL;
	guint i;

	proc_read_mem (d->summary_info);

	d->summary_cpu = gtop_proc_bar_new
		(d->xpm_cpu, PROC_CPU_SIZE-1,
		 gtop_properties.summary_colors.cpu, summary_cpu);

	d->summary_mem = gtop_proc_bar_new
		(d->xpm_mem, PROC_MEM_SIZE-2,
		 gtop_properties.summary_colors.mem, summary_mem);

	d->summary_swap = gtop_proc_bar_new
		(d->xpm_swap, PROC_SWAP_SIZE-1,
		 gtop_properties.summary_colors.swap, summary_swap);

	d->summary_load = gtop_proc_bar_new
		(d->xpm_load, PROC_LOAD_SIZE-1,
		 gtop_properties.summary_colors.load, summary_load);

#ifdef HAVE_LIBGTOP_SMP
	for (i = 0; i < ncpu; i++)
		d->summary_xcpu [i] = gtop_proc_bar_new
			(d->xpm_xcpu [i], PROC_CPU_SIZE-1,
			 gtop_properties.summary_colors.cpu, NULL);
#endif

	hbox = gtk_hbox_new (TRUE, GNOME_PAD_SMALL);

	gtk_box_pack_start_defaults (GTK_BOX (hbox), d->summary_cpu);

#ifdef HAVE_LIBGTOP_SMP
	for (i = 0; i < ncpu; i++)
		gtk_box_pack_start_defaults
			(GTK_BOX (hbox), d->summary_xcpu [i]);
#endif

	gtk_box_pack_start_defaults (GTK_BOX (hbox), d->summary_mem);
	gtk_box_pack_start_defaults (GTK_BOX (hbox), d->summary_swap);
	gtk_box_pack_start_defaults (GTK_BOX (hbox), d->summary_load);
	
	return hbox;
}

static void
summary_update (GTopSummaryData *d)
{
	gint config_has_swap;
	gint i;

	if (SUMMARY_MODE_BIT (SHOW_GRAPH) &&
	    SUMMARY_SUPPORTED_BIT (SHOW_GRAPH))
		gtk_widget_show (d->box);
	else
		gtk_widget_hide (d->box);

    if (SUMMARY_GRAPH_BIT (GRAPH_CPU))
		gtk_widget_show (d->summary_cpu);
	else
		gtk_widget_hide (d->summary_cpu);

#ifdef HAVE_LIBGTOP_SMP
    if (SUMMARY_GRAPH_BIT (GRAPH_XCPU))
		for (i = 0; i < ncpu; i++)
			gtk_widget_show (d->summary_xcpu [i]);
	else
		for (i = 0; i < ncpu; i++)
			gtk_widget_hide (d->summary_xcpu [i]);
#endif

    if (SUMMARY_GRAPH_BIT (GRAPH_MEM))
		gtk_widget_show (d->summary_mem);
	else
		gtk_widget_hide (d->summary_mem);

	config_has_swap = (d->summary_info->swap [PROC_SWAP_TOTAL]) &&
	SUMMARY_GRAPH_BIT (GRAPH_SWAP);

	if (config_has_swap)
		gtk_widget_show (d->summary_swap);
	else
		gtk_widget_hide (d->summary_swap);

    if (SUMMARY_GRAPH_BIT (GRAPH_LOAD))
		gtk_widget_show (d->summary_load);
	else
		gtk_widget_hide (d->summary_load);


    if (SUMMARY_GRAPH_BIT (GRAPH_CPU))
		gtop_proc_bar_start
			(GTOP_PROC_BAR (d->summary_cpu),
			 gtop_properties.global.update_times [GTOP_UPDATE_CPU],
			 d);
	else
		gtop_proc_bar_stop (GTOP_PROC_BAR (d->summary_cpu));

#ifdef HAVE_LIBGTOP_SMP
    if (SUMMARY_GRAPH_BIT (GRAPH_XCPU)) {
		if (d->summary_xcpu [0])
			gtop_proc_bar_start
				(GTOP_PROC_BAR (d->summary_xcpu [0]),
				 gtop_properties.global.update_times
				 [GTOP_UPDATE_CPU], d);
	} else {
		if (d->summary_xcpu [0])
			gtop_proc_bar_stop
				(GTOP_PROC_BAR (d->summary_xcpu [0]));
	}
#endif

    if (SUMMARY_GRAPH_BIT (GRAPH_MEM))
		gtop_proc_bar_start
			(GTOP_PROC_BAR (d->summary_mem),
			 gtop_properties.global.update_times [GTOP_UPDATE_MEM],
			 d);
	else
		gtop_proc_bar_stop (GTOP_PROC_BAR (d->summary_mem));

	config_has_swap = (d->summary_info->swap [PROC_SWAP_TOTAL]) &&
	SUMMARY_GRAPH_BIT (GRAPH_SWAP);

	if (config_has_swap)
		gtop_proc_bar_start
			(GTOP_PROC_BAR (d->summary_swap),
			 gtop_properties.global.update_times [GTOP_UPDATE_MEM],
			 d);
	else
		gtop_proc_bar_stop (GTOP_PROC_BAR (d->summary_swap));

    if (SUMMARY_GRAPH_BIT (GRAPH_LOAD))
		gtop_proc_bar_start
			(GTOP_PROC_BAR (d->summary_load),
			 gtop_properties.global.update_times [GTOP_UPDATE_LOAD],
			 d);
	else
		gtop_proc_bar_stop (GTOP_PROC_BAR (d->summary_load));
}

void
gtop_summary_map (GTopSummaryData *d)
{
    if (SUMMARY_GRAPH_BIT (GRAPH_CPU))
		gtop_proc_bar_start
			(GTOP_PROC_BAR (d->summary_cpu),
			 gtop_properties.global.update_times [GTOP_UPDATE_CPU],
			 d);

#ifdef LIBGTOP_HAVE_SMP
    if (SUMMARY_GRAPH_BIT (GRAPH_XCPU)) {
		if (d->summary_xcpu [0])
			gtop_proc_bar_start
				(GTOP_PROC_BAR (d->summary_xcpu [0]),
				 gtop_properties.global.update_times
				 [GTOP_UPDATE_CPU], d);
	}
#endif

    if (SUMMARY_GRAPH_BIT (GRAPH_MEM))
		gtop_proc_bar_start
			(GTOP_PROC_BAR (d->summary_mem),
			 gtop_properties.global.update_times [GTOP_UPDATE_MEM],
			 d);

    if (SUMMARY_GRAPH_BIT (GRAPH_SWAP))
		gtop_proc_bar_start
			(GTOP_PROC_BAR (d->summary_swap),
			 gtop_properties.global.update_times [GTOP_UPDATE_MEM],
			 d);

    if (SUMMARY_GRAPH_BIT (GRAPH_LOAD))
		gtop_proc_bar_start
			(GTOP_PROC_BAR (d->summary_load),
			 gtop_properties.global.update_times [GTOP_UPDATE_LOAD],
			 d);
}

void
gtop_summary_unmap (GTopSummaryData *d)
{
	gtop_proc_bar_stop (GTOP_PROC_BAR (d->summary_cpu));

#ifdef LIBGTOP_HAVE_SMP
	if (d->summary_xcpu [0])
		gtop_proc_bar_stop (GTOP_PROC_BAR (d->summary_xcpu [0]));
#endif

	gtop_proc_bar_stop (GTOP_PROC_BAR (d->summary_mem));

	gtop_proc_bar_stop (GTOP_PROC_BAR (d->summary_swap));

	gtop_proc_bar_stop (GTOP_PROC_BAR (d->summary_load));
}

static void
summary_destroy_cb (GtkWidget *summary, GTopSummaryData *d)
{
	gtop_summary_unmap (d);

	summary_object_list = g_list_remove (summary_object_list, d);
}

GTopSummaryData *
gtop_summary_new (void)
{
	GTopSummaryData *d = g_new0 (GTopSummaryData, 1);
	GtkWidget *widget;

	d->xpm_cpu = PIX (cpu);
	d->xpm_mem = PIX (mem);
	d->xpm_swap = PIX (swap);
	d->xpm_load = PIX (load);

#ifdef HAVE_LIBGTOP_SMP
	if (ncpu == -1) {
		glibtop_init ();
		ncpu = glibtop_global_server->ncpu;
	}

	d->xpm_xcpu [0] = PIX (xcpu0);
	d->xpm_xcpu [1] = PIX (xcpu1);
	d->xpm_xcpu [2] = PIX (xcpu2);
	d->xpm_xcpu [3] = PIX (xcpu3);
#endif

	d->summary_info = g_new0 (ProcInfo, 1);

	d->summary = summary_create (d);

	if (d->summary) {
		d->box = gtk_handle_box_new ();

		gtk_container_add (GTK_CONTAINER (d->box), d->summary);
		gtk_widget_show (d->box);
	}

	summary_update (d);

	if (d->summary)
		gtk_widget_show (d->summary);

	summary_object_list = g_list_append (summary_object_list, d);

	gtk_signal_connect (GTK_OBJECT (d->summary), "destroy",
			    summary_destroy_cb, d);

	return d;
}

/* =======================================================================
 * Textual summary.
 * =======================================================================
 */

static gint
statusbar_uptime_cb (gpointer label)
{
	gchar buffer [BUFSIZ], buffer2 [BUFSIZ];
	struct tm *tm_time;
	time_t timeval;

	glibtop_uptime uptime;

	glibtop_get_uptime (&uptime);

	time (&timeval);
	tm_time = localtime (&timeval);

	if (uptime.uptime >= 86400.0) {
		guint days = (long) uptime.uptime / 86400;

		sprintf (buffer2, "%d days", days);
	} else {
		guint hours = (long) uptime.uptime / 3600;
		guint secs = (long) uptime.uptime % 3600;

		sprintf (buffer2, "%2d:%02d", hours, secs / 60);
	}

	sprintf (buffer, "%2d:%02d%s, up %s",
		 tm_time->tm_hour % 12, tm_time->tm_min,
		 tm_time->tm_hour >= 12 ? "pm" : "am", buffer2);

	gtk_label_set_text (GTK_LABEL (label), buffer);
	return TRUE;
}

static gint
statusbar_loadavg_cb (gpointer label)
{
	gchar buffer [BUFSIZ];

	glibtop_loadavg loadavg;

	glibtop_get_loadavg (&loadavg);

	sprintf (buffer, "loadavg: %4.2f, %4.2f, %4.2f",
		 loadavg.loadavg [0], loadavg.loadavg [1],
		 loadavg.loadavg [2]);

	gtk_label_set_text (GTK_LABEL (label), buffer);
	return TRUE;
}

static gint
statusbar_cpu_cb (gpointer label)
{
	gchar buffer [BUFSIZ];
	double total, user = 0.0, sys = 0.0;

	glibtop_cpu cpu;
	static glibtop_cpu cpu_last;
	static guint init = 0;

	glibtop_get_cpu (&cpu);

	if (!init) {
		init = TRUE;
		memcpy (&cpu_last, &cpu, sizeof (glibtop_cpu));
	}

	total = cpu.total - cpu_last.total;

	if (total >= 0.01) {
		/* avoid divison by zero. */
		user = (double) (cpu.user - cpu_last.user) / total * 100.0;
		sys  = (double) (cpu.sys  - cpu_last.sys)  / total * 100.0;
	}

	sprintf (buffer, "CPU: %5.2f%% user, %5.2f%% system", user, sys);

	gtk_label_set_text (GTK_LABEL (label), buffer);

	memcpy (&cpu_last, &cpu, sizeof (glibtop_cpu));

	return TRUE;
}

static gint
statusbar_mem_cb (gpointer label)
{
	gchar buffer [BUFSIZ];
	glong total, used, free;

	glibtop_mem mem;

	glibtop_get_mem (&mem);

	total = mem.total / 1024;
	free  = mem.free  / 1024;
	used  = total - free;

	if (SUMMARY_MODE_BIT (TEXT_SWAP)) {
		glibtop_swap swap;

		glibtop_get_swap (&swap);

		sprintf (buffer, "Memory: %ldK used, %ldK free, %ldK swapped",
			 used, free, (long) (swap.used / 1024));
	} else {
		sprintf (buffer, "Memory: %ldK used, %ldK free", used, free);
	}

	gtk_label_set_text (GTK_LABEL (label), buffer);

	return TRUE;
}

static void
statusbar_destroy_cb (GtkWidget *status_bar, GTopStatusBarData *d)
{
	if (d->status_cpu_timeout > 0)
		gtk_timeout_remove (d->status_cpu_timeout);

	if (d->status_mem_timeout > 0)
		gtk_timeout_remove (d->status_mem_timeout);

	if (d->status_uptime_timeout > 0)
		gtk_timeout_remove (d->status_uptime_timeout);

	if (d->status_loadavg_timeout > 0)
		gtk_timeout_remove (d->status_loadavg_timeout);

	statusbar_object_list = g_list_remove (statusbar_object_list, d);
}

GTopStatusBarData *
gtop_statusbar_new (void)
{
	GTopStatusBarData *d = g_new0 (GTopStatusBarData, 1);
	GtkWidget *label;
	GtkStyle *style;

	d->container = gtk_hbox_new (FALSE, GNOME_PAD_SMALL);
	gtk_container_border_width
		(GTK_CONTAINER (d->container), GNOME_PAD_SMALL);

	style = gtk_style_copy (gtk_widget_get_style (d->container));

	gdk_font_unref (style->font);
	style->font = gtop_properties.summary.statusbar_font;
	gdk_font_ref (style->font);

	gtk_widget_push_style (style);
	
	d->status_bar = gnome_appbar_new
		(FALSE, TRUE, GNOME_PREFERENCES_NEVER);

	gtk_box_pack_start
		(GTK_BOX (d->container), d->status_bar, TRUE, TRUE, 0);

	d->status_cpu = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type (GTK_FRAME (d->status_cpu), GTK_SHADOW_IN);

	label = gtk_label_new ("");
	gtk_misc_set_padding (GTK_MISC (label), GNOME_PAD, 0);
	gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);

	gtk_container_add (GTK_CONTAINER (d->status_cpu), label);
	d->status_cpu_label = label;
	gtk_widget_show (label);

	gtk_box_pack_start
		(GTK_BOX (d->container), d->status_cpu, FALSE, FALSE, 0);

	d->status_mem = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type (GTK_FRAME (d->status_mem), GTK_SHADOW_IN);

	label = gtk_label_new ("");
	gtk_misc_set_padding (GTK_MISC (label), GNOME_PAD, 0);
	gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);

	gtk_container_add (GTK_CONTAINER (d->status_mem), label);
	d->status_mem_label = label;
	gtk_widget_show (label);

	gtk_box_pack_start
		(GTK_BOX (d->container), d->status_mem, FALSE, FALSE, 0);

	d->status_uptime = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type (GTK_FRAME (d->status_uptime), GTK_SHADOW_IN);

	label = gtk_label_new ("");
	gtk_misc_set_padding (GTK_MISC (label), GNOME_PAD, 0);
	gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);

	gtk_container_add (GTK_CONTAINER (d->status_uptime), label);
	d->status_uptime_label = label;
	gtk_widget_show (label);

	gtk_box_pack_start
		(GTK_BOX (d->container), d->status_uptime, FALSE, FALSE, 0);

	d->status_loadavg = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type (GTK_FRAME (d->status_loadavg), GTK_SHADOW_IN);

	label = gtk_label_new ("");
	gtk_misc_set_padding (GTK_MISC (label), GNOME_PAD, 0);
	gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);

	gtk_container_add (GTK_CONTAINER (d->status_loadavg), label);
	d->status_loadavg_label = label;
	gtk_widget_show (label);

	gtk_box_pack_start
		(GTK_BOX (d->container), d->status_loadavg, FALSE, FALSE, 0);

	gtk_widget_pop_style ();
	gtk_style_unref (style);

	update_font (GNOME_APPBAR (d->status_bar)->status);
	update_font (d->status_cpu_label);
	update_font (d->status_mem_label);
	update_font (d->status_uptime_label);
	update_font (d->status_loadavg_label);

	statusbar_object_list = g_list_append (statusbar_object_list, d);

	gtk_signal_connect (GTK_OBJECT (d->status_bar), "destroy",
			    statusbar_destroy_cb, d);

	return d;
}

void
gtop_statusbar_update (GTopStatusBarData *d)
{
	if (SUMMARY_MODE_BIT (SHOW_TEXT))
		gtk_widget_show (d->container);
	else
		gtk_widget_hide (d->container);

    if (SUMMARY_TEXT_BIT (TEXT_STATUSBAR)) {
		/* Normally, the hostname won't change while GTop is running,
		 * so we don't need a timeout here - but we need to set the
		 * label text here since the user can choose whether to use
		 * the fully qualified hostname in the properties dialog.
		 */

		if (SUMMARY_MODE_BIT (TEXT_HOSTNAME)) {
			struct hostent *hent = NULL;
			char buffer [BUFSIZ];

			if (gethostname (buffer, BUFSIZ-1))
				strcpy (buffer, _("unknown"));
			else if (SUMMARY_MODE_BIT (TEXT_USE_FQDN))
				hent = gethostbyname (buffer);

			gnome_appbar_set_default
				(GNOME_APPBAR (d->status_bar),
				 hent ? hent->h_name : buffer);
		} else {
			gnome_appbar_set_default
				(GNOME_APPBAR (d->status_bar), "");
		}

		gtk_widget_show (d->status_bar);
	} else {
		gtk_widget_hide (d->status_bar);
	}

    if (SUMMARY_TEXT_BIT (TEXT_CPU)) {
		gtk_widget_show (d->status_cpu);
		statusbar_cpu_cb (d->status_cpu_label);
		if (d->status_cpu_timeout > 0)
			gtk_timeout_remove (d->status_cpu_timeout);
		d->status_cpu_timeout = gtk_timeout_add
			(gtop_properties.global.update_times
			 [GTOP_UPDATE_STATUS_CPU],
			 statusbar_cpu_cb,
			 d->status_cpu_label);
	} else {
		if (d->status_cpu_timeout > 0) {
			gtk_timeout_remove (d->status_cpu_timeout);
			d->status_cpu_timeout = 0;
		}
		gtk_widget_hide (d->status_cpu);
	}

    if (SUMMARY_TEXT_BIT (TEXT_MEMORY)) {
		gtk_widget_show (d->status_mem);
		statusbar_mem_cb (d->status_mem_label);
		if (d->status_mem_timeout > 0)
			gtk_timeout_remove (d->status_mem_timeout);
		d->status_mem_timeout = gtk_timeout_add
			(gtop_properties.global.update_times
			 [GTOP_UPDATE_STATUS_MEMORY],
			 statusbar_mem_cb,
			 d->status_mem_label);
	} else {
		if (d->status_mem_timeout > 0) {
			gtk_timeout_remove (d->status_mem_timeout);
			d->status_mem_timeout = 0;
		}
		gtk_widget_hide (d->status_mem);
	}

    if (SUMMARY_TEXT_BIT (TEXT_UPTIME)) {
		gtk_widget_show (d->status_uptime);
		statusbar_uptime_cb (d->status_uptime_label);
		if (d->status_uptime_timeout > 0)
			gtk_timeout_remove (d->status_uptime_timeout);
		d->status_uptime_timeout = gtk_timeout_add
			(gtop_properties.global.update_times
			 [GTOP_UPDATE_STATUS_UPTIME],
			 statusbar_uptime_cb,
			 d->status_uptime_label);
	} else {
		if (d->status_uptime_timeout > 0) {
			gtk_timeout_remove (d->status_uptime_timeout);
			d->status_uptime_timeout = 0;
		}
		gtk_widget_hide (d->status_uptime);
	}

    if (SUMMARY_TEXT_BIT (TEXT_LOADAVG)) {
		gtk_widget_show (d->status_loadavg);
		statusbar_loadavg_cb (d->status_loadavg_label);
		if (d->status_loadavg_timeout > 0)
			gtk_timeout_remove (d->status_loadavg_timeout);
		d->status_loadavg_timeout = gtk_timeout_add
			(gtop_properties.global.update_times
			 [GTOP_UPDATE_STATUS_LOADAVG],
			 statusbar_loadavg_cb,
			 d->status_loadavg_label);
	} else {
		if (d->status_loadavg_timeout > 0) {
			gtk_timeout_remove (d->status_loadavg_timeout);
			d->status_loadavg_timeout = 0;
		}
		gtk_widget_hide (d->status_loadavg);
	}
}

/* =======================================================================
 * Summary properties code.
 * =======================================================================
 */

static void
summary_mode_cb (GtkWidget *widget, RadioButtonCbData *cb_data)
{
	GTopSummaryProperties *prop_ptr = cb_data->object->temp_data;

	if (GTK_TOGGLE_BUTTON (cb_data->button)->active)
		prop_ptr->summary_mode |= (1 << cb_data->index);
	else
		prop_ptr->summary_mode &= ~(1 << cb_data->index);

	gtop_properties_changed ();
}

static void
adjustment_changed_cb (GtkWidget *widget,
		       GtkWidget *adjustment)
{
	gtop_properties_changed ();
}

static void
update_font (GtkWidget *widget)
{
	GtkStyle *style;

	if (!widget) return;

	gtk_widget_ensure_style (widget);
	style = gtk_style_copy (gtk_widget_get_style (widget));

	gdk_font_unref (style->font);
	style->font = gtop_properties.summary.statusbar_font;
	gdk_font_ref (style->font);

	gtk_widget_set_style (widget, style);
	gtk_style_unref (style);
}

static void
summary_properties_update (GnomePropertyObject *object)
{
	GList *c;

	for (c = statusbar_object_list; c; c = c->next) {
		GTopStatusBarData *d = c->data;

		update_font (GNOME_APPBAR (d->status_bar)->status);
		update_font (d->status_cpu_label);
		update_font (d->status_mem_label);
		update_font (d->status_uptime_label);
		update_font (d->status_loadavg_label);

		gtop_statusbar_update (d);
	}
}

static GtkWidget *
summary_properties_init (GnomePropertyObject *object)
{
	GTopSummaryProperties *prop_ptr = &gtop_properties.summary;
	GtkWidget *vb, *hb1, *frame, *label, *table, *spin, *hb;
	RadioButtonCbData *cb_data;
	GtkObject *adjustment;
	glong summary_mode;
	glong summary_supported;

	vb    = gtk_vbox_new (FALSE, 0);
	gtk_container_border_width (GTK_CONTAINER (vb), GNOME_PAD_SMALL);

	frame = gtk_frame_new (_("Statusbar Font"));

	table = gnome_property_entry_font
		(object, _("Statusbar Font"),
		 &gtop_properties.summary.statusbar_font_name,
		 &gtop_properties.summary.statusbar_font);

	gtk_container_add (GTK_CONTAINER (frame), table);
	gtk_container_border_width (GTK_CONTAINER (table), GNOME_PAD_SMALL);
	gtk_box_pack_start (GTK_BOX (vb), frame, FALSE, TRUE, 0);

	frame = gtk_frame_new (_("Graphical Summary"));
	table = gtk_table_new (7, 3, FALSE);
	gtk_table_set_col_spacings (GTK_TABLE (table), GNOME_PAD << 1);
	gtk_container_border_width (GTK_CONTAINER (table), GNOME_PAD_SMALL);

	summary_mode = prop_ptr->summary_mode;
	summary_supported = prop_ptr->summary_supported;

	label = gtk_check_button_new_with_label (_("Show graphical summary"));
	if (summary_mode & (1 << GTOP_SUMMARY_SHOW_GRAPH))
		gtk_toggle_button_set_state (GTK_TOGGLE_BUTTON (label), TRUE);

	cb_data = g_new0 (RadioButtonCbData, 1);
	cb_data->index = GTOP_SUMMARY_SHOW_GRAPH;
	cb_data->button = label;
	cb_data->object = object;
	
	if (!(summary_supported & (1 << GTOP_SUMMARY_SHOW_GRAPH)))
		gtk_widget_set_sensitive (label, FALSE);
	
	gtk_signal_connect
		(GTK_OBJECT (label), "toggled", summary_mode_cb, cb_data);
	gtk_table_attach (GTK_TABLE (table), label, 0, 3, 0, 1,
			  GTK_EXPAND | GTK_FILL, GTK_FILL, 0, 0);

	label = gtk_check_button_new_with_label (_("Show cpu usage"));
	if (summary_mode & (1 << GTOP_SUMMARY_GRAPH_CPU))
		gtk_toggle_button_set_state (GTK_TOGGLE_BUTTON (label), TRUE);

	cb_data = g_new0 (RadioButtonCbData, 1);
	cb_data->index = GTOP_SUMMARY_GRAPH_CPU;
	cb_data->button = label;
	cb_data->object = object;

	if (!(summary_supported & (1 << GTOP_SUMMARY_GRAPH_CPU)))
		gtk_widget_set_sensitive (label, FALSE);
	
	gtk_signal_connect
		(GTK_OBJECT (label), "toggled", summary_mode_cb, cb_data);
	gtk_table_attach (GTK_TABLE (table), label, 1, 3, 1, 2,
			  GTK_EXPAND | GTK_FILL, GTK_FILL, 0, 0);

	label = gtk_check_button_new_with_label (_("Enable SMP support"));
	if (summary_mode & (1 << GTOP_SUMMARY_GRAPH_XCPU))
		gtk_toggle_button_set_state (GTK_TOGGLE_BUTTON (label), TRUE);

	cb_data = g_new0 (RadioButtonCbData, 1);
	cb_data->index = GTOP_SUMMARY_GRAPH_XCPU;
	cb_data->button = label;
	cb_data->object = object;
	
	if (!(summary_supported & (1 << GTOP_SUMMARY_GRAPH_XCPU)))
		gtk_widget_set_sensitive (label, FALSE);
	
	gtk_signal_connect
		(GTK_OBJECT (label), "toggled", summary_mode_cb, cb_data);
	gtk_table_attach (GTK_TABLE (table), label, 2, 3, 2, 3,
			  GTK_EXPAND | GTK_FILL, GTK_FILL, 0, 0);

	label = gtk_check_button_new_with_label (_("Show physical memory usage"));
	if (summary_mode & (1 << GTOP_SUMMARY_GRAPH_MEM))
		gtk_toggle_button_set_state (GTK_TOGGLE_BUTTON (label), TRUE);

	cb_data = g_new0 (RadioButtonCbData, 1);
	cb_data->index = GTOP_SUMMARY_GRAPH_MEM;
	cb_data->button = label;
	cb_data->object = object;
	
	if (!(summary_supported & (1 << GTOP_SUMMARY_GRAPH_MEM)))
		gtk_widget_set_sensitive (label, FALSE);
	
	gtk_signal_connect
		(GTK_OBJECT (label), "toggled", summary_mode_cb, cb_data);
	gtk_table_attach (GTK_TABLE (table), label, 1, 3, 3, 4,
			  GTK_EXPAND | GTK_FILL, GTK_FILL, 0, 0);

	label = gtk_check_button_new_with_label (_("Show swap memory usage"));
	if (summary_mode & (1 << GTOP_SUMMARY_GRAPH_SWAP))
		gtk_toggle_button_set_state (GTK_TOGGLE_BUTTON (label), TRUE);

	cb_data = g_new0 (RadioButtonCbData, 1);
	cb_data->index = GTOP_SUMMARY_GRAPH_SWAP;
	cb_data->button = label;
	cb_data->object = object;
	
	if (!(summary_supported & (1 << GTOP_SUMMARY_GRAPH_SWAP)))
		gtk_widget_set_sensitive (label, FALSE);
	
	gtk_signal_connect
		(GTK_OBJECT (label), "toggled", summary_mode_cb, cb_data);
	gtk_table_attach (GTK_TABLE (table), label, 1, 3, 4, 5,
			  GTK_EXPAND | GTK_FILL, GTK_FILL, 0, 0);

	label = gtk_check_button_new_with_label (_("Show load average"));
	if (summary_mode & (1 << GTOP_SUMMARY_GRAPH_LOAD))
		gtk_toggle_button_set_state (GTK_TOGGLE_BUTTON (label), TRUE);

	cb_data = g_new0 (RadioButtonCbData, 1);
	cb_data->index = GTOP_SUMMARY_GRAPH_LOAD;
	cb_data->button = label;
	cb_data->object = object;
	
	if (!(summary_supported & (1 << GTOP_SUMMARY_GRAPH_LOAD)))
		gtk_widget_set_sensitive (label, FALSE);
	
	gtk_signal_connect
		(GTK_OBJECT (label), "toggled", summary_mode_cb, cb_data);
	gtk_table_attach (GTK_TABLE (table), label, 1, 3, 5, 6,
			  GTK_EXPAND | GTK_FILL, GTK_FILL, 0, 0);

	gtk_container_add (GTK_CONTAINER (frame), table);
	hb1 = gtk_hbox_new (FALSE, GNOME_PAD_SMALL);
	gtk_box_pack_start (GTK_BOX (hb1), frame, TRUE, TRUE, 0);

	hb = gtk_hbox_new (FALSE, GNOME_PAD);

	label = gtk_label_new (_("Maximum load average:"));
	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
	gtk_box_pack_start_defaults (GTK_BOX (hb), label);

	if (!(summary_supported & (1 << GTOP_SUMMARY_GRAPH_LOAD)))
		gtk_widget_set_sensitive (label, FALSE);
	
	spin = gtk_spin_button_new (NULL, 0.1, 1);
	adjustment = gtk_adjustment_new (1.0, 1.0, 10000.0, 0.1, 1.0, 1.0);
	gtk_spin_button_set_adjustment
		(GTK_SPIN_BUTTON (spin), GTK_ADJUSTMENT (adjustment));
	gtk_spin_button_set_value
		(GTK_SPIN_BUTTON (spin), prop_ptr->maximum_loadavg);
	gtk_widget_set_usize (spin, 100, -1);
	gtk_box_pack_start_defaults (GTK_BOX (hb), spin);
	gtk_table_attach (GTK_TABLE (table), hb, 2, 3, 6, 7,
			  GTK_EXPAND | GTK_FILL, GTK_FILL, 0, 0);

	if (!(summary_supported & (1 << GTOP_SUMMARY_GRAPH_LOAD)))
		gtk_widget_set_sensitive (spin, FALSE);
	
	gtk_signal_connect
		(GTK_OBJECT (adjustment), "value_changed",
		 adjustment_changed_cb, adjustment);

	max_loadavg_adjustment = GTK_ADJUSTMENT (adjustment);

	frame = gtk_frame_new (_("Textual Summary"));
	table = gtk_table_new (9, 3, FALSE);
	gtk_table_set_col_spacings (GTK_TABLE (table), GNOME_PAD << 1);
	gtk_container_border_width (GTK_CONTAINER (table), GNOME_PAD_SMALL);

	label = gtk_check_button_new_with_label (_("Show text summary"));
	if (summary_mode & (1 << GTOP_SUMMARY_SHOW_TEXT))
		gtk_toggle_button_set_state (GTK_TOGGLE_BUTTON (label), TRUE);

	cb_data = g_new0 (RadioButtonCbData, 1);

	cb_data->index = GTOP_SUMMARY_SHOW_TEXT;
	cb_data->button = label;	
	cb_data->object = object;

	gtk_signal_connect
		(GTK_OBJECT (label), "toggled", summary_mode_cb, cb_data);

	gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 3, 0, 1);

	label = gtk_check_button_new_with_label (_("Show statusbar"));
	if (summary_mode & (1 << GTOP_SUMMARY_TEXT_STATUSBAR))
		gtk_toggle_button_set_state (GTK_TOGGLE_BUTTON (label), TRUE);

	cb_data = g_new0 (RadioButtonCbData, 1);
	cb_data->index = GTOP_SUMMARY_TEXT_STATUSBAR;
	cb_data->button = label;
	cb_data->object = object;
	
	gtk_signal_connect
		(GTK_OBJECT (label), "toggled", summary_mode_cb, cb_data);
	gtk_table_attach_defaults (GTK_TABLE (table), label, 1, 3, 1, 2);

	label = gtk_check_button_new_with_label (_("include hostname"));
	if (summary_mode & (1 << GTOP_SUMMARY_TEXT_HOSTNAME))
		gtk_toggle_button_set_state (GTK_TOGGLE_BUTTON (label), TRUE);

	cb_data = g_new0 (RadioButtonCbData, 1);
	cb_data->index = GTOP_SUMMARY_TEXT_HOSTNAME;
	cb_data->button = label;
	cb_data->object = object;
	
	gtk_signal_connect
		(GTK_OBJECT (label), "toggled", summary_mode_cb, cb_data);
	gtk_table_attach_defaults (GTK_TABLE (table), label, 2, 3, 2, 3);

	label = gtk_check_button_new_with_label (_("use fully qualified name"));
	if (summary_mode & (1 << GTOP_SUMMARY_TEXT_USE_FQDN))
		gtk_toggle_button_set_state (GTK_TOGGLE_BUTTON (label), TRUE);

	cb_data = g_new0 (RadioButtonCbData, 1);
	cb_data->index = GTOP_SUMMARY_TEXT_USE_FQDN;
	cb_data->button = label;
	cb_data->object = object;
	
	gtk_signal_connect
		(GTK_OBJECT (label), "toggled", summary_mode_cb, cb_data);
	gtk_table_attach_defaults (GTK_TABLE (table), label, 2, 3, 3, 4);

	label = gtk_check_button_new_with_label (_("Show cpu statistics"));
	if (summary_mode & (1 << GTOP_SUMMARY_TEXT_CPU))
		gtk_toggle_button_set_state (GTK_TOGGLE_BUTTON (label), TRUE);

	cb_data = g_new0 (RadioButtonCbData, 1);
	cb_data->index = GTOP_SUMMARY_TEXT_CPU;
	cb_data->button = label;
	cb_data->object = object;
	
	if (!(summary_supported & (1 << GTOP_SUMMARY_TEXT_CPU)))
		gtk_widget_set_sensitive (label, FALSE);

	gtk_signal_connect
		(GTK_OBJECT (label), "toggled", summary_mode_cb, cb_data);
	gtk_table_attach_defaults (GTK_TABLE (table), label, 1, 3, 4, 5);

	label = gtk_check_button_new_with_label (_("Show memory statistics"));
	if (summary_mode & (1 << GTOP_SUMMARY_TEXT_MEMORY))
		gtk_toggle_button_set_state (GTK_TOGGLE_BUTTON (label), TRUE);

	cb_data = g_new0 (RadioButtonCbData, 1);
	cb_data->index = GTOP_SUMMARY_TEXT_MEMORY;
	cb_data->button = label;
	cb_data->object = object;
	
	if (!(summary_supported & (1 << GTOP_SUMMARY_TEXT_MEMORY)))
		gtk_widget_set_sensitive (label, FALSE);

	gtk_signal_connect
		(GTK_OBJECT (label), "toggled", summary_mode_cb, cb_data);
	gtk_table_attach_defaults (GTK_TABLE (table), label, 1, 3, 5, 6);

	label = gtk_check_button_new_with_label (_("include swap statistics"));
	if (summary_mode & (1 << GTOP_SUMMARY_TEXT_SWAP))
		gtk_toggle_button_set_state (GTK_TOGGLE_BUTTON (label), TRUE);

	cb_data = g_new0 (RadioButtonCbData, 1);
	cb_data->index = GTOP_SUMMARY_TEXT_SWAP;
	cb_data->button = label;
	cb_data->object = object;
	
	if (!(summary_supported & (1 << GTOP_SUMMARY_TEXT_SWAP)))
		gtk_widget_set_sensitive (label, FALSE);

	gtk_signal_connect
		(GTK_OBJECT (label), "toggled", summary_mode_cb, cb_data);
	gtk_table_attach_defaults (GTK_TABLE (table), label, 2, 3, 6, 7);

	label = gtk_check_button_new_with_label (_("Show uptime"));
	if (summary_mode & (1 << GTOP_SUMMARY_TEXT_UPTIME))
		gtk_toggle_button_set_state (GTK_TOGGLE_BUTTON (label), TRUE);

	cb_data = g_new0 (RadioButtonCbData, 1);
	cb_data->index = GTOP_SUMMARY_TEXT_UPTIME;
	cb_data->button = label;
	cb_data->object = object;
	
	if (!(summary_supported & (1 << GTOP_SUMMARY_TEXT_UPTIME)))
		gtk_widget_set_sensitive (label, FALSE);

	gtk_signal_connect
		(GTK_OBJECT (label), "toggled", summary_mode_cb, cb_data);
	gtk_table_attach_defaults (GTK_TABLE (table), label, 1, 3, 7, 8);

	label = gtk_check_button_new_with_label (_("Show load average"));
	if (summary_mode & (1 << GTOP_SUMMARY_TEXT_LOADAVG))
		gtk_toggle_button_set_state (GTK_TOGGLE_BUTTON (label), TRUE);

	cb_data = g_new0 (RadioButtonCbData, 1);
	cb_data->index = GTOP_SUMMARY_TEXT_LOADAVG;
	cb_data->button = label;
	cb_data->object = object;
	
	if (!(summary_supported & (1 << GTOP_SUMMARY_TEXT_LOADAVG)))
		gtk_widget_set_sensitive (label, FALSE);

	gtk_signal_connect
		(GTK_OBJECT (label), "toggled", summary_mode_cb, cb_data);
	gtk_table_attach_defaults (GTK_TABLE (table), label, 1, 3, 8, 9);

	gtk_container_add (GTK_CONTAINER (frame), table);
	gtk_box_pack_start (GTK_BOX (hb1), frame, TRUE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (vb), hb1, FALSE, TRUE, GNOME_PAD_SMALL);

	return vb;
}

static void
summary_properties_changed (GnomePropertyObject *object)
{
	gtop_properties_changed ();
}

static void
summary_properties_load (GnomePropertyObject *object)
{
	gchar name [BUFSIZ];

	gint max_loadavg;

	gtop_properties.summary.summary_mode =
		gnome_config_get_int ("gtop/procview/summary_mode=30623");

	summary_check_libgtop_support ();

	sprintf (name, "gtop/procview/statusbar_font_name=%s",
		 STATUSBAR_DEFAULT_FONT);
	gtop_properties.summary.statusbar_font_name =
		gnome_config_get_string (name);

	gtop_properties.summary.statusbar_font = gdk_fontset_load
		(gtop_properties.summary.statusbar_font_name);

	if (!gtop_properties.summary.statusbar_font) {
		g_free (gtop_properties.summary.statusbar_font_name);
		gtop_properties.summary.statusbar_font_name =
			g_strdup (STATUSBAR_DEFAULT_FONT);
		gtop_properties.summary.statusbar_font = gdk_fontset_load
			(gtop_properties.summary.statusbar_font_name);
	}

	max_loadavg = gnome_config_get_int
		("gtop/procview/max_loadavg=200");

	gtop_properties.summary.maximum_loadavg = (float) max_loadavg / 10.0;
}

static void
summary_properties_save (GnomePropertyObject *object)
{
	gint max_loadavg;

	gnome_config_set_int ("gtop/procview/summary_mode",
			      gtop_properties.summary.summary_mode);

	gnome_config_set_string ("gtop/procview/statusbar_font_name",
				 gtop_properties.summary.statusbar_font_name);

	max_loadavg = (gint) (gtop_properties.summary.maximum_loadavg * 10.0);
	gnome_config_set_int ("gtop/procview/max_loadavg", max_loadavg);
}

static void
summary_properties_apply (GnomePropertyObject *object)
{
	GTopSummaryProperties *prop_ptr = object->temp_data;

	prop_ptr->maximum_loadavg = max_loadavg_adjustment->value;
}

/* =======================================================================
 * Summary colors properties code.
 * =======================================================================
 */

static void
summary_colors_properties_update (GnomePropertyObject *object)
{
	GList *c;

	for (c = summary_object_list; c; c = c->next) {
		GTopSummaryData *d = c->data;
		gint i;

		summary_update (d);

		if (d->summary_cpu && SUMMARY_GRAPH_BIT (GRAPH_CPU) &&
		    GTK_WIDGET_REALIZED (d->summary_cpu))
			gtop_proc_bar_update
				(GTOP_PROC_BAR (d->summary_cpu),
				 gtop_properties.summary_colors.cpu);

#ifdef HAVE_LIBGTOP_SMP
		for (i = 0; i < ncpu; i++)
			if (d->summary_xcpu [i] &&
			    SUMMARY_GRAPH_BIT (GRAPH_XCPU) &&
			    GTK_WIDGET_REALIZED (d->summary_xcpu [i]))
				gtop_proc_bar_update
					(GTOP_PROC_BAR (d->summary_xcpu [i]),
					 gtop_properties.summary_colors.cpu);
#endif

		if (d->summary_mem && SUMMARY_GRAPH_BIT (GRAPH_MEM) &&
		    GTK_WIDGET_REALIZED (d->summary_mem))
			gtop_proc_bar_update
				(GTOP_PROC_BAR (d->summary_mem),
				 gtop_properties.summary_colors.mem);

		if (d->summary_swap && SUMMARY_GRAPH_BIT (GRAPH_SWAP) &&
		    GTK_WIDGET_REALIZED (d->summary_swap))
			gtop_proc_bar_update
				(GTOP_PROC_BAR (d->summary_swap),
				 gtop_properties.summary_colors.swap);

		if (d->summary_load && SUMMARY_GRAPH_BIT (GRAPH_LOAD) &&
		    GTK_WIDGET_REALIZED (d->summary_load))
			gtop_proc_bar_update
				(GTOP_PROC_BAR (d->summary_load),
				 gtop_properties.summary_colors.load);
	}
}

static GtkWidget *
summary_colors_properties_init (GnomePropertyObject *object)
{
	GTopSummaryColorsProperties *prop_ptr = &gtop_properties.summary_colors;
	GtkWidget *vb;
	
	gint pos [2] = { 0, 3 };

	vb = gtk_vbox_new (FALSE, 0);
	gtk_container_border_width (GTK_CONTAINER (vb), GNOME_PAD_SMALL);

	gtk_box_pack_start (GTK_BOX (vb),
			    gnome_property_entry_colors
			    (object, N_("Summary CPU"),
			     4, 4, NULL, prop_ptr->cpu, cpu_texts),
			    FALSE, TRUE, 0);

	gtk_box_pack_start (GTK_BOX (vb),
			    gnome_property_entry_colors
			    (object, N_("Summary physical memory"),
			     4, 4, NULL, prop_ptr->mem, mem_texts),
			    FALSE, TRUE, 0);

	gtk_box_pack_start (GTK_BOX (vb),
			    gnome_property_entry_colors
			    (object, N_("Summary swap memory"),
			     2, 4, pos, prop_ptr->swap, swap_texts),
			    FALSE, TRUE, 0);

	gtk_box_pack_start (GTK_BOX (vb),
			    gnome_property_entry_colors
			    (object, N_("Summary load average"),
			     2, 4, pos, prop_ptr->load, load_texts),
			    FALSE, TRUE, 0);

	return vb;
}

static void
summary_colors_properties_changed (GnomePropertyObject *object)
{
	GTopSummaryColorsProperties *temp_ptr = object->temp_data;
	GTopSummaryColorsProperties *prop_ptr = object->prop_data;
	gint i;

	/* [FIXME]: Yes, this is an ugly hack, but we cannot fix it in
	 *          libgnomeui right now. Sep 29, 1999 Martin Baulig */

	for (i = 0; i < 4; i++)
		temp_ptr->cpu [i] = prop_ptr->cpu [i];

	for (i = 0; i < 4; i++)
		temp_ptr->mem [i] = prop_ptr->mem [i];

	for (i = 0; i < 2; i++)
		temp_ptr->swap [i] = prop_ptr->swap [i];

	for (i = 0; i < 2; i++)
		temp_ptr->load [i] = prop_ptr->load [i];

	summary_colors_properties_update (object);

	gtop_properties_changed ();
}

static void
summary_colors_properties_load (GnomePropertyObject *object)
{
	char name [BUFSIZ], *tmp;
	gint i;

	for (i=0; i<4; i++) {
		GdkColor *color = &(gtop_properties.summary_colors.cpu [i]);

		sprintf (name, "gtop/summary_cpu/color%d=#%04x%04x%04x", i,
			 cpu_defaults [i].red,
			 cpu_defaults [i].green,
			 cpu_defaults [i].blue);

		tmp = gnome_config_get_string (name);
		gdk_color_parse (tmp, color);
		g_free (tmp);
	}

	for (i=0; i<4; i++) {
		GdkColor *color = &(gtop_properties.summary_colors.mem [i]);

		sprintf (name, "gtop/summary_mem/color%d=#%04x%04x%04x", i,
			 mem_defaults [i].red,
			 mem_defaults [i].green,
			 mem_defaults [i].blue);

		tmp = gnome_config_get_string (name);
		gdk_color_parse (tmp, color);
		g_free (tmp);
	}

	for (i=0; i<2; i++) {
		GdkColor *color = &(gtop_properties.summary_colors.swap [i]);

		sprintf (name, "gtop/summary_swap/color%d=#%04x%04x%04x", i,
			 swap_defaults [i].red,
			 swap_defaults [i].green,
			 swap_defaults [i].blue);

		tmp = gnome_config_get_string (name);
		gdk_color_parse (tmp, color);
		g_free (tmp);
	}

	for (i=0; i<2; i++) {
		GdkColor *color = &(gtop_properties.summary_colors.load [i]);

		sprintf (name, "gtop/summary_load/color%d=#%04x%04x%04x", i,
			 load_defaults [i].red,
			 load_defaults [i].green,
			 load_defaults [i].blue);

		tmp = gnome_config_get_string (name);
		gdk_color_parse (tmp, color);
		g_free (tmp);
	}

}

static void
summary_colors_properties_save (GnomePropertyObject *object)
{
	char name [BUFSIZ], tmp [BUFSIZ];
	gint i;

	for (i=0; i<4; i++) {
		GdkColor *color = &(gtop_properties.summary_colors.cpu [i]);
		
		sprintf (tmp, "#%04x%04x%04x",
			 color->red, color->green, color->blue);

		sprintf (name, "gtop/summary_cpu/color%d", i);
		gnome_config_set_string (name, tmp);
	}

	for (i=0; i<4; i++) {
		GdkColor *color = &(gtop_properties.summary_colors.mem [i]);

		sprintf (tmp, "#%04x%04x%04x",
			 color->red, color->green, color->blue);

		sprintf (name, "gtop/summary_mem/color%d", i);
		gnome_config_set_string (name, tmp);
	}

	for (i=0; i<2; i++) {
		GdkColor *color = &(gtop_properties.summary_colors.swap [i]);

		sprintf (tmp, "#%04x%04x%04x",
			 color->red, color->green, color->blue);

		sprintf (name, "gtop/summary_swap/color%d", i);
		gnome_config_set_string (name, tmp);
	}

	for (i=0; i<2; i++) {
		GdkColor *color = &(gtop_properties.summary_colors.load [i]);

		sprintf (tmp, "#%04x%04x%04x",
			 color->red, color->green, color->blue);

		sprintf (name, "gtop/summary_load/color%d", i);
		gnome_config_set_string (name, tmp);
	}
}

