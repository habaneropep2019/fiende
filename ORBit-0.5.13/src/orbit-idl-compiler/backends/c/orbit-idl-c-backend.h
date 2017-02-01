#ifndef ORBIT_IDL_C_BACKEND_H
#define ORBIT_IDL_C_BACKEND_H

#include "orbit-idl2.h"

#include <unistd.h>

#define BACKWARDS_COMPAT_0_4

typedef struct {
  char *base_name, *c_base_name;
  FILE *fh;
} OIDL_C_Info;

typedef struct {
  OIDL_C_Info *ci;
  gchar *orb_name;
  int last_tail_align;
  int alloc_on_stack : 1; /* TRUE for demarshalling in skeletons, etc. */
  int endian_swap_pass : 1; /* Demarshalling, again */
  int in_skels : 1;
} OIDL_C_Marshal_Info;

void orbit_idl_output_c(OIDL_Output_Tree *tree, OIDL_Run_Info *rinfo);

/* Used internally */
void orbit_idl_output_c_headers(OIDL_Output_Tree *tree, OIDL_Run_Info *rinfo, OIDL_C_Info *ci);
void orbit_idl_output_c_stubs(OIDL_Output_Tree *tree, OIDL_Run_Info *rinfo, OIDL_C_Info *ci);
void orbit_idl_output_c_skeletons(OIDL_Output_Tree *tree, OIDL_Run_Info *rinfo, OIDL_C_Info *ci);
void orbit_idl_output_c_common(OIDL_Output_Tree *tree, OIDL_Run_Info *rinfo, OIDL_C_Info *ci);
void orbit_idl_output_c_skelimpl(OIDL_Output_Tree *tree, OIDL_Run_Info *rinfo, OIDL_C_Info *ci);

void orbit_output_typecode(OIDL_C_Info *ci, IDL_tree ts);

void c_marshalling_generate(OIDL_Marshal_Node *node, OIDL_C_Info *ci, gboolean on_stack);
void c_demarshalling_generate(OIDL_Marshal_Node *node, OIDL_C_Info *ci, gboolean in_skels);

/* utils */
void orbit_cbe_write_typespec(FILE *of, IDL_tree tree);
char * orbit_cbe_get_typename(IDL_tree tree);
void orbit_cbe_op_write_proto(FILE *of, IDL_tree op, const char *nom_prefix, gboolean for_epv);
IDL_tree orbit_cbe_get_typespec(IDL_tree node);
void orbit_cbe_write_const(FILE *of, IDL_tree tree);
void orbit_cbe_write_const_node(FILE *of, OIDL_Marshal_Node *node);
char *oidl_marshal_node_valuestr(OIDL_Marshal_Node *node);
gboolean orbit_cbe_type_is_fixed_length(IDL_tree ts);
void orbit_cbe_write_node_typespec(FILE *of, OIDL_Marshal_Node *node);
gboolean orbit_cbe_type_is_builtin(IDL_tree);
void orbit_cbe_param_printptrs(FILE *of, IDL_tree param, IDL_ParamRole role);
void orbit_cbe_id_define_hack(FILE *fh, const char *def_prefix, const char *def_name, const char *def_value);
void orbit_cbe_id_cond_hack(FILE *fh, const char *def_prefix, const char *def_name, const char *def_value);

#endif
