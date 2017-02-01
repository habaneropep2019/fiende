/* guile.c */
/* This is a generated file.  Please modify `guile.pl' */

#include <glibtop.h>
#include <glibtop/xmalloc.h>
#include <glibtop/sysdeps.h>
#include <glibtop/union.h>

#include <guile/gh.h>

SCM_PROC (s_cpu, "glibtop-get-cpu", 0, 0, 0, glibtop_guile_get_cpu);

static SCM
glibtop_guile_get_cpu (void)
{
	glibtop_cpu cpu;
	SCM list;

	glibtop_get_cpu (&cpu);

	list = gh_list (gh_ulong2scm  (cpu.flags),
			gh_ulong2scm  (cpu.total),
			gh_ulong2scm  (cpu.user),
			gh_ulong2scm  (cpu.nice),
			gh_ulong2scm  (cpu.sys),
			gh_ulong2scm  (cpu.idle),
			gh_ulong2scm  (cpu.frequency),
			gh_list
			(gh_ulong2scm  (cpu.xcpu_total [0]),
			 gh_ulong2scm  (cpu.xcpu_total [1]),
			 gh_ulong2scm  (cpu.xcpu_total [2]),
			 gh_ulong2scm  (cpu.xcpu_total [3]),
			 SCM_UNDEFINED),
			gh_list
			(gh_ulong2scm  (cpu.xcpu_user [0]),
			 gh_ulong2scm  (cpu.xcpu_user [1]),
			 gh_ulong2scm  (cpu.xcpu_user [2]),
			 gh_ulong2scm  (cpu.xcpu_user [3]),
			 SCM_UNDEFINED),
			gh_list
			(gh_ulong2scm  (cpu.xcpu_nice [0]),
			 gh_ulong2scm  (cpu.xcpu_nice [1]),
			 gh_ulong2scm  (cpu.xcpu_nice [2]),
			 gh_ulong2scm  (cpu.xcpu_nice [3]),
			 SCM_UNDEFINED),
			gh_list
			(gh_ulong2scm  (cpu.xcpu_sys [0]),
			 gh_ulong2scm  (cpu.xcpu_sys [1]),
			 gh_ulong2scm  (cpu.xcpu_sys [2]),
			 gh_ulong2scm  (cpu.xcpu_sys [3]),
			 SCM_UNDEFINED),
			gh_list
			(gh_ulong2scm  (cpu.xcpu_idle [0]),
			 gh_ulong2scm  (cpu.xcpu_idle [1]),
			 gh_ulong2scm  (cpu.xcpu_idle [2]),
			 gh_ulong2scm  (cpu.xcpu_idle [3]),
			 SCM_UNDEFINED),
			SCM_UNDEFINED);

	return list;
}

SCM_PROC (s_mem, "glibtop-get-mem", 0, 0, 0, glibtop_guile_get_mem);

static SCM
glibtop_guile_get_mem (void)
{
	glibtop_mem mem;
	SCM list;

	glibtop_get_mem (&mem);

	list = gh_list (gh_ulong2scm  (mem.flags),
			gh_ulong2scm  (mem.total),
			gh_ulong2scm  (mem.used),
			gh_ulong2scm  (mem.free),
			gh_ulong2scm  (mem.shared),
			gh_ulong2scm  (mem.buffer),
			gh_ulong2scm  (mem.cached),
			gh_ulong2scm  (mem.user),
			gh_ulong2scm  (mem.locked),
			SCM_UNDEFINED);

	return list;
}

SCM_PROC (s_swap, "glibtop-get-swap", 0, 0, 0, glibtop_guile_get_swap);

static SCM
glibtop_guile_get_swap (void)
{
	glibtop_swap swap;
	SCM list;

	glibtop_get_swap (&swap);

	list = gh_list (gh_ulong2scm  (swap.flags),
			gh_ulong2scm  (swap.total),
			gh_ulong2scm  (swap.used),
			gh_ulong2scm  (swap.free),
			gh_ulong2scm  (swap.pagein),
			gh_ulong2scm  (swap.pageout),
			SCM_UNDEFINED);

	return list;
}

SCM_PROC (s_uptime, "glibtop-get-uptime", 0, 0, 0, glibtop_guile_get_uptime);

static SCM
glibtop_guile_get_uptime (void)
{
	glibtop_uptime uptime;
	SCM list;

	glibtop_get_uptime (&uptime);

	list = gh_list (gh_ulong2scm  (uptime.flags),
			gh_double2scm (uptime.uptime),
			gh_double2scm (uptime.idletime),
			SCM_UNDEFINED);

	return list;
}

SCM_PROC (s_loadavg, "glibtop-get-loadavg", 0, 0, 0, glibtop_guile_get_loadavg);

static SCM
glibtop_guile_get_loadavg (void)
{
	glibtop_loadavg loadavg;
	SCM list;

	glibtop_get_loadavg (&loadavg);

	list = gh_list (gh_ulong2scm  (loadavg.flags),
			gh_list
			(gh_double2scm (loadavg.loadavg [0]),
			 gh_double2scm (loadavg.loadavg [1]),
			 gh_double2scm (loadavg.loadavg [2]),
			 SCM_UNDEFINED),
			gh_ulong2scm  (loadavg.nr_running),
			gh_ulong2scm  (loadavg.nr_tasks),
			gh_ulong2scm  (loadavg.last_pid),
			SCM_UNDEFINED);

	return list;
}

SCM_PROC (s_shm_limits, "glibtop-get-shm-limits", 0, 0, 0, glibtop_guile_get_shm_limits);

static SCM
glibtop_guile_get_shm_limits (void)
{
	glibtop_shm_limits shm_limits;
	SCM list;

	glibtop_get_shm_limits (&shm_limits);

	list = gh_list (gh_ulong2scm  (shm_limits.flags),
			gh_ulong2scm  (shm_limits.shmmax),
			gh_ulong2scm  (shm_limits.shmmin),
			gh_ulong2scm  (shm_limits.shmmni),
			gh_ulong2scm  (shm_limits.shmseg),
			gh_ulong2scm  (shm_limits.shmall),
			SCM_UNDEFINED);

	return list;
}

SCM_PROC (s_msg_limits, "glibtop-get-msg-limits", 0, 0, 0, glibtop_guile_get_msg_limits);

static SCM
glibtop_guile_get_msg_limits (void)
{
	glibtop_msg_limits msg_limits;
	SCM list;

	glibtop_get_msg_limits (&msg_limits);

	list = gh_list (gh_ulong2scm  (msg_limits.flags),
			gh_ulong2scm  (msg_limits.msgpool),
			gh_ulong2scm  (msg_limits.msgmap),
			gh_ulong2scm  (msg_limits.msgmax),
			gh_ulong2scm  (msg_limits.msgmnb),
			gh_ulong2scm  (msg_limits.msgmni),
			gh_ulong2scm  (msg_limits.msgssz),
			gh_ulong2scm  (msg_limits.msgtql),
			SCM_UNDEFINED);

	return list;
}

SCM_PROC (s_sem_limits, "glibtop-get-sem-limits", 0, 0, 0, glibtop_guile_get_sem_limits);

static SCM
glibtop_guile_get_sem_limits (void)
{
	glibtop_sem_limits sem_limits;
	SCM list;

	glibtop_get_sem_limits (&sem_limits);

	list = gh_list (gh_ulong2scm  (sem_limits.flags),
			gh_ulong2scm  (sem_limits.semmap),
			gh_ulong2scm  (sem_limits.semmni),
			gh_ulong2scm  (sem_limits.semmns),
			gh_ulong2scm  (sem_limits.semmnu),
			gh_ulong2scm  (sem_limits.semmsl),
			gh_ulong2scm  (sem_limits.semopm),
			gh_ulong2scm  (sem_limits.semume),
			gh_ulong2scm  (sem_limits.semusz),
			gh_ulong2scm  (sem_limits.semvmx),
			gh_ulong2scm  (sem_limits.semaem),
			SCM_UNDEFINED);

	return list;
}

SCM_PROC (s_proclist, "glibtop-get-proclist", 2, 0, 0, glibtop_guile_get_proclist);

static SCM
glibtop_guile_get_proclist (SCM which, SCM arg)
{
	glibtop_proclist proclist;
	unsigned * retval;
	unsigned i;
	SCM list;

	retval = glibtop_get_proclist (&proclist, gh_scm2long (which), gh_scm2long (arg));

	list = gh_list (gh_ulong2scm  (proclist.flags),
			gh_ulong2scm  (proclist.number),
			gh_ulong2scm  (proclist.size),
			gh_ulong2scm  (proclist.total),
			SCM_UNDEFINED);

	if (retval == NULL)
		return list;

	for (i = 0; i < proclist.number; i++)
		list = scm_append
			(gh_list (list,
				  gh_list (gh_ulong2scm ((unsigned long) retval [i])),
				  SCM_UNDEFINED));

	glibtop_free (retval);

	return list;
}

SCM_PROC (s_proc_state, "glibtop-get-proc-state", 1, 0, 0, glibtop_guile_get_proc_state);

static SCM
glibtop_guile_get_proc_state (SCM pid)
{
	glibtop_proc_state proc_state;
	SCM list;

	glibtop_get_proc_state (&proc_state, gh_scm2ulong (pid));

	list = gh_list (gh_ulong2scm  (proc_state.flags),
			gh_str02scm   (proc_state.cmd),
			gh_char2scm   (proc_state.state),
			gh_ulong2scm  (proc_state.uid),
			gh_ulong2scm  (proc_state.gid),
			SCM_UNDEFINED);

	return list;
}

SCM_PROC (s_proc_uid, "glibtop-get-proc-uid", 1, 0, 0, glibtop_guile_get_proc_uid);

static SCM
glibtop_guile_get_proc_uid (SCM pid)
{
	glibtop_proc_uid proc_uid;
	SCM list;

	glibtop_get_proc_uid (&proc_uid, gh_scm2ulong (pid));

	list = gh_list (gh_ulong2scm  (proc_uid.flags),
			gh_long2scm   (proc_uid.uid),
			gh_long2scm   (proc_uid.euid),
			gh_long2scm   (proc_uid.gid),
			gh_long2scm   (proc_uid.egid),
			gh_long2scm   (proc_uid.pid),
			gh_long2scm   (proc_uid.ppid),
			gh_long2scm   (proc_uid.pgrp),
			gh_long2scm   (proc_uid.session),
			gh_long2scm   (proc_uid.tty),
			gh_long2scm   (proc_uid.tpgid),
			gh_long2scm   (proc_uid.priority),
			gh_long2scm   (proc_uid.nice),
			SCM_UNDEFINED);

	return list;
}

SCM_PROC (s_proc_mem, "glibtop-get-proc-mem", 1, 0, 0, glibtop_guile_get_proc_mem);

static SCM
glibtop_guile_get_proc_mem (SCM pid)
{
	glibtop_proc_mem proc_mem;
	SCM list;

	glibtop_get_proc_mem (&proc_mem, gh_scm2ulong (pid));

	list = gh_list (gh_ulong2scm  (proc_mem.flags),
			gh_long2scm   (proc_mem.size),
			gh_long2scm   (proc_mem.vsize),
			gh_long2scm   (proc_mem.resident),
			gh_long2scm   (proc_mem.share),
			gh_long2scm   (proc_mem.rss),
			gh_long2scm   (proc_mem.rss_rlim),
			SCM_UNDEFINED);

	return list;
}

SCM_PROC (s_proc_time, "glibtop-get-proc-time", 1, 0, 0, glibtop_guile_get_proc_time);

static SCM
glibtop_guile_get_proc_time (SCM pid)
{
	glibtop_proc_time proc_time;
	SCM list;

	glibtop_get_proc_time (&proc_time, gh_scm2ulong (pid));

	list = gh_list (gh_ulong2scm  (proc_time.flags),
			gh_long2scm   (proc_time.start_time),
			gh_long2scm   (proc_time.rtime),
			gh_long2scm   (proc_time.utime),
			gh_long2scm   (proc_time.stime),
			gh_long2scm   (proc_time.cutime),
			gh_long2scm   (proc_time.cstime),
			gh_long2scm   (proc_time.timeout),
			gh_long2scm   (proc_time.it_real_value),
			gh_long2scm   (proc_time.frequency),
			gh_list
			(gh_long2scm   (proc_time.xcpu_utime [0]),
			 gh_long2scm   (proc_time.xcpu_utime [1]),
			 gh_long2scm   (proc_time.xcpu_utime [2]),
			 gh_long2scm   (proc_time.xcpu_utime [3]),
			 SCM_UNDEFINED),
			gh_list
			(gh_long2scm   (proc_time.xcpu_stime [0]),
			 gh_long2scm   (proc_time.xcpu_stime [1]),
			 gh_long2scm   (proc_time.xcpu_stime [2]),
			 gh_long2scm   (proc_time.xcpu_stime [3]),
			 SCM_UNDEFINED),
			SCM_UNDEFINED);

	return list;
}

SCM_PROC (s_proc_signal, "glibtop-get-proc-signal", 1, 0, 0, glibtop_guile_get_proc_signal);

static SCM
glibtop_guile_get_proc_signal (SCM pid)
{
	glibtop_proc_signal proc_signal;
	SCM list;

	glibtop_get_proc_signal (&proc_signal, gh_scm2ulong (pid));

	list = gh_list (gh_ulong2scm  (proc_signal.flags),
			gh_list
			(gh_ulong2scm  (proc_signal.signal [0]),
			 gh_ulong2scm  (proc_signal.signal [1]),
			 SCM_UNDEFINED),
			gh_list
			(gh_ulong2scm  (proc_signal.blocked [0]),
			 gh_ulong2scm  (proc_signal.blocked [1]),
			 SCM_UNDEFINED),
			gh_list
			(gh_ulong2scm  (proc_signal.sigignore [0]),
			 gh_ulong2scm  (proc_signal.sigignore [1]),
			 SCM_UNDEFINED),
			gh_list
			(gh_ulong2scm  (proc_signal.sigcatch [0]),
			 gh_ulong2scm  (proc_signal.sigcatch [1]),
			 SCM_UNDEFINED),
			SCM_UNDEFINED);

	return list;
}

SCM_PROC (s_proc_kernel, "glibtop-get-proc-kernel", 1, 0, 0, glibtop_guile_get_proc_kernel);

static SCM
glibtop_guile_get_proc_kernel (SCM pid)
{
	glibtop_proc_kernel proc_kernel;
	SCM list;

	glibtop_get_proc_kernel (&proc_kernel, gh_scm2ulong (pid));

	list = gh_list (gh_ulong2scm  (proc_kernel.flags),
			gh_ulong2scm  (proc_kernel.k_flags),
			gh_ulong2scm  (proc_kernel.min_flt),
			gh_ulong2scm  (proc_kernel.maj_flt),
			gh_ulong2scm  (proc_kernel.cmin_flt),
			gh_ulong2scm  (proc_kernel.cmaj_flt),
			gh_ulong2scm  (proc_kernel.kstk_esp),
			gh_ulong2scm  (proc_kernel.kstk_eip),
			gh_ulong2scm  (proc_kernel.nwchan),
			gh_str02scm   (proc_kernel.wchan),
			SCM_UNDEFINED);

	return list;
}

SCM_PROC (s_proc_segment, "glibtop-get-proc-segment", 1, 0, 0, glibtop_guile_get_proc_segment);

static SCM
glibtop_guile_get_proc_segment (SCM pid)
{
	glibtop_proc_segment proc_segment;
	SCM list;

	glibtop_get_proc_segment (&proc_segment, gh_scm2ulong (pid));

	list = gh_list (gh_ulong2scm  (proc_segment.flags),
			gh_ulong2scm  (proc_segment.text_rss),
			gh_ulong2scm  (proc_segment.shlib_rss),
			gh_ulong2scm  (proc_segment.data_rss),
			gh_ulong2scm  (proc_segment.stack_rss),
			gh_ulong2scm  (proc_segment.dirty_size),
			gh_ulong2scm  (proc_segment.start_code),
			gh_ulong2scm  (proc_segment.end_code),
			gh_ulong2scm  (proc_segment.start_stack),
			SCM_UNDEFINED);

	return list;
}

SCM_PROC (s_proc_args, "glibtop-get-proc-args", 2, 0, 0, glibtop_guile_get_proc_args);

static SCM
glibtop_guile_get_proc_args (SCM pid, SCM max_len)
{
	glibtop_proc_args proc_args;
	char * retval;
	unsigned i;
	SCM list, scm_args, args_list;
	char *start;

	retval = glibtop_get_proc_args (&proc_args, gh_scm2ulong (pid), gh_scm2ulong (max_len));

	list = gh_list (gh_ulong2scm  (proc_args.flags),
			gh_ulong2scm  (proc_args.size),
			SCM_UNDEFINED);

	if (retval == NULL)
		return list;

	start = retval;
	scm_args = gh_list (SCM_UNDEFINED);

	for (i = 0; i <= proc_args.size; i++) {
		SCM arg_list;

		if (retval [i]) continue;

		arg_list = gh_list (gh_str02scm (start), SCM_UNDEFINED);
		scm_args = scm_append
			(gh_list (scm_args, arg_list, SCM_UNDEFINED));
;
		start = &(retval [i+1]);
	};

	args_list = gh_list (scm_args, SCM_UNDEFINED);
	list = scm_append (gh_list (list, args_list, SCM_UNDEFINED));

	glibtop_free (retval);

	return list;
}

SCM_PROC (s_proc_map, "glibtop-get-proc-map", 1, 0, 0, glibtop_guile_get_proc_map);

static SCM
glibtop_guile_get_proc_map (SCM pid)
{
	glibtop_proc_map proc_map;
	glibtop_map_entry * retval;
	unsigned i;
	SCM list;

	retval = glibtop_get_proc_map (&proc_map, gh_scm2ulong (pid));

	list = gh_list (gh_ulong2scm  (proc_map.flags),
			gh_ulong2scm  (proc_map.number),
			gh_ulong2scm  (proc_map.size),
			gh_ulong2scm  (proc_map.total),
			SCM_UNDEFINED);

	if (retval == NULL)
		return list;

	for (i = 0; i < proc_map.number; i++) {
		glibtop_map_entry *entry = &(retval [i]);
		SCM scm_entry = gh_list
			(gh_ulong2scm ((unsigned long) entry->flags),
			 gh_ulong2scm ((unsigned long) entry->start),
			 gh_ulong2scm ((unsigned long) entry->end),
			 gh_ulong2scm ((unsigned long) entry->offset),
			 gh_ulong2scm ((unsigned long) entry->perm),
			 gh_ulong2scm ((unsigned long) entry->inode),
			 gh_ulong2scm ((unsigned long) entry->device),
			 gh_str02scm (entry->filename), SCM_UNDEFINED);
		SCM entry_list = gh_list (scm_entry, SCM_UNDEFINED);

		list = scm_append (gh_list (list, entry_list, SCM_UNDEFINED));
	};

	glibtop_free (retval);

	return list;
}

SCM_PROC (s_mountlist, "glibtop-get-mountlist", 1, 0, 0, glibtop_guile_get_mountlist);

static SCM
glibtop_guile_get_mountlist (SCM all_fs)
{
	glibtop_mountlist mountlist;
	glibtop_mountentry * retval;
	unsigned i;
	SCM list;

	retval = glibtop_get_mountlist (&mountlist, gh_scm2long (all_fs));

	list = gh_list (gh_ulong2scm  (mountlist.flags),
			gh_ulong2scm  (mountlist.number),
			gh_ulong2scm  (mountlist.size),
			gh_ulong2scm  (mountlist.total),
			SCM_UNDEFINED);

	if (retval == NULL)
		return list;

	for (i = 0; i < mountlist.number; i++) {
		glibtop_mountentry *entry = &(retval [i]);
		SCM scm_entry = gh_list
			(gh_ulong2scm ((unsigned long) entry->dev),
			 gh_str02scm (entry->devname),
			 gh_str02scm (entry->mountdir),
			 gh_str02scm (entry->type), SCM_UNDEFINED);
		SCM entry_list = gh_list (scm_entry, SCM_UNDEFINED);

		list = scm_append (gh_list (list, entry_list, SCM_UNDEFINED));
	};

	glibtop_free (retval);

	return list;
}

SCM_PROC (s_fsusage, "glibtop-get-fsusage", 1, 0, 0, glibtop_guile_get_fsusage);

static SCM
glibtop_guile_get_fsusage (SCM mount_dir)
{
	glibtop_fsusage fsusage;
	SCM list;

	glibtop_get_fsusage (&fsusage, gh_scm2newstr( mount_dir, NULL));

	list = gh_list (gh_ulong2scm  (fsusage.flags),
			gh_ulong2scm  (fsusage.blocks),
			gh_ulong2scm  (fsusage.bfree),
			gh_ulong2scm  (fsusage.bavail),
			gh_ulong2scm  (fsusage.files),
			gh_ulong2scm  (fsusage.ffree),
			SCM_UNDEFINED);

	return list;
}

SCM_PROC (s_netload, "glibtop-get-netload", 1, 0, 0, glibtop_guile_get_netload);

static SCM
glibtop_guile_get_netload (SCM interface)
{
	glibtop_netload netload;
	SCM list;

	glibtop_get_netload (&netload, gh_scm2newstr( interface, NULL));

	list = gh_list (gh_ulong2scm  (netload.flags),
			gh_ulong2scm  (netload.if_flags),
			gh_ulong2scm  (netload.mtu),
			gh_ulong2scm  (netload.subnet),
			gh_ulong2scm  (netload.address),
			gh_ulong2scm  (netload.packets_in),
			gh_ulong2scm  (netload.packets_out),
			gh_ulong2scm  (netload.packets_total),
			gh_ulong2scm  (netload.bytes_in),
			gh_ulong2scm  (netload.bytes_out),
			gh_ulong2scm  (netload.bytes_total),
			gh_ulong2scm  (netload.errors_in),
			gh_ulong2scm  (netload.errors_out),
			gh_ulong2scm  (netload.errors_total),
			gh_ulong2scm  (netload.collisions),
			SCM_UNDEFINED);

	return list;
}

SCM_PROC (s_ppp, "glibtop-get-ppp", 1, 0, 0, glibtop_guile_get_ppp);

static SCM
glibtop_guile_get_ppp (SCM device)
{
	glibtop_ppp ppp;
	SCM list;

	glibtop_get_ppp (&ppp,  (device));

	list = gh_list (gh_ulong2scm  (ppp.flags),
			gh_ulong2scm  (ppp.state),
			gh_ulong2scm  (ppp.bytes_in),
			gh_ulong2scm  (ppp.bytes_out),
			SCM_UNDEFINED);

	return list;
}

SCM_PROC (s_sysdeps, "glibtop-get-sysdeps", 0, 0, 0, glibtop_guile_get_sysdeps);

static SCM
glibtop_guile_get_sysdeps (void)
{
	glibtop_sysdeps sysdeps;
	SCM list;

	glibtop_get_sysdeps (&sysdeps);

	list = gh_list (gh_ulong2scm  (sysdeps.flags),
			gh_ulong2scm  (sysdeps.cpu),
			gh_ulong2scm  (sysdeps.mem),
			gh_ulong2scm  (sysdeps.swap),
			gh_ulong2scm  (sysdeps.uptime),
			gh_ulong2scm  (sysdeps.loadavg),
			gh_ulong2scm  (sysdeps.shm_limits),
			gh_ulong2scm  (sysdeps.msg_limits),
			gh_ulong2scm  (sysdeps.sem_limits),
			gh_ulong2scm  (sysdeps.proclist),
			gh_ulong2scm  (sysdeps.proc_state),
			gh_ulong2scm  (sysdeps.proc_uid),
			gh_ulong2scm  (sysdeps.proc_mem),
			gh_ulong2scm  (sysdeps.proc_time),
			gh_ulong2scm  (sysdeps.proc_signal),
			gh_ulong2scm  (sysdeps.proc_kernel),
			gh_ulong2scm  (sysdeps.proc_segment),
			gh_ulong2scm  (sysdeps.proc_args),
			gh_ulong2scm  (sysdeps.proc_map),
			gh_ulong2scm  (sysdeps.mountlist),
			gh_ulong2scm  (sysdeps.fsusage),
			gh_ulong2scm  (sysdeps.netload),
			gh_ulong2scm  (sysdeps.ppp),
			SCM_UNDEFINED);

	return list;
}

void
glibtop_boot_guile (void)
{
#include "guile.x"
}
