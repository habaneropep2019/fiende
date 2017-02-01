/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */
#ifndef OAFD_H
#define OAFD_H 1

#include "oaf.h"

#ifdef g_alloca
#define oaf_alloca g_alloca
#else
#define oaf_alloca alloca
#ifdef HAVE_ALLOCA_H
#include <alloca.h>
#endif
#endif

/* od-corba.c */
OAF_ObjectDirectory OAF_ObjectDirectory_create (PortableServer_POA poa,
                                                const char *domain,
                                                const char *source_directory,
                                                CORBA_Environment * ev);
/* od-load.c */

void OAF_ServerInfo_load (char                **dirs,
                          OAF_ServerInfoList   *servers,
                          GHashTable          **by_iid,
                          const char           *host, 
                          const char           *domain);

/* od-activate.c */
typedef struct
{
	OAF_ActivationContext ac;
	OAF_ActivationFlags flags;
	CORBA_Context ctx;
}
ODActivationInfo;

CORBA_Object od_server_activate (OAF_ServerInfo * si,
				 ODActivationInfo * actinfo,
				 CORBA_Object od_obj, CORBA_Environment * ev);

/* ac-corba.c */
OAF_ActivationContext
OAF_ActivationContext_create (PortableServer_POA poa, CORBA_Environment * ev);

#endif
