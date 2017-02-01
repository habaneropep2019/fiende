#ifndef __FROMTOP_H__
#define __FROMTOP_H__

#include <config.h>

#include <sys/time.h>
#include <glibtop/procstate.h>
#include <dummy.h>

#define MAX_NR_TASKS 4096

float get_elapsed_time (void);
void sprint_time (char *s, time_t t, unsigned long frequency);
char *status (gtop_proc_t * task);
gtop_proc_t *get_proc_data (gtop_proc_t *p, pid_t pid);

#endif
