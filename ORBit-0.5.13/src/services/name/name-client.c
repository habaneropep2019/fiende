/* 
 *  name-client : a client for the CORBA CosNaming Service
 *
 *  Copyright (C) 1998 Sebastian Wilhelmi; University of Karlsruhe
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "CosNaming.h"
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>

/* how many bindings to fetch at once */
#define FETCH_AT_ONCE 10

#define IS_EXCEPTION(ev, name)						\
  (strcmp (CORBA_exception_id (ev), ex_##name) == 0)

typedef enum
  {
    mode_bind, mode_rebind, mode_bind_context,
    mode_rebind_context, mode_unbind, mode_resolve,
    mode_new_context, mode_bind_new_context, mode_destroy,
    mode_list, mode_not_set
  }
mode_type;

typedef struct
  {
    mode_type type;
    gchar *name;
    int num_of_args;
  }
modes_array_type;

static modes_array_type modes[] =
{
  {
    mode_bind, "bind", 2
  }
  ,
  {
    mode_rebind, "rebind", 2
  }
  ,
  {
    mode_bind_context, "bind_context", 2
  }
  ,
  {
    mode_rebind_context, "rebind_context", 2
  }
  ,
  {
    mode_unbind, "unbind", 1
  }
  ,
  {
    mode_resolve, "resolve", 1
  }
  ,
  {
    mode_new_context, "new_context", 0
  }
  ,
  {
    mode_bind_new_context, "bind_new_context", 1
  }
  ,
  {
    mode_destroy, "destroy", 1
  }
  ,
  {
    mode_list, "list", 1
  }
};

static gchar *requested_name = NULL;

static void
Usage (gchar * name)
{
  g_printerr
    ("Usage:\n"
     "  %s root [bind|rebind|bind_context|rebind_context] name object-id\n"
     "  %s root [resolve|unbind|bind_new_context|list|destroy] name\n"
     "  %s root [new_context]\n"
     "    components of name are delimited by '/'\n"
     "    id and kind of those components are delimited by '.'\n"
     "    root denotes the IOR of the root naming context, it is either:\n"
     "       * the IOR itself\n"
     "       * '--stdin' - the IOR is read from stdin\n"
     "       * '--gnome' - the Gnome Name Server is used\n"
     "    object-id is the IOR of an object\n", name, name, name);
  exit (1);
}

static void
Exception (CORBA_Environment * ev)
{
  switch (ev->_major)
    {
    case CORBA_SYSTEM_EXCEPTION:
      g_printerr ("CORBA system exception.\n");
      exit (1);
    case CORBA_USER_EXCEPTION:
      if (IS_EXCEPTION (ev, CosNaming_NamingContext_NotFound))
	{
	  CosNaming_NamingContext_NotFound *ex = CORBA_exception_value (ev);
	  g_printerr ("'%s' can't be found, because '%s%s%s' is ",
		      requested_name,
		      ex->rest_of_name._buffer->id,
		      strlen (ex->rest_of_name._buffer->kind) > 0 ? "." : "",
		      ex->rest_of_name._buffer->kind);
	  switch (ex->why)
	    {
	    case CosNaming_NamingContext_missing_node:
	      g_printerr ("missing");
	      break;
	    case CosNaming_NamingContext_not_context:
	      g_printerr ("not a context");
	      break;
	    case CosNaming_NamingContext_not_object:
	      g_printerr ("not an object");
	      break;
	    }
	  g_printerr (".\n");
	}
      else if (IS_EXCEPTION (ev, CosNaming_NamingContext_CannotProceed))
	{
	  CosNaming_NamingContext_CannotProceed *ex =
	  CORBA_exception_value (ev);
	  g_printerr ("The name service implementation couldn't proceed"
		      " while processing");
	  g_printerr ("%s%s%s.\n",
		      ex->rest_of_name._buffer->id,
		      strlen (ex->rest_of_name._buffer->kind) > 0 ? "." : "",
		      ex->rest_of_name._buffer->kind);
	}
      else if (IS_EXCEPTION (ev, CosNaming_NamingContext_InvalidName))
	{
	  g_printerr ("'%s' has an invalid name.\n", requested_name);
	}
      else if (IS_EXCEPTION (ev, CosNaming_NamingContext_AlreadyBound))
	{
	  g_printerr ("'%s' is already bound.\n", requested_name);
	}
      else if (IS_EXCEPTION (ev, CosNaming_NamingContext_NotEmpty))
	{
	  g_printerr ("'%s' can't be deleted as it is not empty.\n",
		      requested_name);
	}
      else
	{
	  g_printerr ("Unknown CORBA user exception: %s.\n",
		      CORBA_exception_id (ev));
	}

      exit (1);
    default:
      break;
    }
}

static void
assert_NamingContext (CORBA_Object object, gchar * name)
{
  CORBA_Environment ev;
  gboolean is_context;

  CORBA_exception_init (&ev);

  is_context =
    CORBA_Object_is_a (object, "IDL:omg.org/CosNaming/NamingContext:1.0", &ev);

  Exception (&ev);
  if (!is_context)
    {
      g_printerr ("'%s' is not a naming context.\n", name);
      exit (1);
    }
}

static CORBA_Object
resolve (CosNaming_NamingContext context,
	 CosNaming_Name * name,
	 CORBA_Environment * ev)
{
  CORBA_Object result;
  if (name->_length != 0)
    {
      result = CosNaming_NamingContext_resolve (context, name, ev);
      Exception (ev);
    }
  else
    {
      result = CORBA_Object_duplicate (context, ev);
      Exception (ev);
    }
  return result;
}

int
main (int argc, char *argv[])
{
  CORBA_ORB orb;
  CORBA_Environment ev;

  guchar objref_str[2048];
  CORBA_Object object = CORBA_OBJECT_NIL, outobject = CORBA_OBJECT_NIL;

  int i;
  mode_type mode = mode_not_set;

  CosNaming_NamingContext NamingContext_obj;

  CosNaming_Name *name = NULL;

  if (argc < 3)
    {
      Usage (argv[0]);
    }

  for (i = 0; i < sizeof (modes) / sizeof (modes[0]); i++)
    {
      if (strcmp (argv[2], modes[i].name) == 0)
	{
	  mode = modes[i].type;
	  if (argc != modes[i].num_of_args + 3)
	    {
	      fprintf (stderr, "wrong number of arguments for mode %s, should"
		       " be %d.\n", modes[i].name, modes[i].num_of_args);
	      Usage (argv[0]);
	    }
	  break;
	}
    }

  if (mode == mode_not_set)
    {
      fprintf (stderr, "mode %s not supported.\n", argv[1]);
      Usage (argv[0]);
    }

  CORBA_exception_init (&ev);

  orb = CORBA_ORB_init (&argc, argv, "orbit-local-orb", &ev);

  if (strcmp (argv[1], "--gnome") == 0)
    {
      guchar principal_str[2048];
      CORBA_Principal request_cookie;

      FILE *infile = popen ("xprop -root GNOME_NAME_SERVER |"
			    "awk '{print $5}' |"
			    "xargs xprop GNOME_NAME_SERVER_IOR -id |"
			    "sed 's/^[^\"]*\"//;s/\"//'", "r");
      fgets (objref_str, 2048, infile);
      pclose (infile);

      infile = popen ("xprop -root GNOME_SESSION_CORBA_COOKIE |"
		      "sed 's/^[^\"]*\"//;s/\"$//;s:\\\\\\(.\\):\\1:g'", "r");
      fgets (principal_str, 2048, infile);
      pclose (infile);

      g_strchug (principal_str);
      g_strchomp (principal_str);

      request_cookie._buffer = principal_str;
      request_cookie._length = strlen (principal_str) + 1;

      ORBit_set_default_principal (&request_cookie); 
    }
  else if (strcmp (argv[1], "--stdin") == 0)
    fgets (objref_str, 2048, stdin);
  else if (strncmp (argv[1], "IOR:", 4) == 0)
    strcpy (objref_str, argv[1]);
  else
    Usage (argv[0]);

  g_strchug (objref_str);
  g_strchomp (objref_str);

  g_assert (!(strlen (objref_str) % 2));

  NamingContext_obj = CORBA_ORB_string_to_object (orb, objref_str, &ev);
  Exception (&ev);

  assert_NamingContext (NamingContext_obj, "root context");

  if (argc > 3)			/* we have a name on argv[ 3 ] */
    {
      requested_name = argv[3];
      name = ORBit_string_to_CosNaming_Name (requested_name, &ev);
      Exception (&ev);
    }
  if (argc > 4)			/* we have an object-id on argv[ 3 ] */
    {
      object = CORBA_ORB_string_to_object (orb, argv[4], &ev);
      Exception (&ev);
    }
  switch (mode)
    {
    case mode_bind:
      CosNaming_NamingContext_bind (NamingContext_obj, name,
				    object, &ev);
      break;
    case mode_rebind:
      CosNaming_NamingContext_rebind (NamingContext_obj, name,
				      object, &ev);
      break;
    case mode_bind_context:
      CosNaming_NamingContext_bind_context (NamingContext_obj, name,
					    object, &ev);
      break;
    case mode_rebind_context:
      CosNaming_NamingContext_rebind_context (NamingContext_obj, name,
					      object, &ev);
      break;
    case mode_resolve:
      outobject = resolve (NamingContext_obj, name, &ev);
      break;
    case mode_unbind:
      CosNaming_NamingContext_unbind (NamingContext_obj, name, &ev);
      break;
    case mode_bind_new_context:
      outobject = CosNaming_NamingContext_bind_new_context (NamingContext_obj,
							    name, &ev);
      break;
    case mode_new_context:
      outobject = CosNaming_NamingContext_new_context (NamingContext_obj,
						       &ev);
      break;
    case mode_destroy:
      {
	CosNaming_NamingContext remove_context = resolve (NamingContext_obj,
							  name, &ev);
	Exception (&ev);

	assert_NamingContext (remove_context, requested_name);

	CosNaming_NamingContext_destroy (remove_context, &ev);
	Exception (&ev);
	CORBA_Object_release (remove_context, &ev);
      }
      break;
    case mode_list:
      {
	CosNaming_BindingIterator bi;
	CosNaming_BindingList *bl;

	CosNaming_NamingContext list_context = resolve (NamingContext_obj,
							name, &ev);

	assert_NamingContext (list_context, requested_name);

	CosNaming_NamingContext_list (list_context, FETCH_AT_ONCE,
				      &bl, &bi, &ev);
	Exception (&ev);

	CORBA_Object_release (list_context, &ev);
	Exception (&ev);

	while (TRUE)
	  {
	    int i;
	    for (i = 0; i < bl->_length; i++)
	      {
		CosNaming_NameComponent *name =
		&bl->_buffer[i].binding_name._buffer[0];
		printf ("%s%s%s%s\n",
			name->id,
			strlen (name->kind) > 0 ? "." : "",
			name->kind,
			bl->_buffer[i].binding_type == CosNaming_ncontext ?
			"/" : "");
	      }
	    CORBA_free (bl);
	    if (CORBA_Object_is_nil (bi, &ev))
	      {
		break;
	      }
	    if (!CosNaming_BindingIterator_next_n (bi, FETCH_AT_ONCE,
						   &bl, &ev))
	      {
		Exception (&ev);
		CosNaming_BindingIterator_destroy (bi, &ev);
		Exception (&ev);
		CORBA_Object_release (bi, &ev);
		bi = CORBA_OBJECT_NIL;
	      }
	    Exception (&ev);
	  }
      }
      break;
    default:
      break;
    }
  Exception (&ev);

  if (!CORBA_Object_is_nil (outobject, &ev))
    {
      CORBA_char *object_str = CORBA_ORB_object_to_string (orb, outobject, &ev);
      printf ("%s\n", object_str);
      CORBA_free (object_str);
      CORBA_Object_release (outobject, &ev);
    }

  if (!CORBA_Object_is_nil (object, &ev))
    {
      CORBA_Object_release (object, &ev);
    }

  if (name)
    {
      CORBA_free (name);
    }

  CORBA_Object_release (NamingContext_obj, &ev);
  Exception (&ev);

  /* g_mem_profile(); */

  return 0;
}
