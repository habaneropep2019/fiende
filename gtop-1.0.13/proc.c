#include <config.h>

#include "proc.h"

#include <glibtop.h>
#include <glibtop/cpu.h>
#include <glibtop/mem.h>
#include <glibtop/swap.h>
#include <glibtop/loadavg.h>

#include <properties.h>

#include <stdio.h>

void
proc_read_cpu (ProcInfo *ps)
{
	glibtop_cpu cpu;
	int i, j;

	glibtop_get_cpu (&cpu);

	for (i = 0; i < PROC_CPU_SIZE; i++)
		ps->cpu_last [i] = ps->cpu_now [i];

#ifdef HAVE_LIBGTOP_SMP
	for (i = 0; i < PROC_CPU_SIZE; i++)
		for (j = 0; j < 4; j++)
			ps->xcpu_last [j][i] = ps->xcpu_now [j][i];
#endif

	ps->cpu_now [PROC_CPU_USER] = cpu.user;
	ps->cpu_now [PROC_CPU_NICE] = cpu.nice;
	ps->cpu_now [PROC_CPU_SYS]  = cpu.sys;
	ps->cpu_now [PROC_CPU_IDLE] = cpu.idle;

#ifdef HAVE_LIBGTOP_SMP
	for (j = 0; j < 4; j++) {
		ps->xcpu_now [j][PROC_CPU_USER] = cpu.xcpu_user [j];
		ps->xcpu_now [j][PROC_CPU_NICE] = cpu.xcpu_nice [j];
		ps->xcpu_now [j][PROC_CPU_SYS]  = cpu.xcpu_sys  [j];
		ps->xcpu_now [j][PROC_CPU_IDLE] = cpu.xcpu_idle [j];
	}
#endif

	for (i = 1; i < PROC_CPU_SIZE; i++)
		ps->cpu [i] = ps->cpu_now [i] - ps->cpu_last [i];

#ifdef HAVE_LIBGTOP_SMP
	for (i = 1; i < PROC_CPU_SIZE; i++)
		for (j = 0; j < 4; j++)
			ps->xcpu [j][i] =
				ps->xcpu_now [j][i] - ps->xcpu_last [j][i];
#endif

	ps->cpu [PROC_CPU_TOTAL] = 
		ps->cpu [PROC_CPU_USER] +
		ps->cpu [PROC_CPU_NICE] +
		ps->cpu [PROC_CPU_SYS] +
		ps->cpu [PROC_CPU_IDLE];

#ifdef HAVE_LIBGTOP_SMP
	for (j = 0; j < 4; j++)
		ps->xcpu [j][PROC_CPU_TOTAL] =
			ps->xcpu [j][PROC_CPU_USER] +
			ps->xcpu [j][PROC_CPU_NICE] +
			ps->xcpu [j][PROC_CPU_SYS] +
			ps->xcpu [j][PROC_CPU_IDLE];
#endif
}

void
proc_read_mem (ProcInfo *ps)
{
	glibtop_mem mem;
	glibtop_swap swap;

	glibtop_get_mem (&mem);

	ps->mem [PROC_MEM_TOTAL]  = mem.total;
	ps->mem [PROC_MEM_USED]   = mem.used;
	ps->mem [PROC_MEM_FREE]   = mem.free;
	ps->mem [PROC_MEM_SHARED] = mem.shared;
	ps->mem [PROC_MEM_BUF]    = mem.buffer;

	ps->mem [PROC_MEM_USER] = ps->mem [PROC_MEM_TOTAL]
		- ps->mem [PROC_MEM_FREE]
		- ps->mem [PROC_MEM_BUF]
		- ps->mem [PROC_MEM_SHARED];

	glibtop_get_swap (&swap);

	ps->swap [PROC_SWAP_TOTAL] = swap.total;
	ps->swap [PROC_SWAP_USED]  = swap.used;
	ps->swap [PROC_SWAP_FREE]  = swap.free;
}

void
proc_read_load (ProcInfo *ps)
{
	glibtop_loadavg loadavg;
	gint max_load, load;

	glibtop_get_loadavg (&loadavg);

	max_load = (gint) (gtop_properties.summary.maximum_loadavg * 10.0);
	load = (gint) (loadavg.loadavg [0] * 10.0);
	if (load > max_load) load = max_load;

	ps->load [PROC_LOAD_TOTAL]  = max_load;
	ps->load [PROC_LOAD_USED]   = load;
	ps->load [PROC_LOAD_FREE]   = max_load - load;
}
