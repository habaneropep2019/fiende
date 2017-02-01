#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <orb/orbit.h>

#define CHECK_OK(ev)           g_assert ((ev)->_major == CORBA_NO_EXCEPTION)
#define CHECK_TYPE_MISMATCH(ev) \
	do { \
		g_assert ((ev)->_major == CORBA_USER_EXCEPTION && \
		          !strcmp ((ev)->_repo_id, ex_DynamicAny_DynAny_TypeMismatch)); \
		CORBA_exception_free (ev); \
	} while (0)
#define CHECK_INVALID_VALUE(ev) \
	do { \
		g_assert ((ev)->_major == CORBA_USER_EXCEPTION && \
		          !strcmp ((ev)->_repo_id, ex_DynamicAny_DynAny_InvalidValue)); \
		CORBA_exception_free (ev); \
	} while (0)
#define CHECK_OBJECT_NOT_EXIST(ev) \
	do { \
		g_assert ((ev)->_major == CORBA_SYSTEM_EXCEPTION && \
		          !strcmp ((ev)->_repo_id, "IDL:CORBA/OBJECT_NOT_EXIST:1.0")); \
		CORBA_exception_free (ev); \
	} while (0)

static int
double_equal (double a, double b)
{
	const double delta = 0.0001;

	if (fabs (a - b) < delta)
		return TRUE;
	else
		return FALSE;
}

static void
test_long (CORBA_ORB orb, CORBA_Environment *ev)
{
	DynamicAny_DynAny   dyn_any;
	CORBA_long     value;
	CORBA_TypeCode type;

	dyn_any = CORBA_ORB_create_basic_dyn_any (orb, TC_long, ev);
	CHECK_OK (ev);
	g_assert (dyn_any != CORBA_OBJECT_NIL);

	/* 1. Inserting */
	DynamicAny_DynAny_insert_long (dyn_any, 2, ev);
	CHECK_OK (ev);

	DynamicAny_DynAny_insert_boolean (dyn_any, TRUE, ev);
	CHECK_TYPE_MISMATCH (ev);

	DynamicAny_DynAny_insert_double (dyn_any, 1.3267, ev);
	CHECK_TYPE_MISMATCH (ev);

	DynamicAny_DynAny_insert_reference (dyn_any, (CORBA_Object) dyn_any, ev);
	CHECK_TYPE_MISMATCH (ev);

	/* 2. Getting */
	DynamicAny_DynAny_get_boolean (dyn_any, ev);
	CHECK_TYPE_MISMATCH (ev);

	DynamicAny_DynAny_get_double (dyn_any, ev);
	CHECK_TYPE_MISMATCH (ev);

	DynamicAny_DynAny_get_reference (dyn_any, ev);
	CHECK_TYPE_MISMATCH (ev);

	value = DynamicAny_DynAny_get_long (dyn_any, ev);
	CHECK_OK (ev);
	g_assert (value == 2);

	type = DynamicAny_DynAny_type (dyn_any, ev);
	CHECK_OK (ev);

	g_assert (CORBA_TypeCode_equal (type, TC_long, ev));
	CHECK_OK (ev);

	CORBA_Object_release ((CORBA_Object) type, ev);
	CHECK_OK (ev);

	CORBA_Object_release ((CORBA_Object) dyn_any, ev);
	CHECK_OK (ev);
}

static void
test_string (CORBA_ORB orb, CORBA_Environment *ev)
{
	DynamicAny_DynAny dyn_any;
	CORBA_char  *value;
	const char  *string = "Hello World";

	dyn_any = CORBA_ORB_create_basic_dyn_any (orb, TC_string, ev);
	CHECK_OK (ev);
	g_assert (dyn_any != CORBA_OBJECT_NIL);

	/* 1. Inserting */
	DynamicAny_DynAny_insert_string (dyn_any, (CORBA_char *)string, ev);
	CHECK_OK (ev);

	DynamicAny_DynAny_insert_boolean (dyn_any, TRUE, ev);
	CHECK_TYPE_MISMATCH (ev);

	DynamicAny_DynAny_insert_double (dyn_any, 1.3267, ev);
	CHECK_TYPE_MISMATCH (ev);

	/* 2. Getting */

	DynamicAny_DynAny_get_boolean (dyn_any, ev);
	CHECK_TYPE_MISMATCH (ev);

	DynamicAny_DynAny_get_double (dyn_any, ev);
	CHECK_TYPE_MISMATCH (ev);

	DynamicAny_DynAny_get_reference (dyn_any, ev);
	CHECK_TYPE_MISMATCH (ev);

	value = DynamicAny_DynAny_get_string (dyn_any, ev);
	CHECK_OK (ev);
	g_assert (!strcmp (value, string));
	CORBA_free (value);

	CORBA_Object_release ((CORBA_Object) dyn_any, ev);
	CHECK_OK (ev);
}

static void
test_copy (CORBA_ORB orb, CORBA_Environment *ev)
{
	DynamicAny_DynAny dyn_any;
	DynamicAny_DynAny dyn_any_copy;
	CORBA_any   *any;
	const char  *string = "Hello World2";

	dyn_any = CORBA_ORB_create_basic_dyn_any (orb, TC_string, ev);
	CHECK_OK (ev);
	g_assert (dyn_any != CORBA_OBJECT_NIL);

	DynamicAny_DynAny_insert_string (dyn_any, (CORBA_char *)string, ev);
	CHECK_OK (ev);

	any = DynamicAny_DynAny_to_any (dyn_any, ev);
	CHECK_OK (ev);
	g_assert (any != NULL);

	dyn_any_copy = CORBA_ORB_create_dyn_any (orb, any, ev);
	CHECK_OK (ev);
	g_assert (dyn_any_copy != NULL);
	CORBA_free (any);

	g_assert (DynamicAny_DynAny_equal (dyn_any_copy, dyn_any, ev));
	CHECK_OK (ev);

	{ /* Knock up an integer any */
		DynamicAny_DynAny int_any = CORBA_ORB_create_basic_dyn_any (
			orb, TC_long, ev);
		CHECK_OK (ev);

		DynamicAny_DynAny_insert_long (int_any, 57, ev);
		CHECK_OK (ev);

		any = DynamicAny_DynAny_to_any (int_any, ev);
		CHECK_OK (ev);

		CORBA_Object_release ((CORBA_Object) int_any, ev);
		CHECK_OK (ev);
	}

	DynamicAny_DynAny_from_any (dyn_any, any, ev);
	CHECK_TYPE_MISMATCH (ev);
	CORBA_free (any);

	DynamicAny_DynAny_assign (dyn_any, dyn_any_copy, ev);
	CHECK_OK (ev);

	g_assert (DynamicAny_DynAny_equal (dyn_any_copy, dyn_any, ev));
	CHECK_OK (ev);

	CORBA_Object_release ((CORBA_Object) dyn_any, ev);
	CHECK_OK (ev);

	CORBA_Object_release ((CORBA_Object) dyn_any_copy, ev);
	CHECK_OK (ev);
}

static const CORBA_TypeCode
octet_subtypes_array [] = {
	TC_octet
};

static const struct CORBA_TypeCode_struct
TC_CORBA_sequence_CORBA_octet_struct = {
      {{(ORBit_RootObject_Interface *) & ORBit_TypeCode_epv, TRUE, -1},
       ORBIT_PSEUDO_TYPECODE},

      CORBA_tk_sequence, NULL, NULL,
      0, 1,
      NULL,
      (CORBA_TypeCode *) octet_subtypes_array,
      NULL,
      CORBA_OBJECT_NIL, 0, -1, 0, 0
};

static void
test_sequence (CORBA_ORB orb, CORBA_Environment *ev)
{
	DynamicAny_DynAny dyn_any;
	int          i, len;

	dyn_any = CORBA_ORB_create_basic_dyn_any (
		orb, (CORBA_TypeCode) &TC_CORBA_sequence_CORBA_octet_struct, ev);
	CHECK_OK (ev);
	g_assert (dyn_any != CORBA_OBJECT_NIL);

	DynamicAny_DynAny_insert_long (dyn_any, 5, ev);
	CHECK_TYPE_MISMATCH (ev);

	DynamicAny_DynSequence_set_length (dyn_any, 100, ev);
	CHECK_OK (ev);

	for (i = 0; i < 100; i++) {
		g_assert (DynamicAny_DynAny_seek (dyn_any, i, ev));
		CHECK_OK (ev);
		DynamicAny_DynAny_insert_octet (dyn_any, 100 - i, ev);
		CHECK_OK (ev);
	}

	len = DynamicAny_DynAny_component_count (dyn_any, ev);
	CHECK_OK (ev);
	g_assert (len == 100);

	len = DynamicAny_DynSequence_get_length (dyn_any, ev);
	CHECK_OK (ev);
	g_assert (len == 100);

	/* Only growing the length for now */
	DynamicAny_DynAny_seek (dyn_any, -1, ev);
	CHECK_OK (ev);

	DynamicAny_DynSequence_set_length (dyn_any, 150, ev);
	CHECK_OK (ev);

	len = DynamicAny_DynSequence_get_length (dyn_any, ev);
	CHECK_OK (ev);
	g_assert (len == 150);

	DynamicAny_DynAny_insert_octet (dyn_any, 137, ev);
	CHECK_OK (ev);

	i = DynamicAny_DynAny_get_octet (dyn_any, ev);
	CHECK_OK (ev);

	g_assert (i == 137);

	DynamicAny_DynSequence_set_length (dyn_any, 200, ev);
	CHECK_OK (ev);

	i = DynamicAny_DynAny_get_octet (dyn_any, ev);
	CHECK_OK (ev);

	g_assert (i == 137);

	len = DynamicAny_DynSequence_get_length (dyn_any, ev);
	CHECK_OK (ev);
	g_assert (len == 200);

	{
		DynamicAny_DynAny_AnySeq *seq;

		seq = DynamicAny_DynSequence_get_elements (dyn_any, ev);
		CHECK_OK (ev);

		DynamicAny_DynSequence_set_elements (dyn_any, seq, ev);
		CHECK_OK (ev);

		CORBA_free (seq);
	}	

	CORBA_Object_release ((CORBA_Object) dyn_any, ev);
	CHECK_OK (ev);
}

static const CORBA_TypeCode
array_subtypes_array [] = {
	TC_double
};

static const struct CORBA_TypeCode_struct
TC_my_array_struct = {
      {{(ORBit_RootObject_Interface *) & ORBit_TypeCode_epv, TRUE, -1},
       ORBIT_PSEUDO_TYPECODE},

      CORBA_tk_array, NULL, NULL,
      100, 1,
      NULL,
      (CORBA_TypeCode *) array_subtypes_array,
      NULL,
      CORBA_OBJECT_NIL, 0, -1, 0, 0
};

static void
test_array (CORBA_ORB orb, CORBA_Environment *ev)
{
	DynamicAny_DynAny dyn_any;
	CORBA_double d;
	int          i;

	dyn_any = CORBA_ORB_create_basic_dyn_any (
		orb, (CORBA_TypeCode) &TC_my_array_struct, ev);
	CHECK_OK (ev);
	g_assert (dyn_any != CORBA_OBJECT_NIL);

	g_assert (DynamicAny_DynAny_seek (dyn_any, 99, ev));
	CHECK_OK (ev);

	DynamicAny_DynAny_insert_long (dyn_any, 2, ev);
	CHECK_TYPE_MISMATCH (ev);

	DynamicAny_DynAny_insert_double (dyn_any, 2.71828182845, ev);
	CHECK_OK (ev);

	/* 1. Insert */
	for (i = 0; i < 37; i+= 3) {
		d = i * 2.7 + 3.1;

		DynamicAny_DynAny_seek (dyn_any, i, ev);
		CHECK_OK (ev);

		DynamicAny_DynAny_insert_double (dyn_any, d, ev);
		CHECK_OK (ev);
	}

	/* 1. Extract */
	for (i = 0; i < 37; i+= 3) {
		d = i * 2.7 + 3.1;

		DynamicAny_DynAny_seek (dyn_any, i, ev);
		CHECK_OK (ev);

		g_assert (double_equal (DynamicAny_DynAny_get_double (dyn_any, ev), d));
		CHECK_OK (ev);
	}

	CORBA_Object_release ((CORBA_Object) dyn_any, ev);
	CHECK_OK (ev);
}

static const char *
enum_subnames_array [] = {
	"KIPPER",
	"BLOATER",
	"HERRING"
};

const struct CORBA_TypeCode_struct
TC_org_fish_packers_Fishy_struct = {
   
	{{(ORBit_RootObject_Interface *) & ORBit_TypeCode_epv, TRUE, -1},
	 ORBIT_PSEUDO_TYPECODE},
	
	CORBA_tk_enum, "Fishy", "IDL:org/fish/packers/Fishy:1.0",
	0, 3,
	(const char **) enum_subnames_array,
	NULL,
	NULL,
	CORBA_OBJECT_NIL, 0, -1, 0, 0
};

static void
test_enum (CORBA_ORB orb, CORBA_Environment *ev)
{
	DynamicAny_DynAny dyn_any;
	int          i;

	dyn_any = CORBA_ORB_create_basic_dyn_any (
		orb, (CORBA_TypeCode) &TC_org_fish_packers_Fishy_struct, ev);
	CHECK_OK (ev);
	g_assert (dyn_any != CORBA_OBJECT_NIL);

	i = DynamicAny_DynEnum_get_as_ulong (dyn_any, ev);
	CHECK_OK (ev);
	g_assert (i == 0);

	for (i = 0; i < sizeof (enum_subnames_array) / sizeof (const char *); i++) {

		DynamicAny_DynEnum_set_as_string (dyn_any, enum_subnames_array [i], ev);
		CHECK_OK (ev);

		g_assert (DynamicAny_DynEnum_get_as_ulong (dyn_any, ev) == i);
		CHECK_OK (ev);
	}

	for (i = 0; i < sizeof (enum_subnames_array) / sizeof (const char *); i++) {
		CORBA_char  *str;

		DynamicAny_DynEnum_set_as_ulong (dyn_any, i, ev);
		CHECK_OK (ev);

		str = DynamicAny_DynEnum_get_as_string (dyn_any, ev);
		CHECK_OK (ev);

		g_assert (!strcmp (str, enum_subnames_array [i]));
		CORBA_free (str);
	}

	CORBA_Object_release ((CORBA_Object) dyn_any, ev);
	CHECK_OK (ev);
}

static const char *
union_subnames_array [] = {
	"tgw",
	"nut",
	"atl",
	"rmt",
	"ibid"
};

static CORBA_unsigned_long 
union_sublabel_values_array [] = {
	0, 1, 2, 3, 4
};

static const CORBA_TypeCode
union_subtypes_array [] = {
	TC_long,
	TC_double,
	TC_string,
	(CORBA_TypeCode) & TC_org_fish_packers_Fishy_struct,
	(CORBA_TypeCode) & TC_CORBA_sequence_CORBA_octet_struct
};

static const CORBA_any
union_sublabels_array [] = {
	{ (CORBA_TypeCode) TC_CORBA_unsigned_long, 
	  &union_sublabel_values_array [0], CORBA_FALSE },
	{ (CORBA_TypeCode) TC_CORBA_unsigned_long,
	  &union_sublabel_values_array [1], CORBA_FALSE },
	{ (CORBA_TypeCode) TC_CORBA_unsigned_long,
	  &union_sublabel_values_array [2], CORBA_TRUE  },
	{ (CORBA_TypeCode) TC_CORBA_unsigned_long,
	  &union_sublabel_values_array [3], CORBA_FALSE },
	{ (CORBA_TypeCode) TC_CORBA_unsigned_long,
	  &union_sublabel_values_array [4], CORBA_FALSE },
};

const struct CORBA_TypeCode_struct
TC_England_Unions_UnionValue_struct = {
   
      {{(ORBit_RootObject_Interface *) & ORBit_TypeCode_epv, TRUE, -1},
       ORBIT_PSEUDO_TYPECODE},

      CORBA_tk_union, "UnionValue", "IDL:England/Unions/UnionValue:1.0",
      0, 5,
      (const char **)    union_subnames_array,
      (CORBA_TypeCode *) union_subtypes_array,
      (CORBA_any *)      union_sublabels_array,
      (CORBA_TypeCode)   TC_CORBA_unsigned_long,
      0, -1, 0, 0
};

const struct CORBA_TypeCode_struct
TC_England_Unions_Struct_struct = {
   
      {{(ORBit_RootObject_Interface *) & ORBit_TypeCode_epv, TRUE, -1},
       ORBIT_PSEUDO_TYPECODE},

      CORBA_tk_struct, "Struct", "IDL:England/Unions/Struct:1.0",
      0, 5,
      (const char **)    union_subnames_array,
      (CORBA_TypeCode *) union_subtypes_array,
      NULL,
      CORBA_OBJECT_NIL, 0, -1, 0, 0
};

static void
test_union (CORBA_ORB orb, CORBA_Environment *ev)
{
	DynamicAny_DynAny dyn_any;
	CORBA_TCKind kind;

	dyn_any = CORBA_ORB_create_basic_dyn_any (
		orb, (CORBA_TypeCode) &TC_England_Unions_UnionValue_struct, ev);
	CHECK_OK (ev);
	g_assert (dyn_any != CORBA_OBJECT_NIL);

	kind = DynamicAny_DynUnion_discriminator_kind (dyn_any, ev);
	CHECK_OK (ev);
	g_assert (kind == CORBA_tk_ulong);

	CORBA_Object_release ((CORBA_Object) dyn_any, ev);
	CHECK_OK (ev);
}

static void
test_struct (CORBA_ORB orb, CORBA_Environment *ev)
{
	DynamicAny_DynAny dyn_any, sub_any;
	CORBA_TCKind kind;
	CORBA_char  *str;
	CORBA_double dv = 1.23;
	const char  *test_str = "one is not amused";
	int i;

	dyn_any = CORBA_ORB_create_basic_dyn_any (
		orb, (CORBA_TypeCode) &TC_England_Unions_Struct_struct, ev);
	CHECK_OK (ev);
	g_assert (dyn_any != CORBA_OBJECT_NIL);

	g_assert (!DynamicAny_DynStruct_seek (dyn_any, 6, ev));
	CHECK_OK (ev);

	for (i = 0; i < 5; i++) {
		DynamicAny_DynStruct_seek (dyn_any, i, ev);
		CHECK_OK (ev);

		kind = DynamicAny_DynStruct_current_member_kind (dyn_any, ev);
		CHECK_OK (ev);

		g_assert (union_subtypes_array [i]->kind == kind);

		str = DynamicAny_DynStruct_current_member_name (dyn_any, ev);
		CHECK_OK (ev);

		g_assert (!strcmp (union_subnames_array [i], str));
		CORBA_free (str);
	}

	g_assert (DynamicAny_DynStruct_seek (dyn_any, 0, ev));
	CHECK_OK (ev);

	DynamicAny_DynAny_insert_long (dyn_any, 345, ev);
	CHECK_OK (ev);

	g_assert (DynamicAny_DynAny_next (dyn_any, ev));
	CHECK_OK (ev);

	DynamicAny_DynAny_insert_double (dyn_any, dv, ev);
	CHECK_OK (ev);

	g_assert (DynamicAny_DynAny_next (dyn_any, ev));
	CHECK_OK (ev);

	DynamicAny_DynAny_insert_string (dyn_any, test_str, ev);
	CHECK_OK (ev);

	sub_any = DynamicAny_DynAny_current_component (dyn_any, ev);
	CHECK_OK (ev);

	str = DynamicAny_DynAny_get_string (sub_any, ev);
	CHECK_OK (ev);
	
	g_assert (!strcmp (test_str, str));

	CORBA_free (str);

	g_assert (DynamicAny_DynStruct_seek (dyn_any, 0, ev));
	CHECK_OK (ev);

	g_assert (DynamicAny_DynAny_get_long (dyn_any, ev) == 345);
	CHECK_OK (ev);

	g_assert (DynamicAny_DynAny_next (dyn_any, ev));
	CHECK_OK (ev);

	g_assert (double_equal (DynamicAny_DynAny_get_double (dyn_any, ev), dv));
	CHECK_OK (ev);

	g_assert (DynamicAny_DynAny_next (dyn_any, ev));
	CHECK_OK (ev);

	CORBA_Object_release ((CORBA_Object) dyn_any, ev);
	CHECK_OK (ev);

	str = DynamicAny_DynAny_get_string (sub_any, ev);
	CHECK_OBJECT_NOT_EXIST (ev);

	CORBA_Object_release ((CORBA_Object) sub_any, ev);
	CHECK_OK (ev);
}

int
main (int argc, char *argv[])
{
	CORBA_Environment ev;
	CORBA_ORB    orb;

	free (malloc (8));

	CORBA_exception_init (&ev);
	orb = CORBA_ORB_init (&argc, argv, "orbit-local-orb", &ev);
  
	/*
	 *  Since the API is entirely macro generated
	 * we only need to test a few cases.
	 */
	test_long     (orb, &ev);
	test_string   (orb, &ev);
	test_copy     (orb, &ev);
	test_sequence (orb, &ev);
	test_enum     (orb, &ev);
	test_union    (orb, &ev);
	test_array    (orb, &ev);
	test_struct   (orb, &ev);

	CORBA_Object_release ((CORBA_Object) orb, &ev);
	CHECK_OK (&ev);
  
	printf ("all DynAny tests passed ok.\n");

	return 0;
}
