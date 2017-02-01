#include "config.h"

#include "orbit-idl-c-backend.h"

#include <string.h>

void
orbit_cbe_write_typespec(FILE *of, IDL_tree tree)
{
  if(!tree) {
    fprintf(of, "void");
    return;
  }

  switch(IDL_NODE_TYPE(tree)) {
  case IDLN_TYPE_FLOAT:
    switch(IDL_TYPE_FLOAT(tree).f_type) {
    case IDL_FLOAT_TYPE_FLOAT:
      fprintf(of, "CORBA_float");
      break;
    case IDL_FLOAT_TYPE_DOUBLE:
      fprintf(of, "CORBA_double");
      break;
    case IDL_FLOAT_TYPE_LONGDOUBLE:
      fprintf(of, "CORBA_long_double");
      break;
    };
    break;
  case IDLN_TYPE_BOOLEAN:
    fprintf(of, "CORBA_boolean");
    break;
  case IDLN_TYPE_FIXED:
    fprintf(of, "CORBA_fixed_%" IDL_LL "d_%" IDL_LL "d",
	    IDL_INTEGER(IDL_TYPE_FIXED(tree).positive_int_const).value,
	    IDL_INTEGER(IDL_TYPE_FIXED(tree).integer_lit).value);
    break;
  case IDLN_TYPE_INTEGER:
    fprintf(of, "CORBA_");
    if(!IDL_TYPE_INTEGER(tree).f_signed)
      fprintf(of, "unsigned_");
    switch(IDL_TYPE_INTEGER(tree).f_type) {
    case IDL_INTEGER_TYPE_SHORT:
      fprintf(of, "short");
      break;
    case IDL_INTEGER_TYPE_LONGLONG:
      fprintf(of, "long_");
    case IDL_INTEGER_TYPE_LONG:
      fprintf(of, "long");
      break;
    }
    break;
  case IDLN_TYPE_STRING:
    fprintf(of, "CORBA_char *");
    break;
  case IDLN_TYPE_OCTET:
    fprintf(of, "CORBA_octet");
    break;
  case IDLN_TYPE_WIDE_STRING:
    fprintf(of, "CORBA_wchar *");
    break;
  case IDLN_TYPE_CHAR:
    fprintf(of, "CORBA_char");
    break;
  case IDLN_TYPE_WIDE_CHAR:
    fprintf(of, "CORBA_wchar");
    break;
  case IDLN_TYPE_STRUCT:
  case IDLN_TYPE_ARRAY:    
  case IDLN_TYPE_UNION:
  case IDLN_TYPE_ENUM:
  case IDLN_EXCEPT_DCL:
    {
      char *id = IDL_ns_ident_to_qstring(IDL_IDENT_TO_NS(IDL_TYPE_ENUM(tree).ident), "_", 0);
      fprintf(of, "%s", id);
      g_free(id);
    }
    break;
  case IDLN_IDENT:
    {
      char *id = IDL_ns_ident_to_qstring(IDL_IDENT_TO_NS(tree), "_", 0);
      fprintf(of, "%s", id);
      g_free(id);
    }
    break;
  case IDLN_TYPE_OBJECT:
    fprintf(of, "CORBA_Object");
    break;
  case IDLN_TYPE_SEQUENCE:
    {
      char *ctmp;
      ctmp = orbit_cbe_get_typename(tree);
      fprintf(of, "%s", ctmp);
    }
    break;
  case IDLN_TYPE_ANY:
    fprintf(of, "CORBA_any");
    break;
  case IDLN_NATIVE:
    fprintf(of, "gpointer");
    break;
  case IDLN_TYPE_TYPECODE:
    fprintf(of, "CORBA_TypeCode");
    break;
  default:
    g_error("We were asked to print a typespec %s", IDL_tree_type_names[tree->_type]);
  }
}

IDL_tree
orbit_cbe_get_efftype(IDL_tree tree)
{
  switch(IDL_NODE_TYPE(tree)) {
  case IDLN_IDENT:
  case IDLN_LIST:
    return orbit_cbe_get_efftype(IDL_get_parent_node(tree, IDLN_ANY, NULL));
    break;
  case IDLN_TYPE_DCL:
    return orbit_cbe_get_efftype(IDL_TYPE_DCL(tree).type_spec);
    break;
  default:
    return tree;
    break;
  }
}

char *
orbit_cbe_get_typename(IDL_tree tree)
{
  GString *tmpstr;
  char *retval = NULL;

  if(!tree) {
    return g_strdup("void");
  }

  tmpstr = g_string_new(NULL);

  switch(IDL_NODE_TYPE(tree)) {
  case IDLN_TYPE_ANY:
    retval = "CORBA_any";
    break;
  case IDLN_TYPE_FLOAT:
    switch(IDL_TYPE_FLOAT(tree).f_type) {
    case IDL_FLOAT_TYPE_FLOAT:
      retval = "CORBA_float";
      break;
    case IDL_FLOAT_TYPE_DOUBLE:
      retval = "CORBA_double";
      break;
    case IDL_FLOAT_TYPE_LONGDOUBLE:
      retval = "CORBA_long_double";
      break;
    }
    break;
  case IDLN_TYPE_FIXED:
    g_string_sprintf(tmpstr, "CORBA_fixed_%" IDL_LL "d_%" IDL_LL "d",
		     IDL_INTEGER(IDL_TYPE_FIXED(tree).positive_int_const).value,
		     IDL_INTEGER(IDL_TYPE_FIXED(tree).integer_lit).value);
    break;
  case IDLN_TYPE_INTEGER:
    g_string_assign(tmpstr, "CORBA_");
    if(!IDL_TYPE_INTEGER(tree).f_signed)
      g_string_append(tmpstr, "unsigned_");

    switch(IDL_TYPE_INTEGER(tree).f_type) {
    case IDL_INTEGER_TYPE_SHORT:
      g_string_append(tmpstr, "short");
      break;
    case IDL_INTEGER_TYPE_LONGLONG:
      g_string_append(tmpstr, "long_");
    case IDL_INTEGER_TYPE_LONG:
      g_string_append(tmpstr, "long");
      break;
    }
    break;
  case IDLN_TYPE_STRING:
    retval = "CORBA_string";
    break;
  case IDLN_TYPE_OCTET:
    retval = "CORBA_octet";
    break;
  case IDLN_TYPE_WIDE_STRING:
    retval = "CORBA_wstring";
    break;
  case IDLN_TYPE_CHAR:
    retval = "CORBA_char";
    break;
  case IDLN_TYPE_WIDE_CHAR:
    retval = "CORBA_wchar";
    break;
  case IDLN_TYPE_BOOLEAN:
    retval = "CORBA_boolean";
    break;
  case IDLN_TYPE_STRUCT:
    retval = orbit_cbe_get_typename(IDL_TYPE_STRUCT(tree).ident);
    break;
  case IDLN_EXCEPT_DCL:
    retval = orbit_cbe_get_typename(IDL_EXCEPT_DCL(tree).ident);
    break;
  case IDLN_TYPE_ARRAY:
    retval = orbit_cbe_get_typename(IDL_TYPE_ARRAY(tree).ident);
    break;
  case IDLN_TYPE_UNION:
    retval = orbit_cbe_get_typename(IDL_TYPE_UNION(tree).ident);
    break;
  case IDLN_TYPE_ENUM:
    retval = orbit_cbe_get_typename(IDL_TYPE_ENUM(tree).ident);
    break;
  case IDLN_IDENT:
    {
      char *id = IDL_ns_ident_to_qstring(IDL_IDENT_TO_NS(tree), "_", 0);
      g_string_assign(tmpstr, id);
      g_free(id);
    }
    break;
  case IDLN_PARAM_DCL:
    retval = orbit_cbe_get_typename(IDL_PARAM_DCL(tree).param_type_spec);
    g_string_assign(tmpstr, retval); g_free(retval); retval = NULL;
    break;
  case IDLN_TYPE_SEQUENCE:
    {
      char *ctmp;

      ctmp = orbit_cbe_get_typename(IDL_TYPE_SEQUENCE(tree).simple_type_spec);

      if(orbit_cbe_type_is_builtin(IDL_TYPE_SEQUENCE(tree).simple_type_spec))
	memmove(ctmp, ctmp + strlen("CORBA_"), strlen(ctmp + strlen("CORBA_")) + 1);

      g_string_sprintf(tmpstr, "CORBA_sequence_%s", ctmp);
      g_free(ctmp);
    }
    break;
  case IDLN_INTERFACE:
  case IDLN_FORWARD_DCL:
    retval = orbit_cbe_get_typename(IDL_INTERFACE(tree).ident);
    g_string_assign(tmpstr, retval); g_free(retval); retval = NULL;
    break;
  case IDLN_NATIVE:
    retval = "gpointer";
    break;
  case IDLN_TYPE_OBJECT:
    retval = "CORBA_Object";
    break;
  case IDLN_TYPE_TYPECODE:
    retval = "CORBA_TypeCode";
    break;
  default:
    g_error("We were asked to get a typename for a %s",
	    IDL_tree_type_names[IDL_NODE_TYPE(tree)]);
  }

  if(retval)
    return g_strdup(retval);
  else {
    retval = tmpstr->str;
    g_string_free(tmpstr, FALSE);
    return retval;
  }
}

void
orbit_cbe_param_printptrs(FILE *of, IDL_tree param, IDL_ParamRole role)
{
  int i, n;
  IDL_tree p2;

  if(param == NULL)
    return;

  p2 = orbit_cbe_get_typespec(param);
  n = oidl_param_numptrs(p2, role);

#if 0
  if(IDL_NODE_TYPE(p2) == IDLN_TYPE_ARRAY
     && ((role == DATA_OUT) || (role == DATA_RETURN))) {
    if(role == DATA_RETURN)
      fprintf(of, "_slice*");
    else if(!cbe_type_is_fixed_length(p2)) /* && role == DATA_OUT */
      fprintf(of, "_slice*");
  }
#endif

  for(i = 0; i < n; i++)
    fprintf(of, "*");
}

void
orbit_cbe_op_write_proto(FILE *of,
			 IDL_tree op,
			 const char *nom_prefix,
			 gboolean for_epv)
{
  IDL_tree sub, ttmp;
  char *id;

  if(IDL_OP_DCL(op).op_type_spec) {
    orbit_cbe_write_typespec(of, IDL_OP_DCL(op).op_type_spec);
    
    ttmp = IDL_NODE_UP(IDL_OP_DCL(op).op_type_spec);
    if(IDL_NODE_TYPE(ttmp) == IDLN_TYPE_ARRAY)
      fprintf(of, "_slice*");
  } else
    fprintf(of, "void");

  orbit_cbe_param_printptrs(of, IDL_OP_DCL(op).op_type_spec, DATA_RETURN);

  id = IDL_ns_ident_to_qstring(IDL_IDENT_TO_NS(IDL_INTERFACE(IDL_get_parent_node(op, IDLN_INTERFACE, NULL)).ident), "_", 0);

  if(for_epv) {
    fprintf(of, " (*%s%s)", nom_prefix?nom_prefix:"",
	    IDL_IDENT(IDL_OP_DCL(op).ident).str);
  } else {
    fprintf(of, " %s%s_%s", nom_prefix?nom_prefix:"",
	    id,
	    IDL_IDENT(IDL_OP_DCL(op).ident).str);
  }

  fprintf(of, "(");

  if(for_epv)
    fprintf(of, "PortableServer_Servant _servant, ");
  else
    fprintf(of, "%s _obj, ", id);

  g_free(id);

  for(sub = IDL_OP_DCL(op).parameter_dcls; sub; sub = IDL_LIST(sub).next) {
    IDL_tree parm;

    parm = IDL_LIST(sub).data;

    if (IDL_PARAM_DCL(parm).attr == IDL_PARAM_IN)
      fprintf (of, "const ");

    orbit_cbe_write_typespec(of, IDL_PARAM_DCL(parm).param_type_spec);

    ttmp = IDL_NODE_UP(IDL_PARAM_DCL(parm).param_type_spec);
    if((IDL_NODE_TYPE(ttmp) == IDLN_TYPE_ARRAY)
       && (IDL_PARAM_DCL(parm).attr == IDL_PARAM_OUT)
       && !orbit_cbe_type_is_fixed_length(ttmp))
      fprintf(of, "_slice*");

    {
      IDL_ParamRole r;

      switch(IDL_PARAM_DCL(parm).attr) {
      case IDL_PARAM_IN: r = DATA_IN; break;
      case IDL_PARAM_INOUT: r = DATA_INOUT; break;
      case IDL_PARAM_OUT: r = DATA_OUT; break;
      default:
	g_error("Unknown IDL_PARAM type");
      }

      orbit_cbe_param_printptrs(of, IDL_PARAM_DCL(parm).param_type_spec, r);
    }

    fprintf(of, " %s, ", IDL_IDENT(IDL_PARAM_DCL(parm).simple_declarator).str);
  }

  if(IDL_OP_DCL(op).context_expr)
    fprintf(of, "CORBA_Context _ctx, ");

  fprintf(of, "CORBA_Environment *ev)");
}

/* Writes the value of the constant in 'tree' to file handle 'of' */
char *
orbit_cbe_get_const(IDL_tree tree)
{
  char *opc = NULL, *retval, *ctmp;
  GString *tmpstr = g_string_new(NULL);

  switch(IDL_NODE_TYPE(tree)) {
  case IDLN_BOOLEAN:
    g_string_sprintf(tmpstr, "%s", IDL_BOOLEAN(tree).value?"CORBA_TRUE":"CORBA_FALSE");
    break;
  case IDLN_CHAR:
    g_string_sprintf(tmpstr, "'%s'", IDL_CHAR(tree).value);
    break;
  case IDLN_FLOAT:
    g_string_sprintf(tmpstr, "%f", IDL_FLOAT(tree).value);
    break;
  case IDLN_INTEGER:
    g_string_sprintf(tmpstr, "%" IDL_LL "d", IDL_INTEGER(tree).value);
    break;
  case IDLN_STRING:
    g_string_sprintf(tmpstr, "\"%s\"", IDL_STRING(tree).value);
    break;
  case IDLN_WIDE_CHAR:
    g_string_sprintf(tmpstr, "L'%ls'", IDL_WIDE_CHAR(tree).value);
    break;
  case IDLN_WIDE_STRING:
    g_string_sprintf(tmpstr, "L\"%ls\"", IDL_WIDE_STRING(tree).value);
    break;
  case IDLN_BINOP:
    g_string_sprintf(tmpstr, "(");
    ctmp = orbit_cbe_get_const(IDL_BINOP(tree).left);
    g_string_append(tmpstr, ctmp);
    g_free(ctmp);
    switch(IDL_BINOP(tree).op) {
    case IDL_BINOP_OR:
      opc = "|";
      break;
    case IDL_BINOP_XOR:
      opc = "^";
      break;
    case IDL_BINOP_AND:
      opc = "&";
      break;
    case IDL_BINOP_SHR:
      opc = ">>";
      break;
    case IDL_BINOP_SHL:
      opc = "<<";
      break;
    case IDL_BINOP_ADD:
      opc = "+";
      break;
    case IDL_BINOP_SUB:
      opc = "-";
      break;
    case IDL_BINOP_MULT:
      opc = "*";
      break;
    case IDL_BINOP_DIV:
      opc = "/";
      break;
    case IDL_BINOP_MOD:
      opc = "%";
      break;
    }
    g_string_sprintfa(tmpstr, " %s ", opc);
    ctmp = orbit_cbe_get_const(IDL_BINOP(tree).right);
    g_string_sprintfa(tmpstr, "%s)", ctmp);
    g_free(ctmp);
    break;
  case IDLN_UNARYOP:
    switch(IDL_UNARYOP(tree).op) {
    case IDL_UNARYOP_PLUS: opc = "+"; break;
    case IDL_UNARYOP_MINUS: opc = "-"; break;
    case IDL_UNARYOP_COMPLEMENT: opc = "~"; break;
    }
    ctmp = orbit_cbe_get_const(IDL_UNARYOP(tree).operand);
    g_string_sprintf(tmpstr, "%s%s", opc, ctmp);
    g_free(ctmp);
    break;
  case IDLN_IDENT:
    {
      char *id;
      id = IDL_ns_ident_to_qstring(IDL_IDENT_TO_NS(tree), "_", 0);
      g_string_sprintf(tmpstr, "%s", id);
      g_free(id);
    }
    break;
  default:
    g_error("We were asked to print a constant for %s", IDL_tree_type_names[tree->_type]);
    break;
  }

  retval = tmpstr->str;

  g_string_free(tmpstr, FALSE);

  return retval;
}

void
orbit_cbe_write_const(FILE *of, IDL_tree tree)
{
  char *ctmp;

  ctmp = orbit_cbe_get_const(tree);
  fprintf(of, "%s", ctmp);
  g_free(ctmp);
}

char *
orbit_cbe_get_const_node(OIDL_Marshal_Node *node)
{
  if(node->tree)
    return orbit_cbe_get_const(node->tree);
  else
    return g_strdup_printf("%d", node->u.const_info.amount);
}

void
orbit_cbe_write_const_node(FILE *of, OIDL_Marshal_Node *node)
{
  char *ctmp;

  ctmp = orbit_cbe_get_const_node(node);
  fprintf(of, "%s", ctmp);
  g_free(ctmp);
}

char *oidl_marshal_node_valuestr(OIDL_Marshal_Node *node)
{
  if(node->type == MARSHAL_CONST)
    return orbit_cbe_get_const_node(node);
  else
    return oidl_marshal_node_fqn(node);
}

void
orbit_cbe_write_node_typespec(FILE *of, OIDL_Marshal_Node *node)
{
  if(node->tree)
    orbit_cbe_write_typespec(of, node->tree);
  else if(node->type == MARSHAL_DATUM) {
    static const char * const size_names[] = {NULL, "CORBA_unsigned_char", "CORBA_unsigned_short", NULL, "CORBA_unsigned_long",
					      NULL, NULL, NULL, "CORBA_unsigned_long_long"};
    const char *ctmp;
    ctmp = size_names[node->u.datum_info.datum_size];
    fprintf(of, "%s", ctmp);
  } else if(node->type == MARSHAL_COMPLEX) {
    switch(node->u.complex_info.type) {
    case CX_CORBA_CONTEXT:
      fprintf(of, "CORBA_Context");
      break;
    default:
      break;
    }
  } else
    g_error("Don't know how to write a typespec for node type %d.", node->type);
}

gboolean
orbit_cbe_type_is_builtin(IDL_tree tree)
{ 
  return FALSE;
  switch(IDL_NODE_TYPE(tree)) {
  case IDLN_LIST:
  case IDLN_GENTREE:
  case IDLN_MEMBER:
  case IDLN_NATIVE:
  case IDLN_CASE_STMT:
  case IDLN_MODULE:
  case IDLN_BINOP:
  case IDLN_UNARYOP:
  case IDLN_CODEFRAG:
    g_error("Strange type for being a builtin");
    break;
  case IDLN_INTEGER:
  case IDLN_STRING:
  case IDLN_WIDE_STRING:
  case IDLN_CHAR:
  case IDLN_WIDE_CHAR:
  case IDLN_FIXED:
  case IDLN_FLOAT:
  case IDLN_BOOLEAN:
  case IDLN_CONST_DCL:
  case IDLN_TYPE_INTEGER:
  case IDLN_TYPE_FLOAT:
  case IDLN_TYPE_CHAR:
  case IDLN_TYPE_WIDE_CHAR:
  case IDLN_TYPE_STRING:
  case IDLN_TYPE_WIDE_STRING:
  case IDLN_TYPE_BOOLEAN:
  case IDLN_TYPE_OCTET:
  case IDLN_TYPE_ANY:
  case IDLN_TYPE_OBJECT:
  case IDLN_TYPE_TYPECODE:
  case IDLN_TYPE_ENUM:
    return TRUE;
    break;
  case IDLN_TYPE_DCL:
  case IDLN_EXCEPT_DCL:
  case IDLN_ATTR_DCL:
  case IDLN_OP_DCL:
  case IDLN_PARAM_DCL:
  case IDLN_TYPE_FIXED:
  case IDLN_TYPE_SEQUENCE:
  case IDLN_TYPE_ARRAY:
  case IDLN_TYPE_STRUCT:
  case IDLN_TYPE_UNION:
  case IDLN_IDENT:
  case IDLN_INTERFACE:
  case IDLN_FORWARD_DCL:
  default:
    return FALSE;
    break;
  }

  return FALSE;
}

/* This is the WORST HACK in the WORLD, really truly, but the C preprocessor doesn't allow us to use
   strings, so we have to work around it by using individual characters. */
void
orbit_cbe_id_define_hack(FILE *fh, const char *def_prefix, const char *def_name, const char *def_value)
{
  int i, n;
  n = strlen(def_value);
  for(i = 0; i < n; i++)
    fprintf(fh, "#define %s_%s_%d '%c'\n", def_prefix, def_name, i, def_value[i]);
}

void
orbit_cbe_id_cond_hack(FILE *fh, const char *def_prefix, const char *def_name, const char *def_value)
{
  int i, n;
  n = strlen(def_value);
  if(n <= 0)
    return;

  fprintf(fh, "(");

  for(i = 0; i < n; i++)
    fprintf(fh, "%s (%s_%s_%d == '%c') \\\n", i?"&&":"", def_prefix, def_name, i, def_value[i]);
  fprintf(fh, ")");
}
