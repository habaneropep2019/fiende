#include "config.h"

#include <string.h>

#include "orbit-idl-c-backend.h"

static void c_demarshal_generate(OIDL_Marshal_Node *node, OIDL_C_Marshal_Info *cmi);
static void c_demarshal_datum(OIDL_Marshal_Node *node, OIDL_C_Marshal_Info *cmi);
static void c_demarshal_loop(OIDL_Marshal_Node *node, OIDL_C_Marshal_Info *cmi);
static void c_demarshal_switch(OIDL_Marshal_Node *node, OIDL_C_Marshal_Info *cmi);
static void c_demarshal_complex(OIDL_Marshal_Node *node, OIDL_C_Marshal_Info *cmi);
static void c_demarshal_set(OIDL_Marshal_Node *node, OIDL_C_Marshal_Info *cmi);
static void c_demarshal_load_curptr(OIDL_C_Marshal_Info *cmi);
static void c_demarshal_store_curptr(OIDL_C_Marshal_Info *cmi);

void
c_demarshalling_generate(OIDL_Marshal_Node *node, OIDL_C_Info *ci, gboolean in_skels)
{
  OIDL_C_Marshal_Info cmi;

  cmi.ci = ci;
  cmi.last_tail_align = 1;
  cmi.endian_swap_pass = TRUE;
  cmi.in_skels = in_skels?1:0;

  c_demarshal_load_curptr(&cmi);

  if(in_skels) {
    cmi.alloc_on_stack = TRUE;
    cmi.orb_name = "(((ORBit_ObjectKey *) _ORBIT_servant->_private)->object->orb)";
  } else {
    cmi.alloc_on_stack = FALSE;
#if 0
    cmi.orb_name = "_obj->orb";
#else
    cmi.orb_name = "GIOP_MESSAGE_BUFFER(_ORBIT_recv_buffer)->connection->orb_data";
#endif
  }

  fprintf(ci->fh, "if(giop_msg_conversion_needed(GIOP_MESSAGE_BUFFER(_ORBIT_recv_buffer))) {\n");
  c_demarshal_generate(node, &cmi);
  fprintf(ci->fh, "} else {\n");
  cmi.last_tail_align = 1;
  cmi.endian_swap_pass = FALSE;
  c_demarshal_generate(node, &cmi);
  fprintf(ci->fh, "}\n");
}

static void
c_demarshal_generate(OIDL_Marshal_Node *node, OIDL_C_Marshal_Info *cmi)
{
  if(node->flags & MN_NOMARSHAL) return;

  switch(node->type) {
  case MARSHAL_DATUM:
    c_demarshal_datum(node, cmi);
    break;
  case MARSHAL_LOOP:
    c_demarshal_loop(node, cmi);
    break;
  case MARSHAL_SWITCH:
    c_demarshal_switch(node, cmi);
    break;
  case MARSHAL_COMPLEX:
    c_demarshal_complex(node, cmi);
    break;
  case MARSHAL_SET:
    c_demarshal_set(node, cmi);
    break;
  default:
    break;
  }
}

static void
c_demarshal_update_curptr(OIDL_Marshal_Node *node, char *sizestr, OIDL_C_Marshal_Info *cmi)
{
  if((node->flags & MN_DEMARSHAL_UPDATE_AFTER)
     || (node->flags & MN_LOOPED))
    fprintf(cmi->ci->fh, "_ORBIT_curptr += %s;\n", sizestr);
}

static void
c_demarshal_load_curptr(OIDL_C_Marshal_Info *cmi)
{
  fprintf(cmi->ci->fh, "_ORBIT_curptr = GIOP_RECV_BUFFER(_ORBIT_recv_buffer)->cur;\n");
}

static void
c_demarshal_store_curptr(OIDL_C_Marshal_Info *cmi)
{
  fprintf(cmi->ci->fh, "GIOP_RECV_BUFFER(_ORBIT_recv_buffer)->cur = _ORBIT_curptr;\n");
}

static void
c_demarshal_alignfor(OIDL_Marshal_Node *node, OIDL_C_Marshal_Info *cmi)
{
  /* do we need to generate an alignment space? */
  if(node->iiop_head_align > cmi->last_tail_align) {
    fprintf(cmi->ci->fh, "_ORBIT_curptr = ALIGN_ADDRESS(_ORBIT_curptr, %d);\n", node->iiop_head_align);
  }

  cmi->last_tail_align = node->iiop_tail_align;
}

static void
c_demarshal_datum(OIDL_Marshal_Node *node, OIDL_C_Marshal_Info *cmi)
{
  char *ctmp;

  ctmp = oidl_marshal_node_valuestr(node);

  c_demarshal_alignfor(node, cmi);

  if(cmi->endian_swap_pass
     && (node->flags & MN_ENDIAN_DEPENDANT)) {
    int n;

    n = node->u.datum_info.datum_size * 8;

    if(n >= 64)
      fprintf(cmi->ci->fh, "iiop_byteswap((guchar *)&(%s), _ORBIT_curptr, %d);\n", ctmp, node->u.datum_info.datum_size);
    else {
      fprintf(cmi->ci->fh, "(*((guint%d *)&(%s))) = ", n, ctmp);
      fprintf(cmi->ci->fh, "GUINT%d_SWAP_LE_BE(*((guint%d *)_ORBIT_curptr));",
	      n, n);
    }
  } else {
    fprintf(cmi->ci->fh, "%s = *((", ctmp);
    orbit_cbe_write_node_typespec(cmi->ci->fh, node);
    fprintf(cmi->ci->fh, "*)_ORBIT_curptr);\n");
  }

  {
    char buf[32];
    g_snprintf(buf, sizeof(buf), "%d", node->u.datum_info.datum_size);
    c_demarshal_update_curptr(node, buf, cmi);
  }

  g_free(ctmp);
}

static void
c_demarshal_loop(OIDL_Marshal_Node *node, OIDL_C_Marshal_Info *cmi)
{
  char *ctmp, *ctmp_len, *ctmp_loop, *ctmp_contents;

  ctmp = oidl_marshal_node_valuestr(node);
  ctmp_loop = oidl_marshal_node_valuestr(node->u.loop_info.loop_var);
  ctmp_len = oidl_marshal_node_valuestr(node->u.loop_info.length_var);
  ctmp_contents = oidl_marshal_node_valuestr(node->u.loop_info.contents);

  c_demarshal_generate(node->u.loop_info.length_var, cmi);

  if(cmi->alloc_on_stack && !(node->flags & MN_DEMARSHAL_CORBA_ALLOC)) {
    if(!(node->u.loop_info.contents->flags & MN_COALESCABLE)
       || (cmi->endian_swap_pass && (node->u.loop_info.contents->flags & MN_ENDIAN_DEPENDANT))) {
      if(node->flags & (MN_ISSEQ|MN_ISSTRING)) {
	fprintf(cmi->ci->fh, "%s%s = alloca(sizeof(%s) * %s);\n", ctmp, (node->flags & MN_ISSEQ)?"._buffer":"",
		ctmp_contents, ctmp_len);
	if(node->flags & MN_ISSEQ)
	  fprintf(cmi->ci->fh, "%s._release = CORBA_FALSE;\n", ctmp);
      } else {
	/* Array - no alloc needed */
      }
    }
  } else {
    char *tname, *tcname;
    
    tname = orbit_cbe_get_typename(node->tree);
    tcname = orbit_cbe_get_typename(node->u.loop_info.contents->tree);

    if(node->flags & MN_ISSEQ) {
      IDL_tree seq = orbit_cbe_get_typespec(node->tree);
      if(orbit_cbe_type_is_builtin(node->u.loop_info.contents->tree))
	fprintf(cmi->ci->fh, "%s._buffer = CORBA_sequence_%s_allocbuf(%s);\n", ctmp, tcname+strlen("CORBA_"), ctmp_len);
      else {
	if(IDL_TYPE_SEQUENCE(seq).positive_int_const) {
	  /* bounded */
	  fprintf(cmi->ci->fh,"%s._maximum = ",ctmp);
	  orbit_cbe_write_const(cmi->ci->fh, IDL_TYPE_SEQUENCE(seq).positive_int_const);
	  fprintf(cmi->ci->fh,";\n");
	  fprintf(cmi->ci->fh, "%s._buffer = CORBA_sequence_%s_allocbuf(%s._maximum);\n", ctmp, tcname, ctmp);
	} else {
	  /* unbounded */
	  fprintf(cmi->ci->fh, "%s._maximum = %s._length;\n",ctmp,ctmp);
	  fprintf(cmi->ci->fh, "%s._buffer = CORBA_sequence_%s_allocbuf(%s);\n", ctmp, tcname, ctmp_len);
	}
      }
      fprintf(cmi->ci->fh, "%s._release = CORBA_TRUE;\n", ctmp);
    } else if(node->flags & MN_ISSTRING)
      fprintf(cmi->ci->fh, "%s = CORBA_string_alloc(%s);\n", ctmp, ctmp_len);
#if 0
    /* I'm pretty sure we don't ever need to alloc an array. Woo! */
    else if(node->flags & MN_DEMARSHAL_CORBA_ALLOC) {
      fprintf(cmi->ci->fh, "%s = %s__alloc();\n", ctmp, tname);
    }
#endif
    g_free(tcname);
    g_free(tname);
  }

  if((!cmi->endian_swap_pass || !(node->u.loop_info.contents->flags & MN_ENDIAN_DEPENDANT))
     && (node->u.loop_info.contents->flags & MN_COALESCABLE)) {
    GString *tmpstr, *tmpstr2;

    c_demarshal_alignfor(node->u.loop_info.contents, cmi);

    tmpstr = g_string_new(NULL);
    tmpstr2 = g_string_new(NULL);
    g_string_sprintf(tmpstr, "sizeof(%s) * %s", ctmp_contents, ctmp_len);
    /* XXX badhack - what if 'node' is a pointer thingie? Need to find out whether to append '._buffer' or '->_buffer' */
    g_string_sprintf(tmpstr2, "%s%s", ctmp, (node->flags & MN_ISSEQ)?"._buffer":"");

    if(cmi->alloc_on_stack
       && !(node->flags & MN_DEMARSHAL_CORBA_ALLOC)
       && ((node->flags & MN_ISSEQ) /* Doesn't work for arrays. */
	   || (node->flags & MN_ISSTRING))) {
      fprintf(cmi->ci->fh, "%s = (", tmpstr2->str);
      orbit_cbe_write_typespec(cmi->ci->fh, node->u.loop_info.contents->tree);
      fprintf(cmi->ci->fh, "*)_ORBIT_curptr;\n");
    } else {      
      fprintf(cmi->ci->fh, "memcpy(%s, _ORBIT_curptr, %s);\n", tmpstr2->str, tmpstr->str);
    }

    c_demarshal_update_curptr(node, tmpstr->str, cmi);

    g_string_free(tmpstr2, TRUE);
    g_string_free(tmpstr, TRUE);
  } else {
    cmi->last_tail_align = MIN(cmi->last_tail_align, node->u.loop_info.contents->iiop_tail_align);
    fprintf(cmi->ci->fh, "for(%s = 0; %s < %s; %s++) {\n", ctmp_loop, ctmp_loop, ctmp_len, ctmp_loop);
    c_demarshal_generate(node->u.loop_info.loop_var, cmi);
    c_demarshal_generate(node->u.loop_info.contents, cmi);
    fprintf(cmi->ci->fh, "}\n\n");
  }

  g_free(ctmp_contents);
  g_free(ctmp_len);
  g_free(ctmp_loop);
  g_free(ctmp);
}

static void
c_demarshal_switch(OIDL_Marshal_Node *node, OIDL_C_Marshal_Info *cmi)
{
  char *ctmp;
  GSList *ltmp;
  guint8 last_tail_align;
  gboolean need_default;

  c_demarshal_generate(node->u.switch_info.discrim, cmi);

  last_tail_align = cmi->last_tail_align;

  ctmp = oidl_marshal_node_valuestr(node->u.switch_info.discrim);
  fprintf(cmi->ci->fh, "switch(%s) {\n", ctmp);
  g_free(ctmp);

  need_default = TRUE;
  for(ltmp = node->u.switch_info.cases; ltmp; ltmp = g_slist_next(ltmp)) {
    GSList *ltmp2;
    OIDL_Marshal_Node *sub;

    cmi->last_tail_align = last_tail_align;

    sub = ltmp->data;
    g_assert(sub->type == MARSHAL_CASE);
    if(sub->u.case_info.labels) {
      for(ltmp2 = sub->u.case_info.labels; ltmp2; ltmp2 = g_slist_next(ltmp2)) {
	if(ltmp2->data) {
	  fprintf(cmi->ci->fh, "case ");
	  orbit_cbe_write_const_node(cmi->ci->fh, ltmp2->data);
	  fprintf(cmi->ci->fh, ":\n");
	} else {
	  fprintf(cmi->ci->fh, "default:\n");
	  need_default = FALSE;
	}
      }
    } else {
      fprintf(cmi->ci->fh, "default:\n");
      need_default = FALSE;
    }
    c_demarshal_generate(sub->u.case_info.contents, cmi);
    fprintf(cmi->ci->fh, "break;\n");
  }
  if(need_default) {
    fprintf(cmi->ci->fh, "default:\nbreak;\n");
  }
  fprintf(cmi->ci->fh, "}\n");

  cmi->last_tail_align = node->iiop_tail_align;
}

static void
c_demarshal_complex(OIDL_Marshal_Node *node, OIDL_C_Marshal_Info *cmi)
{
  char *ctmp;

  c_demarshal_store_curptr(cmi);
  ctmp = oidl_marshal_node_valuestr(node);

  switch(node->u.complex_info.type) {
  case CX_CORBA_FIXED:
    g_error("Don't know how to demarshal a CORBA_fixed yet.");
    break;
  case CX_CORBA_ANY:
    fprintf(cmi->ci->fh, "ORBit_demarshal_any(_ORBIT_recv_buffer, &(%s), %s, %s);\n",
	    ctmp, (cmi->alloc_on_stack && !(node->flags & MN_DEMARSHAL_CORBA_ALLOC))?"CORBA_FALSE":"CORBA_TRUE", cmi->orb_name);
    break;
  case CX_CORBA_OBJECT:
    fprintf(cmi->ci->fh, "%s = ORBit_demarshal_object(_ORBIT_recv_buffer, %s);\n",
	    ctmp, cmi->orb_name);
    break;
  case CX_CORBA_TYPECODE:
    fprintf(cmi->ci->fh, "ORBit_decode_CORBA_TypeCode(&%s, _ORBIT_recv_buffer);\n", ctmp);
    break;
  case CX_CORBA_CONTEXT:
    if(cmi->in_skels)
      fprintf(cmi->ci->fh, "ORBit_Context_demarshal(NULL, &_ctx, _ORBIT_recv_buffer);\n");
    break;
  }

  c_demarshal_load_curptr(cmi);

  g_free(ctmp);
}

static void
c_demarshal_set(OIDL_Marshal_Node *node, OIDL_C_Marshal_Info *cmi)
{
  if((!cmi->endian_swap_pass || !(node->flags & MN_ENDIAN_DEPENDANT))
     && node->name
     && (node->flags & MN_COALESCABLE)) {
    char *ctmp, *ctmp2;

    ctmp = oidl_marshal_node_valuestr(node);

    c_demarshal_alignfor(node, cmi);

    fprintf(cmi->ci->fh, "memcpy(&(%s), _ORBIT_curptr, sizeof(%s));\n", ctmp, ctmp);
    ctmp2 = g_strdup_printf("sizeof(%s)", ctmp);

    c_demarshal_update_curptr(node, ctmp2, cmi);

    g_free(ctmp2);
    g_free(ctmp);
  } else
    g_slist_foreach(node->u.set_info.subnodes, (GFunc)c_demarshal_generate, cmi);
}
