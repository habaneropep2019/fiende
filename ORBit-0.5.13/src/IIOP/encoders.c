#include "config.h"
#include <string.h>
#include "IIOP.h"

ENCODER_DEC(IOP_ServiceContext)
{
  APA(&mem->context_id, sizeof(mem->context_id));
  ENCODER_CALL(CORBA_sequence_octet, &mem->context_data);
}

ENCODER_DEC(IOP_ServiceContextList)
{
  int i;

  if(!mem)
    {
      APA((gpointer)giop_scratch_space, sizeof(mem->_length));
      return;
    }

  APA(&mem->_length, sizeof(mem->_length));

  for(i = 0; i < mem->_length; i++)
    ENCODER_CALL(IOP_ServiceContext, &mem->_buffer[i]);
}

ENCODER_DEC(CORBA_sequence_octet)
{
  if(!mem)
    {
      APA((gpointer)giop_scratch_space, sizeof(mem->_length));
      return;
    }

  APIA(&mem->_length, sizeof(mem->_length));
  if(mem->_length > 0)
    AP(mem->_buffer, mem->_length);
}

ENCODER_DEC(CORBA_char)
{
  GIOP_unsigned_long len = strlen(mem) + 1;

  APIA(&len, sizeof(len));
  AP(mem, len);
}
