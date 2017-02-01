#ifndef __DUMMY_H__
#define __DUMMY_H__

#include <config.h>

#define PROC_FILLMEM	0
#define PROC_FILLCMD	0
#define PROC_FILLENV	0
#define PROC_FILLTTY	0
#define PROC_FILLUSR	0
#define PROC_ANYTTY	0
#define PROC_UID	0

#define NR_TASKS	1024

typedef struct _gtop_proc_t gtop_proc_t;

struct _gtop_proc_t
{
    long update_count;
    long pid, ppid, uid, pcpu, wcpu, pmem;
    unsigned long start_utime, start_stime;
    unsigned long start_cpu_user, start_cpu_system;
    unsigned long cpu_user, cpu_system, last_utime, last_stime;
    long frequency, stime, utime, cutime, cstime, start_time;
    long size, rss, resident, nice, priority, share;
    char *user, *cmd, *state_string;
#if (LIBGTOP_VERSION_CODE >= 1001000)
    unsigned state;
#else
    char state;
#endif
};

#endif
