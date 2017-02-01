#include "orbit-idl-c-backend.h"

#include <stdlib.h>
#include <string.h>

typedef struct {
  FILE *of;
  IDL_tree ts;
  char *structname, *substructname;
  int array_gen_ctr;
  OIDL_C_Info *ci;
} CBETCGenInfo;

static int random_id = 0;

static char *cbe_tc_generate_tcstruct_name(IDL_tree ts);
static void cbe_tc_generate(CBETCGenInfo *tci);

void
orbit_output_typecode(OIDL_C_Info *ci,
		      IDL_tree ts)
{
  CBETCGenInfo tci;

  switch(IDL_NODE_TYPE(ts)) {
  case IDLN_TYPE_DCL:
  case IDLN_TYPE_STRUCT:
  case IDLN_TYPE_UNION:
  case IDLN_TYPE_ENUM:
  case IDLN_EXCEPT_DCL:
  case IDLN_TYPE_SEQUENCE:
  case IDLN_INTERFACE:
    break;
  default:
    g_error("You can't produce a typecode for a %s", IDL_tree_type_names[IDL_NODE_TYPE(ts)]);
  }

  memset(&tci, 0, sizeof(tci));
  tci.of = ci->fh;
  tci.ts = ts;
  tci.ci = ci;
  tci.structname = cbe_tc_generate_tcstruct_name(ts);
  cbe_tc_generate(&tci);
  g_free(tci.structname);
}

static char *
cbe_tc_generate_tcstruct_name(IDL_tree ts)
{
  GString *tmpstr;
  char *retval;

  tmpstr = g_string_new(NULL);

  switch(IDL_NODE_TYPE(ts)) {
  case IDLN_TYPE_INTEGER:
  case IDLN_TYPE_ANY:
  case IDLN_TYPE_STRING:
  case IDLN_TYPE_WIDE_STRING:
  case IDLN_TYPE_CHAR:
  case IDLN_TYPE_WIDE_CHAR:
  case IDLN_TYPE_FLOAT:
  case IDLN_TYPE_BOOLEAN:
  case IDLN_TYPE_OCTET:
  case IDLN_TYPE_SEQUENCE:
  case IDLN_TYPE_STRUCT:
  case IDLN_TYPE_UNION:
  case IDLN_TYPE_ENUM:
  case IDLN_IDENT:
  case IDLN_EXCEPT_DCL:
  case IDLN_INTERFACE:
  case IDLN_FORWARD_DCL:
  case IDLN_TYPE_OBJECT:
    retval = orbit_cbe_get_typename(ts);
    g_string_sprintf(tmpstr, "TC_%s", retval);
    g_free(retval);
    break;
  default:
    /* Generate a random one */
    g_string_sprintf(tmpstr, "anonTC_%d", random_id++);
    break;
  }

  retval = tmpstr->str;
  g_string_free(tmpstr, FALSE);

  return retval;
}

static void
cbe_tc_generate(CBETCGenInfo *tci)
{
  IDL_tree curitem;
  int n;
  char *ctmp;
  CBETCGenInfo subtci;
  int union_sublabels_array_ctr=random_id++, union_default_index = -1;
  int subnames_id = random_id++,
    subtypes_id = random_id++, sublabels_id = random_id++;

  if(strncmp(tci->structname, "anon", 4)) {
    fprintf(tci->of, "#if ");
    orbit_cbe_id_cond_hack(tci->of, "TC_IMPL", tci->structname, tci->ci->c_base_name);
    fprintf(tci->of, " && !defined(TC_DEF_%s)\n", tci->structname);
    fprintf(tci->of, "#define TC_DEF_%s 1\n", tci->structname);
  }

  if(IDL_NODE_TYPE(tci->ts) == IDLN_TYPE_DCL) {
    subtci = *tci;

    curitem = IDL_TYPE_DCL(tci->ts).type_spec;
    subtci.substructname = ctmp = cbe_tc_generate_tcstruct_name(curitem);

    /* The only type not already defined elsewhere that can be
       in the left half of a TypeCode */
    if(IDL_NODE_TYPE(curitem) == IDLN_TYPE_SEQUENCE) {
      subtci.structname = ctmp;
      subtci.ts = curitem;
      cbe_tc_generate(&subtci);
    }

    for(curitem = IDL_TYPE_DCL(tci->ts).dcls;
	curitem; curitem = IDL_LIST(curitem).next) {
      subtci.ts = IDL_LIST(curitem).data;
      if(IDL_NODE_TYPE(subtci.ts) == IDLN_TYPE_ARRAY)
	subtci.structname = cbe_tc_generate_tcstruct_name(IDL_TYPE_ARRAY(subtci.ts).ident);
      else
	subtci.structname = cbe_tc_generate_tcstruct_name(subtci.ts);
      cbe_tc_generate(&subtci);
      g_free(subtci.structname);
    }

    g_free(ctmp);
    return;
  }

  /* Do magic here - nesting of typecodes for arrays */
  if(IDL_NODE_TYPE(tci->ts) == IDLN_TYPE_ARRAY
     && (IDL_list_length(IDL_TYPE_ARRAY(tci->ts).size_list)
	 > tci->array_gen_ctr)) {

    curitem = IDL_list_nth(IDL_TYPE_ARRAY(tci->ts).size_list,
			   tci->array_gen_ctr-1);

    subtci = *tci;
    subtci.structname = ctmp = cbe_tc_generate_tcstruct_name(curitem);
    subtci.array_gen_ctr++;

    cbe_tc_generate(&subtci);

    tci->substructname = ctmp; /* XXX memory leak */
  }

  /* Do magic here - make some values to be used in the anys of sublabels */
  if(IDL_NODE_TYPE(tci->ts) == IDLN_TYPE_UNION
     && IDL_TYPE_UNION(tci->ts).switch_body) {
    ctmp = orbit_cbe_get_typename(IDL_TYPE_UNION(tci->ts).switch_type_spec);
    union_sublabels_array_ctr = random_id++;
    fprintf(tci->of, "static const %s anon_sublabel_values_array%d[] = {", ctmp,
	    union_sublabels_array_ctr);
    g_free(ctmp);

    n = 0;
    for(curitem = IDL_TYPE_UNION(tci->ts).switch_body; curitem;
	curitem = IDL_LIST(curitem).next) {
      IDL_tree curlabel;

      g_assert(IDL_NODE_TYPE(IDL_LIST(curitem).data) == IDLN_CASE_STMT);

      for(curlabel = IDL_CASE_STMT(IDL_LIST(curitem).data).labels;
	  curlabel;
	  curlabel = IDL_LIST(curlabel).next, n++) {

	if(!IDL_LIST(curlabel).data) { /* default case */
	  union_default_index = n;
	  continue;
	}

	orbit_cbe_write_const(tci->of, IDL_LIST(curlabel).data);

	if(IDL_LIST(curlabel).next || IDL_LIST(curitem).next)
	  fprintf(tci->of, ", ");
      }
    }
    fprintf(tci->of, "};\n");
  }


  /* subnames */
  switch(IDL_NODE_TYPE(tci->ts)) {
  case IDLN_TYPE_ENUM:
    if(!IDL_TYPE_ENUM(tci->ts).enumerator_list)
      break;
    fprintf(tci->of, "static const char *anon_subnames_array%d[] = {",
	    subnames_id);
    for(curitem = IDL_TYPE_ENUM(tci->ts).enumerator_list; curitem;
	curitem = IDL_LIST(curitem).next) {
      g_assert(IDL_NODE_TYPE(IDL_LIST(curitem).data) == IDLN_IDENT);
      fprintf(tci->of, "\"%s\"",
	      IDL_IDENT(IDL_LIST(curitem).data).str);
      if(IDL_LIST(curitem).next)
	fprintf(tci->of, ", ");
    }
    fprintf(tci->of, "};\n");
    break;
  case IDLN_EXCEPT_DCL:
  case IDLN_TYPE_STRUCT:
    if(!IDL_TYPE_STRUCT(tci->ts).member_list)
      break;
    fprintf(tci->of, "static const char *anon_subnames_array%d[] = {",
	    subnames_id);
    for(curitem = IDL_TYPE_STRUCT(tci->ts).member_list; curitem;
	curitem = IDL_LIST(curitem).next) {
      IDL_tree curdcl;

      g_assert(IDL_NODE_TYPE(IDL_LIST(curitem).data) == IDLN_MEMBER);
      for(curdcl = IDL_MEMBER(IDL_LIST(curitem).data).dcls; curdcl;
	  curdcl = IDL_LIST(curdcl).next) {
	IDL_tree p = IDL_LIST(curdcl).data;

	if (IDL_NODE_TYPE(p) == IDLN_IDENT)
	  fprintf(tci->of, "\"%s\"", IDL_IDENT(p).str);
	else if(IDL_NODE_TYPE(p) == IDLN_TYPE_ARRAY)
	  fprintf(tci->of, "\"%s\"",
		  IDL_IDENT(IDL_TYPE_ARRAY(p).ident).str);
	else
	  g_error("Unknown structure element.");
	
	if(IDL_LIST(curdcl).next || IDL_LIST(curitem).next)
	  fprintf(tci->of, ", ");
      }
    }
    fprintf(tci->of, "};\n");
    break;
  case IDLN_TYPE_UNION:
    if(!IDL_TYPE_UNION(tci->ts).switch_body)
      break;
    fprintf(tci->of, "static const char * anon_subnames_array%d[] = {",
	    subnames_id);
    for(curitem = IDL_TYPE_UNION(tci->ts).switch_body; curitem;
	curitem = IDL_LIST(curitem).next) {
      IDL_tree curdcl;

      g_assert(IDL_NODE_TYPE(IDL_LIST(curitem).data) == IDLN_CASE_STMT);

      curdcl = IDL_CASE_STMT(IDL_LIST(curitem).data).element_spec;
      curdcl = IDL_LIST(IDL_MEMBER(curdcl).dcls).data;

      if (IDL_NODE_TYPE(curdcl) == IDLN_IDENT)
	fprintf(tci->of, "\"%s\"", IDL_IDENT(curdcl).str);
      else if(IDL_NODE_TYPE(curdcl) == IDLN_TYPE_ARRAY)
	fprintf(tci->of, "\"%s\"",
		IDL_IDENT(IDL_TYPE_ARRAY(curdcl).ident).str);
      else
	g_error("Unknown union member.");

      if(IDL_LIST(curitem).next)
	fprintf(tci->of, ", ");
    }
    fprintf(tci->of, "};\n");
    break;
  default:
    break;
  }

  /* subtypes */
  switch(IDL_NODE_TYPE(tci->ts)) {
  case IDLN_EXCEPT_DCL:
  case IDLN_TYPE_STRUCT:
    if(!IDL_TYPE_STRUCT(tci->ts).member_list)
      break;
    fprintf(tci->of, "static const CORBA_TypeCode anon_subtypes_array%d[] = {",
	    subtypes_id);
    for(curitem = IDL_TYPE_STRUCT(tci->ts).member_list; curitem;
	curitem = IDL_LIST(curitem).next) {
      IDL_tree curdcl;

      curdcl = IDL_MEMBER(IDL_LIST(curitem).data).type_spec;
      switch(IDL_NODE_TYPE(curdcl)) {
      case IDLN_IDENT:
      case IDLN_INTERFACE:
      case IDLN_TYPE_OBJECT:
      case IDLN_FORWARD_DCL:
	curdcl = orbit_cbe_get_typespec(curdcl);
	if(IDL_NODE_TYPE(curdcl) == IDLN_TYPE_OBJECT
	   || IDL_NODE_TYPE(curdcl) == IDLN_INTERFACE
	   || IDL_NODE_TYPE(curdcl) == IDLN_FORWARD_DCL) {
	  ctmp = g_strdup("Object");
	  break;
	}
      default:
	ctmp = orbit_cbe_get_typename(IDL_MEMBER(IDL_LIST(curitem).data).type_spec);
      }

      g_assert(IDL_NODE_TYPE(IDL_LIST(curitem).data) == IDLN_MEMBER);

      for(curdcl = IDL_MEMBER(IDL_LIST(curitem).data).dcls; curdcl;
	  curdcl = IDL_LIST(curdcl).next) {

	fprintf(tci->of, "(CORBA_TypeCode)&TC_%s_struct", ctmp);

	if(IDL_LIST(curdcl).next || IDL_LIST(curitem).next)
	  fprintf(tci->of, ", ");
      }

      g_free(ctmp);
    }
    fprintf(tci->of, "};\n");
    break;
  case IDLN_TYPE_UNION:
    if(!IDL_TYPE_UNION(tci->ts).switch_body)
      break;
    fprintf(tci->of, "static const CORBA_TypeCode anon_subtypes_array%d[] = {",
	    subtypes_id);
    for(curitem = IDL_TYPE_UNION(tci->ts).switch_body; curitem;
	curitem = IDL_LIST(curitem).next) {
      IDL_tree curlabel, curdcl;

      g_assert(IDL_NODE_TYPE(IDL_LIST(curitem).data) == IDLN_CASE_STMT);

      curdcl = IDL_CASE_STMT(IDL_LIST(curitem).data).element_spec;
      curdcl = IDL_MEMBER(curdcl).type_spec;
      switch(IDL_NODE_TYPE(orbit_cbe_get_typespec(curdcl))) {
      case IDLN_TYPE_OBJECT:
      case IDLN_INTERFACE:
      case IDLN_FORWARD_DCL:
	ctmp = g_strdup("Object");
	break;
      default:
	ctmp = orbit_cbe_get_typename(curdcl);
      }

      for(curlabel = IDL_CASE_STMT(IDL_LIST(curitem).data).labels;
	  curlabel;
	  curlabel = IDL_LIST(curlabel).next) {
	fprintf(tci->of, "(CORBA_TypeCode)&TC_%s_struct", ctmp);

	if(IDL_LIST(curlabel).next || IDL_LIST(curitem).next)
	  fprintf(tci->of, ", ");
      }
      g_free(ctmp);
    }
    fprintf(tci->of, "};\n");
    break;
  case IDLN_TYPE_SEQUENCE:
    {
      IDL_tree seqts;

      seqts = orbit_cbe_get_typespec(IDL_TYPE_SEQUENCE(tci->ts).simple_type_spec);
      fprintf(tci->of, "static const CORBA_TypeCode anon_subtypes_array%d[] = ",
	      subtypes_id);
      switch(IDL_NODE_TYPE(seqts)) {
      case IDLN_TYPE_OBJECT:
      case IDLN_INTERFACE:
      case IDLN_FORWARD_DCL:
	ctmp = g_strdup("Object");
	break;
      default:
	ctmp = orbit_cbe_get_typename(IDL_TYPE_SEQUENCE(tci->ts).simple_type_spec);
      }
    }
    fprintf(tci->of, "{(CORBA_TypeCode)&TC_%s_struct};\n", ctmp);
    g_free(ctmp);
    break;
  case IDLN_TYPE_ARRAY:
  case IDLN_IDENT:
    fprintf(tci->of, "static const CORBA_TypeCode anon_subtypes_array%d[] = ",
	    subtypes_id);
    fprintf(tci->of, "{(CORBA_TypeCode)&%s_struct};\n", tci->substructname);
    break;
  default:
    break;
  }

  /* sublabels */
  switch(IDL_NODE_TYPE(tci->ts)) {
  case IDLN_TYPE_UNION:
    if(!IDL_TYPE_UNION(tci->ts).switch_body)
      break;
    n = 0;
    fprintf(tci->of, "static const CORBA_any anon_sublabels_array%d[] = {",
	    sublabels_id);
    ctmp = orbit_cbe_get_typename(IDL_TYPE_UNION(tci->ts).switch_type_spec);
    for(curitem = IDL_TYPE_UNION(tci->ts).switch_body; curitem;
	curitem = IDL_LIST(curitem).next) {
      IDL_tree curlabel;

      g_assert(IDL_NODE_TYPE(IDL_LIST(curitem).data) == IDLN_CASE_STMT);

      for(curlabel = IDL_CASE_STMT(IDL_LIST(curitem).data).labels;
	  curlabel;
	  curlabel = IDL_LIST(curlabel).next) {

	if(IDL_LIST(curlabel).data) {
	  fprintf(tci->of, "{(CORBA_TypeCode)&TC_%s_struct, (gpointer)&anon_sublabel_values_array%d[%d], CORBA_FALSE}",
		  ctmp, union_sublabels_array_ctr, n);
	  n++;
	} else {
	  /* default case */
	  fprintf(tci->of, "{(CORBA_TypeCode)&TC_CORBA_octet_struct, (int *)&ORBit_zero_int, CORBA_FALSE}");
	}

	if(IDL_LIST(curlabel).next || IDL_LIST(curitem).next)
	  fprintf(tci->of, ", ");

      }
    }
    g_free(ctmp);
    fprintf(tci->of, "};\n");
    break;
  default:
    break;
  }

  if(!strncmp(tci->structname, "anon", 4)) /* For anonymous typecodes, don't
					      export the struct */
    fprintf(tci->of, "static ");

  fprintf(tci->of, "const struct CORBA_TypeCode_struct %s_struct = {\n",
	  tci->structname);

  /* pseudoobject stuff */
  fprintf(tci->of, "{ { (ORBit_RootObject_Interface*)&ORBit_TypeCode_epv, TRUE, -1 }, ORBIT_PSEUDO_TYPECODE },\n");

  switch(IDL_NODE_TYPE(tci->ts)) {
  case IDLN_TYPE_ARRAY:
    if(tci->array_gen_ctr)
      fprintf(tci->of, "CORBA_tk_array");
    else
      fprintf(tci->of, "CORBA_tk_alias");
    break;
  case IDLN_IDENT:
    fprintf(tci->of, "CORBA_tk_alias");
    break;
  case IDLN_TYPE_STRUCT:
    fprintf(tci->of, "CORBA_tk_struct");
    break;
  case IDLN_TYPE_UNION:
    fprintf(tci->of, "CORBA_tk_union");
    break;
  case IDLN_TYPE_ENUM:
    fprintf(tci->of, "CORBA_tk_enum");
    break;
  case IDLN_TYPE_OBJECT:
  case IDLN_INTERFACE:
  case IDLN_FORWARD_DCL:
    fprintf(tci->of, "CORBA_tk_objref");
    break;
  case IDLN_EXCEPT_DCL:
    fprintf(tci->of, "CORBA_tk_except");
    break;
  case IDLN_TYPE_INTEGER:
    fprintf(tci->of, "CORBA_tk_");
    if(!IDL_TYPE_INTEGER(tci->ts).f_signed)
      fprintf(tci->of, "u");
    switch(IDL_TYPE_INTEGER(tci->ts).f_type) {
    case IDL_INTEGER_TYPE_SHORT:
      fprintf(tci->of, "short");
      break;
    case IDL_INTEGER_TYPE_LONG:
      fprintf(tci->of, "long");
      break;
    case IDL_INTEGER_TYPE_LONGLONG:
      fprintf(tci->of, "longlong");
      break;
    }
    break;
  case IDLN_TYPE_FLOAT:
    fprintf(tci->of, "CORBA_tk_");
    switch(IDL_TYPE_FLOAT(tci->ts).f_type) {
    case IDL_FLOAT_TYPE_FLOAT:
      fprintf(tci->of, "float");
      break;
    case IDL_FLOAT_TYPE_DOUBLE:
      fprintf(tci->of, "double");
      break;
    case IDL_FLOAT_TYPE_LONGDOUBLE:
      fprintf(tci->of, "longdouble");
      break;
    }
    break;
  case IDLN_TYPE_BOOLEAN:
    fprintf(tci->of, "CORBA_tk_boolean");
    break;
  case IDLN_TYPE_OCTET:
    fprintf(tci->of, "CORBA_tk_octet");
    break;
  case IDLN_TYPE_STRING:
    fprintf(tci->of, "CORBA_tk_string");
    break;
  case IDLN_TYPE_WIDE_STRING:
    fprintf(tci->of, "CORBA_tk_wstring");
    break;
  case IDLN_TYPE_CHAR:
    fprintf(tci->of, "CORBA_tk_char");
    break;
  case IDLN_TYPE_WIDE_CHAR:
    fprintf(tci->of, "CORBA_tk_wchar");
    break;
  case IDLN_TYPE_ANY:
    fprintf(tci->of, "CORBA_tk_any");
    break;
  case IDLN_TYPE_SEQUENCE:
    fprintf(tci->of, "CORBA_tk_sequence");
    break;
  default:
    g_message("Type %s has no tk constant!",
	      IDL_tree_type_names[IDL_NODE_TYPE(tci->ts)]);
  }

  fprintf(tci->of, ",");

  switch(IDL_NODE_TYPE(tci->ts)) {
  case IDLN_TYPE_STRUCT:
    fprintf(tci->of, "\"%s\"", IDL_IDENT(IDL_TYPE_STRUCT(tci->ts).ident).str);
    break;
  case IDLN_TYPE_UNION:
    fprintf(tci->of, "\"%s\"", IDL_IDENT(IDL_TYPE_UNION(tci->ts).ident).str);
    break;
  case IDLN_TYPE_ENUM:
    fprintf(tci->of, "\"%s\"", IDL_IDENT(IDL_TYPE_ENUM(tci->ts).ident).str);
    break;
  case IDLN_INTERFACE:
  case IDLN_FORWARD_DCL:
    fprintf(tci->of, "\"%s\"", IDL_IDENT(IDL_INTERFACE(tci->ts).ident).str);
    break;
  case IDLN_EXCEPT_DCL:
    fprintf(tci->of, "\"%s\"", IDL_IDENT(IDL_EXCEPT_DCL(tci->ts).ident).str);
    break;
  case IDLN_IDENT:
    fprintf(tci->of, "\"%s\"", IDL_IDENT(tci->ts).str);
    break;
  case IDLN_TYPE_ARRAY:
    if(!tci->array_gen_ctr) {
      fprintf(tci->of, "\"%s\"", IDL_IDENT(IDL_TYPE_ARRAY(tci->ts).ident).str);
      break;
    }
  default:
    fprintf(tci->of, "NULL");
    break;
  }

  fprintf(tci->of, ",");

  switch(IDL_NODE_TYPE(tci->ts)) {
  case IDLN_TYPE_STRUCT:
    fprintf(tci->of, "\"%s\"",
	    IDL_IDENT(IDL_TYPE_STRUCT(tci->ts).ident).repo_id);
    break;
  case IDLN_TYPE_UNION:
    fprintf(tci->of, "\"%s\"", IDL_IDENT(IDL_TYPE_UNION(tci->ts).ident).repo_id);
    break;
  case IDLN_TYPE_ENUM:
    fprintf(tci->of, "\"%s\"", IDL_IDENT(IDL_TYPE_ENUM(tci->ts).ident).repo_id);
    break;
  case IDLN_INTERFACE:
  case IDLN_FORWARD_DCL:
    fprintf(tci->of, "\"%s\"", IDL_IDENT(IDL_INTERFACE(tci->ts).ident).repo_id);
    break;
  case IDLN_EXCEPT_DCL:
    fprintf(tci->of, "\"%s\"", IDL_IDENT(IDL_EXCEPT_DCL(tci->ts).ident).repo_id);
    break;
  case IDLN_IDENT:
    fprintf(tci->of, "\"%s\"", IDL_IDENT(tci->ts).repo_id);
    break;
  case IDLN_TYPE_ARRAY:
    if(!tci->array_gen_ctr) {
      fprintf(tci->of, "\"%s\"", IDL_IDENT(IDL_TYPE_ARRAY(tci->ts).ident).repo_id);
      break;
    }
  default:
    fprintf(tci->of, "NULL");
    break;
  }

  fprintf(tci->of, ",\n");

  switch(IDL_NODE_TYPE(tci->ts)) {
  case IDLN_TYPE_SEQUENCE:
    if(IDL_TYPE_SEQUENCE(tci->ts).positive_int_const)
      n = IDL_INTEGER(IDL_TYPE_SEQUENCE(tci->ts).positive_int_const).value;
    else
      n = 0;
    break;
  case IDLN_TYPE_STRING:
    if(IDL_TYPE_STRING(tci->ts).positive_int_const)
      n = IDL_INTEGER(IDL_TYPE_STRING(tci->ts).positive_int_const).value;
    else
      n = 0;
    break;
  case IDLN_TYPE_WIDE_STRING:
    if(IDL_TYPE_WIDE_STRING(tci->ts).positive_int_const)
      n = IDL_INTEGER(IDL_TYPE_STRING(tci->ts).positive_int_const).value;
    else
      n = 0;
    break;
  case IDLN_TYPE_ARRAY:
    if(tci->array_gen_ctr) {
      IDL_tree sizer;

      sizer = IDL_list_nth(IDL_TYPE_ARRAY(tci->ts).size_list,
			   tci->array_gen_ctr-1);
      g_assert(IDL_NODE_TYPE(IDL_LIST(sizer).data) == IDLN_INTEGER);
      n = IDL_INTEGER(IDL_LIST(sizer).data).value;
    } else
      n = 0;
    break;
  default:
    n = 0;
    break;
  }
  fprintf(tci->of, "%d,", n);

  switch(IDL_NODE_TYPE(tci->ts)) {
  case IDLN_TYPE_STRUCT:
  case IDLN_EXCEPT_DCL:
    n = 0;
    for(curitem = IDL_TYPE_STRUCT(tci->ts).member_list;
	curitem;
	curitem = IDL_LIST(curitem).next) {
      IDL_tree curdcl;

      curdcl = IDL_LIST(curitem).data;
      g_assert(IDL_NODE_TYPE(curdcl) == IDLN_MEMBER);

      n += IDL_list_length(IDL_MEMBER(curdcl).dcls);
    }
    break;
  case IDLN_TYPE_UNION:
    n = 0;
    for(curitem = IDL_TYPE_UNION(tci->ts).switch_body;
	curitem;
	curitem = IDL_LIST(curitem).next) {
      IDL_tree curdcl;

      curdcl = IDL_LIST(curitem).data;
      g_assert(IDL_NODE_TYPE(curdcl) == IDLN_CASE_STMT);

      n += IDL_list_length(IDL_CASE_STMT(curdcl).labels);
    }
    break;
  case IDLN_TYPE_ENUM:
    n = IDL_list_length(IDL_TYPE_ENUM(tci->ts).enumerator_list);
    break;
  case IDLN_IDENT: /* CORBA_tk_alias */
  case IDLN_TYPE_SEQUENCE:
  case IDLN_TYPE_ARRAY:
    n = 1;
    break;
  default:
    n = 0;
    break;
  }

  fprintf(tci->of, "%d,\n", n);

  switch(IDL_NODE_TYPE(tci->ts)) {
  case IDLN_TYPE_ENUM:
    if(IDL_TYPE_ENUM(tci->ts).enumerator_list)
      fprintf(tci->of, "(const char **)anon_subnames_array%d", subnames_id);
    else
      fprintf(tci->of, "NULL");
    break;
  case IDLN_TYPE_UNION:
    if(IDL_TYPE_UNION(tci->ts).switch_body)
      fprintf(tci->of, "(const char **)anon_subnames_array%d", subnames_id);
    else
      fprintf(tci->of, "NULL");
    break;
  case IDLN_TYPE_STRUCT:
  case IDLN_EXCEPT_DCL:
    if(IDL_TYPE_STRUCT(tci->ts).member_list)
      fprintf(tci->of, "(const char **)anon_subnames_array%d", subnames_id);
    else
      fprintf(tci->of, "NULL");
    break;
  default:
    fprintf(tci->of, "NULL");
    break;
  }

  fprintf(tci->of, ",\n");

  /* subtypes */
  switch(IDL_NODE_TYPE(tci->ts)) {
  case IDLN_EXCEPT_DCL:
  case IDLN_TYPE_STRUCT:
    if(IDL_TYPE_STRUCT(tci->ts).member_list)
      fprintf(tci->of, "(CORBA_TypeCode *)anon_subtypes_array%d",
	      subtypes_id);
    else
      fprintf(tci->of, "NULL");
    break;
  case IDLN_TYPE_UNION:
    if(IDL_TYPE_UNION(tci->ts).switch_body)
      fprintf(tci->of, "(CORBA_TypeCode *)anon_subtypes_array%d",
	      subtypes_id);
    else
      fprintf(tci->of, "NULL");
    break;
  case IDLN_TYPE_SEQUENCE:
  case IDLN_TYPE_ARRAY:
  case IDLN_IDENT:
    fprintf(tci->of, "(CORBA_TypeCode *)anon_subtypes_array%d",
	    subtypes_id);
    break;
  default:
    fprintf(tci->of, "NULL");
    break;
  }

  fprintf(tci->of, ",\n");

  /* sublabels */
  switch(IDL_NODE_TYPE(tci->ts)) {
  case IDLN_TYPE_UNION:
    fprintf(tci->of, "(CORBA_any *)anon_sublabels_array%d", sublabels_id);
    break;
  default:
    fprintf(tci->of, "NULL");
    break;
  }

  fprintf(tci->of, ",\n");

  switch(IDL_NODE_TYPE(tci->ts)) {
  case IDLN_TYPE_UNION:
    ctmp = orbit_cbe_get_typename(IDL_TYPE_UNION(tci->ts).switch_type_spec);
    fprintf(tci->of, "(CORBA_TypeCode)&TC_%s_struct", ctmp);
    g_free(ctmp);
    break;
  default:
    fprintf(tci->of, "CORBA_OBJECT_NIL");
    break;
  }

  fprintf(tci->of, ", ");

  /* recursive sequence, no clue how they work */
  fprintf(tci->of, "0");

  fprintf(tci->of, ", ");

  /* default index */
  fprintf(tci->of, "%d", union_default_index);

  fprintf(tci->of, ", ");

  if(IDL_NODE_TYPE(tci->ts) == IDLN_TYPE_FIXED) {
    fprintf(tci->of, "%d, %d",
	    (int)IDL_INTEGER(IDL_TYPE_FIXED(tci->ts).positive_int_const).value,
	    (int)IDL_INTEGER(IDL_TYPE_FIXED(tci->ts).integer_lit).value);
  } else
    fprintf(tci->of, "0, 0");

  fprintf(tci->of, "\n};\n");
  if(strncmp(tci->structname, "anon", 4))
    fprintf(tci->of, "#endif\n");
}
