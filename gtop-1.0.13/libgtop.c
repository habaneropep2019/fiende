#include <config.h>
#include <gnome.h>
#include <global.h>

#include <summary.h>

#include <glibtop.h>
#include <glibtop/union.h>
#include <glibtop/parameter.h>

static u_int64_t _gtop_libgtop_sysdeps_text_cpu =
(1 << GLIBTOP_CPU_USER) + (1 << GLIBTOP_CPU_NICE) +
(1 << GLIBTOP_CPU_SYS) + (1 << GLIBTOP_CPU_IDLE);

static u_int64_t _gtop_libgtop_sysdeps_text_memory =
(1 << GLIBTOP_MEM_TOTAL) + (1 << GLIBTOP_MEM_USED) +
(1 << GLIBTOP_MEM_FREE) + (1 << GLIBTOP_MEM_SHARED) +
(1 << GLIBTOP_MEM_BUFFER);

static u_int64_t _gtop_libgtop_sysdeps_text_swap =
(1 << GLIBTOP_SWAP_TOTAL) + (1 << GLIBTOP_SWAP_USED) +
(1 << GLIBTOP_SWAP_FREE);

static u_int64_t _gtop_libgtop_sysdeps_text_load =
(1 << GLIBTOP_LOADAVG_LOADAVG);

static u_int64_t _gtop_libgtop_sysdeps_text_uptime =
(1 << GLIBTOP_UPTIME_UPTIME);

static u_int64_t _gtop_libgtop_sysdeps_graph_cpu =
(1 << GLIBTOP_CPU_USER) + (1 << GLIBTOP_CPU_NICE) +
(1 << GLIBTOP_CPU_SYS) + (1 << GLIBTOP_CPU_IDLE);

static u_int64_t _gtop_libgtop_sysdeps_graph_xcpu =
(1 << GLIBTOP_XCPU_USER) + (1 << GLIBTOP_XCPU_NICE) +
(1 << GLIBTOP_XCPU_SYS) + (1 << GLIBTOP_XCPU_IDLE);

static u_int64_t _gtop_libgtop_sysdeps_graph_mem =
(1 << GLIBTOP_MEM_TOTAL) + (1 << GLIBTOP_MEM_USED) +
(1 << GLIBTOP_MEM_FREE) + (1 << GLIBTOP_MEM_SHARED) +
(1 << GLIBTOP_MEM_BUFFER);

static u_int64_t _gtop_libgtop_sysdeps_graph_swap =
(1 << GLIBTOP_SWAP_TOTAL) + (1 << GLIBTOP_SWAP_USED) +
(1 << GLIBTOP_SWAP_FREE);

static u_int64_t _gtop_libgtop_sysdeps_graph_load =
(1 << GLIBTOP_LOADAVG_LOADAVG);

void
gtop_init_libgtop (void)
{
	glibtop *server = glibtop_global_server;
	/* LibGTop 1.0.1 has a bug which will make this print out a lot of trash;
	 * so you need at least 1.0.2 if you really want to have error checking.  */
#if LIBGTOP_VERSION_CODE >= 1000002
	unsigned error_method = GLIBTOP_ERROR_METHOD_WARN_ONCE;
#else
	unsigned error_method = 0;
#endif

	/* Initialize LibGTop server. */
	glibtop_init ();

	/* Make sure LibGTop returns all fields it claims to support. */
	memcpy (&server->required, &server->sysdeps,
		sizeof (glibtop_sysdeps));

	/* If LibGTop fails to return some value it claims to support,
	 * warn once and then modify the sysdeps so it does no longer
	 * claim to support that field. */
	glibtop_set_parameter (GLIBTOP_PARAM_ERROR_METHOD,
			       &error_method, sizeof (error_method));
}

gint
gtop_libgtop_is_summary_supported (gint summary_mode)
{
	glibtop *server = glibtop_global_server;
	u_int64_t provided, required;

	switch (summary_mode) {
	case GTOP_SUMMARY_TEXT_CPU:
		provided = server->sysdeps.cpu;
		required = _gtop_libgtop_sysdeps_text_cpu;
		break;
	case GTOP_SUMMARY_TEXT_MEMORY:
		provided = server->sysdeps.mem;
		required = _gtop_libgtop_sysdeps_text_memory;
		break;
	case GTOP_SUMMARY_TEXT_SWAP:
		provided = server->sysdeps.swap;
		required = _gtop_libgtop_sysdeps_text_swap;
		break;
	case GTOP_SUMMARY_TEXT_LOADAVG:
		provided = server->sysdeps.loadavg;
		required = _gtop_libgtop_sysdeps_text_load;
		break;
	case GTOP_SUMMARY_TEXT_UPTIME:
		provided = server->sysdeps.uptime;
		required = _gtop_libgtop_sysdeps_text_uptime;
		break;
	case GTOP_SUMMARY_GRAPH_CPU:
		provided = server->sysdeps.cpu;
		required = _gtop_libgtop_sysdeps_graph_cpu;
		break;
	case GTOP_SUMMARY_GRAPH_XCPU:
		provided = server->sysdeps.cpu;
		required = _gtop_libgtop_sysdeps_graph_xcpu;
		break;
	case GTOP_SUMMARY_GRAPH_MEM:
		provided = server->sysdeps.mem;
		required = _gtop_libgtop_sysdeps_graph_mem;
		break;
	case GTOP_SUMMARY_GRAPH_SWAP:
		provided = server->sysdeps.swap;
		required = _gtop_libgtop_sysdeps_graph_swap;
		break;
	case GTOP_SUMMARY_GRAPH_LOAD:
		provided = server->sysdeps.loadavg;
		required = _gtop_libgtop_sysdeps_graph_load;
		break;
	default:
		return -1;
	}

	return (provided & required) ? 0 : 1;
}
