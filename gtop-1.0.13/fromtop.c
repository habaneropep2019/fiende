#include <stdio.h>
#include <sys/time.h>
#include <pwd.h>

#include <fromtop.h>
#include <global.h>

#include <glibtop.h>
#include <glibtop/union.h>

void
sprint_time (char *s, time_t ti, unsigned long frequency)
{
	time_t t = ti / frequency;
	time_t centi_sec = (ti % 100) * 100 / 100;

	if (t < 0)                            /* time overflow */
		sprintf(s, ">>d");
	else if (t >= 48*60*60)               /* > 2 days */
		sprintf(s, "%5lud", t/(24*60*60));
	else if (t >= 60*60)                  /* > 1 hour */
		sprintf(s, "%2lu:%02uh", t/(60*60), (unsigned) ((t/60)%60));
	else if (t > 60)                      /* > 1 minute */
		sprintf(s, "%2lu:%02um", t/60, (unsigned) t%60);
	else
		sprintf(s, "%2lu.%02lus", t, centi_sec);
}

/*
 * Finds the current time (in microseconds) and calculates the time
 * elapsed since the last update. This is essential for computing
 * percent CPU usage.
 */
float get_elapsed_time(void)
{
    struct timeval time;
    struct timezone timez;
    static double oldtime = 0.0;
    float elapsed_time;
    double newtime;

    gettimeofday (&time, &timez);

    newtime = ((double) time.tv_sec) + ((double) time.tv_usec / 1000000.0);

    elapsed_time = (float) (newtime - oldtime);
    oldtime = newtime;

    return elapsed_time;
}

char * status(gtop_proc_t* task) {
#if (LIBGTOP_VERSION_CODE >= 1001000)
    static char buf[32];
    int pos = 0;

    if (task->state & GLIBTOP_PROCESS_RUNNING)
	buf[pos++] = 'R';
    if (task->state & GLIBTOP_PROCESS_INTERRUPTIBLE)
	buf[pos++] = 'S';
    if (task->state & GLIBTOP_PROCESS_UNINTERRUPTIBLE)
	buf[pos++] = 'D';
    if (task->state & GLIBTOP_PROCESS_ZOMBIE)
	buf[pos++] = 'Z';
    if (task->state & GLIBTOP_PROCESS_STOPPED)
	buf[pos++] = 'T';

    if (((task->rss == 0) && !(task->state & GLIBTOP_PROCESS_ZOMBIE)) ||
	(task->state & GLIBTOP_PROCESS_SWAPPING))
	buf[pos++] = 'W';

    if (task->nice < 0)
	buf[pos++] = '<';
    else if (task->nice > 0)
	buf[pos++] = 'N';

    buf[pos] = '\0';
#else
    static char buf[4] = "   ";

    buf[0] = task->state;
    if (task->rss == 0 && task->state != 'Z')
        buf[1] = 'W';
    else
        buf[1] = ' ';
    if (task->nice < 0)
	buf[2] = '<';
    else if (task->nice > 0)
	buf[2] = 'N';
    else
	buf[2] = ' ';
#endif

    return(buf);
}

gtop_proc_t *
get_proc_data (gtop_proc_t *p, pid_t pid)
{
    glibtop_proc_state procstate;
    glibtop_proc_time proctime;
    glibtop_proc_uid procuid;
    glibtop_proc_mem procmem;
    glibtop_mem mem;
    glibtop_cpu cpu;

    unsigned long p_gl_main_mem;
    struct passwd *pwd;

    if (!p)
	p = g_new0 (gtop_proc_t, 1);

    p->pid = pid;

    glibtop_get_proc_state (&procstate, p->pid);
		
    p->cmd = g_strdup (procstate.cmd);
    p->state = procstate.state;

    pwd = getpwuid (procstate.uid);
    if (pwd)
	p->user = g_strdup (pwd->pw_name);
    else
	p->user = g_strdup_printf ("<%d>", procstate.uid);

    glibtop_get_proc_uid (&procuid, p->pid);

    p->ppid	= procuid.ppid;
    p->uid	= procuid.uid;

    p->nice     = procuid.nice;
    p->priority = procuid.priority;

    glibtop_get_proc_mem (&procmem, p->pid);

    p->size     = (unsigned long) procmem.size >> 10;
    p->rss      = (unsigned long) procmem.rss >> 10;
    p->resident = (unsigned long) procmem.resident >> 10;
    p->share    = (unsigned long) procmem.share >> 10;

    glibtop_get_proc_time (&proctime, p->pid);

    p->frequency = proctime.frequency ? proctime.frequency : 1000000;

    p->utime  = ((unsigned long) (proctime.utime * 100) / p->frequency);
    p->stime  = ((unsigned long) (proctime.stime * 100) / p->frequency);
    p->cutime = ((unsigned long) (proctime.cutime * 100) / p->frequency);
    p->cstime = ((unsigned long) (proctime.cstime * 100) / p->frequency);

    p->start_time =
	((unsigned long) (proctime.start_time * 100) / p->frequency);

    p->state_string = status (p);

    glibtop_get_cpu (&cpu);

    if (!p->start_cpu_user || !p->start_cpu_system) {
	p->start_cpu_user   = (unsigned long) cpu.user;
	p->start_cpu_system = (unsigned long) cpu.sys;
	p->start_utime      = p->utime;
	p->start_stime      = p->stime;
    } else {
	unsigned long user_diff, system_diff;
	unsigned long utime_diff, stime_diff;
	unsigned long this_diff, total_diff;

	user_diff	= (unsigned long) cpu.user - p->start_cpu_user;
	system_diff	= (unsigned long) cpu.sys  - p->start_cpu_system;

	utime_diff	= p->utime - p->start_utime;
	stime_diff	= p->stime - p->start_stime;

	total_diff	= user_diff + system_diff;
	this_diff	= utime_diff + stime_diff;

#if 0
	fprintf (stderr, "WCPU: (%lu,%lu) - (%lu,%lu) - (%lu,%lu)\n",
		 user_diff, system_diff, utime_diff, stime_diff,
		 total_diff, this_diff);
#endif

	p->wcpu		= this_diff * 1000 / total_diff;
    }

    if (p->cpu_user && p->cpu_system) {
	unsigned long user_diff, system_diff;
	unsigned long utime_diff, stime_diff;
	unsigned long this_diff, total_diff;

	user_diff	= (unsigned long) cpu.user - p->cpu_user;
	system_diff	= (unsigned long) cpu.sys  - p->cpu_system;

	utime_diff	= p->utime - p->last_utime;
	stime_diff	= p->stime - p->last_stime;

	total_diff	= user_diff + system_diff;
	this_diff	= utime_diff + stime_diff;

	p->pcpu		= this_diff * 1000 / total_diff;

#if 0
	fprintf (stderr, "CPU: (%lu,%lu) - (%lu,%lu) - (%lu,%lu) - %lu\n",
		 user_diff, system_diff, utime_diff, stime_diff,
		 total_diff, this_diff, p->pcpu);
#endif
    }

    p->cpu_user   = (unsigned long) cpu.user;
    p->cpu_system = (unsigned long) cpu.sys;

    p->last_utime = p->utime;
    p->last_stime = p->stime;

    glibtop_get_mem (&mem);
    p_gl_main_mem = mem.total >> 10;
    /* avoid divison by zero later. */
    if (!p_gl_main_mem) p_gl_main_mem = 1;

    p->pmem = p->rss * 1000 / p_gl_main_mem;

    p->update_count++;

    return p;
}
