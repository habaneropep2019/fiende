#include "config.h"
#include "orbit-idl2.h"

typedef struct {
  GHashTable *visited;
} OIDL_Populate_Info;

static OIDL_Marshal_Node *
oidl_marshal_node_new(OIDL_Marshal_Node *parent, OIDL_Marshal_Node_Type type, const char *name)
{
  OIDL_Marshal_Node *retval;

  retval = g_new0(OIDL_Marshal_Node, 1);

  retval->up = parent;
  retval->type = type;
  retval->name = (char *)name;

  if(parent)
    retval->flags |= (parent->flags & (MN_LOOPED));

  return retval;
}

/* A node is an array element if it is
   Underneath a MARSHAL_LOOP and zero or more nameless nodes.
 */
static gboolean
oidl_marshal_node_is_arrayel(OIDL_Marshal_Node *node, OIDL_Marshal_Node **loopvar_ret) G_GNUC_UNUSED;
static gboolean
oidl_marshal_node_is_arrayel(OIDL_Marshal_Node *node, OIDL_Marshal_Node **loopvar_ret)
{
  OIDL_Marshal_Node *curnode;
  gboolean retval = FALSE;

  g_assert(node);

  for(curnode = node->up; curnode && !curnode->name && curnode->type != MARSHAL_LOOP; curnode = curnode->up) /* */;

  if(!curnode) return FALSE;

  if(curnode
     && (curnode->type == MARSHAL_LOOP)
     && curnode->u.loop_info.loop_var != node
     && curnode->u.loop_info.length_var != node) {
     retval = TRUE;

     if(loopvar_ret)
       *loopvar_ret = curnode->u.loop_info.loop_var;
  } else if(loopvar_ret)
    *loopvar_ret = NULL;
  
  return retval;
}

/* If we are trying to produce C code to access a specific variable,
   then we need to be able get the C-compilable string that refers to
   that var.
 */
/* This routine is one of the most difficult ones to get right. Dragons be here. */
char *
oidl_marshal_node_fqn(OIDL_Marshal_Node *node)
{
  GString *tmpstr;
  char *retval, *ctmp;
  OIDL_Marshal_Node *curnode;
  guint nptrs = 0, i;
  gboolean did_append = FALSE, new_did_append;

  if(!node->name
     && (
	 (node->flags & MN_NEED_TMPVAR)
	 || (node->type == MARSHAL_CONST))) {
    return g_strdup("<Unassigned>");
  }

  tmpstr = g_string_new("");

#if 1
  for(curnode = node; curnode; curnode = curnode->up) {
    new_did_append = FALSE;

    if(curnode->up
       && !(curnode->flags & MN_NSROOT)) {
      switch(curnode->up->type) {
      case MARSHAL_LOOP:
	if(curnode == curnode->up->u.loop_info.contents) {
	  ctmp = oidl_marshal_node_fqn(curnode->up->u.loop_info.loop_var);
	  
	  if(did_append) {
	    g_string_prepend_c(tmpstr, '.');
	    did_append = FALSE;
	  }

	  g_string_prepend_c(tmpstr, ']');
	  g_string_prepend(tmpstr, ctmp);
	  g_string_prepend_c(tmpstr, '[');
	  g_free(ctmp);
	  if(curnode->up->flags & MN_ISSEQ) {
	    g_string_prepend(tmpstr, "._buffer");
	    did_append = FALSE;
	  }
	}
	break;
      default:
	break;
      }
    }
    if(curnode->name) {
      if(did_append)
	g_string_prepend_c(tmpstr, '.');

      nptrs += curnode->nptrs;
      
      for(i = 0; i < curnode->nptrs; i++)
	g_string_prepend_c(tmpstr, ')');

      g_string_prepend(tmpstr, curnode->name);

      for(i = 0; i < curnode->nptrs; i++)
	g_string_prepend_c(tmpstr, '*');

      new_did_append = TRUE;

      did_append = new_did_append;
    }

    if(curnode->flags & MN_NSROOT)
      break;
  }
#else
  for(curnode = node; curnode; curnode = curnode->up) {
    new_did_append = FALSE;

    if(curnode->up
       && !(curnode->flags & MN_NSROOT)) {
      switch(curnode->up->type) {
      case MARSHAL_LOOP:
	if(curnode == curnode->up->u.loop_info.contents) {
	  ctmp = oidl_marshal_node_fqn(curnode->up->u.loop_info.loop_var);
	  if(did_append)
	    g_string_prepend_c(tmpstr, '.');

	  g_string_prepend_c(tmpstr, ']');
	  g_string_prepend(tmpstr, ctmp);
	  g_string_prepend_c(tmpstr, '[');
	  g_free(ctmp);
	  if(curnode->up->flags & MN_ISSEQ) {
	    g_string_prepend(tmpstr, "_buffer");
	    did_append = TRUE;
	  }
	}
	break;
      default:
	break;
      }
    }

    if(curnode->name) {
      if(did_append)
	g_string_prepend_c(tmpstr, '.');

      nptrs += curnode->nptrs;
      
      for(i = 0; i < curnode->nptrs; i++)
	g_string_prepend_c(tmpstr, ')');

      g_string_prepend(tmpstr, curnode->name);

      for(i = 0; i < curnode->nptrs; i++)
	g_string_prepend_c(tmpstr, '*');

      new_did_append = TRUE;

      did_append = new_did_append;
    }

    if(curnode->flags & MN_NSROOT)
      break;
  }
#endif

  for(i = 0; i < nptrs; i++)
    g_string_prepend_c(tmpstr, '(');

  retval = tmpstr->str;

  g_string_free(tmpstr, FALSE);

  return retval;
}

static OIDL_Marshal_Node *
marshal_populate(IDL_tree tree, OIDL_Marshal_Node *parent, gboolean is_out, OIDL_Populate_Info *pi)
{
  OIDL_Marshal_Node *retval = NULL;

  if(!tree) return NULL;

  switch(IDL_NODE_TYPE(tree)) {
  case IDLN_INTEGER:
    retval = oidl_marshal_node_new(parent, MARSHAL_CONST, NULL);
    retval->u.const_info.amount = IDL_INTEGER(tree).value;
    retval->tree = tree;
    retval->flags |= MN_NOMARSHAL;
    break;
  case IDLN_TYPE_OCTET:
    retval = oidl_marshal_node_new(parent, MARSHAL_DATUM, NULL);
    retval->u.datum_info.datum_size = sizeof(CORBA_octet);
    retval->tree = tree;
    break;
  case IDLN_TYPE_BOOLEAN:
    retval = oidl_marshal_node_new(parent, MARSHAL_DATUM, NULL);
    retval->u.datum_info.datum_size = sizeof(CORBA_boolean);
    retval->tree = tree;
    break;
  case IDLN_TYPE_CHAR:
    retval = oidl_marshal_node_new(parent, MARSHAL_DATUM, NULL);
    retval->u.datum_info.datum_size = sizeof(CORBA_char);
    retval->tree = tree;
    break;
  case IDLN_TYPE_WIDE_CHAR:
    retval = oidl_marshal_node_new(parent, MARSHAL_DATUM, NULL);
    retval->u.datum_info.datum_size = sizeof(CORBA_wchar);
    retval->tree = tree;
    break;
  case IDLN_TYPE_FLOAT:
    retval = oidl_marshal_node_new(parent, MARSHAL_DATUM, NULL);
    switch(IDL_TYPE_FLOAT(tree).f_type) {
    case IDL_FLOAT_TYPE_FLOAT:
      retval->u.datum_info.datum_size = sizeof(CORBA_float);
      break;
    case IDL_FLOAT_TYPE_DOUBLE:
      retval->u.datum_info.datum_size = sizeof(CORBA_double);
      break;
    case IDL_FLOAT_TYPE_LONGDOUBLE:
      retval->u.datum_info.datum_size = sizeof(CORBA_long_double);
      break;
    default:
      g_assert(0);
      break;
    }
    retval->tree = tree;
    break;
  case IDLN_TYPE_INTEGER:
    retval = oidl_marshal_node_new(parent, MARSHAL_DATUM, NULL);
    switch(IDL_TYPE_INTEGER(tree).f_type) {
    case IDL_INTEGER_TYPE_SHORT:
      retval->u.datum_info.datum_size = sizeof(CORBA_short);
      break;
    case IDL_INTEGER_TYPE_LONG:
      retval->u.datum_info.datum_size = sizeof(CORBA_long);
      break;
    case IDL_INTEGER_TYPE_LONGLONG:
      retval->u.datum_info.datum_size = sizeof(CORBA_long_long);
      break;
    default:
      g_assert(0);
      break;
    }
    retval->tree = tree;
    break;
  case IDLN_TYPE_STRING:
  case IDLN_TYPE_WIDE_STRING:
    retval = oidl_marshal_node_new(parent, MARSHAL_LOOP, NULL);
    retval->flags |= MN_ISSTRING;
    retval->u.loop_info.length_var = oidl_marshal_node_new(retval, MARSHAL_DATUM, NULL);
    retval->u.loop_info.length_var->u.datum_info.datum_size = sizeof(CORBA_long);
    retval->u.loop_info.length_var->flags |= MN_NEED_TMPVAR;
    retval->u.loop_info.loop_var = oidl_marshal_node_new(retval, MARSHAL_DATUM, NULL);
    retval->u.loop_info.loop_var->u.datum_info.datum_size = sizeof(CORBA_long);
    retval->u.loop_info.loop_var->flags |= MN_NEED_TMPVAR|MN_NOMARSHAL|MN_LOOPED;
    retval->u.loop_info.contents = oidl_marshal_node_new(retval, MARSHAL_DATUM, NULL);
    retval->u.loop_info.contents->u.datum_info.datum_size = sizeof(CORBA_octet);
    retval->tree = tree;
    retval->flags &= ~(parent->flags & MN_LOOPED);
    break;
  case IDLN_TYPE_ENUM:
    retval = oidl_marshal_node_new(parent, MARSHAL_DATUM, NULL);
    retval->u.datum_info.datum_size = sizeof(CORBA_long);
    retval->tree = tree;
    break;
  case IDLN_TYPE_ARRAY:
    {
      OIDL_Marshal_Node *cursub = parent, *newsub;
      IDL_tree curlevel;

      for(curlevel = IDL_TYPE_ARRAY(tree).size_list; curlevel; curlevel = IDL_LIST(curlevel).next) {
	newsub = oidl_marshal_node_new(cursub, MARSHAL_LOOP, NULL);
	if(cursub != parent)
	  newsub->flags |= MN_LOOPED;

	if(!retval)
	  retval = newsub;

	cursub->u.loop_info.contents = newsub;
	cursub = newsub;

	cursub->u.loop_info.loop_var = oidl_marshal_node_new(cursub, MARSHAL_DATUM, NULL);
	cursub->u.loop_info.loop_var->u.datum_info.datum_size = sizeof(CORBA_long);
	cursub->u.loop_info.loop_var->flags |= MN_NOMARSHAL|MN_NEED_TMPVAR;

	cursub->u.loop_info.length_var = marshal_populate(IDL_LIST(curlevel).data, cursub, is_out, pi);
	cursub->u.loop_info.length_var->flags |= MN_NOMARSHAL;
      }

      cursub->u.loop_info.contents = marshal_populate(orbit_idl_get_array_type(tree), cursub, is_out, pi);
    }
    retval->tree = tree;
    break;
  case IDLN_TYPE_SEQUENCE:
    retval = oidl_marshal_node_new(parent, MARSHAL_LOOP, NULL);
    retval->flags |= MN_ISSEQ|MN_LOOPED;

    retval->u.loop_info.loop_var = oidl_marshal_node_new(retval, MARSHAL_DATUM, NULL);
    retval->u.loop_info.loop_var->u.datum_info.datum_size = sizeof(CORBA_unsigned_long);
    retval->u.loop_info.loop_var->flags |= MN_NOMARSHAL|MN_NEED_TMPVAR;

    retval->u.loop_info.length_var = oidl_marshal_node_new(retval, MARSHAL_DATUM, NULL);
    retval->u.loop_info.length_var->u.datum_info.datum_size = sizeof(CORBA_unsigned_long);
    retval->u.loop_info.length_var->name = "_length";

    retval->u.loop_info.contents = marshal_populate(IDL_TYPE_SEQUENCE(tree).simple_type_spec, retval, is_out, pi);
    retval->u.loop_info.contents->flags |= MN_LOOPED;
    retval->tree = tree;
    retval->flags &= ~(parent->flags & MN_LOOPED);
    break;
  case IDLN_TYPE_STRUCT:
  case IDLN_EXCEPT_DCL:
    {
      IDL_tree curitem;
      retval = oidl_marshal_node_new(parent, MARSHAL_SET, NULL);

      for(curitem = IDL_TYPE_STRUCT(tree).member_list; curitem; curitem = IDL_LIST(curitem).next) {
	retval->u.set_info.subnodes = g_slist_append(retval->u.set_info.subnodes,
						     marshal_populate(IDL_LIST(curitem).data, retval, is_out, pi));
      }
    }
    retval->tree = tree;
    break;
  case IDLN_MEMBER:
    {
      IDL_tree curitem, curnode;
      OIDL_Marshal_Node *tnode;

      retval = oidl_marshal_node_new(parent, MARSHAL_SET, NULL);
      for(curitem = IDL_MEMBER(tree).dcls; curitem; curitem = IDL_LIST(curitem).next) {
	curnode = IDL_LIST(curitem).data;

	if(IDL_NODE_TYPE(curnode) == IDLN_IDENT) {
	  tnode = marshal_populate(IDL_MEMBER(tree).type_spec, retval, is_out, pi);
	} else if(IDL_NODE_TYPE(curnode) == IDLN_TYPE_ARRAY) {
	  tnode = marshal_populate(curnode, retval, is_out, pi);
	} else
	  g_error("A member that is not an ident nor an array?");

	tnode->name = orbit_idl_member_get_name(curnode);

	retval->u.set_info.subnodes = g_slist_append(retval->u.set_info.subnodes, tnode);
      }
    }
    retval->tree = tree;
    break;
  case IDLN_TYPE_UNION:
    {
      IDL_tree ntmp;
      retval = oidl_marshal_node_new(parent, MARSHAL_SWITCH, NULL);
      retval->tree = tree;
      retval->u.switch_info.discrim = marshal_populate(IDL_TYPE_UNION(tree).switch_type_spec, retval, is_out, pi);
      retval->u.switch_info.discrim->name = "_d";
      for(ntmp = IDL_TYPE_UNION(tree).switch_body; ntmp; ntmp = IDL_LIST(ntmp).next) {
	OIDL_Marshal_Node * newnode;

	newnode = marshal_populate(IDL_LIST(ntmp).data, retval, is_out, pi);

	retval->u.switch_info.cases = g_slist_append(retval->u.switch_info.cases, newnode);
      }
    }
    break;
  case IDLN_CASE_STMT:
    {
      IDL_tree ntmp;
      retval = oidl_marshal_node_new(parent, MARSHAL_CASE, "_u");
      retval->u.case_info.contents = marshal_populate(IDL_CASE_STMT(tree).element_spec, retval, is_out, pi);
      for(ntmp = IDL_CASE_STMT(tree).labels; ntmp; ntmp = IDL_LIST(ntmp).next) {
	OIDL_Marshal_Node * newnode;

	newnode = marshal_populate(IDL_LIST(ntmp).data, retval, is_out, pi);

	retval->u.case_info.labels = g_slist_append(retval->u.case_info.labels, newnode);
      }

      retval->tree = tree;
    }
    break;
  case IDLN_IDENT:
    retval = marshal_populate(IDL_get_parent_node(tree, IDLN_ANY, NULL), parent, is_out, pi);
    retval->tree = tree;
    break;
  case IDLN_LIST:
    retval = marshal_populate(IDL_get_parent_node(tree, IDLN_ANY, NULL), parent, is_out, pi);
    break;
  case IDLN_TYPE_DCL:
    retval = marshal_populate(IDL_TYPE_DCL(tree).type_spec, parent, is_out, pi);
    break;
  case IDLN_PARAM_DCL:
    retval = marshal_populate(IDL_PARAM_DCL(tree).param_type_spec, parent, is_out, pi);
    g_assert(retval);
    g_assert(!retval->name);
    retval->name = IDL_IDENT(IDL_PARAM_DCL(tree).simple_declarator).str;
    break;
  case IDLN_TYPE_FIXED:
    retval = oidl_marshal_node_new(parent, MARSHAL_COMPLEX, NULL);
    retval->u.complex_info.type = CX_CORBA_FIXED;
    retval->tree = tree;
    break;
  case IDLN_TYPE_ANY:
    retval = oidl_marshal_node_new(parent, MARSHAL_COMPLEX, NULL);
    retval->u.complex_info.type = CX_CORBA_ANY;
    retval->tree = tree;
    break;
  case IDLN_TYPE_TYPECODE:
    retval = oidl_marshal_node_new(parent, MARSHAL_COMPLEX, NULL);
    retval->u.complex_info.type = CX_CORBA_TYPECODE;
    retval->tree = tree;
    break;
  case IDLN_TYPE_OBJECT:
  case IDLN_INTERFACE:
    retval = oidl_marshal_node_new(parent, MARSHAL_COMPLEX, NULL);
    retval->u.complex_info.type = CX_CORBA_OBJECT;
    retval->tree = tree;
    break;
  case IDLN_CHAR:
  case IDLN_WIDE_CHAR:
  case IDLN_STRING:
  case IDLN_WIDE_STRING:
  case IDLN_FIXED:
  case IDLN_FLOAT:
  case IDLN_BOOLEAN:
    retval = oidl_marshal_node_new(parent, MARSHAL_CONST, NULL);
    retval->tree = tree;
    break;
  default:
    g_warning("Not populating for %s", IDL_tree_type_names[IDL_NODE_TYPE(tree)]);
    break;
  }

  g_return_val_if_fail(retval, retval);

  return retval;
}

OIDL_Marshal_Node *
orbit_idl_marshal_populate_in(IDL_tree tree, gboolean is_skels)
{
  OIDL_Marshal_Node *retval;
  IDL_tree curitem, curparam, ts;
  OIDL_Populate_Info pi;

  g_assert(IDL_NODE_TYPE(tree) == IDLN_OP_DCL);

  if(!(IDL_OP_DCL(tree).parameter_dcls
       || IDL_OP_DCL(tree).context_expr)) return NULL;

  retval = oidl_marshal_node_new(NULL, MARSHAL_SET, NULL);

  for(curitem = IDL_OP_DCL(tree).parameter_dcls; curitem; curitem = IDL_LIST(curitem).next) {
    OIDL_Marshal_Node *sub;


    curparam = IDL_LIST(curitem).data;

    if(IDL_PARAM_DCL(curparam).attr == IDL_PARAM_OUT)
      continue;

    pi.visited = g_hash_table_new(g_direct_hash, g_direct_equal);
    sub = marshal_populate(curparam, retval, is_skels, &pi);
    g_hash_table_destroy(pi.visited);

    ts = orbit_cbe_get_typespec(curparam);

    switch(IDL_PARAM_DCL(curparam).attr) {
    case IDL_PARAM_INOUT:
      if(is_skels) {
	sub->nptrs = MAX(oidl_param_numptrs(curparam, DATA_INOUT) - 1, 0);
      } else {
	sub->nptrs = oidl_param_numptrs(curparam, DATA_INOUT);
      }
      sub->flags |= MN_DEMARSHAL_CORBA_ALLOC|MN_DEMARSHAL_USER_MOD;
      break;
    case IDL_PARAM_IN:
      if(is_skels) {
	sub->nptrs = MAX(oidl_param_numptrs(curparam, DATA_IN) - 1, 0);
	if(orbit_cbe_type_contains_complex(IDL_PARAM_DCL(curparam).param_type_spec))
	  sub->flags |= MN_DEMARSHAL_CORBA_ALLOC;
      } else
	sub->nptrs = oidl_param_numptrs(curparam, DATA_IN);
      break;
    default:
      g_error("Weird param direction for in pass.");
      break;
    }

    retval->u.set_info.subnodes = g_slist_append(retval->u.set_info.subnodes, sub);
  }

  if(IDL_OP_DCL(tree).context_expr) {
    OIDL_Marshal_Node *mnode;
    int i;
    IDL_tree curitem;

    for(i = 0, curitem = IDL_OP_DCL(tree).context_expr; curitem; curitem = IDL_LIST(curitem).next, i++) /* */ ;

    mnode = oidl_marshal_node_new(retval, MARSHAL_COMPLEX, NULL);
    mnode->u.complex_info.type = CX_CORBA_CONTEXT;
    mnode->u.complex_info.context_item_count = i;

    retval->u.set_info.subnodes = g_slist_append(retval->u.set_info.subnodes, mnode);
  }

  return retval;
}

OIDL_Marshal_Node *
orbit_idl_marshal_populate_out(IDL_tree tree, gboolean is_skels)
{
  OIDL_Marshal_Node *retval, *rvnode;
  IDL_tree curitem, curparam;
  OIDL_Populate_Info pi;

  g_assert(IDL_NODE_TYPE(tree) == IDLN_OP_DCL);

  retval = oidl_marshal_node_new(NULL, MARSHAL_SET, NULL);

  if(IDL_OP_DCL(tree).op_type_spec) {
    pi.visited = g_hash_table_new(g_direct_hash, g_direct_equal);
    rvnode = marshal_populate(IDL_OP_DCL(tree).op_type_spec, retval, !is_skels, &pi);
    g_hash_table_destroy(pi.visited);
    if(!rvnode) goto out1;

    rvnode->nptrs = oidl_param_numptrs(IDL_OP_DCL(tree).op_type_spec, DATA_RETURN);
    rvnode->flags |= MN_NEED_TMPVAR|MN_DEMARSHAL_CORBA_ALLOC;

    retval->u.set_info.subnodes = g_slist_append(retval->u.set_info.subnodes, rvnode);

    g_assert(! rvnode->name);

    rvnode->name = ORBIT_RETVAL_VAR_NAME;
  }

 out1:

  for(curitem = IDL_OP_DCL(tree).parameter_dcls; curitem; curitem = IDL_LIST(curitem).next) {
    OIDL_Marshal_Node *sub;

    curparam = IDL_LIST(curitem).data;

    if(IDL_PARAM_DCL(curparam).attr == IDL_PARAM_IN)
      continue;

    pi.visited = g_hash_table_new(g_direct_hash, g_direct_equal);
    sub = marshal_populate(curparam, retval, !is_skels, &pi);
    g_hash_table_destroy(pi.visited);

    switch(IDL_PARAM_DCL(curparam).attr) {
    case IDL_PARAM_INOUT:
      sub->flags |= MN_DEMARSHAL_CORBA_ALLOC;
      if(is_skels)
	sub->nptrs = MAX(oidl_param_numptrs(curparam, DATA_INOUT) - 1, 0);
      else
	sub->nptrs = oidl_param_numptrs(curparam, DATA_INOUT);
      break;
    case IDL_PARAM_OUT:
      if(is_skels)
	sub->nptrs = MAX(oidl_param_numptrs(curparam, DATA_OUT) - 1, 0);
      else
	sub->nptrs = oidl_param_numptrs(curparam, DATA_OUT);
      break;
    default:
      g_error("Weird param direction for out pass.");
      break;
    }

    retval->u.set_info.subnodes = g_slist_append(retval->u.set_info.subnodes, sub);
  }

  return retval;
}

OIDL_Marshal_Node *
orbit_idl_marshal_populate_except_marshal(IDL_tree tree)
{
  OIDL_Marshal_Node *retval;
  OIDL_Populate_Info pi;

  pi.visited = g_hash_table_new(g_direct_hash, g_direct_equal);
  retval = marshal_populate(tree, NULL, FALSE, &pi);
  g_hash_table_destroy(pi.visited);
  retval->name = "_ORBIT_exdata";
  retval->nptrs = 1;

  return retval;
}

OIDL_Marshal_Node *
orbit_idl_marshal_populate_except_demarshal(IDL_tree tree)
{
  OIDL_Marshal_Node *retval;
  OIDL_Populate_Info pi;

  pi.visited = g_hash_table_new(g_direct_hash, g_direct_equal);
  retval = marshal_populate(tree, NULL, TRUE, &pi);
  g_hash_table_destroy(pi.visited);
  retval->name = "_ORBIT_exdata";
  retval->nptrs = 1;

  return retval;
}
