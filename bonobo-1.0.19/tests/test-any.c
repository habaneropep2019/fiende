#include <config.h>
#include <gnome.h>
#include <liboaf/liboaf.h>

#include <gdk/gdkprivate.h>
#include <gdk/gdkx.h>
#include <orb/orbit.h>
#include <bonobo.h>
#include <bonobo/bonobo-property-bag-xml.h>

#define CHECK_OK(ev) g_assert ((ev)->_major == CORBA_NO_EXCEPTION)

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

int
main (int argc, char *argv [])
{
	DynamicAny_DynAny dyn_any;
	CORBA_Environment real_ev, *ev;
	CORBA_any   *any, *any2;
	CORBA_ORB    orb;
	const char  *test_str = "one is not amused";

	free (malloc (8));

	fprintf (stderr, "Testing DynamicAny\n");

	ev = &real_ev;
	CORBA_exception_init (ev);

	gtk_type_init ();

	orb = oaf_init (argc, argv);
	
	if (bonobo_init (orb, NULL, NULL) == FALSE)
		g_error ("Can not bonobo_init");
	
	dyn_any = CORBA_ORB_create_basic_dyn_any (
		orb, (CORBA_TypeCode) &TC_England_Unions_Struct_struct, ev);
	CHECK_OK (ev);
	g_assert (dyn_any != CORBA_OBJECT_NIL);

	g_assert (DynamicAny_DynStruct_seek (dyn_any, 0, ev));
	CHECK_OK (ev);

	DynamicAny_DynAny_insert_long (dyn_any, 345, ev);
	CHECK_OK (ev);

	g_assert (DynamicAny_DynAny_next (dyn_any, ev));
	CHECK_OK (ev);

	DynamicAny_DynAny_insert_double (dyn_any, 1.245, ev);
	CHECK_OK (ev);

	g_assert (DynamicAny_DynAny_next (dyn_any, ev));
	CHECK_OK (ev);

	DynamicAny_DynAny_insert_string (dyn_any, test_str, ev);
	CHECK_OK (ev);

	any = DynamicAny_DynAny_to_any (dyn_any, ev);
	CHECK_OK (ev);

	{
		BonoboUINode *node;
		char *str;

		node = bonobo_property_bag_xml_encode_any (NULL, any, ev);
		str = bonobo_ui_node_to_string (node, TRUE);

		printf ("'%s'\n", str);
		bonobo_ui_node_free_string (str);

		any2 = bonobo_property_bag_xml_decode_any (node, ev);

		g_assert (any2);
		g_assert (ORBit_any_equivalent (any, any2, ev));
		CHECK_OK (ev);

		CORBA_free (any2);
		CORBA_free (any);

		bonobo_ui_node_free (node);
	}

	CORBA_Object_release ((CORBA_Object) dyn_any, ev);
	CHECK_OK (ev);

	fprintf (stderr, "Testing BonoboArg\n");
	{
		int            i;
		CORBA_TypeCode tcs [] = {
			TC_null,
			TC_void,
			TC_string,
			TC_short,
			TC_long,
			TC_ushort,
			TC_ulong,
			TC_float,
			TC_double,
			TC_longdouble,
			TC_boolean,
			TC_char,
			TC_wchar,
			TC_octet,
			TC_any,
			TC_TypeCode,
			TC_Object,
			TC_wstring,
			TC_longlong,
			TC_ulonglong,
			(CORBA_TypeCode) & TC_org_fish_packers_Fishy_struct,
			(CORBA_TypeCode) & TC_CORBA_sequence_CORBA_octet_struct,
			(CORBA_TypeCode) &TC_England_Unions_Struct_struct
		};
		for (i = 0; i < sizeof (tcs)/sizeof(CORBA_TypeCode); i++) {
			BonoboArg *arg;

			arg = bonobo_arg_new (tcs [i]);
			bonobo_arg_release (arg);
		}
	}

	return 0;
}
