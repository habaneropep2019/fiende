/**************************************************************************

    orbit-idl-driver.c (Dispatch parsed tree to various backends)

    Copyright (C) 1999 Elliot Lee

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    $Id: orbit-idl-driver.c,v 1.12.4.1 2001/01/15 22:22:41 philipd Exp $

***************************************************************************/

#include "config.h"
#include "orbit-idl2.h"
#include <string.h>

static void orbit_idl_tree_populate(IDL_tree tree, IDL_ns ns);

/****************
  orbit_idl_to_backend:
     Input: IDL filename, and any arguments to be passed to CPP
     Output: Number of IDL files (1 or 0) that were successfully processed
     Does: Reads in 'filename' & parses it into a tree, using libIDL.
	   Calls the backend producer.
****************/
int
orbit_idl_to_backend(const char *filename, OIDL_Run_Info *rinfo)
{
  OIDL_Backend_Info *binfo;
  IDL_ns namespace;
  IDL_tree tree;
  int errcode;
  char *basename, *ctmp;
  OIDL_Output_Tree otree;

  binfo = orbit_idl_backend_for_lang(rinfo->output_language,rinfo->backend_directory);

  g_return_val_if_fail(binfo && binfo->op_output, 0);

  errcode = IDL_parse_filename(filename, rinfo->cpp_args, NULL,
			       &tree, &namespace,
			       IDLF_TYPECODES|IDLF_CODEFRAGS,
			       IDL_WARNINGMAX);
  if(rinfo->debug_level > 1)
    orbit_idl_print_node(tree, 0);

  if(IDL_SUCCESS != errcode) {
    if(errcode == -1)
      g_warning("Parse of %s failed: %s", filename, g_strerror(errno));

    return 0;
  }

  basename = alloca(strlen(filename) + 1);
  strcpy(basename, filename);
  ctmp = strrchr(basename, '.');
  if(ctmp) *ctmp = '\0';

  otree.tree = tree;

  orbit_idl_tree_populate(otree.tree, namespace);
  orbit_idl_do_passes(otree.tree, rinfo);

  binfo->op_output(&otree, rinfo);

  return 1;
}

static void orbit_idl_op_populate(IDL_tree tree)
{
  OIDL_Op_Info *setme;

  setme = g_new0(OIDL_Op_Info, 1);

  setme->in_stubs = orbit_idl_marshal_populate_in(tree, FALSE);
  setme->out_stubs = orbit_idl_marshal_populate_out(tree, FALSE);
  setme->in_skels = orbit_idl_marshal_populate_in(tree, TRUE);
  setme->out_skels = orbit_idl_marshal_populate_out(tree, TRUE);
  setme->counter = 0;

  tree->data = setme;
}

static void orbit_idl_except_populate(IDL_tree tree)
{
  OIDL_Except_Info *setme;

  setme = g_new0(OIDL_Except_Info, 1);

  setme->marshal = orbit_idl_marshal_populate_except_marshal(tree);
  setme->demarshal = orbit_idl_marshal_populate_except_demarshal(tree);

  tree->data = setme;
}

static void
orbit_idl_tree_populate(IDL_tree tree, IDL_ns ns)
{
  IDL_tree node;

  if(!tree) return;

  switch(IDL_NODE_TYPE(tree)) {
  case IDLN_LIST:
    for(node = tree; node; node = IDL_LIST(node).next) {
      orbit_idl_tree_populate(IDL_LIST(node).data, ns);
    }
    break;
  case IDLN_MODULE:
    orbit_idl_tree_populate(IDL_MODULE(tree).definition_list, ns);
    break;
  case IDLN_INTERFACE:
    orbit_idl_tree_populate(IDL_INTERFACE(tree).body, ns);
    break;
  case IDLN_OP_DCL:
    orbit_idl_op_populate(tree);
    break;
  case IDLN_EXCEPT_DCL:
    orbit_idl_except_populate(tree);
    break;
  case IDLN_ATTR_DCL:
    {
      IDL_tree curnode, attr_name;

      orbit_idl_attr_fake_ops(tree, ns);

      for(curnode = IDL_ATTR_DCL(tree).simple_declarations; curnode; curnode = IDL_LIST(curnode).next) {
	attr_name = IDL_LIST(curnode).data;

	orbit_idl_tree_populate(((OIDL_Attr_Info *)attr_name->data)->op1, ns);
	if(((OIDL_Attr_Info *)attr_name->data)->op2)
	  orbit_idl_tree_populate(((OIDL_Attr_Info *)attr_name->data)->op2, ns);
      }
    }
    break;
  default:
    break;
  }
}
