/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

/* By Elliot Lee. Copyright (c) 1998 Red Hat Software */

#include "orbit.h"

#if 0
#define CORBA_Object_release(x, y) ({ g_message(__FILE__ ":%d Releasing object %#x from %d", __LINE__, \
x, ORBIT_ROOT_OBJECT(x)->refs); CORBA_Object_release(x, y); })
#define CORBA_Object_duplicate(x, y) ({ g_message(__FILE__ ":%d Duping object %#x from %d", __LINE__, \
x, ORBIT_ROOT_OBJECT(x)->refs); CORBA_Object_duplicate(x, y); })
#endif

#define FREE_MARKER_IS_ALLOCATED 0xfefefefe

/* The memory chunk stuff */

#define ALLOCATOR_DEFINITION
#include "allocator-defs.h"
#undef ALLOCATOR_DEFINITION

void
ORBit_chunks_init(void)
{
#define ALLOCATOR_INITIALIZATION
#include "allocator-defs.h"
#undef ALLOCATOR_INTIALIZATION
}

gpointer
ORBit_chunk_alloc(GMemChunk *chunk,
		  PARAM_LOCK(chunk))
{
  gpointer retval;

  GET_LOCK(chunk);
  retval = g_mem_chunk_alloc(chunk);
  RELEASE_LOCK(chunk);

  return retval;
}

void
ORBit_chunk_free(GMemChunk *chunk,
		 PARAM_LOCK(chunk),
		 gpointer mem)
{
  GET_LOCK(chunk);
  g_mem_chunk_free(chunk, mem);
  RELEASE_LOCK(chunk);
}

/* end memory chunk routines */

/****************************************************************/

/************* begin funky memory alloc/free system */

/****** functions */

gpointer ORBit_alloc(size_t block_size,
		     ORBit_free_childvals freefunc,
		     gpointer func_data)
{
	return ORBit_alloc_2(block_size, freefunc, func_data, 0);
}

gpointer
ORBit_alloc_2(size_t block_size,
	      ORBit_free_childvals freefunc,
	      gpointer func_data,
	      size_t before_size)
{
	ORBit_mem_info *block;

	if(block_size == 0) return NULL;

	block = (ORBit_mem_info *)((char *)
		g_malloc(block_size + sizeof(ORBit_mem_info) + before_size)
		+ before_size);

#ifdef ORBIT_DEBUG
	block->magic = 0xdeadbeef;
#endif
	block->free = freefunc;
	block->func_data = func_data;
	block->u.free_marker= FREE_MARKER_IS_ALLOCATED;

	return MEMINFO_TO_PTR(block);
}

/*
  ORBit_free
  ----------

  Frees a corba primitive type.

  mem = pointer to the memory block. (must have a preceeding pointer to a meminfo block)

  1)obtains a pointer to the preceeding meminfo structure
  2)Uses the meminfo structure to find the number of elements in the memory block
  3)iterates through the memory block, calling the free function for each item.
  
 */

void
ORBit_free(gpointer mem, CORBA_boolean ignore)
{
	ORBit_mem_info *block;

	if (!mem)
		return;

	block = PTR_TO_MEMINFO (mem);

	/* This block has not been allocated by CORBA_alloc. Instead
         * it is memory in the receive buffer, that has been used in
         * demarshalling. Let's just return. */
	if (block->u.free_marker != FREE_MARKER_IS_ALLOCATED)
		return;

#ifdef ORBIT_DEBUG
	g_assert (block->magic == 0xdeadbeef);
#endif

	if (block->free) {
		int      i;
		gpointer x;
		gpointer my_data;

		if ((gpointer) block->free == (gpointer) ORBit_free_via_TypeCode)
			my_data = ((guchar *) block) - sizeof(CORBA_TypeCode);
		else
			my_data = NULL;

#ifdef ORBIT_DEBUG
		if (block->func_data == NULL)
			g_warning("block with freefunc %p has no items", block->free);
#endif

		for (i = 0, x = mem; i < (gulong) block->func_data; i++)
			x = block->free(x, my_data, CORBA_TRUE);

		if (my_data) {
			CORBA_Object_release (*(CORBA_Object *)my_data, NULL);
			block = my_data;
		}
	}
	g_free (block);
}

gpointer
ORBit_free_via_TypeCode (gpointer mem, gpointer tcp, gboolean ignore)
{
	int i;
	guchar *retval = NULL;
	CORBA_TypeCode tc = *(CORBA_TypeCode *) tcp;

	switch(tc->kind) {
	case CORBA_tk_any: {
		CORBA_any *anyval = mem;
		if (anyval->_release)
			CORBA_free (anyval->_value);
		retval = (guchar *)(anyval + 1);
		break;
	}
	case CORBA_tk_TypeCode:
	case CORBA_tk_objref:
		CORBA_Object_release (*(CORBA_Object *) mem, NULL);

		retval = (guchar *) mem + sizeof(CORBA_Object);
		break;
	case CORBA_tk_Principal: {
		CORBA_Principal *pval = mem;

		if (pval->_release)
			CORBA_free (pval->_buffer);

		retval = (guchar *)(pval + 1);
		break;
	}
	case CORBA_tk_except:
	case CORBA_tk_struct:
		mem = ALIGN_ADDRESS (mem, ORBit_find_alignment (tc));
		for (i = 0; i < tc->sub_parts; i++)
			mem = ORBit_free_via_TypeCode (
				mem, &tc->subtypes[i], CORBA_TRUE);
		retval = mem;
		break;
	case CORBA_tk_union: {
		CORBA_TypeCode tag;
		int sz = 0;
		int al = 1;

		tag = ORBit_get_union_tag (tc, &mem, TRUE);

		for (i = 0; i < tc->sub_parts; i++) {
			al = MAX (al, ORBit_find_alignment (tc->subtypes[i]));
			sz = MAX (sz, ORBit_gather_alloc_info (tc->subtypes[i]));
		}
		mem = ALIGN_ADDRESS (mem, al);
		ORBit_free_via_TypeCode (mem, &tag, CORBA_TRUE);
		/* the end of the body (subtc) may not be the
		 * same as the end of the union */
		retval = (guchar *)mem + sz;
		break;
	}
	case CORBA_tk_wstring:
	case CORBA_tk_string:
		CORBA_free (*(char **) mem);
		retval = (guchar *) mem + sizeof (char *);
		break;
	case CORBA_tk_sequence: {
		CORBA_sequence_octet *pval = mem;

		if (pval->_release)
			CORBA_free (pval->_buffer);

		retval = (guchar *)mem + sizeof (CORBA_sequence_octet);
		break;
	}
	case CORBA_tk_array:
		for (i = 0; i < tc->length; i++)
			mem = ORBit_free_via_TypeCode(
				mem, &tc->subtypes[0], CORBA_TRUE);
		retval = mem;
		break;
	case CORBA_tk_alias:
		retval = ORBit_free_via_TypeCode (mem, &tc->subtypes[0], CORBA_TRUE);
		break;
	default: {
		int sz = ORBit_gather_alloc_info (tc);
		int al = ORBit_find_alignment (tc);
		
		/* NB. often alignment != alloc size */
		retval = (guchar *) ALIGN_ADDRESS (mem, al) + sz;
		break;
	}
	}

	return (gpointer) retval;
}
