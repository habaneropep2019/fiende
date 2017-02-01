#include "util.h"

#ifdef NOT_REENTRANT
#include <pthread.h>

pthread_key_t thread_data;

void init_thread_data(void) __attribute__ ((constructor));

void init_thread_data(void)
{
  pthread_key_create(&thread_data, NULL);
}

#else

gpointer prog_data = NULL;

#endif
