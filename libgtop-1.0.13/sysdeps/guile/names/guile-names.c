/* guile_names.c */
/* This is a generated file.  Please modify `guile-names.pl' */

#include <glibtop.h>
#include <glibtop/sysdeps.h>
#include <glibtop/union.h>

#include <guile/gh.h>

static SCM
glibtop_guile_names_proc_kernel (void)
{
	int i;
	SCM list;

	list = gh_list (SCM_UNDEFINED);

	for (i = 0; i < GLIBTOP_MAX_PROC_KERNEL; i++)
		list = scm_append
			(gh_list (list,
				  gh_list (gh_str02scm ((char *) glibtop_names_proc_kernel [i])),
				  SCM_UNDEFINED));

	return list;
}

static SCM
glibtop_guile_types_proc_kernel (void)
{
	SCM list;

	list = gh_list (gh_ulong2scm (glibtop_types_proc_kernel [0]),
			gh_ulong2scm (glibtop_types_proc_kernel [1]),
			gh_ulong2scm (glibtop_types_proc_kernel [2]),
			gh_ulong2scm (glibtop_types_proc_kernel [3]),
			gh_ulong2scm (glibtop_types_proc_kernel [4]),
			gh_ulong2scm (glibtop_types_proc_kernel [5]),
			gh_ulong2scm (glibtop_types_proc_kernel [6]),
			gh_ulong2scm (glibtop_types_proc_kernel [7]),
			gh_ulong2scm (glibtop_types_proc_kernel [0]),
			SCM_UNDEFINED);

	return list;
}

static SCM
glibtop_guile_labels_proc_kernel (void)
{
	int i;
	SCM list;

	list = gh_list (SCM_UNDEFINED);

	for (i = 0; i < GLIBTOP_MAX_PROC_KERNEL; i++)
		list = scm_append
			(gh_list (list,
				  gh_list (gh_str02scm (_(glibtop_labels_proc_kernel [i]))),
				  SCM_UNDEFINED));

	return list;
}

static SCM
glibtop_guile_descriptions_proc_kernel (void)
{
	int i;
	SCM list;

	list = gh_list (SCM_UNDEFINED);

	for (i = 0; i < GLIBTOP_MAX_PROC_KERNEL; i++)
		list = scm_append
			(gh_list (list,
				  gh_list (gh_str02scm (_(glibtop_descriptions_proc_kernel [i]))),
				  SCM_UNDEFINED));

	return list;
}

static SCM
glibtop_guile_names_sysdeps (void)
{
	int i;
	SCM list;

	list = gh_list (SCM_UNDEFINED);

	for (i = 0; i < GLIBTOP_MAX_SYSDEPS; i++)
		list = scm_append
			(gh_list (list,
				  gh_list (gh_str02scm ((char *) glibtop_names_sysdeps [i])),
				  SCM_UNDEFINED));

	return list;
}

static SCM
glibtop_guile_types_sysdeps (void)
{
	SCM list;

	list = gh_list (SCM_UNDEFINED);

	return list;
}

static SCM
glibtop_guile_labels_sysdeps (void)
{
	int i;
	SCM list;

	list = gh_list (SCM_UNDEFINED);

	for (i = 0; i < GLIBTOP_MAX_SYSDEPS; i++)
		list = scm_append
			(gh_list (list,
				  gh_list (gh_str02scm (_(glibtop_labels_sysdeps [i]))),
				  SCM_UNDEFINED));

	return list;
}

static SCM
glibtop_guile_descriptions_sysdeps (void)
{
	int i;
	SCM list;

	list = gh_list (SCM_UNDEFINED);

	for (i = 0; i < GLIBTOP_MAX_SYSDEPS; i++)
		list = scm_append
			(gh_list (list,
				  gh_list (gh_str02scm (_(glibtop_descriptions_sysdeps [i]))),
				  SCM_UNDEFINED));

	return list;
}

static SCM
glibtop_guile_names_proclist (void)
{
	int i;
	SCM list;

	list = gh_list (SCM_UNDEFINED);

	for (i = 0; i < GLIBTOP_MAX_PROCLIST; i++)
		list = scm_append
			(gh_list (list,
				  gh_list (gh_str02scm ((char *) glibtop_names_proclist [i])),
				  SCM_UNDEFINED));

	return list;
}

static SCM
glibtop_guile_types_proclist (void)
{
	SCM list;

	list = gh_list (gh_ulong2scm (glibtop_types_proclist [0]),
			gh_ulong2scm (glibtop_types_proclist [1]),
			gh_ulong2scm (glibtop_types_proclist [2]),
			SCM_UNDEFINED);

	return list;
}

static SCM
glibtop_guile_labels_proclist (void)
{
	int i;
	SCM list;

	list = gh_list (SCM_UNDEFINED);

	for (i = 0; i < GLIBTOP_MAX_PROCLIST; i++)
		list = scm_append
			(gh_list (list,
				  gh_list (gh_str02scm (_(glibtop_labels_proclist [i]))),
				  SCM_UNDEFINED));

	return list;
}

static SCM
glibtop_guile_descriptions_proclist (void)
{
	int i;
	SCM list;

	list = gh_list (SCM_UNDEFINED);

	for (i = 0; i < GLIBTOP_MAX_PROCLIST; i++)
		list = scm_append
			(gh_list (list,
				  gh_list (gh_str02scm (_(glibtop_descriptions_proclist [i]))),
				  SCM_UNDEFINED));

	return list;
}

static SCM
glibtop_guile_names_shm_limits (void)
{
	int i;
	SCM list;

	list = gh_list (SCM_UNDEFINED);

	for (i = 0; i < GLIBTOP_MAX_SHM_LIMITS; i++)
		list = scm_append
			(gh_list (list,
				  gh_list (gh_str02scm ((char *) glibtop_names_shm_limits [i])),
				  SCM_UNDEFINED));

	return list;
}

static SCM
glibtop_guile_types_shm_limits (void)
{
	SCM list;

	list = gh_list (gh_ulong2scm (glibtop_types_shm_limits [0]),
			gh_ulong2scm (glibtop_types_shm_limits [1]),
			gh_ulong2scm (glibtop_types_shm_limits [2]),
			gh_ulong2scm (glibtop_types_shm_limits [3]),
			gh_ulong2scm (glibtop_types_shm_limits [4]),
			SCM_UNDEFINED);

	return list;
}

static SCM
glibtop_guile_labels_shm_limits (void)
{
	int i;
	SCM list;

	list = gh_list (SCM_UNDEFINED);

	for (i = 0; i < GLIBTOP_MAX_SHM_LIMITS; i++)
		list = scm_append
			(gh_list (list,
				  gh_list (gh_str02scm (_(glibtop_labels_shm_limits [i]))),
				  SCM_UNDEFINED));

	return list;
}

static SCM
glibtop_guile_descriptions_shm_limits (void)
{
	int i;
	SCM list;

	list = gh_list (SCM_UNDEFINED);

	for (i = 0; i < GLIBTOP_MAX_SHM_LIMITS; i++)
		list = scm_append
			(gh_list (list,
				  gh_list (gh_str02scm (_(glibtop_descriptions_shm_limits [i]))),
				  SCM_UNDEFINED));

	return list;
}

static SCM
glibtop_guile_names_mem (void)
{
	int i;
	SCM list;

	list = gh_list (SCM_UNDEFINED);

	for (i = 0; i < GLIBTOP_MAX_MEM; i++)
		list = scm_append
			(gh_list (list,
				  gh_list (gh_str02scm ((char *) glibtop_names_mem [i])),
				  SCM_UNDEFINED));

	return list;
}

static SCM
glibtop_guile_types_mem (void)
{
	SCM list;

	list = gh_list (gh_ulong2scm (glibtop_types_mem [0]),
			gh_ulong2scm (glibtop_types_mem [1]),
			gh_ulong2scm (glibtop_types_mem [2]),
			gh_ulong2scm (glibtop_types_mem [3]),
			gh_ulong2scm (glibtop_types_mem [4]),
			gh_ulong2scm (glibtop_types_mem [5]),
			gh_ulong2scm (glibtop_types_mem [6]),
			gh_ulong2scm (glibtop_types_mem [7]),
			SCM_UNDEFINED);

	return list;
}

static SCM
glibtop_guile_labels_mem (void)
{
	int i;
	SCM list;

	list = gh_list (SCM_UNDEFINED);

	for (i = 0; i < GLIBTOP_MAX_MEM; i++)
		list = scm_append
			(gh_list (list,
				  gh_list (gh_str02scm (_(glibtop_labels_mem [i]))),
				  SCM_UNDEFINED));

	return list;
}

static SCM
glibtop_guile_descriptions_mem (void)
{
	int i;
	SCM list;

	list = gh_list (SCM_UNDEFINED);

	for (i = 0; i < GLIBTOP_MAX_MEM; i++)
		list = scm_append
			(gh_list (list,
				  gh_list (gh_str02scm (_(glibtop_descriptions_mem [i]))),
				  SCM_UNDEFINED));

	return list;
}

static SCM
glibtop_guile_names_cpu (void)
{
	int i;
	SCM list;

	list = gh_list (SCM_UNDEFINED);

	for (i = 0; i < GLIBTOP_MAX_CPU; i++)
		list = scm_append
			(gh_list (list,
				  gh_list (gh_str02scm ((char *) glibtop_names_cpu [i])),
				  SCM_UNDEFINED));

	return list;
}

static SCM
glibtop_guile_types_cpu (void)
{
	SCM list;

	list = gh_list (gh_ulong2scm (glibtop_types_cpu [0]),
			gh_ulong2scm (glibtop_types_cpu [1]),
			gh_ulong2scm (glibtop_types_cpu [2]),
			gh_ulong2scm (glibtop_types_cpu [3]),
			gh_ulong2scm (glibtop_types_cpu [4]),
			gh_ulong2scm (glibtop_types_cpu [5]),
			gh_cons
			(gh_ulong2scm (glibtop_types_cpu [6]),
			 gh_ulong2scm (4)),
			gh_cons
			(gh_ulong2scm (glibtop_types_cpu [7]),
			 gh_ulong2scm (4)),
			gh_cons
			(gh_ulong2scm (glibtop_types_cpu [8]),
			 gh_ulong2scm (4)),
			gh_cons
			(gh_ulong2scm (glibtop_types_cpu [9]),
			 gh_ulong2scm (4)),
			gh_cons
			(gh_ulong2scm (glibtop_types_cpu [10]),
			 gh_ulong2scm (4)),
			SCM_UNDEFINED);

	return list;
}

static SCM
glibtop_guile_labels_cpu (void)
{
	int i;
	SCM list;

	list = gh_list (SCM_UNDEFINED);

	for (i = 0; i < GLIBTOP_MAX_CPU; i++)
		list = scm_append
			(gh_list (list,
				  gh_list (gh_str02scm (_(glibtop_labels_cpu [i]))),
				  SCM_UNDEFINED));

	return list;
}

static SCM
glibtop_guile_descriptions_cpu (void)
{
	int i;
	SCM list;

	list = gh_list (SCM_UNDEFINED);

	for (i = 0; i < GLIBTOP_MAX_CPU; i++)
		list = scm_append
			(gh_list (list,
				  gh_list (gh_str02scm (_(glibtop_descriptions_cpu [i]))),
				  SCM_UNDEFINED));

	return list;
}

static SCM
glibtop_guile_names_proc_state (void)
{
	int i;
	SCM list;

	list = gh_list (SCM_UNDEFINED);

	for (i = 0; i < GLIBTOP_MAX_PROC_STATE; i++)
		list = scm_append
			(gh_list (list,
				  gh_list (gh_str02scm ((char *) glibtop_names_proc_state [i])),
				  SCM_UNDEFINED));

	return list;
}

static SCM
glibtop_guile_types_proc_state (void)
{
	SCM list;

	list = gh_list (gh_ulong2scm (glibtop_types_proc_state [0]),
			gh_ulong2scm (glibtop_types_proc_state [0]),
			gh_ulong2scm (glibtop_types_proc_state [0]),
			gh_ulong2scm (glibtop_types_proc_state [1]),
			SCM_UNDEFINED);

	return list;
}

static SCM
glibtop_guile_labels_proc_state (void)
{
	int i;
	SCM list;

	list = gh_list (SCM_UNDEFINED);

	for (i = 0; i < GLIBTOP_MAX_PROC_STATE; i++)
		list = scm_append
			(gh_list (list,
				  gh_list (gh_str02scm (_(glibtop_labels_proc_state [i]))),
				  SCM_UNDEFINED));

	return list;
}

static SCM
glibtop_guile_descriptions_proc_state (void)
{
	int i;
	SCM list;

	list = gh_list (SCM_UNDEFINED);

	for (i = 0; i < GLIBTOP_MAX_PROC_STATE; i++)
		list = scm_append
			(gh_list (list,
				  gh_list (gh_str02scm (_(glibtop_descriptions_proc_state [i]))),
				  SCM_UNDEFINED));

	return list;
}

static SCM
glibtop_guile_names_ppp (void)
{
	int i;
	SCM list;

	list = gh_list (SCM_UNDEFINED);

	for (i = 0; i < GLIBTOP_MAX_PPP; i++)
		list = scm_append
			(gh_list (list,
				  gh_list (gh_str02scm ((char *) glibtop_names_ppp [i])),
				  SCM_UNDEFINED));

	return list;
}

static SCM
glibtop_guile_types_ppp (void)
{
	SCM list;

	list = gh_list (gh_ulong2scm (glibtop_types_ppp [0]),
			gh_ulong2scm (glibtop_types_ppp [1]),
			gh_ulong2scm (glibtop_types_ppp [2]),
			SCM_UNDEFINED);

	return list;
}

static SCM
glibtop_guile_labels_ppp (void)
{
	int i;
	SCM list;

	list = gh_list (SCM_UNDEFINED);

	for (i = 0; i < GLIBTOP_MAX_PPP; i++)
		list = scm_append
			(gh_list (list,
				  gh_list (gh_str02scm (_(glibtop_labels_ppp [i]))),
				  SCM_UNDEFINED));

	return list;
}

static SCM
glibtop_guile_descriptions_ppp (void)
{
	int i;
	SCM list;

	list = gh_list (SCM_UNDEFINED);

	for (i = 0; i < GLIBTOP_MAX_PPP; i++)
		list = scm_append
			(gh_list (list,
				  gh_list (gh_str02scm (_(glibtop_descriptions_ppp [i]))),
				  SCM_UNDEFINED));

	return list;
}

static SCM
glibtop_guile_names_uptime (void)
{
	int i;
	SCM list;

	list = gh_list (SCM_UNDEFINED);

	for (i = 0; i < GLIBTOP_MAX_UPTIME; i++)
		list = scm_append
			(gh_list (list,
				  gh_list (gh_str02scm ((char *) glibtop_names_uptime [i])),
				  SCM_UNDEFINED));

	return list;
}

static SCM
glibtop_guile_types_uptime (void)
{
	SCM list;

	list = gh_list (gh_ulong2scm (glibtop_types_uptime [0]),
			gh_ulong2scm (glibtop_types_uptime [1]),
			SCM_UNDEFINED);

	return list;
}

static SCM
glibtop_guile_labels_uptime (void)
{
	int i;
	SCM list;

	list = gh_list (SCM_UNDEFINED);

	for (i = 0; i < GLIBTOP_MAX_UPTIME; i++)
		list = scm_append
			(gh_list (list,
				  gh_list (gh_str02scm (_(glibtop_labels_uptime [i]))),
				  SCM_UNDEFINED));

	return list;
}

static SCM
glibtop_guile_descriptions_uptime (void)
{
	int i;
	SCM list;

	list = gh_list (SCM_UNDEFINED);

	for (i = 0; i < GLIBTOP_MAX_UPTIME; i++)
		list = scm_append
			(gh_list (list,
				  gh_list (gh_str02scm (_(glibtop_descriptions_uptime [i]))),
				  SCM_UNDEFINED));

	return list;
}

static SCM
glibtop_guile_names_sem_limits (void)
{
	int i;
	SCM list;

	list = gh_list (SCM_UNDEFINED);

	for (i = 0; i < GLIBTOP_MAX_SEM_LIMITS; i++)
		list = scm_append
			(gh_list (list,
				  gh_list (gh_str02scm ((char *) glibtop_names_sem_limits [i])),
				  SCM_UNDEFINED));

	return list;
}

static SCM
glibtop_guile_types_sem_limits (void)
{
	SCM list;

	list = gh_list (gh_ulong2scm (glibtop_types_sem_limits [0]),
			gh_ulong2scm (glibtop_types_sem_limits [1]),
			gh_ulong2scm (glibtop_types_sem_limits [2]),
			gh_ulong2scm (glibtop_types_sem_limits [3]),
			gh_ulong2scm (glibtop_types_sem_limits [4]),
			gh_ulong2scm (glibtop_types_sem_limits [5]),
			gh_ulong2scm (glibtop_types_sem_limits [6]),
			gh_ulong2scm (glibtop_types_sem_limits [7]),
			gh_ulong2scm (glibtop_types_sem_limits [8]),
			gh_ulong2scm (glibtop_types_sem_limits [9]),
			SCM_UNDEFINED);

	return list;
}

static SCM
glibtop_guile_labels_sem_limits (void)
{
	int i;
	SCM list;

	list = gh_list (SCM_UNDEFINED);

	for (i = 0; i < GLIBTOP_MAX_SEM_LIMITS; i++)
		list = scm_append
			(gh_list (list,
				  gh_list (gh_str02scm (_(glibtop_labels_sem_limits [i]))),
				  SCM_UNDEFINED));

	return list;
}

static SCM
glibtop_guile_descriptions_sem_limits (void)
{
	int i;
	SCM list;

	list = gh_list (SCM_UNDEFINED);

	for (i = 0; i < GLIBTOP_MAX_SEM_LIMITS; i++)
		list = scm_append
			(gh_list (list,
				  gh_list (gh_str02scm (_(glibtop_descriptions_sem_limits [i]))),
				  SCM_UNDEFINED));

	return list;
}

static SCM
glibtop_guile_names_proc_args (void)
{
	int i;
	SCM list;

	list = gh_list (SCM_UNDEFINED);

	for (i = 0; i < GLIBTOP_MAX_PROC_ARGS; i++)
		list = scm_append
			(gh_list (list,
				  gh_list (gh_str02scm ((char *) glibtop_names_proc_args [i])),
				  SCM_UNDEFINED));

	return list;
}

static SCM
glibtop_guile_types_proc_args (void)
{
	SCM list;

	list = gh_list (gh_ulong2scm (glibtop_types_proc_args [0]),
			SCM_UNDEFINED);

	return list;
}

static SCM
glibtop_guile_labels_proc_args (void)
{
	int i;
	SCM list;

	list = gh_list (SCM_UNDEFINED);

	for (i = 0; i < GLIBTOP_MAX_PROC_ARGS; i++)
		list = scm_append
			(gh_list (list,
				  gh_list (gh_str02scm (_(glibtop_labels_proc_args [i]))),
				  SCM_UNDEFINED));

	return list;
}

static SCM
glibtop_guile_descriptions_proc_args (void)
{
	int i;
	SCM list;

	list = gh_list (SCM_UNDEFINED);

	for (i = 0; i < GLIBTOP_MAX_PROC_ARGS; i++)
		list = scm_append
			(gh_list (list,
				  gh_list (gh_str02scm (_(glibtop_descriptions_proc_args [i]))),
				  SCM_UNDEFINED));

	return list;
}

static SCM
glibtop_guile_names_loadavg (void)
{
	int i;
	SCM list;

	list = gh_list (SCM_UNDEFINED);

	for (i = 0; i < GLIBTOP_MAX_LOADAVG; i++)
		list = scm_append
			(gh_list (list,
				  gh_list (gh_str02scm ((char *) glibtop_names_loadavg [i])),
				  SCM_UNDEFINED));

	return list;
}

static SCM
glibtop_guile_types_loadavg (void)
{
	SCM list;

	list = gh_list (gh_cons
			(gh_ulong2scm (glibtop_types_loadavg [0]),
			 gh_ulong2scm (3)),
			gh_ulong2scm (glibtop_types_loadavg [0]),
			gh_ulong2scm (glibtop_types_loadavg [1]),
			gh_ulong2scm (glibtop_types_loadavg [2]),
			SCM_UNDEFINED);

	return list;
}

static SCM
glibtop_guile_labels_loadavg (void)
{
	int i;
	SCM list;

	list = gh_list (SCM_UNDEFINED);

	for (i = 0; i < GLIBTOP_MAX_LOADAVG; i++)
		list = scm_append
			(gh_list (list,
				  gh_list (gh_str02scm (_(glibtop_labels_loadavg [i]))),
				  SCM_UNDEFINED));

	return list;
}

static SCM
glibtop_guile_descriptions_loadavg (void)
{
	int i;
	SCM list;

	list = gh_list (SCM_UNDEFINED);

	for (i = 0; i < GLIBTOP_MAX_LOADAVG; i++)
		list = scm_append
			(gh_list (list,
				  gh_list (gh_str02scm (_(glibtop_descriptions_loadavg [i]))),
				  SCM_UNDEFINED));

	return list;
}

static SCM
glibtop_guile_names_swap (void)
{
	int i;
	SCM list;

	list = gh_list (SCM_UNDEFINED);

	for (i = 0; i < GLIBTOP_MAX_SWAP; i++)
		list = scm_append
			(gh_list (list,
				  gh_list (gh_str02scm ((char *) glibtop_names_swap [i])),
				  SCM_UNDEFINED));

	return list;
}

static SCM
glibtop_guile_types_swap (void)
{
	SCM list;

	list = gh_list (gh_ulong2scm (glibtop_types_swap [0]),
			gh_ulong2scm (glibtop_types_swap [1]),
			gh_ulong2scm (glibtop_types_swap [2]),
			gh_ulong2scm (glibtop_types_swap [3]),
			gh_ulong2scm (glibtop_types_swap [4]),
			SCM_UNDEFINED);

	return list;
}

static SCM
glibtop_guile_labels_swap (void)
{
	int i;
	SCM list;

	list = gh_list (SCM_UNDEFINED);

	for (i = 0; i < GLIBTOP_MAX_SWAP; i++)
		list = scm_append
			(gh_list (list,
				  gh_list (gh_str02scm (_(glibtop_labels_swap [i]))),
				  SCM_UNDEFINED));

	return list;
}

static SCM
glibtop_guile_descriptions_swap (void)
{
	int i;
	SCM list;

	list = gh_list (SCM_UNDEFINED);

	for (i = 0; i < GLIBTOP_MAX_SWAP; i++)
		list = scm_append
			(gh_list (list,
				  gh_list (gh_str02scm (_(glibtop_descriptions_swap [i]))),
				  SCM_UNDEFINED));

	return list;
}

static SCM
glibtop_guile_names_proc_segment (void)
{
	int i;
	SCM list;

	list = gh_list (SCM_UNDEFINED);

	for (i = 0; i < GLIBTOP_MAX_PROC_SEGMENT; i++)
		list = scm_append
			(gh_list (list,
				  gh_list (gh_str02scm ((char *) glibtop_names_proc_segment [i])),
				  SCM_UNDEFINED));

	return list;
}

static SCM
glibtop_guile_types_proc_segment (void)
{
	SCM list;

	list = gh_list (gh_ulong2scm (glibtop_types_proc_segment [0]),
			gh_ulong2scm (glibtop_types_proc_segment [1]),
			gh_ulong2scm (glibtop_types_proc_segment [2]),
			gh_ulong2scm (glibtop_types_proc_segment [3]),
			gh_ulong2scm (glibtop_types_proc_segment [4]),
			gh_ulong2scm (glibtop_types_proc_segment [5]),
			gh_ulong2scm (glibtop_types_proc_segment [6]),
			gh_ulong2scm (glibtop_types_proc_segment [7]),
			SCM_UNDEFINED);

	return list;
}

static SCM
glibtop_guile_labels_proc_segment (void)
{
	int i;
	SCM list;

	list = gh_list (SCM_UNDEFINED);

	for (i = 0; i < GLIBTOP_MAX_PROC_SEGMENT; i++)
		list = scm_append
			(gh_list (list,
				  gh_list (gh_str02scm (_(glibtop_labels_proc_segment [i]))),
				  SCM_UNDEFINED));

	return list;
}

static SCM
glibtop_guile_descriptions_proc_segment (void)
{
	int i;
	SCM list;

	list = gh_list (SCM_UNDEFINED);

	for (i = 0; i < GLIBTOP_MAX_PROC_SEGMENT; i++)
		list = scm_append
			(gh_list (list,
				  gh_list (gh_str02scm (_(glibtop_descriptions_proc_segment [i]))),
				  SCM_UNDEFINED));

	return list;
}

static SCM
glibtop_guile_names_proc_uid (void)
{
	int i;
	SCM list;

	list = gh_list (SCM_UNDEFINED);

	for (i = 0; i < GLIBTOP_MAX_PROC_UID; i++)
		list = scm_append
			(gh_list (list,
				  gh_list (gh_str02scm ((char *) glibtop_names_proc_uid [i])),
				  SCM_UNDEFINED));

	return list;
}

static SCM
glibtop_guile_types_proc_uid (void)
{
	SCM list;

	list = gh_list (gh_ulong2scm (glibtop_types_proc_uid [0]),
			gh_ulong2scm (glibtop_types_proc_uid [1]),
			gh_ulong2scm (glibtop_types_proc_uid [2]),
			gh_ulong2scm (glibtop_types_proc_uid [3]),
			gh_ulong2scm (glibtop_types_proc_uid [4]),
			gh_ulong2scm (glibtop_types_proc_uid [5]),
			gh_ulong2scm (glibtop_types_proc_uid [6]),
			gh_ulong2scm (glibtop_types_proc_uid [7]),
			gh_ulong2scm (glibtop_types_proc_uid [8]),
			gh_ulong2scm (glibtop_types_proc_uid [9]),
			gh_ulong2scm (glibtop_types_proc_uid [10]),
			gh_ulong2scm (glibtop_types_proc_uid [11]),
			SCM_UNDEFINED);

	return list;
}

static SCM
glibtop_guile_labels_proc_uid (void)
{
	int i;
	SCM list;

	list = gh_list (SCM_UNDEFINED);

	for (i = 0; i < GLIBTOP_MAX_PROC_UID; i++)
		list = scm_append
			(gh_list (list,
				  gh_list (gh_str02scm (_(glibtop_labels_proc_uid [i]))),
				  SCM_UNDEFINED));

	return list;
}

static SCM
glibtop_guile_descriptions_proc_uid (void)
{
	int i;
	SCM list;

	list = gh_list (SCM_UNDEFINED);

	for (i = 0; i < GLIBTOP_MAX_PROC_UID; i++)
		list = scm_append
			(gh_list (list,
				  gh_list (gh_str02scm (_(glibtop_descriptions_proc_uid [i]))),
				  SCM_UNDEFINED));

	return list;
}

static SCM
glibtop_guile_names_proc_signal (void)
{
	int i;
	SCM list;

	list = gh_list (SCM_UNDEFINED);

	for (i = 0; i < GLIBTOP_MAX_PROC_SIGNAL; i++)
		list = scm_append
			(gh_list (list,
				  gh_list (gh_str02scm ((char *) glibtop_names_proc_signal [i])),
				  SCM_UNDEFINED));

	return list;
}

static SCM
glibtop_guile_types_proc_signal (void)
{
	SCM list;

	list = gh_list (gh_cons
			(gh_ulong2scm (glibtop_types_proc_signal [0]),
			 gh_ulong2scm (2)),
			gh_cons
			(gh_ulong2scm (glibtop_types_proc_signal [1]),
			 gh_ulong2scm (2)),
			gh_cons
			(gh_ulong2scm (glibtop_types_proc_signal [2]),
			 gh_ulong2scm (2)),
			gh_cons
			(gh_ulong2scm (glibtop_types_proc_signal [3]),
			 gh_ulong2scm (2)),
			SCM_UNDEFINED);

	return list;
}

static SCM
glibtop_guile_labels_proc_signal (void)
{
	int i;
	SCM list;

	list = gh_list (SCM_UNDEFINED);

	for (i = 0; i < GLIBTOP_MAX_PROC_SIGNAL; i++)
		list = scm_append
			(gh_list (list,
				  gh_list (gh_str02scm (_(glibtop_labels_proc_signal [i]))),
				  SCM_UNDEFINED));

	return list;
}

static SCM
glibtop_guile_descriptions_proc_signal (void)
{
	int i;
	SCM list;

	list = gh_list (SCM_UNDEFINED);

	for (i = 0; i < GLIBTOP_MAX_PROC_SIGNAL; i++)
		list = scm_append
			(gh_list (list,
				  gh_list (gh_str02scm (_(glibtop_descriptions_proc_signal [i]))),
				  SCM_UNDEFINED));

	return list;
}

static SCM
glibtop_guile_names_proc_map (void)
{
	int i;
	SCM list;

	list = gh_list (SCM_UNDEFINED);

	for (i = 0; i < GLIBTOP_MAX_PROC_MAP; i++)
		list = scm_append
			(gh_list (list,
				  gh_list (gh_str02scm ((char *) glibtop_names_proc_map [i])),
				  SCM_UNDEFINED));

	return list;
}

static SCM
glibtop_guile_types_proc_map (void)
{
	SCM list;

	list = gh_list (gh_ulong2scm (glibtop_types_proc_map [0]),
			gh_ulong2scm (glibtop_types_proc_map [1]),
			gh_ulong2scm (glibtop_types_proc_map [2]),
			SCM_UNDEFINED);

	return list;
}

static SCM
glibtop_guile_labels_proc_map (void)
{
	int i;
	SCM list;

	list = gh_list (SCM_UNDEFINED);

	for (i = 0; i < GLIBTOP_MAX_PROC_MAP; i++)
		list = scm_append
			(gh_list (list,
				  gh_list (gh_str02scm (_(glibtop_labels_proc_map [i]))),
				  SCM_UNDEFINED));

	return list;
}

static SCM
glibtop_guile_descriptions_proc_map (void)
{
	int i;
	SCM list;

	list = gh_list (SCM_UNDEFINED);

	for (i = 0; i < GLIBTOP_MAX_PROC_MAP; i++)
		list = scm_append
			(gh_list (list,
				  gh_list (gh_str02scm (_(glibtop_descriptions_proc_map [i]))),
				  SCM_UNDEFINED));

	return list;
}

static SCM
glibtop_guile_names_proc_mem (void)
{
	int i;
	SCM list;

	list = gh_list (SCM_UNDEFINED);

	for (i = 0; i < GLIBTOP_MAX_PROC_MEM; i++)
		list = scm_append
			(gh_list (list,
				  gh_list (gh_str02scm ((char *) glibtop_names_proc_mem [i])),
				  SCM_UNDEFINED));

	return list;
}

static SCM
glibtop_guile_types_proc_mem (void)
{
	SCM list;

	list = gh_list (gh_ulong2scm (glibtop_types_proc_mem [0]),
			gh_ulong2scm (glibtop_types_proc_mem [1]),
			gh_ulong2scm (glibtop_types_proc_mem [2]),
			gh_ulong2scm (glibtop_types_proc_mem [3]),
			gh_ulong2scm (glibtop_types_proc_mem [4]),
			gh_ulong2scm (glibtop_types_proc_mem [5]),
			SCM_UNDEFINED);

	return list;
}

static SCM
glibtop_guile_labels_proc_mem (void)
{
	int i;
	SCM list;

	list = gh_list (SCM_UNDEFINED);

	for (i = 0; i < GLIBTOP_MAX_PROC_MEM; i++)
		list = scm_append
			(gh_list (list,
				  gh_list (gh_str02scm (_(glibtop_labels_proc_mem [i]))),
				  SCM_UNDEFINED));

	return list;
}

static SCM
glibtop_guile_descriptions_proc_mem (void)
{
	int i;
	SCM list;

	list = gh_list (SCM_UNDEFINED);

	for (i = 0; i < GLIBTOP_MAX_PROC_MEM; i++)
		list = scm_append
			(gh_list (list,
				  gh_list (gh_str02scm (_(glibtop_descriptions_proc_mem [i]))),
				  SCM_UNDEFINED));

	return list;
}

static SCM
glibtop_guile_names_mountlist (void)
{
	int i;
	SCM list;

	list = gh_list (SCM_UNDEFINED);

	for (i = 0; i < GLIBTOP_MAX_MOUNTLIST; i++)
		list = scm_append
			(gh_list (list,
				  gh_list (gh_str02scm ((char *) glibtop_names_mountlist [i])),
				  SCM_UNDEFINED));

	return list;
}

static SCM
glibtop_guile_types_mountlist (void)
{
	SCM list;

	list = gh_list (gh_ulong2scm (glibtop_types_mountlist [0]),
			gh_ulong2scm (glibtop_types_mountlist [1]),
			gh_ulong2scm (glibtop_types_mountlist [2]),
			SCM_UNDEFINED);

	return list;
}

static SCM
glibtop_guile_labels_mountlist (void)
{
	int i;
	SCM list;

	list = gh_list (SCM_UNDEFINED);

	for (i = 0; i < GLIBTOP_MAX_MOUNTLIST; i++)
		list = scm_append
			(gh_list (list,
				  gh_list (gh_str02scm (_(glibtop_labels_mountlist [i]))),
				  SCM_UNDEFINED));

	return list;
}

static SCM
glibtop_guile_descriptions_mountlist (void)
{
	int i;
	SCM list;

	list = gh_list (SCM_UNDEFINED);

	for (i = 0; i < GLIBTOP_MAX_MOUNTLIST; i++)
		list = scm_append
			(gh_list (list,
				  gh_list (gh_str02scm (_(glibtop_descriptions_mountlist [i]))),
				  SCM_UNDEFINED));

	return list;
}

static SCM
glibtop_guile_names_netload (void)
{
	int i;
	SCM list;

	list = gh_list (SCM_UNDEFINED);

	for (i = 0; i < GLIBTOP_MAX_NETLOAD; i++)
		list = scm_append
			(gh_list (list,
				  gh_list (gh_str02scm ((char *) glibtop_names_netload [i])),
				  SCM_UNDEFINED));

	return list;
}

static SCM
glibtop_guile_types_netload (void)
{
	SCM list;

	list = gh_list (gh_ulong2scm (glibtop_types_netload [0]),
			gh_ulong2scm (glibtop_types_netload [1]),
			gh_ulong2scm (glibtop_types_netload [2]),
			gh_ulong2scm (glibtop_types_netload [3]),
			gh_ulong2scm (glibtop_types_netload [4]),
			gh_ulong2scm (glibtop_types_netload [5]),
			gh_ulong2scm (glibtop_types_netload [6]),
			gh_ulong2scm (glibtop_types_netload [7]),
			gh_ulong2scm (glibtop_types_netload [8]),
			gh_ulong2scm (glibtop_types_netload [9]),
			gh_ulong2scm (glibtop_types_netload [10]),
			gh_ulong2scm (glibtop_types_netload [11]),
			gh_ulong2scm (glibtop_types_netload [12]),
			gh_ulong2scm (glibtop_types_netload [13]),
			SCM_UNDEFINED);

	return list;
}

static SCM
glibtop_guile_labels_netload (void)
{
	int i;
	SCM list;

	list = gh_list (SCM_UNDEFINED);

	for (i = 0; i < GLIBTOP_MAX_NETLOAD; i++)
		list = scm_append
			(gh_list (list,
				  gh_list (gh_str02scm (_(glibtop_labels_netload [i]))),
				  SCM_UNDEFINED));

	return list;
}

static SCM
glibtop_guile_descriptions_netload (void)
{
	int i;
	SCM list;

	list = gh_list (SCM_UNDEFINED);

	for (i = 0; i < GLIBTOP_MAX_NETLOAD; i++)
		list = scm_append
			(gh_list (list,
				  gh_list (gh_str02scm (_(glibtop_descriptions_netload [i]))),
				  SCM_UNDEFINED));

	return list;
}

static SCM
glibtop_guile_names_fsusage (void)
{
	int i;
	SCM list;

	list = gh_list (SCM_UNDEFINED);

	for (i = 0; i < GLIBTOP_MAX_FSUSAGE; i++)
		list = scm_append
			(gh_list (list,
				  gh_list (gh_str02scm ((char *) glibtop_names_fsusage [i])),
				  SCM_UNDEFINED));

	return list;
}

static SCM
glibtop_guile_types_fsusage (void)
{
	SCM list;

	list = gh_list (gh_ulong2scm (glibtop_types_fsusage [0]),
			gh_ulong2scm (glibtop_types_fsusage [1]),
			gh_ulong2scm (glibtop_types_fsusage [2]),
			gh_ulong2scm (glibtop_types_fsusage [3]),
			gh_ulong2scm (glibtop_types_fsusage [4]),
			SCM_UNDEFINED);

	return list;
}

static SCM
glibtop_guile_labels_fsusage (void)
{
	int i;
	SCM list;

	list = gh_list (SCM_UNDEFINED);

	for (i = 0; i < GLIBTOP_MAX_FSUSAGE; i++)
		list = scm_append
			(gh_list (list,
				  gh_list (gh_str02scm (_(glibtop_labels_fsusage [i]))),
				  SCM_UNDEFINED));

	return list;
}

static SCM
glibtop_guile_descriptions_fsusage (void)
{
	int i;
	SCM list;

	list = gh_list (SCM_UNDEFINED);

	for (i = 0; i < GLIBTOP_MAX_FSUSAGE; i++)
		list = scm_append
			(gh_list (list,
				  gh_list (gh_str02scm (_(glibtop_descriptions_fsusage [i]))),
				  SCM_UNDEFINED));

	return list;
}

static SCM
glibtop_guile_names_msg_limits (void)
{
	int i;
	SCM list;

	list = gh_list (SCM_UNDEFINED);

	for (i = 0; i < GLIBTOP_MAX_MSG_LIMITS; i++)
		list = scm_append
			(gh_list (list,
				  gh_list (gh_str02scm ((char *) glibtop_names_msg_limits [i])),
				  SCM_UNDEFINED));

	return list;
}

static SCM
glibtop_guile_types_msg_limits (void)
{
	SCM list;

	list = gh_list (gh_ulong2scm (glibtop_types_msg_limits [0]),
			gh_ulong2scm (glibtop_types_msg_limits [1]),
			gh_ulong2scm (glibtop_types_msg_limits [2]),
			gh_ulong2scm (glibtop_types_msg_limits [3]),
			gh_ulong2scm (glibtop_types_msg_limits [4]),
			gh_ulong2scm (glibtop_types_msg_limits [5]),
			gh_ulong2scm (glibtop_types_msg_limits [6]),
			SCM_UNDEFINED);

	return list;
}

static SCM
glibtop_guile_labels_msg_limits (void)
{
	int i;
	SCM list;

	list = gh_list (SCM_UNDEFINED);

	for (i = 0; i < GLIBTOP_MAX_MSG_LIMITS; i++)
		list = scm_append
			(gh_list (list,
				  gh_list (gh_str02scm (_(glibtop_labels_msg_limits [i]))),
				  SCM_UNDEFINED));

	return list;
}

static SCM
glibtop_guile_descriptions_msg_limits (void)
{
	int i;
	SCM list;

	list = gh_list (SCM_UNDEFINED);

	for (i = 0; i < GLIBTOP_MAX_MSG_LIMITS; i++)
		list = scm_append
			(gh_list (list,
				  gh_list (gh_str02scm (_(glibtop_descriptions_msg_limits [i]))),
				  SCM_UNDEFINED));

	return list;
}

static SCM
glibtop_guile_names_proc_time (void)
{
	int i;
	SCM list;

	list = gh_list (SCM_UNDEFINED);

	for (i = 0; i < GLIBTOP_MAX_PROC_TIME; i++)
		list = scm_append
			(gh_list (list,
				  gh_list (gh_str02scm ((char *) glibtop_names_proc_time [i])),
				  SCM_UNDEFINED));

	return list;
}

static SCM
glibtop_guile_types_proc_time (void)
{
	SCM list;

	list = gh_list (gh_ulong2scm (glibtop_types_proc_time [0]),
			gh_ulong2scm (glibtop_types_proc_time [1]),
			gh_ulong2scm (glibtop_types_proc_time [2]),
			gh_ulong2scm (glibtop_types_proc_time [3]),
			gh_ulong2scm (glibtop_types_proc_time [4]),
			gh_ulong2scm (glibtop_types_proc_time [5]),
			gh_ulong2scm (glibtop_types_proc_time [6]),
			gh_ulong2scm (glibtop_types_proc_time [7]),
			gh_ulong2scm (glibtop_types_proc_time [8]),
			gh_cons
			(gh_ulong2scm (glibtop_types_proc_time [9]),
			 gh_ulong2scm (4)),
			gh_cons
			(gh_ulong2scm (glibtop_types_proc_time [10]),
			 gh_ulong2scm (4)),
			SCM_UNDEFINED);

	return list;
}

static SCM
glibtop_guile_labels_proc_time (void)
{
	int i;
	SCM list;

	list = gh_list (SCM_UNDEFINED);

	for (i = 0; i < GLIBTOP_MAX_PROC_TIME; i++)
		list = scm_append
			(gh_list (list,
				  gh_list (gh_str02scm (_(glibtop_labels_proc_time [i]))),
				  SCM_UNDEFINED));

	return list;
}

static SCM
glibtop_guile_descriptions_proc_time (void)
{
	int i;
	SCM list;

	list = gh_list (SCM_UNDEFINED);

	for (i = 0; i < GLIBTOP_MAX_PROC_TIME; i++)
		list = scm_append
			(gh_list (list,
				  gh_list (gh_str02scm (_(glibtop_descriptions_proc_time [i]))),
				  SCM_UNDEFINED));

	return list;
}

SCM_GLOBAL_VCELL (s_names_proc_kernel, "glibtop-names-proc-kernel");
SCM_GLOBAL_VCELL (s_labels_proc_kernel, "glibtop-labels-proc-kernel");
SCM_GLOBAL_VCELL (s_types_proc_kernel, "glibtop-types-proc-kernel");
SCM_GLOBAL_VCELL (s_descriptions_proc_kernel, "glibtop-descriptions-proc-kernel");
SCM_GLOBAL_VCELL (s_names_sysdeps, "glibtop-names-sysdeps");
SCM_GLOBAL_VCELL (s_labels_sysdeps, "glibtop-labels-sysdeps");
SCM_GLOBAL_VCELL (s_types_sysdeps, "glibtop-types-sysdeps");
SCM_GLOBAL_VCELL (s_descriptions_sysdeps, "glibtop-descriptions-sysdeps");
SCM_GLOBAL_VCELL (s_names_proclist, "glibtop-names-proclist");
SCM_GLOBAL_VCELL (s_labels_proclist, "glibtop-labels-proclist");
SCM_GLOBAL_VCELL (s_types_proclist, "glibtop-types-proclist");
SCM_GLOBAL_VCELL (s_descriptions_proclist, "glibtop-descriptions-proclist");
SCM_GLOBAL_VCELL (s_names_shm_limits, "glibtop-names-shm-limits");
SCM_GLOBAL_VCELL (s_labels_shm_limits, "glibtop-labels-shm-limits");
SCM_GLOBAL_VCELL (s_types_shm_limits, "glibtop-types-shm-limits");
SCM_GLOBAL_VCELL (s_descriptions_shm_limits, "glibtop-descriptions-shm-limits");
SCM_GLOBAL_VCELL (s_names_mem, "glibtop-names-mem");
SCM_GLOBAL_VCELL (s_labels_mem, "glibtop-labels-mem");
SCM_GLOBAL_VCELL (s_types_mem, "glibtop-types-mem");
SCM_GLOBAL_VCELL (s_descriptions_mem, "glibtop-descriptions-mem");
SCM_GLOBAL_VCELL (s_names_cpu, "glibtop-names-cpu");
SCM_GLOBAL_VCELL (s_labels_cpu, "glibtop-labels-cpu");
SCM_GLOBAL_VCELL (s_types_cpu, "glibtop-types-cpu");
SCM_GLOBAL_VCELL (s_descriptions_cpu, "glibtop-descriptions-cpu");
SCM_GLOBAL_VCELL (s_names_proc_state, "glibtop-names-proc-state");
SCM_GLOBAL_VCELL (s_labels_proc_state, "glibtop-labels-proc-state");
SCM_GLOBAL_VCELL (s_types_proc_state, "glibtop-types-proc-state");
SCM_GLOBAL_VCELL (s_descriptions_proc_state, "glibtop-descriptions-proc-state");
SCM_GLOBAL_VCELL (s_names_ppp, "glibtop-names-ppp");
SCM_GLOBAL_VCELL (s_labels_ppp, "glibtop-labels-ppp");
SCM_GLOBAL_VCELL (s_types_ppp, "glibtop-types-ppp");
SCM_GLOBAL_VCELL (s_descriptions_ppp, "glibtop-descriptions-ppp");
SCM_GLOBAL_VCELL (s_names_uptime, "glibtop-names-uptime");
SCM_GLOBAL_VCELL (s_labels_uptime, "glibtop-labels-uptime");
SCM_GLOBAL_VCELL (s_types_uptime, "glibtop-types-uptime");
SCM_GLOBAL_VCELL (s_descriptions_uptime, "glibtop-descriptions-uptime");
SCM_GLOBAL_VCELL (s_names_sem_limits, "glibtop-names-sem-limits");
SCM_GLOBAL_VCELL (s_labels_sem_limits, "glibtop-labels-sem-limits");
SCM_GLOBAL_VCELL (s_types_sem_limits, "glibtop-types-sem-limits");
SCM_GLOBAL_VCELL (s_descriptions_sem_limits, "glibtop-descriptions-sem-limits");
SCM_GLOBAL_VCELL (s_names_proc_args, "glibtop-names-proc-args");
SCM_GLOBAL_VCELL (s_labels_proc_args, "glibtop-labels-proc-args");
SCM_GLOBAL_VCELL (s_types_proc_args, "glibtop-types-proc-args");
SCM_GLOBAL_VCELL (s_descriptions_proc_args, "glibtop-descriptions-proc-args");
SCM_GLOBAL_VCELL (s_names_loadavg, "glibtop-names-loadavg");
SCM_GLOBAL_VCELL (s_labels_loadavg, "glibtop-labels-loadavg");
SCM_GLOBAL_VCELL (s_types_loadavg, "glibtop-types-loadavg");
SCM_GLOBAL_VCELL (s_descriptions_loadavg, "glibtop-descriptions-loadavg");
SCM_GLOBAL_VCELL (s_names_swap, "glibtop-names-swap");
SCM_GLOBAL_VCELL (s_labels_swap, "glibtop-labels-swap");
SCM_GLOBAL_VCELL (s_types_swap, "glibtop-types-swap");
SCM_GLOBAL_VCELL (s_descriptions_swap, "glibtop-descriptions-swap");
SCM_GLOBAL_VCELL (s_names_proc_segment, "glibtop-names-proc-segment");
SCM_GLOBAL_VCELL (s_labels_proc_segment, "glibtop-labels-proc-segment");
SCM_GLOBAL_VCELL (s_types_proc_segment, "glibtop-types-proc-segment");
SCM_GLOBAL_VCELL (s_descriptions_proc_segment, "glibtop-descriptions-proc-segment");
SCM_GLOBAL_VCELL (s_names_proc_uid, "glibtop-names-proc-uid");
SCM_GLOBAL_VCELL (s_labels_proc_uid, "glibtop-labels-proc-uid");
SCM_GLOBAL_VCELL (s_types_proc_uid, "glibtop-types-proc-uid");
SCM_GLOBAL_VCELL (s_descriptions_proc_uid, "glibtop-descriptions-proc-uid");
SCM_GLOBAL_VCELL (s_names_proc_signal, "glibtop-names-proc-signal");
SCM_GLOBAL_VCELL (s_labels_proc_signal, "glibtop-labels-proc-signal");
SCM_GLOBAL_VCELL (s_types_proc_signal, "glibtop-types-proc-signal");
SCM_GLOBAL_VCELL (s_descriptions_proc_signal, "glibtop-descriptions-proc-signal");
SCM_GLOBAL_VCELL (s_names_proc_map, "glibtop-names-proc-map");
SCM_GLOBAL_VCELL (s_labels_proc_map, "glibtop-labels-proc-map");
SCM_GLOBAL_VCELL (s_types_proc_map, "glibtop-types-proc-map");
SCM_GLOBAL_VCELL (s_descriptions_proc_map, "glibtop-descriptions-proc-map");
SCM_GLOBAL_VCELL (s_names_proc_mem, "glibtop-names-proc-mem");
SCM_GLOBAL_VCELL (s_labels_proc_mem, "glibtop-labels-proc-mem");
SCM_GLOBAL_VCELL (s_types_proc_mem, "glibtop-types-proc-mem");
SCM_GLOBAL_VCELL (s_descriptions_proc_mem, "glibtop-descriptions-proc-mem");
SCM_GLOBAL_VCELL (s_names_mountlist, "glibtop-names-mountlist");
SCM_GLOBAL_VCELL (s_labels_mountlist, "glibtop-labels-mountlist");
SCM_GLOBAL_VCELL (s_types_mountlist, "glibtop-types-mountlist");
SCM_GLOBAL_VCELL (s_descriptions_mountlist, "glibtop-descriptions-mountlist");
SCM_GLOBAL_VCELL (s_names_netload, "glibtop-names-netload");
SCM_GLOBAL_VCELL (s_labels_netload, "glibtop-labels-netload");
SCM_GLOBAL_VCELL (s_types_netload, "glibtop-types-netload");
SCM_GLOBAL_VCELL (s_descriptions_netload, "glibtop-descriptions-netload");
SCM_GLOBAL_VCELL (s_names_fsusage, "glibtop-names-fsusage");
SCM_GLOBAL_VCELL (s_labels_fsusage, "glibtop-labels-fsusage");
SCM_GLOBAL_VCELL (s_types_fsusage, "glibtop-types-fsusage");
SCM_GLOBAL_VCELL (s_descriptions_fsusage, "glibtop-descriptions-fsusage");
SCM_GLOBAL_VCELL (s_names_msg_limits, "glibtop-names-msg-limits");
SCM_GLOBAL_VCELL (s_labels_msg_limits, "glibtop-labels-msg-limits");
SCM_GLOBAL_VCELL (s_types_msg_limits, "glibtop-types-msg-limits");
SCM_GLOBAL_VCELL (s_descriptions_msg_limits, "glibtop-descriptions-msg-limits");
SCM_GLOBAL_VCELL (s_names_proc_time, "glibtop-names-proc-time");
SCM_GLOBAL_VCELL (s_labels_proc_time, "glibtop-labels-proc-time");
SCM_GLOBAL_VCELL (s_types_proc_time, "glibtop-types-proc-time");
SCM_GLOBAL_VCELL (s_descriptions_proc_time, "glibtop-descriptions-proc-time");

void
glibtop_boot_guile_names (void)
{
#include "guile-names.x"
SCM_SETCDR (s_names_proc_kernel, glibtop_guile_names_proc_kernel ());
SCM_SETCDR (s_labels_proc_kernel, glibtop_guile_labels_proc_kernel ());
SCM_SETCDR (s_types_proc_kernel, glibtop_guile_types_proc_kernel ());
SCM_SETCDR (s_descriptions_proc_kernel, glibtop_guile_descriptions_proc_kernel ());
SCM_SETCDR (s_names_sysdeps, glibtop_guile_names_sysdeps ());
SCM_SETCDR (s_labels_sysdeps, glibtop_guile_labels_sysdeps ());
SCM_SETCDR (s_types_sysdeps, glibtop_guile_types_sysdeps ());
SCM_SETCDR (s_descriptions_sysdeps, glibtop_guile_descriptions_sysdeps ());
SCM_SETCDR (s_names_proclist, glibtop_guile_names_proclist ());
SCM_SETCDR (s_labels_proclist, glibtop_guile_labels_proclist ());
SCM_SETCDR (s_types_proclist, glibtop_guile_types_proclist ());
SCM_SETCDR (s_descriptions_proclist, glibtop_guile_descriptions_proclist ());
SCM_SETCDR (s_names_shm_limits, glibtop_guile_names_shm_limits ());
SCM_SETCDR (s_labels_shm_limits, glibtop_guile_labels_shm_limits ());
SCM_SETCDR (s_types_shm_limits, glibtop_guile_types_shm_limits ());
SCM_SETCDR (s_descriptions_shm_limits, glibtop_guile_descriptions_shm_limits ());
SCM_SETCDR (s_names_mem, glibtop_guile_names_mem ());
SCM_SETCDR (s_labels_mem, glibtop_guile_labels_mem ());
SCM_SETCDR (s_types_mem, glibtop_guile_types_mem ());
SCM_SETCDR (s_descriptions_mem, glibtop_guile_descriptions_mem ());
SCM_SETCDR (s_names_cpu, glibtop_guile_names_cpu ());
SCM_SETCDR (s_labels_cpu, glibtop_guile_labels_cpu ());
SCM_SETCDR (s_types_cpu, glibtop_guile_types_cpu ());
SCM_SETCDR (s_descriptions_cpu, glibtop_guile_descriptions_cpu ());
SCM_SETCDR (s_names_proc_state, glibtop_guile_names_proc_state ());
SCM_SETCDR (s_labels_proc_state, glibtop_guile_labels_proc_state ());
SCM_SETCDR (s_types_proc_state, glibtop_guile_types_proc_state ());
SCM_SETCDR (s_descriptions_proc_state, glibtop_guile_descriptions_proc_state ());
SCM_SETCDR (s_names_ppp, glibtop_guile_names_ppp ());
SCM_SETCDR (s_labels_ppp, glibtop_guile_labels_ppp ());
SCM_SETCDR (s_types_ppp, glibtop_guile_types_ppp ());
SCM_SETCDR (s_descriptions_ppp, glibtop_guile_descriptions_ppp ());
SCM_SETCDR (s_names_uptime, glibtop_guile_names_uptime ());
SCM_SETCDR (s_labels_uptime, glibtop_guile_labels_uptime ());
SCM_SETCDR (s_types_uptime, glibtop_guile_types_uptime ());
SCM_SETCDR (s_descriptions_uptime, glibtop_guile_descriptions_uptime ());
SCM_SETCDR (s_names_sem_limits, glibtop_guile_names_sem_limits ());
SCM_SETCDR (s_labels_sem_limits, glibtop_guile_labels_sem_limits ());
SCM_SETCDR (s_types_sem_limits, glibtop_guile_types_sem_limits ());
SCM_SETCDR (s_descriptions_sem_limits, glibtop_guile_descriptions_sem_limits ());
SCM_SETCDR (s_names_proc_args, glibtop_guile_names_proc_args ());
SCM_SETCDR (s_labels_proc_args, glibtop_guile_labels_proc_args ());
SCM_SETCDR (s_types_proc_args, glibtop_guile_types_proc_args ());
SCM_SETCDR (s_descriptions_proc_args, glibtop_guile_descriptions_proc_args ());
SCM_SETCDR (s_names_loadavg, glibtop_guile_names_loadavg ());
SCM_SETCDR (s_labels_loadavg, glibtop_guile_labels_loadavg ());
SCM_SETCDR (s_types_loadavg, glibtop_guile_types_loadavg ());
SCM_SETCDR (s_descriptions_loadavg, glibtop_guile_descriptions_loadavg ());
SCM_SETCDR (s_names_swap, glibtop_guile_names_swap ());
SCM_SETCDR (s_labels_swap, glibtop_guile_labels_swap ());
SCM_SETCDR (s_types_swap, glibtop_guile_types_swap ());
SCM_SETCDR (s_descriptions_swap, glibtop_guile_descriptions_swap ());
SCM_SETCDR (s_names_proc_segment, glibtop_guile_names_proc_segment ());
SCM_SETCDR (s_labels_proc_segment, glibtop_guile_labels_proc_segment ());
SCM_SETCDR (s_types_proc_segment, glibtop_guile_types_proc_segment ());
SCM_SETCDR (s_descriptions_proc_segment, glibtop_guile_descriptions_proc_segment ());
SCM_SETCDR (s_names_proc_uid, glibtop_guile_names_proc_uid ());
SCM_SETCDR (s_labels_proc_uid, glibtop_guile_labels_proc_uid ());
SCM_SETCDR (s_types_proc_uid, glibtop_guile_types_proc_uid ());
SCM_SETCDR (s_descriptions_proc_uid, glibtop_guile_descriptions_proc_uid ());
SCM_SETCDR (s_names_proc_signal, glibtop_guile_names_proc_signal ());
SCM_SETCDR (s_labels_proc_signal, glibtop_guile_labels_proc_signal ());
SCM_SETCDR (s_types_proc_signal, glibtop_guile_types_proc_signal ());
SCM_SETCDR (s_descriptions_proc_signal, glibtop_guile_descriptions_proc_signal ());
SCM_SETCDR (s_names_proc_map, glibtop_guile_names_proc_map ());
SCM_SETCDR (s_labels_proc_map, glibtop_guile_labels_proc_map ());
SCM_SETCDR (s_types_proc_map, glibtop_guile_types_proc_map ());
SCM_SETCDR (s_descriptions_proc_map, glibtop_guile_descriptions_proc_map ());
SCM_SETCDR (s_names_proc_mem, glibtop_guile_names_proc_mem ());
SCM_SETCDR (s_labels_proc_mem, glibtop_guile_labels_proc_mem ());
SCM_SETCDR (s_types_proc_mem, glibtop_guile_types_proc_mem ());
SCM_SETCDR (s_descriptions_proc_mem, glibtop_guile_descriptions_proc_mem ());
SCM_SETCDR (s_names_mountlist, glibtop_guile_names_mountlist ());
SCM_SETCDR (s_labels_mountlist, glibtop_guile_labels_mountlist ());
SCM_SETCDR (s_types_mountlist, glibtop_guile_types_mountlist ());
SCM_SETCDR (s_descriptions_mountlist, glibtop_guile_descriptions_mountlist ());
SCM_SETCDR (s_names_netload, glibtop_guile_names_netload ());
SCM_SETCDR (s_labels_netload, glibtop_guile_labels_netload ());
SCM_SETCDR (s_types_netload, glibtop_guile_types_netload ());
SCM_SETCDR (s_descriptions_netload, glibtop_guile_descriptions_netload ());
SCM_SETCDR (s_names_fsusage, glibtop_guile_names_fsusage ());
SCM_SETCDR (s_labels_fsusage, glibtop_guile_labels_fsusage ());
SCM_SETCDR (s_types_fsusage, glibtop_guile_types_fsusage ());
SCM_SETCDR (s_descriptions_fsusage, glibtop_guile_descriptions_fsusage ());
SCM_SETCDR (s_names_msg_limits, glibtop_guile_names_msg_limits ());
SCM_SETCDR (s_labels_msg_limits, glibtop_guile_labels_msg_limits ());
SCM_SETCDR (s_types_msg_limits, glibtop_guile_types_msg_limits ());
SCM_SETCDR (s_descriptions_msg_limits, glibtop_guile_descriptions_msg_limits ());
SCM_SETCDR (s_names_proc_time, glibtop_guile_names_proc_time ());
SCM_SETCDR (s_labels_proc_time, glibtop_guile_labels_proc_time ());
SCM_SETCDR (s_types_proc_time, glibtop_guile_types_proc_time ());
SCM_SETCDR (s_descriptions_proc_time, glibtop_guile_descriptions_proc_time ());
}
