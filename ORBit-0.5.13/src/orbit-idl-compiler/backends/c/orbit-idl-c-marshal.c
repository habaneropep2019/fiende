#include "config.h"

#include "orbit-idl-c-backend.h"

#define NEEDS_INDIRECT(node) (((node)->flags & MN_LOOPED) || ((node)->flags & MN_NEED_TMPVAR))

static void c_marshal_generate(OIDL_Marshal_Node *node, OIDL_C_Marshal_Info *cmi);
static void c_marshal_datum(OIDL_Marshal_Node *node, OIDL_C_Marshal_Info *cmi);
static void c_marshal_switch(OIDL_Marshal_Node *node, OIDL_C_Marshal_Info *cmi);
static void c_marshal_loop(OIDL_Marshal_Node *node, OIDL_C_Marshal_Info *cmi);
static void c_marshal_complex(OIDL_Marshal_Node *node, OIDL_C_Marshal_Info *cmi);
static void c_marshal_set(OIDL_Marshal_Node *node, OIDL_C_Marshal_Info *cmi);
static void c_marshal_alignfor(OIDL_Marshal_Node *node, OIDL_C_Marshal_Info *cmi);

void
c_marshalling_generate(OIDL_Marshal_Node *node, OIDL_C_Info *ci, gboolean on_stack)
{
  OIDL_C_Marshal_Info cmi;

  if(!node) return;

  cmi.ci = ci;
  cmi.last_tail_align = 1;
  cmi.alloc_on_stack = on_stack;
  cmi.in_skels = on_stack?1:0;

  c_marshal_generate(node, &cmi);
}

static void
c_marshal_generate(OIDL_Marshal_Node *node, OIDL_C_Marshal_Info *cmi)
{
  if(!node) return;

  if(node->flags & MN_NOMARSHAL) return;

  switch(node->type) {
  case MARSHAL_LOOP:
    c_marshal_loop(node, cmi);
    break;
  case MARSHAL_SWITCH:
    c_marshal_switch(node, cmi);
    break;
  case MARSHAL_DATUM:
    c_marshal_datum(node, cmi);
    break;
  case MARSHAL_COMPLEX:
    c_marshal_complex(node, cmi);
    break;
  case MARSHAL_SET:
    c_marshal_set(node, cmi);
    break;
  case MARSHAL_CONST:
  default:
    g_error("We're supposed to marshal a %d node?", node->type);
    break;
  }
}

static void
c_marshal_append(OIDL_C_Marshal_Info *cmi, OIDL_Marshal_Node *node, char *itemstr, char *sizestr)
{
  gboolean indirect = NEEDS_INDIRECT(node), addrof = FALSE;

  if(((node->type == MARSHAL_DATUM)
      || (node->type == MARSHAL_SET))
     && (!node->up
	 || (node->up->type != MARSHAL_LOOP)
	 || (node != node->up->u.loop_info.contents)))
    addrof = TRUE;

  if(indirect && cmi->alloc_on_stack)
     fprintf(cmi->ci->fh, "{ guchar *_ORBIT_t; _ORBIT_t = alloca(%s); memcpy(_ORBIT_t, %s(%s), %s);\n", sizestr, addrof?"&":"",
	     itemstr, sizestr);

  if(indirect && !cmi->alloc_on_stack) {
    fprintf(cmi->ci->fh, "giop_send_buffer_append_mem_indirect(GIOP_SEND_BUFFER(_ORBIT_send_buffer), %s(%s), %s);\n",
	    addrof?"&":"", itemstr, sizestr);
  } else {
    fprintf(cmi->ci->fh, "giop_message_buffer_append_mem(GIOP_MESSAGE_BUFFER(_ORBIT_send_buffer), %s(%s), %s);\n",
	    (addrof && !indirect)?"&":"",
	    indirect?"_ORBIT_t":itemstr, sizestr);
  }

  if(indirect && cmi->alloc_on_stack)
    fprintf(cmi->ci->fh, "}\n");
}

#if 0
#define AP(itemstr, sizestr) c_marshal_append(cmi, itemstr, sizestr, FALSE, FALSE, FALSE)
#define APA(itemstr, sizestr) c_marshal_append(cmi, itemstr, sizestr, TRUE, FALSE, FALSE)

#define API(itemstr, sizestr) c_marshal_append(cmi, itemstr, sizestr, FALSE, TRUE, FALSE)
#define APIA(itemstr, sizestr) c_marshal_append(cmi, itemstr, sizestr, TRUE, TRUE, FALSE)
#endif

static void
c_marshal_alignfor(OIDL_Marshal_Node *node, OIDL_C_Marshal_Info *cmi)
{
  /* do we need to generate an alignment space? */
  if(node->iiop_head_align > cmi->last_tail_align) {
    fprintf(cmi->ci->fh, "giop_message_buffer_do_alignment(GIOP_MESSAGE_BUFFER(_ORBIT_send_buffer), %d);\n",
	    node->iiop_head_align);
  }
  cmi->last_tail_align = node->iiop_tail_align;
}

static void
c_marshal_datum(OIDL_Marshal_Node *node, OIDL_C_Marshal_Info *cmi)
{
  GString *tmpstr;
  char *ctmp;

  c_marshal_alignfor(node, cmi);
  tmpstr = g_string_new(NULL);

  ctmp = oidl_marshal_node_valuestr(node);
  g_string_sprintf(tmpstr, "sizeof(%s)", ctmp);

  c_marshal_append(cmi, node, ctmp, tmpstr->str);

  g_free(ctmp);
  g_string_free(tmpstr, TRUE);
}

static void
c_marshal_switch(OIDL_Marshal_Node *node, OIDL_C_Marshal_Info *cmi)
{
  char *ctmp;
  GSList *ltmp;
  guint8 last_tail_align;
  gboolean need_default;

  c_marshal_generate(node->u.switch_info.discrim, cmi);

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
    c_marshal_generate(sub->u.case_info.contents, cmi);
    fprintf(cmi->ci->fh, "break;\n");
  }
  if(need_default) {
    fprintf(cmi->ci->fh, "default:\nbreak;\n");
  }
  fprintf(cmi->ci->fh, "}\n");

  cmi->last_tail_align = node->iiop_tail_align;
}

static void
c_marshal_loop(OIDL_Marshal_Node *node, OIDL_C_Marshal_Info *cmi)
{
  char *ctmp, *ctmp_len, *ctmp_loop, *ctmp_contents;

  ctmp = oidl_marshal_node_valuestr(node);
  ctmp_loop = oidl_marshal_node_valuestr(node->u.loop_info.loop_var);
  ctmp_len = oidl_marshal_node_valuestr(node->u.loop_info.length_var);
  ctmp_contents = oidl_marshal_node_valuestr(node->u.loop_info.contents);

  if(node->flags & MN_ISSTRING) {
    fprintf(cmi->ci->fh, "%s = strlen(%s) + 1;\n", ctmp_len, ctmp);
  }

  c_marshal_generate(node->u.loop_info.length_var, cmi);

  if(node->u.loop_info.contents->flags & MN_COALESCABLE) {
    GString *tmpstr, *tmpstr2;

    c_marshal_alignfor(node->u.loop_info.contents, cmi);

    tmpstr = g_string_new(NULL);
    tmpstr2 = g_string_new(NULL);
    g_string_sprintf(tmpstr, "sizeof(%s) * %s", ctmp_contents, ctmp_len);

    /* XXX badhack - what if 'node' is a pointer thingie? Need to find out whether to append '._buffer' or '->_buffer' */
    g_string_sprintf(tmpstr2, "%s%s", ctmp, (node->flags & MN_ISSEQ)?"._buffer":"");

    c_marshal_append(cmi, node->u.loop_info.contents, tmpstr2->str, tmpstr->str);

    g_string_free(tmpstr2, TRUE);
    g_string_free(tmpstr, TRUE);
  } else {
    cmi->last_tail_align = MIN(cmi->last_tail_align, node->u.loop_info.contents->iiop_tail_align);
    fprintf(cmi->ci->fh, "for(%s = 0; %s < %s; %s++) {\n", ctmp_loop, ctmp_loop, ctmp_len, ctmp_loop);
    c_marshal_generate(node->u.loop_info.loop_var, cmi);
    c_marshal_generate(node->u.loop_info.contents, cmi);
    fprintf(cmi->ci->fh, "}\n\n");
  }

  g_free(ctmp_contents);
  g_free(ctmp_len);
  g_free(ctmp_loop);
  g_free(ctmp);
}

static void
c_marshal_complex(OIDL_Marshal_Node *node, OIDL_C_Marshal_Info *cmi)
{
  char *ctmp;

  ctmp = oidl_marshal_node_valuestr(node);

  switch(node->u.complex_info.type) {
  case CX_CORBA_ANY:
    fprintf(cmi->ci->fh, "ORBit_marshal_any(_ORBIT_send_buffer, &(%s));\n", ctmp);
    break;
  case CX_CORBA_OBJECT:
    fprintf(cmi->ci->fh, "ORBit_marshal_object(_ORBIT_send_buffer, %s);\n", ctmp);
    break;
  case CX_CORBA_FIXED:
    g_error("CORBA_fixed marshalling NYI.");
    break;
  case CX_CORBA_TYPECODE:
    fprintf(cmi->ci->fh, "ORBit_encode_CORBA_TypeCode(%s, _ORBIT_send_buffer);\n", ctmp);
    break;
  case CX_CORBA_CONTEXT:
    fprintf(cmi->ci->fh, "ORBit_Context_marshal(_ctx, _context_items, %d, _ORBIT_send_buffer);\n",
	    node->u.complex_info.context_item_count);
    break;
  }

  g_free(ctmp);
}

static void
c_marshal_set(OIDL_Marshal_Node *node, OIDL_C_Marshal_Info *cmi)
{
  if(node->name
     && (node->flags & MN_COALESCABLE)) {
    char *ctmp;
    GString *tmpstr = g_string_new(NULL);

    c_marshal_alignfor(node, cmi);

    ctmp = oidl_marshal_node_fqn(node);
    g_string_sprintf(tmpstr, "sizeof(%s)", ctmp);

    c_marshal_append(cmi, node, ctmp, tmpstr->str);

  } else {
    GSList *ltmp;

    for(ltmp = node->u.set_info.subnodes; ltmp; ltmp = g_slist_next(ltmp))
      c_marshal_generate(ltmp->data, cmi);
  }
}
