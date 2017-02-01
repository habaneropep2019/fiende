#include "orbit.h"
#include "sequences.h"

gpointer CORBA_sequence_octet_free(gpointer mem,
				   gpointer func_data)
{
  CORBA_sequence_octet *seqo = mem;

  if(seqo->_release)
    CORBA_free(seqo->_buffer);

  return (gpointer)((guchar *)mem + sizeof(CORBA_sequence_octet));
}

CORBA_octet *
CORBA_octet_allocbuf(CORBA_unsigned_long len)
{
  return (CORBA_octet *)ORBit_alloc(len, NULL, NULL);
}

CORBA_sequence_octet *CORBA_sequence_octet__alloc(void)
{
  CORBA_sequence_octet *seqo;

  seqo = ORBit_alloc(sizeof(CORBA_sequence_octet),
		     (ORBit_free_childvals)CORBA_sequence_octet_free,
		     GUINT_TO_POINTER(1));

  seqo->_length = seqo->_maximum = 0;
  seqo->_buffer = NULL;
  seqo->_release = CORBA_TRUE;

  return seqo;
}



gpointer
CORBA_sequence_CORBA_any__free(gpointer mem, gpointer dat,
			       CORBA_boolean free_strings)
{
   CORBA_sequence_CORBA_any *val = mem;

   if (val->_release)
      ORBit_free(val->_buffer, free_strings);
   return (gpointer) (val + 1);
}

CORBA_sequence_CORBA_any *
CORBA_sequence_CORBA_any__alloc(void)
{
   CORBA_sequence_CORBA_any *retval;

   retval =
      ORBit_alloc(sizeof(CORBA_sequence_CORBA_any),
		  (ORBit_free_childvals) CORBA_sequence_CORBA_any__free,
		  GUINT_TO_POINTER(1));
   retval->_maximum = 0;
   retval->_length = 0;
   retval->_buffer = NULL;
   retval->_release = CORBA_FALSE;
   return retval;
}

CORBA_any *
CORBA_sequence_CORBA_any_allocbuf(CORBA_unsigned_long len)
{
   CORBA_any *retval =
      ORBit_alloc(sizeof(CORBA_any) * len,
		  (ORBit_free_childvals) CORBA_any__free,

		  GUINT_TO_POINTER(len));
   memset(retval, '\0', sizeof(CORBA_any) * len);
   return retval;
}
