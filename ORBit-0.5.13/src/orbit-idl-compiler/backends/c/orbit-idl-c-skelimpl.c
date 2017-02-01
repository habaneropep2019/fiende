#include "config.h"

#include "orbit-idl-c-backend.h"

/* This file copied from the old IDL compiler orbit-c-skelimpl.c, with minimal changes. */

static void orbit_cbe_write_skelimpl(FILE *outfile, IDL_tree tree, const char *hdrname);

void
orbit_idl_output_c_skelimpl(OIDL_Output_Tree *tree, OIDL_Run_Info *rinfo, OIDL_C_Info *ci)
{
  orbit_cbe_write_skelimpl(ci->fh, tree->tree, ci->base_name);
}

#include <ctype.h>
#include <glib.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

/* Abbreviations used here:
   "cbe" stands for "C backend"
   "hdr" -> "header" (duh :)
   "of" -> "output file"
   "ns" -> "name space"
*/

typedef struct {
  FILE *of;
  IDL_tree tree;
  enum { PASS_SERVANTS, PASS_PROTOS, PASS_EPVS, PASS_VEPVS,
	 PASS_IMPLSTUBS, PASS_LAST } pass;
} CBESkelImplInfo;

const char *passnames[] = {
  "App-specific servant structures",
  "Implementation stub prototypes",
  "epv structures",
  "vepv structures",
  "Stub implementations",
  "Boohoo!"
};

static void orbit_cbe_ski_process_piece(CBESkelImplInfo *ski);
static void cbe_ski_do_list(CBESkelImplInfo *ski);
static void cbe_ski_do_inherited_attr_dcl(CBESkelImplInfo *ski, IDL_tree current_interface);
static void cbe_ski_do_attr_dcl(CBESkelImplInfo *ski);
static void cbe_ski_do_inherited_op_dcl(CBESkelImplInfo *ski, IDL_tree current_interface);
static void cbe_ski_do_op_dcl(CBESkelImplInfo *ski);
static void cbe_ski_do_param_dcl(CBESkelImplInfo *ski);
static void cbe_ski_do_interface(CBESkelImplInfo *ski);
static void cbe_ski_do_module(CBESkelImplInfo *ski);

static void
orbit_cbe_write_skelimpl(FILE *outfile, IDL_tree tree, const char *hdrname)
{
  CBESkelImplInfo ski = {NULL, NULL, PASS_SERVANTS};

  ski.of = outfile;
  ski.tree = tree;

  g_return_if_fail(IDL_NODE_TYPE(tree) == IDLN_LIST);

  fprintf(outfile, "#include \"%s.h\"\n", hdrname);

  for(ski.pass = PASS_SERVANTS; ski.pass < PASS_LAST; ski.pass++) {
    fprintf(outfile, "\n/*** %s ***/\n", passnames[ski.pass]);
    orbit_cbe_ski_process_piece(&ski);
  }
}

static void
orbit_cbe_ski_process_piece(CBESkelImplInfo *ski)
{
  /* I'm not implementing this as an array of function pointers
     because we may want to do special logic for particular cases in
     the future. Hope this is clear enough. -ECL */

  switch(IDL_NODE_TYPE(ski->tree)) {
  case IDLN_ATTR_DCL:
    cbe_ski_do_attr_dcl(ski);
    break;
  case IDLN_INTERFACE:
    cbe_ski_do_interface(ski);
    break;
  case IDLN_LIST:
    cbe_ski_do_list(ski);
    break;
  case IDLN_MODULE:
    cbe_ski_do_module(ski);
    break;
  case IDLN_OP_DCL:
    cbe_ski_do_op_dcl(ski);
    break;
  case IDLN_PARAM_DCL:
    cbe_ski_do_param_dcl(ski);
    break;
  default:
#if 0
    g_warning("Node type %s unhandled in \"skeleton implementation producer\" routines.",
	      IDL_tree_type_names[IDL_NODE_TYPE(ski->tree)]);
#endif
    break;
  }
}

static void
cbe_ski_do_module(CBESkelImplInfo *ski)
{
  CBESkelImplInfo subski = *ski;
  subski.tree = IDL_MODULE(ski->tree).definition_list;
  cbe_ski_do_list(&subski);
}

static void
cbe_ski_do_list(CBESkelImplInfo *ski)
{
  CBESkelImplInfo subski = *ski;
  IDL_tree curitem;

  for(curitem = ski->tree; curitem; curitem = IDL_LIST(curitem).next) {
    subski.tree = IDL_LIST(curitem).data;
    orbit_cbe_ski_process_piece(&subski);
    fprintf(ski->of, "\n");
  }
}

static void cbe_ski_do_attr_dcl_internal(CBESkelImplInfo *ski, IDL_tree current_interface, gboolean inherited)
{
  IDL_tree curop, curitem;
  GString *attrname = g_string_new(NULL);
  CBESkelImplInfo subski = *ski;

  if(ski->pass == PASS_SERVANTS) {
    for(curitem = IDL_ATTR_DCL(ski->tree).simple_declarations; curitem;
	curitem = IDL_LIST(curitem).next) {
      orbit_cbe_write_typespec(ski->of, IDL_ATTR_DCL(ski->tree).param_type_spec);
      fprintf(ski->of, " attr_%s;\n", IDL_IDENT(IDL_LIST(curitem).data).str);
    }
  }

  for(curitem = IDL_ATTR_DCL(ski->tree).simple_declarations;
      curitem; curitem = IDL_LIST(curitem).next) {

    /* Fake the attribute get/set methods as operation declarations */
    IDL_tree ident, ns_data_save;
    int i;

    for (i = 0; i < 2; ++i) {

	    if (i && IDL_ATTR_DCL(ski->tree).f_readonly)
		    break;
	    /* Output the operation on this attribute */
	    g_string_sprintf(attrname, i ? "_set_%s" : "_get_%s",
			     IDL_IDENT(IDL_LIST(curitem).data).str);
	    ident = IDL_ident_new(g_strdup(attrname->str));
	    
	    /* Tell the ident where our namespace node is, and request a return value
	       if this is the _get operation */
	    IDL_IDENT_TO_NS(ident) = IDL_IDENT_TO_NS(IDL_LIST(curitem).data);
	    curop = IDL_op_dcl_new(0, i == 0 ?
				   IDL_ATTR_DCL(ski->tree).param_type_spec : NULL,
				   ident, NULL, NULL, NULL);
	    
	    curop->up = ski->tree->up;
	    subski.tree = curop;
	    
	    /* Save the namespace ident (IDL_GENTREE data) reference, assign
	       back to the temporary tree, output the operation, then restore
	       the namespace ident link */
	    ns_data_save = IDL_GENTREE(IDL_IDENT_TO_NS(IDL_LIST(curitem).data)).data;
	    IDL_GENTREE(IDL_IDENT_TO_NS(IDL_LIST(curitem).data)).data = ident;

	    if (i) {
		    /* The set routine also needs the value, so we
		       temporarily add that to the operation
		       declaration */
		    IDL_OP_DCL(curop).parameter_dcls = IDL_list_new(
			    IDL_param_dcl_new(IDL_PARAM_IN,
					      IDL_ATTR_DCL(ski->tree).param_type_spec,
					      IDL_ident_new(g_strdup("value"))));
	    }
	    
	    if(inherited==TRUE)
	      cbe_ski_do_inherited_op_dcl(&subski, current_interface);
	    else
	      orbit_cbe_ski_process_piece(&subski);

	    /* Restore the fake link to the original in the namespace */
	    IDL_GENTREE(IDL_IDENT_TO_NS(IDL_LIST(curitem).data)).data = ns_data_save;

	    if (i) {
		    /* Free only what we've created for the fake node, so remove 
		       the attribute node element and then free the rest */
		    IDL_PARAM_DCL(IDL_LIST(
			    IDL_OP_DCL(curop).parameter_dcls).data).param_type_spec = NULL;
	    }
	    
	    /* Remove what we've "borrowed" from ATTR_DCL from the
	       fake curop node then free the rest */
	    IDL_OP_DCL(curop).op_type_spec = NULL;
	    IDL_tree_free(curop);
    }
  }

  g_string_free(attrname, TRUE);
}

static void cbe_ski_do_attr_dcl(CBESkelImplInfo *ski)
{
  cbe_ski_do_attr_dcl_internal(ski, NULL, FALSE);
}

void
cbe_ski_do_inherited_attr_dcl(CBESkelImplInfo *ski, IDL_tree current_interface)
{
  cbe_ski_do_attr_dcl_internal(ski, current_interface, TRUE);
}

static void
cbe_ski_do_op_dcl(CBESkelImplInfo *ski)
{
  /* If you fix anything here, please also fix it in
     cbe_ski_interface_print_epv_for_op(), which is almost a
     cut-and-paste of this routine */

  char *id, *id2;
  IDL_tree curitem, op;
  int level;
  CBESkelImplInfo subski = *ski;

  switch(ski->pass) {
  case PASS_PROTOS:
  case PASS_IMPLSTUBS:
    fprintf(ski->of, "static ");
    orbit_cbe_write_typespec(ski->of, IDL_OP_DCL(ski->tree).op_type_spec);
    orbit_cbe_param_printptrs(ski->of, IDL_OP_DCL(ski->tree).op_type_spec, DATA_RETURN);
    
    id = IDL_ns_ident_to_qstring(IDL_IDENT_TO_NS(IDL_OP_DCL(ski->tree).ident), "_", 0);
    
    curitem = IDL_get_parent_node(ski->tree, IDLN_INTERFACE, &level);

    g_assert(curitem);
    id2 = IDL_ns_ident_to_qstring(IDL_IDENT_TO_NS(IDL_INTERFACE(curitem).ident), "_", 0);
    fprintf(ski->of, "\nimpl_%s(impl_POA_%s *servant,\n", id, id2);
    g_free(id); g_free(id2);
    
    op = ski->tree;
    for(curitem = IDL_OP_DCL(ski->tree).parameter_dcls;
	curitem; curitem = IDL_LIST(curitem).next) {
      subski.tree = IDL_LIST(curitem).data;
      orbit_cbe_ski_process_piece(&subski);
    }

    if(IDL_OP_DCL(op).context_expr)
      fprintf(ski->of, "CORBA_Context ctx,\n");

    fprintf(ski->of, "CORBA_Environment *ev)");
    if(ski->pass == PASS_IMPLSTUBS) {
      fprintf(ski->of, "\n{\n");
      if(IDL_OP_DCL(op).op_type_spec) {
	orbit_cbe_write_typespec(ski->of, IDL_OP_DCL(ski->tree).op_type_spec);
	orbit_cbe_param_printptrs(ski->of, IDL_OP_DCL(ski->tree).op_type_spec, DATA_RETURN);
	fprintf(ski->of, " retval;\n\nreturn retval;\n");
      }
      fprintf(ski->of, "}\n\n");
    } else /* PASS_PROTOS */
      fprintf(ski->of, ";\n");
    break; /* End PASS_PROTOS | PASS_IMPLSTUBS */

  case PASS_EPVS:
    id = IDL_ns_ident_to_qstring(IDL_IDENT_TO_NS(IDL_OP_DCL(ski->tree).ident),
				 "_", 0);
    fprintf(ski->of, "(gpointer)&impl_%s,\n", id);
    g_free(id);
    break;
  default:
    break;
  }
}

static void
cbe_ski_do_inherited_op_dcl(CBESkelImplInfo *ski, IDL_tree current_interface)
{
  char *id, *id2;
  IDL_tree ident, curitem, intf, op;
  int level;
  CBESkelImplInfo subski = *ski;

  id = IDL_ns_ident_to_qstring(IDL_IDENT_TO_NS(IDL_INTERFACE(current_interface).ident), "_", 0);
  intf = IDL_get_parent_node(ski->tree, IDLN_INTERFACE, NULL);
  id2 = IDL_ns_ident_to_qstring(IDL_IDENT_TO_NS(IDL_INTERFACE(intf).ident), "_", 0);

  ident=IDL_OP_DCL(ski->tree).ident;
  g_assert(ident);

  switch(ski->pass) {
  case PASS_PROTOS:
  case PASS_IMPLSTUBS:
    fprintf(ski->of, "static ");
    orbit_cbe_write_typespec(ski->of, IDL_OP_DCL(ski->tree).op_type_spec);
    orbit_cbe_param_printptrs(ski->of, IDL_OP_DCL(ski->tree).op_type_spec, DATA_RETURN);
    
    curitem = IDL_get_parent_node(ski->tree, IDLN_INTERFACE, &level);
    g_assert(curitem);

    fprintf(ski->of, "\nimpl_%s_%s(impl_POA_%s *servant,\n", id, IDL_IDENT(ident).str, id);
    
    op = ski->tree;
    for(curitem = IDL_OP_DCL(ski->tree).parameter_dcls;
	curitem; curitem = IDL_LIST(curitem).next) {
      subski.tree = IDL_LIST(curitem).data;
      orbit_cbe_ski_process_piece(&subski);
    }

    if(IDL_OP_DCL(op).context_expr)
      fprintf(ski->of, "CORBA_Context ctx,\n");

    fprintf(ski->of, "CORBA_Environment *ev)");
    if(ski->pass == PASS_IMPLSTUBS) {
      fprintf(ski->of, "\n{\n");
      if(IDL_OP_DCL(op).op_type_spec) {
	orbit_cbe_write_typespec(ski->of, IDL_OP_DCL(ski->tree).op_type_spec);
	orbit_cbe_param_printptrs(ski->of, IDL_OP_DCL(ski->tree).op_type_spec, DATA_RETURN);
	fprintf(ski->of, " retval;\n\nreturn retval;\n");
      }
      fprintf(ski->of, "}\n\n");
    } else /* PASS_PROTOS */
      fprintf(ski->of, ";\n");
    break; /* End PASS_PROTOS | PASS_IMPLSTUBS */

  case PASS_EPVS:
    ident=IDL_OP_DCL(ski->tree).ident;
    g_assert(ident);

    fprintf(ski->of, "(gpointer)&impl_%s_%s,\n", id, IDL_IDENT(ident).str);
  default:
    break;
  }

  g_free(id);
  g_free(id2);
}

static void
cbe_ski_do_param_dcl(CBESkelImplInfo *ski)
{
  IDL_ParamRole r = DATA_IN;

  orbit_cbe_write_typespec(ski->of,
			   IDL_PARAM_DCL(ski->tree).param_type_spec);

  switch(IDL_PARAM_DCL(ski->tree).attr) {
  case IDL_PARAM_IN: r = DATA_IN; break;
  case IDL_PARAM_INOUT: r = DATA_INOUT; break;
  case IDL_PARAM_OUT: r = DATA_OUT; break;
  default:
    g_error("Unknown IDL_PARAM type");
  }

  orbit_cbe_param_printptrs(ski->of,
			    IDL_PARAM_DCL(ski->tree).param_type_spec, r);
  fprintf(ski->of, " %s,\n", IDL_IDENT(IDL_PARAM_DCL(ski->tree).simple_declarator).str);
}

static void
cbe_ski_do_interface_vepv_entry(IDL_tree interface, CBESkelImplInfo *ski)
{
  char *id, *inherit_id;

  if(interface==ski->tree) {
    id = IDL_ns_ident_to_qstring(IDL_IDENT_TO_NS(IDL_INTERFACE(ski->tree).ident), "_", 0);
    fprintf(ski->of, "&impl_%s_epv,\n", id);
    g_free(id);
    return;
  }

  id = IDL_ns_ident_to_qstring(IDL_IDENT_TO_NS(IDL_INTERFACE(ski->tree).ident), "_", 0);
  inherit_id = IDL_ns_ident_to_qstring(IDL_IDENT_TO_NS(IDL_INTERFACE(interface).ident),
			       "_", 0);
  fprintf(ski->of, "&impl_%s_%s_epv,\n", id, inherit_id);

  g_free(id);
  g_free(inherit_id);
}

static void
cbe_ski_do_inherited_methods(IDL_tree interface, CBESkelImplInfo *ski)
{
  CBESkelImplInfo subski= *ski;
  IDL_tree curitem;
  char *id, *inherit_id;

  if(interface==ski->tree)
    return;

  if(ski->pass==PASS_EPVS) {
    id = IDL_ns_ident_to_qstring(IDL_IDENT_TO_NS(IDL_INTERFACE(ski->tree).ident),
      "_", 0);
    inherit_id = IDL_ns_ident_to_qstring(IDL_IDENT_TO_NS(IDL_INTERFACE(interface).ident),
      "_", 0);
    fprintf(ski->of, "static POA_%s__epv impl_%s_%s_epv = {\nNULL, /* _private */\n",
      inherit_id, id, inherit_id);
  }

  for(curitem = IDL_INTERFACE(interface).body; curitem; curitem=IDL_LIST(curitem).next) {
  	subski.tree=IDL_LIST(curitem).data;

	switch(IDL_NODE_TYPE(IDL_LIST(curitem).data)) {
	case IDLN_OP_DCL:
		cbe_ski_do_inherited_op_dcl(&subski, ski->tree);
		break;
	case IDLN_ATTR_DCL:
		cbe_ski_do_inherited_attr_dcl(&subski, ski->tree);
		break;
	default:
	  break;
	}
  }

  if(ski->pass==PASS_EPVS) {
    fprintf(ski->of, "};");

    g_free(id);
    g_free(inherit_id);
  }
}

static void
cbe_ski_do_interface(CBESkelImplInfo *ski)
{
  char *id;
  CBESkelImplInfo subski = *ski;

  id = IDL_ns_ident_to_qstring(IDL_IDENT_TO_NS(IDL_INTERFACE(ski->tree).ident), "_", 0);

  switch(ski->pass) {
  case PASS_SERVANTS:
    fprintf(ski->of, "typedef struct {\nPOA_%s servant;\nPortableServer_POA poa;\n", id);
    subski.tree = IDL_INTERFACE(ski->tree).body;
    cbe_ski_do_list(&subski);
    IDL_tree_traverse_parents(ski->tree, (GFunc)&cbe_ski_do_inherited_methods,
    				ski);
    fprintf(ski->of, "} impl_POA_%s;\n\n", id);
    break;
  case PASS_EPVS:
    fprintf(ski->of,
	    "static PortableServer_ServantBase__epv impl_%s_base_epv = {\n",
	    id);
    fprintf(ski->of, "NULL, /* _private data */\n");
    fprintf(ski->of, "NULL, /* finalize routine */\n");
    fprintf(ski->of, "NULL, /* default_POA routine */\n");
    fprintf(ski->of, "};\n");
    fprintf(ski->of, "static POA_%s__epv impl_%s_epv = {\nNULL, /* _private */\n", id, id);
    subski.tree = IDL_INTERFACE(ski->tree).body;
    cbe_ski_do_list(&subski);
    fprintf(ski->of, "};");
    IDL_tree_traverse_parents(ski->tree, (GFunc)&cbe_ski_do_inherited_methods,
    				ski);
    break;
  case PASS_VEPVS:
    fprintf(ski->of, "static POA_%s__vepv impl_%s_vepv = {\n", id, id);
    fprintf(ski->of, "&impl_%s_base_epv,\n", id);
    IDL_tree_traverse_parents(ski->tree, (GFunc)&cbe_ski_do_interface_vepv_entry,
			      ski);
    fprintf(ski->of, "};");
    break;
  case PASS_IMPLSTUBS:
    fprintf(ski->of, "static %s impl_%s__create(PortableServer_POA poa, CORBA_Environment *ev)\n", id, id);
    fprintf(ski->of, "{\n%s retval;\nimpl_POA_%s *newservant;\nPortableServer_ObjectId *objid;\n\n", id, id);
    fprintf(ski->of, "newservant = g_new0(impl_POA_%s, 1);\n", id);
    fprintf(ski->of, "newservant->servant.vepv = &impl_%s_vepv;\n", id);
    fprintf(ski->of, "newservant->poa = poa;\n");
    fprintf(ski->of, "POA_%s__init((PortableServer_Servant)newservant, ev);\n", id);
    fprintf(ski->of, "objid = PortableServer_POA_activate_object(poa, newservant, ev);\n");
    fprintf(ski->of, "CORBA_free(objid);\n");
    fprintf(ski->of, "retval = PortableServer_POA_servant_to_reference(poa, newservant, ev);\n");
    fprintf(ski->of, "\nreturn retval;\n}\n\n");
    fprintf(ski->of, "static void\nimpl_%s__destroy(impl_POA_%s *servant, CORBA_Environment *ev)\n{\n", id, id);
    fprintf(ski->of, "PortableServer_ObjectId *objid;\n\n");
    fprintf(ski->of, "objid = PortableServer_POA_servant_to_id(servant->poa, servant, ev);\n");
    fprintf(ski->of, "PortableServer_POA_deactivate_object(servant->poa, objid, ev);\n");
    fprintf(ski->of, "CORBA_free(objid);\n");
    fprintf(ski->of, "\nPOA_%s__fini((PortableServer_Servant)servant, ev);\n", id);
    fprintf(ski->of, "g_free(servant);\n}\n\n");
    subski.tree = IDL_INTERFACE(ski->tree).body;
    cbe_ski_do_list(&subski);
    IDL_tree_traverse_parents(ski->tree, (GFunc)&cbe_ski_do_inherited_methods,
    				ski);
    break;
  case PASS_PROTOS:
    fprintf(ski->of, "static void impl_%s__destroy(impl_POA_%s *servant,\nCORBA_Environment *ev);\n", id, id);
    subski.tree = IDL_INTERFACE(ski->tree).body;
    cbe_ski_do_list(&subski);
    IDL_tree_traverse_parents(ski->tree, (GFunc)&cbe_ski_do_inherited_methods,
    				ski);
    break;
  default:
    break;
  }

  g_free(id);
}
