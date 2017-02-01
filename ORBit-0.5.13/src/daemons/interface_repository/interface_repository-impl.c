#include <stdio.h>
#include <stdlib.h>
#include <orb/interface_repository.h>

/*** App-specific servant structures ***/
typedef struct {
	CORBA_Identifier search_name;
	CORBA_long levels_to_search;
	CORBA_DefinitionKind limit_type;
	CORBA_boolean exclude_inherited;
	GSList *seq;
} ContainedIterData;

/* Make sure all repo contents structs have attr_def_kind as the third
   member, attr_name and attr_version as the fourth and fifth (not all
   items have a name, so those that dont will have these set to NULL)
   and an object reference as the sixth */

/* This structure isn't actually used anywhere, apart from typecasting
   other structures: using one of the typed contents structs would be
   confusing - RHP */
typedef struct {
	POA_CORBA_Contained servant;  /* Not really a Contained */
	PortableServer_POA poa;
	CORBA_DefinitionKind attr_def_kind;
	CORBA_Identifier attr_name;
	CORBA_VersionSpec attr_version;
	CORBA_Object obj;
} IfaceRepoContents;

typedef struct {
	POA_CORBA_IDLType servant;
	PortableServer_POA poa;
	CORBA_DefinitionKind attr_def_kind;
	CORBA_Identifier attr_name;
	CORBA_VersionSpec attr_version;
	CORBA_IDLType obj;

	CORBA_TypeCode attr_type;
} impl_POA_CORBA_IDLType;

typedef struct {
	POA_CORBA_Repository servant;
	PortableServer_POA poa;
	CORBA_DefinitionKind attr_def_kind;
	CORBA_Identifier attr_name;
	CORBA_VersionSpec attr_version;
	CORBA_Repository obj;

	GHashTable *id_hash;

	/* Contains: constants, typedefs, exceptions, interface
	   definitions and modules
	 */
	GSList *contents;

	/* Contains: non-Containeds - string, wstring, primitive,
           sequence, array, fixed */
	GSList *noncontaineds;
} impl_POA_CORBA_Repository;

typedef struct {
	POA_CORBA_ModuleDef servant;
	PortableServer_POA poa;
	CORBA_DefinitionKind attr_def_kind;
	CORBA_Identifier attr_name;
	CORBA_VersionSpec attr_version;
	CORBA_ModuleDef obj;

	CORBA_RepositoryId attr_id;
	CORBA_Container attr_defined_in;

	/* Contains: constants, typedefs, exceptions, interface definitions
	   and other other modules
	 */
	GSList *contents;
} impl_POA_CORBA_ModuleDef;

typedef struct {
	POA_CORBA_ConstantDef servant;
	PortableServer_POA poa;
	CORBA_DefinitionKind attr_def_kind;
	CORBA_Identifier attr_name;
	CORBA_VersionSpec attr_version;
	CORBA_ConstantDef obj;

	CORBA_IDLType attr_type_def;
	CORBA_any *attr_value;
	CORBA_RepositoryId attr_id;
	CORBA_Container attr_defined_in;
} impl_POA_CORBA_ConstantDef;

typedef struct {
	POA_CORBA_TypedefDef servant;
	PortableServer_POA poa;
	CORBA_DefinitionKind attr_def_kind;
	CORBA_Identifier attr_name;
	CORBA_VersionSpec attr_version;
	CORBA_TypedefDef obj;

	CORBA_RepositoryId attr_id;
	CORBA_Container attr_defined_in;
	CORBA_TypeCode attr_type;
} impl_POA_CORBA_TypedefDef;

typedef struct {
	POA_CORBA_StructDef servant;
	PortableServer_POA poa;
	CORBA_DefinitionKind attr_def_kind;
	CORBA_Identifier attr_name;
	CORBA_VersionSpec attr_version;
	CORBA_StructDef obj;

	CORBA_StructMemberSeq *attr_members;
	CORBA_RepositoryId attr_id;
	CORBA_Container attr_defined_in;
	CORBA_TypeCode attr_type;

	/* Contains: structs, unions and enums
	 */
	GSList *contents;
} impl_POA_CORBA_StructDef;

typedef struct {
	POA_CORBA_UnionDef servant;
	PortableServer_POA poa;
	CORBA_DefinitionKind attr_def_kind;
	CORBA_Identifier attr_name;
	CORBA_VersionSpec attr_version;
	CORBA_UnionDef obj;

	CORBA_TypeCode attr_discriminator_type;
	CORBA_IDLType attr_discriminator_type_def;
	CORBA_UnionMemberSeq *attr_members;
	CORBA_RepositoryId attr_id;
	CORBA_Container attr_defined_in;
	CORBA_TypeCode attr_type;

	/* Contains: structs, unions and enums
	 */
	GSList *contents;
} impl_POA_CORBA_UnionDef;

typedef struct {
	POA_CORBA_EnumDef servant;
	PortableServer_POA poa;
	CORBA_DefinitionKind attr_def_kind;
	CORBA_Identifier attr_name;
	CORBA_VersionSpec attr_version;
	CORBA_EnumDef obj;

	CORBA_EnumMemberSeq *attr_members;
	CORBA_RepositoryId attr_id;
	CORBA_Container attr_defined_in;
	CORBA_TypeCode attr_type;
} impl_POA_CORBA_EnumDef;

typedef struct {
	POA_CORBA_AliasDef servant;
	PortableServer_POA poa;
	CORBA_DefinitionKind attr_def_kind;
	CORBA_Identifier attr_name;
	CORBA_VersionSpec attr_version;
	CORBA_AliasDef obj;

	CORBA_IDLType attr_original_type_def;
	CORBA_RepositoryId attr_id;
	CORBA_Container attr_defined_in;
	CORBA_TypeCode attr_type;
} impl_POA_CORBA_AliasDef;

typedef struct {
	POA_CORBA_PrimitiveDef servant;
	PortableServer_POA poa;
	CORBA_DefinitionKind attr_def_kind;
	CORBA_Identifier attr_name;
	CORBA_VersionSpec attr_version;
	CORBA_PrimitiveDef obj;

	CORBA_PrimitiveKind attr_kind;
	CORBA_TypeCode attr_type;
} impl_POA_CORBA_PrimitiveDef;

typedef struct {
	POA_CORBA_StringDef servant;
	PortableServer_POA poa;
	CORBA_DefinitionKind attr_def_kind;
	CORBA_Identifier attr_name;
	CORBA_VersionSpec attr_version;
	CORBA_StringDef obj;

	CORBA_unsigned_long attr_bound;
	CORBA_TypeCode attr_type;
} impl_POA_CORBA_StringDef;

typedef struct {
	POA_CORBA_WstringDef servant;
	PortableServer_POA poa;
	CORBA_DefinitionKind attr_def_kind;
	CORBA_Identifier attr_name;
	CORBA_VersionSpec attr_version;
	CORBA_WstringDef obj;

	CORBA_unsigned_long attr_bound;
	CORBA_TypeCode attr_type;
} impl_POA_CORBA_WstringDef;

typedef struct {
	POA_CORBA_FixedDef servant;
	PortableServer_POA poa;
	CORBA_DefinitionKind attr_def_kind;
	CORBA_Identifier attr_name;
	CORBA_VersionSpec attr_version;
	CORBA_FixedDef obj;

	CORBA_unsigned_short attr_digits;
	CORBA_short attr_scale;
	CORBA_TypeCode attr_type;
} impl_POA_CORBA_FixedDef;

typedef struct {
	POA_CORBA_SequenceDef servant;
	PortableServer_POA poa;
	CORBA_DefinitionKind attr_def_kind;
	CORBA_Identifier attr_name;
	CORBA_VersionSpec attr_version;
	CORBA_SequenceDef obj;

	CORBA_unsigned_long attr_bound;
	CORBA_IDLType attr_element_type_def;
	CORBA_TypeCode attr_type;
} impl_POA_CORBA_SequenceDef;

typedef struct {
	POA_CORBA_ArrayDef servant;
	PortableServer_POA poa;
	CORBA_DefinitionKind attr_def_kind;
	CORBA_Identifier attr_name;
	CORBA_VersionSpec attr_version;
	CORBA_ArrayDef obj;

	CORBA_unsigned_long attr_length;
	CORBA_IDLType attr_element_type_def;
	CORBA_TypeCode attr_type;
} impl_POA_CORBA_ArrayDef;

typedef struct {
	POA_CORBA_ExceptionDef servant;
	PortableServer_POA poa;
	CORBA_DefinitionKind attr_def_kind;
	CORBA_Identifier attr_name;
	CORBA_VersionSpec attr_version;
	CORBA_ExceptionDef obj;

	CORBA_TypeCode attr_type;
	CORBA_StructMemberSeq *attr_members;
	CORBA_RepositoryId attr_id;
	CORBA_Container attr_defined_in;

	/* Contains: structs, unions and enums
	 */
	GSList *contents;
} impl_POA_CORBA_ExceptionDef;

typedef struct {
	POA_CORBA_AttributeDef servant;
	PortableServer_POA poa;
	CORBA_DefinitionKind attr_def_kind;
	CORBA_Identifier attr_name;
	CORBA_VersionSpec attr_version;
	CORBA_AttributeDef obj;

	CORBA_TypeCode attr_type;
	CORBA_IDLType attr_type_def;
	CORBA_AttributeMode attr_mode;
	CORBA_RepositoryId attr_id;
	CORBA_Container attr_defined_in;
} impl_POA_CORBA_AttributeDef;

typedef struct {
	POA_CORBA_OperationDef servant;
	PortableServer_POA poa;
	CORBA_DefinitionKind attr_def_kind;
	CORBA_Identifier attr_name;
	CORBA_VersionSpec attr_version;
	CORBA_OperationDef obj;

	CORBA_TypeCode attr_result;
	CORBA_IDLType attr_result_def;
	CORBA_ParDescriptionSeq *attr_params;
	CORBA_OperationMode attr_mode;
	CORBA_ContextIdSeq *attr_contexts;
	CORBA_ExceptionDefSeq *attr_exceptions;
	CORBA_RepositoryId attr_id;
	CORBA_Container attr_defined_in;
} impl_POA_CORBA_OperationDef;

typedef struct {
	POA_CORBA_InterfaceDef servant;
	PortableServer_POA poa;
	CORBA_DefinitionKind attr_def_kind;
	CORBA_Identifier attr_name;
	CORBA_VersionSpec attr_version;
	CORBA_InterfaceDef obj;

	CORBA_InterfaceDefSeq *attr_base_interfaces;
	CORBA_RepositoryId attr_id;
	CORBA_Container attr_defined_in;
	CORBA_TypeCode attr_type;

	/* Contains: constants, types, exceptions, operations and attributes */
	GSList *contents;
	/* Contains: operations contained via inheritance */
	GSList *inherited;
} impl_POA_CORBA_InterfaceDef;

/*** Implementation stub prototypes ***/

static void impl_CORBA_IDLType__destroy(impl_POA_CORBA_IDLType * servant,
					CORBA_Environment * ev);

CORBA_TypeCode
impl_CORBA_IDLType__get_type(impl_POA_CORBA_IDLType * servant,
			     CORBA_Environment * ev);

CORBA_DefinitionKind
impl_CORBA_IDLType__get_def_kind(impl_POA_CORBA_IDLType * servant,
				 CORBA_Environment * ev);
void
 impl_CORBA_IDLType_destroy(impl_POA_CORBA_IDLType * servant,
 			    CORBA_Environment * ev);

static void impl_CORBA_Repository__destroy(impl_POA_CORBA_Repository * servant,
					   CORBA_Environment * ev);

CORBA_Contained
impl_CORBA_Repository_lookup_id(impl_POA_CORBA_Repository * servant,
				CORBA_RepositoryId search_id,
				CORBA_Environment * ev);

CORBA_PrimitiveDef
impl_CORBA_Repository_get_primitive(impl_POA_CORBA_Repository * servant,
				    CORBA_PrimitiveKind kind,
				    CORBA_Environment * ev);

CORBA_StringDef
impl_CORBA_Repository_create_string(impl_POA_CORBA_Repository * servant,
				    CORBA_unsigned_long bound,
				    CORBA_Environment * ev);

CORBA_WstringDef
impl_CORBA_Repository_create_wstring(impl_POA_CORBA_Repository * servant,
				     CORBA_unsigned_long bound,
				     CORBA_Environment * ev);

CORBA_SequenceDef
impl_CORBA_Repository_create_sequence(impl_POA_CORBA_Repository * servant,
				      CORBA_unsigned_long bound,
				      CORBA_IDLType element_type,
				      CORBA_Environment * ev);

CORBA_ArrayDef
impl_CORBA_Repository_create_array(impl_POA_CORBA_Repository * servant,
				   CORBA_unsigned_long length,
				   CORBA_IDLType element_type,
				   CORBA_Environment * ev);

CORBA_FixedDef
impl_CORBA_Repository_create_fixed(impl_POA_CORBA_Repository * servant,
				   CORBA_unsigned_short digits,
				   CORBA_short scale,
				   CORBA_Environment * ev);

CORBA_DefinitionKind
impl_CORBA_Repository__get_def_kind(impl_POA_CORBA_Repository * servant,
				    CORBA_Environment * ev);
void
 impl_CORBA_Repository_destroy(impl_POA_CORBA_Repository * servant,
			       CORBA_Environment * ev);

CORBA_Contained
impl_CORBA_Repository_lookup(impl_POA_CORBA_Repository * servant,
			     CORBA_ScopedName search_name,
			     CORBA_Environment * ev);
CORBA_ContainedSeq *
 impl_CORBA_Repository_contents(impl_POA_CORBA_Repository * servant,
				CORBA_DefinitionKind limit_type,
				CORBA_boolean exclude_inherited,
				CORBA_Environment * ev);
CORBA_ContainedSeq *
 impl_CORBA_Repository_lookup_name(impl_POA_CORBA_Repository * servant,
				   CORBA_Identifier search_name,
				   CORBA_long levels_to_search,
				   CORBA_DefinitionKind limit_type,
				   CORBA_boolean exclude_inherited,
				   CORBA_Environment * ev);
CORBA_Container_DescriptionSeq *
 impl_CORBA_Repository_describe_contents(impl_POA_CORBA_Repository * servant,
					 CORBA_DefinitionKind limit_type,
					 CORBA_boolean exclude_inherited,
					 CORBA_long max_returned_objs,
					 CORBA_Environment * ev);

CORBA_ModuleDef
impl_CORBA_Repository_create_module(impl_POA_CORBA_Repository * servant,
				    CORBA_RepositoryId id,
				    CORBA_Identifier name,
				    CORBA_VersionSpec version,
				    CORBA_Environment * ev);
CORBA_ConstantDef
impl_CORBA_Repository_create_constant(impl_POA_CORBA_Repository * servant,
				      CORBA_RepositoryId id,
				      CORBA_Identifier name,
				      CORBA_VersionSpec version,
				      CORBA_IDLType type,
				      CORBA_any * value,
				      CORBA_Environment * ev);
CORBA_StructDef
impl_CORBA_Repository_create_struct(impl_POA_CORBA_Repository * servant,
				    CORBA_RepositoryId id,
				    CORBA_Identifier name,
				    CORBA_VersionSpec version,
				    CORBA_StructMemberSeq * members,
				    CORBA_Environment * ev);
CORBA_UnionDef
impl_CORBA_Repository_create_union(impl_POA_CORBA_Repository * servant,
				   CORBA_RepositoryId id,
				   CORBA_Identifier name,
				   CORBA_VersionSpec version,
				   CORBA_IDLType discriminator_type,
				   CORBA_UnionMemberSeq * members,
				   CORBA_Environment * ev);
CORBA_EnumDef
impl_CORBA_Repository_create_enum(impl_POA_CORBA_Repository * servant,
				  CORBA_RepositoryId id,
				  CORBA_Identifier name,
				  CORBA_VersionSpec version,
				  CORBA_EnumMemberSeq * members,
				  CORBA_Environment * ev);
CORBA_AliasDef
impl_CORBA_Repository_create_alias(impl_POA_CORBA_Repository * servant,
				   CORBA_RepositoryId id,
				   CORBA_Identifier name,
				   CORBA_VersionSpec version,
				   CORBA_IDLType original_type,
				   CORBA_Environment * ev);
CORBA_InterfaceDef
impl_CORBA_Repository_create_interface(impl_POA_CORBA_Repository * servant,
				       CORBA_RepositoryId id,
				       CORBA_Identifier name,
				       CORBA_VersionSpec version,
				       CORBA_InterfaceDefSeq * base_interfaces,
				       CORBA_Environment * ev);
CORBA_ExceptionDef
impl_CORBA_Repository_create_exception(impl_POA_CORBA_Repository * servant,
				       CORBA_RepositoryId id,
				       CORBA_Identifier name,
				       CORBA_VersionSpec version,
				       CORBA_StructMemberSeq * members,
				       CORBA_Environment * ev);

static void impl_CORBA_ModuleDef__destroy(impl_POA_CORBA_ModuleDef * servant,
					  CORBA_Environment * ev);

CORBA_DefinitionKind
impl_CORBA_ModuleDef__get_def_kind(impl_POA_CORBA_ModuleDef * servant,
				   CORBA_Environment * ev);
void
 impl_CORBA_ModuleDef_destroy(impl_POA_CORBA_ModuleDef * servant,
			      CORBA_Environment * ev);

CORBA_Contained
impl_CORBA_ModuleDef_lookup(impl_POA_CORBA_ModuleDef * servant,
			    CORBA_ScopedName search_name,
			    CORBA_Environment * ev);
CORBA_ContainedSeq *
 impl_CORBA_ModuleDef_contents(impl_POA_CORBA_ModuleDef * servant,
			       CORBA_DefinitionKind limit_type,
			       CORBA_boolean exclude_inherited,
			       CORBA_Environment * ev);
CORBA_ContainedSeq *
 impl_CORBA_ModuleDef_lookup_name(impl_POA_CORBA_ModuleDef * servant,
				  CORBA_Identifier search_name,
				  CORBA_long levels_to_search,
				  CORBA_DefinitionKind limit_type,
				  CORBA_boolean exclude_inherited,
				  CORBA_Environment * ev);
CORBA_Container_DescriptionSeq *
 impl_CORBA_ModuleDef_describe_contents(impl_POA_CORBA_ModuleDef * servant,
					CORBA_DefinitionKind limit_type,
					CORBA_boolean exclude_inherited,
					CORBA_long max_returned_objs,
					CORBA_Environment * ev);

CORBA_ModuleDef
impl_CORBA_ModuleDef_create_module(impl_POA_CORBA_ModuleDef * servant,
				   CORBA_RepositoryId id,
				   CORBA_Identifier name,
				   CORBA_VersionSpec version,
				   CORBA_Environment * ev);
CORBA_ConstantDef
impl_CORBA_ModuleDef_create_constant(impl_POA_CORBA_ModuleDef * servant,
				     CORBA_RepositoryId id,
				     CORBA_Identifier name,
				     CORBA_VersionSpec version,
				     CORBA_IDLType type,
				     CORBA_any * value,
				     CORBA_Environment * ev);
CORBA_StructDef
impl_CORBA_ModuleDef_create_struct(impl_POA_CORBA_ModuleDef * servant,
				   CORBA_RepositoryId id,
				   CORBA_Identifier name,
				   CORBA_VersionSpec version,
				   CORBA_StructMemberSeq * members,
				   CORBA_Environment * ev);
CORBA_UnionDef
impl_CORBA_ModuleDef_create_union(impl_POA_CORBA_ModuleDef * servant,
				  CORBA_RepositoryId id,
				  CORBA_Identifier name,
				  CORBA_VersionSpec version,
				  CORBA_IDLType discriminator_type,
				  CORBA_UnionMemberSeq * members,
				  CORBA_Environment * ev);
CORBA_EnumDef
impl_CORBA_ModuleDef_create_enum(impl_POA_CORBA_ModuleDef * servant,
				 CORBA_RepositoryId id,
				 CORBA_Identifier name,
				 CORBA_VersionSpec version,
				 CORBA_EnumMemberSeq * members,
				 CORBA_Environment * ev);
CORBA_AliasDef
impl_CORBA_ModuleDef_create_alias(impl_POA_CORBA_ModuleDef * servant,
				  CORBA_RepositoryId id,
				  CORBA_Identifier name,
				  CORBA_VersionSpec version,
				  CORBA_IDLType original_type,
				  CORBA_Environment * ev);
CORBA_InterfaceDef
impl_CORBA_ModuleDef_create_interface(impl_POA_CORBA_ModuleDef * servant,
				      CORBA_RepositoryId id,
				      CORBA_Identifier name,
				      CORBA_VersionSpec version,
				      CORBA_InterfaceDefSeq * base_interfaces,
				      CORBA_Environment * ev);
CORBA_ExceptionDef
impl_CORBA_ModuleDef_create_exception(impl_POA_CORBA_ModuleDef * servant,
				      CORBA_RepositoryId id,
				      CORBA_Identifier name,
				      CORBA_VersionSpec version,
				      CORBA_StructMemberSeq * members,
				      CORBA_Environment * ev);
CORBA_RepositoryId
impl_CORBA_ModuleDef__get_id(impl_POA_CORBA_ModuleDef * servant,
			     CORBA_Environment * ev);
void
 impl_CORBA_ModuleDef__set_id(impl_POA_CORBA_ModuleDef * servant,
			      CORBA_RepositoryId value,
			      CORBA_Environment * ev);

CORBA_Identifier
impl_CORBA_ModuleDef__get_name(impl_POA_CORBA_ModuleDef * servant,
			       CORBA_Environment * ev);
void
 impl_CORBA_ModuleDef__set_name(impl_POA_CORBA_ModuleDef * servant,
				CORBA_Identifier value,
				CORBA_Environment * ev);

CORBA_VersionSpec
impl_CORBA_ModuleDef__get_version(impl_POA_CORBA_ModuleDef * servant,
				  CORBA_Environment * ev);
void
 impl_CORBA_ModuleDef__set_version(impl_POA_CORBA_ModuleDef * servant,
				   CORBA_VersionSpec value,
				   CORBA_Environment * ev);

CORBA_Container
impl_CORBA_ModuleDef__get_defined_in(impl_POA_CORBA_ModuleDef * servant,
				     CORBA_Environment * ev);
CORBA_ScopedName
impl_CORBA_ModuleDef__get_absolute_name(impl_POA_CORBA_ModuleDef * servant,
					CORBA_Environment * ev);
CORBA_Repository
impl_CORBA_ModuleDef__get_containing_repository(impl_POA_CORBA_ModuleDef * servant,
						CORBA_Environment * ev);
CORBA_Contained_Description *
 impl_CORBA_ModuleDef_describe(impl_POA_CORBA_ModuleDef * servant,
			       CORBA_Environment * ev);
void
 impl_CORBA_ModuleDef_move(impl_POA_CORBA_ModuleDef * servant,
			   CORBA_Container new_container,
			   CORBA_Identifier new_name,
			   CORBA_VersionSpec new_version,
			   CORBA_Environment * ev);

static void impl_CORBA_ConstantDef__destroy(impl_POA_CORBA_ConstantDef * servant,
					    CORBA_Environment * ev);

CORBA_TypeCode
impl_CORBA_ConstantDef__get_type(impl_POA_CORBA_ConstantDef * servant,
				 CORBA_Environment * ev);

CORBA_IDLType
impl_CORBA_ConstantDef__get_type_def(impl_POA_CORBA_ConstantDef * servant,
				     CORBA_Environment * ev);
void
 impl_CORBA_ConstantDef__set_type_def(impl_POA_CORBA_ConstantDef * servant,
				      CORBA_IDLType value,
				      CORBA_Environment * ev);

CORBA_any *
 impl_CORBA_ConstantDef__get_value(impl_POA_CORBA_ConstantDef * servant,
				   CORBA_Environment * ev);
void
 impl_CORBA_ConstantDef__set_value(impl_POA_CORBA_ConstantDef * servant,
				   CORBA_any * value,
				   CORBA_Environment * ev);

CORBA_DefinitionKind
impl_CORBA_ConstantDef__get_def_kind(impl_POA_CORBA_ConstantDef * servant,
				     CORBA_Environment * ev);
void
 impl_CORBA_ConstantDef_destroy(impl_POA_CORBA_ConstantDef * servant,
				CORBA_Environment * ev);

CORBA_RepositoryId
impl_CORBA_ConstantDef__get_id(impl_POA_CORBA_ConstantDef * servant,
			       CORBA_Environment * ev);
void
 impl_CORBA_ConstantDef__set_id(impl_POA_CORBA_ConstantDef * servant,
				CORBA_RepositoryId value,
				CORBA_Environment * ev);

CORBA_Identifier
impl_CORBA_ConstantDef__get_name(impl_POA_CORBA_ConstantDef * servant,
				 CORBA_Environment * ev);
void
 impl_CORBA_ConstantDef__set_name(impl_POA_CORBA_ConstantDef * servant,
				  CORBA_Identifier value,
				  CORBA_Environment * ev);

CORBA_VersionSpec
impl_CORBA_ConstantDef__get_version(impl_POA_CORBA_ConstantDef * servant,
				    CORBA_Environment * ev);
void
 impl_CORBA_ConstantDef__set_version(impl_POA_CORBA_ConstantDef * servant,
				     CORBA_VersionSpec value,
				     CORBA_Environment * ev);

CORBA_Container
impl_CORBA_ConstantDef__get_defined_in(impl_POA_CORBA_ConstantDef * servant,
				       CORBA_Environment * ev);
CORBA_ScopedName
impl_CORBA_ConstantDef__get_absolute_name(impl_POA_CORBA_ConstantDef * servant,
					  CORBA_Environment * ev);
CORBA_Repository
impl_CORBA_ConstantDef__get_containing_repository(impl_POA_CORBA_ConstantDef * servant,
						  CORBA_Environment * ev);
CORBA_Contained_Description *
 impl_CORBA_ConstantDef_describe(impl_POA_CORBA_ConstantDef * servant,
				 CORBA_Environment * ev);
void
 impl_CORBA_ConstantDef_move(impl_POA_CORBA_ConstantDef * servant,
			     CORBA_Container new_container,
			     CORBA_Identifier new_name,
			     CORBA_VersionSpec new_version,
			     CORBA_Environment * ev);

static void impl_CORBA_TypedefDef__destroy(impl_POA_CORBA_TypedefDef * servant,
					   CORBA_Environment * ev);

CORBA_DefinitionKind
impl_CORBA_TypedefDef__get_def_kind(impl_POA_CORBA_TypedefDef * servant,
				    CORBA_Environment * ev);
void
 impl_CORBA_TypedefDef_destroy(impl_POA_CORBA_TypedefDef * servant,
 			       CORBA_Environment * ev);

CORBA_RepositoryId
impl_CORBA_TypedefDef__get_id(impl_POA_CORBA_TypedefDef * servant,
			      CORBA_Environment * ev);
void
 impl_CORBA_TypedefDef__set_id(impl_POA_CORBA_TypedefDef * servant,
 			       CORBA_RepositoryId value,
			       CORBA_Environment * ev);

CORBA_Identifier
impl_CORBA_TypedefDef__get_name(impl_POA_CORBA_TypedefDef * servant,
				CORBA_Environment * ev);
void
 impl_CORBA_TypedefDef__set_name(impl_POA_CORBA_TypedefDef * servant,
 				 CORBA_Identifier value,
				 CORBA_Environment * ev);

CORBA_VersionSpec
impl_CORBA_TypedefDef__get_version(impl_POA_CORBA_TypedefDef * servant,
				   CORBA_Environment * ev);
void
 impl_CORBA_TypedefDef__set_version(impl_POA_CORBA_TypedefDef * servant,
 				    CORBA_VersionSpec value,
				    CORBA_Environment * ev);

CORBA_Container
impl_CORBA_TypedefDef__get_defined_in(impl_POA_CORBA_TypedefDef * servant,
				      CORBA_Environment * ev);
CORBA_ScopedName
impl_CORBA_TypedefDef__get_absolute_name(impl_POA_CORBA_TypedefDef * servant,
					 CORBA_Environment * ev);
CORBA_Repository
impl_CORBA_TypedefDef__get_containing_repository(impl_POA_CORBA_TypedefDef * servant,
						 CORBA_Environment * ev);
CORBA_Contained_Description *
 impl_CORBA_TypedefDef_describe(impl_POA_CORBA_TypedefDef * servant,
 				CORBA_Environment * ev);
void
 impl_CORBA_TypedefDef_move(impl_POA_CORBA_TypedefDef * servant,
 			    CORBA_Container new_container,
			    CORBA_Identifier new_name,
			    CORBA_VersionSpec new_version,
			    CORBA_Environment * ev);

CORBA_TypeCode
impl_CORBA_TypedefDef__get_type(impl_POA_CORBA_TypedefDef * servant,
				CORBA_Environment * ev);

static void impl_CORBA_StructDef__destroy(impl_POA_CORBA_StructDef * servant,
					  CORBA_Environment * ev);
CORBA_StructMemberSeq *
 impl_CORBA_StructDef__get_members(impl_POA_CORBA_StructDef * servant,
				   CORBA_Environment * ev);
void
 impl_CORBA_StructDef__set_members(impl_POA_CORBA_StructDef * servant,
				   CORBA_StructMemberSeq * value,
				   CORBA_Environment * ev);

CORBA_DefinitionKind
impl_CORBA_StructDef__get_def_kind(impl_POA_CORBA_StructDef * servant,
				   CORBA_Environment * ev);
void
 impl_CORBA_StructDef_destroy(impl_POA_CORBA_StructDef * servant,
			      CORBA_Environment * ev);

CORBA_RepositoryId
impl_CORBA_StructDef__get_id(impl_POA_CORBA_StructDef * servant,
			     CORBA_Environment * ev);
void
 impl_CORBA_StructDef__set_id(impl_POA_CORBA_StructDef * servant,
			      CORBA_RepositoryId value,
			      CORBA_Environment * ev);

CORBA_Identifier
impl_CORBA_StructDef__get_name(impl_POA_CORBA_StructDef * servant,
			       CORBA_Environment * ev);
void
 impl_CORBA_StructDef__set_name(impl_POA_CORBA_StructDef * servant,
				CORBA_Identifier value,
				CORBA_Environment * ev);

CORBA_VersionSpec
impl_CORBA_StructDef__get_version(impl_POA_CORBA_StructDef * servant,
				  CORBA_Environment * ev);
void
 impl_CORBA_StructDef__set_version(impl_POA_CORBA_StructDef * servant,
				   CORBA_VersionSpec value,
				   CORBA_Environment * ev);

CORBA_Container
impl_CORBA_StructDef__get_defined_in(impl_POA_CORBA_StructDef * servant,
				     CORBA_Environment * ev);
CORBA_ScopedName
impl_CORBA_StructDef__get_absolute_name(impl_POA_CORBA_StructDef * servant,
					CORBA_Environment * ev);
CORBA_Repository
impl_CORBA_StructDef__get_containing_repository(impl_POA_CORBA_StructDef * servant,
						CORBA_Environment * ev);
CORBA_Contained_Description *
 impl_CORBA_StructDef_describe(impl_POA_CORBA_StructDef * servant,
			       CORBA_Environment * ev);
void
 impl_CORBA_StructDef_move(impl_POA_CORBA_StructDef * servant,
			   CORBA_Container new_container,
			   CORBA_Identifier new_name,
			   CORBA_VersionSpec new_version,
			   CORBA_Environment * ev);

CORBA_TypeCode
impl_CORBA_StructDef__get_type(impl_POA_CORBA_StructDef * servant,
			       CORBA_Environment * ev);
CORBA_Contained
impl_CORBA_StructDef_lookup(impl_POA_CORBA_StructDef * servant,
			    CORBA_ScopedName search_name,
			    CORBA_Environment * ev);
CORBA_ContainedSeq *
 impl_CORBA_StructDef_contents(impl_POA_CORBA_StructDef * servant,
			       CORBA_DefinitionKind limit_type,
			       CORBA_boolean exclude_inherited,
			       CORBA_Environment * ev);
CORBA_ContainedSeq *
 impl_CORBA_StructDef_lookup_name(impl_POA_CORBA_StructDef * servant,
				  CORBA_Identifier search_name,
				  CORBA_long levels_to_search,
				  CORBA_DefinitionKind limit_type,
				  CORBA_boolean exclude_inherited,
				  CORBA_Environment * ev);
CORBA_Container_DescriptionSeq *
 impl_CORBA_StructDef_describe_contents(impl_POA_CORBA_StructDef * servant,
					CORBA_DefinitionKind limit_type,
					CORBA_boolean exclude_inherited,
					CORBA_long max_returned_objs,
					CORBA_Environment * ev);

CORBA_ModuleDef
impl_CORBA_StructDef_create_module(impl_POA_CORBA_StructDef * servant,
				   CORBA_RepositoryId id,
				   CORBA_Identifier name,
				   CORBA_VersionSpec version,
				   CORBA_Environment * ev);
CORBA_ConstantDef
impl_CORBA_StructDef_create_constant(impl_POA_CORBA_StructDef * servant,
				     CORBA_RepositoryId id,
				     CORBA_Identifier name,
				     CORBA_VersionSpec version,
				     CORBA_IDLType type,
				     CORBA_any * value,
				     CORBA_Environment * ev);
CORBA_StructDef
impl_CORBA_StructDef_create_struct(impl_POA_CORBA_StructDef * servant,
				   CORBA_RepositoryId id,
				   CORBA_Identifier name,
				   CORBA_VersionSpec version,
				   CORBA_StructMemberSeq * members,
				   CORBA_Environment * ev);
CORBA_UnionDef
impl_CORBA_StructDef_create_union(impl_POA_CORBA_StructDef * servant,
				  CORBA_RepositoryId id,
				  CORBA_Identifier name,
				  CORBA_VersionSpec version,
				  CORBA_IDLType discriminator_type,
				  CORBA_UnionMemberSeq * members,
				  CORBA_Environment * ev);
CORBA_EnumDef
impl_CORBA_StructDef_create_enum(impl_POA_CORBA_StructDef * servant,
				 CORBA_RepositoryId id,
				 CORBA_Identifier name,
				 CORBA_VersionSpec version,
				 CORBA_EnumMemberSeq * members,
				 CORBA_Environment * ev);
CORBA_AliasDef
impl_CORBA_StructDef_create_alias(impl_POA_CORBA_StructDef * servant,
				  CORBA_RepositoryId id,
				  CORBA_Identifier name,
				  CORBA_VersionSpec version,
				  CORBA_IDLType original_type,
				  CORBA_Environment * ev);
CORBA_InterfaceDef
impl_CORBA_StructDef_create_interface(impl_POA_CORBA_StructDef * servant,
				      CORBA_RepositoryId id,
				      CORBA_Identifier name,
				      CORBA_VersionSpec version,
				      CORBA_InterfaceDefSeq * base_interfaces,
				      CORBA_Environment * ev);
CORBA_ExceptionDef
impl_CORBA_StructDef_create_exception(impl_POA_CORBA_StructDef * servant,
				      CORBA_RepositoryId id,
				      CORBA_Identifier name,
				      CORBA_VersionSpec version,
				      CORBA_StructMemberSeq * members,
				      CORBA_Environment * ev);

static void impl_CORBA_UnionDef__destroy(impl_POA_CORBA_UnionDef * servant,
					 CORBA_Environment * ev);

CORBA_TypeCode
impl_CORBA_UnionDef__get_discriminator_type(impl_POA_CORBA_UnionDef * servant,
					    CORBA_Environment * ev);

CORBA_IDLType
impl_CORBA_UnionDef__get_discriminator_type_def(impl_POA_CORBA_UnionDef * servant,
						CORBA_Environment * ev);
void
 impl_CORBA_UnionDef__set_discriminator_type_def(impl_POA_CORBA_UnionDef * servant,
						 CORBA_IDLType value,
						 CORBA_Environment * ev);

CORBA_UnionMemberSeq *
 impl_CORBA_UnionDef__get_members(impl_POA_CORBA_UnionDef * servant,
				  CORBA_Environment * ev);
void
 impl_CORBA_UnionDef__set_members(impl_POA_CORBA_UnionDef * servant,
				  CORBA_UnionMemberSeq * value,
				  CORBA_Environment * ev);

CORBA_DefinitionKind
impl_CORBA_UnionDef__get_def_kind(impl_POA_CORBA_UnionDef * servant,
				  CORBA_Environment * ev);
void
 impl_CORBA_UnionDef_destroy(impl_POA_CORBA_UnionDef * servant,
			     CORBA_Environment * ev);

CORBA_RepositoryId
impl_CORBA_UnionDef__get_id(impl_POA_CORBA_UnionDef * servant,
			    CORBA_Environment * ev);
void
 impl_CORBA_UnionDef__set_id(impl_POA_CORBA_UnionDef * servant,
			     CORBA_RepositoryId value,
			     CORBA_Environment * ev);

CORBA_Identifier
impl_CORBA_UnionDef__get_name(impl_POA_CORBA_UnionDef * servant,
			      CORBA_Environment * ev);
void
 impl_CORBA_UnionDef__set_name(impl_POA_CORBA_UnionDef * servant,
			       CORBA_Identifier value,
			       CORBA_Environment * ev);

CORBA_VersionSpec
impl_CORBA_UnionDef__get_version(impl_POA_CORBA_UnionDef * servant,
				 CORBA_Environment * ev);
void
 impl_CORBA_UnionDef__set_version(impl_POA_CORBA_UnionDef * servant,
				  CORBA_VersionSpec value,
				  CORBA_Environment * ev);

CORBA_Container
impl_CORBA_UnionDef__get_defined_in(impl_POA_CORBA_UnionDef * servant,
				    CORBA_Environment * ev);
CORBA_ScopedName
impl_CORBA_UnionDef__get_absolute_name(impl_POA_CORBA_UnionDef * servant,
				       CORBA_Environment * ev);
CORBA_Repository
impl_CORBA_UnionDef__get_containing_repository(impl_POA_CORBA_UnionDef * servant,
					       CORBA_Environment * ev);
CORBA_Contained_Description *
 impl_CORBA_UnionDef_describe(impl_POA_CORBA_UnionDef * servant,
			      CORBA_Environment * ev);
void
 impl_CORBA_UnionDef_move(impl_POA_CORBA_UnionDef * servant,
			  CORBA_Container new_container,
			  CORBA_Identifier new_name,
			  CORBA_VersionSpec new_version,
			  CORBA_Environment * ev);

CORBA_TypeCode
impl_CORBA_UnionDef__get_type(impl_POA_CORBA_UnionDef * servant,
			      CORBA_Environment * ev);
CORBA_Contained
impl_CORBA_UnionDef_lookup(impl_POA_CORBA_UnionDef * servant,
			   CORBA_ScopedName search_name,
			   CORBA_Environment * ev);
CORBA_ContainedSeq *
 impl_CORBA_UnionDef_contents(impl_POA_CORBA_UnionDef * servant,
			      CORBA_DefinitionKind limit_type,
			      CORBA_boolean exclude_inherited,
			      CORBA_Environment * ev);
CORBA_ContainedSeq *
 impl_CORBA_UnionDef_lookup_name(impl_POA_CORBA_UnionDef * servant,
				 CORBA_Identifier search_name,
				 CORBA_long levels_to_search,
				 CORBA_DefinitionKind limit_type,
				 CORBA_boolean exclude_inherited,
				 CORBA_Environment * ev);
CORBA_Container_DescriptionSeq *
 impl_CORBA_UnionDef_describe_contents(impl_POA_CORBA_UnionDef * servant,
				       CORBA_DefinitionKind limit_type,
				       CORBA_boolean exclude_inherited,
				       CORBA_long max_returned_objs,
				       CORBA_Environment * ev);

CORBA_ModuleDef
impl_CORBA_UnionDef_create_module(impl_POA_CORBA_UnionDef * servant,
				  CORBA_RepositoryId id,
				  CORBA_Identifier name,
				  CORBA_VersionSpec version,
				  CORBA_Environment * ev);
CORBA_ConstantDef
impl_CORBA_UnionDef_create_constant(impl_POA_CORBA_UnionDef * servant,
				    CORBA_RepositoryId id,
				    CORBA_Identifier name,
				    CORBA_VersionSpec version,
				    CORBA_IDLType type,
				    CORBA_any * value,
				    CORBA_Environment * ev);
CORBA_StructDef
impl_CORBA_UnionDef_create_struct(impl_POA_CORBA_UnionDef * servant,
				  CORBA_RepositoryId id,
				  CORBA_Identifier name,
				  CORBA_VersionSpec version,
				  CORBA_StructMemberSeq * members,
				  CORBA_Environment * ev);
CORBA_UnionDef
impl_CORBA_UnionDef_create_union(impl_POA_CORBA_UnionDef * servant,
				 CORBA_RepositoryId id,
				 CORBA_Identifier name,
				 CORBA_VersionSpec version,
				 CORBA_IDLType discriminator_type,
				 CORBA_UnionMemberSeq * members,
				 CORBA_Environment * ev);
CORBA_EnumDef
impl_CORBA_UnionDef_create_enum(impl_POA_CORBA_UnionDef * servant,
				CORBA_RepositoryId id,
				CORBA_Identifier name,
				CORBA_VersionSpec version,
				CORBA_EnumMemberSeq * members,
				CORBA_Environment * ev);
CORBA_AliasDef
impl_CORBA_UnionDef_create_alias(impl_POA_CORBA_UnionDef * servant,
				 CORBA_RepositoryId id,
				 CORBA_Identifier name,
				 CORBA_VersionSpec version,
				 CORBA_IDLType original_type,
				 CORBA_Environment * ev);
CORBA_InterfaceDef
impl_CORBA_UnionDef_create_interface(impl_POA_CORBA_UnionDef * servant,
				     CORBA_RepositoryId id,
				     CORBA_Identifier name,
				     CORBA_VersionSpec version,
				     CORBA_InterfaceDefSeq * base_interfaces,
				     CORBA_Environment * ev);
CORBA_ExceptionDef
impl_CORBA_UnionDef_create_exception(impl_POA_CORBA_UnionDef * servant,
				     CORBA_RepositoryId id,
				     CORBA_Identifier name,
				     CORBA_VersionSpec version,
				     CORBA_StructMemberSeq * members,
				     CORBA_Environment * ev);

static void impl_CORBA_EnumDef__destroy(impl_POA_CORBA_EnumDef * servant,
					CORBA_Environment * ev);
CORBA_EnumMemberSeq *
 impl_CORBA_EnumDef__get_members(impl_POA_CORBA_EnumDef * servant,
				 CORBA_Environment * ev);
void
 impl_CORBA_EnumDef__set_members(impl_POA_CORBA_EnumDef * servant,
				 CORBA_EnumMemberSeq * value,
				 CORBA_Environment * ev);

CORBA_DefinitionKind
impl_CORBA_EnumDef__get_def_kind(impl_POA_CORBA_EnumDef * servant,
				 CORBA_Environment * ev);
void
 impl_CORBA_EnumDef_destroy(impl_POA_CORBA_EnumDef * servant,
			    CORBA_Environment * ev);

CORBA_RepositoryId
impl_CORBA_EnumDef__get_id(impl_POA_CORBA_EnumDef * servant,
			   CORBA_Environment * ev);
void
 impl_CORBA_EnumDef__set_id(impl_POA_CORBA_EnumDef * servant,
			    CORBA_RepositoryId value,
			    CORBA_Environment * ev);

CORBA_Identifier
impl_CORBA_EnumDef__get_name(impl_POA_CORBA_EnumDef * servant,
			     CORBA_Environment * ev);
void
 impl_CORBA_EnumDef__set_name(impl_POA_CORBA_EnumDef * servant,
			      CORBA_Identifier value,
			      CORBA_Environment * ev);

CORBA_VersionSpec
impl_CORBA_EnumDef__get_version(impl_POA_CORBA_EnumDef * servant,
				CORBA_Environment * ev);
void
 impl_CORBA_EnumDef__set_version(impl_POA_CORBA_EnumDef * servant,
				 CORBA_VersionSpec value,
				 CORBA_Environment * ev);

CORBA_Container
impl_CORBA_EnumDef__get_defined_in(impl_POA_CORBA_EnumDef * servant,
				   CORBA_Environment * ev);
CORBA_ScopedName
impl_CORBA_EnumDef__get_absolute_name(impl_POA_CORBA_EnumDef * servant,
				      CORBA_Environment * ev);
CORBA_Repository
impl_CORBA_EnumDef__get_containing_repository(impl_POA_CORBA_EnumDef * servant,
					      CORBA_Environment * ev);
CORBA_Contained_Description *
 impl_CORBA_EnumDef_describe(impl_POA_CORBA_EnumDef * servant,
			     CORBA_Environment * ev);
void
 impl_CORBA_EnumDef_move(impl_POA_CORBA_EnumDef * servant,
			 CORBA_Container new_container,
			 CORBA_Identifier new_name,
			 CORBA_VersionSpec new_version,
			 CORBA_Environment * ev);

CORBA_TypeCode
impl_CORBA_EnumDef__get_type(impl_POA_CORBA_EnumDef * servant,
			     CORBA_Environment * ev);

static void impl_CORBA_AliasDef__destroy(impl_POA_CORBA_AliasDef * servant,
					 CORBA_Environment * ev);

CORBA_IDLType
impl_CORBA_AliasDef__get_original_type_def(impl_POA_CORBA_AliasDef * servant,
					   CORBA_Environment * ev);
void
 impl_CORBA_AliasDef__set_original_type_def(impl_POA_CORBA_AliasDef * servant,
					    CORBA_IDLType value,
					    CORBA_Environment * ev);

CORBA_DefinitionKind
impl_CORBA_AliasDef__get_def_kind(impl_POA_CORBA_AliasDef * servant,
				  CORBA_Environment * ev);
void
 impl_CORBA_AliasDef_destroy(impl_POA_CORBA_AliasDef * servant,
			     CORBA_Environment * ev);

CORBA_RepositoryId
impl_CORBA_AliasDef__get_id(impl_POA_CORBA_AliasDef * servant,
			    CORBA_Environment * ev);
void
 impl_CORBA_AliasDef__set_id(impl_POA_CORBA_AliasDef * servant,
			     CORBA_RepositoryId value,
			     CORBA_Environment * ev);

CORBA_Identifier
impl_CORBA_AliasDef__get_name(impl_POA_CORBA_AliasDef * servant,
			      CORBA_Environment * ev);
void
 impl_CORBA_AliasDef__set_name(impl_POA_CORBA_AliasDef * servant,
			       CORBA_Identifier value,
			       CORBA_Environment * ev);

CORBA_VersionSpec
impl_CORBA_AliasDef__get_version(impl_POA_CORBA_AliasDef * servant,
				 CORBA_Environment * ev);
void
 impl_CORBA_AliasDef__set_version(impl_POA_CORBA_AliasDef * servant,
				  CORBA_VersionSpec value,
				  CORBA_Environment * ev);

CORBA_Container
impl_CORBA_AliasDef__get_defined_in(impl_POA_CORBA_AliasDef * servant,
				    CORBA_Environment * ev);
CORBA_ScopedName
impl_CORBA_AliasDef__get_absolute_name(impl_POA_CORBA_AliasDef * servant,
				       CORBA_Environment * ev);
CORBA_Repository
impl_CORBA_AliasDef__get_containing_repository(impl_POA_CORBA_AliasDef * servant,
					       CORBA_Environment * ev);
CORBA_Contained_Description *
 impl_CORBA_AliasDef_describe(impl_POA_CORBA_AliasDef * servant,
			      CORBA_Environment * ev);
void
 impl_CORBA_AliasDef_move(impl_POA_CORBA_AliasDef * servant,
			  CORBA_Container new_container,
			  CORBA_Identifier new_name,
			  CORBA_VersionSpec new_version,
			  CORBA_Environment * ev);

CORBA_TypeCode
impl_CORBA_AliasDef__get_type(impl_POA_CORBA_AliasDef * servant,
			      CORBA_Environment * ev);

static void impl_CORBA_PrimitiveDef__destroy(impl_POA_CORBA_PrimitiveDef * servant,
					     CORBA_Environment * ev);

CORBA_PrimitiveKind
impl_CORBA_PrimitiveDef__get_kind(impl_POA_CORBA_PrimitiveDef * servant,
				  CORBA_Environment * ev);

CORBA_DefinitionKind
impl_CORBA_PrimitiveDef__get_def_kind(impl_POA_CORBA_PrimitiveDef * servant,
				      CORBA_Environment * ev);
void
 impl_CORBA_PrimitiveDef_destroy(impl_POA_CORBA_PrimitiveDef * servant,
				 CORBA_Environment * ev);

CORBA_TypeCode
impl_CORBA_PrimitiveDef__get_type(impl_POA_CORBA_PrimitiveDef * servant,
				  CORBA_Environment * ev);

static void impl_CORBA_StringDef__destroy(impl_POA_CORBA_StringDef * servant,
					  CORBA_Environment * ev);

CORBA_unsigned_long
impl_CORBA_StringDef__get_bound(impl_POA_CORBA_StringDef * servant,
				CORBA_Environment * ev);
void
 impl_CORBA_StringDef__set_bound(impl_POA_CORBA_StringDef * servant,
				 CORBA_unsigned_long value,
				 CORBA_Environment * ev);

CORBA_DefinitionKind
impl_CORBA_StringDef__get_def_kind(impl_POA_CORBA_StringDef * servant,
				   CORBA_Environment * ev);
void
 impl_CORBA_StringDef_destroy(impl_POA_CORBA_StringDef * servant,
			      CORBA_Environment * ev);

CORBA_TypeCode
impl_CORBA_StringDef__get_type(impl_POA_CORBA_StringDef * servant,
			       CORBA_Environment * ev);

static void impl_CORBA_WstringDef__destroy(impl_POA_CORBA_WstringDef * servant,
					   CORBA_Environment * ev);

CORBA_unsigned_long
impl_CORBA_WstringDef__get_bound(impl_POA_CORBA_WstringDef * servant,
				 CORBA_Environment * ev);
void
 impl_CORBA_WstringDef__set_bound(impl_POA_CORBA_WstringDef * servant,
				  CORBA_unsigned_long value,
				  CORBA_Environment * ev);

CORBA_DefinitionKind
impl_CORBA_WstringDef__get_def_kind(impl_POA_CORBA_WstringDef * servant,
				    CORBA_Environment * ev);
void
 impl_CORBA_WstringDef_destroy(impl_POA_CORBA_WstringDef * servant,
			       CORBA_Environment * ev);

CORBA_TypeCode
impl_CORBA_WstringDef__get_type(impl_POA_CORBA_WstringDef * servant,
				CORBA_Environment * ev);

static void impl_CORBA_FixedDef__destroy(impl_POA_CORBA_FixedDef * servant,
					 CORBA_Environment * ev);

CORBA_unsigned_short
impl_CORBA_FixedDef__get_digits(impl_POA_CORBA_FixedDef * servant,
				CORBA_Environment * ev);
void
 impl_CORBA_FixedDef__set_digits(impl_POA_CORBA_FixedDef * servant,
				 CORBA_unsigned_short value,
				 CORBA_Environment * ev);

CORBA_short
impl_CORBA_FixedDef__get_scale(impl_POA_CORBA_FixedDef * servant,
			       CORBA_Environment * ev);
void
 impl_CORBA_FixedDef__set_scale(impl_POA_CORBA_FixedDef * servant,
				CORBA_short value,
				CORBA_Environment * ev);

CORBA_DefinitionKind
impl_CORBA_FixedDef__get_def_kind(impl_POA_CORBA_FixedDef * servant,
				  CORBA_Environment * ev);
void
 impl_CORBA_FixedDef_destroy(impl_POA_CORBA_FixedDef * servant,
			     CORBA_Environment * ev);

CORBA_TypeCode
impl_CORBA_FixedDef__get_type(impl_POA_CORBA_FixedDef * servant,
			      CORBA_Environment * ev);

static void impl_CORBA_SequenceDef__destroy(impl_POA_CORBA_SequenceDef * servant,
					    CORBA_Environment * ev);

CORBA_unsigned_long
impl_CORBA_SequenceDef__get_bound(impl_POA_CORBA_SequenceDef * servant,
				  CORBA_Environment * ev);
void
 impl_CORBA_SequenceDef__set_bound(impl_POA_CORBA_SequenceDef * servant,
				   CORBA_unsigned_long value,
				   CORBA_Environment * ev);

CORBA_TypeCode
impl_CORBA_SequenceDef__get_element_type(impl_POA_CORBA_SequenceDef * servant,
					 CORBA_Environment * ev);

CORBA_IDLType
impl_CORBA_SequenceDef__get_element_type_def(impl_POA_CORBA_SequenceDef * servant,
					     CORBA_Environment * ev);
void
 impl_CORBA_SequenceDef__set_element_type_def(impl_POA_CORBA_SequenceDef * servant,
					      CORBA_IDLType value,
					      CORBA_Environment * ev);

CORBA_DefinitionKind
impl_CORBA_SequenceDef__get_def_kind(impl_POA_CORBA_SequenceDef * servant,
				     CORBA_Environment * ev);
void
 impl_CORBA_SequenceDef_destroy(impl_POA_CORBA_SequenceDef * servant,
				CORBA_Environment * ev);

CORBA_TypeCode
impl_CORBA_SequenceDef__get_type(impl_POA_CORBA_SequenceDef * servant,
				 CORBA_Environment * ev);

static void impl_CORBA_ArrayDef__destroy(impl_POA_CORBA_ArrayDef * servant,
					 CORBA_Environment * ev);

CORBA_unsigned_long
impl_CORBA_ArrayDef__get_length(impl_POA_CORBA_ArrayDef * servant,
				CORBA_Environment * ev);
void
 impl_CORBA_ArrayDef__set_length(impl_POA_CORBA_ArrayDef * servant,
				 CORBA_unsigned_long value,
				 CORBA_Environment * ev);

CORBA_TypeCode
impl_CORBA_ArrayDef__get_element_type(impl_POA_CORBA_ArrayDef * servant,
				      CORBA_Environment * ev);

CORBA_IDLType
impl_CORBA_ArrayDef__get_element_type_def(impl_POA_CORBA_ArrayDef * servant,
					  CORBA_Environment * ev);
void
 impl_CORBA_ArrayDef__set_element_type_def(impl_POA_CORBA_ArrayDef * servant,
					   CORBA_IDLType value,
					   CORBA_Environment * ev);

CORBA_DefinitionKind
impl_CORBA_ArrayDef__get_def_kind(impl_POA_CORBA_ArrayDef * servant,
				  CORBA_Environment * ev);
void
 impl_CORBA_ArrayDef_destroy(impl_POA_CORBA_ArrayDef * servant,
			     CORBA_Environment * ev);

CORBA_TypeCode
impl_CORBA_ArrayDef__get_type(impl_POA_CORBA_ArrayDef * servant,
			      CORBA_Environment * ev);

static void impl_CORBA_ExceptionDef__destroy(impl_POA_CORBA_ExceptionDef * servant,
					     CORBA_Environment * ev);

CORBA_TypeCode
impl_CORBA_ExceptionDef__get_type(impl_POA_CORBA_ExceptionDef * servant,
				  CORBA_Environment * ev);

CORBA_StructMemberSeq *
 impl_CORBA_ExceptionDef__get_members(impl_POA_CORBA_ExceptionDef * servant,
				      CORBA_Environment * ev);
void
 impl_CORBA_ExceptionDef__set_members(impl_POA_CORBA_ExceptionDef * servant,
				      CORBA_StructMemberSeq * value,
				      CORBA_Environment * ev);

CORBA_DefinitionKind
impl_CORBA_ExceptionDef__get_def_kind(impl_POA_CORBA_ExceptionDef * servant,
				      CORBA_Environment * ev);
void
 impl_CORBA_ExceptionDef_destroy(impl_POA_CORBA_ExceptionDef * servant,
				 CORBA_Environment * ev);

CORBA_RepositoryId
impl_CORBA_ExceptionDef__get_id(impl_POA_CORBA_ExceptionDef * servant,
				CORBA_Environment * ev);
void
 impl_CORBA_ExceptionDef__set_id(impl_POA_CORBA_ExceptionDef * servant,
				 CORBA_RepositoryId value,
				 CORBA_Environment * ev);

CORBA_Identifier
impl_CORBA_ExceptionDef__get_name(impl_POA_CORBA_ExceptionDef * servant,
				  CORBA_Environment * ev);
void
 impl_CORBA_ExceptionDef__set_name(impl_POA_CORBA_ExceptionDef * servant,
				   CORBA_Identifier value,
				   CORBA_Environment * ev);

CORBA_VersionSpec
impl_CORBA_ExceptionDef__get_version(impl_POA_CORBA_ExceptionDef * servant,
				     CORBA_Environment * ev);
void
 impl_CORBA_ExceptionDef__set_version(impl_POA_CORBA_ExceptionDef * servant,
				      CORBA_VersionSpec value,
				      CORBA_Environment * ev);

CORBA_Container
impl_CORBA_ExceptionDef__get_defined_in(impl_POA_CORBA_ExceptionDef * servant,
					CORBA_Environment * ev);
CORBA_ScopedName
impl_CORBA_ExceptionDef__get_absolute_name(impl_POA_CORBA_ExceptionDef * servant,
					   CORBA_Environment * ev);
CORBA_Repository
impl_CORBA_ExceptionDef__get_containing_repository(impl_POA_CORBA_ExceptionDef * servant,
						 CORBA_Environment * ev);
CORBA_Contained_Description *
 impl_CORBA_ExceptionDef_describe(impl_POA_CORBA_ExceptionDef * servant,
				  CORBA_Environment * ev);
void
 impl_CORBA_ExceptionDef_move(impl_POA_CORBA_ExceptionDef * servant,
			      CORBA_Container new_container,
			      CORBA_Identifier new_name,
			      CORBA_VersionSpec new_version,
			      CORBA_Environment * ev);

CORBA_Contained
impl_CORBA_ExceptionDef_lookup(impl_POA_CORBA_ExceptionDef * servant,
			       CORBA_ScopedName search_name,
			       CORBA_Environment * ev);
CORBA_ContainedSeq *
 impl_CORBA_ExceptionDef_contents(impl_POA_CORBA_ExceptionDef * servant,
				  CORBA_DefinitionKind limit_type,
				  CORBA_boolean exclude_inherited,
				  CORBA_Environment * ev);
CORBA_ContainedSeq *
 impl_CORBA_ExceptionDef_lookup_name(impl_POA_CORBA_ExceptionDef * servant,
				     CORBA_Identifier search_name,
				     CORBA_long levels_to_search,
				     CORBA_DefinitionKind limit_type,
				     CORBA_boolean exclude_inherited,
				     CORBA_Environment * ev);
CORBA_Container_DescriptionSeq *
 impl_CORBA_ExceptionDef_describe_contents(impl_POA_CORBA_ExceptionDef * servant,
					   CORBA_DefinitionKind limit_type,
					   CORBA_boolean exclude_inherited,
					   CORBA_long max_returned_objs,
					   CORBA_Environment * ev);

CORBA_ModuleDef
impl_CORBA_ExceptionDef_create_module(impl_POA_CORBA_ExceptionDef * servant,
				      CORBA_RepositoryId id,
				      CORBA_Identifier name,
				      CORBA_VersionSpec version,
				      CORBA_Environment * ev);
CORBA_ConstantDef
impl_CORBA_ExceptionDef_create_constant(impl_POA_CORBA_ExceptionDef * servant,
					CORBA_RepositoryId id,
					CORBA_Identifier name,
					CORBA_VersionSpec version,
					CORBA_IDLType type,
					CORBA_any * value,
					CORBA_Environment * ev);
CORBA_StructDef
impl_CORBA_ExceptionDef_create_struct(impl_POA_CORBA_ExceptionDef * servant,
				      CORBA_RepositoryId id,
				      CORBA_Identifier name,
				      CORBA_VersionSpec version,
				      CORBA_StructMemberSeq * members,
				      CORBA_Environment * ev);
CORBA_UnionDef
impl_CORBA_ExceptionDef_create_union(impl_POA_CORBA_ExceptionDef * servant,
				     CORBA_RepositoryId id,
				     CORBA_Identifier name,
				     CORBA_VersionSpec version,
				     CORBA_IDLType discriminator_type,
				     CORBA_UnionMemberSeq * members,
				     CORBA_Environment * ev);
CORBA_EnumDef
impl_CORBA_ExceptionDef_create_enum(impl_POA_CORBA_ExceptionDef * servant,
				    CORBA_RepositoryId id,
				    CORBA_Identifier name,
				    CORBA_VersionSpec version,
				    CORBA_EnumMemberSeq * members,
				    CORBA_Environment * ev);
CORBA_AliasDef
impl_CORBA_ExceptionDef_create_alias(impl_POA_CORBA_ExceptionDef * servant,
				     CORBA_RepositoryId id,
				     CORBA_Identifier name,
				     CORBA_VersionSpec version,
				     CORBA_IDLType original_type,
				     CORBA_Environment * ev);
CORBA_InterfaceDef
impl_CORBA_ExceptionDef_create_interface(impl_POA_CORBA_ExceptionDef * servant,
					 CORBA_RepositoryId id,
					 CORBA_Identifier name,
					 CORBA_VersionSpec version,
					 CORBA_InterfaceDefSeq * base_interfaces,
					 CORBA_Environment * ev);
CORBA_ExceptionDef
impl_CORBA_ExceptionDef_create_exception(impl_POA_CORBA_ExceptionDef * servant,
					 CORBA_RepositoryId id,
					 CORBA_Identifier name,
					 CORBA_VersionSpec version,
					 CORBA_StructMemberSeq * members,
					 CORBA_Environment * ev);

static void impl_CORBA_AttributeDef__destroy(impl_POA_CORBA_AttributeDef * servant,
					     CORBA_Environment * ev);

CORBA_TypeCode
impl_CORBA_AttributeDef__get_type(impl_POA_CORBA_AttributeDef * servant,
				  CORBA_Environment * ev);

CORBA_IDLType
impl_CORBA_AttributeDef__get_type_def(impl_POA_CORBA_AttributeDef * servant,
				      CORBA_Environment * ev);
void
 impl_CORBA_AttributeDef__set_type_def(impl_POA_CORBA_AttributeDef * servant,
				       CORBA_IDLType value,
				       CORBA_Environment * ev);

CORBA_AttributeMode
impl_CORBA_AttributeDef__get_mode(impl_POA_CORBA_AttributeDef * servant,
				  CORBA_Environment * ev);
void
 impl_CORBA_AttributeDef__set_mode(impl_POA_CORBA_AttributeDef * servant,
				   CORBA_AttributeMode value,
				   CORBA_Environment * ev);

CORBA_DefinitionKind
impl_CORBA_AttributeDef__get_def_kind(impl_POA_CORBA_AttributeDef * servant,
				      CORBA_Environment * ev);
void
 impl_CORBA_AttributeDef_destroy(impl_POA_CORBA_AttributeDef * servant,
				 CORBA_Environment * ev);

CORBA_RepositoryId
impl_CORBA_AttributeDef__get_id(impl_POA_CORBA_AttributeDef * servant,
				CORBA_Environment * ev);
void
 impl_CORBA_AttributeDef__set_id(impl_POA_CORBA_AttributeDef * servant,
				 CORBA_RepositoryId value,
				 CORBA_Environment * ev);

CORBA_Identifier
impl_CORBA_AttributeDef__get_name(impl_POA_CORBA_AttributeDef * servant,
				  CORBA_Environment * ev);
void
 impl_CORBA_AttributeDef__set_name(impl_POA_CORBA_AttributeDef * servant,
				   CORBA_Identifier value,
				   CORBA_Environment * ev);

CORBA_VersionSpec
impl_CORBA_AttributeDef__get_version(impl_POA_CORBA_AttributeDef * servant,
				     CORBA_Environment * ev);
void
 impl_CORBA_AttributeDef__set_version(impl_POA_CORBA_AttributeDef * servant,
				      CORBA_VersionSpec value,
				      CORBA_Environment * ev);

CORBA_Container
impl_CORBA_AttributeDef__get_defined_in(impl_POA_CORBA_AttributeDef * servant,
					CORBA_Environment * ev);
CORBA_ScopedName
impl_CORBA_AttributeDef__get_absolute_name(impl_POA_CORBA_AttributeDef * servant,
					   CORBA_Environment * ev);
CORBA_Repository
impl_CORBA_AttributeDef__get_containing_repository(impl_POA_CORBA_AttributeDef * servant,
						 CORBA_Environment * ev);
CORBA_Contained_Description *
 impl_CORBA_AttributeDef_describe(impl_POA_CORBA_AttributeDef * servant,
				  CORBA_Environment * ev);
void
 impl_CORBA_AttributeDef_move(impl_POA_CORBA_AttributeDef * servant,
			      CORBA_Container new_container,
			      CORBA_Identifier new_name,
			      CORBA_VersionSpec new_version,
			      CORBA_Environment * ev);

static void impl_CORBA_OperationDef__destroy(impl_POA_CORBA_OperationDef * servant,
					     CORBA_Environment * ev);

CORBA_TypeCode
impl_CORBA_OperationDef__get_result(impl_POA_CORBA_OperationDef * servant,
				    CORBA_Environment * ev);

CORBA_IDLType
impl_CORBA_OperationDef__get_result_def(impl_POA_CORBA_OperationDef * servant,
					CORBA_Environment * ev);
void
 impl_CORBA_OperationDef__set_result_def(impl_POA_CORBA_OperationDef * servant,
					 CORBA_IDLType value,
					 CORBA_Environment * ev);

CORBA_ParDescriptionSeq *
 impl_CORBA_OperationDef__get_params(impl_POA_CORBA_OperationDef * servant,
				     CORBA_Environment * ev);
void
 impl_CORBA_OperationDef__set_params(impl_POA_CORBA_OperationDef * servant,
				     CORBA_ParDescriptionSeq * value,
				     CORBA_Environment * ev);

CORBA_OperationMode
impl_CORBA_OperationDef__get_mode(impl_POA_CORBA_OperationDef * servant,
				  CORBA_Environment * ev);
void
 impl_CORBA_OperationDef__set_mode(impl_POA_CORBA_OperationDef * servant,
				   CORBA_OperationMode value,
				   CORBA_Environment * ev);

CORBA_ContextIdSeq *
 impl_CORBA_OperationDef__get_contexts(impl_POA_CORBA_OperationDef * servant,
				       CORBA_Environment * ev);
void
 impl_CORBA_OperationDef__set_contexts(impl_POA_CORBA_OperationDef * servant,
				       CORBA_ContextIdSeq * value,
				       CORBA_Environment * ev);

CORBA_ExceptionDefSeq *
 impl_CORBA_OperationDef__get_exceptions(impl_POA_CORBA_OperationDef * servant,
					 CORBA_Environment * ev);
void
 impl_CORBA_OperationDef__set_exceptions(impl_POA_CORBA_OperationDef * servant,
					 CORBA_ExceptionDefSeq * value,
					 CORBA_Environment * ev);

CORBA_DefinitionKind
impl_CORBA_OperationDef__get_def_kind(impl_POA_CORBA_OperationDef * servant,
				      CORBA_Environment * ev);
void
 impl_CORBA_OperationDef_destroy(impl_POA_CORBA_OperationDef * servant,
				 CORBA_Environment * ev);

CORBA_RepositoryId
impl_CORBA_OperationDef__get_id(impl_POA_CORBA_OperationDef * servant,
				CORBA_Environment * ev);
void
 impl_CORBA_OperationDef__set_id(impl_POA_CORBA_OperationDef * servant,
				 CORBA_RepositoryId value,
				 CORBA_Environment * ev);

CORBA_Identifier
impl_CORBA_OperationDef__get_name(impl_POA_CORBA_OperationDef * servant,
				  CORBA_Environment * ev);
void
 impl_CORBA_OperationDef__set_name(impl_POA_CORBA_OperationDef * servant,
				   CORBA_Identifier value,
				   CORBA_Environment * ev);

CORBA_VersionSpec
impl_CORBA_OperationDef__get_version(impl_POA_CORBA_OperationDef * servant,
				     CORBA_Environment * ev);
void
 impl_CORBA_OperationDef__set_version(impl_POA_CORBA_OperationDef * servant,
				      CORBA_VersionSpec value,
				      CORBA_Environment * ev);

CORBA_Container
impl_CORBA_OperationDef__get_defined_in(impl_POA_CORBA_OperationDef * servant,
					CORBA_Environment * ev);
CORBA_ScopedName
impl_CORBA_OperationDef__get_absolute_name(impl_POA_CORBA_OperationDef * servant,
					   CORBA_Environment * ev);
CORBA_Repository
impl_CORBA_OperationDef__get_containing_repository(impl_POA_CORBA_OperationDef * servant,
						 CORBA_Environment * ev);
CORBA_Contained_Description *
 impl_CORBA_OperationDef_describe(impl_POA_CORBA_OperationDef * servant,
				  CORBA_Environment * ev);
void
 impl_CORBA_OperationDef_move(impl_POA_CORBA_OperationDef * servant,
			      CORBA_Container new_container,
			      CORBA_Identifier new_name,
			      CORBA_VersionSpec new_version,
			      CORBA_Environment * ev);

static void impl_CORBA_InterfaceDef__destroy(impl_POA_CORBA_InterfaceDef * servant,
					     CORBA_Environment * ev);
CORBA_InterfaceDefSeq *
 impl_CORBA_InterfaceDef__get_base_interfaces(impl_POA_CORBA_InterfaceDef * servant,
					      CORBA_Environment * ev);
void
 impl_CORBA_InterfaceDef__set_base_interfaces(impl_POA_CORBA_InterfaceDef * servant,
					      CORBA_InterfaceDefSeq * value,
					      CORBA_Environment * ev);

CORBA_boolean
impl_CORBA_InterfaceDef_is_a(impl_POA_CORBA_InterfaceDef * servant,
			     CORBA_RepositoryId interface_id,
			     CORBA_Environment * ev);

CORBA_InterfaceDef_FullInterfaceDescription *
 impl_CORBA_InterfaceDef_describe_interface(impl_POA_CORBA_InterfaceDef * servant,
					    CORBA_Environment * ev);

CORBA_AttributeDef
impl_CORBA_InterfaceDef_create_attribute(impl_POA_CORBA_InterfaceDef * servant,
					 CORBA_RepositoryId id,
					 CORBA_Identifier name,
					 CORBA_VersionSpec version,
					 CORBA_IDLType type,
					 CORBA_AttributeMode mode,
					 CORBA_Environment * ev);

CORBA_OperationDef
impl_CORBA_InterfaceDef_create_operation(impl_POA_CORBA_InterfaceDef * servant,
					 CORBA_RepositoryId id,
					 CORBA_Identifier name,
					 CORBA_VersionSpec version,
					 CORBA_IDLType result,
					 CORBA_OperationMode mode,
					 CORBA_ParDescriptionSeq * params,
					 CORBA_ExceptionDefSeq * exceptions,
					 CORBA_ContextIdSeq * contexts,
					 CORBA_Environment * ev);

CORBA_DefinitionKind
impl_CORBA_InterfaceDef__get_def_kind(impl_POA_CORBA_InterfaceDef * servant,
				      CORBA_Environment * ev);
void
 impl_CORBA_InterfaceDef_destroy(impl_POA_CORBA_InterfaceDef * servant,
				 CORBA_Environment * ev);

CORBA_Contained
impl_CORBA_InterfaceDef_lookup(impl_POA_CORBA_InterfaceDef * servant,
			       CORBA_ScopedName search_name,
			       CORBA_Environment * ev);
CORBA_ContainedSeq *
 impl_CORBA_InterfaceDef_contents(impl_POA_CORBA_InterfaceDef * servant,
				  CORBA_DefinitionKind limit_type,
				  CORBA_boolean exclude_inherited,
				  CORBA_Environment * ev);
CORBA_ContainedSeq *
 impl_CORBA_InterfaceDef_lookup_name(impl_POA_CORBA_InterfaceDef * servant,
				     CORBA_Identifier search_name,
				     CORBA_long levels_to_search,
				     CORBA_DefinitionKind limit_type,
				     CORBA_boolean exclude_inherited,
				     CORBA_Environment * ev);
CORBA_Container_DescriptionSeq *
 impl_CORBA_InterfaceDef_describe_contents(impl_POA_CORBA_InterfaceDef * servant,
					   CORBA_DefinitionKind limit_type,
					   CORBA_boolean exclude_inherited,
					   CORBA_long max_returned_objs,
					   CORBA_Environment * ev);

CORBA_ModuleDef
impl_CORBA_InterfaceDef_create_module(impl_POA_CORBA_InterfaceDef * servant,
				      CORBA_RepositoryId id,
				      CORBA_Identifier name,
				      CORBA_VersionSpec version,
				      CORBA_Environment * ev);
CORBA_ConstantDef
impl_CORBA_InterfaceDef_create_constant(impl_POA_CORBA_InterfaceDef * servant,
					CORBA_RepositoryId id,
					CORBA_Identifier name,
					CORBA_VersionSpec version,
					CORBA_IDLType type,
					CORBA_any * value,
					CORBA_Environment * ev);
CORBA_StructDef
impl_CORBA_InterfaceDef_create_struct(impl_POA_CORBA_InterfaceDef * servant,
				      CORBA_RepositoryId id,
				      CORBA_Identifier name,
				      CORBA_VersionSpec version,
				      CORBA_StructMemberSeq * members,
				      CORBA_Environment * ev);
CORBA_UnionDef
impl_CORBA_InterfaceDef_create_union(impl_POA_CORBA_InterfaceDef * servant,
				     CORBA_RepositoryId id,
				     CORBA_Identifier name,
				     CORBA_VersionSpec version,
				     CORBA_IDLType discriminator_type,
				     CORBA_UnionMemberSeq * members,
				     CORBA_Environment * ev);
CORBA_EnumDef
impl_CORBA_InterfaceDef_create_enum(impl_POA_CORBA_InterfaceDef * servant,
				    CORBA_RepositoryId id,
				    CORBA_Identifier name,
				    CORBA_VersionSpec version,
				    CORBA_EnumMemberSeq * members,
				    CORBA_Environment * ev);
CORBA_AliasDef
impl_CORBA_InterfaceDef_create_alias(impl_POA_CORBA_InterfaceDef * servant,
				     CORBA_RepositoryId id,
				     CORBA_Identifier name,
				     CORBA_VersionSpec version,
				     CORBA_IDLType original_type,
				     CORBA_Environment * ev);
CORBA_InterfaceDef
impl_CORBA_InterfaceDef_create_interface(impl_POA_CORBA_InterfaceDef *servant,
					 CORBA_RepositoryId id,
					 CORBA_Identifier name,
					 CORBA_VersionSpec version,
					 CORBA_InterfaceDefSeq *base_interfaces,
					 CORBA_Environment *ev);
CORBA_ExceptionDef
impl_CORBA_InterfaceDef_create_exception(impl_POA_CORBA_InterfaceDef * servant,
					 CORBA_RepositoryId id,
					 CORBA_Identifier name,
					 CORBA_VersionSpec version,
					 CORBA_StructMemberSeq * members,
					 CORBA_Environment * ev);
CORBA_RepositoryId
impl_CORBA_InterfaceDef__get_id(impl_POA_CORBA_InterfaceDef * servant,
				CORBA_Environment * ev);
void
 impl_CORBA_InterfaceDef__set_id(impl_POA_CORBA_InterfaceDef * servant,
				 CORBA_RepositoryId value,
				 CORBA_Environment * ev);

CORBA_Identifier
impl_CORBA_InterfaceDef__get_name(impl_POA_CORBA_InterfaceDef * servant,
				  CORBA_Environment * ev);
void
 impl_CORBA_InterfaceDef__set_name(impl_POA_CORBA_InterfaceDef * servant,
				   CORBA_Identifier value,
				   CORBA_Environment * ev);

CORBA_VersionSpec
impl_CORBA_InterfaceDef__get_version(impl_POA_CORBA_InterfaceDef * servant,
				     CORBA_Environment * ev);
void
 impl_CORBA_InterfaceDef__set_version(impl_POA_CORBA_InterfaceDef * servant,
				      CORBA_VersionSpec value,
				      CORBA_Environment * ev);

CORBA_Container
impl_CORBA_InterfaceDef__get_defined_in(impl_POA_CORBA_InterfaceDef * servant,
					CORBA_Environment * ev);
CORBA_ScopedName
impl_CORBA_InterfaceDef__get_absolute_name(impl_POA_CORBA_InterfaceDef * servant,
					   CORBA_Environment * ev);
CORBA_Repository
impl_CORBA_InterfaceDef__get_containing_repository(impl_POA_CORBA_InterfaceDef * servant,
						 CORBA_Environment * ev);
CORBA_Contained_Description *
 impl_CORBA_InterfaceDef_describe(impl_POA_CORBA_InterfaceDef * servant,
				  CORBA_Environment * ev);
void
 impl_CORBA_InterfaceDef_move(impl_POA_CORBA_InterfaceDef * servant,
			      CORBA_Container new_container,
			      CORBA_Identifier new_name,
			      CORBA_VersionSpec new_version,
			      CORBA_Environment * ev);

CORBA_TypeCode
impl_CORBA_InterfaceDef__get_type(impl_POA_CORBA_InterfaceDef * servant,
				  CORBA_Environment * ev);

/*** epv structures ***/

static PortableServer_ServantBase__epv impl_CORBA_IDLType_base_epv =
{
	NULL,			/* _private data */
	(gpointer) & impl_CORBA_IDLType__destroy,	/* finalize
							 * routine */
	NULL			/* default_POA routine */
};
static POA_CORBA_IDLType__epv impl_CORBA_IDLType_epv =
{
	NULL,			/* _private */
	(gpointer) & impl_CORBA_IDLType__get_type

};
static POA_CORBA_IRObject__epv impl_CORBA_IDLType_CORBA_IRObject_epv =
{
	NULL,			/* _private */
	(gpointer) & impl_CORBA_IDLType__get_def_kind,
	(gpointer) & impl_CORBA_IDLType_destroy
};

static PortableServer_ServantBase__epv impl_CORBA_Repository_base_epv =
{
	NULL,			/* _private data */
	(gpointer) & impl_CORBA_Repository__destroy,	/* finalize
							 * routine */
	NULL			/* default_POA routine */
};
static POA_CORBA_Repository__epv impl_CORBA_Repository_epv =
{
	NULL,			/* _private */
	(gpointer) & impl_CORBA_Repository_lookup_id,
	(gpointer) & impl_CORBA_Repository_get_primitive,
	(gpointer) & impl_CORBA_Repository_create_string,
	(gpointer) & impl_CORBA_Repository_create_wstring,
	(gpointer) & impl_CORBA_Repository_create_sequence,
	(gpointer) & impl_CORBA_Repository_create_array,
	(gpointer) & impl_CORBA_Repository_create_fixed
};
static POA_CORBA_IRObject__epv impl_CORBA_Repository_CORBA_IRObject_epv =
{
	NULL,			/* _private */
	(gpointer) & impl_CORBA_Repository__get_def_kind,
	(gpointer) & impl_CORBA_Repository_destroy
};
static POA_CORBA_Container__epv impl_CORBA_Repository_CORBA_Container_epv =
{
	NULL,			/* _private */
	(gpointer) & impl_CORBA_Repository_lookup,
	(gpointer) & impl_CORBA_Repository_contents,
	(gpointer) & impl_CORBA_Repository_lookup_name,
	(gpointer) & impl_CORBA_Repository_describe_contents,
	(gpointer) & impl_CORBA_Repository_create_module,
	(gpointer) & impl_CORBA_Repository_create_constant,
	(gpointer) & impl_CORBA_Repository_create_struct,
	(gpointer) & impl_CORBA_Repository_create_union,
	(gpointer) & impl_CORBA_Repository_create_enum,
	(gpointer) & impl_CORBA_Repository_create_alias,
	(gpointer) & impl_CORBA_Repository_create_interface,
	(gpointer) & impl_CORBA_Repository_create_exception
};
static PortableServer_ServantBase__epv impl_CORBA_ModuleDef_base_epv =
{
	NULL,			/* _private data */
	(gpointer) & impl_CORBA_ModuleDef__destroy,	/* finalize
							 * routine */
	NULL			/* default_POA routine */
};
static POA_CORBA_ModuleDef__epv impl_CORBA_ModuleDef_epv =
{
	NULL			/* _private */
};
static POA_CORBA_IRObject__epv impl_CORBA_ModuleDef_CORBA_IRObject_epv =
{
	NULL,			/* _private */
	(gpointer) & impl_CORBA_ModuleDef__get_def_kind,
	(gpointer) & impl_CORBA_ModuleDef_destroy
};
static POA_CORBA_Container__epv impl_CORBA_ModuleDef_CORBA_Container_epv =
{
	NULL,			/* _private */
	(gpointer) & impl_CORBA_ModuleDef_lookup,
	(gpointer) & impl_CORBA_ModuleDef_contents,
	(gpointer) & impl_CORBA_ModuleDef_lookup_name,
	(gpointer) & impl_CORBA_ModuleDef_describe_contents,
	(gpointer) & impl_CORBA_ModuleDef_create_module,
	(gpointer) & impl_CORBA_ModuleDef_create_constant,
	(gpointer) & impl_CORBA_ModuleDef_create_struct,
	(gpointer) & impl_CORBA_ModuleDef_create_union,
	(gpointer) & impl_CORBA_ModuleDef_create_enum,
	(gpointer) & impl_CORBA_ModuleDef_create_alias,
	(gpointer) & impl_CORBA_ModuleDef_create_interface,
	(gpointer) & impl_CORBA_ModuleDef_create_exception
};
static POA_CORBA_Contained__epv impl_CORBA_ModuleDef_CORBA_Contained_epv =
{
	NULL,			/* _private */
	(gpointer) & impl_CORBA_ModuleDef__get_id,
	(gpointer) & impl_CORBA_ModuleDef__set_id,
	(gpointer) & impl_CORBA_ModuleDef__get_name,
	(gpointer) & impl_CORBA_ModuleDef__set_name,
	(gpointer) & impl_CORBA_ModuleDef__get_version,
	(gpointer) & impl_CORBA_ModuleDef__set_version,
	(gpointer) & impl_CORBA_ModuleDef__get_defined_in,
	(gpointer) & impl_CORBA_ModuleDef__get_absolute_name,
	(gpointer) & impl_CORBA_ModuleDef__get_containing_repository,
	(gpointer) & impl_CORBA_ModuleDef_describe,
	(gpointer) & impl_CORBA_ModuleDef_move
};

static PortableServer_ServantBase__epv impl_CORBA_ConstantDef_base_epv =
{
	NULL,			/* _private data */
	(gpointer) & impl_CORBA_ConstantDef__destroy,	/* finalize
							 * routine */
	NULL			/* default_POA routine */
};
static POA_CORBA_ConstantDef__epv impl_CORBA_ConstantDef_epv =
{
	NULL,			/* _private */
	(gpointer) & impl_CORBA_ConstantDef__get_type,
	(gpointer) & impl_CORBA_ConstantDef__get_type_def,
	(gpointer) & impl_CORBA_ConstantDef__set_type_def,
	(gpointer) & impl_CORBA_ConstantDef__get_value,
	(gpointer) & impl_CORBA_ConstantDef__set_value
};
static POA_CORBA_IRObject__epv impl_CORBA_ConstantDef_CORBA_IRObject_epv =
{
	NULL,			/* _private */
	(gpointer) & impl_CORBA_ConstantDef__get_def_kind,
	(gpointer) & impl_CORBA_ConstantDef_destroy
};
static POA_CORBA_Contained__epv impl_CORBA_ConstantDef_CORBA_Contained_epv =
{
	NULL,			/* _private */
	(gpointer) & impl_CORBA_ConstantDef__get_id,
	(gpointer) & impl_CORBA_ConstantDef__set_id,
	(gpointer) & impl_CORBA_ConstantDef__get_name,
	(gpointer) & impl_CORBA_ConstantDef__set_name,
	(gpointer) & impl_CORBA_ConstantDef__get_version,
	(gpointer) & impl_CORBA_ConstantDef__set_version,
	(gpointer) & impl_CORBA_ConstantDef__get_defined_in,
	(gpointer) & impl_CORBA_ConstantDef__get_absolute_name,
	(gpointer) & impl_CORBA_ConstantDef__get_containing_repository,
	(gpointer) & impl_CORBA_ConstantDef_describe,
	(gpointer) & impl_CORBA_ConstantDef_move
};

static PortableServer_ServantBase__epv impl_CORBA_TypedefDef_base_epv =
{
	NULL,			/* _private data */
	(gpointer) & impl_CORBA_TypedefDef__destroy,	/* finalize
							 * routine */
	NULL			/* default_POA routine */
};
static POA_CORBA_TypedefDef__epv impl_CORBA_TypedefDef_epv =
{
	NULL			/* _private */
};
static POA_CORBA_IRObject__epv impl_CORBA_TypedefDef_CORBA_IRObject_epv =
{
	NULL,			/* _private */
	(gpointer) & impl_CORBA_TypedefDef__get_def_kind,
	(gpointer) & impl_CORBA_TypedefDef_destroy
};
static POA_CORBA_Contained__epv impl_CORBA_TypedefDef_CORBA_Contained_epv =
{
	NULL,			/* _private */
	(gpointer) & impl_CORBA_TypedefDef__get_id,
	(gpointer) & impl_CORBA_TypedefDef__set_id,
	(gpointer) & impl_CORBA_TypedefDef__get_name,
	(gpointer) & impl_CORBA_TypedefDef__set_name,
	(gpointer) & impl_CORBA_TypedefDef__get_version,
	(gpointer) & impl_CORBA_TypedefDef__set_version,
	(gpointer) & impl_CORBA_TypedefDef__get_defined_in,
	(gpointer) & impl_CORBA_TypedefDef__get_absolute_name,
	(gpointer) & impl_CORBA_TypedefDef__get_containing_repository,
	(gpointer) & impl_CORBA_TypedefDef_describe,
	(gpointer) & impl_CORBA_TypedefDef_move
};
static POA_CORBA_IDLType__epv impl_CORBA_TypedefDef_CORBA_IDLType_epv =
{
	NULL,			/* _private */
	(gpointer) & impl_CORBA_TypedefDef__get_type
};

static PortableServer_ServantBase__epv impl_CORBA_StructDef_base_epv =
{
	NULL,			/* _private data */
	(gpointer) & impl_CORBA_StructDef__destroy,	/* finalize
							 * routine */
	NULL			/* default_POA routine */
};
static POA_CORBA_StructDef__epv impl_CORBA_StructDef_epv =
{
	NULL,			/* _private */
	(gpointer) & impl_CORBA_StructDef__get_members,
	(gpointer) & impl_CORBA_StructDef__set_members

};
static POA_CORBA_IRObject__epv impl_CORBA_StructDef_CORBA_IRObject_epv =
{
	NULL,			/* _private */
	(gpointer) & impl_CORBA_StructDef__get_def_kind,
	(gpointer) & impl_CORBA_StructDef_destroy
};
static POA_CORBA_Contained__epv impl_CORBA_StructDef_CORBA_Contained_epv =
{
	NULL,			/* _private */
	(gpointer) & impl_CORBA_StructDef__get_id,
	(gpointer) & impl_CORBA_StructDef__set_id,
	(gpointer) & impl_CORBA_StructDef__get_name,
	(gpointer) & impl_CORBA_StructDef__set_name,
	(gpointer) & impl_CORBA_StructDef__get_version,
	(gpointer) & impl_CORBA_StructDef__set_version,
	(gpointer) & impl_CORBA_StructDef__get_defined_in,
	(gpointer) & impl_CORBA_StructDef__get_absolute_name,
	(gpointer) & impl_CORBA_StructDef__get_containing_repository,
	(gpointer) & impl_CORBA_StructDef_describe,
	(gpointer) & impl_CORBA_StructDef_move
};
static POA_CORBA_IDLType__epv impl_CORBA_StructDef_CORBA_IDLType_epv =
{
	NULL,			/* _private */
	(gpointer) & impl_CORBA_StructDef__get_type
};
static POA_CORBA_TypedefDef__epv impl_CORBA_StructDef_CORBA_TypedefDef_epv =
{
	NULL			/* _private */
};
static POA_CORBA_Container__epv impl_CORBA_StructDef_CORBA_Container_epv =
{
	NULL,			/* _private */
	(gpointer) & impl_CORBA_StructDef_lookup,
	(gpointer) & impl_CORBA_StructDef_contents,
	(gpointer) & impl_CORBA_StructDef_lookup_name,
	(gpointer) & impl_CORBA_StructDef_describe_contents,
	(gpointer) & impl_CORBA_StructDef_create_module,
	(gpointer) & impl_CORBA_StructDef_create_constant,
	(gpointer) & impl_CORBA_StructDef_create_struct,
	(gpointer) & impl_CORBA_StructDef_create_union,
	(gpointer) & impl_CORBA_StructDef_create_enum,
	(gpointer) & impl_CORBA_StructDef_create_alias,
	(gpointer) & impl_CORBA_StructDef_create_interface,
	(gpointer) & impl_CORBA_StructDef_create_exception
};
static PortableServer_ServantBase__epv impl_CORBA_UnionDef_base_epv =
{
	NULL,			/* _private data */
	(gpointer) & impl_CORBA_UnionDef__destroy,	/* finalize
							 * routine */
	NULL			/* default_POA routine */
};
static POA_CORBA_UnionDef__epv impl_CORBA_UnionDef_epv =
{
	NULL,			/* _private */
	(gpointer) & impl_CORBA_UnionDef__get_discriminator_type,

	(gpointer) & impl_CORBA_UnionDef__get_discriminator_type_def,
	(gpointer) & impl_CORBA_UnionDef__set_discriminator_type_def,

	(gpointer) & impl_CORBA_UnionDef__get_members,
	(gpointer) & impl_CORBA_UnionDef__set_members

};
static POA_CORBA_IRObject__epv impl_CORBA_UnionDef_CORBA_IRObject_epv =
{
	NULL,			/* _private */
	(gpointer) & impl_CORBA_UnionDef__get_def_kind,
	(gpointer) & impl_CORBA_UnionDef_destroy
};
static POA_CORBA_Contained__epv impl_CORBA_UnionDef_CORBA_Contained_epv =
{
	NULL,			/* _private */
	(gpointer) & impl_CORBA_UnionDef__get_id,
	(gpointer) & impl_CORBA_UnionDef__set_id,
	(gpointer) & impl_CORBA_UnionDef__get_name,
	(gpointer) & impl_CORBA_UnionDef__set_name,
	(gpointer) & impl_CORBA_UnionDef__get_version,
	(gpointer) & impl_CORBA_UnionDef__set_version,
	(gpointer) & impl_CORBA_UnionDef__get_defined_in,
	(gpointer) & impl_CORBA_UnionDef__get_absolute_name,
	(gpointer) & impl_CORBA_UnionDef__get_containing_repository,
	(gpointer) & impl_CORBA_UnionDef_describe,
	(gpointer) & impl_CORBA_UnionDef_move
};
static POA_CORBA_IDLType__epv impl_CORBA_UnionDef_CORBA_IDLType_epv =
{
	NULL,			/* _private */
	(gpointer) & impl_CORBA_UnionDef__get_type
};
static POA_CORBA_TypedefDef__epv impl_CORBA_UnionDef_CORBA_TypedefDef_epv =
{
	NULL			/* _private */
};
static POA_CORBA_Container__epv impl_CORBA_UnionDef_CORBA_Container_epv =
{
	NULL,			/* _private */
	(gpointer) & impl_CORBA_UnionDef_lookup,
	(gpointer) & impl_CORBA_UnionDef_contents,
	(gpointer) & impl_CORBA_UnionDef_lookup_name,
	(gpointer) & impl_CORBA_UnionDef_describe_contents,
	(gpointer) & impl_CORBA_UnionDef_create_module,
	(gpointer) & impl_CORBA_UnionDef_create_constant,
	(gpointer) & impl_CORBA_UnionDef_create_struct,
	(gpointer) & impl_CORBA_UnionDef_create_union,
	(gpointer) & impl_CORBA_UnionDef_create_enum,
	(gpointer) & impl_CORBA_UnionDef_create_alias,
	(gpointer) & impl_CORBA_UnionDef_create_interface,
	(gpointer) & impl_CORBA_UnionDef_create_exception
};
static PortableServer_ServantBase__epv impl_CORBA_EnumDef_base_epv =
{
	NULL,			/* _private data */
	(gpointer) & impl_CORBA_EnumDef__destroy,	/* finalize
							 * routine */
	NULL			/* default_POA routine */
};
static POA_CORBA_EnumDef__epv impl_CORBA_EnumDef_epv =
{
	NULL,			/* _private */
	(gpointer) & impl_CORBA_EnumDef__get_members,
	(gpointer) & impl_CORBA_EnumDef__set_members
};
static POA_CORBA_IRObject__epv impl_CORBA_EnumDef_CORBA_IRObject_epv =
{
	NULL,			/* _private */
	(gpointer) & impl_CORBA_EnumDef__get_def_kind,
	(gpointer) & impl_CORBA_EnumDef_destroy
};
static POA_CORBA_Contained__epv impl_CORBA_EnumDef_CORBA_Contained_epv =
{
	NULL,			/* _private */
	(gpointer) & impl_CORBA_EnumDef__get_id,
	(gpointer) & impl_CORBA_EnumDef__set_id,
	(gpointer) & impl_CORBA_EnumDef__get_name,
	(gpointer) & impl_CORBA_EnumDef__set_name,
	(gpointer) & impl_CORBA_EnumDef__get_version,
	(gpointer) & impl_CORBA_EnumDef__set_version,
	(gpointer) & impl_CORBA_EnumDef__get_defined_in,
	(gpointer) & impl_CORBA_EnumDef__get_absolute_name,
	(gpointer) & impl_CORBA_EnumDef__get_containing_repository,
	(gpointer) & impl_CORBA_EnumDef_describe,
	(gpointer) & impl_CORBA_EnumDef_move
};
static POA_CORBA_IDLType__epv impl_CORBA_EnumDef_CORBA_IDLType_epv =
{
	NULL,			/* _private */
	(gpointer) & impl_CORBA_EnumDef__get_type
};
static POA_CORBA_TypedefDef__epv impl_CORBA_EnumDef_CORBA_TypedefDef_epv =
{
	NULL			/* _private */
};
static PortableServer_ServantBase__epv impl_CORBA_AliasDef_base_epv =
{
	NULL,			/* _private data */
	(gpointer) & impl_CORBA_AliasDef__destroy,	/* finalize
							 * routine */
	NULL			/* default_POA routine */
};
static POA_CORBA_AliasDef__epv impl_CORBA_AliasDef_epv =
{
	NULL,			/* _private */
	(gpointer) & impl_CORBA_AliasDef__get_original_type_def,
	(gpointer) & impl_CORBA_AliasDef__set_original_type_def
};
static POA_CORBA_IRObject__epv impl_CORBA_AliasDef_CORBA_IRObject_epv =
{
	NULL,			/* _private */
	(gpointer) & impl_CORBA_AliasDef__get_def_kind,
	(gpointer) & impl_CORBA_AliasDef_destroy
};
static POA_CORBA_Contained__epv impl_CORBA_AliasDef_CORBA_Contained_epv =
{
	NULL,			/* _private */
	(gpointer) & impl_CORBA_AliasDef__get_id,
	(gpointer) & impl_CORBA_AliasDef__set_id,
	(gpointer) & impl_CORBA_AliasDef__get_name,
	(gpointer) & impl_CORBA_AliasDef__set_name,
	(gpointer) & impl_CORBA_AliasDef__get_version,
	(gpointer) & impl_CORBA_AliasDef__set_version,
	(gpointer) & impl_CORBA_AliasDef__get_defined_in,
	(gpointer) & impl_CORBA_AliasDef__get_absolute_name,
	(gpointer) & impl_CORBA_AliasDef__get_containing_repository,
	(gpointer) & impl_CORBA_AliasDef_describe,
	(gpointer) & impl_CORBA_AliasDef_move
};
static POA_CORBA_IDLType__epv impl_CORBA_AliasDef_CORBA_IDLType_epv =
{
	NULL,			/* _private */
	(gpointer) & impl_CORBA_AliasDef__get_type
};
static POA_CORBA_TypedefDef__epv impl_CORBA_AliasDef_CORBA_TypedefDef_epv =
{
	NULL			/* _private */
};
static PortableServer_ServantBase__epv impl_CORBA_PrimitiveDef_base_epv =
{
	NULL,			/* _private data */
	(gpointer) & impl_CORBA_PrimitiveDef__destroy,	/* finalize
							 * routine */
	NULL			/* default_POA routine */
};
static POA_CORBA_PrimitiveDef__epv impl_CORBA_PrimitiveDef_epv =
{
	NULL,			/* _private */
	(gpointer) & impl_CORBA_PrimitiveDef__get_kind
};
static POA_CORBA_IRObject__epv impl_CORBA_PrimitiveDef_CORBA_IRObject_epv =
{
	NULL,			/* _private */
	(gpointer) & impl_CORBA_PrimitiveDef__get_def_kind,
	(gpointer) & impl_CORBA_PrimitiveDef_destroy
};
static POA_CORBA_IDLType__epv impl_CORBA_PrimitiveDef_CORBA_IDLType_epv =
{
	NULL,			/* _private */
	(gpointer) & impl_CORBA_PrimitiveDef__get_type
};
static PortableServer_ServantBase__epv impl_CORBA_StringDef_base_epv =
{
	NULL,			/* _private data */
	(gpointer) & impl_CORBA_StringDef__destroy,	/* finalize
							 * routine */
	NULL			/* default_POA routine */
};
static POA_CORBA_StringDef__epv impl_CORBA_StringDef_epv =
{
	NULL,			/* _private */
	(gpointer) & impl_CORBA_StringDef__get_bound,
	(gpointer) & impl_CORBA_StringDef__set_bound
};
static POA_CORBA_IRObject__epv impl_CORBA_StringDef_CORBA_IRObject_epv =
{
	NULL,			/* _private */
	(gpointer) & impl_CORBA_StringDef__get_def_kind,
	(gpointer) & impl_CORBA_StringDef_destroy
};
static POA_CORBA_IDLType__epv impl_CORBA_StringDef_CORBA_IDLType_epv =
{
	NULL,			/* _private */
	(gpointer) & impl_CORBA_StringDef__get_type
};
static PortableServer_ServantBase__epv impl_CORBA_WstringDef_base_epv =
{
	NULL,			/* _private data */
	(gpointer) & impl_CORBA_WstringDef__destroy,	/* finalize
							 * routine */
	NULL			/* default_POA routine */
};
static POA_CORBA_WstringDef__epv impl_CORBA_WstringDef_epv =
{
	NULL,			/* _private */
	(gpointer) & impl_CORBA_WstringDef__get_bound,
	(gpointer) & impl_CORBA_WstringDef__set_bound
};
static POA_CORBA_IRObject__epv impl_CORBA_WstringDef_CORBA_IRObject_epv =
{
	NULL,			/* _private */
	(gpointer) & impl_CORBA_WstringDef__get_def_kind,
	(gpointer) & impl_CORBA_WstringDef_destroy
};
static POA_CORBA_IDLType__epv impl_CORBA_WstringDef_CORBA_IDLType_epv =
{
	NULL,			/* _private */
	(gpointer) & impl_CORBA_WstringDef__get_type
};
static PortableServer_ServantBase__epv impl_CORBA_FixedDef_base_epv =
{
	NULL,			/* _private data */
	(gpointer) & impl_CORBA_FixedDef__destroy,	/* finalize
							 * routine */
	NULL			/* default_POA routine */
};
static POA_CORBA_FixedDef__epv impl_CORBA_FixedDef_epv =
{
	NULL,			/* _private */
	(gpointer) & impl_CORBA_FixedDef__get_digits,
	(gpointer) & impl_CORBA_FixedDef__set_digits,
	(gpointer) & impl_CORBA_FixedDef__get_scale,
	(gpointer) & impl_CORBA_FixedDef__set_scale
};
static POA_CORBA_IRObject__epv impl_CORBA_FixedDef_CORBA_IRObject_epv =
{
	NULL,			/* _private */
	(gpointer) & impl_CORBA_FixedDef__get_def_kind,
	(gpointer) & impl_CORBA_FixedDef_destroy
};
static POA_CORBA_IDLType__epv impl_CORBA_FixedDef_CORBA_IDLType_epv =
{
	NULL,			/* _private */
	(gpointer) & impl_CORBA_FixedDef__get_type
};
static PortableServer_ServantBase__epv impl_CORBA_SequenceDef_base_epv =
{
	NULL,			/* _private data */
	(gpointer) & impl_CORBA_SequenceDef__destroy,	/* finalize
							 * routine */
	NULL			/* default_POA routine */
};
static POA_CORBA_SequenceDef__epv impl_CORBA_SequenceDef_epv =
{
	NULL,			/* _private */
	(gpointer) & impl_CORBA_SequenceDef__get_bound,
	(gpointer) & impl_CORBA_SequenceDef__set_bound,
	(gpointer) & impl_CORBA_SequenceDef__get_element_type,
	(gpointer) & impl_CORBA_SequenceDef__get_element_type_def,
	(gpointer) & impl_CORBA_SequenceDef__set_element_type_def
};
static POA_CORBA_IRObject__epv impl_CORBA_SequenceDef_CORBA_IRObject_epv =
{
	NULL,			/* _private */
	(gpointer) & impl_CORBA_SequenceDef__get_def_kind,
	(gpointer) & impl_CORBA_SequenceDef_destroy
};
static POA_CORBA_IDLType__epv impl_CORBA_SequenceDef_CORBA_IDLType_epv =
{
	NULL,			/* _private */
	(gpointer) & impl_CORBA_SequenceDef__get_type
};
static PortableServer_ServantBase__epv impl_CORBA_ArrayDef_base_epv =
{
	NULL,			/* _private data */
	(gpointer) & impl_CORBA_ArrayDef__destroy,	/* finalize
							 * routine */
	NULL			/* default_POA routine */
};
static POA_CORBA_ArrayDef__epv impl_CORBA_ArrayDef_epv =
{
	NULL,			/* _private */
	(gpointer) & impl_CORBA_ArrayDef__get_length,
	(gpointer) & impl_CORBA_ArrayDef__set_length,
	(gpointer) & impl_CORBA_ArrayDef__get_element_type,
	(gpointer) & impl_CORBA_ArrayDef__get_element_type_def,
	(gpointer) & impl_CORBA_ArrayDef__set_element_type_def
};
static POA_CORBA_IRObject__epv impl_CORBA_ArrayDef_CORBA_IRObject_epv =
{
	NULL,			/* _private */
	(gpointer) & impl_CORBA_ArrayDef__get_def_kind,
	(gpointer) & impl_CORBA_ArrayDef_destroy
};
static POA_CORBA_IDLType__epv impl_CORBA_ArrayDef_CORBA_IDLType_epv =
{
	NULL,			/* _private */
	(gpointer) & impl_CORBA_ArrayDef__get_type
};
static PortableServer_ServantBase__epv impl_CORBA_ExceptionDef_base_epv =
{
	NULL,			/* _private data */
	(gpointer) & impl_CORBA_ExceptionDef__destroy,	/* finalize
							 * routine */
	NULL			/* default_POA routine */
};
static POA_CORBA_ExceptionDef__epv impl_CORBA_ExceptionDef_epv =
{
	NULL,			/* _private */
	(gpointer) & impl_CORBA_ExceptionDef__get_type,
	(gpointer) & impl_CORBA_ExceptionDef__get_members,
	(gpointer) & impl_CORBA_ExceptionDef__set_members
};
static POA_CORBA_IRObject__epv impl_CORBA_ExceptionDef_CORBA_IRObject_epv =
{
	NULL,			/* _private */
	(gpointer) & impl_CORBA_ExceptionDef__get_def_kind,
	(gpointer) & impl_CORBA_ExceptionDef_destroy
};
static POA_CORBA_Contained__epv impl_CORBA_ExceptionDef_CORBA_Contained_epv =
{
	NULL,			/* _private */
	(gpointer) & impl_CORBA_ExceptionDef__get_id,
	(gpointer) & impl_CORBA_ExceptionDef__set_id,
	(gpointer) & impl_CORBA_ExceptionDef__get_name,
	(gpointer) & impl_CORBA_ExceptionDef__set_name,
	(gpointer) & impl_CORBA_ExceptionDef__get_version,
	(gpointer) & impl_CORBA_ExceptionDef__set_version,
	(gpointer) & impl_CORBA_ExceptionDef__get_defined_in,
	(gpointer) & impl_CORBA_ExceptionDef__get_absolute_name,
	(gpointer) & impl_CORBA_ExceptionDef__get_containing_repository,
	(gpointer) & impl_CORBA_ExceptionDef_describe,
	(gpointer) & impl_CORBA_ExceptionDef_move
};
static POA_CORBA_Container__epv impl_CORBA_ExceptionDef_CORBA_Container_epv =
{
	NULL,			/* _private */
	(gpointer) & impl_CORBA_ExceptionDef_lookup,
	(gpointer) & impl_CORBA_ExceptionDef_contents,
	(gpointer) & impl_CORBA_ExceptionDef_lookup_name,
	(gpointer) & impl_CORBA_ExceptionDef_describe_contents,
	(gpointer) & impl_CORBA_ExceptionDef_create_module,
	(gpointer) & impl_CORBA_ExceptionDef_create_constant,
	(gpointer) & impl_CORBA_ExceptionDef_create_struct,
	(gpointer) & impl_CORBA_ExceptionDef_create_union,
	(gpointer) & impl_CORBA_ExceptionDef_create_enum,
	(gpointer) & impl_CORBA_ExceptionDef_create_alias,
	(gpointer) & impl_CORBA_ExceptionDef_create_interface,
	(gpointer) & impl_CORBA_ExceptionDef_create_exception
};

static PortableServer_ServantBase__epv impl_CORBA_AttributeDef_base_epv =
{
	NULL,			/* _private data */
	(gpointer) & impl_CORBA_AttributeDef__destroy,	/* finalize
							 * routine */
	NULL			/* default_POA routine */
};
static POA_CORBA_AttributeDef__epv impl_CORBA_AttributeDef_epv =
{
	NULL,			/* _private */
	(gpointer) & impl_CORBA_AttributeDef__get_type,
	(gpointer) & impl_CORBA_AttributeDef__get_type_def,
	(gpointer) & impl_CORBA_AttributeDef__set_type_def,
	(gpointer) & impl_CORBA_AttributeDef__get_mode,
	(gpointer) & impl_CORBA_AttributeDef__set_mode
};
static POA_CORBA_IRObject__epv impl_CORBA_AttributeDef_CORBA_IRObject_epv =
{
	NULL,			/* _private */
	(gpointer) & impl_CORBA_AttributeDef__get_def_kind,
	(gpointer) & impl_CORBA_AttributeDef_destroy
};
static POA_CORBA_Contained__epv impl_CORBA_AttributeDef_CORBA_Contained_epv =
{
	NULL,			/* _private */
	(gpointer) & impl_CORBA_AttributeDef__get_id,
	(gpointer) & impl_CORBA_AttributeDef__set_id,
	(gpointer) & impl_CORBA_AttributeDef__get_name,
	(gpointer) & impl_CORBA_AttributeDef__set_name,
	(gpointer) & impl_CORBA_AttributeDef__get_version,
	(gpointer) & impl_CORBA_AttributeDef__set_version,
	(gpointer) & impl_CORBA_AttributeDef__get_defined_in,
	(gpointer) & impl_CORBA_AttributeDef__get_absolute_name,
	(gpointer) & impl_CORBA_AttributeDef__get_containing_repository,
	(gpointer) & impl_CORBA_AttributeDef_describe,
	(gpointer) & impl_CORBA_AttributeDef_move
};

static PortableServer_ServantBase__epv impl_CORBA_OperationDef_base_epv =
{
	NULL,			/* _private data */
	(gpointer) & impl_CORBA_OperationDef__destroy,	/* finalize
							 * routine */
	NULL			/* default_POA routine */
};
static POA_CORBA_OperationDef__epv impl_CORBA_OperationDef_epv =
{
	NULL,			/* _private */
	(gpointer) & impl_CORBA_OperationDef__get_result,
	(gpointer) & impl_CORBA_OperationDef__get_result_def,
	(gpointer) & impl_CORBA_OperationDef__set_result_def,
	(gpointer) & impl_CORBA_OperationDef__get_params,
	(gpointer) & impl_CORBA_OperationDef__set_params,
	(gpointer) & impl_CORBA_OperationDef__get_mode,
	(gpointer) & impl_CORBA_OperationDef__set_mode,
	(gpointer) & impl_CORBA_OperationDef__get_contexts,
	(gpointer) & impl_CORBA_OperationDef__set_contexts,
	(gpointer) & impl_CORBA_OperationDef__get_exceptions,
	(gpointer) & impl_CORBA_OperationDef__set_exceptions
};
static POA_CORBA_IRObject__epv impl_CORBA_OperationDef_CORBA_IRObject_epv =
{
	NULL,			/* _private */
	(gpointer) & impl_CORBA_OperationDef__get_def_kind,
	(gpointer) & impl_CORBA_OperationDef_destroy
};
static POA_CORBA_Contained__epv impl_CORBA_OperationDef_CORBA_Contained_epv =
{
	NULL,			/* _private */
	(gpointer) & impl_CORBA_OperationDef__get_id,
	(gpointer) & impl_CORBA_OperationDef__set_id,
	(gpointer) & impl_CORBA_OperationDef__get_name,
	(gpointer) & impl_CORBA_OperationDef__set_name,
	(gpointer) & impl_CORBA_OperationDef__get_version,
	(gpointer) & impl_CORBA_OperationDef__set_version,
	(gpointer) & impl_CORBA_OperationDef__get_defined_in,
	(gpointer) & impl_CORBA_OperationDef__get_absolute_name,
	(gpointer) & impl_CORBA_OperationDef__get_containing_repository,
	(gpointer) & impl_CORBA_OperationDef_describe,
	(gpointer) & impl_CORBA_OperationDef_move
};

static PortableServer_ServantBase__epv impl_CORBA_InterfaceDef_base_epv =
{
	NULL,			/* _private data */
	(gpointer) & impl_CORBA_InterfaceDef__destroy,	/* finalize
							 * routine */
	NULL			/* default_POA routine */
};
static POA_CORBA_InterfaceDef__epv impl_CORBA_InterfaceDef_epv =
{
	NULL,			/* _private */
	(gpointer) & impl_CORBA_InterfaceDef__get_base_interfaces,
	(gpointer) & impl_CORBA_InterfaceDef__set_base_interfaces,
	(gpointer) & impl_CORBA_InterfaceDef_is_a,
	(gpointer) & impl_CORBA_InterfaceDef_describe_interface,
	(gpointer) & impl_CORBA_InterfaceDef_create_attribute,
	(gpointer) & impl_CORBA_InterfaceDef_create_operation
};
static POA_CORBA_IRObject__epv impl_CORBA_InterfaceDef_CORBA_IRObject_epv =
{
	NULL,			/* _private */
	(gpointer) & impl_CORBA_InterfaceDef__get_def_kind,
	(gpointer) & impl_CORBA_InterfaceDef_destroy
};
static POA_CORBA_Container__epv impl_CORBA_InterfaceDef_CORBA_Container_epv =
{
	NULL,			/* _private */
	(gpointer) & impl_CORBA_InterfaceDef_lookup,
	(gpointer) & impl_CORBA_InterfaceDef_contents,
	(gpointer) & impl_CORBA_InterfaceDef_lookup_name,
	(gpointer) & impl_CORBA_InterfaceDef_describe_contents,
	(gpointer) & impl_CORBA_InterfaceDef_create_module,
	(gpointer) & impl_CORBA_InterfaceDef_create_constant,
	(gpointer) & impl_CORBA_InterfaceDef_create_struct,
	(gpointer) & impl_CORBA_InterfaceDef_create_union,
	(gpointer) & impl_CORBA_InterfaceDef_create_enum,
	(gpointer) & impl_CORBA_InterfaceDef_create_alias,
	(gpointer) & impl_CORBA_InterfaceDef_create_interface,
	(gpointer) & impl_CORBA_InterfaceDef_create_exception
};
static POA_CORBA_Contained__epv impl_CORBA_InterfaceDef_CORBA_Contained_epv =
{
	NULL,			/* _private */
	(gpointer) & impl_CORBA_InterfaceDef__get_id,
	(gpointer) & impl_CORBA_InterfaceDef__set_id,
	(gpointer) & impl_CORBA_InterfaceDef__get_name,
	(gpointer) & impl_CORBA_InterfaceDef__set_name,
	(gpointer) & impl_CORBA_InterfaceDef__get_version,
	(gpointer) & impl_CORBA_InterfaceDef__set_version,
	(gpointer) & impl_CORBA_InterfaceDef__get_defined_in,
	(gpointer) & impl_CORBA_InterfaceDef__get_absolute_name,
	(gpointer) & impl_CORBA_InterfaceDef__get_containing_repository,
	(gpointer) & impl_CORBA_InterfaceDef_describe,
	(gpointer) & impl_CORBA_InterfaceDef_move
};
static POA_CORBA_IDLType__epv impl_CORBA_InterfaceDef_CORBA_IDLType_epv =
{
	NULL,			/* _private */
	(gpointer) & impl_CORBA_InterfaceDef__get_type
};

/*** vepv structures ***/

static POA_CORBA_IDLType__vepv impl_CORBA_IDLType_vepv =
{
	&impl_CORBA_IDLType_base_epv,
	&impl_CORBA_IDLType_CORBA_IRObject_epv,
	&impl_CORBA_IDLType_epv
};

static POA_CORBA_Repository__vepv impl_CORBA_Repository_vepv =
{
	&impl_CORBA_Repository_base_epv,
	&impl_CORBA_Repository_CORBA_IRObject_epv,
	&impl_CORBA_Repository_CORBA_Container_epv,
	&impl_CORBA_Repository_epv
};
static POA_CORBA_ModuleDef__vepv impl_CORBA_ModuleDef_vepv =
{
	&impl_CORBA_ModuleDef_base_epv,
	&impl_CORBA_ModuleDef_CORBA_IRObject_epv,
	&impl_CORBA_ModuleDef_CORBA_Container_epv,
	&impl_CORBA_ModuleDef_CORBA_Contained_epv,
	&impl_CORBA_ModuleDef_epv
};

static POA_CORBA_ConstantDef__vepv impl_CORBA_ConstantDef_vepv =
{
	&impl_CORBA_ConstantDef_base_epv,
	&impl_CORBA_ConstantDef_CORBA_IRObject_epv,
	&impl_CORBA_ConstantDef_CORBA_Contained_epv,
	&impl_CORBA_ConstantDef_epv
};

static POA_CORBA_TypedefDef__vepv impl_CORBA_TypedefDef_vepv =
{
	&impl_CORBA_TypedefDef_base_epv,
	&impl_CORBA_TypedefDef_CORBA_IRObject_epv,
	&impl_CORBA_TypedefDef_CORBA_Contained_epv,
	&impl_CORBA_TypedefDef_CORBA_IDLType_epv,
	&impl_CORBA_TypedefDef_epv
};

static POA_CORBA_StructDef__vepv impl_CORBA_StructDef_vepv =
{
	&impl_CORBA_StructDef_base_epv,
	&impl_CORBA_StructDef_CORBA_IRObject_epv,
	&impl_CORBA_StructDef_CORBA_Contained_epv,
	&impl_CORBA_StructDef_CORBA_IDLType_epv,
	&impl_CORBA_StructDef_CORBA_TypedefDef_epv,
	&impl_CORBA_StructDef_CORBA_Container_epv,
	&impl_CORBA_StructDef_epv
};
static POA_CORBA_UnionDef__vepv impl_CORBA_UnionDef_vepv =
{
	&impl_CORBA_UnionDef_base_epv,
	&impl_CORBA_UnionDef_CORBA_IRObject_epv,
	&impl_CORBA_UnionDef_CORBA_Contained_epv,
	&impl_CORBA_UnionDef_CORBA_IDLType_epv,
	&impl_CORBA_UnionDef_CORBA_TypedefDef_epv,
	&impl_CORBA_UnionDef_CORBA_Container_epv,
	&impl_CORBA_UnionDef_epv
};
static POA_CORBA_EnumDef__vepv impl_CORBA_EnumDef_vepv =
{
	&impl_CORBA_EnumDef_base_epv,
	&impl_CORBA_EnumDef_CORBA_IRObject_epv,
	&impl_CORBA_EnumDef_CORBA_Contained_epv,
	&impl_CORBA_EnumDef_CORBA_IDLType_epv,
	&impl_CORBA_EnumDef_CORBA_TypedefDef_epv,
	&impl_CORBA_EnumDef_epv
};
static POA_CORBA_AliasDef__vepv impl_CORBA_AliasDef_vepv =
{
	&impl_CORBA_AliasDef_base_epv,
	&impl_CORBA_AliasDef_CORBA_IRObject_epv,
	&impl_CORBA_AliasDef_CORBA_Contained_epv,
	&impl_CORBA_AliasDef_CORBA_IDLType_epv,
	&impl_CORBA_AliasDef_CORBA_TypedefDef_epv,
	&impl_CORBA_AliasDef_epv
};
static POA_CORBA_PrimitiveDef__vepv impl_CORBA_PrimitiveDef_vepv =
{
	&impl_CORBA_PrimitiveDef_base_epv,
	&impl_CORBA_PrimitiveDef_CORBA_IRObject_epv,
	&impl_CORBA_PrimitiveDef_CORBA_IDLType_epv,
	&impl_CORBA_PrimitiveDef_epv
};
static POA_CORBA_StringDef__vepv impl_CORBA_StringDef_vepv =
{
	&impl_CORBA_StringDef_base_epv,
	&impl_CORBA_StringDef_CORBA_IRObject_epv,
	&impl_CORBA_StringDef_CORBA_IDLType_epv,
	&impl_CORBA_StringDef_epv
};
static POA_CORBA_WstringDef__vepv impl_CORBA_WstringDef_vepv =
{
	&impl_CORBA_WstringDef_base_epv,
	&impl_CORBA_WstringDef_CORBA_IRObject_epv,
	&impl_CORBA_WstringDef_CORBA_IDLType_epv,
	&impl_CORBA_WstringDef_epv
};
static POA_CORBA_FixedDef__vepv impl_CORBA_FixedDef_vepv =
{
	&impl_CORBA_FixedDef_base_epv,
	&impl_CORBA_FixedDef_CORBA_IRObject_epv,
	&impl_CORBA_FixedDef_CORBA_IDLType_epv,
	&impl_CORBA_FixedDef_epv
};
static POA_CORBA_SequenceDef__vepv impl_CORBA_SequenceDef_vepv =
{
	&impl_CORBA_SequenceDef_base_epv,
	&impl_CORBA_SequenceDef_CORBA_IRObject_epv,
	&impl_CORBA_SequenceDef_CORBA_IDLType_epv,
	&impl_CORBA_SequenceDef_epv
};
static POA_CORBA_ArrayDef__vepv impl_CORBA_ArrayDef_vepv =
{
	&impl_CORBA_ArrayDef_base_epv,
	&impl_CORBA_ArrayDef_CORBA_IRObject_epv,
	&impl_CORBA_ArrayDef_CORBA_IDLType_epv,
	&impl_CORBA_ArrayDef_epv
};
static POA_CORBA_ExceptionDef__vepv impl_CORBA_ExceptionDef_vepv =
{
	&impl_CORBA_ExceptionDef_base_epv,
	&impl_CORBA_ExceptionDef_CORBA_IRObject_epv,
	&impl_CORBA_ExceptionDef_CORBA_Contained_epv,
	&impl_CORBA_ExceptionDef_CORBA_Container_epv,
	&impl_CORBA_ExceptionDef_epv
};

static POA_CORBA_AttributeDef__vepv impl_CORBA_AttributeDef_vepv =
{
	&impl_CORBA_AttributeDef_base_epv,
	&impl_CORBA_AttributeDef_CORBA_IRObject_epv,
	&impl_CORBA_AttributeDef_CORBA_Contained_epv,
	&impl_CORBA_AttributeDef_epv
};

static POA_CORBA_OperationDef__vepv impl_CORBA_OperationDef_vepv =
{
	&impl_CORBA_OperationDef_base_epv,
	&impl_CORBA_OperationDef_CORBA_IRObject_epv,
	&impl_CORBA_OperationDef_CORBA_Contained_epv,
	&impl_CORBA_OperationDef_epv
};

static POA_CORBA_InterfaceDef__vepv impl_CORBA_InterfaceDef_vepv =
{
	&impl_CORBA_InterfaceDef_base_epv,
	&impl_CORBA_InterfaceDef_CORBA_IRObject_epv,
	&impl_CORBA_InterfaceDef_CORBA_Container_epv,
	&impl_CORBA_InterfaceDef_CORBA_Contained_epv,
	&impl_CORBA_InterfaceDef_CORBA_IDLType_epv,
	&impl_CORBA_InterfaceDef_epv
};

/*** Stub implementations ***/

static impl_POA_CORBA_ConstantDef *impl_CORBA_ConstantDef__create(
	PortableServer_POA poa,
	CORBA_RepositoryId id,
	CORBA_Identifier name,
	CORBA_VersionSpec version,
	CORBA_IDLType type_def,
	CORBA_any *value,
	CORBA_Container defined_in,
	CORBA_Environment * ev);
static impl_POA_CORBA_StructDef *impl_CORBA_StructDef__create(
	PortableServer_POA poa,
	CORBA_RepositoryId id,
	CORBA_Identifier name,
	CORBA_VersionSpec version,
	CORBA_StructMemberSeq * members,
	CORBA_Container defined_in,
	CORBA_Environment * ev);
static impl_POA_CORBA_UnionDef *impl_CORBA_UnionDef__create(
	PortableServer_POA poa,
	CORBA_RepositoryId id,
	CORBA_Identifier name,
	CORBA_VersionSpec version,
	CORBA_IDLType discriminator_type,
	CORBA_UnionMemberSeq * members,
	CORBA_Container defined_in,
	CORBA_Environment * ev);
static impl_POA_CORBA_EnumDef *impl_CORBA_EnumDef__create(
	PortableServer_POA poa,
	CORBA_RepositoryId id,
	CORBA_Identifier name,
	CORBA_VersionSpec version,
	CORBA_EnumMemberSeq * members,
	CORBA_Container defined_in,
	CORBA_Environment * ev);
static impl_POA_CORBA_ModuleDef *impl_CORBA_ModuleDef__create(
	PortableServer_POA poa,
	CORBA_RepositoryId id,
	CORBA_Identifier name,
	CORBA_VersionSpec version,
	CORBA_Container defined_in,
	CORBA_Environment * ev);
static impl_POA_CORBA_InterfaceDef *impl_CORBA_InterfaceDef__create(
	PortableServer_POA poa,
	CORBA_RepositoryId id,
	CORBA_Identifier name,
	CORBA_VersionSpec version,
	CORBA_InterfaceDefSeq *base_interfaces,
	CORBA_Container defined_in,
	CORBA_Environment * ev);
static impl_POA_CORBA_ExceptionDef *impl_CORBA_ExceptionDef__create(
	PortableServer_POA poa,
	CORBA_RepositoryId id,
	CORBA_Identifier name,
	CORBA_VersionSpec version,
	CORBA_StructMemberSeq * members,
	CORBA_Container defined_in,
	CORBA_Environment * ev);
static impl_POA_CORBA_AttributeDef *impl_CORBA_AttributeDef__create(
	PortableServer_POA poa,
	CORBA_RepositoryId id,
	CORBA_Identifier name,
	CORBA_VersionSpec version,
	CORBA_IDLType type,
	CORBA_AttributeMode mode,
	CORBA_Container defined_in,
	CORBA_Environment * ev);
static impl_POA_CORBA_AliasDef *impl_CORBA_AliasDef__create(
	PortableServer_POA poa, 
	CORBA_RepositoryId id,
	CORBA_Identifier name,
	CORBA_VersionSpec version,
	CORBA_IDLType original_type,
	CORBA_Container defined_in,
	CORBA_Environment * ev);
static impl_POA_CORBA_StringDef *impl_CORBA_StringDef__create(
	PortableServer_POA poa,
	CORBA_unsigned_long bound,
	CORBA_Environment * ev);
static impl_POA_CORBA_WstringDef *impl_CORBA_WstringDef__create(
	PortableServer_POA poa,
	CORBA_unsigned_long bound,
	CORBA_Environment * ev);
static impl_POA_CORBA_SequenceDef *impl_CORBA_SequenceDef__create(
	PortableServer_POA poa,
	CORBA_unsigned_long bound,
	CORBA_IDLType element_type,
	CORBA_Environment * ev);
static impl_POA_CORBA_ArrayDef *impl_CORBA_ArrayDef__create(
	PortableServer_POA poa,
	CORBA_unsigned_long length,
	CORBA_IDLType element_type,
	CORBA_Environment * ev);
static impl_POA_CORBA_FixedDef *impl_CORBA_FixedDef__create(
	PortableServer_POA poa,
	CORBA_unsigned_short digits,
	CORBA_short scale,
	CORBA_Environment * ev);
static impl_POA_CORBA_OperationDef *impl_CORBA_OperationDef__create(
	PortableServer_POA poa,
	CORBA_RepositoryId id,
	CORBA_Identifier name,
	CORBA_VersionSpec version,
	CORBA_IDLType result,
	CORBA_OperationMode mode,
	CORBA_ParDescriptionSeq *params,
	CORBA_ExceptionDefSeq *exceptions,
	CORBA_ContextIdSeq *contexts,
	CORBA_Container defined_in,
	CORBA_Environment *ev);

static CORBA_ORB orb;
static impl_POA_CORBA_Repository *repo;
static CORBA_PrimitiveDef prim_null=CORBA_OBJECT_NIL;
static CORBA_PrimitiveDef prim_void=CORBA_OBJECT_NIL;
static CORBA_PrimitiveDef prim_short=CORBA_OBJECT_NIL;
static CORBA_PrimitiveDef prim_long=CORBA_OBJECT_NIL;
static CORBA_PrimitiveDef prim_ushort=CORBA_OBJECT_NIL;
static CORBA_PrimitiveDef prim_ulong=CORBA_OBJECT_NIL;
static CORBA_PrimitiveDef prim_float=CORBA_OBJECT_NIL;
static CORBA_PrimitiveDef prim_double=CORBA_OBJECT_NIL;
static CORBA_PrimitiveDef prim_boolean=CORBA_OBJECT_NIL;
static CORBA_PrimitiveDef prim_char=CORBA_OBJECT_NIL;
static CORBA_PrimitiveDef prim_octet=CORBA_OBJECT_NIL;
static CORBA_PrimitiveDef prim_any=CORBA_OBJECT_NIL;
static CORBA_PrimitiveDef prim_TypeCode=CORBA_OBJECT_NIL;
static CORBA_PrimitiveDef prim_Principal=CORBA_OBJECT_NIL;
static CORBA_PrimitiveDef prim_string=CORBA_OBJECT_NIL;
static CORBA_PrimitiveDef prim_objref=CORBA_OBJECT_NIL;
static CORBA_PrimitiveDef prim_longlong=CORBA_OBJECT_NIL;
static CORBA_PrimitiveDef prim_ulonglong=CORBA_OBJECT_NIL;
static CORBA_PrimitiveDef prim_longdouble=CORBA_OBJECT_NIL;
static CORBA_PrimitiveDef prim_wchar=CORBA_OBJECT_NIL;
static CORBA_PrimitiveDef prim_wstring=CORBA_OBJECT_NIL;

static CORBA_boolean repo_check_id(CORBA_RepositoryId id)
{
	CORBA_Contained retval;
	
	retval=g_hash_table_lookup(repo->id_hash, id);
	
	if(retval) {
		return(CORBA_TRUE);
	} else {
		return(CORBA_FALSE);
	}
}

static void repo_remove_id(CORBA_RepositoryId id)
{
	g_hash_table_remove(repo->id_hash, id);
}

static CORBA_boolean repo_check_name_and_version(GSList *list,
						 CORBA_Identifier name,
						 CORBA_VersionSpec version)
{
	for(;list;list=g_slist_next(list)) {
		IfaceRepoContents *contents=(IfaceRepoContents *)list->data;

		if(name!=NULL &&
		   version!=NULL &&
		   !strcmp(name, contents->attr_name) &&
		   !strcmp(version, contents->attr_version)) {
			return(CORBA_TRUE);
		}
	}

	return(CORBA_FALSE);
}

static CORBA_boolean container_add_to_list(ContainedIterData *iter_data,
					   IfaceRepoContents *contents)
{
	/* First, the types have to match (dk_all matches anything) */
	if(iter_data->limit_type!=CORBA_dk_all &&
	   iter_data->limit_type!=contents->attr_def_kind) {
		return(CORBA_FALSE);
	}
		
	/* If we are searching for a particular name, weed out
           anonymous items */
	if(iter_data->search_name!=NULL && contents->attr_name==NULL) {
		return(CORBA_FALSE);
	}
	
	/* ... and items that don't have the name we are looking for */
	if(iter_data->search_name!=NULL &&
	   strcmp(iter_data->search_name, contents->attr_name)) {
		return(CORBA_FALSE);
	}
	
	/* We have a match */
	return(CORBA_TRUE);
}

static void container_contents_internal(gpointer item, gpointer data)
{
	IfaceRepoContents *contents=(IfaceRepoContents *)item;
	ContainedIterData *iter_data=(ContainedIterData *)data;
	CORBA_long level;
	
	if(container_add_to_list(iter_data, contents)) {
		iter_data->seq=g_slist_append(iter_data->seq, contents->obj);
	}

	level=iter_data->levels_to_search;
	if(iter_data->search_name!=NULL && iter_data->levels_to_search==1) {
		return;
	}
	
	if(iter_data->search_name!=NULL && iter_data->levels_to_search!=-1) {
		iter_data->levels_to_search--;
	}
	
	switch(contents->attr_def_kind) {
	case CORBA_dk_Module:
		g_slist_foreach(((impl_POA_CORBA_ModuleDef *)contents)->contents, container_contents_internal, data);
		break;
	case CORBA_dk_Struct:
		g_slist_foreach(((impl_POA_CORBA_StructDef *)contents)->contents, container_contents_internal, data);
		break;
	case CORBA_dk_Union:
		g_slist_foreach(((impl_POA_CORBA_UnionDef *)contents)->contents, container_contents_internal, data);
		break;
	case CORBA_dk_Exception:
		g_slist_foreach(((impl_POA_CORBA_ExceptionDef *)contents)->contents, container_contents_internal, data);
		break;
	case CORBA_dk_Interface:
		g_slist_foreach(((impl_POA_CORBA_InterfaceDef *)contents)->contents, container_contents_internal, data);
		if(iter_data->exclude_inherited==CORBA_FALSE) {
			g_slist_foreach(((impl_POA_CORBA_InterfaceDef *)contents)->inherited, container_contents_internal, data);
		}
		break;
	default:
	  break;
	}

	iter_data->levels_to_search=level;
}

static CORBA_Contained container_search(GSList *list, char *search_name)
{
	CORBA_Contained retval;
	GSList *newlist=NULL;
	gchar **items;
	
	/* split off the next part */
	items=g_strsplit(search_name, "::", 1);
	if(items==NULL) {
		ORBit_Trace(TraceMod_IR, TraceLevel_Debug,
			    "container_search: no tokens in [%s]\n",
			    search_name);
		return(CORBA_OBJECT_NIL);
	} else {
		ORBit_Trace(TraceMod_IR, TraceLevel_Debug,
			    "container_search: looking for [%s]\n", items[0]);
	}
	
	for(;list;list=g_slist_next(list)) {
		IfaceRepoContents *cont=(IfaceRepoContents *)list->data;
		
		if(cont->attr_name!=NULL &&
		   !strcmp(cont->attr_name, items[0])) {
			/* matched this name */
			if(items[1]==NULL) {
				/* last item, so complete match */
				g_strfreev(items);
				return(cont->obj);
			}

			/* more to come, so make sure we have a
			   Container */
			switch(cont->attr_def_kind) {
			case CORBA_dk_Repository:
				newlist=((impl_POA_CORBA_Repository *)cont)->contents;
				break;
			case CORBA_dk_Module:
				newlist=((impl_POA_CORBA_ModuleDef *)cont)->contents;
				break;
			case CORBA_dk_Struct:
				newlist=((impl_POA_CORBA_StructDef *)cont)->contents;
				break;
			case CORBA_dk_Union:
				newlist=((impl_POA_CORBA_UnionDef *)cont)->contents;
				break;
			case CORBA_dk_Exception:
				newlist=((impl_POA_CORBA_ExceptionDef *)cont)->contents;
				break;
			case CORBA_dk_Interface:
				newlist=((impl_POA_CORBA_ExceptionDef *)cont)->contents;
				break;
			default:
				/* Not a container, therefore not
                                   found */
				ORBit_Trace(TraceMod_IR, TraceLevel_Debug,
					    "container_search: not a container\n");
				g_strfreev(items);
				return(CORBA_OBJECT_NIL);
			}

			if(newlist==NULL) {
				ORBit_Trace(TraceMod_IR, TraceLevel_Debug,
					    "container_search: missed container\n");
				
				g_strfreev(items);
				return(CORBA_OBJECT_NIL);
			}
			
			ORBit_Trace(TraceMod_IR, TraceLevel_Debug,
				    "container_search: recursing on [%s]\n",
				    items[1]);
			
			retval=container_search(newlist, items[1]);
			g_strfreev(items);
			return(retval);
		}
	}
	
	g_strfreev(items);
	return(CORBA_OBJECT_NIL);
}

static CORBA_Contained container_lookup(GSList *search_list,
					CORBA_ScopedName search_name,
					char *container_type,
					CORBA_Environment *ev)
{
	CORBA_Contained retval;
	GSList *list;
	char *name;

	if(search_name==NULL) {
		ORBit_Trace(TraceMod_IR, TraceLevel_Info,
			    "%s_lookup: NULL search_name", container_type);
		return(CORBA_OBJECT_NIL);
	}
	
	/* search_name begins with :: for an absolute name */
	if(!strncmp(search_name, "::", 2)) {
		/* start at the repo */
		list=repo->contents;
		name=search_name+2;
	} else {
		/* start at this container */
		list=search_list;
		name=search_name;
	}
	
	retval=container_search(list, name);
	if(retval==CORBA_OBJECT_NIL) {
		ORBit_Trace(TraceMod_IR, TraceLevel_Debug,
			    "%s_lookup: %s not found\n", container_type,
			    search_name);
		return(CORBA_OBJECT_NIL);
	} else {
		ORBit_Trace(TraceMod_IR, TraceLevel_Debug,
			    "%s_lookup: %s found\n", container_type,
			    search_name);
		return(CORBA_Object_duplicate(retval, ev));
	}
}

static CORBA_ContainedSeq *container_contents(GSList *contents,
					      CORBA_DefinitionKind limit_type,
					      CORBA_boolean exclude_inherited,
					      char *container_type,
					      CORBA_Environment *ev)
{
	CORBA_ContainedSeq *retval;
	ContainedIterData data;
	CORBA_unsigned_long len;
	GSList *list;
	int i;
	
	data.search_name=NULL;
	data.limit_type=limit_type;
	data.exclude_inherited=exclude_inherited;
	data.seq=NULL;
	
	g_slist_foreach(contents, container_contents_internal, &data);
	
	len=g_slist_length(data.seq);
	
	retval=CORBA_ContainedSeq__alloc();
	if(retval==NULL) {
		return(CORBA_OBJECT_NIL);
	}
	
	retval->_length=len;
	retval->_maximum=len;
	retval->_buffer=CORBA_sequence_CORBA_Contained_allocbuf(len);

	if(retval->_buffer==NULL) {
		CORBA_free(retval);
		return(CORBA_OBJECT_NIL);
	}
	
	for(i=0,list=data.seq;list;list=g_slist_next(list)) {
		retval->_buffer[i++]=CORBA_Object_duplicate((CORBA_Object)list->data, ev);
	}
	
	g_slist_free(data.seq);
	
	return(retval);
}

static CORBA_ContainedSeq *container_lookup_name(GSList *contents,
						 CORBA_Identifier search_name,
						 CORBA_long levels_to_search,
						 CORBA_DefinitionKind limit_type,
						 CORBA_boolean exclude_inherited,
						 char *container_type,
						 CORBA_Environment *ev)
{
	CORBA_ContainedSeq *retval;
	ContainedIterData data;
	CORBA_unsigned_long len;
	GSList *list;
	int i;
	
	data.search_name=search_name;
	data.levels_to_search=levels_to_search;
	data.limit_type=limit_type;
	data.exclude_inherited=exclude_inherited;
	data.seq=NULL;
	
	g_slist_foreach(contents, container_contents_internal, &data);
	
	len=g_slist_length(data.seq);
	
	retval=CORBA_ContainedSeq__alloc();
	retval->_length=len;
	retval->_maximum=len;
	retval->_buffer=CORBA_sequence_CORBA_Contained_allocbuf(len);

	if(retval->_buffer==NULL) {
		CORBA_free(retval);
		return(CORBA_OBJECT_NIL);
	}
	
	for(i=0,list=data.seq;list;list=g_slist_next(list)) {
		retval->_buffer[i++]=CORBA_Object_duplicate((CORBA_Object)list->data, ev);
	}
	
	g_slist_free(data.seq);
	
	return(retval);
}

static CORBA_Container_DescriptionSeq *
container_describe_contents(GSList *contents,
			    CORBA_DefinitionKind limit_type,
			    CORBA_boolean exclude_inherited,
			    CORBA_long max_returned_objs,
			    char *container_type,
			    CORBA_Environment *ev)
{
	CORBA_Container_DescriptionSeq *retval;
	CORBA_ContainedSeq *cont;
	int i, max;
	
	ORBit_Trace(TraceMod_IR, TraceLevel_Info, "%s_describe_contents\n",
		    container_type);
	
	cont=container_contents(contents, limit_type, exclude_inherited, container_type, ev);
	if(cont==CORBA_OBJECT_NIL) {
		return(CORBA_OBJECT_NIL);
	}

	if(max_returned_objs != -1 && max_returned_objs < cont->_length) {
		ORBit_Trace(TraceMod_IR, TraceLevel_Debug, "%s_describe_contents: limiting returns to %d\n", container_type, max_returned_objs);
		
		max=max_returned_objs;
	} else {
		ORBit_Trace(TraceMod_IR, TraceLevel_Debug, "%s_describe_contents: found %d\n", container_type, cont->_length);
		
		max=cont->_length;
	}
	
	retval=CORBA_Container_DescriptionSeq__alloc();
	if(retval==NULL) {
		CORBA_free(cont);
		return(CORBA_OBJECT_NIL);
	}
	retval->_length=max;
	retval->_maximum=max;
	retval->_buffer=CORBA_sequence_CORBA_Container_Description_allocbuf(max);
	
	if(retval->_buffer==NULL) {
		CORBA_free(cont);
		CORBA_free(retval);
		return(CORBA_OBJECT_NIL);
	}
	
	for(i=0; i<max; i++) {
		CORBA_Contained_Description *contained_desc;
		CORBA_Container_Description container_desc;

		ORBit_Trace(TraceMod_IR, TraceLevel_Debug,
			    "%s_describe_contents: filling slot %d\n",
			    container_type, i);
		
		contained_desc=CORBA_Contained_describe(cont->_buffer[i], ev);
		if(contained_desc==CORBA_OBJECT_NIL) {
			CORBA_free(cont);
			CORBA_free(retval);
			
			return(CORBA_OBJECT_NIL);
		}
		
		container_desc.contained_object=cont->_buffer[i];
		CORBA_any__copy(&container_desc.value,
				&contained_desc->value);
		container_desc.kind=contained_desc->kind;
		CORBA_free(contained_desc);
		
		retval->_buffer[i]=container_desc;
	}

	CORBA_free(cont);

	return(retval);
}

static CORBA_ModuleDef container_create_module(GSList **contents,
					       PortableServer_POA poa,
					       CORBA_RepositoryId id,
					       CORBA_Identifier name,
					       CORBA_VersionSpec version,
					       CORBA_Container defined_in,
					       char *container_type,
					       CORBA_Environment *ev)
{
	impl_POA_CORBA_ModuleDef *newservant;
		
	ORBit_Trace(TraceMod_IR, TraceLevel_Debug,
		    "%s_create_module: Creating module id %s called %s/%s\n",
		    container_type, id, name, version);
	
	/* Make sure this Repo ID isn't taken */
	if(repo_check_id(id)==CORBA_TRUE) {
		ORBit_Trace(TraceMod_IR, TraceLevel_Info,
			    "%s_create_module: RepositoryID %s already exists.\n",
			    container_type, id);
		
		return(CORBA_OBJECT_NIL);
	}

	/* Make sure this name/version doesnt already exist in this
           container */
	if(repo_check_name_and_version(*contents, name, version)==CORBA_TRUE) {
		ORBit_Trace(TraceMod_IR, TraceLevel_Info,
			    "%s_create_module: Name %s/%s already exists in this container.\n",
			    container_type, name, version);
		
		return(CORBA_OBJECT_NIL);
	}

	newservant=impl_CORBA_ModuleDef__create(poa, id, name,
						version, defined_in, ev);
	if(newservant==NULL) {
		return(CORBA_OBJECT_NIL);
	}
	
	/* Add to contents list */
	*contents=g_slist_append(*contents, newservant);

	return(CORBA_Object_duplicate(newservant->obj, ev));
}

static CORBA_ConstantDef container_create_constant(GSList **contents,
						   PortableServer_POA poa,
						   CORBA_RepositoryId id,
						   CORBA_Identifier name,
						   CORBA_VersionSpec version,
						   CORBA_IDLType type,
						   CORBA_any *value,
						   CORBA_Container defined_in,
						   char *container_type,
						   CORBA_Environment *ev)
{
	impl_POA_CORBA_ConstantDef *newservant;
	
	ORBit_Trace(TraceMod_IR, TraceLevel_Debug,
		    "%s_create_constant: Creating constant id %s called %s/%s\n",
		    container_type, id, name, version);
	
	/* Make sure this Repo ID isn't taken */
	if(repo_check_id(id)==CORBA_TRUE) {
		ORBit_Trace(TraceMod_IR, TraceLevel_Info,
			    "%s_create_constant: RepositoryID %s already exists.\n",
			    container_type, id);
		
		return(CORBA_OBJECT_NIL);
	}
	
	/* Make sure this name/version doesnt already exist in this
           container */
	if(repo_check_name_and_version(*contents, name, version)==CORBA_TRUE) {
		ORBit_Trace(TraceMod_IR, TraceLevel_Info,
			    "%s_create_constant: Name %s/%s already exists in this container.\n",
			    container_type, name, version);
		
		return(CORBA_OBJECT_NIL);
	}

	newservant=impl_CORBA_ConstantDef__create(poa, id, name,
						  version, type, value,
						  defined_in, ev);
	if(newservant==NULL) {
		return(CORBA_OBJECT_NIL);
	}
	
	/* Add to contents list */
	*contents=g_slist_append(*contents, newservant);

	return(CORBA_Object_duplicate(newservant->obj, ev));
}

static CORBA_StructDef container_create_struct(GSList **contents,
					       PortableServer_POA poa,
					       CORBA_RepositoryId id,
					       CORBA_Identifier name,
					       CORBA_VersionSpec version,
					       CORBA_StructMemberSeq *members,
					       CORBA_Container defined_in,
					       char *container_type,
					       CORBA_Environment *ev)
{
	impl_POA_CORBA_StructDef *newservant;
	
	ORBit_Trace(TraceMod_IR, TraceLevel_Debug,
		    "%s_create_struct: Creating struct id %s called %s/%s\n",
		    container_type, id, name, version);
	
	/* Make sure this Repo ID isn't taken */
	if(repo_check_id(id)==CORBA_TRUE) {
		ORBit_Trace(TraceMod_IR, TraceLevel_Info,
			    "%s_create_struct: RepositoryID %s already exists.\n",
			    container_type, id);
		
		return(CORBA_OBJECT_NIL);
	}
	
	/* Make sure this name/version doesnt already exist in this
           container */
	if(repo_check_name_and_version(*contents, name, version)==CORBA_TRUE) {
		ORBit_Trace(TraceMod_IR, TraceLevel_Info,
			    "%s_create_struct: Name %s/%s already exists in this container.\n",
			    container_type, name, version);
		
		return(CORBA_OBJECT_NIL);
	}

	newservant=impl_CORBA_StructDef__create(poa, id, name,
						version, members,
						defined_in, ev);
	if(newservant==NULL) {
		return(CORBA_OBJECT_NIL);
	}
	
	/* Add to contents list */
	*contents=g_slist_append(*contents, newservant);

	return(CORBA_Object_duplicate(newservant->obj, ev));
}

static CORBA_UnionDef container_create_union(GSList **contents,
					     PortableServer_POA poa,
					     CORBA_RepositoryId id,
					     CORBA_Identifier name,
					     CORBA_VersionSpec version,
					     CORBA_IDLType discriminator_type,
					     CORBA_UnionMemberSeq *members,
					     CORBA_Container defined_in,
					     char *container_type,
					     CORBA_Environment *ev)
{
	impl_POA_CORBA_UnionDef *newservant;
	
	ORBit_Trace(TraceMod_IR, TraceLevel_Debug,
		    "%s_create_union: Creating union id %s called %s/%s\n",
		    container_type, id, name, version);
	
	/* Make sure this Repo ID isn't taken */
	if(repo_check_id(id)==CORBA_TRUE) {
		ORBit_Trace(TraceMod_IR, TraceLevel_Info,
			    "%s_create_union: RepositoryID %s already exists.\n",
			    container_type, id);
		
		return(CORBA_OBJECT_NIL);
	}
	
	/* Make sure this name/version doesnt already exist in this
           container */
	if(repo_check_name_and_version(*contents, name, version)==CORBA_TRUE) {
		ORBit_Trace(TraceMod_IR, TraceLevel_Info,
			    "%s_create_union: Name %s/%s already exists in this container.\n",
			    container_type, name, version);
		
		return(CORBA_OBJECT_NIL);
	}

	newservant=impl_CORBA_UnionDef__create(poa, id, name,
					       version, discriminator_type,
					       members, defined_in, ev);
	if(newservant==NULL) {
		return(CORBA_OBJECT_NIL);
	}
	
	/* Add to contents list */
	*contents=g_slist_append(*contents, newservant);

	return(CORBA_Object_duplicate(newservant->obj, ev));
}

static CORBA_EnumDef container_create_enum(GSList **contents,
					   PortableServer_POA poa,
					   CORBA_RepositoryId id,
					   CORBA_Identifier name,
					   CORBA_VersionSpec version,
					   CORBA_EnumMemberSeq *members,
					   CORBA_Container defined_in,
					   char *container_type,
					   CORBA_Environment *ev)
{
	impl_POA_CORBA_EnumDef *newservant;
	
	ORBit_Trace(TraceMod_IR, TraceLevel_Debug,
		    "%s_create_enum: Creating enum id %s called %s/%s\n",
		    container_type, id, name, version);
	
	/* Make sure this Repo ID isn't taken */
	if(repo_check_id(id)==CORBA_TRUE) {
		ORBit_Trace(TraceMod_IR, TraceLevel_Info,
			    "%s_create_enum: RepositoryID %s already exists.\n",
			    container_type, id);
		
		return(CORBA_OBJECT_NIL);
	}
	
	/* Make sure this name/version doesnt already exist in this
           container */
	if(repo_check_name_and_version(*contents, name, version)==CORBA_TRUE) {
		ORBit_Trace(TraceMod_IR, TraceLevel_Info,
			    "%s_create_enum: Name %s/%s already exists in this container.\n",
			    container_type, name, version);
		
		return(CORBA_OBJECT_NIL);
	}

	newservant=impl_CORBA_EnumDef__create(poa, id, name,
					      version, members,
					      defined_in, ev);
	if(newservant==NULL) {
		return(CORBA_OBJECT_NIL);
	}
	
	/* Add to contents list */
	*contents=g_slist_append(*contents, newservant);

	return(CORBA_Object_duplicate(newservant->obj, ev));
}

static CORBA_AliasDef container_create_alias(GSList **contents,
					     PortableServer_POA poa,
					     CORBA_RepositoryId id,
					     CORBA_Identifier name,
					     CORBA_VersionSpec version,
					     CORBA_IDLType original_type,
					     CORBA_Container defined_in,
					     char *container_type,
					     CORBA_Environment *ev)
{
	impl_POA_CORBA_AliasDef *newservant;
	
	ORBit_Trace(TraceMod_IR, TraceLevel_Debug,
		    "%s_create_alias: Creating alias id %s called %s/%s\n",
		    container_type, id, name, version);
	
	/* Make sure this Repo ID isn't taken */
	if(repo_check_id(id)==CORBA_TRUE) {
		ORBit_Trace(TraceMod_IR, TraceLevel_Info,
			    "%s_create_alias: RepositoryID %s already exists.\n",
			    container_type, id);
		
		return(CORBA_OBJECT_NIL);
	}
	
	/* Make sure this name/version doesnt already exist in this
           container */
	if(repo_check_name_and_version(*contents, name, version)==CORBA_TRUE) {
		ORBit_Trace(TraceMod_IR, TraceLevel_Info,
			    "%s_create_alias: Name %s/%s already exists in this container.\n",
			    container_type, name, version);
		
		return(CORBA_OBJECT_NIL);
	}

	newservant=impl_CORBA_AliasDef__create(poa, id, name,
					       version, original_type,
					       defined_in, ev);
	if(newservant==NULL) {
		return(CORBA_OBJECT_NIL);
	}
	
	/* Add to contents list */
	*contents=g_slist_append(*contents, newservant);

	return(CORBA_Object_duplicate(newservant->obj, ev));
}

static CORBA_InterfaceDef container_create_interface(GSList **contents,
						     PortableServer_POA poa,
						     CORBA_RepositoryId id,
						     CORBA_Identifier name,
						     CORBA_VersionSpec version,
						     CORBA_InterfaceDefSeq *base_interfaces,
						     CORBA_Container defined_in,
						     char *container_type,
						     CORBA_Environment *ev)
{
	impl_POA_CORBA_InterfaceDef *newservant;
	
	ORBit_Trace(TraceMod_IR, TraceLevel_Debug,
		    "%s_create_interface: Creating interface id %s called %s/%s\n",
		    container_type, id, name, version);
	
	/* Make sure this Repo ID isn't taken */
	if(repo_check_id(id)==CORBA_TRUE) {
		ORBit_Trace(TraceMod_IR, TraceLevel_Info,
			    "%s_create_interface: RepositoryID %s already exists.\n",
			    container_type, id);
		
		return(CORBA_OBJECT_NIL);
	}
	
	/* Make sure this name/version doesnt already exist in this
           container */
	if(repo_check_name_and_version(*contents, name, version)==CORBA_TRUE) {
		ORBit_Trace(TraceMod_IR, TraceLevel_Info,
			    "%s_create_interface: Name %s/%s already exists in this container.\n",
			    container_type, name, version);
		
		return(CORBA_OBJECT_NIL);
	}

	newservant=impl_CORBA_InterfaceDef__create(poa, id, name,
						   version, base_interfaces,
						   defined_in, ev);
	if(newservant==NULL) {
		return(CORBA_OBJECT_NIL);
	}
	
	/* Add to contents list */
	*contents=g_slist_append(*contents, newservant);

	return(CORBA_Object_duplicate(newservant->obj, ev));
}

static CORBA_OperationDef container_create_operation(GSList **contents,
						     PortableServer_POA poa,
						     CORBA_RepositoryId id,
						     CORBA_Identifier name,
						     CORBA_VersionSpec version,
						     CORBA_IDLType result,
						     CORBA_OperationMode mode,
						     CORBA_ParDescriptionSeq *params,
						     CORBA_ExceptionDefSeq *exceptions,
						     CORBA_ContextIdSeq *contexts,
						     CORBA_Container defined_in,
						     char *container_type,
						     CORBA_Environment *ev)
{
	impl_POA_CORBA_OperationDef *newservant;
	
	ORBit_Trace(TraceMod_IR, TraceLevel_Debug,
		    "%s_create_operation: Creating operation id %s called %s/%s\n",
		    container_type, id, name, version);
	
	/* Make sure this Repo ID isn't taken */
	if(repo_check_id(id)==CORBA_TRUE) {
		ORBit_Trace(TraceMod_IR, TraceLevel_Info,
			    "%s_create_operation: RepositoryID %s already exists.\n",
			    container_type, id);
		
		return(CORBA_OBJECT_NIL);
	}
	
	/* Make sure this name/version doesnt already exist in this
           container */
	if(repo_check_name_and_version(*contents, name, version)==CORBA_TRUE) {
		ORBit_Trace(TraceMod_IR, TraceLevel_Info,
			    "%s_create_operation: Name %s/%s already exists in this container.\n",
			    container_type, name, version);
		
		return(CORBA_OBJECT_NIL);
	}

	newservant=impl_CORBA_OperationDef__create(poa, id, name, version,
						   result, mode, params,
						   exceptions, contexts,
						   defined_in, ev);
	if(newservant==NULL) {
		return(CORBA_OBJECT_NIL);
	}
	
	/* Add to contents list */
	*contents=g_slist_append(*contents, newservant);

	return(CORBA_Object_duplicate(newservant->obj, ev));
}

static CORBA_ExceptionDef container_create_exception(GSList **contents,
						     PortableServer_POA poa,
						     CORBA_RepositoryId id,
						     CORBA_Identifier name,
						     CORBA_VersionSpec version,
						     CORBA_StructMemberSeq *members,
						     CORBA_Container defined_in,
						     char *container_type,
						     CORBA_Environment *ev)
{
	impl_POA_CORBA_ExceptionDef *newservant;
	
	ORBit_Trace(TraceMod_IR, TraceLevel_Debug,
		    "%s_create_exception: Creating exception id %s called %s/%s\n",
		    container_type, id, name, version);
	
	/* Make sure this Repo ID isn't taken */
	if(repo_check_id(id)==CORBA_TRUE) {
		ORBit_Trace(TraceMod_IR, TraceLevel_Info,
			    "%s_create_exception: RepositoryID %s already exists.\n",
			    container_type, id);
		
		return(CORBA_OBJECT_NIL);
	}
	
	/* Make sure this name/version doesnt already exist in this
           container */
	if(repo_check_name_and_version(*contents, name, version)==CORBA_TRUE) {
		ORBit_Trace(TraceMod_IR, TraceLevel_Info,
			    "%s_create_exception: Name %s/%s already exists in this container.\n",
			    container_type, name, version);
		
		return(CORBA_OBJECT_NIL);
	}

	newservant=impl_CORBA_ExceptionDef__create(poa, id, name,
						   version, members,
						   defined_in, ev);
	if(newservant==NULL) {
		return(CORBA_OBJECT_NIL);
	}
	
	/* Add to contents list */
	*contents=g_slist_append(*contents, newservant);

	return(CORBA_Object_duplicate(newservant->obj, ev));
}

static void contained__set_id(CORBA_RepositoryId *id,
			      CORBA_RepositoryId value,
			      char *contained_type)
{
	if(value==NULL) {
		ORBit_Trace(TraceMod_IR, TraceLevel_Debug,
			    "%s__set_id: value NULL\n", contained_type);
		return;
	}

	if(repo_check_id(value)==CORBA_TRUE) {
		ORBit_Trace(TraceMod_IR, TraceLevel_Info,
			    "%s__set_id: RepositoryID %s already exists.\n",
			    contained_type, value);
		
		return;
	}
	repo_remove_id(*id);
	
	CORBA_free(*id);
	*id=CORBA_string_dup(value);
}

static void contained__set_name(CORBA_Identifier *name,
				CORBA_Identifier value,
				char *contained_type)
{
	if(value==NULL) {
		ORBit_Trace(TraceMod_IR, TraceLevel_Debug,
			    "%s__set_name: value NULL\n", contained_type);
		return;
	}
	
	/* TODO: check new name and version is available */
	
	CORBA_free(*name);
	*name=CORBA_string_dup(value);
}

static void contained__set_version(CORBA_VersionSpec *version,
				CORBA_VersionSpec value,
				char *contained_type)
{
	if(value==NULL) {
		ORBit_Trace(TraceMod_IR, TraceLevel_Debug,
			    "%s__set_version: value NULL\n", contained_type);
		return;
	}
	
	/* TODO: check new name and version is available */
	
	CORBA_free(*version);
	*version=CORBA_string_dup(value);
}

static CORBA_ScopedName get_absolute_name(CORBA_Identifier name,
					  CORBA_Container defined_in,
					  CORBA_Environment *ev)
{
	CORBA_ScopedName retval;
	gchar *scoped_name;

	ORBit_Trace(TraceMod_IR, TraceLevel_Debug, "get_absolute_name: %s\n",
		    name);
	
	if(defined_in==NULL ||
	   CORBA_IDLType__get_def_kind(defined_in, ev)==CORBA_dk_Repository) {
		scoped_name=g_strconcat("::", name, NULL);
	} else {
		CORBA_ScopedName parent;
		
		parent=CORBA_Contained__get_absolute_name(defined_in, ev);
		scoped_name=g_strconcat(parent, "::", name, NULL);
		CORBA_free(parent);
	}
	
	retval=CORBA_string_dup(scoped_name);

	g_free(scoped_name);
	
	ORBit_Trace(TraceMod_IR, TraceLevel_Debug,
		    "get_absolute_name: returning %s\n", retval);
	
	return(retval);
}

static CORBA_boolean contained_remove_from_parent(GSList **list,
						  gpointer contained)
{
	GSList *listiter= *list, **nextlist;
	
	for(;listiter;listiter=g_slist_next(listiter)) {
		IfaceRepoContents *cont=(IfaceRepoContents *)listiter->data;
		
		switch(cont->attr_def_kind) {
		case CORBA_dk_Repository:
			nextlist=&((impl_POA_CORBA_Repository *)cont)->contents;
			break;
		case CORBA_dk_Module:
			nextlist=&((impl_POA_CORBA_ModuleDef *)cont)->contents;
			break;
		case CORBA_dk_Struct:
			nextlist=&((impl_POA_CORBA_StructDef *)cont)->contents;
			break;
		case CORBA_dk_Union:
			nextlist=&((impl_POA_CORBA_UnionDef *)cont)->contents;
			break;
		case CORBA_dk_Exception:
			nextlist=&((impl_POA_CORBA_ExceptionDef *)cont)->contents;
			break;
		case CORBA_dk_Interface:
			nextlist=&((impl_POA_CORBA_InterfaceDef *)cont)->contents;
			break;
		default:
			/* Not a container, so leave list pointer unset */
			nextlist=NULL;
			break;
		}
		
		if(cont==contained) {
			/* Found what we're looking for */
			*list=g_slist_remove(*list, contained);
			return(CORBA_TRUE);
		} else if(nextlist!=NULL) {
			/* Found a container to search */
			if(contained_remove_from_parent(nextlist, contained)==CORBA_TRUE) {
				return(CORBA_TRUE);
			}
		}
	}

	return(CORBA_FALSE);
}

static IfaceRepoContents *container_lookup_servant(GSList *list,
						   CORBA_Container container)
{
	GSList *nextlist;
	
	g_warning("Replace container_lookup_servant() with PortableServer_POA_reference_to_servant() when the latter has been implemented.");
	
	for(;list;list=g_slist_next(list)) {
		IfaceRepoContents *cont=(IfaceRepoContents *)list->data;
		
		switch(cont->attr_def_kind) {
		case CORBA_dk_Repository:
			nextlist=((impl_POA_CORBA_Repository *)cont)->contents;
			break;
		case CORBA_dk_Module:
			nextlist=((impl_POA_CORBA_ModuleDef *)cont)->contents;
			break;
		case CORBA_dk_Struct:
			nextlist=((impl_POA_CORBA_StructDef *)cont)->contents;
			break;
		case CORBA_dk_Union:
			nextlist=((impl_POA_CORBA_UnionDef *)cont)->contents;
			break;
		case CORBA_dk_Exception:
			nextlist=((impl_POA_CORBA_ExceptionDef *)cont)->contents;
			break;
		case CORBA_dk_Interface:
			nextlist=((impl_POA_CORBA_InterfaceDef *)cont)->contents;
			break;
		default:
			/* Not a container, so leave list pointer unset */
			nextlist=NULL;
			break;
		}
		
		if(cont->obj==container) {
			/* Found what we're looking for */
			return(cont);
		} else if(nextlist!=NULL) {
			/* Found a container to search */
			IfaceRepoContents *ret;
			
			ret=container_lookup_servant(nextlist, container);
			if(ret!=CORBA_OBJECT_NIL) {
				return(ret);
			}
		}
	}

	return(CORBA_OBJECT_NIL);
}

static void contained_move(IfaceRepoContents *servant,
			   CORBA_Container new_container,
			   CORBA_Identifier new_name,
			   CORBA_VersionSpec new_version,
			   char *contained_type,
			   CORBA_Container *defined_in,
			   CORBA_Environment *ev)
{
	IfaceRepoContents *new_servant;
	GSList **contents;
	
	new_servant=container_lookup_servant(repo->contents, new_container);
	
	if(new_servant==CORBA_OBJECT_NIL) {
		ORBit_Trace(TraceMod_IR, TraceLevel_Debug,
			    "%s_move: Can't find new_container servant\n",
			    contained_type);
		return;
	}

	switch(new_servant->attr_def_kind) {
	case CORBA_dk_Repository:
		contents=&((impl_POA_CORBA_Repository *)new_servant)->contents;
		break;
	case CORBA_dk_Module:
		contents=&((impl_POA_CORBA_ModuleDef *)new_servant)->contents;
		break;
	case CORBA_dk_Struct:
		contents=&((impl_POA_CORBA_StructDef *)new_servant)->contents;
		break;
	case CORBA_dk_Union:
		contents=&((impl_POA_CORBA_UnionDef *)new_servant)->contents;
		break;
	case CORBA_dk_Exception:
		contents=&((impl_POA_CORBA_ExceptionDef *)new_servant)->contents;
		break;
	case CORBA_dk_Interface:
		contents=&((impl_POA_CORBA_InterfaceDef *)new_servant)->contents;
		break;
	default:
		ORBit_Trace(TraceMod_IR, TraceLevel_Debug,
			    "%s_move: servant not a container!\n",
			    contained_type);
		return;
	}
	
	/* Make sure this new_name/new_version doesn't already exist
           in thie new_container */
	if(repo_check_name_and_version(*contents, new_name,
				       new_version)==CORBA_TRUE) {
		ORBit_Trace(TraceMod_IR, TraceLevel_Info,
			    "%s_move: Name %s/%s already exists in new container.\n",
			    contained_type, new_name, new_version);
		return;
	}
	
	/* Unlink from the old parent */
	contained_remove_from_parent(&repo->contents, servant);
	/* And link to the new one */
	*contents=g_slist_append(*contents, servant);
	if(*defined_in!=CORBA_OBJECT_NIL) {
		CORBA_Object_release(*defined_in, ev);
	}
	
	*defined_in=CORBA_Object_duplicate(new_container, ev);

	/* Update with the new name and version */
	contained__set_name(&servant->attr_name, new_name, contained_type);
	contained__set_version(&servant->attr_version, new_version, contained_type);
}

/* XXX fill in required checks here */
static CORBA_boolean structmemberseq_verify(const CORBA_StructMemberSeq *seq)
{
	if(seq==NULL) {
		return(CORBA_FALSE);
	}
	
	if(seq->_length==0) {
		return(CORBA_FALSE);
	}
	
	return(CORBA_TRUE);
}

static CORBA_StructMemberSeq *structmemberseq_copy(const CORBA_StructMemberSeq *old)
{
	CORBA_StructMemberSeq *new;
	int i;

	if(old==NULL) {
		return(NULL);
	}
	
	new=CORBA_StructMemberSeq__alloc();
	if(new==NULL) {
		return(NULL);
	}
	
	new->_length=old->_length;
	new->_maximum=old->_maximum;
	new->_buffer=CORBA_sequence_CORBA_StructMember_allocbuf(new->_length);
	if(new->_buffer==NULL) {
		CORBA_free(new);
		return(NULL);
	}
	
	for(i=0; i<new->_length; i++) {
		new->_buffer[i]=old->_buffer[i];
	}
	
	return(new);
}

static void structmemberseq_free(CORBA_StructMemberSeq *seq)
{
	if(seq==NULL) {
		return;
	}
	
	CORBA_free(seq->_buffer);
	CORBA_free(seq);
}

/* XXX fill in required checks here */
static CORBA_boolean unionmemberseq_verify(const CORBA_UnionMemberSeq *seq)
{
	if(seq==NULL) {
		return(CORBA_FALSE);
	}
	
	if(seq->_length==0) {
		return(CORBA_FALSE);
	}
	
	return(CORBA_TRUE);
}

static CORBA_UnionMemberSeq *unionmemberseq_copy(const CORBA_UnionMemberSeq *old)
{
	CORBA_UnionMemberSeq *new;
	int i;

	if(old==NULL) {
		return(NULL);
	}
	
	new=CORBA_UnionMemberSeq__alloc();
	if(new==NULL) {
		return(NULL);
	}
	
	new->_length=old->_length;
	new->_maximum=old->_maximum;
	new->_buffer=CORBA_sequence_CORBA_UnionMember_allocbuf(new->_length);
	if(new->_buffer==NULL) {
		CORBA_free(new);
		return(NULL);
	}
	
	for(i=0; i<new->_length; i++) {
		new->_buffer[i]=old->_buffer[i];
	}
	
	return(new);
}

static void unionmemberseq_free(CORBA_UnionMemberSeq *seq)
{
	if(seq==NULL) {
		return;
	}
	
	CORBA_free(seq->_buffer);
	CORBA_free(seq);
}

/* XXX fill in required checks here */
static CORBA_boolean enummemberseq_verify(const CORBA_EnumMemberSeq *seq)
{
	if(seq==NULL) {
		return(CORBA_FALSE);
	}
	
	if(seq->_length==0) {
		return(CORBA_FALSE);
	}
	
	return(CORBA_TRUE);
}

static CORBA_EnumMemberSeq *enummemberseq_copy(const CORBA_EnumMemberSeq *old)
{
	CORBA_EnumMemberSeq *new;
	int i;

	if(old==NULL) {
		return(NULL);
	}
	
	new=CORBA_EnumMemberSeq__alloc();
	if(new==NULL) {
		return(NULL);
	}
	
	new->_length=old->_length;
	new->_maximum=old->_maximum;
	new->_buffer=CORBA_sequence_CORBA_Identifier_allocbuf(new->_length);
	if(new->_buffer==NULL) {
		CORBA_free(new);
		return(NULL);
	}
	
	for(i=0; i<new->_length; i++) {
		new->_buffer[i]=old->_buffer[i];
	}
	
	return(new);
}

static void enummemberseq_free(CORBA_EnumMemberSeq *seq)
{
	if(seq==NULL) {
		return;
	}
	
	CORBA_free(seq->_buffer);
	CORBA_free(seq);
}

/* XXX fill in required checks here */
static CORBA_boolean pardescriptionseq_verify(const CORBA_ParDescriptionSeq *seq)
{
	if(seq==NULL) {
		return(CORBA_FALSE);
	}
	
	if(seq->_length==0) {
		return(CORBA_FALSE);
	}
	
	return(CORBA_TRUE);
}

static CORBA_ParDescriptionSeq *pardescriptionseq_copy(const CORBA_ParDescriptionSeq *old)
{
	CORBA_ParDescriptionSeq *new;
	int i;

	if(old==NULL) {
		return(NULL);
	}
	
	new=CORBA_ParDescriptionSeq__alloc();
	if(new==NULL) {
		return(NULL);
	}
	
	new->_length=old->_length;
	new->_maximum=old->_maximum;
	new->_buffer=CORBA_sequence_CORBA_ParameterDescription_allocbuf(new->_length);
	if(new->_buffer==NULL) {
		CORBA_free(new);
		return(NULL);
	}
	
	for(i=0; i<new->_length; i++) {
		new->_buffer[i]=old->_buffer[i];
	}
	
	return(new);
}

static void pardescriptionseq_free(CORBA_ParDescriptionSeq *seq)
{
	if(seq==NULL) {
		return;
	}
	
	CORBA_free(seq->_buffer);
	CORBA_free(seq);
}

/* XXX fill in required checks here */
static CORBA_boolean contextidseq_verify(const CORBA_ContextIdSeq *seq)
{
	if(seq==NULL) {
		return(CORBA_FALSE);
	}
	
	if(seq->_length==0) {
		return(CORBA_FALSE);
	}
	
	return(CORBA_TRUE);
}

static CORBA_ContextIdSeq *contextidseq_copy(const CORBA_ContextIdSeq *old)
{
	CORBA_ContextIdSeq *new;
	int i;

	if(old==NULL) {
		return(NULL);
	}
	
	new=CORBA_ContextIdSeq__alloc();
	if(new==NULL) {
		return(NULL);
	}
	
	new->_length=old->_length;
	new->_maximum=old->_maximum;
	new->_buffer=CORBA_sequence_CORBA_ContextIdentifier_allocbuf(new->_length);
	if(new->_buffer==NULL) {
		CORBA_free(new);
		return(NULL);
	}
	
	for(i=0; i<new->_length; i++) {
		new->_buffer[i]=old->_buffer[i];
	}
	
	return(new);
}

static void contextidseq_free(CORBA_ContextIdSeq *seq)
{
	if(seq==NULL) {
		return;
	}
	
	CORBA_free(seq->_buffer);
	CORBA_free(seq);
}

/* XXX fill in required checks here */
static CORBA_boolean exceptiondefseq_verify(const CORBA_ExceptionDefSeq *seq)
{
	if(seq==NULL) {
		return(CORBA_FALSE);
	}
	
	if(seq->_length==0) {
		return(CORBA_FALSE);
	}
	
	return(CORBA_TRUE);
}

static CORBA_ExceptionDefSeq *exceptiondefseq_copy(const CORBA_ExceptionDefSeq *old)
{
	CORBA_ExceptionDefSeq *new;
	int i;

	if(old==NULL) {
		return(NULL);
	}
	
	new=CORBA_ExceptionDefSeq__alloc();
	if(new==NULL) {
		return(NULL);
	}
	
	new->_length=old->_length;
	new->_maximum=old->_maximum;
	new->_buffer=CORBA_sequence_CORBA_ExceptionDef_allocbuf(new->_length);
	if(new->_buffer==NULL) {
		CORBA_free(new);
		return(NULL);
	}
	
	for(i=0; i<new->_length; i++) {
		new->_buffer[i]=old->_buffer[i];
	}
	
	return(new);
}

static void exceptiondefseq_free(CORBA_ExceptionDefSeq *seq)
{
	if(seq==NULL) {
		return;
	}
	
	CORBA_free(seq->_buffer);
	CORBA_free(seq);
}

/* XXX fill in required checks here */
static CORBA_boolean interfacedefseq_verify(const CORBA_InterfaceDefSeq *seq)
{
	if(seq==NULL) {
		return(CORBA_FALSE);
	}
	
	if(seq->_length==0) {
		return(CORBA_FALSE);
	}
	
	return(CORBA_TRUE);
}

static CORBA_InterfaceDefSeq *interfacedefseq_copy(const CORBA_InterfaceDefSeq *old)
{
	CORBA_InterfaceDefSeq *new;
	int i;

	if(old==NULL) {
		return(NULL);
	}
	
	new=CORBA_InterfaceDefSeq__alloc();
	if(new==NULL) {
		return(NULL);
	}
	
	new->_length=old->_length;
	new->_maximum=old->_maximum;
	new->_buffer=CORBA_sequence_CORBA_InterfaceDef_allocbuf(new->_length);
	if(new->_buffer==NULL) {
		CORBA_free(new);
		return(NULL);
	}
	
	for(i=0; i<new->_length; i++) {
		new->_buffer[i]=old->_buffer[i];
	}
	
	return(new);
}

static void interfacedefseq_free(CORBA_InterfaceDefSeq *seq)
{
	if(seq==NULL) {
		return;
	}
	
	CORBA_free(seq->_buffer);
	CORBA_free(seq);
}

static impl_POA_CORBA_IDLType *impl_CORBA_IDLType__create(
	PortableServer_POA poa,
	CORBA_TypeCode type,
	CORBA_Environment * ev)
{
	impl_POA_CORBA_IDLType *newservant;
	PortableServer_ObjectId *objid;

	newservant = g_new0(impl_POA_CORBA_IDLType, 1);
	newservant->servant.vepv = &impl_CORBA_IDLType_vepv;
	newservant->poa = poa;

	newservant->attr_type=type;
	newservant->attr_def_kind=CORBA_dk_none;
	newservant->attr_name=NULL;
	newservant->attr_version=NULL;

	POA_CORBA_IDLType__init((PortableServer_Servant) newservant, ev);
	objid = PortableServer_POA_activate_object(poa, newservant, ev);
	CORBA_free(objid);
	
	newservant->obj=PortableServer_POA_servant_to_reference(poa,
								newservant,
								ev);
	
	return(newservant);
}

/* You shouldn't call this routine directly without first deactivating the servant... */
static void impl_CORBA_IDLType__destroy(impl_POA_CORBA_IDLType * servant, CORBA_Environment * ev)
{
	POA_CORBA_IDLType__fini((PortableServer_Servant) servant, ev);
	g_free(servant);
}

CORBA_TypeCode
impl_CORBA_IDLType__get_type(impl_POA_CORBA_IDLType * servant,
			     CORBA_Environment * ev)
{
	return(servant->attr_type);
}

CORBA_DefinitionKind
impl_CORBA_IDLType__get_def_kind(impl_POA_CORBA_IDLType * servant,
				 CORBA_Environment * ev)
{
	return(servant->attr_def_kind);
}

void impl_CORBA_IDLType_destroy(impl_POA_CORBA_IDLType * servant,
				CORBA_Environment * ev)
{
	ORBit_Trace(TraceMod_IR, TraceLevel_Debug, "Destroying IDLType!\n");
	
	PortableServer_POA_deactivate_object(servant->poa, PortableServer_POA_servant_to_id(servant->poa, servant, ev), ev);
	impl_CORBA_IDLType__destroy(servant, ev);
}

static impl_POA_CORBA_Repository *impl_CORBA_Repository__create(
	PortableServer_POA poa,
	CORBA_Environment * ev)
{
	impl_POA_CORBA_Repository *newservant;
	PortableServer_ObjectId *objid;

	newservant = g_new0(impl_POA_CORBA_Repository, 1);
	newservant->servant.vepv = &impl_CORBA_Repository_vepv;
	newservant->poa = poa;

	newservant->attr_def_kind=CORBA_dk_Repository;
	newservant->attr_name=NULL;
	newservant->attr_version=NULL;
	newservant->id_hash=g_hash_table_new(g_str_hash, g_str_equal);
	newservant->contents=NULL;
	newservant->noncontaineds=NULL;

	POA_CORBA_Repository__init((PortableServer_Servant) newservant, ev);
	objid = PortableServer_POA_activate_object(poa, newservant, ev);
	CORBA_free(objid);

	newservant->obj=PortableServer_POA_servant_to_reference(poa,
								newservant,
								ev);
	
	return(newservant);
}

/* You shouldn't call this routine directly without first deactivating the servant... */
static void impl_CORBA_Repository__destroy(impl_POA_CORBA_Repository * servant, CORBA_Environment * ev)
{
	g_hash_table_destroy(servant->id_hash);
	g_slist_free(servant->contents);
	g_slist_free(servant->noncontaineds);
	POA_CORBA_Repository__fini((PortableServer_Servant) servant, ev);
	g_free(servant);
}

CORBA_Contained
impl_CORBA_Repository_lookup_id(impl_POA_CORBA_Repository * servant,
				CORBA_RepositoryId search_id,
				CORBA_Environment * ev)
{
	CORBA_Contained retval;

	retval = g_hash_table_lookup(servant->id_hash, search_id);

	if(retval) {
		return(CORBA_Object_duplicate(retval, ev));
	} else {
		return(CORBA_OBJECT_NIL);
	}
}

CORBA_PrimitiveDef
impl_CORBA_Repository_get_primitive(impl_POA_CORBA_Repository * servant,
				    CORBA_PrimitiveKind kind,
				    CORBA_Environment * ev)
{
	CORBA_PrimitiveDef retval;

	switch(kind) {
	case CORBA_pk_null:
		retval=prim_null;
		break;
		
	case CORBA_pk_void:
		retval=prim_void;
		break;

	case CORBA_pk_short:
		retval=prim_short;
		break;

	case CORBA_pk_long:
		retval=prim_long;
		break;

	case CORBA_pk_ushort:
		retval=prim_ushort;
		break;

	case CORBA_pk_ulong:
		retval=prim_ulong;
		break;

	case CORBA_pk_float:
		retval=prim_float;
		break;

	case CORBA_pk_double:
		retval=prim_double;
		break;

	case CORBA_pk_boolean:
		retval=prim_boolean;
		break;

	case CORBA_pk_char:
		retval=prim_char;
		break;

	case CORBA_pk_octet:
		retval=prim_octet;
		break;

	case CORBA_pk_any:
		retval=prim_any;
		break;

	case CORBA_pk_TypeCode:
		retval=prim_TypeCode;
		break;

	case CORBA_pk_Principal:
		retval=prim_Principal;
		break;

	case CORBA_pk_string:
		retval=prim_string;
		break;

	case CORBA_pk_objref:
		retval=prim_objref;
		break;

	case CORBA_pk_longlong:
		retval=prim_longlong;
		break;

	case CORBA_pk_ulonglong:
		retval=prim_ulonglong;
		break;

	case CORBA_pk_longdouble:
		retval=prim_longdouble;
		break;

	case CORBA_pk_wchar:
		retval=prim_wchar;
		break;

	case CORBA_pk_wstring:
		retval=prim_wstring;
		break;
	default:
		return(CORBA_OBJECT_NIL);
		break;
	}
		
	return(CORBA_Object_duplicate(retval, ev));
}

CORBA_StringDef
impl_CORBA_Repository_create_string(impl_POA_CORBA_Repository * servant,
				    CORBA_unsigned_long bound,
				    CORBA_Environment * ev)
{
	CORBA_StringDef retval;
	impl_POA_CORBA_StringDef *newservant;
	GSList *list;

	ORBit_Trace(TraceMod_IR, TraceLevel_Debug, "Creating anonymous string of length %d\n", bound);
	
	if(bound==0) {
		ORBit_Trace(TraceMod_IR, TraceLevel_Info, "Unbounded anonymous strings are handled by Primitive\n");
		
		return(CORBA_OBJECT_NIL);
	}
	
	/* See if we already have this length of string defined */
	for(list=repo->noncontaineds; list; list=g_slist_next(list)) {
		IfaceRepoContents *data=(IfaceRepoContents *)list->data;
		
		if(data->attr_def_kind==CORBA_dk_String) {
			impl_POA_CORBA_StringDef *oldservant;

			oldservant=(impl_POA_CORBA_StringDef *)list->data;
			if(oldservant->attr_bound==bound) {
				retval=oldservant->obj;

				ORBit_Trace(TraceMod_IR, TraceLevel_Info, "Returning old copy of string\n");
				return(CORBA_Object_duplicate(retval, ev));
			}
		}
	}
	
	newservant=impl_CORBA_StringDef__create(servant->poa, bound, ev);
	/* Add to non-Containeds list, because StringDef doesn't
           inherit from Contained */
	repo->noncontaineds=g_slist_append(repo->noncontaineds, newservant);
	
	retval=newservant->obj;

	return(CORBA_Object_duplicate(retval, ev));
}

CORBA_WstringDef
impl_CORBA_Repository_create_wstring(impl_POA_CORBA_Repository * servant,
				     CORBA_unsigned_long bound,
				     CORBA_Environment * ev)
{
	CORBA_WstringDef retval;
	impl_POA_CORBA_WstringDef *newservant;
	GSList *list;

	ORBit_Trace(TraceMod_IR, TraceLevel_Debug, "Creating anonymous wstring of length %d\n", bound);
	
	if(bound==0) {
		ORBit_Trace(TraceMod_IR, TraceLevel_Info, "Unbounded anonymous wstrings are handled by Primitive\n");

		return(CORBA_OBJECT_NIL);
	}

	/* See if we already have this length of string defined */
	for(list=repo->noncontaineds; list; list=g_slist_next(list)) {
		IfaceRepoContents *data=(IfaceRepoContents *)list->data;
		
		if(data->attr_def_kind==CORBA_dk_Wstring) {
			impl_POA_CORBA_WstringDef *oldservant;		

			oldservant=(impl_POA_CORBA_WstringDef *)list->data;
			if(oldservant->attr_bound==bound) {
				retval=oldservant->obj;

				ORBit_Trace(TraceMod_IR, TraceLevel_Info, "Returning old copy of wstring\n");
				return(CORBA_Object_duplicate(retval, ev));
			}
		}
	}

	newservant=impl_CORBA_WstringDef__create(servant->poa, bound, ev);
	/* Add to non-Containeds list, because WstringDef doesn't
           inherit from Contained */
	repo->noncontaineds=g_slist_append(repo->noncontaineds, newservant);
	
	retval=newservant->obj;

	return(CORBA_Object_duplicate(retval, ev));
}

CORBA_SequenceDef
impl_CORBA_Repository_create_sequence(impl_POA_CORBA_Repository * servant,
				      CORBA_unsigned_long bound,
				      CORBA_IDLType element_type,
				      CORBA_Environment * ev)
{
	CORBA_SequenceDef retval;
	impl_POA_CORBA_SequenceDef *newservant;
	GSList *list;
	
	ORBit_Trace(TraceMod_IR, TraceLevel_Debug, "Creating anonymous sequence of length %d\n", bound);
	
	/* See if we already have this sequence defined */
	for(list=repo->noncontaineds; list; list=g_slist_next(list)) {
		IfaceRepoContents *data=(IfaceRepoContents *)list->data;
		
		if(data->attr_def_kind==CORBA_dk_Sequence) {
			impl_POA_CORBA_SequenceDef *oldservant;

			oldservant=(impl_POA_CORBA_SequenceDef *)list->data;
			if(oldservant->attr_bound==bound &&
			   CORBA_Object_is_equivalent(oldservant->attr_element_type_def,
						      element_type, ev)) {
				retval=oldservant->obj;

				ORBit_Trace(TraceMod_IR, TraceLevel_Info, "Returning old copy of sequence\n");
				return(CORBA_Object_duplicate(retval, ev));
			}
		}
	}
	
	newservant=impl_CORBA_SequenceDef__create(servant->poa, bound,
						  element_type, ev);
	/* Add to non-Containeds list, because SequenceDef doesn't
           inherit from Contained */
	repo->noncontaineds=g_slist_append(repo->noncontaineds, newservant);

	retval=newservant->obj;

	return(CORBA_Object_duplicate(retval, ev));
}

CORBA_ArrayDef
impl_CORBA_Repository_create_array(impl_POA_CORBA_Repository * servant,
				   CORBA_unsigned_long length,
				   CORBA_IDLType element_type,
				   CORBA_Environment * ev)
{
	CORBA_ArrayDef retval;
	impl_POA_CORBA_ArrayDef *newservant;
	GSList *list;
	
	ORBit_Trace(TraceMod_IR, TraceLevel_Debug, "Creating anonymous array of length %d\n", length);
	
	/* See if we already have this array defined */
	for(list=repo->noncontaineds; list; list=g_slist_next(list)) {
		IfaceRepoContents *data=(IfaceRepoContents *)list->data;
		
		if(data->attr_def_kind==CORBA_dk_Array) {
			impl_POA_CORBA_ArrayDef *oldservant;

			oldservant=(impl_POA_CORBA_ArrayDef *)list->data;
			if(oldservant->attr_length==length &&
			   CORBA_Object_is_equivalent(oldservant->attr_element_type_def,
						      element_type, ev)) {
				retval=oldservant->obj;

				ORBit_Trace(TraceMod_IR, TraceLevel_Info, "Returning old copy of array\n");
				return(CORBA_Object_duplicate(retval, ev));
			}
		}
	}
	
	newservant=impl_CORBA_ArrayDef__create(servant->poa, length,
					       element_type, ev);
	/* Add to non-Containeds list, because ArrayDef doesn't
           inherit from Contained */
	repo->noncontaineds=g_slist_append(repo->noncontaineds, newservant);
	
	retval=newservant->obj;

	return(CORBA_Object_duplicate(retval, ev));
}

CORBA_FixedDef
impl_CORBA_Repository_create_fixed(impl_POA_CORBA_Repository * servant,
				   CORBA_unsigned_short digits,
				   CORBA_short scale,
				   CORBA_Environment * ev)
{
	CORBA_FixedDef retval;
	impl_POA_CORBA_FixedDef *newservant;
	GSList *list;
	
	ORBit_Trace(TraceMod_IR, TraceLevel_Debug, "Creating anonymous fixed with digits %d and scale %d\n", digits, scale);
	
	if(digits<1 || digits>31) {
		ORBit_Trace(TraceMod_IR, TraceLevel_Info, "Fixed digits must be from 1 to 31 inclusive\n");
		
		return(CORBA_OBJECT_NIL);
	}
	
	/* See if we already have this fixed defined */
	for(list=repo->noncontaineds; list; list=g_slist_next(list)) {
		IfaceRepoContents *data=(IfaceRepoContents *)list->data;
		
		if(data->attr_def_kind==CORBA_dk_Fixed) {
			impl_POA_CORBA_FixedDef *oldservant;

			oldservant=(impl_POA_CORBA_FixedDef *)list->data;
			if(oldservant->attr_digits==digits &&
			   oldservant->attr_scale==scale) {
				retval=oldservant->obj;

				ORBit_Trace(TraceMod_IR, TraceLevel_Info, "Returning old copy of fixed\n");
				return(CORBA_Object_duplicate(retval, ev));
			}
		}
	}

	newservant=impl_CORBA_FixedDef__create(servant->poa, digits, scale,
					       ev);
	/* Add to non-Containeds list, because ArrayDef doesn't
           inherit from Contained */
	repo->noncontaineds=g_slist_append(repo->noncontaineds, newservant);
	
	retval=newservant->obj;

	return(CORBA_Object_duplicate(retval, ev));
}

CORBA_DefinitionKind
impl_CORBA_Repository__get_def_kind(impl_POA_CORBA_Repository * servant,
				    CORBA_Environment * ev)
{
	return(servant->attr_def_kind);
}

void impl_CORBA_Repository_destroy(impl_POA_CORBA_Repository * servant,
				   CORBA_Environment * ev)
{
	/* The spec says it's an error to invoke destroy() on a
           Repository.  I don't see why it shouldn't just do the
           logical thing and destroy the entire repo, and shut down.
           - RHP */
	GSList *list, *nextlist;
	
	ORBit_Trace(TraceMod_IR, TraceLevel_Debug, "Destroying Repository!\n");
	
	for(list=servant->contents; list; list=nextlist) {
		/* Recursively delete everything */

		nextlist=g_slist_next(list);	/* _destroy()ing the
                                                   contained items
                                                   fiddles with the
                                                   list we are
                                                   traversing */
		
		CORBA_IRObject_destroy(((IfaceRepoContents *)list->data)->obj,
				       ev);
	}
	
	for(list=servant->noncontaineds; list; list=nextlist) {
		/* Recursively delete everything here too */

		nextlist=g_slist_next(list);	/* _destroy()ing the
                                                   contained items
                                                   fiddles with the
                                                   list we are
                                                   traversing */
		
		CORBA_IRObject_destroy(((IfaceRepoContents *)list->data)->obj,
				       ev);
	}
	
	PortableServer_POA_deactivate_object(servant->poa, PortableServer_POA_servant_to_id(servant->poa, servant, ev), ev);
	impl_CORBA_Repository__destroy(servant, ev);

	CORBA_ORB_shutdown(orb, CORBA_FALSE, ev);
}

CORBA_Contained
impl_CORBA_Repository_lookup(impl_POA_CORBA_Repository * servant,
			     CORBA_ScopedName search_name,
			     CORBA_Environment * ev)
{
	return(container_lookup(servant->contents, search_name, "Repository", ev));
}

CORBA_ContainedSeq *
impl_CORBA_Repository_contents(impl_POA_CORBA_Repository * servant,
			       CORBA_DefinitionKind limit_type,
			       CORBA_boolean exclude_inherited,
			       CORBA_Environment * ev)
{
	return(container_contents(servant->contents, limit_type, exclude_inherited, "Repository", ev));
}

CORBA_ContainedSeq *
impl_CORBA_Repository_lookup_name(impl_POA_CORBA_Repository * servant,
				  CORBA_Identifier search_name,
				  CORBA_long levels_to_search,
				  CORBA_DefinitionKind limit_type,
				  CORBA_boolean exclude_inherited,
				  CORBA_Environment * ev)
{
	return(container_lookup_name(servant->contents, search_name, levels_to_search, limit_type, exclude_inherited, "Repository", ev));
}

CORBA_Container_DescriptionSeq *
 impl_CORBA_Repository_describe_contents(impl_POA_CORBA_Repository * servant,
					 CORBA_DefinitionKind limit_type,
					 CORBA_boolean exclude_inherited,
					 CORBA_long max_returned_objs,
					 CORBA_Environment * ev)
{
	return(container_describe_contents(servant->contents, limit_type, exclude_inherited, max_returned_objs, "Repository", ev));
}

CORBA_ModuleDef
impl_CORBA_Repository_create_module(impl_POA_CORBA_Repository * servant,
				    CORBA_RepositoryId id,
				    CORBA_Identifier name,
				    CORBA_VersionSpec version,
				    CORBA_Environment * ev)
{
	return(container_create_module(&servant->contents, servant->poa, id, name, version, NULL, "Repository", ev));
}

CORBA_ConstantDef
impl_CORBA_Repository_create_constant(impl_POA_CORBA_Repository * servant,
				      CORBA_RepositoryId id,
				      CORBA_Identifier name,
				      CORBA_VersionSpec version,
				      CORBA_IDLType type,
				      CORBA_any * value,
				      CORBA_Environment * ev)
{
	return(container_create_constant(&servant->contents, servant->poa, id, name, version, type, value, NULL, "Repository", ev));
}

CORBA_StructDef
impl_CORBA_Repository_create_struct(impl_POA_CORBA_Repository * servant,
				    CORBA_RepositoryId id,
				    CORBA_Identifier name,
				    CORBA_VersionSpec version,
				    CORBA_StructMemberSeq * members,
				    CORBA_Environment * ev)
{
	return(container_create_struct(&servant->contents, servant->poa, id, name, version, members, NULL, "Repository", ev));
}

CORBA_UnionDef
impl_CORBA_Repository_create_union(impl_POA_CORBA_Repository * servant,
				   CORBA_RepositoryId id,
				   CORBA_Identifier name,
				   CORBA_VersionSpec version,
				   CORBA_IDLType discriminator_type,
				   CORBA_UnionMemberSeq * members,
				   CORBA_Environment * ev)
{
	return(container_create_union(&servant->contents, servant->poa, id, name, version, discriminator_type, members, NULL, "Repository", ev));
}

CORBA_EnumDef
impl_CORBA_Repository_create_enum(impl_POA_CORBA_Repository * servant,
				  CORBA_RepositoryId id,
				  CORBA_Identifier name,
				  CORBA_VersionSpec version,
				  CORBA_EnumMemberSeq * members,
				  CORBA_Environment * ev)
{
	return(container_create_enum(&servant->contents, servant->poa, id, name, version, members, NULL, "Repository", ev));
}

CORBA_AliasDef
impl_CORBA_Repository_create_alias(impl_POA_CORBA_Repository * servant,
				   CORBA_RepositoryId id,
				   CORBA_Identifier name,
				   CORBA_VersionSpec version,
				   CORBA_IDLType original_type,
				   CORBA_Environment * ev)
{
	return(container_create_alias(&servant->contents, servant->poa, id, name, version, original_type, NULL, "Repository", ev));
}

CORBA_InterfaceDef
impl_CORBA_Repository_create_interface(impl_POA_CORBA_Repository * servant,
				       CORBA_RepositoryId id,
				       CORBA_Identifier name,
				       CORBA_VersionSpec version,
				       CORBA_InterfaceDefSeq * base_interfaces,
				       CORBA_Environment * ev)
{
	return(container_create_interface(&servant->contents, servant->poa, id, name, version, base_interfaces, NULL, "Repository", ev));
}

CORBA_ExceptionDef
impl_CORBA_Repository_create_exception(impl_POA_CORBA_Repository * servant,
				       CORBA_RepositoryId id,
				       CORBA_Identifier name,
				       CORBA_VersionSpec version,
				       CORBA_StructMemberSeq * members,
				       CORBA_Environment * ev)
{
	return(container_create_exception(&servant->contents, servant->poa, id, name, version, members, NULL, "Repository", ev));
}

static impl_POA_CORBA_ModuleDef *impl_CORBA_ModuleDef__create(
	PortableServer_POA poa,
	CORBA_RepositoryId id,
	CORBA_Identifier name,
	CORBA_VersionSpec version,
	CORBA_Container defined_in,
	CORBA_Environment * ev)
{
	impl_POA_CORBA_ModuleDef *newservant;
	PortableServer_ObjectId *objid;

	newservant = g_new0(impl_POA_CORBA_ModuleDef, 1);
	newservant->servant.vepv = &impl_CORBA_ModuleDef_vepv;
	newservant->poa = poa;

	newservant->attr_def_kind=CORBA_dk_Module;
	newservant->contents=NULL;

	newservant->attr_id=CORBA_string_dup(id);
	newservant->attr_name=CORBA_string_dup(name);
	newservant->attr_version=CORBA_string_dup(version);
	newservant->attr_defined_in=CORBA_Object_duplicate(defined_in, ev);

	POA_CORBA_ModuleDef__init((PortableServer_Servant) newservant, ev);
	objid = PortableServer_POA_activate_object(poa, newservant, ev);
	CORBA_free(objid);

	newservant->obj=PortableServer_POA_servant_to_reference(poa,
								newservant,
								ev);
	
	g_hash_table_insert(repo->id_hash, g_strdup(id), newservant->obj);
	
	return(newservant);
}

/* You shouldn't call this routine directly without first deactivating the servant... */
static void impl_CORBA_ModuleDef__destroy(impl_POA_CORBA_ModuleDef * servant, CORBA_Environment * ev)
{
	CORBA_free(servant->attr_id);
	CORBA_free(servant->attr_name);
	CORBA_free(servant->attr_version);
	CORBA_Object_release(servant->attr_defined_in, ev);
	
	POA_CORBA_ModuleDef__fini((PortableServer_Servant) servant, ev);
	g_free(servant);
}

CORBA_DefinitionKind
impl_CORBA_ModuleDef__get_def_kind(impl_POA_CORBA_ModuleDef * servant,
				   CORBA_Environment * ev)
{
	return(servant->attr_def_kind);
}

void impl_CORBA_ModuleDef_destroy(impl_POA_CORBA_ModuleDef * servant,
				  CORBA_Environment * ev)
{
	GSList *list, *nextlist;
	
	ORBit_Trace(TraceMod_IR, TraceLevel_Debug,
		    "Destroying ModuleDef %s!\n", servant->attr_id);
	
	for(list=servant->contents; list; list=nextlist) {
		/* Recursively delete everything */

		nextlist=g_slist_next(list);	/* _destroy()ing the
                                                   contained items
                                                   fiddles with the
                                                   list we are
                                                   traversing */
		
		CORBA_IRObject_destroy(((IfaceRepoContents *)list->data)->obj,
				       ev);
	}
	
	/* Unlink from parent container */
	contained_remove_from_parent(&repo->contents, servant);
	
	PortableServer_POA_deactivate_object(servant->poa, PortableServer_POA_servant_to_id(servant->poa, servant, ev), ev);
	impl_CORBA_ModuleDef__destroy(servant, ev);
}

CORBA_Contained
impl_CORBA_ModuleDef_lookup(impl_POA_CORBA_ModuleDef * servant,
			    CORBA_ScopedName search_name,
			    CORBA_Environment * ev)
{
	return(container_lookup(servant->contents, search_name, "ModuleDef", ev));
}

CORBA_ContainedSeq *
 impl_CORBA_ModuleDef_contents(impl_POA_CORBA_ModuleDef * servant,
			       CORBA_DefinitionKind limit_type,
			       CORBA_boolean exclude_inherited,
			       CORBA_Environment * ev)
{
	return(container_contents(servant->contents, limit_type, exclude_inherited, "ModuleDef", ev));
}

CORBA_ContainedSeq *
 impl_CORBA_ModuleDef_lookup_name(impl_POA_CORBA_ModuleDef * servant,
				  CORBA_Identifier search_name,
				  CORBA_long levels_to_search,
				  CORBA_DefinitionKind limit_type,
				  CORBA_boolean exclude_inherited,
				  CORBA_Environment * ev)
{
	return(container_lookup_name(servant->contents, search_name, levels_to_search, limit_type, exclude_inherited, "ModuleDef", ev));
}

CORBA_Container_DescriptionSeq *
 impl_CORBA_ModuleDef_describe_contents(impl_POA_CORBA_ModuleDef * servant,
					CORBA_DefinitionKind limit_type,
					CORBA_boolean exclude_inherited,
					CORBA_long max_returned_objs,
					CORBA_Environment * ev)
{
	return(container_describe_contents(servant->contents, limit_type, exclude_inherited, max_returned_objs, "ModuleDef", ev));
}

CORBA_ModuleDef
impl_CORBA_ModuleDef_create_module(impl_POA_CORBA_ModuleDef * servant,
				   CORBA_RepositoryId id,
				   CORBA_Identifier name,
				   CORBA_VersionSpec version,
				   CORBA_Environment * ev)
{
	return(container_create_module(&servant->contents, servant->poa, id, name, version, servant->obj, "ModuleDef", ev));
}

CORBA_ConstantDef
impl_CORBA_ModuleDef_create_constant(impl_POA_CORBA_ModuleDef * servant,
				     CORBA_RepositoryId id,
				     CORBA_Identifier name,
				     CORBA_VersionSpec version,
				     CORBA_IDLType type,
				     CORBA_any * value,
				     CORBA_Environment * ev)
{
	return(container_create_constant(&servant->contents, servant->poa, id, name, version, type, value, servant->obj, "ModuleDef", ev));
}

CORBA_StructDef
impl_CORBA_ModuleDef_create_struct(impl_POA_CORBA_ModuleDef * servant,
				   CORBA_RepositoryId id,
				   CORBA_Identifier name,
				   CORBA_VersionSpec version,
				   CORBA_StructMemberSeq * members,
				   CORBA_Environment * ev)
{
	return(container_create_struct(&servant->contents, servant->poa, id, name, version, members, servant->obj, "ModuleDef", ev));
}

CORBA_UnionDef
impl_CORBA_ModuleDef_create_union(impl_POA_CORBA_ModuleDef * servant,
				  CORBA_RepositoryId id,
				  CORBA_Identifier name,
				  CORBA_VersionSpec version,
				  CORBA_IDLType discriminator_type,
				  CORBA_UnionMemberSeq * members,
				  CORBA_Environment * ev)
{
	return(container_create_union(&servant->contents, servant->poa, id, name, version, discriminator_type, members, servant->obj, "ModuleDef", ev));
}

CORBA_EnumDef
impl_CORBA_ModuleDef_create_enum(impl_POA_CORBA_ModuleDef * servant,
				 CORBA_RepositoryId id,
				 CORBA_Identifier name,
				 CORBA_VersionSpec version,
				 CORBA_EnumMemberSeq * members,
				 CORBA_Environment * ev)
{
	return(container_create_enum(&servant->contents, servant->poa, id, name, version, members, servant->obj, "ModuleDef", ev));
}

CORBA_AliasDef
impl_CORBA_ModuleDef_create_alias(impl_POA_CORBA_ModuleDef * servant,
				  CORBA_RepositoryId id,
				  CORBA_Identifier name,
				  CORBA_VersionSpec version,
				  CORBA_IDLType original_type,
				  CORBA_Environment * ev)
{
	return(container_create_alias(&servant->contents, servant->poa, id, name, version, original_type, servant->obj, "ModuleDef", ev));
}

CORBA_InterfaceDef
impl_CORBA_ModuleDef_create_interface(impl_POA_CORBA_ModuleDef * servant,
				      CORBA_RepositoryId id,
				      CORBA_Identifier name,
				      CORBA_VersionSpec version,
				      CORBA_InterfaceDefSeq * base_interfaces,
				      CORBA_Environment * ev)
{
	return(container_create_interface(&servant->contents, servant->poa, id, name, version, base_interfaces, servant->obj, "ModuleDef", ev));
}

CORBA_ExceptionDef
impl_CORBA_ModuleDef_create_exception(impl_POA_CORBA_ModuleDef * servant,
				      CORBA_RepositoryId id,
				      CORBA_Identifier name,
				      CORBA_VersionSpec version,
				      CORBA_StructMemberSeq * members,
				      CORBA_Environment * ev)
{
	return(container_create_exception(&servant->contents, servant->poa, id, name, version, members, servant->obj, "ModuleDef", ev));
}

CORBA_RepositoryId
impl_CORBA_ModuleDef__get_id(impl_POA_CORBA_ModuleDef * servant,
			     CORBA_Environment * ev)
{
	return(CORBA_string_dup(servant->attr_id));
}

void impl_CORBA_ModuleDef__set_id(impl_POA_CORBA_ModuleDef * servant,
				  CORBA_RepositoryId value,
				  CORBA_Environment * ev)
{
	contained__set_id(&servant->attr_id, value, "ModuleDef");
}

CORBA_Identifier
impl_CORBA_ModuleDef__get_name(impl_POA_CORBA_ModuleDef * servant,
			       CORBA_Environment * ev)
{
	return(CORBA_string_dup(servant->attr_name));
}

void impl_CORBA_ModuleDef__set_name(impl_POA_CORBA_ModuleDef * servant,
				    CORBA_Identifier value,
				    CORBA_Environment * ev)
{
	contained__set_name(&servant->attr_name, value, "ModuleDef");
}

CORBA_VersionSpec
impl_CORBA_ModuleDef__get_version(impl_POA_CORBA_ModuleDef * servant,
				  CORBA_Environment * ev)
{
	return(CORBA_string_dup(servant->attr_version));
}

void impl_CORBA_ModuleDef__set_version(impl_POA_CORBA_ModuleDef * servant,
				       CORBA_VersionSpec value,
				       CORBA_Environment * ev)
{
	contained__set_version(&servant->attr_version, value, "ModuleDef");
}

CORBA_Container
impl_CORBA_ModuleDef__get_defined_in(impl_POA_CORBA_ModuleDef * servant,
				     CORBA_Environment * ev)
{
	return(CORBA_Object_duplicate(servant->attr_defined_in, ev));
}

CORBA_ScopedName
impl_CORBA_ModuleDef__get_absolute_name(impl_POA_CORBA_ModuleDef * servant,
					CORBA_Environment * ev)
{
	return(get_absolute_name(servant->attr_name, servant->attr_defined_in, ev));
}

CORBA_Repository
impl_CORBA_ModuleDef__get_containing_repository(impl_POA_CORBA_ModuleDef * servant,
						CORBA_Environment * ev)
{
	return(CORBA_Object_duplicate(repo->obj, ev));
}

CORBA_Contained_Description *
 impl_CORBA_ModuleDef_describe(impl_POA_CORBA_ModuleDef * servant,
			       CORBA_Environment * ev)
{
	CORBA_Contained_Description *retval;
	CORBA_ModuleDescription *desc;

	desc=CORBA_ModuleDescription__alloc();
	if(desc==NULL) {
		return(CORBA_OBJECT_NIL);
	}
	
	retval=CORBA_Contained_Description__alloc();
	if(retval==NULL) {
		CORBA_free(desc);
		return(CORBA_OBJECT_NIL);
	}
	
	desc->name=CORBA_string_dup(servant->attr_name);
	desc->id=CORBA_string_dup(servant->attr_id);
	desc->defined_in=CORBA_string_dup(CORBA_Contained__get_id(servant->attr_defined_in, ev));
	desc->version=CORBA_string_dup(servant->attr_version);
	
	retval->kind=CORBA_dk_Module;
	retval->value._type=(CORBA_TypeCode)TC_CORBA_ModuleDescription;
	retval->value._value=desc;
	
	return(retval);
}

void impl_CORBA_ModuleDef_move(impl_POA_CORBA_ModuleDef * servant,
			       CORBA_Container new_container,
			       CORBA_Identifier new_name,
			       CORBA_VersionSpec new_version,
			       CORBA_Environment * ev)
{
	contained_move((IfaceRepoContents *)servant, new_container, new_name, new_version, "ModuleDef", &servant->attr_defined_in, ev);
}

static impl_POA_CORBA_ConstantDef *impl_CORBA_ConstantDef__create(
	PortableServer_POA poa,
	CORBA_RepositoryId id,
	CORBA_Identifier name,
	CORBA_VersionSpec version,
	CORBA_IDLType type_def,
	CORBA_any *value,
	CORBA_Container defined_in,
	CORBA_Environment * ev)
{
	impl_POA_CORBA_ConstantDef *newservant;
	PortableServer_ObjectId *objid;

	newservant = g_new0(impl_POA_CORBA_ConstantDef, 1);
	newservant->servant.vepv = &impl_CORBA_ConstantDef_vepv;
	newservant->poa = poa;

	newservant->attr_def_kind=CORBA_dk_Constant;
	newservant->attr_id=CORBA_string_dup(id);
	newservant->attr_name=CORBA_string_dup(name);
	newservant->attr_version=CORBA_string_dup(version);
	newservant->attr_type_def=CORBA_Object_duplicate(type_def, ev);
	newservant->attr_value=CORBA_any_alloc();
	CORBA_any__copy(newservant->attr_value, value);
	newservant->attr_defined_in=CORBA_Object_duplicate(defined_in, ev);

	POA_CORBA_ConstantDef__init((PortableServer_Servant) newservant, ev);
	objid = PortableServer_POA_activate_object(poa, newservant, ev);
	CORBA_free(objid);

	newservant->obj=PortableServer_POA_servant_to_reference(poa,
								newservant,
								ev);
	
	g_hash_table_insert(repo->id_hash, g_strdup(id), newservant->obj);
	
	return(newservant);
}

/* You shouldn't call this routine directly without first deactivating the servant... */
static void impl_CORBA_ConstantDef__destroy(impl_POA_CORBA_ConstantDef * servant, CORBA_Environment * ev)
{
	CORBA_free(servant->attr_id);
	CORBA_free(servant->attr_name);
	CORBA_free(servant->attr_version);
	CORBA_Object_release(servant->attr_type_def, ev);
	CORBA_Object_release(servant->attr_defined_in, ev);

	POA_CORBA_ConstantDef__fini((PortableServer_Servant) servant, ev);
	g_free(servant);
}

CORBA_TypeCode
impl_CORBA_ConstantDef__get_type(impl_POA_CORBA_ConstantDef * servant,
				 CORBA_Environment * ev)
{
	return(CORBA_IDLType__get_type(servant->attr_type_def, ev));
}

CORBA_IDLType
impl_CORBA_ConstantDef__get_type_def(impl_POA_CORBA_ConstantDef * servant,
				     CORBA_Environment * ev)
{
	return(CORBA_Object_duplicate(servant->attr_type_def, ev));
}

void impl_CORBA_ConstantDef__set_type_def(impl_POA_CORBA_ConstantDef * servant,
					  CORBA_IDLType value,
					  CORBA_Environment * ev)
{
	if(!CORBA_Object_is_nil(servant->attr_type_def, ev)) {
		CORBA_Object_release(servant->attr_type_def, ev);
	}
	servant->attr_type_def=CORBA_Object_duplicate(value, ev);
}

CORBA_any *
 impl_CORBA_ConstantDef__get_value(impl_POA_CORBA_ConstantDef * servant,
				   CORBA_Environment * ev)
{
	CORBA_any *retval;

	retval=CORBA_any_alloc();
	CORBA_any__copy(retval, servant->attr_value);
	
	return(retval);
}

void impl_CORBA_ConstantDef__set_value(impl_POA_CORBA_ConstantDef * servant,
				       CORBA_any * value,
				       CORBA_Environment * ev)
{
	if(servant->attr_value!=NULL) {
		CORBA_free(servant->attr_value);
	}
	servant->attr_value=CORBA_any_alloc();
	CORBA_any__copy(servant->attr_value, value);
}

CORBA_DefinitionKind
impl_CORBA_ConstantDef__get_def_kind(impl_POA_CORBA_ConstantDef * servant,
				     CORBA_Environment * ev)
{
	return(servant->attr_def_kind);
}

void impl_CORBA_ConstantDef_destroy(impl_POA_CORBA_ConstantDef * servant,
				    CORBA_Environment * ev)
{
	ORBit_Trace(TraceMod_IR, TraceLevel_Debug,
		    "Destroying Constant %s %s/%s!\n", servant->attr_id,
		    servant->attr_name, servant->attr_version);

	/* Unlink from parent container */
	contained_remove_from_parent(&repo->contents, servant);
	
	PortableServer_POA_deactivate_object(servant->poa, PortableServer_POA_servant_to_id(servant->poa, servant, ev), ev);
	impl_CORBA_ConstantDef__destroy(servant, ev);
}

CORBA_RepositoryId
impl_CORBA_ConstantDef__get_id(impl_POA_CORBA_ConstantDef * servant,
			       CORBA_Environment * ev)
{
	return(CORBA_string_dup(servant->attr_id));
}

void impl_CORBA_ConstantDef__set_id(impl_POA_CORBA_ConstantDef * servant,
				    CORBA_RepositoryId value,
				    CORBA_Environment * ev)
{
	contained__set_id(&servant->attr_id, value, "ConstantDef");
}

CORBA_Identifier
impl_CORBA_ConstantDef__get_name(impl_POA_CORBA_ConstantDef * servant,
				 CORBA_Environment * ev)
{
	return(CORBA_string_dup(servant->attr_name));
}

void impl_CORBA_ConstantDef__set_name(impl_POA_CORBA_ConstantDef * servant,
				      CORBA_Identifier value,
				      CORBA_Environment * ev)
{
	contained__set_name(&servant->attr_name, value, "ConstantDef");
}

CORBA_VersionSpec
impl_CORBA_ConstantDef__get_version(impl_POA_CORBA_ConstantDef * servant,
				    CORBA_Environment * ev)
{
	return(CORBA_string_dup(servant->attr_version));
}

void impl_CORBA_ConstantDef__set_version(impl_POA_CORBA_ConstantDef * servant,
					 CORBA_VersionSpec value,
					 CORBA_Environment * ev)
{
	contained__set_version(&servant->attr_version, value, "ConstantDef");
}

CORBA_Container
impl_CORBA_ConstantDef__get_defined_in(impl_POA_CORBA_ConstantDef * servant,
				       CORBA_Environment * ev)
{
	return(CORBA_Object_duplicate(servant->attr_defined_in, ev));
}

CORBA_ScopedName
impl_CORBA_ConstantDef__get_absolute_name(impl_POA_CORBA_ConstantDef * servant,
					  CORBA_Environment * ev)
{
	return(get_absolute_name(servant->attr_name, servant->attr_defined_in, ev));
}

CORBA_Repository
impl_CORBA_ConstantDef__get_containing_repository(impl_POA_CORBA_ConstantDef * servant,
						  CORBA_Environment * ev)
{
	return(CORBA_Object_duplicate(repo->obj, ev));
}

CORBA_Contained_Description *
 impl_CORBA_ConstantDef_describe(impl_POA_CORBA_ConstantDef * servant,
				 CORBA_Environment * ev)
{
	CORBA_Contained_Description *retval;
	CORBA_ConstantDescription *desc;
	
	desc=CORBA_ConstantDescription__alloc();
	if(desc==NULL) {
		return(CORBA_OBJECT_NIL);
	}
	
	retval=CORBA_Contained_Description__alloc();
	if(retval==NULL) {
		CORBA_free(desc);
		return(CORBA_OBJECT_NIL);
	}
	
	desc->name=CORBA_string_dup(servant->attr_name);
	desc->id=CORBA_string_dup(servant->attr_id);
	if(servant->attr_defined_in!=NULL) {
		desc->defined_in=CORBA_string_dup(CORBA_Contained__get_id(servant->attr_defined_in, ev));
	} else {
		desc->defined_in=CORBA_string_dup("");
	}
	desc->version=CORBA_string_dup(servant->attr_version);
	desc->type=CORBA_IDLType__get_type(servant->attr_type_def, ev);
	CORBA_any__copy(&desc->value, servant->attr_value);

	retval->kind=CORBA_dk_Constant;
	retval->value._type=(CORBA_TypeCode)TC_CORBA_ConstantDescription;
	retval->value._value=desc;

	return(retval);
}

void impl_CORBA_ConstantDef_move(impl_POA_CORBA_ConstantDef * servant,
				 CORBA_Container new_container,
				 CORBA_Identifier new_name,
				 CORBA_VersionSpec new_version,
				 CORBA_Environment * ev)
{
	contained_move((IfaceRepoContents *)servant, new_container, new_name, new_version, "ConstantDef", &servant->attr_defined_in, ev);
}

static impl_POA_CORBA_TypedefDef *impl_CORBA_TypedefDef__create(
	PortableServer_POA poa,
	CORBA_Environment * ev)
{
	impl_POA_CORBA_TypedefDef *newservant;
	PortableServer_ObjectId *objid;

	newservant = g_new0(impl_POA_CORBA_TypedefDef, 1);
	newservant->servant.vepv = &impl_CORBA_TypedefDef_vepv;
	newservant->poa = poa;
	newservant->attr_name=NULL;
	newservant->attr_version=NULL;

	newservant->attr_def_kind=CORBA_dk_Typedef;

	POA_CORBA_TypedefDef__init((PortableServer_Servant) newservant, ev);
	objid = PortableServer_POA_activate_object(poa, newservant, ev);
	CORBA_free(objid);

	newservant->obj=PortableServer_POA_servant_to_reference(poa,
								newservant,
								ev);
	
	return(newservant);
}

/* You shouldn't call this routine directly without first deactivating the servant... */
static void impl_CORBA_TypedefDef__destroy(impl_POA_CORBA_TypedefDef * servant, CORBA_Environment * ev)
{

	POA_CORBA_TypedefDef__fini((PortableServer_Servant) servant, ev);
	g_free(servant);
}

CORBA_DefinitionKind
impl_CORBA_TypedefDef__get_def_kind(impl_POA_CORBA_TypedefDef * servant,
				    CORBA_Environment * ev)
{
	return(servant->attr_def_kind);
}

void impl_CORBA_TypedefDef_destroy(impl_POA_CORBA_TypedefDef * servant,
				   CORBA_Environment * ev)
{
	ORBit_Trace(TraceMod_IR, TraceLevel_Debug, "Destroying Typedef %s!\n",
		    servant->attr_id);
	
	/* Unlink from parent container */
	contained_remove_from_parent(&repo->contents, servant);
	
	PortableServer_POA_deactivate_object(servant->poa, PortableServer_POA_servant_to_id(servant->poa, servant, ev), ev);
	impl_CORBA_TypedefDef__destroy(servant, ev);
}

CORBA_RepositoryId
impl_CORBA_TypedefDef__get_id(impl_POA_CORBA_TypedefDef * servant,
			      CORBA_Environment * ev)
{
	return(CORBA_string_dup(servant->attr_id));
}

void impl_CORBA_TypedefDef__set_id(impl_POA_CORBA_TypedefDef * servant,
				   CORBA_RepositoryId value,
				   CORBA_Environment * ev)
{
	contained__set_id(&servant->attr_id, value, "TypedefDef");
}

CORBA_Identifier
impl_CORBA_TypedefDef__get_name(impl_POA_CORBA_TypedefDef * servant,
				CORBA_Environment * ev)
{
	return(CORBA_string_dup(servant->attr_name));
}

void impl_CORBA_TypedefDef__set_name(impl_POA_CORBA_TypedefDef * servant,
				     CORBA_Identifier value,
				     CORBA_Environment * ev)
{
	contained__set_name(&servant->attr_name, value, "TypedefDef");
}

CORBA_VersionSpec
impl_CORBA_TypedefDef__get_version(impl_POA_CORBA_TypedefDef * servant,
				   CORBA_Environment * ev)
{
	return(CORBA_string_dup(servant->attr_version));
}

void impl_CORBA_TypedefDef__set_version(impl_POA_CORBA_TypedefDef * servant,
					CORBA_VersionSpec value,
					CORBA_Environment * ev)
{
	contained__set_version(&servant->attr_version, value, "TypedefDef");
}

CORBA_Container
impl_CORBA_TypedefDef__get_defined_in(impl_POA_CORBA_TypedefDef * servant,
				      CORBA_Environment * ev)
{
	return(CORBA_Object_duplicate(servant->attr_defined_in, ev));
}

CORBA_ScopedName
impl_CORBA_TypedefDef__get_absolute_name(impl_POA_CORBA_TypedefDef * servant,
					 CORBA_Environment * ev)
{
	return(get_absolute_name(servant->attr_name, servant->attr_defined_in, ev));
}

CORBA_Repository
impl_CORBA_TypedefDef__get_containing_repository(impl_POA_CORBA_TypedefDef * servant,
						 CORBA_Environment * ev)
{
	return(CORBA_Object_duplicate(repo->obj, ev));
}

CORBA_Contained_Description *
 impl_CORBA_TypedefDef_describe(impl_POA_CORBA_TypedefDef * servant,
 				CORBA_Environment * ev)
{
	CORBA_Contained_Description *retval;
	CORBA_TypeDescription *desc;

	desc=CORBA_TypeDescription__alloc();
	if(desc==NULL) {
		return(CORBA_OBJECT_NIL);
	}
	
	retval=CORBA_Contained_Description__alloc();
	if(retval==NULL) {
		CORBA_free(desc);
		return(CORBA_OBJECT_NIL);
	}
	
	desc->name=CORBA_string_dup(servant->attr_name);
	desc->id=CORBA_string_dup(servant->attr_id);
	desc->defined_in=CORBA_string_dup(CORBA_Contained__get_id(servant->attr_defined_in, ev));
	desc->version=CORBA_string_dup(servant->attr_version);
	desc->type=servant->attr_type;
	
	retval->kind=CORBA_dk_Typedef;
	retval->value._type=(CORBA_TypeCode)TC_CORBA_TypeDescription;
	retval->value._value=desc;
	
	return(retval);
}

void impl_CORBA_TypedefDef_move(impl_POA_CORBA_TypedefDef * servant,
				CORBA_Container new_container,
				CORBA_Identifier new_name,
				CORBA_VersionSpec new_version,
				CORBA_Environment * ev)
{
}

CORBA_TypeCode
impl_CORBA_TypedefDef__get_type(impl_POA_CORBA_TypedefDef * servant,
				CORBA_Environment * ev)
{
	return(servant->attr_type);
}

static impl_POA_CORBA_StructDef *impl_CORBA_StructDef__create(
	PortableServer_POA poa,
	CORBA_RepositoryId id,
	CORBA_Identifier name,
	CORBA_VersionSpec version,
	CORBA_StructMemberSeq * members,
	CORBA_Container defined_in,
	CORBA_Environment * ev)
{
	impl_POA_CORBA_StructDef *newservant;
	PortableServer_ObjectId *objid;

	if(structmemberseq_verify(members)==CORBA_FALSE) {
		return(NULL);
	}
	
	newservant = g_new0(impl_POA_CORBA_StructDef, 1);
	newservant->servant.vepv = &impl_CORBA_StructDef_vepv;
	newservant->poa = poa;

	newservant->attr_members=structmemberseq_copy(members);
	newservant->attr_def_kind=CORBA_dk_Struct;
	newservant->attr_id=CORBA_string_dup(id);
	newservant->attr_name=CORBA_string_dup(name);
	newservant->attr_version=CORBA_string_dup(version);
	newservant->attr_defined_in=CORBA_Object_duplicate(defined_in, ev);
	
	newservant->contents=NULL;
	
	POA_CORBA_StructDef__init((PortableServer_Servant) newservant, ev);
	objid = PortableServer_POA_activate_object(poa, newservant, ev);
	CORBA_free(objid);

	newservant->obj=PortableServer_POA_servant_to_reference(poa,
								newservant,
								ev);
	
	g_hash_table_insert(repo->id_hash, g_strdup(id), newservant->obj);
	
	return(newservant);
}

/* You shouldn't call this routine directly without first deactivating the servant... */
static void impl_CORBA_StructDef__destroy(impl_POA_CORBA_StructDef * servant, CORBA_Environment * ev)
{

	g_slist_free(servant->contents);
	
	POA_CORBA_StructDef__fini((PortableServer_Servant) servant, ev);
	g_free(servant);
}

CORBA_StructMemberSeq *
 impl_CORBA_StructDef__get_members(impl_POA_CORBA_StructDef * servant,
				   CORBA_Environment * ev)
{
	CORBA_StructMemberSeq *retval;

	retval=structmemberseq_copy(servant->attr_members);
	if(retval==NULL) {
		return(CORBA_OBJECT_NIL);
	}

	return(retval);
}

void impl_CORBA_StructDef__set_members(impl_POA_CORBA_StructDef * servant,
				       CORBA_StructMemberSeq * value,
				       CORBA_Environment * ev)
{
	if(structmemberseq_verify(value)==CORBA_FALSE) {
		ORBit_Trace(TraceMod_IR, TraceLevel_Info,
			    "StructDef__set_members: bogus value!\n");
		return;
	}
	
	structmemberseq_free(servant->attr_members);
	servant->attr_members=structmemberseq_copy(value);
}

CORBA_DefinitionKind
impl_CORBA_StructDef__get_def_kind(impl_POA_CORBA_StructDef * servant,
				   CORBA_Environment * ev)
{
	return(servant->attr_def_kind);
}

void impl_CORBA_StructDef_destroy(impl_POA_CORBA_StructDef * servant,
				  CORBA_Environment * ev)
{
	GSList *list, *nextlist;
	
	ORBit_Trace(TraceMod_IR, TraceLevel_Debug, "Destroying Struct %s\n",
		    servant->attr_id);
	
	for(list=servant->contents; list; list=nextlist) {
		/* Recursively delete everything */

		nextlist=g_slist_next(list);	/* _destroy()ing the
                                                   contained items
                                                   fiddles with the
                                                   list we are
                                                   traversing */

		CORBA_IRObject_destroy(((IfaceRepoContents *)list->data)->obj,
				       ev);
	}
	
	/* Unlink from parent container */
	contained_remove_from_parent(&repo->contents, servant);
	
	PortableServer_POA_deactivate_object(servant->poa, PortableServer_POA_servant_to_id(servant->poa, servant, ev), ev);
	impl_CORBA_StructDef__destroy(servant, ev);
}

CORBA_RepositoryId
impl_CORBA_StructDef__get_id(impl_POA_CORBA_StructDef * servant,
			     CORBA_Environment * ev)
{
	return(CORBA_string_dup(servant->attr_id));
}

void impl_CORBA_StructDef__set_id(impl_POA_CORBA_StructDef * servant,
				  CORBA_RepositoryId value,
				  CORBA_Environment * ev)
{
	contained__set_id(&servant->attr_id, value, "StructDef");
}

CORBA_Identifier
impl_CORBA_StructDef__get_name(impl_POA_CORBA_StructDef * servant,
			       CORBA_Environment * ev)
{
	return(CORBA_string_dup(servant->attr_name));
}

void impl_CORBA_StructDef__set_name(impl_POA_CORBA_StructDef * servant,
				    CORBA_Identifier value,
				    CORBA_Environment * ev)
{
	contained__set_name(&servant->attr_name, value, "StructDef");
}

CORBA_VersionSpec
impl_CORBA_StructDef__get_version(impl_POA_CORBA_StructDef * servant,
				  CORBA_Environment * ev)
{
	return(CORBA_string_dup(servant->attr_version));
}

void impl_CORBA_StructDef__set_version(impl_POA_CORBA_StructDef * servant,
				       CORBA_VersionSpec value,
				       CORBA_Environment * ev)
{
	contained__set_version(&servant->attr_version, value, "StructDef");
}

CORBA_Container
impl_CORBA_StructDef__get_defined_in(impl_POA_CORBA_StructDef * servant,
				     CORBA_Environment * ev)
{
	return(CORBA_Object_duplicate(servant->attr_defined_in, ev));
}

CORBA_ScopedName
impl_CORBA_StructDef__get_absolute_name(impl_POA_CORBA_StructDef * servant,
					CORBA_Environment * ev)
{
	return(get_absolute_name(servant->attr_name, servant->attr_defined_in, ev));
}

CORBA_Repository
impl_CORBA_StructDef__get_containing_repository(impl_POA_CORBA_StructDef * servant,
						CORBA_Environment * ev)
{
	return(CORBA_Object_duplicate(repo->obj, ev));
}

CORBA_Contained_Description *
 impl_CORBA_StructDef_describe(impl_POA_CORBA_StructDef * servant,
			       CORBA_Environment * ev)
{
	CORBA_Contained_Description *retval;
	CORBA_TypeDescription *desc;	/* StructDef inherits from TypedefDef */

	desc=CORBA_TypeDescription__alloc();
	if(desc==NULL) {
		return(CORBA_OBJECT_NIL);
	}
	
	retval=CORBA_Contained_Description__alloc();
	if(retval==NULL) {
		CORBA_free(desc);
		return(CORBA_OBJECT_NIL);
	}
	
	desc->name=CORBA_string_dup(servant->attr_name);
	desc->id=CORBA_string_dup(servant->attr_id);
	desc->defined_in=CORBA_string_dup(CORBA_Contained__get_id(servant->attr_defined_in, ev));
	desc->version=CORBA_string_dup(servant->attr_version);
	desc->type=servant->attr_type;
	
	retval->kind=CORBA_dk_Struct;
	retval->value._type=(CORBA_TypeCode)TC_CORBA_TypeDescription;
	retval->value._value=desc;
	
	return(retval);
}

void impl_CORBA_StructDef_move(impl_POA_CORBA_StructDef * servant,
			       CORBA_Container new_container,
			       CORBA_Identifier new_name,
			       CORBA_VersionSpec new_version,
			       CORBA_Environment * ev)
{
	contained_move((IfaceRepoContents *)servant, new_container, new_name, new_version, "StructDef", &servant->attr_defined_in, ev);
}

CORBA_TypeCode
impl_CORBA_StructDef__get_type(impl_POA_CORBA_StructDef * servant,
			       CORBA_Environment * ev)
{
	return(servant->attr_type);
}

CORBA_Contained
impl_CORBA_StructDef_lookup(impl_POA_CORBA_StructDef * servant,
			    CORBA_ScopedName search_name,
			    CORBA_Environment * ev)
{
	return(container_lookup(servant->contents, search_name, "StructDef", ev));
}

CORBA_ContainedSeq *
 impl_CORBA_StructDef_contents(impl_POA_CORBA_StructDef * servant,
			       CORBA_DefinitionKind limit_type,
			       CORBA_boolean exclude_inherited,
			       CORBA_Environment * ev)
{
	return(container_contents(servant->contents, limit_type, exclude_inherited, "StructDef", ev));
}

CORBA_ContainedSeq *
 impl_CORBA_StructDef_lookup_name(impl_POA_CORBA_StructDef * servant,
				  CORBA_Identifier search_name,
				  CORBA_long levels_to_search,
				  CORBA_DefinitionKind limit_type,
				  CORBA_boolean exclude_inherited,
				  CORBA_Environment * ev)
{
	return(container_lookup_name(servant->contents, search_name, levels_to_search, limit_type, exclude_inherited, "StructDef", ev));
}

CORBA_Container_DescriptionSeq *
 impl_CORBA_StructDef_describe_contents(impl_POA_CORBA_StructDef * servant,
					CORBA_DefinitionKind limit_type,
					CORBA_boolean exclude_inherited,
					CORBA_long max_returned_objs,
					CORBA_Environment * ev)
{
	return(container_describe_contents(servant->contents, limit_type, exclude_inherited, max_returned_objs, "StructDef", ev));
}

CORBA_ModuleDef
impl_CORBA_StructDef_create_module(impl_POA_CORBA_StructDef * servant,
				   CORBA_RepositoryId id,
				   CORBA_Identifier name,
				   CORBA_VersionSpec version,
				   CORBA_Environment * ev)
{
	/* Structs can only contain structs, unions and
           enums. Unfortunately, the spec doesn't specify any
           exceptions, so I will just ignore the request for being
           silly */
	return(CORBA_OBJECT_NIL);
}

CORBA_ConstantDef
impl_CORBA_StructDef_create_constant(impl_POA_CORBA_StructDef * servant,
				     CORBA_RepositoryId id,
				     CORBA_Identifier name,
				     CORBA_VersionSpec version,
				     CORBA_IDLType type,
				     CORBA_any * value,
				     CORBA_Environment * ev)
{
	/* Structs can only contain structs, unions and
           enums. Unfortunately, the spec doesn't specify any
           exceptions, so I will just ignore the request for being
           silly */
	return(CORBA_OBJECT_NIL);
}

CORBA_StructDef
impl_CORBA_StructDef_create_struct(impl_POA_CORBA_StructDef * servant,
				   CORBA_RepositoryId id,
				   CORBA_Identifier name,
				   CORBA_VersionSpec version,
				   CORBA_StructMemberSeq * members,
				   CORBA_Environment * ev)
{
	return(container_create_struct(&servant->contents, servant->poa, id, name, version, members, servant->obj, "StructDef", ev));
}

CORBA_UnionDef
impl_CORBA_StructDef_create_union(impl_POA_CORBA_StructDef * servant,
				  CORBA_RepositoryId id,
				  CORBA_Identifier name,
				  CORBA_VersionSpec version,
				  CORBA_IDLType discriminator_type,
				  CORBA_UnionMemberSeq * members,
				  CORBA_Environment * ev)
{
	return(container_create_union(&servant->contents, servant->poa, id, name, version, discriminator_type, members, servant->obj, "StructDef", ev));
}

CORBA_EnumDef
impl_CORBA_StructDef_create_enum(impl_POA_CORBA_StructDef * servant,
				 CORBA_RepositoryId id,
				 CORBA_Identifier name,
				 CORBA_VersionSpec version,
				 CORBA_EnumMemberSeq * members,
				 CORBA_Environment * ev)
{
	return(container_create_enum(&servant->contents, servant->poa, id, name, version, members, servant->obj, "StructDef", ev));
}

CORBA_AliasDef
impl_CORBA_StructDef_create_alias(impl_POA_CORBA_StructDef * servant,
				  CORBA_RepositoryId id,
				  CORBA_Identifier name,
				  CORBA_VersionSpec version,
				  CORBA_IDLType original_type,
				  CORBA_Environment * ev)
{
	/* Structs can only contain structs, unions and
           enums. Unfortunately, the spec doesn't specify any
           exceptions, so I will just ignore the request for being
           silly */
	return(CORBA_OBJECT_NIL);
}

CORBA_InterfaceDef
impl_CORBA_StructDef_create_interface(impl_POA_CORBA_StructDef * servant,
				      CORBA_RepositoryId id,
				      CORBA_Identifier name,
				      CORBA_VersionSpec version,
				      CORBA_InterfaceDefSeq * base_interfaces,
				      CORBA_Environment * ev)
{
	/* Structs can only contain structs, unions and
           enums. Unfortunately, the spec doesn't specify any
           exceptions, so I will just ignore the request for being
           silly */
	return(CORBA_OBJECT_NIL);
}

CORBA_ExceptionDef
impl_CORBA_StructDef_create_exception(impl_POA_CORBA_StructDef * servant,
				      CORBA_RepositoryId id,
				      CORBA_Identifier name,
				      CORBA_VersionSpec version,
				      CORBA_StructMemberSeq * members,
				      CORBA_Environment * ev)
{
	/* Structs can only contain structs, unions and
           enums. Unfortunately, the spec doesn't specify any
           exceptions, so I will just ignore the request for being
           silly */
	return(CORBA_OBJECT_NIL);
}

static impl_POA_CORBA_UnionDef *impl_CORBA_UnionDef__create(
	PortableServer_POA poa,
	CORBA_RepositoryId id,
	CORBA_Identifier name,
	CORBA_VersionSpec version,
	CORBA_IDLType discriminator_type,
	CORBA_UnionMemberSeq * members,
	CORBA_Container defined_in,
	CORBA_Environment * ev)
{
	impl_POA_CORBA_UnionDef *newservant;
	PortableServer_ObjectId *objid;

	if(unionmemberseq_verify(members)==CORBA_FALSE) {
		return(NULL);
	}
	
	newservant = g_new0(impl_POA_CORBA_UnionDef, 1);
	newservant->servant.vepv = &impl_CORBA_UnionDef_vepv;
	newservant->poa = poa;

	newservant->attr_members=unionmemberseq_copy(members);
	newservant->attr_discriminator_type_def=CORBA_Object_duplicate(discriminator_type, ev);
	
	newservant->attr_def_kind=CORBA_dk_Union;
	newservant->attr_id=CORBA_string_dup(id);
	newservant->attr_name=CORBA_string_dup(name);
	newservant->attr_version=CORBA_string_dup(version);
	newservant->attr_defined_in=CORBA_Object_duplicate(defined_in, ev);

	newservant->contents=NULL;
	
	POA_CORBA_UnionDef__init((PortableServer_Servant) newservant, ev);
	objid = PortableServer_POA_activate_object(poa, newservant, ev);
	CORBA_free(objid);

	newservant->obj=PortableServer_POA_servant_to_reference(poa,
								newservant,
								ev);
	
	g_hash_table_insert(repo->id_hash, g_strdup(id), newservant->obj);
	
	return(newservant);
}

/* You shouldn't call this routine directly without first deactivating the servant... */
static void impl_CORBA_UnionDef__destroy(impl_POA_CORBA_UnionDef * servant, CORBA_Environment * ev)
{

	g_slist_free(servant->contents);
	
	POA_CORBA_UnionDef__fini((PortableServer_Servant) servant, ev);
	g_free(servant);
}

CORBA_TypeCode
impl_CORBA_UnionDef__get_discriminator_type(impl_POA_CORBA_UnionDef * servant,
					    CORBA_Environment * ev)
{
	return(servant->attr_discriminator_type);
}

CORBA_IDLType
impl_CORBA_UnionDef__get_discriminator_type_def(impl_POA_CORBA_UnionDef * servant,
						CORBA_Environment * ev)
{
	return(CORBA_Object_duplicate(servant->attr_discriminator_type_def, ev));
}

void impl_CORBA_UnionDef__set_discriminator_type_def(impl_POA_CORBA_UnionDef * servant,
						     CORBA_IDLType value,
						     CORBA_Environment * ev)
{
	if(servant->attr_discriminator_type_def!=CORBA_OBJECT_NIL) {
		CORBA_Object_release(servant->attr_discriminator_type_def, ev);
	}
	servant->attr_discriminator_type_def=CORBA_Object_duplicate(value, ev);
}

CORBA_UnionMemberSeq *
 impl_CORBA_UnionDef__get_members(impl_POA_CORBA_UnionDef * servant,
				  CORBA_Environment * ev)
{
	CORBA_UnionMemberSeq *retval;

	retval=unionmemberseq_copy(servant->attr_members);
	if(retval==NULL) {
		return(CORBA_OBJECT_NIL);
	}
	
	return retval;
}

void impl_CORBA_UnionDef__set_members(impl_POA_CORBA_UnionDef * servant,
				      CORBA_UnionMemberSeq * value,
				      CORBA_Environment * ev)
{
	if(unionmemberseq_verify(value)==CORBA_FALSE) {
		ORBit_Trace(TraceMod_IR, TraceLevel_Info,
			    "UnionDef__set_members: bogus value!\n");
		return;
	}
	
	unionmemberseq_free(servant->attr_members);
	servant->attr_members=unionmemberseq_copy(value);
}

CORBA_DefinitionKind
impl_CORBA_UnionDef__get_def_kind(impl_POA_CORBA_UnionDef * servant,
				  CORBA_Environment * ev)
{
	return(servant->attr_def_kind);
}

void impl_CORBA_UnionDef_destroy(impl_POA_CORBA_UnionDef * servant,
				 CORBA_Environment * ev)
{
	GSList *list, *nextlist;
	
	ORBit_Trace(TraceMod_IR, TraceLevel_Debug, "Destroying Union %s\n",
		    servant->attr_id);
	
	for(list=servant->contents; list; list=nextlist) {
		/* Recursively delete everything */

		nextlist=g_slist_next(list);	/* _destroy()ing the
                                                   contained items
                                                   fiddles with the
                                                   list we are
                                                   traversing */

		CORBA_IRObject_destroy(((IfaceRepoContents *)list->data)->obj,
				       ev);
	}
	
	/* Unlink from parent container */
	contained_remove_from_parent(&repo->contents, servant);
	
	PortableServer_POA_deactivate_object(servant->poa, PortableServer_POA_servant_to_id(servant->poa, servant, ev), ev);
	impl_CORBA_UnionDef__destroy(servant, ev);
}

CORBA_RepositoryId
impl_CORBA_UnionDef__get_id(impl_POA_CORBA_UnionDef * servant,
			    CORBA_Environment * ev)
{
	return(CORBA_string_dup(servant->attr_id));
}

void impl_CORBA_UnionDef__set_id(impl_POA_CORBA_UnionDef * servant,
				 CORBA_RepositoryId value,
				 CORBA_Environment * ev)
{
	contained__set_id(&servant->attr_id, value, "UnionDef");
}

CORBA_Identifier
impl_CORBA_UnionDef__get_name(impl_POA_CORBA_UnionDef * servant,
			      CORBA_Environment * ev)
{
	return(CORBA_string_dup(servant->attr_name));
}

void impl_CORBA_UnionDef__set_name(impl_POA_CORBA_UnionDef * servant,
				   CORBA_Identifier value,
				   CORBA_Environment * ev)
{
	contained__set_name(&servant->attr_name, value, "UnionDef");
}

CORBA_VersionSpec
impl_CORBA_UnionDef__get_version(impl_POA_CORBA_UnionDef * servant,
				 CORBA_Environment * ev)
{
	return(CORBA_string_dup(servant->attr_version));
}

void impl_CORBA_UnionDef__set_version(impl_POA_CORBA_UnionDef * servant,
				      CORBA_VersionSpec value,
				      CORBA_Environment * ev)
{
	contained__set_version(&servant->attr_version, value, "UnionDef");
}

CORBA_Container
impl_CORBA_UnionDef__get_defined_in(impl_POA_CORBA_UnionDef * servant,
				    CORBA_Environment * ev)
{
	return(CORBA_Object_duplicate(servant->attr_defined_in, ev));
}

CORBA_ScopedName
impl_CORBA_UnionDef__get_absolute_name(impl_POA_CORBA_UnionDef * servant,
				       CORBA_Environment * ev)
{
	return(get_absolute_name(servant->attr_name, servant->attr_defined_in, ev));
}

CORBA_Repository
impl_CORBA_UnionDef__get_containing_repository(impl_POA_CORBA_UnionDef * servant,
					       CORBA_Environment * ev)
{
	return(CORBA_Object_duplicate(repo->obj, ev));
}

CORBA_Contained_Description *
 impl_CORBA_UnionDef_describe(impl_POA_CORBA_UnionDef * servant,
			      CORBA_Environment * ev)
{
	CORBA_Contained_Description *retval;
	CORBA_TypeDescription *desc;	/* UnionDef inherits from TypedefDef */

	desc=CORBA_TypeDescription__alloc();
	if(desc==NULL) {
		return(CORBA_OBJECT_NIL);
	}
	
	retval=CORBA_Contained_Description__alloc();
	if(retval==NULL) {
		CORBA_free(desc);
		return(CORBA_OBJECT_NIL);
	}
	
	desc->name=CORBA_string_dup(servant->attr_name);
	desc->id=CORBA_string_dup(servant->attr_id);
	desc->defined_in=CORBA_string_dup(CORBA_Contained__get_id(servant->attr_defined_in, ev));
	desc->version=CORBA_string_dup(servant->attr_version);
	desc->type=servant->attr_type;
	
	retval->kind=CORBA_dk_Union;
	retval->value._type=(CORBA_TypeCode)TC_CORBA_TypeDescription;
	retval->value._value=desc;
	
	return(retval);
}

void impl_CORBA_UnionDef_move(impl_POA_CORBA_UnionDef * servant,
			      CORBA_Container new_container,
			      CORBA_Identifier new_name,
			      CORBA_VersionSpec new_version,
			      CORBA_Environment * ev)
{
	contained_move((IfaceRepoContents *)servant, new_container, new_name, new_version, "UnionDef", &servant->attr_defined_in, ev);
}

CORBA_TypeCode
impl_CORBA_UnionDef__get_type(impl_POA_CORBA_UnionDef * servant,
			      CORBA_Environment * ev)
{
	return(servant->attr_type);
}

CORBA_Contained
impl_CORBA_UnionDef_lookup(impl_POA_CORBA_UnionDef * servant,
			   CORBA_ScopedName search_name,
			   CORBA_Environment * ev)
{
	return(container_lookup(servant->contents, search_name, "UnionDef", ev));
}

CORBA_ContainedSeq *
 impl_CORBA_UnionDef_contents(impl_POA_CORBA_UnionDef * servant,
			      CORBA_DefinitionKind limit_type,
			      CORBA_boolean exclude_inherited,
			      CORBA_Environment * ev)
{
	return(container_contents(servant->contents, limit_type, exclude_inherited, "UnionDef", ev));
}

CORBA_ContainedSeq *
 impl_CORBA_UnionDef_lookup_name(impl_POA_CORBA_UnionDef * servant,
				 CORBA_Identifier search_name,
				 CORBA_long levels_to_search,
				 CORBA_DefinitionKind limit_type,
				 CORBA_boolean exclude_inherited,
				 CORBA_Environment * ev)
{
	return(container_lookup_name(servant->contents, search_name, levels_to_search, limit_type, exclude_inherited, "UnionDef", ev));
}

CORBA_Container_DescriptionSeq *
 impl_CORBA_UnionDef_describe_contents(impl_POA_CORBA_UnionDef * servant,
				       CORBA_DefinitionKind limit_type,
				       CORBA_boolean exclude_inherited,
				       CORBA_long max_returned_objs,
				       CORBA_Environment * ev)
{
	return(container_describe_contents(servant->contents, limit_type, exclude_inherited, max_returned_objs, "UnionDef", ev));
}

CORBA_ModuleDef
impl_CORBA_UnionDef_create_module(impl_POA_CORBA_UnionDef * servant,
				  CORBA_RepositoryId id,
				  CORBA_Identifier name,
				  CORBA_VersionSpec version,
				  CORBA_Environment * ev)
{
	/* Unions can only contain structs, unions and
           enums. Unfortunately, the spec doesn't specify any
           exceptions, so I will just ignore the request for being
           silly */
	return(CORBA_OBJECT_NIL);
}

CORBA_ConstantDef
impl_CORBA_UnionDef_create_constant(impl_POA_CORBA_UnionDef * servant,
				    CORBA_RepositoryId id,
				    CORBA_Identifier name,
				    CORBA_VersionSpec version,
				    CORBA_IDLType type,
				    CORBA_any * value,
				    CORBA_Environment * ev)
{
	/* Unions can only contain structs, unions and
           enums. Unfortunately, the spec doesn't specify any
           exceptions, so I will just ignore the request for being
           silly */
	return(CORBA_OBJECT_NIL);
}

CORBA_StructDef
impl_CORBA_UnionDef_create_struct(impl_POA_CORBA_UnionDef * servant,
				  CORBA_RepositoryId id,
				  CORBA_Identifier name,
				  CORBA_VersionSpec version,
				  CORBA_StructMemberSeq * members,
				  CORBA_Environment * ev)
{
	return(container_create_struct(&servant->contents, servant->poa, id, name, version, members, servant->obj, "UnionDef", ev));
}

CORBA_UnionDef
impl_CORBA_UnionDef_create_union(impl_POA_CORBA_UnionDef * servant,
				 CORBA_RepositoryId id,
				 CORBA_Identifier name,
				 CORBA_VersionSpec version,
				 CORBA_IDLType discriminator_type,
				 CORBA_UnionMemberSeq * members,
				 CORBA_Environment * ev)
{
	return(container_create_union(&servant->contents, servant->poa, id, name, version, discriminator_type, members, servant->obj, "UnionDef", ev));
}

CORBA_EnumDef
impl_CORBA_UnionDef_create_enum(impl_POA_CORBA_UnionDef * servant,
				CORBA_RepositoryId id,
				CORBA_Identifier name,
				CORBA_VersionSpec version,
				CORBA_EnumMemberSeq * members,
				CORBA_Environment * ev)
{
	return(container_create_enum(&servant->contents, servant->poa, id, name, version, members, servant->obj, "UnionDef", ev));
}

CORBA_AliasDef
impl_CORBA_UnionDef_create_alias(impl_POA_CORBA_UnionDef * servant,
				 CORBA_RepositoryId id,
				 CORBA_Identifier name,
				 CORBA_VersionSpec version,
				 CORBA_IDLType original_type,
				 CORBA_Environment * ev)
{
	/* Unions can only contain structs, unions and
           enums. Unfortunately, the spec doesn't specify any
           exceptions, so I will just ignore the request for being
           silly */
	return(CORBA_OBJECT_NIL);
}

CORBA_InterfaceDef
impl_CORBA_UnionDef_create_interface(impl_POA_CORBA_UnionDef * servant,
				     CORBA_RepositoryId id,
				     CORBA_Identifier name,
				     CORBA_VersionSpec version,
				     CORBA_InterfaceDefSeq * base_interfaces,
				     CORBA_Environment * ev)
{
	/* Unions can only contain structs, unions and
           enums. Unfortunately, the spec doesn't specify any
           exceptions, so I will just ignore the request for being
           silly */
	return(CORBA_OBJECT_NIL);
}

CORBA_ExceptionDef
impl_CORBA_UnionDef_create_exception(impl_POA_CORBA_UnionDef * servant,
				     CORBA_RepositoryId id,
				     CORBA_Identifier name,
				     CORBA_VersionSpec version,
				     CORBA_StructMemberSeq * members,
				     CORBA_Environment * ev)
{
	/* Unions can only contain structs, unions and
           enums. Unfortunately, the spec doesn't specify any
           exceptions, so I will just ignore the request for being
           silly */
	return(CORBA_OBJECT_NIL);
}

static impl_POA_CORBA_EnumDef *impl_CORBA_EnumDef__create(
	PortableServer_POA poa,
	CORBA_RepositoryId id,
	CORBA_Identifier name,
	CORBA_VersionSpec version,
	CORBA_EnumMemberSeq * members,
	CORBA_Container defined_in,
	CORBA_Environment * ev)
{
	impl_POA_CORBA_EnumDef *newservant;
	PortableServer_ObjectId *objid;

	if(enummemberseq_verify(members)==CORBA_FALSE) {
		return(NULL);
	}
	
	newservant = g_new0(impl_POA_CORBA_EnumDef, 1);
	newservant->servant.vepv = &impl_CORBA_EnumDef_vepv;
	newservant->poa = poa;

	newservant->attr_members=enummemberseq_copy(members);
	
	newservant->attr_def_kind=CORBA_dk_Enum;
	newservant->attr_id=CORBA_string_dup(id);
	newservant->attr_name=CORBA_string_dup(name);
	newservant->attr_version=CORBA_string_dup(version);
	newservant->attr_defined_in=CORBA_Object_duplicate(defined_in, ev);

	POA_CORBA_EnumDef__init((PortableServer_Servant) newservant, ev);
	objid = PortableServer_POA_activate_object(poa, newservant, ev);
	CORBA_free(objid);

	newservant->obj=PortableServer_POA_servant_to_reference(poa,
								newservant,
								ev);
	
	g_hash_table_insert(repo->id_hash, g_strdup(id), newservant->obj);
	
	return(newservant);
}

/* You shouldn't call this routine directly without first deactivating the servant... */
static void impl_CORBA_EnumDef__destroy(impl_POA_CORBA_EnumDef * servant, CORBA_Environment * ev)
{

	POA_CORBA_EnumDef__fini((PortableServer_Servant) servant, ev);
	g_free(servant);
}

CORBA_EnumMemberSeq *
 impl_CORBA_EnumDef__get_members(impl_POA_CORBA_EnumDef * servant,
				 CORBA_Environment * ev)
{
	CORBA_EnumMemberSeq *retval;

	retval=enummemberseq_copy(servant->attr_members);
	if(retval==NULL) {
		return(CORBA_OBJECT_NIL);
	}
	
	return(retval);
}

void impl_CORBA_EnumDef__set_members(impl_POA_CORBA_EnumDef * servant,
				     CORBA_EnumMemberSeq * value,
				     CORBA_Environment * ev)
{
	if(enummemberseq_verify(value)==CORBA_FALSE) {
		ORBit_Trace(TraceMod_IR, TraceLevel_Info,
			    "EnumDef__set_members: bogus value!\n");
		return;
	}
	
	enummemberseq_free(servant->attr_members);
	servant->attr_members=enummemberseq_copy(value);
}

CORBA_DefinitionKind
impl_CORBA_EnumDef__get_def_kind(impl_POA_CORBA_EnumDef * servant,
				 CORBA_Environment * ev)
{
	return(servant->attr_def_kind);
}

void impl_CORBA_EnumDef_destroy(impl_POA_CORBA_EnumDef * servant,
				CORBA_Environment * ev)
{
	ORBit_Trace(TraceMod_IR, TraceLevel_Debug, "Destroying Enum %s!\n",
		    servant->attr_id);
	
	/* Unlink from parent container */
	contained_remove_from_parent(&repo->contents, servant);
	
	PortableServer_POA_deactivate_object(servant->poa, PortableServer_POA_servant_to_id(servant->poa, servant, ev), ev);
	impl_CORBA_EnumDef__destroy(servant, ev);
}

CORBA_RepositoryId
impl_CORBA_EnumDef__get_id(impl_POA_CORBA_EnumDef * servant,
			   CORBA_Environment * ev)
{
	return(CORBA_string_dup(servant->attr_id));
}

void impl_CORBA_EnumDef__set_id(impl_POA_CORBA_EnumDef * servant,
				CORBA_RepositoryId value,
				CORBA_Environment * ev)
{
	contained__set_id(&servant->attr_id, value, "EnumDef");
}

CORBA_Identifier
impl_CORBA_EnumDef__get_name(impl_POA_CORBA_EnumDef * servant,
			     CORBA_Environment * ev)
{
	return(CORBA_string_dup(servant->attr_name));
}

void impl_CORBA_EnumDef__set_name(impl_POA_CORBA_EnumDef * servant,
				  CORBA_Identifier value,
				  CORBA_Environment * ev)
{
	contained__set_name(&servant->attr_name, value, "EnumDef");
}

CORBA_VersionSpec
impl_CORBA_EnumDef__get_version(impl_POA_CORBA_EnumDef * servant,
				CORBA_Environment * ev)
{
	return(CORBA_string_dup(servant->attr_version));
}

void impl_CORBA_EnumDef__set_version(impl_POA_CORBA_EnumDef * servant,
				     CORBA_VersionSpec value,
				     CORBA_Environment * ev)
{
	contained__set_version(&servant->attr_version, value, "EnumDef");
}

CORBA_Container
impl_CORBA_EnumDef__get_defined_in(impl_POA_CORBA_EnumDef * servant,
				   CORBA_Environment * ev)
{
	return(CORBA_Object_duplicate(servant->attr_defined_in, ev));
}

CORBA_ScopedName
impl_CORBA_EnumDef__get_absolute_name(impl_POA_CORBA_EnumDef * servant,
				      CORBA_Environment * ev)
{
	return(get_absolute_name(servant->attr_name, servant->attr_defined_in, ev));
}

CORBA_Repository
impl_CORBA_EnumDef__get_containing_repository(impl_POA_CORBA_EnumDef * servant,
					      CORBA_Environment * ev)
{
	return(CORBA_Object_duplicate(repo->obj, ev));
}

CORBA_Contained_Description *
 impl_CORBA_EnumDef_describe(impl_POA_CORBA_EnumDef * servant,
			     CORBA_Environment * ev)
{
	CORBA_Contained_Description *retval;
	CORBA_TypeDescription *desc;	/* EnumDef inherits from TypedefDef */

	desc=CORBA_TypeDescription__alloc();
	if(desc==NULL) {
		return(CORBA_OBJECT_NIL);
	}
	
	retval=CORBA_Contained_Description__alloc();
	if(retval==NULL) {
		CORBA_free(desc);
		return(CORBA_OBJECT_NIL);
	}
	
	desc->name=CORBA_string_dup(servant->attr_name);
	desc->id=CORBA_string_dup(servant->attr_id);
	desc->defined_in=CORBA_string_dup(CORBA_Contained__get_id(servant->attr_defined_in, ev));
	desc->version=CORBA_string_dup(servant->attr_version);
	desc->type=servant->attr_type;
	
	retval->kind=CORBA_dk_Enum;
	retval->value._type=(CORBA_TypeCode)TC_CORBA_TypeDescription;
	retval->value._value=desc;
	
	return(retval);
}

void impl_CORBA_EnumDef_move(impl_POA_CORBA_EnumDef * servant,
			     CORBA_Container new_container,
			     CORBA_Identifier new_name,
			     CORBA_VersionSpec new_version,
			     CORBA_Environment * ev)
{
	contained_move((IfaceRepoContents *)servant, new_container, new_name, new_version, "EnumDef", &servant->attr_defined_in, ev);
}

CORBA_TypeCode
impl_CORBA_EnumDef__get_type(impl_POA_CORBA_EnumDef * servant,
			     CORBA_Environment * ev)
{
	return(servant->attr_type);
}

static impl_POA_CORBA_AliasDef *impl_CORBA_AliasDef__create(
	PortableServer_POA poa, 
	CORBA_RepositoryId id,
	CORBA_Identifier name,
	CORBA_VersionSpec version,
	CORBA_IDLType original_type,
	CORBA_Container defined_in,
	CORBA_Environment * ev)
{
	impl_POA_CORBA_AliasDef *newservant;
	PortableServer_ObjectId *objid;

	newservant = g_new0(impl_POA_CORBA_AliasDef, 1);
	newservant->servant.vepv = &impl_CORBA_AliasDef_vepv;
	newservant->poa = poa;

	newservant->attr_def_kind=CORBA_dk_Alias;
	newservant->attr_id=CORBA_string_dup(id);
	newservant->attr_name=CORBA_string_dup(name);
	newservant->attr_version=CORBA_string_dup(version);
	newservant->attr_original_type_def=CORBA_Object_duplicate(original_type, ev);
	newservant->attr_defined_in=CORBA_Object_duplicate(defined_in, ev);

	POA_CORBA_AliasDef__init((PortableServer_Servant) newservant, ev);
	objid = PortableServer_POA_activate_object(poa, newservant, ev);
	CORBA_free(objid);

	newservant->obj=PortableServer_POA_servant_to_reference(poa,
								newservant,
								ev);
	
	g_hash_table_insert(repo->id_hash, g_strdup(id), newservant->obj);
	
	return(newservant);
}

/* You shouldn't call this routine directly without first deactivating the servant... */
static void impl_CORBA_AliasDef__destroy(impl_POA_CORBA_AliasDef * servant, CORBA_Environment * ev)
{

	POA_CORBA_AliasDef__fini((PortableServer_Servant) servant, ev);
	g_free(servant);
}

CORBA_IDLType
impl_CORBA_AliasDef__get_original_type_def(impl_POA_CORBA_AliasDef * servant,
					   CORBA_Environment * ev)
{
	return(CORBA_Object_duplicate(servant->attr_original_type_def, ev));
}

void impl_CORBA_AliasDef__set_original_type_def(impl_POA_CORBA_AliasDef * servant,
						CORBA_IDLType value,
						CORBA_Environment * ev)
{
	if(servant->attr_original_type_def!=CORBA_OBJECT_NIL) {
		CORBA_Object_release(servant->attr_original_type_def, ev);
	}
	servant->attr_original_type_def=CORBA_Object_duplicate(value, ev);
}

CORBA_DefinitionKind
impl_CORBA_AliasDef__get_def_kind(impl_POA_CORBA_AliasDef * servant,
				  CORBA_Environment * ev)
{
	return(servant->attr_def_kind);
}

void impl_CORBA_AliasDef_destroy(impl_POA_CORBA_AliasDef * servant,
				 CORBA_Environment * ev)
{
	ORBit_Trace(TraceMod_IR, TraceLevel_Debug, "Destroying Alias %s!\n",
		    servant->attr_id);
	
	/* Unlink from parent container */
	contained_remove_from_parent(&repo->contents, servant);
	
	PortableServer_POA_deactivate_object(servant->poa, PortableServer_POA_servant_to_id(servant->poa, servant, ev), ev);
	impl_CORBA_AliasDef__destroy(servant, ev);
}

CORBA_RepositoryId
impl_CORBA_AliasDef__get_id(impl_POA_CORBA_AliasDef * servant,
			    CORBA_Environment * ev)
{
	return(CORBA_string_dup(servant->attr_id));
}

void impl_CORBA_AliasDef__set_id(impl_POA_CORBA_AliasDef * servant,
				 CORBA_RepositoryId value,
				 CORBA_Environment * ev)
{
	contained__set_id(&servant->attr_id, value, "AliasDef");
}

CORBA_Identifier
impl_CORBA_AliasDef__get_name(impl_POA_CORBA_AliasDef * servant,
			      CORBA_Environment * ev)
{
	return(CORBA_string_dup(servant->attr_name));
}

void impl_CORBA_AliasDef__set_name(impl_POA_CORBA_AliasDef * servant,
				   CORBA_Identifier value,
				   CORBA_Environment * ev)
{
	contained__set_name(&servant->attr_name, value, "AliasDef");
}

CORBA_VersionSpec
impl_CORBA_AliasDef__get_version(impl_POA_CORBA_AliasDef * servant,
				 CORBA_Environment * ev)
{
	return(CORBA_string_dup(servant->attr_version));
}

void impl_CORBA_AliasDef__set_version(impl_POA_CORBA_AliasDef * servant,
				      CORBA_VersionSpec value,
				      CORBA_Environment * ev)
{
	contained__set_version(&servant->attr_version, value, "AliasDef");
}

CORBA_Container
impl_CORBA_AliasDef__get_defined_in(impl_POA_CORBA_AliasDef * servant,
				    CORBA_Environment * ev)
{
	return(CORBA_Object_duplicate(servant->attr_defined_in, ev));
}

CORBA_ScopedName
impl_CORBA_AliasDef__get_absolute_name(impl_POA_CORBA_AliasDef * servant,
				       CORBA_Environment * ev)
{
	return(get_absolute_name(servant->attr_name, servant->attr_defined_in, ev));
}

CORBA_Repository
impl_CORBA_AliasDef__get_containing_repository(impl_POA_CORBA_AliasDef * servant,
					       CORBA_Environment * ev)
{
	return(CORBA_Object_duplicate(repo->obj, ev));
}

CORBA_Contained_Description *
 impl_CORBA_AliasDef_describe(impl_POA_CORBA_AliasDef * servant,
			      CORBA_Environment * ev)
{
	CORBA_Contained_Description *retval;
	CORBA_TypeDescription *desc;	/* AliasDef inherits from TypedefDef */

	desc=CORBA_TypeDescription__alloc();
	if(desc==NULL) {
		return(CORBA_OBJECT_NIL);
	}
	
	retval=CORBA_Contained_Description__alloc();
	if(retval==NULL) {
		CORBA_free(desc);
		return(CORBA_OBJECT_NIL);
	}
	
	desc->name=CORBA_string_dup(servant->attr_name);
	desc->id=CORBA_string_dup(servant->attr_id);
	desc->defined_in=CORBA_string_dup(CORBA_Contained__get_id(servant->attr_defined_in, ev));
	desc->version=CORBA_string_dup(servant->attr_version);
	desc->type=servant->attr_type;
	
	retval->kind=CORBA_dk_Alias;
	retval->value._type=(CORBA_TypeCode)TC_CORBA_TypeDescription;
	retval->value._value=desc;
	
	return(retval);
}

void impl_CORBA_AliasDef_move(impl_POA_CORBA_AliasDef * servant,
			      CORBA_Container new_container,
			      CORBA_Identifier new_name,
			      CORBA_VersionSpec new_version,
			      CORBA_Environment * ev)
{
	contained_move((IfaceRepoContents *)servant, new_container, new_name, new_version, "AliasDef", &servant->attr_defined_in, ev);
}

CORBA_TypeCode
impl_CORBA_AliasDef__get_type(impl_POA_CORBA_AliasDef * servant,
			      CORBA_Environment * ev)
{
	return(servant->attr_type);
}

static impl_POA_CORBA_PrimitiveDef *impl_CORBA_PrimitiveDef__create(
	PortableServer_POA poa,
	CORBA_PrimitiveKind kind,
	CORBA_Environment * ev)
{
	impl_POA_CORBA_PrimitiveDef *newservant;
	PortableServer_ObjectId *objid;

	newservant = g_new0(impl_POA_CORBA_PrimitiveDef, 1);
	newservant->servant.vepv = &impl_CORBA_PrimitiveDef_vepv;
	newservant->poa = poa;

	newservant->attr_kind=kind;
	newservant->attr_def_kind=CORBA_dk_Primitive;
	newservant->attr_name=NULL;
	newservant->attr_version=NULL;

	POA_CORBA_PrimitiveDef__init((PortableServer_Servant) newservant, ev);
	objid = PortableServer_POA_activate_object(poa, newservant, ev);
	CORBA_free(objid);

	newservant->obj=PortableServer_POA_servant_to_reference(poa,
								newservant,
								ev);
	
	/* Add this primitive to the non-containeds list */
	repo->noncontaineds=g_slist_append(repo->noncontaineds, newservant);
	
	return(newservant);
}

/* You shouldn't call this routine directly without first deactivating the servant... */
static void impl_CORBA_PrimitiveDef__destroy(impl_POA_CORBA_PrimitiveDef * servant, CORBA_Environment * ev)
{

	POA_CORBA_PrimitiveDef__fini((PortableServer_Servant) servant, ev);
	g_free(servant);
}

CORBA_PrimitiveKind
impl_CORBA_PrimitiveDef__get_kind(impl_POA_CORBA_PrimitiveDef * servant,
				  CORBA_Environment * ev)
{
	return(servant->attr_kind);
}

CORBA_DefinitionKind
impl_CORBA_PrimitiveDef__get_def_kind(impl_POA_CORBA_PrimitiveDef * servant,
				      CORBA_Environment * ev)
{
	return(servant->attr_def_kind);
}

void impl_CORBA_PrimitiveDef_destroy(impl_POA_CORBA_PrimitiveDef * servant,
				     CORBA_Environment * ev)
{
	ORBit_Trace(TraceMod_IR, TraceLevel_Debug,
		    "Destroying Primitive type %d!\n", servant->attr_kind);
	
	PortableServer_POA_deactivate_object(servant->poa, PortableServer_POA_servant_to_id(servant->poa, servant, ev), ev);
	impl_CORBA_PrimitiveDef__destroy(servant, ev);
}

CORBA_TypeCode
impl_CORBA_PrimitiveDef__get_type(impl_POA_CORBA_PrimitiveDef * servant,
				  CORBA_Environment * ev)
{
	return(servant->attr_type);
}

static impl_POA_CORBA_StringDef *impl_CORBA_StringDef__create(
	PortableServer_POA poa,
	CORBA_unsigned_long bound,
	CORBA_Environment * ev)
{
	impl_POA_CORBA_StringDef *newservant;
	PortableServer_ObjectId *objid;

	newservant = g_new0(impl_POA_CORBA_StringDef, 1);
	newservant->servant.vepv = &impl_CORBA_StringDef_vepv;
	newservant->poa = poa;

	newservant->attr_def_kind=CORBA_dk_String;
	newservant->attr_bound=bound;
	newservant->attr_name=NULL;
	newservant->attr_version=NULL;

	POA_CORBA_StringDef__init((PortableServer_Servant) newservant, ev);
	objid = PortableServer_POA_activate_object(poa, newservant, ev);
	CORBA_free(objid);

	newservant->obj=PortableServer_POA_servant_to_reference(poa,
								newservant,
								ev);
	
	return(newservant);
}

/* You shouldn't call this routine directly without first deactivating the servant... */
static void impl_CORBA_StringDef__destroy(impl_POA_CORBA_StringDef * servant, CORBA_Environment * ev)
{

	POA_CORBA_StringDef__fini((PortableServer_Servant) servant, ev);
	g_free(servant);
}

CORBA_unsigned_long
impl_CORBA_StringDef__get_bound(impl_POA_CORBA_StringDef * servant,
				CORBA_Environment * ev)
{
	return(servant->attr_bound);
}

void impl_CORBA_StringDef__set_bound(impl_POA_CORBA_StringDef * servant,
				     CORBA_unsigned_long value,
				     CORBA_Environment * ev)
{
	servant->attr_bound=value;
}

CORBA_DefinitionKind
impl_CORBA_StringDef__get_def_kind(impl_POA_CORBA_StringDef * servant,
				   CORBA_Environment * ev)
{
	return(servant->attr_def_kind);
}

void impl_CORBA_StringDef_destroy(impl_POA_CORBA_StringDef * servant,
				  CORBA_Environment * ev)
{
	ORBit_Trace(TraceMod_IR, TraceLevel_Debug,
		    "Destroying StringDef length %d!\n", servant->attr_bound);

	PortableServer_POA_deactivate_object(servant->poa, PortableServer_POA_servant_to_id(servant->poa, servant, ev), ev);
	impl_CORBA_StringDef__destroy(servant, ev);
}

CORBA_TypeCode
impl_CORBA_StringDef__get_type(impl_POA_CORBA_StringDef * servant,
			       CORBA_Environment * ev)
{
	return(servant->attr_type);
}

static impl_POA_CORBA_WstringDef *impl_CORBA_WstringDef__create(
	PortableServer_POA poa,
	CORBA_unsigned_long bound,
	CORBA_Environment * ev)
{
	impl_POA_CORBA_WstringDef *newservant;
	PortableServer_ObjectId *objid;

	newservant = g_new0(impl_POA_CORBA_WstringDef, 1);
	newservant->servant.vepv = &impl_CORBA_WstringDef_vepv;
	newservant->poa = poa;

	newservant->attr_def_kind=CORBA_dk_Wstring;
	newservant->attr_bound=bound;
	newservant->attr_name=NULL;
	newservant->attr_version=NULL;

	POA_CORBA_WstringDef__init((PortableServer_Servant) newservant, ev);
	objid = PortableServer_POA_activate_object(poa, newservant, ev);
	CORBA_free(objid);

	newservant->obj=PortableServer_POA_servant_to_reference(poa,
								newservant,
								ev);
	
	return(newservant);
}

/* You shouldn't call this routine directly without first deactivating the servant... */
static void impl_CORBA_WstringDef__destroy(impl_POA_CORBA_WstringDef * servant, CORBA_Environment * ev)
{

	POA_CORBA_WstringDef__fini((PortableServer_Servant) servant, ev);
	g_free(servant);
}

CORBA_unsigned_long
impl_CORBA_WstringDef__get_bound(impl_POA_CORBA_WstringDef * servant,
				 CORBA_Environment * ev)
{
	return(servant->attr_bound);
}

void impl_CORBA_WstringDef__set_bound(impl_POA_CORBA_WstringDef * servant,
				      CORBA_unsigned_long value,
				      CORBA_Environment * ev)
{
	servant->attr_bound=value;
}

CORBA_DefinitionKind
impl_CORBA_WstringDef__get_def_kind(impl_POA_CORBA_WstringDef * servant,
				    CORBA_Environment * ev)
{
	return(servant->attr_def_kind);
}

void impl_CORBA_WstringDef_destroy(impl_POA_CORBA_WstringDef * servant,
				   CORBA_Environment * ev)
{
	ORBit_Trace(TraceMod_IR, TraceLevel_Debug,
		    "Destroying WstringDef length %d!\n", servant->attr_bound);

	PortableServer_POA_deactivate_object(servant->poa, PortableServer_POA_servant_to_id(servant->poa, servant, ev), ev);
	impl_CORBA_WstringDef__destroy(servant, ev);
}

CORBA_TypeCode
impl_CORBA_WstringDef__get_type(impl_POA_CORBA_WstringDef * servant,
				CORBA_Environment * ev)
{
	return(servant->attr_type);
}

static impl_POA_CORBA_FixedDef *impl_CORBA_FixedDef__create(
	PortableServer_POA poa,
	CORBA_unsigned_short digits,
	CORBA_short scale,
	CORBA_Environment * ev)
{
	impl_POA_CORBA_FixedDef *newservant;
	PortableServer_ObjectId *objid;

	newservant = g_new0(impl_POA_CORBA_FixedDef, 1);
	newservant->servant.vepv = &impl_CORBA_FixedDef_vepv;
	newservant->poa = poa;

	newservant->attr_def_kind=CORBA_dk_Fixed;
	newservant->attr_name=NULL;
	newservant->attr_version=NULL;
	newservant->attr_digits=digits;
	newservant->attr_scale=scale;

	POA_CORBA_FixedDef__init((PortableServer_Servant) newservant, ev);
	objid = PortableServer_POA_activate_object(poa, newservant, ev);
	CORBA_free(objid);

	newservant->obj=PortableServer_POA_servant_to_reference(poa,
								newservant,
								ev);
	
	return(newservant);
}

/* You shouldn't call this routine directly without first deactivating the servant... */
static void impl_CORBA_FixedDef__destroy(impl_POA_CORBA_FixedDef * servant, CORBA_Environment * ev)
{

	POA_CORBA_FixedDef__fini((PortableServer_Servant) servant, ev);
	g_free(servant);
}

CORBA_unsigned_short
impl_CORBA_FixedDef__get_digits(impl_POA_CORBA_FixedDef * servant,
				CORBA_Environment * ev)
{
	return(servant->attr_digits);
}

void impl_CORBA_FixedDef__set_digits(impl_POA_CORBA_FixedDef * servant,
				     CORBA_unsigned_short value,
				     CORBA_Environment * ev)
{
	servant->attr_digits=value;
}

CORBA_short
impl_CORBA_FixedDef__get_scale(impl_POA_CORBA_FixedDef * servant,
			       CORBA_Environment * ev)
{
	return(servant->attr_scale);
}

void impl_CORBA_FixedDef__set_scale(impl_POA_CORBA_FixedDef * servant,
				    CORBA_short value,
				    CORBA_Environment * ev)
{
	servant->attr_scale=value;
}

CORBA_DefinitionKind
impl_CORBA_FixedDef__get_def_kind(impl_POA_CORBA_FixedDef * servant,
				  CORBA_Environment * ev)
{
	return(servant->attr_def_kind);
}

void impl_CORBA_FixedDef_destroy(impl_POA_CORBA_FixedDef * servant,
				 CORBA_Environment * ev)
{
	ORBit_Trace(TraceMod_IR, TraceLevel_Debug,
		    "Destroying FixedDef %d/%d!\n", servant->attr_digits,
		    servant->attr_scale);

	PortableServer_POA_deactivate_object(servant->poa, PortableServer_POA_servant_to_id(servant->poa, servant, ev), ev);
	impl_CORBA_FixedDef__destroy(servant, ev);
}

CORBA_TypeCode
impl_CORBA_FixedDef__get_type(impl_POA_CORBA_FixedDef * servant,
			      CORBA_Environment * ev)
{
	return(servant->attr_type);
}

static impl_POA_CORBA_SequenceDef *impl_CORBA_SequenceDef__create(
	PortableServer_POA poa,
	CORBA_unsigned_long bound,
	CORBA_IDLType element_type,
	CORBA_Environment * ev)
{
	impl_POA_CORBA_SequenceDef *newservant;
	PortableServer_ObjectId *objid;

	newservant = g_new0(impl_POA_CORBA_SequenceDef, 1);
	newservant->servant.vepv = &impl_CORBA_SequenceDef_vepv;
	newservant->poa = poa;

	newservant->attr_def_kind=CORBA_dk_Sequence;
	newservant->attr_name=NULL;
	newservant->attr_version=NULL;
	newservant->attr_bound=bound;
	newservant->attr_element_type_def=CORBA_Object_duplicate(element_type, ev);

	POA_CORBA_SequenceDef__init((PortableServer_Servant) newservant, ev);
	objid = PortableServer_POA_activate_object(poa, newservant, ev);
	CORBA_free(objid);

	newservant->obj=PortableServer_POA_servant_to_reference(poa,
								newservant,
								ev);
	
	return(newservant);
}

/* You shouldn't call this routine directly without first deactivating the servant... */
static void impl_CORBA_SequenceDef__destroy(impl_POA_CORBA_SequenceDef * servant, CORBA_Environment * ev)
{

	POA_CORBA_SequenceDef__fini((PortableServer_Servant) servant, ev);
	g_free(servant);
}

CORBA_unsigned_long
impl_CORBA_SequenceDef__get_bound(impl_POA_CORBA_SequenceDef * servant,
				  CORBA_Environment * ev)
{
	return(servant->attr_bound);
}

void impl_CORBA_SequenceDef__set_bound(impl_POA_CORBA_SequenceDef * servant,
				       CORBA_unsigned_long value,
				       CORBA_Environment * ev)
{
	servant->attr_bound=value;
}

CORBA_TypeCode
impl_CORBA_SequenceDef__get_element_type(impl_POA_CORBA_SequenceDef * servant,
					 CORBA_Environment * ev)
{
	return(CORBA_IDLType__get_type(servant->attr_element_type_def, ev));
}

CORBA_IDLType
impl_CORBA_SequenceDef__get_element_type_def(impl_POA_CORBA_SequenceDef * servant,
					     CORBA_Environment * ev)
{
	return(CORBA_Object_duplicate(servant->attr_element_type_def, ev));
}

void impl_CORBA_SequenceDef__set_element_type_def(impl_POA_CORBA_SequenceDef * servant,
						  CORBA_IDLType value,
						  CORBA_Environment * ev)
{
	if(servant->attr_element_type_def!=CORBA_OBJECT_NIL) {
		CORBA_Object_release(servant->attr_element_type_def, ev);
	}
	servant->attr_element_type_def=CORBA_Object_duplicate(value, ev);
}

CORBA_DefinitionKind
impl_CORBA_SequenceDef__get_def_kind(impl_POA_CORBA_SequenceDef * servant,
				     CORBA_Environment * ev)
{
	return(servant->attr_def_kind);
}

void impl_CORBA_SequenceDef_destroy(impl_POA_CORBA_SequenceDef * servant,
				    CORBA_Environment * ev)
{
	ORBit_Trace(TraceMod_IR, TraceLevel_Debug,
		    "Destroying SequenceDef %s!\n", servant->attr_name);

	PortableServer_POA_deactivate_object(servant->poa, PortableServer_POA_servant_to_id(servant->poa, servant, ev), ev);
	impl_CORBA_SequenceDef__destroy(servant, ev);
}

CORBA_TypeCode
impl_CORBA_SequenceDef__get_type(impl_POA_CORBA_SequenceDef * servant,
				 CORBA_Environment * ev)
{
	return(servant->attr_type);
}

static impl_POA_CORBA_ArrayDef *impl_CORBA_ArrayDef__create(
	PortableServer_POA poa,
	CORBA_unsigned_long length,
	CORBA_IDLType element_type,
	CORBA_Environment * ev)
{
	impl_POA_CORBA_ArrayDef *newservant;
	PortableServer_ObjectId *objid;

	newservant = g_new0(impl_POA_CORBA_ArrayDef, 1);
	newservant->servant.vepv = &impl_CORBA_ArrayDef_vepv;
	newservant->poa = poa;

	newservant->attr_def_kind=CORBA_dk_Array;
	newservant->attr_name=NULL;
	newservant->attr_version=NULL;
	newservant->attr_length=length;
	newservant->attr_element_type_def=CORBA_Object_duplicate(element_type, ev);
	
	POA_CORBA_ArrayDef__init((PortableServer_Servant) newservant, ev);
	objid = PortableServer_POA_activate_object(poa, newservant, ev);
	CORBA_free(objid);

	newservant->obj=PortableServer_POA_servant_to_reference(poa,
								newservant,
								ev);
	
	return(newservant);
}

/* You shouldn't call this routine directly without first deactivating the servant... */
static void impl_CORBA_ArrayDef__destroy(impl_POA_CORBA_ArrayDef * servant, CORBA_Environment * ev)
{

	POA_CORBA_ArrayDef__fini((PortableServer_Servant) servant, ev);
	g_free(servant);
}

CORBA_unsigned_long
impl_CORBA_ArrayDef__get_length(impl_POA_CORBA_ArrayDef * servant,
				CORBA_Environment * ev)
{
	return(servant->attr_length);
}

void impl_CORBA_ArrayDef__set_length(impl_POA_CORBA_ArrayDef * servant,
				     CORBA_unsigned_long value,
				     CORBA_Environment * ev)
{
	servant->attr_length=value;
}

CORBA_TypeCode
impl_CORBA_ArrayDef__get_element_type(impl_POA_CORBA_ArrayDef * servant,
				      CORBA_Environment * ev)
{
	return(CORBA_IDLType__get_type(servant->attr_element_type_def, ev));
}

CORBA_IDLType
impl_CORBA_ArrayDef__get_element_type_def(impl_POA_CORBA_ArrayDef * servant,
					  CORBA_Environment * ev)
{
	return(CORBA_Object_duplicate(servant->attr_element_type_def, ev));
}

void impl_CORBA_ArrayDef__set_element_type_def(impl_POA_CORBA_ArrayDef * servant,
					       CORBA_IDLType value,
					       CORBA_Environment * ev)
{
	if(servant->attr_element_type_def!=CORBA_OBJECT_NIL) {
		CORBA_Object_release(servant->attr_element_type_def, ev);
	}
	servant->attr_element_type_def=CORBA_Object_duplicate(value, ev);
}

CORBA_DefinitionKind
impl_CORBA_ArrayDef__get_def_kind(impl_POA_CORBA_ArrayDef * servant,
				  CORBA_Environment * ev)
{
	return(servant->attr_def_kind);
}

void impl_CORBA_ArrayDef_destroy(impl_POA_CORBA_ArrayDef * servant,
				 CORBA_Environment * ev)
{
	ORBit_Trace(TraceMod_IR, TraceLevel_Debug,
		    "Destroying ArrayDef %s!\n", servant->attr_name);

	PortableServer_POA_deactivate_object(servant->poa, PortableServer_POA_servant_to_id(servant->poa, servant, ev), ev);
	impl_CORBA_ArrayDef__destroy(servant, ev);
}

CORBA_TypeCode
impl_CORBA_ArrayDef__get_type(impl_POA_CORBA_ArrayDef * servant,
			      CORBA_Environment * ev)
{
	return(servant->attr_type);
}

static impl_POA_CORBA_ExceptionDef *impl_CORBA_ExceptionDef__create(
	PortableServer_POA poa,
	CORBA_RepositoryId id,
	CORBA_Identifier name,
	CORBA_VersionSpec version,
	CORBA_StructMemberSeq *members,
	CORBA_Container defined_in,
	CORBA_Environment * ev)
{
	impl_POA_CORBA_ExceptionDef *newservant;
	PortableServer_ObjectId *objid;

	if(structmemberseq_verify(members)==CORBA_FALSE) {
		return(NULL);
	}
	
	newservant = g_new0(impl_POA_CORBA_ExceptionDef, 1);
	newservant->servant.vepv = &impl_CORBA_ExceptionDef_vepv;
	newservant->poa = poa;

	newservant->attr_def_kind=CORBA_dk_Exception;
	newservant->attr_id=CORBA_string_dup(id);
	newservant->attr_name=CORBA_string_dup(name);
	newservant->attr_version=CORBA_string_dup(version);
	newservant->attr_defined_in=CORBA_Object_duplicate(defined_in, ev);
	newservant->attr_members=structmemberseq_copy(members);

	newservant->contents=NULL;
	
	POA_CORBA_ExceptionDef__init((PortableServer_Servant) newservant, ev);
	objid = PortableServer_POA_activate_object(poa, newservant, ev);
	CORBA_free(objid);

	newservant->obj=PortableServer_POA_servant_to_reference(poa,
								newservant,
								ev);
	
	g_hash_table_insert(repo->id_hash, g_strdup(id), newservant->obj);
	
	return(newservant);
}

/* You shouldn't call this routine directly without first deactivating the servant... */
static void impl_CORBA_ExceptionDef__destroy(impl_POA_CORBA_ExceptionDef * servant, CORBA_Environment * ev)
{
	g_slist_free(servant->contents);

	POA_CORBA_ExceptionDef__fini((PortableServer_Servant) servant, ev);
	g_free(servant);
}

CORBA_TypeCode
impl_CORBA_ExceptionDef__get_type(impl_POA_CORBA_ExceptionDef * servant,
				  CORBA_Environment * ev)
{
	return(servant->attr_type);
}

CORBA_StructMemberSeq *
 impl_CORBA_ExceptionDef__get_members(impl_POA_CORBA_ExceptionDef * servant,
				      CORBA_Environment * ev)
{
	CORBA_StructMemberSeq *retval;

	retval=structmemberseq_copy(servant->attr_members);
	if(retval==NULL) {
		return(CORBA_OBJECT_NIL);
	}
	
	return(retval);
}

void impl_CORBA_ExceptionDef__set_members(impl_POA_CORBA_ExceptionDef * servant,
					  CORBA_StructMemberSeq * value,
					  CORBA_Environment * ev)
{
	if(structmemberseq_verify(value)==CORBA_FALSE) {
		ORBit_Trace(TraceMod_IR, TraceLevel_Info,
			    "ExceptionDef__set_members: bogus value!\n");
		return;
	}
	
	structmemberseq_free(servant->attr_members);
	servant->attr_members=structmemberseq_copy(value);
}

CORBA_DefinitionKind
impl_CORBA_ExceptionDef__get_def_kind(impl_POA_CORBA_ExceptionDef * servant,
				      CORBA_Environment * ev)
{
	return(servant->attr_def_kind);
}

void impl_CORBA_ExceptionDef_destroy(impl_POA_CORBA_ExceptionDef * servant,
				     CORBA_Environment * ev)
{
	GSList *list, *nextlist;
	
	ORBit_Trace(TraceMod_IR, TraceLevel_Debug,
		    "Destroying Exception %s!\n", servant->attr_id);
	
	for(list=servant->contents; list; list=nextlist) {
		/* Recursively delete everything */

		nextlist=g_slist_next(list);	/* _destroy()ing the
                                                   contained items
                                                   fiddles with the
                                                   list we are
                                                   traversing */

		CORBA_IRObject_destroy(((IfaceRepoContents *)list->data)->obj,
				       ev);
	}
	
	/* Unlink from parent container */
	contained_remove_from_parent(&repo->contents, servant);
	
	PortableServer_POA_deactivate_object(servant->poa, PortableServer_POA_servant_to_id(servant->poa, servant, ev), ev);
	impl_CORBA_ExceptionDef__destroy(servant, ev);
}

CORBA_RepositoryId
impl_CORBA_ExceptionDef__get_id(impl_POA_CORBA_ExceptionDef * servant,
				CORBA_Environment * ev)
{
	return(CORBA_string_dup(servant->attr_id));
}

void impl_CORBA_ExceptionDef__set_id(impl_POA_CORBA_ExceptionDef * servant,
				     CORBA_RepositoryId value,
				     CORBA_Environment * ev)
{
	contained__set_id(&servant->attr_id, value, "ExceptionDef");
}

CORBA_Identifier
impl_CORBA_ExceptionDef__get_name(impl_POA_CORBA_ExceptionDef * servant,
				  CORBA_Environment * ev)
{
	return(CORBA_string_dup(servant->attr_name));
}

void impl_CORBA_ExceptionDef__set_name(impl_POA_CORBA_ExceptionDef * servant,
				       CORBA_Identifier value,
				       CORBA_Environment * ev)
{
	contained__set_name(&servant->attr_name, value, "ExceptionDef");
}

CORBA_VersionSpec
impl_CORBA_ExceptionDef__get_version(impl_POA_CORBA_ExceptionDef * servant,
				     CORBA_Environment * ev)
{
	return(CORBA_string_dup(servant->attr_version));
}

void impl_CORBA_ExceptionDef__set_version(impl_POA_CORBA_ExceptionDef * servant,
					  CORBA_VersionSpec value,
					  CORBA_Environment * ev)
{
	contained__set_version(&servant->attr_version, value, "ExceptionDef");
}

CORBA_Container
impl_CORBA_ExceptionDef__get_defined_in(impl_POA_CORBA_ExceptionDef * servant,
					CORBA_Environment * ev)
{
	return(CORBA_Object_duplicate(servant->attr_defined_in, ev));
}

CORBA_ScopedName
impl_CORBA_ExceptionDef__get_absolute_name(impl_POA_CORBA_ExceptionDef * servant,
					   CORBA_Environment * ev)
{
	return(get_absolute_name(servant->attr_name, servant->attr_defined_in, ev));
}

CORBA_Repository
impl_CORBA_ExceptionDef__get_containing_repository(impl_POA_CORBA_ExceptionDef * servant,
						   CORBA_Environment * ev)
{
	return(CORBA_Object_duplicate(repo->obj, ev));
}

CORBA_Contained_Description *
 impl_CORBA_ExceptionDef_describe(impl_POA_CORBA_ExceptionDef * servant,
				  CORBA_Environment * ev)
{
	CORBA_Contained_Description *retval;
	CORBA_ExceptionDescription *desc;

	desc=CORBA_ExceptionDescription__alloc();
	if(desc==NULL) {
		return(CORBA_OBJECT_NIL);
	}
	
	retval=CORBA_Contained_Description__alloc();
	if(retval==NULL) {
		CORBA_free(desc);
		return(CORBA_OBJECT_NIL);
	}
	
	desc->name=CORBA_string_dup(servant->attr_name);
	desc->id=CORBA_string_dup(servant->attr_id);
	desc->defined_in=CORBA_string_dup(CORBA_Contained__get_id(servant->attr_defined_in, ev));
	desc->version=CORBA_string_dup(servant->attr_version);
	desc->type=servant->attr_type;
	
	retval->kind=CORBA_dk_Exception;
	retval->value._type=(CORBA_TypeCode)TC_CORBA_ExceptionDescription;
	retval->value._value=desc;
	
	return(retval);
}

void impl_CORBA_ExceptionDef_move(impl_POA_CORBA_ExceptionDef * servant,
				  CORBA_Container new_container,
				  CORBA_Identifier new_name,
				  CORBA_VersionSpec new_version,
				  CORBA_Environment * ev)
{
	contained_move((IfaceRepoContents *)servant, new_container, new_name, new_version, "ExceptionDef", &servant->attr_defined_in, ev);
}

CORBA_Contained
impl_CORBA_ExceptionDef_lookup(impl_POA_CORBA_ExceptionDef * servant,
			       CORBA_ScopedName search_name,
			       CORBA_Environment * ev)
{
	return(container_lookup(servant->contents, search_name, "ExceptionDef", ev));
}

CORBA_ContainedSeq *
 impl_CORBA_ExceptionDef_contents(impl_POA_CORBA_ExceptionDef * servant,
				  CORBA_DefinitionKind limit_type,
				  CORBA_boolean exclude_inherited,
				  CORBA_Environment * ev)
{
	return(container_contents(servant->contents, limit_type, exclude_inherited, "ExceptionDef", ev));
}

CORBA_ContainedSeq *
 impl_CORBA_ExceptionDef_lookup_name(impl_POA_CORBA_ExceptionDef * servant,
				     CORBA_Identifier search_name,
				     CORBA_long levels_to_search,
				     CORBA_DefinitionKind limit_type,
				     CORBA_boolean exclude_inherited,
				     CORBA_Environment * ev)
{
	return(container_lookup_name(servant->contents, search_name, levels_to_search, limit_type, exclude_inherited, "ExceptionDef", ev));
}

CORBA_Container_DescriptionSeq *
 impl_CORBA_ExceptionDef_describe_contents(impl_POA_CORBA_ExceptionDef * servant,
					 CORBA_DefinitionKind limit_type,
					 CORBA_boolean exclude_inherited,
					   CORBA_long max_returned_objs,
					   CORBA_Environment * ev)
{
	return(container_describe_contents(servant->contents, limit_type, exclude_inherited, max_returned_objs, "ExceptionDef", ev));
}

CORBA_ModuleDef
impl_CORBA_ExceptionDef_create_module(impl_POA_CORBA_ExceptionDef * servant,
				      CORBA_RepositoryId id,
				      CORBA_Identifier name,
				      CORBA_VersionSpec version,
				      CORBA_Environment * ev)
{
	/* Exceptions can only contain structs, unions and
           enums. Unfortunately, the spec doesn't specify any
           exceptions, so I will just ignore the request for being
           silly */
	return(CORBA_OBJECT_NIL);
}

CORBA_ConstantDef
impl_CORBA_ExceptionDef_create_constant(impl_POA_CORBA_ExceptionDef * servant,
					CORBA_RepositoryId id,
					CORBA_Identifier name,
					CORBA_VersionSpec version,
					CORBA_IDLType type,
					CORBA_any * value,
					CORBA_Environment * ev)
{
	/* Exceptions can only contain structs, unions and
           enums. Unfortunately, the spec doesn't specify any
           exceptions, so I will just ignore the request for being
           silly */
	return(CORBA_OBJECT_NIL);
}

CORBA_StructDef
impl_CORBA_ExceptionDef_create_struct(impl_POA_CORBA_ExceptionDef * servant,
				      CORBA_RepositoryId id,
				      CORBA_Identifier name,
				      CORBA_VersionSpec version,
				      CORBA_StructMemberSeq * members,
				      CORBA_Environment * ev)
{
	return(container_create_struct(&servant->contents, servant->poa, id, name, version, members, servant->obj, "ExceptionDef", ev));
}

CORBA_UnionDef
impl_CORBA_ExceptionDef_create_union(impl_POA_CORBA_ExceptionDef * servant,
				     CORBA_RepositoryId id,
				     CORBA_Identifier name,
				     CORBA_VersionSpec version,
				     CORBA_IDLType discriminator_type,
				     CORBA_UnionMemberSeq * members,
				     CORBA_Environment * ev)
{
	return(container_create_union(&servant->contents, servant->poa, id, name, version, discriminator_type, members, servant->obj, "ExceptionDef", ev));
}

CORBA_EnumDef
impl_CORBA_ExceptionDef_create_enum(impl_POA_CORBA_ExceptionDef * servant,
				    CORBA_RepositoryId id,
				    CORBA_Identifier name,
				    CORBA_VersionSpec version,
				    CORBA_EnumMemberSeq * members,
				    CORBA_Environment * ev)
{
	return(container_create_enum(&servant->contents, servant->poa, id, name, version, members, servant->obj, "ExceptionDef", ev));
}

CORBA_AliasDef
impl_CORBA_ExceptionDef_create_alias(impl_POA_CORBA_ExceptionDef * servant,
				     CORBA_RepositoryId id,
				     CORBA_Identifier name,
				     CORBA_VersionSpec version,
				     CORBA_IDLType original_type,
				     CORBA_Environment * ev)
{
	/* Exceptions can only contain structs, unions and
           enums. Unfortunately, the spec doesn't specify any
           exceptions, so I will just ignore the request for being
           silly */
	return(CORBA_OBJECT_NIL);
}

CORBA_InterfaceDef
impl_CORBA_ExceptionDef_create_interface(impl_POA_CORBA_ExceptionDef * servant,
					 CORBA_RepositoryId id,
					 CORBA_Identifier name,
					 CORBA_VersionSpec version,
					 CORBA_InterfaceDefSeq * base_interfaces,
					 CORBA_Environment * ev)
{
	/* Exceptions can only contain structs, unions and
           enums. Unfortunately, the spec doesn't specify any
           exceptions, so I will just ignore the request for being
           silly */
	return(CORBA_OBJECT_NIL);
}

CORBA_ExceptionDef
impl_CORBA_ExceptionDef_create_exception(impl_POA_CORBA_ExceptionDef * servant,
					 CORBA_RepositoryId id,
					 CORBA_Identifier name,
					 CORBA_VersionSpec version,
					 CORBA_StructMemberSeq * members,
					 CORBA_Environment * ev)
{
	/* Exceptions can only contain structs, unions and
           enums. Unfortunately, the spec doesn't specify any
           exceptions, so I will just ignore the request for being
           silly */
	return(CORBA_OBJECT_NIL);
}

static impl_POA_CORBA_AttributeDef *impl_CORBA_AttributeDef__create(
	PortableServer_POA poa,
	CORBA_RepositoryId id,
	CORBA_Identifier name,
	CORBA_VersionSpec version,
	CORBA_IDLType type,
	CORBA_AttributeMode mode,
	CORBA_Container defined_in,
	CORBA_Environment * ev)
{
	impl_POA_CORBA_AttributeDef *newservant;
	PortableServer_ObjectId *objid;

	newservant = g_new0(impl_POA_CORBA_AttributeDef, 1);
	newservant->servant.vepv = &impl_CORBA_AttributeDef_vepv;
	newservant->poa = poa;

	newservant->attr_type_def=CORBA_Object_duplicate(type, ev);
	newservant->attr_mode=mode;

	newservant->attr_def_kind=CORBA_dk_Attribute;
	newservant->attr_id=CORBA_string_dup(id);
	newservant->attr_name=CORBA_string_dup(name);
	newservant->attr_version=CORBA_string_dup(version);
	newservant->attr_defined_in=CORBA_Object_duplicate(defined_in, ev);

	POA_CORBA_AttributeDef__init((PortableServer_Servant) newservant, ev);
	objid = PortableServer_POA_activate_object(poa, newservant, ev);
	CORBA_free(objid);

	newservant->obj=PortableServer_POA_servant_to_reference(poa,
								newservant,
								ev);
	
	g_hash_table_insert(repo->id_hash, g_strdup(id), newservant->obj);
	
	return(newservant);
}

/* You shouldn't call this routine directly without first deactivating the servant... */
static void impl_CORBA_AttributeDef__destroy(impl_POA_CORBA_AttributeDef * servant, CORBA_Environment * ev)
{

	POA_CORBA_AttributeDef__fini((PortableServer_Servant) servant, ev);
	g_free(servant);
}

CORBA_TypeCode
impl_CORBA_AttributeDef__get_type(impl_POA_CORBA_AttributeDef * servant,
				  CORBA_Environment * ev)
{
	return(servant->attr_type);
}

CORBA_IDLType
impl_CORBA_AttributeDef__get_type_def(impl_POA_CORBA_AttributeDef * servant,
				      CORBA_Environment * ev)
{
	return(CORBA_Object_duplicate(servant->attr_type_def, ev));
}

void impl_CORBA_AttributeDef__set_type_def(impl_POA_CORBA_AttributeDef * servant,
					   CORBA_IDLType value,
					   CORBA_Environment * ev)
{
	if(!CORBA_Object_is_nil(servant->attr_type_def, ev)) {
		CORBA_Object_release(servant->attr_type_def, ev);
	}
	servant->attr_type_def=CORBA_Object_duplicate(value, ev);
}

CORBA_AttributeMode
impl_CORBA_AttributeDef__get_mode(impl_POA_CORBA_AttributeDef * servant,
				  CORBA_Environment * ev)
{
	return(servant->attr_mode);
}

void impl_CORBA_AttributeDef__set_mode(impl_POA_CORBA_AttributeDef * servant,
				       CORBA_AttributeMode value,
				       CORBA_Environment * ev)
{
	servant->attr_mode=value;
}

CORBA_DefinitionKind
impl_CORBA_AttributeDef__get_def_kind(impl_POA_CORBA_AttributeDef * servant,
				      CORBA_Environment * ev)
{
	return(servant->attr_def_kind);
}

void impl_CORBA_AttributeDef_destroy(impl_POA_CORBA_AttributeDef * servant,
				     CORBA_Environment * ev)
{
	ORBit_Trace(TraceMod_IR, TraceLevel_Debug,
		    "Destroying Attribute %s!\n", servant->attr_id);
	
	/* Unlink from parent container */
	contained_remove_from_parent(&repo->contents, servant);
	
	PortableServer_POA_deactivate_object(servant->poa, PortableServer_POA_servant_to_id(servant->poa, servant, ev), ev);
	impl_CORBA_AttributeDef__destroy(servant, ev);
}

CORBA_RepositoryId
impl_CORBA_AttributeDef__get_id(impl_POA_CORBA_AttributeDef * servant,
				CORBA_Environment * ev)
{
	return(CORBA_string_dup(servant->attr_id));
}

void impl_CORBA_AttributeDef__set_id(impl_POA_CORBA_AttributeDef * servant,
				     CORBA_RepositoryId value,
				     CORBA_Environment * ev)
{
	contained__set_id(&servant->attr_id, value, "AttributeDef");
}

CORBA_Identifier
impl_CORBA_AttributeDef__get_name(impl_POA_CORBA_AttributeDef * servant,
				  CORBA_Environment * ev)
{
	return(CORBA_string_dup(servant->attr_name));
}

void impl_CORBA_AttributeDef__set_name(impl_POA_CORBA_AttributeDef * servant,
				       CORBA_Identifier value,
				       CORBA_Environment * ev)
{
	contained__set_name(&servant->attr_name, value, "AttributeDef");
}

CORBA_VersionSpec
impl_CORBA_AttributeDef__get_version(impl_POA_CORBA_AttributeDef * servant,
				     CORBA_Environment * ev)
{
	return(CORBA_string_dup(servant->attr_version));
}

void impl_CORBA_AttributeDef__set_version(impl_POA_CORBA_AttributeDef * servant,
					  CORBA_VersionSpec value,
					  CORBA_Environment * ev)
{
	contained__set_version(&servant->attr_version, value, "AttributeDef");
}

CORBA_Container
impl_CORBA_AttributeDef__get_defined_in(impl_POA_CORBA_AttributeDef * servant,
					CORBA_Environment * ev)
{
	return(CORBA_Object_duplicate(servant->attr_defined_in, ev));
}

CORBA_ScopedName
impl_CORBA_AttributeDef__get_absolute_name(impl_POA_CORBA_AttributeDef * servant,
					   CORBA_Environment * ev)
{
	return(get_absolute_name(servant->attr_name, servant->attr_defined_in, ev));
}

CORBA_Repository
impl_CORBA_AttributeDef__get_containing_repository(impl_POA_CORBA_AttributeDef * servant,
						   CORBA_Environment * ev)
{
	return(CORBA_Object_duplicate(repo->obj, ev));
}

CORBA_Contained_Description *
 impl_CORBA_AttributeDef_describe(impl_POA_CORBA_AttributeDef * servant,
				  CORBA_Environment * ev)
{
	CORBA_Contained_Description *retval;
	CORBA_AttributeDescription *desc;

	desc=CORBA_AttributeDescription__alloc();
	if(desc==NULL) {
		return(CORBA_OBJECT_NIL);
	}
	
	retval=CORBA_Contained_Description__alloc();
	if(retval==NULL) {
		CORBA_free(desc);
		return(CORBA_OBJECT_NIL);
	}
	
	desc->name=CORBA_string_dup(servant->attr_name);
	desc->id=CORBA_string_dup(servant->attr_id);
	desc->defined_in=CORBA_string_dup(CORBA_Contained__get_id(servant->attr_defined_in, ev));
	desc->version=CORBA_string_dup(servant->attr_version);
	desc->type=servant->attr_type;
	desc->mode=servant->attr_mode;
	
	retval->kind=CORBA_dk_Attribute;
	retval->value._type=(CORBA_TypeCode)TC_CORBA_AttributeDescription;
	retval->value._value=desc;
	
	return(retval);
}

void impl_CORBA_AttributeDef_move(impl_POA_CORBA_AttributeDef * servant,
				  CORBA_Container new_container,
				  CORBA_Identifier new_name,
				  CORBA_VersionSpec new_version,
				  CORBA_Environment * ev)
{
	contained_move((IfaceRepoContents *)servant, new_container, new_name, new_version, "AttributeDef", &servant->attr_defined_in, ev);
}

static impl_POA_CORBA_OperationDef *impl_CORBA_OperationDef__create(
	PortableServer_POA poa,
	CORBA_RepositoryId id,
	CORBA_Identifier name,
	CORBA_VersionSpec version,
	CORBA_IDLType result,
	CORBA_OperationMode mode,
	CORBA_ParDescriptionSeq *params,
	CORBA_ExceptionDefSeq *exceptions,
	CORBA_ContextIdSeq *contexts,
	CORBA_Container defined_in,
	CORBA_Environment *ev)
{
	impl_POA_CORBA_OperationDef *newservant;
	PortableServer_ObjectId *objid;

	if(pardescriptionseq_verify(params)==CORBA_FALSE) {
		return(NULL);
	}
	if(exceptiondefseq_verify(exceptions)==CORBA_FALSE) {
		return(NULL);
	}
	if(contextidseq_verify(contexts)==CORBA_FALSE) {
		return(NULL);
	}
	
	newservant = g_new0(impl_POA_CORBA_OperationDef, 1);
	newservant->servant.vepv = &impl_CORBA_OperationDef_vepv;
	newservant->poa = poa;

	newservant->attr_def_kind=CORBA_dk_Operation;
	newservant->attr_id=CORBA_string_dup(id);
	newservant->attr_name=CORBA_string_dup(name);
	newservant->attr_version=CORBA_string_dup(version);
	newservant->attr_defined_in=CORBA_Object_duplicate(defined_in, ev);
	newservant->attr_result_def=CORBA_Object_duplicate(result, ev);
	newservant->attr_mode=mode;
	newservant->attr_params=pardescriptionseq_copy(params);
	newservant->attr_exceptions=exceptiondefseq_copy(exceptions);
	newservant->attr_contexts=contextidseq_copy(contexts);

	POA_CORBA_OperationDef__init((PortableServer_Servant) newservant, ev);
	objid = PortableServer_POA_activate_object(poa, newservant, ev);
	CORBA_free(objid);

	newservant->obj=PortableServer_POA_servant_to_reference(poa,
								newservant,
								ev);
	
	g_hash_table_insert(repo->id_hash, g_strdup(id), newservant->obj);
	
	return(newservant);
}

/* You shouldn't call this routine directly without first deactivating the servant... */
static void impl_CORBA_OperationDef__destroy(impl_POA_CORBA_OperationDef * servant, CORBA_Environment * ev)
{

	POA_CORBA_OperationDef__fini((PortableServer_Servant) servant, ev);
	g_free(servant);
}

CORBA_TypeCode
impl_CORBA_OperationDef__get_result(impl_POA_CORBA_OperationDef * servant,
				    CORBA_Environment * ev)
{
	return(CORBA_IDLType__get_type(servant->attr_result_def, ev));
}

CORBA_IDLType
impl_CORBA_OperationDef__get_result_def(impl_POA_CORBA_OperationDef * servant,
					CORBA_Environment * ev)
{
	return(CORBA_Object_duplicate(servant->attr_result_def, ev));
}

void impl_CORBA_OperationDef__set_result_def(impl_POA_CORBA_OperationDef * servant,
					     CORBA_IDLType value,
					     CORBA_Environment * ev)
{
	if(servant->attr_result_def!=CORBA_OBJECT_NIL) {
		CORBA_Object_release(servant->attr_result_def, ev);
	}
	servant->attr_result_def=CORBA_Object_duplicate(value, ev);
}

CORBA_ParDescriptionSeq *
 impl_CORBA_OperationDef__get_params(impl_POA_CORBA_OperationDef * servant,
				     CORBA_Environment * ev)
{
	CORBA_ParDescriptionSeq *retval;

	retval=pardescriptionseq_copy(servant->attr_params);
	if(retval==NULL) {
		return(CORBA_OBJECT_NIL);
	}
	
	return(retval);
}

void impl_CORBA_OperationDef__set_params(impl_POA_CORBA_OperationDef * servant,
					 CORBA_ParDescriptionSeq * value,
					 CORBA_Environment * ev)
{
	if(pardescriptionseq_verify(value)==CORBA_FALSE) {
		ORBit_Trace(TraceMod_IR, TraceLevel_Info,
			    "OperationDef__set_params: bogus value!\n");
		return;
	}
	
	pardescriptionseq_free(servant->attr_params);
	servant->attr_params=pardescriptionseq_copy(value);
}

CORBA_OperationMode
impl_CORBA_OperationDef__get_mode(impl_POA_CORBA_OperationDef * servant,
				  CORBA_Environment * ev)
{
	return(servant->attr_mode);
}

void impl_CORBA_OperationDef__set_mode(impl_POA_CORBA_OperationDef * servant,
				       CORBA_OperationMode value,
				       CORBA_Environment * ev)
{
	servant->attr_mode=value;
}

CORBA_ContextIdSeq *
 impl_CORBA_OperationDef__get_contexts(impl_POA_CORBA_OperationDef * servant,
				       CORBA_Environment * ev)
{
	CORBA_ContextIdSeq *retval;

	retval=contextidseq_copy(servant->attr_contexts);
	if(retval==NULL) {
		return(CORBA_OBJECT_NIL);
	}
	
	return(retval);
}

void impl_CORBA_OperationDef__set_contexts(impl_POA_CORBA_OperationDef * servant,
					   CORBA_ContextIdSeq * value,
					   CORBA_Environment * ev)
{
	if(contextidseq_verify(value)==CORBA_FALSE) {
		ORBit_Trace(TraceMod_IR, TraceLevel_Info,
			    "OperationDef__set_contexts: bogus value!\n");
		return;
	}
	
	contextidseq_free(servant->attr_contexts);
	servant->attr_contexts=contextidseq_copy(value);
}

CORBA_ExceptionDefSeq *
 impl_CORBA_OperationDef__get_exceptions(impl_POA_CORBA_OperationDef * servant,
					 CORBA_Environment * ev)
{
	CORBA_ExceptionDefSeq *retval;

	retval=exceptiondefseq_copy(servant->attr_exceptions);
	if(retval==NULL) {
		return(CORBA_OBJECT_NIL);
	}
	
	return(retval);
}

void impl_CORBA_OperationDef__set_exceptions(impl_POA_CORBA_OperationDef * servant,
					     CORBA_ExceptionDefSeq * value,
					     CORBA_Environment * ev)
{
	if(exceptiondefseq_verify(value)==CORBA_FALSE) {
		ORBit_Trace(TraceMod_IR, TraceLevel_Info,
			    "OperationDef__get_exceptions: bogus value!\n");
		return;
	}
	
	exceptiondefseq_free(servant->attr_exceptions);
	servant->attr_exceptions=exceptiondefseq_copy(value);
}

CORBA_DefinitionKind
impl_CORBA_OperationDef__get_def_kind(impl_POA_CORBA_OperationDef * servant,
				      CORBA_Environment * ev)
{
	return(servant->attr_def_kind);
}

void impl_CORBA_OperationDef_destroy(impl_POA_CORBA_OperationDef * servant,
				     CORBA_Environment * ev)
{
	ORBit_Trace(TraceMod_IR, TraceLevel_Debug,
		    "Destroying Operation %s!\n", servant->attr_id);
	
	/* Unlink from parent container */
	contained_remove_from_parent(&repo->contents, servant);
	
	PortableServer_POA_deactivate_object(servant->poa, PortableServer_POA_servant_to_id(servant->poa, servant, ev), ev);
	impl_CORBA_OperationDef__destroy(servant, ev);
}

CORBA_RepositoryId
impl_CORBA_OperationDef__get_id(impl_POA_CORBA_OperationDef * servant,
				CORBA_Environment * ev)
{
	return(CORBA_string_dup(servant->attr_id));
}

void impl_CORBA_OperationDef__set_id(impl_POA_CORBA_OperationDef * servant,
				     CORBA_RepositoryId value,
				     CORBA_Environment * ev)
{
	contained__set_id(&servant->attr_id, value, "OperationDef");
}

CORBA_Identifier
impl_CORBA_OperationDef__get_name(impl_POA_CORBA_OperationDef * servant,
				  CORBA_Environment * ev)
{
	return(CORBA_string_dup(servant->attr_name));
}

void impl_CORBA_OperationDef__set_name(impl_POA_CORBA_OperationDef * servant,
				       CORBA_Identifier value,
				       CORBA_Environment * ev)
{
	contained__set_name(&servant->attr_name, value, "OperationDef");
}

CORBA_VersionSpec
impl_CORBA_OperationDef__get_version(impl_POA_CORBA_OperationDef * servant,
				     CORBA_Environment * ev)
{
	return(CORBA_string_dup(servant->attr_version));
}

void impl_CORBA_OperationDef__set_version(impl_POA_CORBA_OperationDef * servant,
					  CORBA_VersionSpec value,
					  CORBA_Environment * ev)
{
	contained__set_version(&servant->attr_version, value, "OperationDef");
}

CORBA_Container
impl_CORBA_OperationDef__get_defined_in(impl_POA_CORBA_OperationDef * servant,
					CORBA_Environment * ev)
{
	return(CORBA_Object_duplicate(servant->attr_defined_in, ev));
}

CORBA_ScopedName
impl_CORBA_OperationDef__get_absolute_name(impl_POA_CORBA_OperationDef * servant,
					   CORBA_Environment * ev)
{
	return(get_absolute_name(servant->attr_name, servant->attr_defined_in, ev));
}

CORBA_Repository
impl_CORBA_OperationDef__get_containing_repository(impl_POA_CORBA_OperationDef * servant,
						   CORBA_Environment * ev)
{
	return(CORBA_Object_duplicate(repo->obj, ev));
}

CORBA_Contained_Description *
 impl_CORBA_OperationDef_describe(impl_POA_CORBA_OperationDef * servant,
				  CORBA_Environment * ev)
{
	CORBA_Contained_Description *retval;
	CORBA_OperationDescription *desc;
	CORBA_ContextIdSeq *contexts;
	CORBA_ParDescriptionSeq *params;

	desc=CORBA_OperationDescription__alloc();
	if(desc==NULL) {
		return(CORBA_OBJECT_NIL);
	}
	
	retval=CORBA_Contained_Description__alloc();
	if(retval==NULL) {
		CORBA_free(desc);
		return(CORBA_OBJECT_NIL);
	}
	
	desc->name=CORBA_string_dup(servant->attr_name);
	desc->id=CORBA_string_dup(servant->attr_id);
	desc->defined_in=CORBA_string_dup(CORBA_Contained__get_id(servant->attr_defined_in, ev));
	desc->version=CORBA_string_dup(servant->attr_version);
	desc->result=servant->attr_result;
	desc->mode=servant->attr_mode;
	contexts=contextidseq_copy(servant->attr_contexts);
	memcpy(&desc->contexts, contexts, sizeof(CORBA_ContextIdSeq));

	params=pardescriptionseq_copy(servant->attr_params);
	memcpy(&desc->parameters, params, sizeof(CORBA_ParDescriptionSeq));
	
/* XXX fill in here
	desc->execeptions=
 */
	retval->kind=CORBA_dk_Operation;
	retval->value._type=(CORBA_TypeCode)TC_CORBA_OperationDescription;
	retval->value._value=desc;
	
	return(retval);
}

void impl_CORBA_OperationDef_move(impl_POA_CORBA_OperationDef * servant,
				  CORBA_Container new_container,
				  CORBA_Identifier new_name,
				  CORBA_VersionSpec new_version,
				  CORBA_Environment * ev)
{
	contained_move((IfaceRepoContents *)servant, new_container, new_name, new_version, "OperationDef", &servant->attr_defined_in, ev);
}

static impl_POA_CORBA_InterfaceDef *impl_CORBA_InterfaceDef__create(
	PortableServer_POA poa,
	CORBA_RepositoryId id,
	CORBA_Identifier name,
	CORBA_VersionSpec version,
	CORBA_InterfaceDefSeq *base_interfaces,
	CORBA_Container defined_in,
	CORBA_Environment * ev)
{
	impl_POA_CORBA_InterfaceDef *newservant;
	PortableServer_ObjectId *objid;

	if(interfacedefseq_verify(base_interfaces)==CORBA_FALSE) {
		return(NULL);
	}
	
	newservant = g_new0(impl_POA_CORBA_InterfaceDef, 1);
	newservant->servant.vepv = &impl_CORBA_InterfaceDef_vepv;
	newservant->poa = poa;

	newservant->attr_def_kind=CORBA_dk_Interface;
	newservant->contents=NULL;

	newservant->attr_id=CORBA_string_dup(id);
	newservant->attr_name=CORBA_string_dup(name);
	newservant->attr_version=CORBA_string_dup(version);
	newservant->attr_base_interfaces=interfacedefseq_copy(base_interfaces);
	newservant->attr_defined_in=CORBA_Object_duplicate(defined_in, ev);

	POA_CORBA_InterfaceDef__init((PortableServer_Servant) newservant, ev);
	objid = PortableServer_POA_activate_object(poa, newservant, ev);
	CORBA_free(objid);

	newservant->obj=PortableServer_POA_servant_to_reference(poa,
								newservant,
								ev);
	
	g_hash_table_insert(repo->id_hash, g_strdup(id), newservant->obj);
	
	return(newservant);
}

/* You shouldn't call this routine directly without first deactivating the servant... */
static void impl_CORBA_InterfaceDef__destroy(impl_POA_CORBA_InterfaceDef * servant, CORBA_Environment * ev)
{

	POA_CORBA_InterfaceDef__fini((PortableServer_Servant) servant, ev);
	g_free(servant);
}

CORBA_InterfaceDefSeq *
 impl_CORBA_InterfaceDef__get_base_interfaces(impl_POA_CORBA_InterfaceDef * servant,
					      CORBA_Environment * ev)
{
	CORBA_InterfaceDefSeq *retval;

	retval=interfacedefseq_copy(servant->attr_base_interfaces);
	if(retval==NULL) {
		return(CORBA_OBJECT_NIL);
	}
	
	return(retval);
}

void impl_CORBA_InterfaceDef__set_base_interfaces(impl_POA_CORBA_InterfaceDef * servant,
						  CORBA_InterfaceDefSeq * value,
						  CORBA_Environment * ev)
{
	if(interfacedefseq_verify(value)==CORBA_FALSE) {
		ORBit_Trace(TraceMod_IR, TraceLevel_Info,
			    "InterfaceDef__set_base_interfaces: bogus value!\n");
		return;
	}
	
	interfacedefseq_free(servant->attr_base_interfaces);
	servant->attr_base_interfaces=interfacedefseq_copy(value);
}

CORBA_boolean
impl_CORBA_InterfaceDef_is_a(impl_POA_CORBA_InterfaceDef * servant,
			     CORBA_RepositoryId interface_id,
			     CORBA_Environment * ev)
{
	int i;
	
	if(!strcmp(interface_id, servant->attr_id)) {
		return(CORBA_TRUE);
	}
	
	if(servant->attr_base_interfaces==NULL) {
		return(CORBA_FALSE);
	}
	
	for(i=0; i<servant->attr_base_interfaces->_length; i++) {
		CORBA_InterfaceDef base_iface;

		base_iface=servant->attr_base_interfaces->_buffer[i];
		
		if(CORBA_InterfaceDef_is_a(base_iface, interface_id, ev)==CORBA_TRUE) {
			return(CORBA_TRUE);
		}
	}
	
	return(CORBA_FALSE);
}

CORBA_InterfaceDef_FullInterfaceDescription *
 impl_CORBA_InterfaceDef_describe_interface(impl_POA_CORBA_InterfaceDef * servant,
					    CORBA_Environment * ev)
{
	CORBA_InterfaceDef_FullInterfaceDescription *retval;
	
	retval=CORBA_InterfaceDef_FullInterfaceDescription__alloc();
	if(retval==NULL) {
		return(CORBA_OBJECT_NIL);
	}
	
	retval->name=CORBA_string_dup(servant->attr_name);
	retval->id=CORBA_string_dup(servant->attr_id);
	retval->defined_in=CORBA_string_dup(CORBA_Contained__get_id(servant->attr_defined_in, ev));
	retval->version=CORBA_string_dup(servant->attr_version);
/* XXX fill in here
	retval->operations=
	retval->attributes=
	retval->base_interfaces=
	retval->type=
 */
	
	return(retval);
}

CORBA_AttributeDef
impl_CORBA_InterfaceDef_create_attribute(impl_POA_CORBA_InterfaceDef * servant,
					 CORBA_RepositoryId id,
					 CORBA_Identifier name,
					 CORBA_VersionSpec version,
					 CORBA_IDLType type,
					 CORBA_AttributeMode mode,
					 CORBA_Environment * ev)
{
	CORBA_AttributeDef retval;
	impl_POA_CORBA_AttributeDef *newservant;
	
	/* Make sure this Repo ID isn't taken */
	if(repo_check_id(id)==CORBA_TRUE) {
		return(CORBA_OBJECT_NIL);
	}
	
	newservant=impl_CORBA_AttributeDef__create(servant->poa, id, name,
						   version, type, mode,
						   servant->obj,
						   ev);
	/* Add to contents list */
	servant->contents=g_slist_append(servant->contents, newservant);
	
	retval=newservant->obj;

	return(CORBA_Object_duplicate(retval, ev));
}

CORBA_OperationDef
impl_CORBA_InterfaceDef_create_operation(impl_POA_CORBA_InterfaceDef * servant,
					 CORBA_RepositoryId id,
					 CORBA_Identifier name,
					 CORBA_VersionSpec version,
					 CORBA_IDLType result,
					 CORBA_OperationMode mode,
					 CORBA_ParDescriptionSeq * params,
					 CORBA_ExceptionDefSeq * exceptions,
					 CORBA_ContextIdSeq * contexts,
					 CORBA_Environment * ev)
{
	return(container_create_operation(&servant->contents, servant->poa, id, name, version, result, mode, params, exceptions, contexts, servant->obj, "InterfaceDef", ev));
}

CORBA_DefinitionKind
impl_CORBA_InterfaceDef__get_def_kind(impl_POA_CORBA_InterfaceDef * servant,
				      CORBA_Environment * ev)
{
	return(servant->attr_def_kind);
}

void impl_CORBA_InterfaceDef_destroy(impl_POA_CORBA_InterfaceDef * servant,
				     CORBA_Environment * ev)
{
	GSList *list, *nextlist;
	
	ORBit_Trace(TraceMod_IR, TraceLevel_Debug,
		    "Destroying Interface %s!\n", servant->attr_id);
	
	for(list=servant->contents; list; list=nextlist) {
		/* Recursively delete everything */

		nextlist=g_slist_next(list);	/* _destroy()ing the
                                                   contained items
                                                   fiddles with the
                                                   list we are
                                                   traversing */

		CORBA_IRObject_destroy(((IfaceRepoContents *)list->data)->obj,
				       ev);
	}
	
	/* Unlink from parent container */
	contained_remove_from_parent(&repo->contents, servant);
	
	PortableServer_POA_deactivate_object(servant->poa, PortableServer_POA_servant_to_id(servant->poa, servant, ev), ev);
	impl_CORBA_InterfaceDef__destroy(servant, ev);
}

CORBA_Contained
impl_CORBA_InterfaceDef_lookup(impl_POA_CORBA_InterfaceDef * servant,
			       CORBA_ScopedName search_name,
			       CORBA_Environment * ev)
{
	return(container_lookup(servant->contents, search_name, "InterfaceDef", ev));
}

CORBA_ContainedSeq *
 impl_CORBA_InterfaceDef_contents(impl_POA_CORBA_InterfaceDef * servant,
				  CORBA_DefinitionKind limit_type,
				  CORBA_boolean exclude_inherited,
				  CORBA_Environment * ev)
{
	return(container_contents(servant->contents, limit_type, exclude_inherited, "InterfaceDef", ev));
}

CORBA_ContainedSeq *
 impl_CORBA_InterfaceDef_lookup_name(impl_POA_CORBA_InterfaceDef * servant,
				     CORBA_Identifier search_name,
				     CORBA_long levels_to_search,
				     CORBA_DefinitionKind limit_type,
				     CORBA_boolean exclude_inherited,
				     CORBA_Environment * ev)
{
	return(container_lookup_name(servant->contents, search_name, levels_to_search, limit_type, exclude_inherited, "InterfaceDef", ev));
}

CORBA_Container_DescriptionSeq *
 impl_CORBA_InterfaceDef_describe_contents(impl_POA_CORBA_InterfaceDef * servant,
					 CORBA_DefinitionKind limit_type,
					 CORBA_boolean exclude_inherited,
					   CORBA_long max_returned_objs,
					   CORBA_Environment * ev)
{
	return(container_describe_contents(servant->contents, limit_type, exclude_inherited, max_returned_objs, "InterfaceDef", ev));
}

CORBA_ModuleDef
impl_CORBA_InterfaceDef_create_module(impl_POA_CORBA_InterfaceDef * servant,
				      CORBA_RepositoryId id,
				      CORBA_Identifier name,
				      CORBA_VersionSpec version,
				      CORBA_Environment * ev)
{
	return(container_create_module(&servant->contents, servant->poa, id, name, version, servant->obj, "InterfaceDef", ev));
}

CORBA_ConstantDef
impl_CORBA_InterfaceDef_create_constant(impl_POA_CORBA_InterfaceDef * servant,
					CORBA_RepositoryId id,
					CORBA_Identifier name,
					CORBA_VersionSpec version,
					CORBA_IDLType type,
					CORBA_any * value,
					CORBA_Environment * ev)
{
	return(container_create_constant(&servant->contents, servant->poa, id, name, version, type, value, servant->obj, "InterfaceDef", ev));
}

CORBA_StructDef
impl_CORBA_InterfaceDef_create_struct(impl_POA_CORBA_InterfaceDef * servant,
				      CORBA_RepositoryId id,
				      CORBA_Identifier name,
				      CORBA_VersionSpec version,
				      CORBA_StructMemberSeq * members,
				      CORBA_Environment * ev)
{
	return(container_create_struct(&servant->contents, servant->poa, id, name, version, members, servant->obj, "InterfaceDef", ev));
}

CORBA_UnionDef
impl_CORBA_InterfaceDef_create_union(impl_POA_CORBA_InterfaceDef * servant,
				     CORBA_RepositoryId id,
				     CORBA_Identifier name,
				     CORBA_VersionSpec version,
				     CORBA_IDLType discriminator_type,
				     CORBA_UnionMemberSeq * members,
				     CORBA_Environment * ev)
{
	return(container_create_union(&servant->contents, servant->poa, id, name, version, discriminator_type, members, servant->obj, "InterfaceDef", ev));
}

CORBA_EnumDef
impl_CORBA_InterfaceDef_create_enum(impl_POA_CORBA_InterfaceDef * servant,
				    CORBA_RepositoryId id,
				    CORBA_Identifier name,
				    CORBA_VersionSpec version,
				    CORBA_EnumMemberSeq * members,
				    CORBA_Environment * ev)
{
	return(container_create_enum(&servant->contents, servant->poa, id, name, version, members, servant->obj, "InterfaceDef", ev));
}

CORBA_AliasDef
impl_CORBA_InterfaceDef_create_alias(impl_POA_CORBA_InterfaceDef * servant,
				     CORBA_RepositoryId id,
				     CORBA_Identifier name,
				     CORBA_VersionSpec version,
				     CORBA_IDLType original_type,
				     CORBA_Environment * ev)
{
	return(container_create_alias(&servant->contents, servant->poa, id, name, version, original_type, servant->obj, "InterfaceDef", ev));
}

CORBA_InterfaceDef
impl_CORBA_InterfaceDef_create_interface(impl_POA_CORBA_InterfaceDef * servant,
					 CORBA_RepositoryId id,
					 CORBA_Identifier name,
					 CORBA_VersionSpec version,
					 CORBA_InterfaceDefSeq * base_interfaces,
					 CORBA_Environment * ev)
{
	return(container_create_interface(&servant->contents, servant->poa, id, name, version, base_interfaces, servant->obj, "InterfaceDef", ev));
}

CORBA_ExceptionDef
impl_CORBA_InterfaceDef_create_exception(impl_POA_CORBA_InterfaceDef * servant,
					 CORBA_RepositoryId id,
					 CORBA_Identifier name,
					 CORBA_VersionSpec version,
					 CORBA_StructMemberSeq * members,
					 CORBA_Environment * ev)
{
	return(container_create_exception(&servant->contents, servant->poa, id, name, version, members, servant->obj, "InterfaceDef", ev));
}

CORBA_RepositoryId
impl_CORBA_InterfaceDef__get_id(impl_POA_CORBA_InterfaceDef * servant,
				CORBA_Environment * ev)
{
	return(CORBA_string_dup(servant->attr_id));
}

void impl_CORBA_InterfaceDef__set_id(impl_POA_CORBA_InterfaceDef * servant,
				     CORBA_RepositoryId value,
				     CORBA_Environment * ev)
{
	contained__set_id(&servant->attr_id, value, "InterfaceDef");
}

CORBA_Identifier
impl_CORBA_InterfaceDef__get_name(impl_POA_CORBA_InterfaceDef * servant,
				  CORBA_Environment * ev)
{
	return(CORBA_string_dup(servant->attr_name));
}

void impl_CORBA_InterfaceDef__set_name(impl_POA_CORBA_InterfaceDef * servant,
				       CORBA_Identifier value,
				       CORBA_Environment * ev)
{
	contained__set_name(&servant->attr_name, value, "InterfaceDef");
}

CORBA_VersionSpec
impl_CORBA_InterfaceDef__get_version(impl_POA_CORBA_InterfaceDef * servant,
				     CORBA_Environment * ev)
{
	return(CORBA_string_dup(servant->attr_version));
}

void impl_CORBA_InterfaceDef__set_version(impl_POA_CORBA_InterfaceDef * servant,
					  CORBA_VersionSpec value,
					  CORBA_Environment * ev)
{
	contained__set_version(&servant->attr_version, value, "InterfaceDef");
}

CORBA_Container
impl_CORBA_InterfaceDef__get_defined_in(impl_POA_CORBA_InterfaceDef * servant,
					CORBA_Environment * ev)
{
	return(CORBA_Object_duplicate(servant->attr_defined_in, ev));
}

CORBA_ScopedName
impl_CORBA_InterfaceDef__get_absolute_name(impl_POA_CORBA_InterfaceDef * servant,
					   CORBA_Environment * ev)
{
	return(get_absolute_name(servant->attr_name, servant->attr_defined_in, ev));
}

CORBA_Repository
impl_CORBA_InterfaceDef__get_containing_repository(impl_POA_CORBA_InterfaceDef * servant,
						   CORBA_Environment * ev)
{
	return(CORBA_Object_duplicate(repo->obj, ev));
}

CORBA_Contained_Description *
 impl_CORBA_InterfaceDef_describe(impl_POA_CORBA_InterfaceDef * servant,
				  CORBA_Environment * ev)
{
	CORBA_Contained_Description *retval;
	CORBA_InterfaceDescription *desc;
	int i;

	desc=CORBA_InterfaceDescription__alloc();
	if(desc==NULL) {
		return(CORBA_OBJECT_NIL);
	}
	
	retval=CORBA_Contained_Description__alloc();
	if(retval==NULL) {
		CORBA_free(desc);
		return(CORBA_OBJECT_NIL);
	}
	
	desc->name=CORBA_string_dup(servant->attr_name);
	desc->id=CORBA_string_dup(servant->attr_id);
	desc->defined_in=CORBA_string_dup(CORBA_Contained__get_id(servant->attr_defined_in, ev));
	desc->version=CORBA_string_dup(servant->attr_version);
	if(servant->attr_base_interfaces!=NULL) {
		desc->base_interfaces._length=servant->attr_base_interfaces->_length;
		desc->base_interfaces._maximum=servant->attr_base_interfaces->_maximum;
		desc->base_interfaces._buffer=CORBA_sequence_CORBA_RepositoryId_allocbuf(desc->base_interfaces._length);
		if(desc->base_interfaces._buffer==NULL) {
			CORBA_free(desc);
			return(CORBA_OBJECT_NIL);
		} else {
			for(i=0;i<desc->base_interfaces._length;i++) {
				desc->base_interfaces._buffer[i]=CORBA_InterfaceDef__get_id(servant->attr_base_interfaces->_buffer[i], ev);
			}
		}
	} else {
		desc->base_interfaces._length=0;
		desc->base_interfaces._maximum=0;
		desc->base_interfaces._buffer=NULL;
	}
	
	retval->kind=CORBA_dk_Interface;
	retval->value._type=(CORBA_TypeCode)TC_CORBA_InterfaceDescription;
	retval->value._value=desc;
	
	return(retval);
}

void impl_CORBA_InterfaceDef_move(impl_POA_CORBA_InterfaceDef * servant,
				  CORBA_Container new_container,
				  CORBA_Identifier new_name,
				  CORBA_VersionSpec new_version,
				  CORBA_Environment * ev)
{
	contained_move((IfaceRepoContents *)servant, new_container, new_name, new_version, "InterfaceDef", &servant->attr_defined_in, ev);
}

CORBA_TypeCode
impl_CORBA_InterfaceDef__get_type(impl_POA_CORBA_InterfaceDef * servant,
				  CORBA_Environment * ev)
{
	return(servant->attr_type);
}

int main(int argc, char **argv)
{
	PortableServer_POA poa;
	CORBA_Environment ev;
	impl_POA_CORBA_PrimitiveDef *primservant;
	char *retval;

	CORBA_exception_init(&ev);
	orb = CORBA_ORB_init(&argc, argv, "orbit-local-orb", &ev);

	poa = (PortableServer_POA)CORBA_ORB_resolve_initial_references(orb, "RootPOA", &ev);
	PortableServer_POAManager_activate(PortableServer_POA__get_the_POAManager(poa, &ev), &ev);

	repo = impl_CORBA_Repository__create(poa, &ev);
	if(!repo) {
		g_print("Cannot get object reference for Interface Repository\n");
		exit(-1);
	}

	/* Create the primitive objects */
	primservant=impl_CORBA_PrimitiveDef__create(poa, CORBA_pk_null, &ev);
	prim_null=primservant->obj;

	primservant=impl_CORBA_PrimitiveDef__create(poa, CORBA_pk_void, &ev);
	prim_void=primservant->obj;

	primservant=impl_CORBA_PrimitiveDef__create(poa, CORBA_pk_short, &ev);
	prim_short=primservant->obj;

	primservant=impl_CORBA_PrimitiveDef__create(poa, CORBA_pk_long, &ev);
	prim_long=primservant->obj;

	primservant=impl_CORBA_PrimitiveDef__create(poa, CORBA_pk_ushort, &ev);
	prim_ushort=primservant->obj;

	primservant=impl_CORBA_PrimitiveDef__create(poa, CORBA_pk_ulong, &ev);
	prim_ulong=primservant->obj;

	primservant=impl_CORBA_PrimitiveDef__create(poa, CORBA_pk_float, &ev);
	prim_float=primservant->obj;

	primservant=impl_CORBA_PrimitiveDef__create(poa, CORBA_pk_double, &ev);
	prim_double=primservant->obj;

	primservant=impl_CORBA_PrimitiveDef__create(poa, CORBA_pk_boolean,
						    &ev);
	prim_boolean=primservant->obj;

	primservant=impl_CORBA_PrimitiveDef__create(poa, CORBA_pk_char, &ev);
	prim_char=primservant->obj;

	primservant=impl_CORBA_PrimitiveDef__create(poa, CORBA_pk_octet, &ev);
	prim_octet=primservant->obj;

	primservant=impl_CORBA_PrimitiveDef__create(poa, CORBA_pk_any, &ev);
	prim_any=primservant->obj;

	primservant=impl_CORBA_PrimitiveDef__create(poa, CORBA_pk_TypeCode,
						    &ev);
	prim_TypeCode=primservant->obj;

	primservant=impl_CORBA_PrimitiveDef__create(poa, CORBA_pk_Principal,
						    &ev);
	prim_Principal=primservant->obj;

	primservant=impl_CORBA_PrimitiveDef__create(poa, CORBA_pk_string, &ev);
	prim_string=primservant->obj;

	primservant=impl_CORBA_PrimitiveDef__create(poa, CORBA_pk_objref, &ev);
	prim_objref=primservant->obj;

	primservant=impl_CORBA_PrimitiveDef__create(poa, CORBA_pk_longlong,
						    &ev);
	prim_longlong=primservant->obj;

	primservant=impl_CORBA_PrimitiveDef__create(poa, CORBA_pk_ulonglong,
						    &ev);
	prim_ulonglong=primservant->obj;

	primservant=impl_CORBA_PrimitiveDef__create(poa, CORBA_pk_longdouble,
						    &ev);
	prim_longdouble=primservant->obj;

	primservant=impl_CORBA_PrimitiveDef__create(poa, CORBA_pk_wchar, &ev);
	prim_wchar=primservant->obj;

	primservant=impl_CORBA_PrimitiveDef__create(poa, CORBA_pk_wstring,
						    &ev);
	prim_wstring=primservant->obj;
	
	retval = CORBA_ORB_object_to_string(orb, repo->obj, &ev);
	g_print("%s\n", retval);
	fflush(stdout);
	CORBA_free(retval);

	CORBA_ORB_run(orb, &ev);

	exit(0);
}
