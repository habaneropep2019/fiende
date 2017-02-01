#include "CosNaming.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <signal.h>
#include "CosNaming_impl.h"

void
signal_handler(int signo){
  syslog(LOG_ERR,"Receveived signal %d\nshutting down.", signo);
  switch(signo) {
    case SIGSEGV:
	abort();
    default:
	exit(1);
  }
}

int
main (int argc, char *argv[])
{
  CORBA_ORB orb;
  CORBA_Environment ev;
  CosNaming_NamingContext context;
  CORBA_char *objref;
  PortableServer_POA root_poa;
  PortableServer_POAManager pm;
  struct sigaction act;
  sigset_t         empty_mask;
  
  openlog("orbit-name-server", LOG_NDELAY | LOG_PID, LOG_DAEMON);
  syslog(LOG_INFO,"starting");
  sigemptyset(&empty_mask);
  act.sa_handler = signal_handler;
  act.sa_mask    = empty_mask;
  act.sa_flags   = 0;
  
  sigaction(SIGINT,  &act, 0);
  sigaction(SIGHUP,  &act, 0);
  sigaction(SIGSEGV, &act, 0);
  sigaction(SIGABRT, &act, 0);
  
  act.sa_handler = SIG_IGN;
  sigaction(SIGPIPE, &act, 0);
  
  CORBA_exception_init (&ev);
  
  orb = CORBA_ORB_init (&argc, argv, "orbit-local-orb", &ev);

  root_poa = (PortableServer_POA)
    CORBA_ORB_resolve_initial_references (orb, "RootPOA", &ev);

  context = impl_CosNaming_NamingContext__create(root_poa, &ev);

  objref = CORBA_ORB_object_to_string (orb, context, &ev);

  g_print ("%s\n", objref);
  fflush (stdout);

  CORBA_free (objref);

#if 0
  /* Don't release it so that the same-address-space info will stay around */
  CORBA_Object_release (context, &ev);
#endif

  pm = PortableServer_POA__get_the_POAManager (root_poa, &ev);
  PortableServer_POAManager_activate (pm, &ev);

  CORBA_ORB_run (orb, &ev);
  syslog(LOG_INFO, "exiting");
  return 0;
}
