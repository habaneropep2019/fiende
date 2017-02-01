/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

/*
 *  ORBit: A CORBA v2.2 ORB
 *
 *  Copyright (C) 1998 Richard H. Porter and Red Hat Software
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the Free
 *  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  Author: Dick Porter <dick@acm.org>
 *          Elliot Lee <sopwith@redhat.com>
 *
 */

#undef PROFILE_DEBUG

#define o_return_val_if_fail(expr, val) if(!(expr)) { CORBA_exception_set_system(ev, ex_CORBA_BAD_PARAM, CORBA_COMPLETED_NO); return (val); }
#define o_return_if_fail(expr) if(!(expr)) { CORBA_exception_set_system(ev, ex_CORBA_BAD_PARAM, CORBA_COMPLETED_NO); return; }

#include "config.h"

#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <pwd.h>
#include <time.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <utime.h>

#include "../IIOP/iiop-endianP.h"
#include "orbit.h"

#include "orbit_poa.h"
#include "orbit_object.h"
#include "orbit_object_type.h"

#ifndef SUN_LEN
/* This system is not POSIX.1g.  */
#define SUN_LEN(ptr) ((size_t) (((struct sockaddr_un *) 0)->sun_path)  \
                      + strlen ((ptr)->sun_path))
#endif

static void ORBit_ORB_release(CORBA_ORB orb, CORBA_Environment *ev);

static const ORBit_RootObject_Interface CORBA_ORB_epv =
{
	(void (*)(gpointer, CORBA_Environment *))ORBit_ORB_release
};


static int ORBit_ORBid_setup(CORBA_ORB orb, CORBA_ORBid id)
{
	g_assert(orb!=NULL);
	g_assert(id!=NULL);

	if(strcmp(id, "orbit-local-orb")) {
		fprintf(stderr, "ORBit_ORBid_setup: Unknown ORB id: %s\n", id);
		return(0);
	}

	orb->orb_identifier=g_strdup(id);

	return(1);
}

static gboolean
free_key_and_data(gpointer key, gpointer data, gpointer user_data)
{
	g_free(key); g_free(data);

	return TRUE;
}

static void ORBit_rc_load(const char *rcfile, ORBit_orb_options *options)
{
	FILE *fp;
	GHashTable *read_options;
	ORBit_orb_options *search;
	char buf[1024];

	fp=fopen(rcfile, "r");

	if(fp==NULL)
		return;

	read_options=g_hash_table_new(g_str_hash, g_str_equal);

	while(fgets(buf, 1024, fp)) {
		guchar *bptr, *tmp, *key, *data;
		size_t start, len;
		if(buf[0]=='#')
			continue;
		
		bptr=buf;
		tmp=strpbrk(bptr, " \t\n=");
		if(tmp==NULL) continue;
		*tmp++='\0';
		key=g_strdup(bptr);
		bptr=tmp;

		start=0;
		while(bptr+start != '\0' &&
		     (isspace(bptr[start]) || bptr[start]=='='))
			start++;
		len=strcspn(bptr+start, " \t\n");
		bptr[start+len]='\0';
		if(len>0) {
			data=g_strdup(bptr+start);
		} else {
			data=g_strdup("TRUE");
		}
		g_hash_table_insert(read_options, key, data);
	}
	fclose(fp);

	for(search=options;search->name!=NULL;search++) {
		char *read_val;

		read_val=g_hash_table_lookup(read_options, search->name);
		if(read_val!=NULL) {
			ORBit_option_set(search, read_val);
		}
	}

	g_hash_table_foreach_remove(read_options, free_key_and_data, NULL);
	g_hash_table_destroy(read_options);
}

static void ORBit_ORB_release(CORBA_ORB orb, CORBA_Environment *ev) 
{
	g_assert(orb!=NULL);

	if(--(ORBIT_ROOT_OBJECT(orb)->refs))
		return;

	if(orb->orb_identifier!=NULL) {
		g_free(orb->orb_identifier);
	}
	if(!CORBA_Object_is_nil(orb->imr, ev)) {
		CORBA_Object_release(orb->imr, ev); 
	}
	if(!CORBA_Object_is_nil(orb->ir, ev)) {
		CORBA_Object_release(orb->ir, ev); 
	}
	if(!CORBA_Object_is_nil(orb->naming, ev)) {
		CORBA_Object_release(orb->naming, ev);
	}
	if(!CORBA_Object_is_nil(orb->root_poa, ev)) {
		CORBA_Object_release(orb->root_poa, ev);
	}
	if(orb->cnx.ipv4)
		giop_connection_unref(orb->cnx.ipv4);
	if(orb->cnx.ipv6)
		giop_connection_unref(orb->cnx.ipv6);
	if(orb->cnx.usock)
		giop_connection_unref(orb->cnx.usock);

	g_free(orb);
}

static void
ORBit_make_local_tmpdir(void)
{
	struct stat statbuf;
	GString *tmpstr;

	tmpstr = g_string_new(NULL);

	g_string_sprintf(tmpstr, "/tmp/orbit-%s", g_get_user_name());
		
	if(mkdir(tmpstr->str, 0700) != 0) {
		int e = errno;
			
		switch (e) {
		case 0:
		case EEXIST:
			if (stat(tmpstr->str, &statbuf) != 0)
				g_error ("Can not stat %s\n", tmpstr->str);

			if (statbuf.st_uid != getuid ())
				g_error ("Owner of %s is not the current user\n",
					 tmpstr->str);

			if((statbuf.st_mode & (S_IRWXG|S_IRWXO))
			   || !S_ISDIR(statbuf.st_mode))
				g_error ("Wrong permissions for %s\n",
					 tmpstr->str);
			break;
				
		default:
			g_error("Unknown error on directory creation of %s (%s)\n",
				tmpstr->str, g_strerror (e));
		}
	}

	{
		struct utimbuf utb;
		memset(&utb, 0, sizeof(utb));
		/* I think this is here to hide some information from somebody.
		   I forgot exactly why. */
		utime(tmpstr->str, &utb);
	}

	g_string_free(tmpstr, TRUE);
}

static GIOPConnection*
ORBit_ORB_make_usock_connection(void)
{
	GIOPConnection *retval = NULL;
	GString *tmpstr;

	tmpstr = g_string_new(NULL);

#ifdef WE_DONT_CARE_ABOUT_STUPID_2DOT0DOTX_KERNELS
	g_string_sprintf(tmpstr, "/tmp/orbit-%s",
			 g_get_user_name());
	dirh = opendir(tmpstr->str);
	while(!retval && (dent = readdir(dirh))) {
		int usfd, ret;
		struct sockaddr_un saddr;

		saddr.sun_family = AF_UNIX;

		if(strncmp(dent->d_name, "orb-", 4))
			continue;

		g_snprintf(saddr.sun_path, sizeof(saddr.sun_path),
			   "/tmp/orbit-%s/%s",
			   g_get_user_name(), dent->d_name);

		usfd = socket(AF_UNIX, SOCK_STREAM, 0);
		g_assert(usfd >= 0);

		ret = connect(usfd, &saddr, SUN_LEN(&saddr));
		close(usfd);

		if(ret >= 0)
			continue;

		unlink(saddr.sun_path);
	}
	closedir(dirh);
#endif /* WE_DONT_CARE_ABOUT_STUPID_2DOT0DOTX_KERNELS */

	srand(time(NULL));
	while(!retval) {
		g_string_sprintf(tmpstr, "/tmp/orbit-%s/orb-%d%d",
				 g_get_user_name(), rand(), rand());
		retval =
			GIOP_CONNECTION(iiop_connection_server_unix(tmpstr->str));
	}

	g_string_free(tmpstr, TRUE);

	return retval;
}

/* Section 4.4
 *
 * Adjusts argc and argv appropriately
 */
CORBA_ORB CORBA_ORB_init(int *argc, char **argv, CORBA_ORBid orb_identifier, CORBA_Environment *ev)
{
	int no_iiop_server=0;
	int no_iiop_proxy=0;
	int use_ipv4=0;
	int use_ipv6=0;
	int use_usock=1;
	int debug_level=0;
	int debug_modules=0;
	int nosysrc=0;
	int nouserrc=0;
	char *imr_ior=NULL, *imr_addr=NULL;
	char *ir_ior=NULL, *ir_addr=NULL;
	char *naming_ior=NULL, *naming_addr=NULL;
	char *root_poa_ior=NULL, *root_poa_addr=NULL;
	char *orb_id_opt=NULL;
	char *ctmp;
	static CORBA_ORB orb=NULL;

	/* The variable addresses in these structs need to be assigned at
	 * run-time if you want to compile with -pedantic
	 *
	 * (You will also get scads of warnings about "long long" too)
	 */

	ORBit_orb_options pre_rc_options[]={
		{"ORBNoSystemRC", no_arg, NULL},
		{"ORBNoUserRC", no_arg, NULL},
		{NULL, 0, NULL},
	};

	/* These options are compatible with MICO */
	ORBit_orb_options options[]={
		{"ORBNoIIOPServer", no_arg, NULL},
		{"ORBNoIIOPProxy", no_arg, NULL},
		{"ORBid", string_arg, NULL},
		{"ORBImplRepoIOR", string_arg, NULL},
		{"ORBImplRepoAddr", string_arg, NULL},
		{"ORBIfaceRepoIOR", string_arg, NULL},
		{"ORBIfaceRepoAddr", string_arg, NULL},
		{"ORBNamingIOR", string_arg, NULL},
		{"ORBNamingAddr", string_arg, NULL},
		{"ORBDebugLevel", int_arg, NULL},
		{"ORBBindAddr", string_arg, NULL}, /* XXX need to make
						      libIIOP support this */
		{"ORBIIOPAddr", string_arg, NULL},

	/* These options aren't */
		{"ORBDebugModules", int_arg, NULL},
		{"ORBRootPOAIOR", string_arg, NULL},
		{"ORBRootPOAAddr", string_arg, NULL},
		{"ORBIIOPUSock", int_arg, NULL},
		{"ORBIIOPIPv4", int_arg, NULL},
		{"ORBIIOPIPv6", int_arg, NULL},
		{NULL,0,NULL},
	};

	g_return_val_if_fail(ev != NULL, NULL);
	o_return_val_if_fail(argc && argv && orb_identifier, NULL);

	if(orb)
		return orb;

	pre_rc_options[0].arg = (void *) &nosysrc;
	pre_rc_options[1].arg = (void *) &nouserrc;
	options[0].arg = (void *) &no_iiop_server;
	options[1].arg = (void *) &no_iiop_proxy;
	options[2].arg = (void *) &orb_id_opt;
	options[3].arg = (void *) &imr_ior;
	options[4].arg = (void *) &imr_addr;
	options[5].arg = (void *) &ir_ior;
	options[6].arg = (void *) &ir_addr;
	options[7].arg = (void *) &naming_ior;
	options[8].arg = (void *) &naming_addr;
	options[9].arg = (void *) &debug_level;
	options[12].arg = (void *) &debug_modules;
	options[13].arg = (void *) &root_poa_ior;
	options[14].arg = (void *) &root_poa_addr;
	options[15].arg = (void *) &use_usock;
	options[16].arg = (void *) &use_ipv4;
	options[17].arg = (void *) &use_ipv6;

	ORBit_option_parse(argc, argv, pre_rc_options);

	if(!nosysrc) {
		ORBit_rc_load(ORBit_SYSRC, options);
	}

	if(!nouserrc) {
		char *buf;

		ctmp = g_get_home_dir();

		buf = alloca(strlen(ctmp) + sizeof("/.orbitrc"));
		sprintf(buf, "%s/.orbitrc", ctmp);
		ORBit_rc_load(buf, options);
	}

	ORBit_option_parse(argc, argv, options);

	ORBit_Trace_setLevel(debug_level);
	ORBit_Trace_setModules(debug_modules);

	CORBA_exception_init(ev);

	ORBit_chunks_init();

	giop_init(argv[0]);

	orb=g_new0(struct CORBA_ORB_type, 1);

	if(orb==NULL) {
		CORBA_exception_set_system(ev, ex_CORBA_NO_MEMORY, CORBA_COMPLETED_NO);
		goto error;
	}

	ORBit_pseudo_object_init(ORBIT_PSEUDO_OBJECT(orb), ORBIT_PSEUDO_ORB, ev);

	ORBit_RootObject_set_interface(ORBIT_ROOT_OBJECT(orb),
				       (gpointer)&CORBA_ORB_epv, ev);

	ORBIT_ROOT_OBJECT(orb)->refs = 1;

	if(orb_identifier!=NULL && *orb_identifier!='\0') {
		if(!ORBit_ORBid_setup(orb, orb_identifier))
		goto error;
	} else if(orb_id_opt!=NULL) {
		if(!ORBit_ORBid_setup(orb, orb_id_opt))
			goto error;
	} else {
		orb->orb_identifier=g_strdup("orbit-local-orb");
	}

	if(orb->orb_identifier==NULL) {
		CORBA_exception_set_system(ev, ex_CORBA_NO_MEMORY, CORBA_COMPLETED_NO);
		goto error;
	}

	if(use_ipv4) {
		orb->cnx.ipv4 = GIOP_CONNECTION(iiop_connection_server());
		giop_connection_ref(orb->cnx.ipv4);
		GIOP_CONNECTION(orb->cnx.ipv4)->orb_data = orb;
	
	}

	if(use_ipv6) {
		orb->cnx.ipv6 = GIOP_CONNECTION(iiop_connection_server_ipv6());
		giop_connection_ref(orb->cnx.ipv6);
		GIOP_CONNECTION(orb->cnx.ipv6)->orb_data = orb;
	}

	ORBit_make_local_tmpdir();

	if(use_usock) {
		orb->cnx.usock = ORBit_ORB_make_usock_connection();

		giop_connection_ref(orb->cnx.usock);
		GIOP_CONNECTION(orb->cnx.usock)->orb_data = orb;
	}

	orb->objrefs = g_hash_table_new((GHashFunc)g_CORBA_Object_hash,
					(GCompareFunc)g_CORBA_Object_equal);
	orb->poas = g_ptr_array_new();

	/* when I figure out what MICO is doing with the iiop_proxy and
	 * iiop_server stuff, it'll get handled here.
	 */

	/*
	 * Connect to / create implementation repository
	 */

	{
		CORBA_Object imr=NULL;

		if(imr_ior!=NULL) {
			imr=CORBA_ORB_string_to_object(orb, imr_ior, ev);
			g_free(imr_ior);
		} else if(imr_addr!=NULL) {
			/*imr=CORBA_ORB_bind(orb, "IDL:omg.org/CORBA/ImplRepository:1.0", imr_addr, ev);*/
			g_free(imr_addr);
		}

		if(!CORBA_Object_is_nil(imr, ev)) {
			CORBA_ORB_set_initial_reference(orb, "ImplementationRepository", imr, ev);
		}
	}

	/*
	 * Connect to / create interface repository
	 */

	{
		CORBA_Object ir=NULL;

		if(ir_ior!=NULL) {
			ir=CORBA_ORB_string_to_object(orb, ir_ior, ev);
			g_free(ir_ior);
		} else if(ir_addr!=NULL) {
			/*ir=CORBA_ORB_bind(orb, "IDL:omg.org/CORBA/Repository:1.0", ir_addr, ev);*/
			g_free(ir_addr);
		}

		if(!CORBA_Object_is_nil(ir, ev)) {
			CORBA_ORB_set_initial_reference(orb, "InterfaceRepository", ir, ev);
		}
	}

	/*
	 * Connect to naming service
	 */

	{
		CORBA_Object naming=NULL;

		if(naming_ior!=NULL) {
			naming=CORBA_ORB_string_to_object(orb, naming_ior, ev);
			g_free(naming_ior);
		} else if(naming_addr!=NULL) {
			/*CORBA_ORB_ObjectTag tag=CORBA_ORB_string_to_tag(orb, "root", ev);*/

			/*naming=CORBA_ORB_bind_tag(orb, "IDL:omg.org/CosNaming/NamingContext:1.0", tag, naming_addr, ev);*/
			g_free(naming_addr);
		}

		if(!CORBA_Object_is_nil(naming, ev)) {
			CORBA_ORB_set_initial_reference(orb, "NameService", naming, ev);
		}
	}

	/*
	 * Connect to / create RootPOA
	 */

	{
		PortableServer_POA root_poa=CORBA_OBJECT_NIL;
	  
		if(root_poa_ior!=NULL) {
			root_poa=(PortableServer_POA)
				CORBA_ORB_string_to_object(orb,
							   root_poa_ior, ev);
			g_free(root_poa_ior);
		}
		       
		/* And attatch it to the orb */

		if(!CORBA_Object_is_nil((CORBA_Object)root_poa, ev)) {
			CORBA_ORB_set_initial_reference((CORBA_ORB)orb,
							"RootPOA",
							(CORBA_Object)root_poa,
							ev);
		}
	}

	ORBit_custom_run_setup(orb, ev);

	return (CORBA_ORB)CORBA_Object_duplicate((CORBA_Object)orb, ev);

error:
	if(orb!=NULL) {
		ORBit_ORB_release(orb, ev);
		orb = NULL;
	}
	g_free(imr_ior);
	g_free(imr_addr);
	g_free(ir_ior);
	g_free(ir_addr);
	g_free(naming_ior);
	g_free(naming_addr);
	g_free(root_poa_ior);
	g_free(root_poa_addr);
	g_free(orb_id_opt);

	return(NULL);
}

typedef struct {
	CORBA_Object obj;
	CDR_Codec *codec;
	gboolean emit_active;
} profile_user_data;

static void ORBit_emit_profile(gpointer item, gpointer userdata)
{
	ORBit_Object_info *profile=(ORBit_Object_info *)item;
	profile_user_data *data=(profile_user_data *)userdata;
	CORBA_Object obj=data->obj;
	CDR_Codec encaps_codec_d;
	CDR_Codec *codec=data->codec, *encaps = &encaps_codec_d;
	gboolean emit_active=data->emit_active;
	static const CORBA_octet iiopversion[] = {1,0};
	CORBA_octet codecbuf[2048];

	g_assert(obj!=NULL);
	g_assert(codec!=NULL);
	g_assert(profile!=NULL);

	if((profile == obj->active_profile) && (emit_active == FALSE))
		return;			/* we already did this one */

	switch(profile->profile_type) {
	case IOP_TAG_INTERNET_IOP:
		CDR_codec_init_static(encaps);
		encaps->buffer = codecbuf;
		encaps->release_buffer = CORBA_FALSE;
		encaps->buf_len = 2048;
		encaps->readonly = CORBA_FALSE;
		encaps->host_endian = encaps->data_endian = FLAG_ENDIANNESS;

		CDR_put_ulong(codec, IOP_TAG_INTERNET_IOP);
		CDR_put_octet(encaps, FLAG_ENDIANNESS);
		CDR_put_octets(encaps, (gpointer)iiopversion, sizeof(iiopversion));
		CDR_put_string(encaps, profile->tag.iopinfo.host);
		CDR_put_ushort(encaps, profile->tag.iopinfo.port);
		CDR_put_ulong(encaps, profile->object_key._length);
		CDR_put_octets(encaps, profile->object_key._buffer,
			       profile->object_key._length);
		CDR_put_ulong(codec, encaps->wptr);
		CDR_put_octets(codec, encaps->buffer, encaps->wptr);
		break;

	case IOP_TAG_ORBIT_SPECIFIC:
		CDR_codec_init_static(encaps);
		encaps->buffer = codecbuf;
		encaps->release_buffer = CORBA_FALSE;
		encaps->buf_len = 2048;
		encaps->readonly = CORBA_FALSE;
		encaps->host_endian = encaps->data_endian = FLAG_ENDIANNESS;

		CDR_put_ulong(codec, IOP_TAG_ORBIT_SPECIFIC);
		CDR_put_octet(encaps, FLAG_ENDIANNESS);
		CDR_put_octets(encaps, (gpointer)iiopversion, sizeof(iiopversion));
		CDR_put_string(encaps, profile->tag.orbitinfo.unix_sock_path);
		CDR_put_ushort(encaps, profile->tag.orbitinfo.ipv6_port);
		CDR_put_ulong(encaps, profile->object_key._length);
		CDR_put_octets(encaps, profile->object_key._buffer,
			profile->object_key._length);
		CDR_put_ulong(codec, encaps->wptr);
		CDR_put_octets(codec, encaps->buffer, encaps->wptr);
		break;

	default:
#ifdef PROFILE_DEBUG
		g_warning("Unknown tag %d", profile->profile_type);
#endif
		break;
	}
}

CORBA_char *CORBA_ORB_object_to_string(CORBA_ORB orb,
				       CORBA_Object obj,
				       CORBA_Environment *ev)
{
  int i;
  CDR_Codec codec_d;
  CDR_Codec *codec = &codec_d;
  CORBA_char *rc = NULL;
  CORBA_unsigned_long ntags;
  profile_user_data data;
  CORBA_octet codecbuf[2048];
  char *ctmp;

  g_return_val_if_fail(ev, NULL);
  o_return_val_if_fail(orb && obj, NULL);

  if(!obj || !orb) {
	  CORBA_exception_set_system(ev,
				     ex_CORBA_BAD_PARAM,
				     CORBA_COMPLETED_NO);
	  return NULL;
  }

  if(ORBIT_ROOT_OBJECT(obj)->is_pseudo_object) {
	  CORBA_exception_set_system(ev,
				     ex_CORBA_MARSHAL,
				     CORBA_COMPLETED_NO);
	  return NULL;
  }

  CDR_codec_init_static(codec);

  codec->buffer = codecbuf;
  codec->release_buffer = CORBA_FALSE;
  codec->buf_len = 2048;
  codec->readonly = CORBA_FALSE;
  codec->host_endian = codec->data_endian = FLAG_ENDIANNESS;

  CDR_put_octet(codec, FLAG_ENDIANNESS);

  CDR_put_string(codec, obj->object_id);
  ntags = g_slist_length(obj->profile_list);
  CDR_put_ulong(codec, ntags);

  data.obj=obj;
  data.codec=codec;
  data.emit_active=TRUE;
  if(obj->active_profile != NULL)
	  ORBit_emit_profile(obj->active_profile, &data); /* do this one first */

  data.emit_active=FALSE;
  g_slist_foreach(obj->profile_list, ORBit_emit_profile, &data);

  rc = CORBA_string_alloc(4 + (codec->wptr * 2) + 1);
  strcpy(rc, "IOR:");

#define hexdigit(n) (((n)>9)?(n+'a'-10):(n+'0'))

  for(i = 0, ctmp = rc + strlen("IOR:"); i < codec->wptr; i++) {
	  *(ctmp++) = hexdigit((((codec->buffer[i]) & 0xF0) >> 4));
	  *(ctmp++) = hexdigit(((codec->buffer[i]) & 0xF));
  }
  *ctmp = '\0';

  {
	  /* Debug check */
	  CORBA_Object obj;
	  CORBA_Environment myev;

	  CORBA_exception_init(&myev);

	  obj = CORBA_ORB_string_to_object(orb, rc, &myev);

	  if (CORBA_Object_is_nil(obj, &myev)) {
		  g_warning("Bug in %s, created bad IOR `%s'\n",
			    G_GNUC_FUNCTION, rc);
		  CORBA_free(rc);
		  return NULL;
	  }
			  
	  CORBA_Object_release(obj, &myev);
  }
  
  return rc;
}

/* Quote from the GNU libc manual:

   "If you try to allocate more storage than the machine can provide,
    you don't get a clean error message. Instead you get a fatal
    signal like the one you would get from an infinite recursion;
    probably a segmentation violation (see section Program Error
    Signals)."

   The man page claims alloca() returns NULL on failure; this appears
   to be a load of shit on Linux where you just get flaming death, but
   we check anyway in case other systems work that way.

   On Linux we check that the size is less than MAX_STACK_ALLOC

   Note that the CORBA_alloc() calls in here can still cause
   program abort, and really that should be fixed in a similar
   way since our lengths are coming in from unpredictable sources
   like files or the network.
*/

#define MAX_STACK_ALLOC 8192

CORBA_Object CORBA_ORB_string_to_object(CORBA_ORB orb, CORBA_char *str,
					CORBA_Environment *ev)
{
	GSList *profiles=NULL;
	CORBA_Object retval = NULL;
	CORBA_char *type_id;
	ORBit_Object_info *object_info;
	CDR_Codec codec_d, encaps_codec_d;
	CDR_Codec *codec = &codec_d, *encaps_codec = &encaps_codec_d;
	CORBA_octet *buffer, endian;
	int i, j;
	CORBA_unsigned_long len, seq_len, misclen;

	g_return_val_if_fail(ev, CORBA_OBJECT_NIL);
	o_return_val_if_fail(orb && str, CORBA_OBJECT_NIL);

	if(strncmp(str, "IOR:", 4)) {
		CORBA_exception_set_system(ev, ex_CORBA_MARSHAL,
					   CORBA_COMPLETED_NO);
		return(CORBA_OBJECT_NIL);
	}

	CDR_codec_init_static(codec);
	len = strlen(str);

	if((len % 2) || len <= 4) {
		CORBA_exception_set_system(ev, ex_CORBA_MARSHAL,
					   CORBA_COMPLETED_NO);
		return(CORBA_OBJECT_NIL);
	}

	codec->buf_len = (len-4)/2;
	buffer = alloca(codec->buf_len);

	codec->buffer=buffer;
	codec->release_buffer = CORBA_FALSE;
	codec->readonly = TRUE;

	for(j = 0, i = 4; i < len; i+=2) {
		buffer[j++] = HEXOCTET(str[i], str[i+1]);
	};

	CDR_get_octet(codec, &endian);

	codec->data_endian = endian;
	codec->host_endian = FLAG_ENDIANNESS;

	CDR_get_string_static(codec, &type_id);

	CDR_get_seq_begin(codec, &seq_len);

	for(i = 0; i < seq_len; i++) {
		IOP_ProfileId tag;

		object_info=g_new0(ORBit_Object_info, 1);

		if (!CDR_get_ulong(codec, &tag))
			goto error_out;

		switch(tag) {
		case IOP_TAG_INTERNET_IOP:
			if (!CDR_get_ulong(codec, &misclen))
				goto error_out;
			
			CDR_codec_init_static(encaps_codec);

			if (misclen > MAX_STACK_ALLOC)
				goto error_out;
			
			encaps_codec->buffer = alloca(misclen);
			if (encaps_codec->buffer == NULL)
				/* misclen was probably junk */
				goto error_out;
			
			encaps_codec->release_buffer = FALSE;
			if(!CDR_buffer_gets(codec, encaps_codec->buffer, misclen))
				goto error_out;

			encaps_codec->buf_len = misclen;
			encaps_codec->readonly = CORBA_TRUE;
			if(!CDR_get_octet(encaps_codec, &endian))
				goto error_out;
			encaps_codec->data_endian = endian;
			encaps_codec->host_endian = FLAG_ENDIANNESS;

			if (encaps_codec->data_endian > 1)
				goto error_out;
			
			object_info->profile_type = IOP_TAG_INTERNET_IOP;
			if(!CDR_get_octet(encaps_codec, &object_info->iiop_major))
				goto error_out;
			if(object_info->iiop_major != 1)
				goto error_out;
			if(!CDR_get_octet(encaps_codec, &object_info->iiop_minor))
				goto error_out;
			if(!CDR_get_string(encaps_codec, &object_info->tag.iopinfo.host))
				goto error_out;
			if(!CDR_get_ushort(encaps_codec, &object_info->tag.iopinfo.port))
				goto error_out;
			if(!CDR_get_seq_begin(encaps_codec, &object_info->object_key._length))
				goto error_out;

			object_info->object_key._maximum = 0;

			/* The POA gives out ORBit_alloc()d profiles, so we have to too */
			object_info->object_key._buffer = ORBit_alloc(object_info->object_key._length, NULL, NULL);
			if(!CDR_buffer_gets(encaps_codec, object_info->object_key._buffer,
					    object_info->object_key._length))
				goto error_out;

			ORBit_set_object_key(object_info);
			profiles=g_slist_append(profiles, object_info);
			break;

		case IOP_TAG_MULTIPLE_COMPONENTS:
			/* Just skip any multiple_components data, for now */
			if(!CDR_get_ulong(codec, &misclen))
				goto error_out;

			CDR_codec_init_static(encaps_codec);

			if (misclen > MAX_STACK_ALLOC)
				goto error_out;
			
			encaps_codec->buf_len = misclen;
			encaps_codec->buffer = alloca(misclen);
			if (encaps_codec->buffer == NULL)
				/* misclen was probably junk */
				goto error_out;

			encaps_codec->release_buffer = FALSE;
			encaps_codec->readonly = CORBA_TRUE;
			if(!CDR_buffer_gets(codec, encaps_codec->buffer, misclen))
				goto error_out;
			break;

		case IOP_TAG_ORBIT_SPECIFIC:
			if(!CDR_get_ulong(codec, &misclen))
				goto error_out;

			CDR_codec_init_static(encaps_codec);

			if (misclen > MAX_STACK_ALLOC)
				goto error_out;
			
			encaps_codec->buffer = alloca(misclen);
			if (encaps_codec->buffer == NULL)
				/* misclen was probably junk */
				goto error_out;
			
			encaps_codec->release_buffer = FALSE;
			if(!CDR_buffer_gets(codec, encaps_codec->buffer, misclen))
				goto error_out;

			encaps_codec->buf_len = misclen;
			encaps_codec->readonly = CORBA_TRUE;

			if(!CDR_get_octet(encaps_codec, &endian))
				goto error_out;

			encaps_codec->data_endian = endian;
			encaps_codec->host_endian = FLAG_ENDIANNESS;

			if (encaps_codec->data_endian > 1)
				goto error_out;
			
			object_info->profile_type=IOP_TAG_ORBIT_SPECIFIC;
			if(!CDR_get_octet(encaps_codec, &object_info->iiop_major))
				goto error_out;

			if(object_info->iiop_major != 1)
				goto error_out;
			if(!CDR_get_octet(encaps_codec, &object_info->iiop_minor))
				goto error_out;

			if(!CDR_get_string(encaps_codec, &object_info->tag.orbitinfo.unix_sock_path))
				goto error_out;

			if(!CDR_get_ushort(encaps_codec, &object_info->tag.orbitinfo.ipv6_port))
				goto error_out;
			if(!CDR_get_seq_begin(encaps_codec, &object_info->object_key._length))
				goto error_out;
			object_info->object_key._maximum = 0;

			/* The POA gives out ORBit_alloc()d profiles, so we have to too */
			object_info->object_key._buffer = ORBit_alloc(object_info->object_key._length, NULL, NULL);
			if(!CDR_buffer_gets(encaps_codec, object_info->object_key._buffer,
					    object_info->object_key._length))
				goto error_out;

			ORBit_set_object_key(object_info);
			profiles=g_slist_append(profiles, object_info);
			break;
		default:
#ifdef PROFILE_DEBUG
			g_warning("Unknown tag 0x%x", tag);
#endif

			/* Skip it */
			if(!CDR_get_ulong(codec, &misclen))
				goto error_out;

			CDR_codec_init_static(encaps_codec);

			if (misclen > MAX_STACK_ALLOC)
				goto error_out;
			
			encaps_codec->buf_len = misclen;
			encaps_codec->buffer = alloca(misclen);
			if (encaps_codec->buffer == NULL)
				/* misclen was probably junk */
				goto error_out;

			encaps_codec->release_buffer = FALSE;
			encaps_codec->readonly = CORBA_TRUE;
			if(!CDR_buffer_gets(codec, encaps_codec->buffer, misclen))
				goto error_out;

			break;
		}
	}

	return ORBit_create_object_with_info(profiles, type_id, orb, ev);

 error_out:

	if(object_info) {
		CORBA_free(object_info->object_key._buffer);
		g_free(object_info);
		ORBit_delete_profiles(profiles);
	}


	return retval;
}

/* Section 4.1.2 */
CORBA_boolean CORBA_ORB_get_service_information(CORBA_ORB orb, CORBA_ServiceType service_type, CORBA_ServiceInformation *service_information, CORBA_Environment *ev)
{
	g_assert(!"Not yet implemented");
	return(CORBA_FALSE);
}

CORBA_Current *CORBA_ORB_get_current(CORBA_ORB orb, CORBA_Environment *ev)
{
	g_return_val_if_fail(ev, NULL);
	o_return_val_if_fail(orb, NULL);

	/* XXX check this over */
	return (CORBA_Current *)GET_THREAD_DATA();
}

/* Section 4.5 */
CORBA_ORB_ObjectIdList *CORBA_ORB_list_initial_services(CORBA_ORB orb, CORBA_Environment *ev)
{
	static const char *services[] = {"RootPOA"};
	CORBA_ORB_ObjectIdList *list;

	g_return_val_if_fail(ev, NULL);
	o_return_val_if_fail(orb, NULL);

	list = (CORBA_ORB_ObjectIdList *)CORBA_sequence_octet__alloc();
	list->_maximum=list->_length= 1;
	list->_buffer = (CORBA_ORB_ObjectId *)services;
	CORBA_sequence_set_release((void *)list, CORBA_FALSE);

	/* defined reserved references are:
	 *	RootPOA
	 *	POACurrent
	 *	InterfaceRepository
	 *	NameService
	 *	TradingService
	 *	SecurityCurrent
	 *	TransactionCurrent
	 */

	return list;
}

/* Section 4.5
 *
 * raises InvalidName
 */
CORBA_Object CORBA_ORB_resolve_initial_references(CORBA_ORB orb, CORBA_ORB_ObjectId identifier, CORBA_Environment *ev)
{
	g_return_val_if_fail(ev, CORBA_OBJECT_NIL);
	o_return_val_if_fail(orb, CORBA_OBJECT_NIL);

	CORBA_exception_free(ev);

	if(!strcmp(identifier, "ImplementationRepository"))
		return CORBA_Object_duplicate(orb->imr, ev);
	else if(!strcmp(identifier, "InterfaceRepository"))
		return CORBA_Object_duplicate(orb->ir, ev);
	else if(!strcmp(identifier, "NameService"))
		return CORBA_Object_duplicate(orb->naming, ev);
	else if(!strcmp(identifier, "RootPOA")) {
		if(CORBA_Object_is_nil(orb->root_poa, ev)) {
			CORBA_Policy policybuf[1];
			CORBA_PolicyList policies  = {1,1,NULL,CORBA_FALSE};
			PortableServer_POAManager poa_mgr;
			policies._buffer = policybuf;
			/* The only non-default policy used by the RootPOA is IMPLICIT ACTIVATION */ 
			policies._buffer[0]= (CORBA_Policy)
				PortableServer_POA_create_implicit_activation_policy(NULL,
										     PortableServer_IMPLICIT_ACTIVATION,
										     ev);
			/* Create a poa manager */
			poa_mgr = ORBit_POAManager_new();
			poa_mgr->orb = orb;

			/* Create the root poa */
			orb->root_poa = (CORBA_Object)
				ORBit_POA_new(orb,
					      "RootPOA",
					      poa_mgr,
					      &policies,
					      ev);
			CORBA_Object_duplicate(orb->root_poa, ev);
			CORBA_Object_release((CORBA_Object)policies._buffer[0],ev);
		}

		return CORBA_Object_duplicate(orb->root_poa, ev);
	}

	/* throw user exception: InvalidName */
	CORBA_exception_set(ev,CORBA_USER_EXCEPTION,
			    ex_CORBA_ORB_InvalidName,
			    NULL);

	goto error;
error:
	return(NULL);
}

/* This is a MICO extension
 *
 * raises InvalidName
 */
void CORBA_ORB_set_initial_reference(CORBA_ORB orb, CORBA_ORB_ObjectId identifier, CORBA_Object obj, CORBA_Environment *ev)
{
	g_return_if_fail(ev);
	o_return_if_fail(orb && identifier && obj);

	if(!strcmp(identifier, "ImplementationRepository")) {
		if(!CORBA_Object_is_nil(orb->imr, ev)) {
			CORBA_Object_release(orb->imr, ev);
		}
		orb->imr=CORBA_Object_duplicate(obj, ev);
	} else if(!strcmp(identifier, "InterfaceRepository")) {
		if(!CORBA_Object_is_nil(orb->ir, ev)) {
			CORBA_Object_release(orb->ir, ev);
		}
		orb->ir=CORBA_Object_duplicate(obj, ev);
	} else if(!strcmp(identifier, "NameService")) {
		if(!CORBA_Object_is_nil(orb->naming, ev)) {
			CORBA_Object_release(orb->naming, ev);
		}
		orb->naming=CORBA_Object_duplicate(obj, ev);
	} else if(!strcmp(identifier, "RootPOA")) {
		if(!CORBA_Object_is_nil(orb->root_poa, ev)) {
			CORBA_Object_release(orb->root_poa, ev);
		}
		orb->root_poa=CORBA_Object_duplicate(obj, ev);
	} else {
		/* throw user exception: InvalidName */
		CORBA_exception_set(ev,CORBA_USER_EXCEPTION,ex_CORBA_ORB_InvalidName,NULL);	
		goto error;
	}

	return;
error:
	return;
}

/* Section 4.9.1 */
CORBA_boolean CORBA_ORB_work_pending(CORBA_ORB orb, CORBA_Environment *ev)
{
	g_assert(!"Not yet implemented");
	return(CORBA_FALSE);
}

/* Section 4.9.2 */
void CORBA_ORB_perform_work(CORBA_ORB orb, CORBA_Environment *ev)
{
	g_assert(!"Not yet implemented");
	return;
}

/* Section 4.9.4 */
void
CORBA_ORB_shutdown(CORBA_ORB orb,
		   CORBA_boolean wait_for_completion,
		   CORBA_Environment *ev)
{
	g_return_if_fail(ev);
	o_return_if_fail(orb);

	/* XXX implement on a per-ORB basis, and also
	   handle whatever wait_for_completion means */

	if(orb->cnx.ipv4)
		giop_connection_unref(orb->cnx.ipv4);
	if(orb->cnx.ipv6)
		giop_connection_unref(orb->cnx.ipv6);
	if(orb->cnx.usock)
		giop_connection_unref(orb->cnx.usock);

	giop_main_quit();
}

/* Section 4.9.3 */
/* CORBA_ORB_run is in server.c */

/* Section 4.7 */
CORBA_PolicyType
CORBA_Policy__get_policy_type(CORBA_Policy obj, CORBA_Environment *ev)
{
	g_return_val_if_fail(ev, 0);
	o_return_val_if_fail(obj, 0);

	return obj->policy_type;
}

/* Section 4.7 */
CORBA_Policy CORBA_Policy_copy(CORBA_Policy obj, CORBA_Environment *ev)
{
	g_return_val_if_fail(ev, CORBA_OBJECT_NIL);
	o_return_val_if_fail(obj, CORBA_OBJECT_NIL);

	ORBIT_ROOT_OBJECT_REF(obj);

	return obj;
}

/* Section 4.7
 *
 * raises CORBA_NO_PERMISSION
 */
void CORBA_Policy_destroy(CORBA_Policy obj, CORBA_Environment *ev)
{
	g_return_if_fail(ev);
	o_return_if_fail(obj);

	ORBIT_ROOT_OBJECT_UNREF(obj);
	if(ORBIT_ROOT_OBJECT(obj)->refs <= 0)
		ORBIT_ROOT_OBJECT_release(obj, ev);
}

/* Section 4.8.2 */
CORBA_Policy CORBA_DomainManager_get_domain_policy(CORBA_DomainManager obj, CORBA_PolicyType policy_type, CORBA_Environment *ev)
{
	g_return_val_if_fail(ev, CORBA_OBJECT_NIL);
	o_return_val_if_fail(obj, CORBA_OBJECT_NIL);

	g_assert(!"Not yet implemented");
	return(NULL);
}

/* Section 4.8.2 */
void CORBA_ConstructionPolicy_make_domain_manager(CORBA_ConstructionPolicy obj, CORBA_InterfaceDef object_type, CORBA_boolean constr_policy, CORBA_Environment *
ev)
{
	g_return_if_fail(ev);
	o_return_if_fail(obj && object_type);

	g_assert(!"Not yet implemented");
	return;
}

/* Section 4.2.8 */
CORBA_DomainManagerList *CORBA_Object_get_domain_managers(CORBA_Object obj, CORBA_Environment *ev)
{
	g_return_val_if_fail(ev, NULL);
	o_return_val_if_fail(obj, NULL);

	g_assert(!"Not yet implemented");
	return(NULL);
}

CORBA_TypeCode CORBA_ORB_create_struct_tc(CORBA_ORB obj, CORBA_RepositoryId id, CORBA_Identifier name, CORBA_StructMemberSeq members, CORBA_Environment *ev)
{
	CORBA_TypeCode tc;
	int i;

	tc=ORBIT_CHUNK_ALLOC(CORBA_TypeCode);
	if(tc == NULL)
	  goto tc_alloc_failed;

	tc->subtypes=g_new0(CORBA_TypeCode, members._length);
	if(tc->subtypes == NULL)
	  goto subtypes_alloc_failed;

	tc->subnames=g_new0(const char *, members._length);
	if(tc->subnames == NULL)
	  goto subnames_alloc_failed;

	tc->kind=CORBA_tk_struct;
	tc->name=g_strdup(name);
	tc->repo_id=g_strdup(id);
	tc->sub_parts=members._length;
	tc->length=members._length;

	for(i=0;i<members._length;i++) {
		CORBA_StructMember *mem=(CORBA_StructMember *)&(members._buffer[i]);

		g_assert(&(mem->type)!=NULL);

		tc->subtypes[i] = ORBIT_CHUNK_ALLOC(CORBA_TypeCode);
		memcpy(tc->subtypes[i], mem->type, (size_t)sizeof(struct CORBA_TypeCode_struct));
		tc->subnames[i]=g_strdup(mem->name);
	}

	return(tc);

 subnames_alloc_failed:
	g_free(tc->subtypes);
 subtypes_alloc_failed:
	ORBIT_CHUNK_FREE(CORBA_TypeCode, tc);
 tc_alloc_failed:
	CORBA_exception_set_system(ev, ex_CORBA_NO_MEMORY, CORBA_COMPLETED_NO);
	return NULL;
}

CORBA_TypeCode 
CORBA_ORB_create_union_tc(CORBA_ORB obj, CORBA_RepositoryId id,
			  CORBA_Identifier name,
			  CORBA_TypeCode discriminator_type,
			  CORBA_UnionMemberSeq members,
			  CORBA_Environment *ev)
{
	CORBA_TypeCode tc;
	int i;

	tc=ORBIT_CHUNK_ALLOC(CORBA_TypeCode);

	if(tc == NULL)
	  goto tc_alloc_failed;

	tc->discriminator = ORBIT_CHUNK_ALLOC(CORBA_TypeCode);

	if(tc->discriminator == NULL)
	  goto discriminator_alloc_failed;

	memcpy(tc->discriminator, discriminator_type, (size_t)sizeof(CORBA_TypeCode));

	tc->subtypes=g_new0(CORBA_TypeCode, members._length);
	if(tc->subtypes==NULL)
	  goto subtypes_alloc_failed;

	tc->subnames=g_new0(const char *, members._length);
	if(tc->subnames==NULL)
	  goto subnames_alloc_failed;

	tc->sublabels=g_new0(CORBA_any, members._length);
	if(tc->sublabels == NULL)
	  goto sublabels_alloc_failed;

	tc->kind=CORBA_tk_union;
	tc->name=g_strdup(name);
	tc->repo_id=g_strdup(id);
	tc->sub_parts=members._length;
	tc->length=members._length;
	tc->default_index=-1;

	for(i=0;i<members._length;i++) {
		CORBA_UnionMember *mem=(CORBA_UnionMember *)&(members._buffer[i]);

		g_assert(&(mem->label)!=NULL);
		memcpy(&(tc->sublabels[i]), &(mem->label), (size_t)sizeof(CORBA_any));
		g_assert(&(mem->type)!=NULL);
		tc->subtypes[i] = ORBIT_CHUNK_ALLOC(CORBA_TypeCode);
		memcpy(tc->subtypes[i], mem->type, (size_t)sizeof(struct CORBA_TypeCode_struct));
		tc->subnames[i]=g_strdup(mem->name);

		if(mem->label._type->kind==CORBA_tk_octet) {
			tc->default_index=i;
		}
	}

	return(tc);

sublabels_alloc_failed:
	g_free(tc->sublabels);
subnames_alloc_failed:
	g_free(tc->subtypes);
subtypes_alloc_failed:
	ORBIT_CHUNK_FREE(CORBA_TypeCode, tc->discriminator);
discriminator_alloc_failed:
	ORBIT_CHUNK_FREE(CORBA_TypeCode, tc);
 tc_alloc_failed:
	CORBA_exception_set_system(ev, ex_CORBA_NO_MEMORY, CORBA_COMPLETED_NO);
	return NULL;
}

CORBA_TypeCode CORBA_ORB_create_enum_tc(CORBA_ORB obj, CORBA_RepositoryId id, CORBA_Identifier name, CORBA_EnumMemberSeq members, CORBA_Environment *ev)
{
	CORBA_TypeCode tc;
	int i;

	tc = ORBIT_CHUNK_ALLOC(CORBA_TypeCode);
	if(tc == NULL)
	  goto tc_alloc_failed;

	tc->subnames=g_new0(const char *, members._length);
	if(tc->subnames==NULL)
	  goto subnames_alloc_failed;

	tc->kind = CORBA_tk_enum;
	tc->name = g_strdup(name);
	tc->repo_id = g_strdup(id);
	tc->sub_parts = members._length;
	tc->length = members._length;

	for(i=0;i<members._length;i++) {
		tc->subnames[i]=g_strdup(members._buffer[i]);
	}

	return(tc);

 subnames_alloc_failed:
	ORBIT_CHUNK_FREE(CORBA_TypeCode, tc);
 tc_alloc_failed:
	CORBA_exception_set_system(ev, ex_CORBA_NO_MEMORY, CORBA_COMPLETED_NO);
	return(NULL);
}

CORBA_TypeCode CORBA_ORB_create_alias_tc(CORBA_ORB obj, CORBA_RepositoryId id, CORBA_Identifier name, CORBA_TypeCode original_type, CORBA_Environment *ev)
{
	CORBA_TypeCode tc;

	tc = ORBIT_CHUNK_ALLOC(CORBA_TypeCode);
	if(tc==NULL)
	  goto tc_alloc_failed;
	
	/* Can't use chunks here, because it's sometimes an array. Doh! */
	tc->subtypes=g_new0(CORBA_TypeCode, 1);
	if(tc->subtypes==NULL)
	  goto subtypes_alloc_failed;

	tc->kind=CORBA_tk_alias;
	tc->name=g_strdup(name);
	tc->repo_id=g_strdup(id);
	tc->sub_parts=1;
	tc->length=1;

	tc->subtypes[0] = ORBIT_CHUNK_ALLOC(CORBA_TypeCode);
	memcpy(tc->subtypes[0], original_type, (size_t)sizeof(struct CORBA_TypeCode_struct));

	return(tc);
 subtypes_alloc_failed:
	ORBIT_CHUNK_FREE(CORBA_TypeCode, tc);
tc_alloc_failed:
	CORBA_exception_set_system(ev, ex_CORBA_NO_MEMORY, CORBA_COMPLETED_NO);
	return NULL;
}

CORBA_TypeCode CORBA_ORB_create_exception_tc(CORBA_ORB obj, CORBA_RepositoryId id, CORBA_Identifier name, CORBA_StructMemberSeq members, CORBA_Environment *ev)
{
	CORBA_TypeCode tc;
	int i;

	tc=ORBIT_CHUNK_ALLOC(CORBA_TypeCode);
	if(tc==NULL)
	  goto tc_alloc_failed;

	tc->subtypes=g_new0(CORBA_TypeCode, members._length);
	if(tc->subtypes==NULL)
	  goto subtypes_alloc_failed;

	tc->subnames=g_new0(const char *, members._length);
	if(tc->subnames==NULL)
	  goto subnames_alloc_failed;

	tc->kind=CORBA_tk_except;
	tc->name=g_strdup(name);
	tc->repo_id=g_strdup(id);
	tc->sub_parts=members._length;
	tc->length=members._length;

	for(i=0;i<members._length;i++) {
		CORBA_StructMember *mem=(CORBA_StructMember *)&(members._buffer[i]);

		g_assert(mem->type != NULL);
		tc->subtypes[i] = ORBIT_CHUNK_ALLOC(CORBA_TypeCode);
		memcpy(tc->subtypes[i], mem->type, (size_t)sizeof(struct CORBA_TypeCode_struct));
		tc->subnames[i]=g_strdup(mem->name);
	}

	return(tc);

 subnames_alloc_failed:
	g_free(tc->subtypes);
 subtypes_alloc_failed:
	ORBIT_CHUNK_FREE(CORBA_TypeCode, tc);
 tc_alloc_failed:
	CORBA_exception_set_system(ev, ex_CORBA_NO_MEMORY, CORBA_COMPLETED_NO);
	return(NULL);
}

CORBA_TypeCode CORBA_ORB_create_interface_tc(CORBA_ORB obj, CORBA_RepositoryId id, CORBA_Identifier name, CORBA_Environment *ev)
{
	CORBA_TypeCode tc;

	tc=ORBIT_CHUNK_ALLOC(CORBA_TypeCode);
	if(tc==NULL) {
		CORBA_exception_set_system(ev, ex_CORBA_NO_MEMORY,
					   CORBA_COMPLETED_NO);
		return(NULL);
	}

	tc->kind=CORBA_tk_objref;
	tc->name=g_strdup(name);
	tc->repo_id=g_strdup(id);

	return(tc);
}

CORBA_TypeCode CORBA_ORB_create_string_tc(CORBA_ORB obj, CORBA_unsigned_long bound, CORBA_Environment *ev)
{
	CORBA_TypeCode tc;

	tc=ORBIT_CHUNK_ALLOC(CORBA_TypeCode);
	if(tc==NULL) {
		CORBA_exception_set_system(ev, ex_CORBA_NO_MEMORY, CORBA_COMPLETED_NO);
		return(NULL);
	}

	tc->kind=CORBA_tk_string;
	tc->length=bound;

	return(tc);
}

CORBA_TypeCode CORBA_ORB_create_wstring_tc(CORBA_ORB obj, CORBA_unsigned_long bound, CORBA_Environment *ev)
{
	CORBA_TypeCode tc;

	tc=ORBIT_CHUNK_ALLOC(CORBA_TypeCode);
	if(tc==NULL) {
		CORBA_exception_set_system(ev, ex_CORBA_NO_MEMORY, CORBA_COMPLETED_NO);
		return(NULL);
	}

	tc->kind=CORBA_tk_wstring;
	tc->length=bound;

	return(tc);
}

CORBA_TypeCode CORBA_ORB_create_fixed_tc(CORBA_ORB obj, CORBA_unsigned_short digits, CORBA_short scale, CORBA_Environment *ev)
{
	CORBA_TypeCode tc;

	tc=ORBIT_CHUNK_ALLOC(CORBA_TypeCode);
	if(tc==NULL) {
		CORBA_exception_set_system(ev, ex_CORBA_NO_MEMORY, CORBA_COMPLETED_NO);
		return(NULL);
	}

	tc->kind=CORBA_tk_fixed;
	tc->digits=digits;
	tc->scale=scale;

	return(tc);
}

CORBA_TypeCode CORBA_ORB_create_sequence_tc(CORBA_ORB obj, CORBA_unsigned_long bound, CORBA_TypeCode element_type, CORBA_Environment *ev)
{
	CORBA_TypeCode tc;

	tc=ORBIT_CHUNK_ALLOC(CORBA_TypeCode);
	if(tc==NULL)
	  goto tc_alloc_failed;

	/* Can't use chunks here because we can only be sure of getting
	   one consecutive chunk from glib */
	tc->subtypes=g_new0(CORBA_TypeCode, 1);
	if(tc->subtypes==NULL)
	  goto subtypes_alloc_failed;

	tc->kind=CORBA_tk_sequence;
	tc->sub_parts=1;
	tc->length=bound;

	tc->subtypes[0] = ORBIT_CHUNK_ALLOC(CORBA_TypeCode);
	memcpy(tc->subtypes[0], element_type,
	       (size_t)sizeof(struct CORBA_TypeCode_struct));

	return(tc);

 subtypes_alloc_failed:
	ORBIT_CHUNK_FREE(CORBA_TypeCode, tc);
 tc_alloc_failed:
	CORBA_exception_set_system(ev, ex_CORBA_NO_MEMORY, CORBA_COMPLETED_NO);
	return(NULL);
}

CORBA_TypeCode CORBA_ORB_create_recursive_sequence_tc(CORBA_ORB obj, CORBA_unsigned_long bound, CORBA_unsigned_long offset, CORBA_Environment *ev)
{
	CORBA_TypeCode tc;

	tc=ORBIT_CHUNK_ALLOC(CORBA_TypeCode);
	if(tc==NULL)
	  goto tc_alloc_failed;

	tc->subtypes=g_new0(CORBA_TypeCode, 1);
	if(tc->subtypes==NULL)
	  goto subtypes_alloc_failed;

	tc->kind=CORBA_tk_sequence;
	tc->sub_parts=1;
	tc->length=bound;

	tc->subtypes[0] = ORBIT_CHUNK_ALLOC(CORBA_TypeCode);
	tc->subtypes[0]->kind=CORBA_tk_recursive;
	tc->subtypes[0]->recurse_depth=offset;

	return(tc);

 subtypes_alloc_failed:
	ORBIT_CHUNK_FREE(CORBA_TypeCode, tc);
 tc_alloc_failed:
	CORBA_exception_set_system(ev, ex_CORBA_NO_MEMORY, CORBA_COMPLETED_NO);
	return(NULL);
}

CORBA_TypeCode CORBA_ORB_create_array_tc(CORBA_ORB obj, CORBA_unsigned_long length, CORBA_TypeCode element_type, CORBA_Environment *ev)
{
	CORBA_TypeCode tc;

	tc=ORBIT_CHUNK_ALLOC(CORBA_TypeCode);
	if(tc==NULL)
	  goto tc_alloc_failed;

	tc->subtypes=g_new0(CORBA_TypeCode, 1);
	if(tc->subtypes==NULL)
	  goto subtypes_alloc_failed;

	tc->kind=CORBA_tk_array;
	tc->sub_parts=1;
	tc->length=length;

	tc->subtypes[0] = ORBIT_CHUNK_ALLOC(CORBA_TypeCode);
	memcpy(tc->subtypes[0], element_type, (size_t)sizeof(CORBA_TypeCode));

	return(tc);

 subtypes_alloc_failed:
	ORBIT_CHUNK_FREE(CORBA_TypeCode, tc);
 tc_alloc_failed:
	CORBA_exception_set_system(ev, ex_CORBA_NO_MEMORY, CORBA_COMPLETED_NO);
	return(NULL);
}
