#include "LName.h"
#include <string.h>

#define THROW(name,arg,ev) CORBA_exception_set( ev, CORBA_USER_EXCEPTION, \
                                                ex_##name, arg )


typedef struct _RealLNameComponent RealLNameComponent;
struct _RealLNameComponent
  {
    CORBA_char *id;
    CORBA_char *kind;
  };

LNameComponent 
ORBit_string_to_LNameComponent(const CORBA_char* string, 
			       CORBA_Environment * ev)
{
  RealLNameComponent* result = g_new0 (RealLNameComponent, 1);
  gchar **tmparray;

  tmparray = g_strsplit(string, ".", 2);

  result->id = CORBA_string_dup(tmparray[0]);
  result->kind = CORBA_string_dup(tmparray[1]?tmparray[1]:"");

  g_strfreev(tmparray);

  return result;
}

LName 
ORBit_string_to_LName(const CORBA_char* string, CORBA_Environment * ev)
{
  GPtrArray *array = g_ptr_array_new ();
  gchar** vector = g_strsplit( string, "/", 0 );
  int curr_pos;
  LNameComponent component;
  for( curr_pos = 0; vector[ curr_pos ]; curr_pos++ )
    {
      if( strlen( vector[ curr_pos ] ) > 0 )
	{
	  component = ORBit_string_to_LNameComponent( vector[ curr_pos ], ev );
	  g_ptr_array_set_size (array, array->len + 1);
	  g_ptr_array_index( array, array->len - 1 ) = component;
	}
    }  
  g_strfreev( vector );
  return array;
}

CosNaming_Name* 
ORBit_string_to_CosNaming_Name(const CORBA_char* string, 
			       CORBA_Environment * ev)
{
  LName lname = ORBit_string_to_LName( string, ev );
  CosNaming_Name* result = LName_to_idl_form( lname, ev );
  LName_destroy( lname, ev );
  return result;
}

LNameComponent
create_lname_component ()
{
  return g_new0 (RealLNameComponent, 1);
}

CORBA_char *
LNameComponent_get_id (LNameComponent component,
		       CORBA_Environment * ev)
{
  RealLNameComponent *real = component;
  if (!real->id)
    {
      THROW (LNameComponent_NotSet, NULL, ev);
      return NULL;
    }
  return CORBA_string_dup (real->id);
}

void
LNameComponent_set_id (LNameComponent component,
		       CORBA_char * i,
		       CORBA_Environment * ev)
{
  RealLNameComponent *real = component;
  CORBA_free (real->id);
  real->id = CORBA_string_dup (i);
}

CORBA_char *
LNameComponent_get_kind (LNameComponent component,
			 CORBA_Environment * ev)
{
  RealLNameComponent *real = component;
  if (!real->kind)
    {
      THROW (LNameComponent_NotSet, NULL, ev);
      return NULL;
    }
  return CORBA_string_dup (real->kind);
}

void
LNameComponent_set_kind (LNameComponent component,
			 CORBA_char * k,
			 CORBA_Environment * ev)
{
  RealLNameComponent *real = component;
  CORBA_free (real->kind);
  real->kind = CORBA_string_dup (k);
}

void
LNameComponent_destroy (LNameComponent component,
			CORBA_Environment * ev)
{
  RealLNameComponent *real = component;
  CORBA_free (real->id);
  CORBA_free (real->kind);
  g_free (real);
}

LName
create_lname ()
{
  return g_ptr_array_new ();
}

/* insert LNameComponent n on position i [1...length(name)+1] into
  LName name, moving all subsequent LNameComponents back one place. if
  i is bigger then the number of LNameComponents in name + 1, or if i
  is smaller then 1, LName_NoComponent is thrown. If you insert
  component, ownership is handed over to name. so do not destroy
  it. */

LName
LName_insert_component (LName name,
			CORBA_unsigned_long i,
			LNameComponent n,
			CORBA_Environment * ev)
{
  GPtrArray *array = name;
  int pos = i - 1;		/* just to make it clear */
  if (pos < 0 || pos >= array->len + 1)
    {
      THROW (LName_NoComponent, NULL, ev);
      return name;
    }
  g_ptr_array_set_size (array, array->len + 1);
  if (pos != array->len - 1)
    {
      g_memmove( &g_ptr_array_index( array, pos + 1),
		 &g_ptr_array_index( array, pos ),
		 (array->len - 1 - pos) * sizeof(gpointer) );
    }
  g_ptr_array_index( array, pos ) = n;
  return name;
}

/* return the LNameComponent on position i [1...length(name)].
   Ownership remains at name, do not destroy. Changed to the returned
   LNameComponent are effective in LName name too. */

LNameComponent
LName_get_component (LName name,
		     CORBA_unsigned_long i,
		     CORBA_Environment * ev)
{
  GPtrArray *array = name;
  int pos = i - 1;		/* just to make it clear */
  if (pos < 0 || pos >= array->len)
    {
      THROW (LName_NoComponent, NULL, ev);
      return NULL;
    }
  return (LNameComponent) g_ptr_array_index( array, pos );
}

/* remove and return the LNameComponent on position i
   [1....length(name)]. Ownership is transferred to the calling
   routine, so destroy it properly. */

LNameComponent
LName_delete_component (LName name,
			CORBA_unsigned_long i,
			CORBA_Environment * ev)
{
  GPtrArray *array = name;
  LNameComponent *result;
  int pos = i - 1;		/* just to make it clear */
  if (pos < 0 || pos >= array->len)
    {
      THROW (LName_NoComponent, NULL, ev);
      return NULL;
    }
  result = (LNameComponent) g_ptr_array_index( array, pos );
  if (pos != array->len - 1)
    {
      g_memmove( &g_ptr_array_index( array, pos ),
		 &g_ptr_array_index( array, pos + 1),
		 (array->len - 1 - pos) * sizeof(gpointer) );
    }
  g_ptr_array_set_size (array, array->len - 1);
  return result;
}

CORBA_unsigned_long
LName_num_components (LName name,
		      CORBA_Environment * ev)
{
  GPtrArray *array = name;
  return array->len;
}

static
int
LName_compare (GPtrArray * first,
	       GPtrArray * second)
{
  int i;
  int len = MIN (first->len, second->len);
  for (i = 0; i < len; i++)
    {
      RealLNameComponent *first_component = g_ptr_array_index (first, i);
      RealLNameComponent *second_component = g_ptr_array_index (second, i);
      int result = strcmp (first_component->id, second_component->id);
      if (result != 0)
	{
	  return result;
	}
      result = strcmp (first_component->kind, second_component->kind);
      if (result != 0)
	{
	  return result;
	}
    }
  return first->len - second->len;
}


CORBA_boolean
LName_equal (LName name,
	     LName ln,
	     CORBA_Environment * ev)
{
  return LName_compare ( name, ln) == 0 ? CORBA_TRUE : CORBA_FALSE;
}

CORBA_boolean
LName_less_than (LName name,
		 LName ln,
		 CORBA_Environment * ev)
{
  return LName_compare ( name, ln) < 0 ? CORBA_TRUE : CORBA_FALSE;
}

CosNaming_Name *
LName_to_idl_form (LName name,
		   CORBA_Environment * ev)
{
  GPtrArray *array = name;
  int len = array->len;
  CosNaming_Name *result = CosNaming_Name__alloc ();
  int i;

  result->_buffer = CORBA_sequence_CosNaming_NameComponent_allocbuf (len);
  result->_length = len;
  for (i = 0; i < len; i++)
    {
      RealLNameComponent *component = g_ptr_array_index(array, i);
      result->_buffer[i].id =
	CORBA_string_dup (component->id != 0 ? component->id : "");
      result->_buffer[i].kind =
	CORBA_string_dup (component->kind != 0 ? component->kind : "");
    }
  return result;
}

static void
LName_free_components (GPtrArray * array,
		       CORBA_Environment * ev)
{
  int i;

  for (i = 0; i < array->len; i++)
    {
      LNameComponent_destroy( g_ptr_array_index(array, i), ev );
    }
}

void
LName_from_idl_form (LName name,
		     CosNaming_Name * n,
		     CORBA_Environment * ev)
{
  GPtrArray *array = name;
  int i;

  LName_free_components (array,ev);
  g_ptr_array_set_size (array, n->_length);
  for (i = 0; i < array->len; i++)
    {
      LNameComponent component = create_lname_component();
      LNameComponent_set_id (component, n->_buffer[i].id, ev);
      LNameComponent_set_kind (component, n->_buffer[i].kind, ev);
      g_ptr_array_index (array, i) = component;
    }
}

void
LName_destroy (LName name,
	       CORBA_Environment * ev)
{
  LName_free_components (name,ev);
  g_ptr_array_free (name, TRUE);
}
