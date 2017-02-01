#include "orb/orbit.h"

#define o_return_val_if_fail(expr, val) if(!(expr)) { CORBA_exception_set_system(ev, ex_CORBA_BAD_PARAM, CORBA_COMPLETED_NO); return (val); }
#define o_return_if_fail(expr) if(!(expr)) { CORBA_exception_set_system(ev, ex_CORBA_BAD_PARAM, CORBA_COMPLETED_NO); return; }

static gboolean
free_entry(gpointer key, gpointer value, gpointer user_data)
{
  g_free(key);
  g_free(value);

  return TRUE;
}

static void
ORBit_Context_release(CORBA_Context ctx, CORBA_Environment *ev);

static gboolean
free_child(gpointer value, gpointer user_data)
{
  CORBA_Context ctx = value;

  ORBIT_ROOT_OBJECT(ctx)->refs = 1;
  ctx->parent_ctx = CORBA_OBJECT_NIL;
  ORBit_Context_release(ctx, NULL);

  return TRUE;
}

static void
ORBit_Context_release(CORBA_Context ctx,
		      CORBA_Environment *ev)
{
  ORBIT_ROOT_OBJECT_UNREF(ctx);

  if(ORBIT_ROOT_OBJECT(ctx)->refs <= 0) {
    if(ctx->children) {
      g_slist_foreach(ctx->children, (GFunc)free_child, ctx);
      g_slist_free(ctx->children);
    }

    if(ctx->mappings) {
      g_hash_table_foreach_remove(ctx->mappings, free_entry, ctx);
      g_hash_table_destroy(ctx->mappings);
    }

    if(ctx->parent_ctx != CORBA_OBJECT_NIL)
      ctx->parent_ctx->children = g_slist_remove(ctx->parent_ctx->children, ctx->the_name);
	  
    g_free(ctx->the_name);
	  
    g_free(ctx);
  }
}

static const ORBit_RootObject_Interface CORBA_Context_epv =
{
  (void (*)(gpointer, CORBA_Environment *))ORBit_Context_release,
};

static CORBA_Context
CORBA_Context_new(CORBA_Context parent, const char *name, CORBA_Environment *ev)
{
  CORBA_Context retval;

  retval = g_new0(struct CORBA_Context_type, 1);

  ORBit_pseudo_object_init(ORBIT_PSEUDO_OBJECT(retval), ORBIT_PSEUDO_CONTEXT, ev);

  ORBIT_ROOT_OBJECT(retval)->refs = 0;
  ORBit_RootObject_set_interface(ORBIT_ROOT_OBJECT(retval), (gpointer)&CORBA_Context_epv, ev);

  if(name)
    retval->the_name = g_strdup(name);

  retval->parent_ctx = parent;
  if(parent)
    parent->children = g_slist_prepend(parent->children, retval);

  return retval;
}

/* Section 5.6.1 */
CORBA_Status CORBA_ORB_get_default_context(CORBA_ORB orb, CORBA_Context *ctx, CORBA_Environment *ev)
{
  g_return_if_fail(ev != NULL);
  o_return_if_fail(orb && ctx);

  if(!orb->default_ctx)
    orb->default_ctx = CORBA_Context_new(CORBA_OBJECT_NIL, NULL, ev);

  *ctx = (CORBA_Context)CORBA_Object_duplicate((CORBA_Object)orb->default_ctx, ev);
}

/********* XXX todo - CORBA_Context support */
CORBA_Status CORBA_Context_set_one_value(CORBA_Context ctx, CORBA_Identifier prop_name, char *value, CORBA_Environment *ev)
{
  gpointer old_nom, old_value;
  g_return_if_fail(ev != NULL);
  o_return_if_fail(ctx && prop_name && value);

  if(!ctx->mappings)
    ctx->mappings = g_hash_table_new(g_str_hash, g_str_equal);

  if(g_hash_table_lookup_extended(ctx->mappings, prop_name, &old_nom, &old_value)) {
    g_free(old_nom);
    g_free(old_value);
  }

  g_hash_table_insert(ctx->mappings, g_strdup(prop_name), g_strdup(value));
}

/* Section 5.6.3 */
CORBA_Status CORBA_Context_set_values(CORBA_Context ctx, CORBA_NVList *values, CORBA_Environment *ev)
{
  int i;

  for(i = 0; i < values->list->len; i++) {
    CORBA_NamedValue *nvp;

    nvp = ((CORBA_NamedValue *)values->list->data) + i;

    g_assert(nvp->argument._type == TC_string);

    CORBA_Context_set_one_value(ctx, nvp->name, nvp->argument._value, ev);
  }
}

/* Section 5.6.4 */

typedef struct {
  CORBA_Context ctx;
  CORBA_Identifier prop_name;
  CORBA_NVList *values;
  CORBA_Environment *ev;
  int len;
} CTXSearchInfo;

static gboolean
list_has_key(CORBA_NVList *list, const char *key)
{
  int i;

  for(i = 0; i < list->list->len; i++) {
    CORBA_NamedValue *nvp;

    nvp = ((CORBA_NamedValue *)list->list->data) + i;

    if(!strcmp(nvp->name, key))
      return TRUE;
  }

  return FALSE;
}

static void
search_props(gpointer key, gpointer value, CTXSearchInfo *csi)
{
  if(strncmp(key, csi->prop_name, (csi->len == 0) ? strlen(key) : csi->len))
    return;

  if(list_has_key(csi->values, key))
    return;

  CORBA_NVList_add_item(csi->values, key, TC_string, &value, strlen(value) + 1, CORBA_IN_COPY_VALUE, NULL);
}

static void
ctx_get_values(CORBA_Context ctx, CORBA_Flags op_flags,
	       CORBA_Identifier prop_name, CORBA_NVList **values,
	       gint is_wc,
	       CORBA_Environment *ev)
{
  gboolean go_up = FALSE;

  if(is_wc >= 0) {
    CTXSearchInfo csi;
  
    csi.ctx = ctx;
    csi.prop_name = prop_name;
    csi.values = *values;
    csi.ev = ev;
    csi.len = is_wc;

    if(ctx->mappings)
      g_hash_table_foreach(ctx->mappings, (GHFunc)search_props, &csi);

    go_up = TRUE;

  } else {
    char *val = NULL;

    if(ctx->mappings)
      val = g_hash_table_lookup(ctx->mappings, prop_name);

    if(val)
      CORBA_NVList_add_item(*values, prop_name, TC_string, &val, strlen(val) + 1, CORBA_IN_COPY_VALUE, ev);
    else
      go_up = TRUE;
  }

  if(go_up
     && ctx->parent_ctx
     && !(op_flags & CORBA_CTX_RESTRICT_SCOPE))
    ctx_get_values(ctx->parent_ctx, op_flags, prop_name, values, is_wc, ev);
}

CORBA_Status CORBA_Context_get_values(CORBA_Context ctx,
				      CORBA_Identifier start_scope,
				      CORBA_Flags op_flags,
				      CORBA_Identifier prop_name,
				      CORBA_NVList **values,
				      CORBA_Environment *ev)
{
  char *ctmp;
  int wc_pos;

  CORBA_ORB_create_list(CORBA_OBJECT_NIL, 0, values, ev);

  if(start_scope && *start_scope) {
    while(ctx && (!ctx->the_name || strcmp(ctx->the_name, start_scope)))
      ctx = ctx->parent_ctx;

    if(!ctx) {
      CORBA_exception_set_system(ev, ex_CORBA_INV_IDENT, CORBA_COMPLETED_NO);
      return;
    }
  }

  ctmp = strchr(prop_name, '*');
  if(ctmp)
    wc_pos = ctmp - prop_name;
  else
    wc_pos = -1;

  CORBA_ORB_create_list(CORBA_OBJECT_NIL, 0, values, ev);

  ctx_get_values(ctx, op_flags, prop_name, values, (prop_name[strlen(prop_name) - 1] == '*'), ev);

  if((*values)->list->len == 0)
    {
      CORBA_NVList_free(*values, ev);
      *values = NULL;
      CORBA_exception_set_system(ev, ex_CORBA_UNKNOWN, CORBA_COMPLETED_NO);
    }
}

/* Section 5.6.5 */
static void
delete_props(gpointer key, gpointer value, CTXSearchInfo *csi)
{
  if(strncmp(key, csi->prop_name, csi->len))
    return;

  g_hash_table_remove(csi->ctx->mappings, key);
  g_free(key);
  g_free(value);
}

CORBA_Status CORBA_Context_delete_values(CORBA_Context ctx, CORBA_Identifier prop_name, CORBA_Environment *ev)
{
  char *ctmp;
  int wc_pos;

  if(!ctx->mappings)
    return;

  ctmp = strchr(prop_name, '*');
  if(ctmp)
    wc_pos = ctmp - prop_name;
  else
    wc_pos = -1;

  if(wc_pos >= 0) {
    CTXSearchInfo csi;

    memset(&csi, 0, sizeof(csi));
    csi.ctx = ctx;
    csi.prop_name = prop_name;
    csi.ev = ev;
    csi.len = wc_pos;

    g_hash_table_foreach(ctx->mappings, (GHFunc)delete_props, &csi);
  } else {
    gpointer old_nom, old_value;

    if(g_hash_table_lookup_extended(ctx->mappings, prop_name, &old_nom, &old_value)) {
      g_free(old_nom);
      g_free(old_value);
    }
  }
}

/* Section 5.6.6 */
CORBA_Status CORBA_Context_create_child(CORBA_Context ctx, CORBA_Identifier ctx_name, CORBA_Context *child_ctx, CORBA_Environment *ev)
{
  *child_ctx = CORBA_Context_new(ctx, ctx_name, ev);
}

/* Section 5.6.7 */
CORBA_Status CORBA_Context_delete(CORBA_Context ctx, CORBA_Flags del_flags, CORBA_Environment *ev)
{
  if((del_flags & CORBA_CTX_DELETE_DESCENDENTS)
     || !ctx->children)
    free_child(ctx, NULL);
}

void
ORBit_Context_marshal(CORBA_Context ctx, const ORBit_ContextMarshalItem *mlist, CORBA_unsigned_long nitems, GIOPSendBuffer *buf)
{
  int i;
  CORBA_unsigned_long *real_nitems, ltmp;

  real_nitems = giop_send_buffer_append_mem_indirect_a(buf, &nitems, sizeof(nitems));
  if(!ctx->mappings) {
    *real_nitems = 0;
    return;
  }

  for(*real_nitems = i = 0; i < nitems; i++) {
    char *value;

    value = g_hash_table_lookup(ctx->mappings, mlist[i].str);
    if(!value)
      continue;

    /* Key */
    giop_message_buffer_append_mem_a(GIOP_MESSAGE_BUFFER(buf), &(mlist[i].len), sizeof(mlist[i].len));
    giop_message_buffer_append_mem(GIOP_MESSAGE_BUFFER(buf), mlist[i].str, mlist[i].len);
    (*real_nitems)++;

    /* Value */
    ltmp = strlen(value) + 1;
    giop_send_buffer_append_mem_indirect_a(buf, &ltmp, sizeof(ltmp));
    giop_message_buffer_append_mem(GIOP_MESSAGE_BUFFER(buf), value, ltmp);
    (*real_nitems)++;
  }
}

#define GET_ATOM(x) G_STMT_START{ GIOP_RECV_BUFFER(recv_buffer)->decoder(&x, (GIOP_RECV_BUFFER(recv_buffer)->cur), sizeof(x)); \
GIOP_RECV_BUFFER(recv_buffer)->cur = ((guchar *)GIOP_RECV_BUFFER(recv_buffer)->cur) + sizeof(x); \
}G_STMT_END
#define ALIGNFOR(x) recv_buffer->cur = ALIGN_ADDRESS(recv_buffer->cur, sizeof(x))

void
ORBit_Context_demarshal(CORBA_Context parent, CORBA_Context initme, GIOPRecvBuffer *recv_buffer)
{
  CORBA_unsigned_long nstrings, keylen, vallen, i;
  char *key, *value;

  memset(initme, 0, sizeof(struct CORBA_Context_type));
  ORBIT_ROOT_OBJECT(initme)->refs = -1;

  initme->parent_ctx = parent;

  ALIGNFOR(nstrings);
  GET_ATOM(nstrings);

  if(nstrings)
    initme->mappings = g_hash_table_new(g_str_hash, g_str_equal);
  else
    return;

  g_hash_table_freeze(initme->mappings);
  for(i = 0; i < nstrings; ) {
    ALIGNFOR(keylen);
    GET_ATOM(keylen);
    key = recv_buffer->cur;
    recv_buffer->cur = ((char *)recv_buffer->cur) + keylen;
    i++;

    if(i >= nstrings)
      break;

    ALIGNFOR(vallen);
    GET_ATOM(vallen);
    value = recv_buffer->cur;
    recv_buffer->cur = ((char *)recv_buffer->cur) + vallen;
    i++;

    g_hash_table_insert(initme->mappings, key, value);
  }
  g_hash_table_thaw(initme->mappings);
}

void
ORBit_Context_server_free(CORBA_Context ctx)
{
  g_hash_table_destroy(ctx->mappings);
}
