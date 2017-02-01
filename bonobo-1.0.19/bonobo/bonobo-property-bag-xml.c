/*
 * Some XML-based BonoboPropertyBag persistence helpers.
 *
 * Authors:
 *   Michael Meeks  (michael@ximian.com)
 *   Dietmar Maurer (dietmar@ximian.com)
 *
 * Copyright 2000 Ximian, Inc.
 */
#include <config.h>
#include <stdlib.h>
#include <gtk/gtksignal.h>
#include <gtk/gtkmarshal.h>
#include <bonobo/bonobo-stream-client.h>
#include <gnome-xml/tree.h>
#include <gnome-xml/parser.h>
#include <bonobo/bonobo-property-bag.h>
#include <bonobo/bonobo-property-bag-xml.h>
#include <bonobo/bonobo-ui-node.h>

/*
  A rough tag outline:

  Ignore Fixed, Unions for now; they suck badly.

  <any>
	<type name="%s" repo_id="%s" tckind="%d" length="%d"
	sub_parts="%d" recurse_depth="%d" default_index="%d">
		<subnames>
			<name>MyName</name>
			<name>Fish</name>
		</subnames>
		<subtypes>
			<type.../>
		</subtypes>
	</type>

	<value length="3">
		<value>2.5</value>
		<value>1</value>
		<value>hello</value>
	</value>
  </any>
 */

static gpointer
ORBit_demarshal_allocate_mem(CORBA_TypeCode tc, gint nelements)
{
	size_t block_size;
	gpointer retval = NULL;

	if (!nelements)
		return retval;

	block_size = ORBit_gather_alloc_info(tc);

	if (block_size) {
		retval = ORBit_alloc_2 (block_size * nelements,
				       (ORBit_free_childvals) ORBit_free_via_TypeCode,
				       GINT_TO_POINTER (nelements),
				       sizeof (CORBA_TypeCode));

		*(CORBA_TypeCode *)((char *)retval-sizeof(ORBit_mem_info)-sizeof(CORBA_TypeCode)) = (CORBA_TypeCode)CORBA_Object_duplicate((CORBA_Object)tc, NULL);
	}

	return retval;
}

static void
encode_type (BonoboUINode      *type_parent,
	     CORBA_TypeCode     tc,
	     CORBA_Environment *ev);

static void
encode_subtypes (BonoboUINode      *parent,
		 CORBA_TypeCode     tc,
		 int                num_subtypes,
		 CORBA_Environment *ev)
{
	BonoboUINode *subtypes;
	int           i;

	subtypes = bonobo_ui_node_new_child (parent, "subtypes");

	for (i = 0; i < num_subtypes; i++)
		encode_type (subtypes, tc->subtypes [i], ev);
}

static void
encode_type (BonoboUINode      *type_parent,
	     CORBA_TypeCode     tc,
	     CORBA_Environment *ev)
{
	BonoboUINode *node;
	char scratch [128];
	int  i;

	node = bonobo_ui_node_new_child (type_parent, "type");

	if (tc->name)
		bonobo_ui_node_set_attr (node, "name", tc->name);
	
	if (tc->repo_id)
		bonobo_ui_node_set_attr (node, "repo_id", tc->repo_id);

	snprintf (scratch, 127, "%d", tc->kind);
	bonobo_ui_node_set_attr (node, "tckind", scratch);

	snprintf (scratch, 127, "%u", tc->length);
	bonobo_ui_node_set_attr (node, "length", scratch);

	snprintf (scratch, 127, "%u", tc->sub_parts);
	bonobo_ui_node_set_attr (node, "sub_parts", scratch);

	switch (tc->kind) {
 	case CORBA_tk_struct:
	case CORBA_tk_union:
	case CORBA_tk_enum:
	case CORBA_tk_except: { /* subnames */
		BonoboUINode *subnames;

		subnames = bonobo_ui_node_new_child (node, "subnames");

		for (i = 0; i < tc->sub_parts; i++) {
			BonoboUINode *subname;
			subname = bonobo_ui_node_new_child (subnames, "name");
			bonobo_ui_node_set_content (subname, tc->subnames [i]);
		}
	}
	if (tc->kind != CORBA_tk_enum)
		encode_subtypes (node, tc, tc->sub_parts, ev);
	break;

	case CORBA_tk_alias:
	case CORBA_tk_array:
	case CORBA_tk_sequence:
		encode_subtypes (node, tc, 1, ev);
		break;

	default:
		break;
	}
}

#define DO_ENCODE(tckind,format,corbatype,value,align)			\
	case tckind:							\
		*value = ALIGN_ADDRESS (*value, align);			\
		snprintf (scratch, 127, format,				\
			  * (corbatype *) *value);			\
		*value = ((guchar *)*value) + sizeof (corbatype);	\
		break;

static void
encode_value (BonoboUINode      *parent,
	      CORBA_TypeCode     tc,
	      gpointer          *value,
	      CORBA_Environment *ev)
{
	BonoboUINode *node;
	char scratch [256] = "";
	int  i;

	node = bonobo_ui_node_new_child (parent, "value");

	switch (tc->kind) {
	case CORBA_tk_null:
	case CORBA_tk_void:
		break;

		DO_ENCODE (CORBA_tk_short,  "%d", CORBA_short, value, ALIGNOF_CORBA_SHORT);
		DO_ENCODE (CORBA_tk_ushort, "%u", CORBA_unsigned_short, value, ALIGNOF_CORBA_SHORT);
		DO_ENCODE (CORBA_tk_long,   "%d", CORBA_long, value, ALIGNOF_CORBA_LONG);
		DO_ENCODE (CORBA_tk_enum,   "%d", CORBA_enum, value, ALIGNOF_CORBA_LONG);
		DO_ENCODE (CORBA_tk_ulong,  "%u", CORBA_unsigned_long, value, ALIGNOF_CORBA_LONG);

		DO_ENCODE (CORBA_tk_float,   "%g", CORBA_float,   value, ALIGNOF_CORBA_FLOAT);
		DO_ENCODE (CORBA_tk_double,  "%g", CORBA_double,  value, ALIGNOF_CORBA_DOUBLE);
		DO_ENCODE (CORBA_tk_boolean, "%d", CORBA_boolean, value, 1);

		DO_ENCODE (CORBA_tk_char,  "%d", CORBA_char, value,  1);
		DO_ENCODE (CORBA_tk_octet, "%d", CORBA_octet, value, 1);

		DO_ENCODE (CORBA_tk_wchar, "%d", CORBA_wchar, value, ALIGNOF_CORBA_SHORT);

/*
#ifdef G_HAVE_GINT64
		DO_ENCODE (CORBA_tk_longlong,   "%ll",  CORBA_long_long, value);
		DO_ENCODE (CORBA_tk_ulonglong,  "%ull", CORBA_unsigned_long_long, value);
#endif
		DO_ENCODE (CORBA_tk_longdouble, "%L", CORBA_long_double, value);
*/

	case CORBA_tk_string:
	case CORBA_tk_wstring:
		*value = ALIGN_ADDRESS(*value, ALIGNOF_CORBA_POINTER);
		bonobo_ui_node_set_content (node, *(CORBA_char **) *value);
		*value = ((guchar *)*value) + sizeof (CORBA_char *);
		break;

	case CORBA_tk_objref:
		g_warning ("Cannot serialize an objref");
		break;

	case CORBA_tk_TypeCode:
		*value = ALIGN_ADDRESS (*value, ALIGNOF_CORBA_POINTER);
		encode_type (node, * (CORBA_TypeCode *) *value, ev);
		*value = ((guchar *)*value) + sizeof(CORBA_TypeCode);
		break;

	case CORBA_tk_any:
		*value = ALIGN_ADDRESS (*value,
				       MAX (ALIGNOF_CORBA_LONG,
					   MAX (ALIGNOF_CORBA_POINTER, ALIGNOF_CORBA_STRUCT)));
		bonobo_property_bag_xml_encode_any (node, (CORBA_any *) *value, ev);
		*value = ((guchar *)*value) + sizeof (CORBA_any);
		break;

	case CORBA_tk_sequence: {
		CORBA_sequence_octet *sval = *value;
		gpointer subval;

		*value = ALIGN_ADDRESS (*value,
					MAX(MAX(ALIGNOF_CORBA_LONG, ALIGNOF_CORBA_STRUCT), ALIGNOF_CORBA_POINTER));

		snprintf (scratch, 127, "%d", sval->_length);
		bonobo_ui_node_set_attr (node, "length", scratch);

		subval = sval->_buffer;

		for (i = 0; i < sval->_length; i++)
			encode_value (node, tc->subtypes [0], &subval, ev);;
	    
		*value = ((guchar *)*value) + sizeof (CORBA_sequence_octet);
		scratch [0] = '\0';
		break;
	}

	case CORBA_tk_array:
		for (i = 0; i < tc->length; i++)
			encode_value (node, tc->subtypes [0], value, ev);
		break;

 	case CORBA_tk_struct:
	case CORBA_tk_except:
		for (i = 0; i < tc->sub_parts; i++)
			encode_value (node, tc->subtypes [i], value, ev);
		break;

	case CORBA_tk_alias:
		encode_value (node, tc->subtypes [0], value, ev);
		break;

	case CORBA_tk_union:
	case CORBA_tk_Principal:
	case CORBA_tk_fixed:
	case CORBA_tk_recursive:
	default:
		g_warning ("Unhandled kind '%d'", tc->kind);
		break;
	}

	if (scratch [0])
		bonobo_ui_node_set_content (node, scratch);
}

/**
 * bonobo_property_bag_xml_encode_any:
 * @opt_parent: optional parent, should be NULL
 * @any: the Any to serialize
 * @ev: a corba exception environment
 * 
 * This routine encodes @any into an XML tree using the
 * #BonoboUINode XML abstraction. @ev is used for flagging
 * any non-fatal exceptions during the process. On exception
 * NULL will be returned. opt_parent should be NULL, and is
 * used internally for recursive tree construction.
 *
 * Both type and content data are dumped in a non-standard, but
 * trivial format.
 * 
 * Return value: the XML tree representing the Any
 **/
BonoboUINode *
bonobo_property_bag_xml_encode_any (BonoboUINode      *opt_parent,
				    const CORBA_any   *any,
				    CORBA_Environment *ev)
{
	BonoboUINode *node;
	gpointer      value;

	g_return_val_if_fail (any != NULL, NULL);

	if (opt_parent)
		node = bonobo_ui_node_new_child (opt_parent, "any");
	else
		node = bonobo_ui_node_new ("any");

	value = any->_value;

	encode_type  (node, any->_type, ev);
	encode_value (node, any->_type, &value, ev);

	return node;
}

static CORBA_TypeCode
decode_type (BonoboUINode      *node,
	     CORBA_Environment *ev);

static gboolean
decode_subtypes_into (BonoboUINode      *parent,
		      CORBA_TypeCode     tc,
		      int                num_subtypes,
		      CORBA_Environment *ev)
{
	BonoboUINode *l, *subtypes = NULL;
	int           i = 0;

	for (l = bonobo_ui_node_children (parent); l;
	     l = bonobo_ui_node_next (l)) {
		if (bonobo_ui_node_has_name (l, "subtypes"))
			subtypes = l;
	}
	if (!subtypes) {
		g_warning ("Missing subtypes field - leak");
		return FALSE;
	}

	tc->subtypes = g_new (CORBA_TypeCode, num_subtypes);

	for (l = bonobo_ui_node_children (subtypes); l;
	     l = bonobo_ui_node_next (l)) {

		if (i >= num_subtypes)
			g_warning ("Too many sub types should be %d", num_subtypes);
		else {
			tc->subtypes [i] = decode_type (l, ev);
			g_assert (tc->subtypes [i]);
		}
		i++;
	}

	if (i < num_subtypes) {
		g_warning ("Not enough sub names: %d should be %d", i, num_subtypes);
		return FALSE;
	}

	return TRUE;
}

static CORBA_TypeCode
decode_type (BonoboUINode      *node,
	     CORBA_Environment *ev)
{
	CORBA_TypeCode tc;
	BonoboUINode  *l;
	CORBA_TCKind   kind;
	char *txt;

	if ((txt = bonobo_ui_node_get_attr (node, "tckind"))) {
		kind = atoi (txt);
		bonobo_ui_node_free_string (txt);
	} else {
		g_warning ("Format error no tckind");
		return NULL;
	}
	
	switch (kind) {
#define HANDLE_SIMPLE_TYPE(tc,kind)		\
	case CORBA_tk_##kind:			\
		return tc;

	HANDLE_SIMPLE_TYPE (TC_string,             string);
	HANDLE_SIMPLE_TYPE (TC_short,              short);
	HANDLE_SIMPLE_TYPE (TC_long,               long);
	HANDLE_SIMPLE_TYPE (TC_ushort,             ushort);
	HANDLE_SIMPLE_TYPE (TC_ulong,              ulong);
	HANDLE_SIMPLE_TYPE (TC_float,              float);
	HANDLE_SIMPLE_TYPE (TC_double,             double);
	HANDLE_SIMPLE_TYPE (TC_longdouble,         longdouble);
	HANDLE_SIMPLE_TYPE (TC_boolean,            boolean);
	HANDLE_SIMPLE_TYPE (TC_char,               char);
	HANDLE_SIMPLE_TYPE (TC_wchar,              wchar);
	HANDLE_SIMPLE_TYPE (TC_octet,              octet);
	HANDLE_SIMPLE_TYPE (TC_any,                any);
	HANDLE_SIMPLE_TYPE (TC_wstring,            wstring);
	HANDLE_SIMPLE_TYPE (TC_longlong,           longlong); 
	HANDLE_SIMPLE_TYPE (TC_ulonglong,          ulonglong);

	default:
		break;
	}

	tc = g_new0 (struct CORBA_TypeCode_struct, 1);
	tc->kind = kind;

	/* Passing in NULL for CORBA_Environment is patently dangerous. */
	ORBit_pseudo_object_init ((ORBit_PseudoObject) tc,
				  ORBIT_PSEUDO_TYPECODE, NULL);
	ORBit_RootObject_set_interface ((ORBit_RootObject) tc,
					(ORBit_RootObject_Interface *) &ORBit_TypeCode_epv,
					NULL);
	/* set refs to 1 */
	CORBA_Object_duplicate ((CORBA_Object)tc, NULL);

	if ((txt = bonobo_ui_node_get_attr (node, "name"))) {
		tc->name = g_strdup (txt);
		bonobo_ui_node_free_string (txt);
	}

	if ((txt = bonobo_ui_node_get_attr (node, "repo_id"))) {
		tc->repo_id = g_strdup (txt);
		bonobo_ui_node_free_string (txt);
	}

	if ((txt = bonobo_ui_node_get_attr (node, "length"))) {
		tc->length = atoi (txt);
		bonobo_ui_node_free_string (txt);
	} else
		g_warning ("Format error no length");

	if ((txt = bonobo_ui_node_get_attr (node, "sub_parts"))) {
		tc->sub_parts = atoi (txt);
		bonobo_ui_node_free_string (txt);
	} else
		g_warning ("Format error no sub_parts");

	switch (tc->kind) {
 	case CORBA_tk_struct:
	case CORBA_tk_union:
	case CORBA_tk_enum:
	case CORBA_tk_except: { /* subnames */
		BonoboUINode *subnames = NULL;
		int           i = 0;

		for (l = bonobo_ui_node_children (node); l;
		     l = bonobo_ui_node_next (l)) {
			if (bonobo_ui_node_has_name (l, "subnames"))
				subnames = l;
		}
		if (!subnames) {
			g_warning ("Missing subnames field - leak");
			goto decode_error;
		}

		tc->subnames = (const char **) g_new (char *, tc->sub_parts);

		for (l = bonobo_ui_node_children (subnames); l;
		     l = bonobo_ui_node_next (l)) {
			if (i >= tc->sub_parts)
				g_warning ("Too many sub names should be %d", tc->sub_parts);
			else {
				char *txt = bonobo_ui_node_get_content (l);
				tc->subnames [i++] = g_strdup (txt);
				bonobo_ui_node_free_string (txt);
			}
		}
		if (i < tc->sub_parts) {
			g_warning ("Not enough sub names: %d should be %d", i, tc->sub_parts);
			goto decode_error;
		}
	}
	if (tc->kind != CORBA_tk_enum)
		if (!decode_subtypes_into (node, tc, tc->sub_parts, ev))
			goto decode_error;
	break;

	case CORBA_tk_alias:
	case CORBA_tk_array:
	case CORBA_tk_sequence:
		if (!decode_subtypes_into (node, tc, 1, ev))
			goto decode_error;
		break;

	default:
		break;
	}

	return tc;

 decode_error:
	CORBA_Object_release ((CORBA_Object) tc, ev);
	return NULL;
}

#define DO_DECODE(tckind,format,corbatype,value,align)				\
	case tckind:								\
		*value = ALIGN_ADDRESS (*value, align);				\
		if (!scratch)							\
			g_warning ("Null content");				\
		else if (sscanf (scratch, format, (corbatype *) *value) != 1)	\
			g_warning ("Broken scanf on '%s'", scratch);		\
		*value = ((guchar *)*value) + sizeof (corbatype);		\
		break;

#define DO_DECODEI(tckind,format,corbatype,value,align)				\
	case tckind: {								\
		CORBA_unsigned_long i;						\
		*value = ALIGN_ADDRESS (*value, align);				\
		if (!scratch)							\
			g_warning ("Null content");				\
		else if (sscanf (scratch, format, &i) != 1)			\
			g_warning ("Broken scanf on '%s'", scratch);		\
		*(corbatype *) *value = i;					\
		*value = ((guchar *)*value) + sizeof (corbatype);		\
		break;								\
	}

static void
decode_value (BonoboUINode      *node,
	      CORBA_TypeCode     tc,
	      gpointer          *value,
	      CORBA_Environment *ev)
{
	BonoboUINode *l;
	char *scratch;
	int   i;

	scratch = bonobo_ui_node_get_content (node);

	switch (tc->kind) {
	case CORBA_tk_null:
	case CORBA_tk_void:
		break;

		DO_DECODEI (CORBA_tk_short,  "%d", CORBA_short, value, ALIGNOF_CORBA_SHORT);
		DO_DECODEI (CORBA_tk_ushort, "%u", CORBA_unsigned_short, value, ALIGNOF_CORBA_SHORT);
		DO_DECODEI (CORBA_tk_long,   "%d", CORBA_long, value, ALIGNOF_CORBA_LONG);
		DO_DECODEI (CORBA_tk_enum,   "%d", CORBA_enum, value, ALIGNOF_CORBA_LONG);
		DO_DECODEI (CORBA_tk_ulong,  "%u", CORBA_unsigned_long, value, ALIGNOF_CORBA_LONG);
		DO_DECODEI (CORBA_tk_boolean, "%d", CORBA_boolean, value, 1);
		DO_DECODEI (CORBA_tk_char,    "%d", CORBA_char, value,  1);
		DO_DECODEI (CORBA_tk_octet,   "%d", CORBA_octet, value, 1);
		DO_DECODEI (CORBA_tk_wchar,   "%d", CORBA_wchar, value, ALIGNOF_CORBA_SHORT);

		DO_DECODE (CORBA_tk_float,   "%g",  CORBA_float,   value, ALIGNOF_CORBA_FLOAT);
		DO_DECODE (CORBA_tk_double,  "%lg", CORBA_double,  value, ALIGNOF_CORBA_DOUBLE);

/*
#ifdef G_HAVE_GINT64
		DO_DECODE (CORBA_tk_longlong,   "%ll",  CORBA_long_long, value);
		DO_DECODE (CORBA_tk_ulonglong,  "%ull", CORBA_unsigned_long_long, value);
#endif
		DO_DECODE (CORBA_tk_longdouble, "%L", CORBA_long_double, value);
*/

	case CORBA_tk_string:
	case CORBA_tk_wstring:
		*value = ALIGN_ADDRESS (*value, ALIGNOF_CORBA_POINTER);
		if (scratch)
			*(CORBA_char **) *value = CORBA_string_dup (scratch);
		else 
			*(CORBA_char **) *value = CORBA_string_dup ("");
		*value = ((guchar *)*value) + sizeof (CORBA_char *);
		break;

	case CORBA_tk_objref:
		g_warning ("Error objref in stream");
		break;

	case CORBA_tk_TypeCode:
		*value = ALIGN_ADDRESS (*value, ALIGNOF_CORBA_POINTER);
		*(CORBA_TypeCode *) *value = decode_type (node, ev);
		*value = ((guchar *) *value) + sizeof (CORBA_TypeCode);
		break;

	case CORBA_tk_any:
		*value = ALIGN_ADDRESS (*value,
				       MAX (ALIGNOF_CORBA_LONG,
					    MAX (ALIGNOF_CORBA_POINTER, ALIGNOF_CORBA_STRUCT)));
		*(CORBA_any **)*value = bonobo_property_bag_xml_decode_any (node, ev);
		*value = ((guchar *) *value) + sizeof (CORBA_any);
		break;

	case CORBA_tk_sequence: {
		CORBA_sequence_octet *sval = *value;
		gpointer subval;
		char *txt = bonobo_ui_node_get_attr (node, "length");

		if (!txt) {
			g_warning ("No length on sequence");
			break;
		}

		sval->_length = atoi (txt);
		sval->_maximum = tc->length;
		if (sval->_maximum && sval->_maximum <= sval->_length)
			g_warning ("Sequence too long");

		sval->_buffer = sval->_length ? ORBit_demarshal_allocate_mem (
			tc->subtypes [0], sval->_length) : NULL;

		*value = ALIGN_ADDRESS (*value,
				      MAX(MAX(ALIGNOF_CORBA_LONG, ALIGNOF_CORBA_STRUCT), ALIGNOF_CORBA_POINTER));

		subval = sval->_buffer;
		i = 0;
		for (l = bonobo_ui_node_children (node); l;
		     l = bonobo_ui_node_next (l)) {
			if (i < sval->_length)
				decode_value (l, tc->subtypes [0], &subval, ev);
			else
				g_warning ("Too many sequence elements %d", i);
			i++;
		}
		if (i < sval->_length)
			g_warning ("Not enough sequence elements: %d should be %d",
				   i, tc->length);

		bonobo_ui_node_free_string (txt);
		*value = ((guchar *)*value) + sizeof (CORBA_sequence_octet);
		break;
	}

	case CORBA_tk_array:
		i = 0;
		for (l = bonobo_ui_node_children (node); l;
		     l = bonobo_ui_node_next (l)) {
			if (i < tc->length)
				decode_value (l, tc->subtypes [0], value, ev);
			else
				g_warning ("Too many elements %d", tc->length);
			i++;
		}
		if (i < tc->length)
			g_warning ("Not enough elements: %d should be %d",
				   i, tc->length);
		break;

 	case CORBA_tk_struct:
	case CORBA_tk_except:
		i = 0;
		for (l = bonobo_ui_node_children (node); l;
		     l = bonobo_ui_node_next (l)) {
			if (i < tc->sub_parts)
				decode_value (l, tc->subtypes [i], value, ev);
			else
				g_warning ("Too many structure elements %d",
					   tc->sub_parts);
			i++;
		}
		if (i < tc->sub_parts)
			g_warning ("Not enough structure elements: %d should be %d",
				   i, tc->sub_parts);
		break;

	case CORBA_tk_alias:
		l = bonobo_ui_node_children (node);
		decode_value (l, tc->subtypes [0], value, ev);
		break;

	case CORBA_tk_union:
	case CORBA_tk_Principal:
	case CORBA_tk_fixed:
	case CORBA_tk_recursive:
	default:
		g_warning ("Unhandled");
		break;
	}

	bonobo_ui_node_free_string (scratch);
}

/**
 * bonobo_property_bag_xml_decode_any:
 * @node: the parsed XML representation of an any
 * @ev: a corba exception environment
 * 
 * This routine is the converse of bonobo_property_bag_xml_encode_any.
 * It hydrates a serialized CORBA_any.
 * 
 * Return value: the CORBA_any or NULL on error
 **/
CORBA_any *
bonobo_property_bag_xml_decode_any (BonoboUINode      *node,
				    CORBA_Environment *ev)
{
	CORBA_any *any;
	CORBA_TypeCode tc;
	BonoboUINode *l, *type = NULL, *value = NULL;
	size_t   block_size;
	gpointer retval;

	g_return_val_if_fail (node != NULL, NULL);

	if (!bonobo_ui_node_has_name (node, "any")) {
		g_warning ("Not an any");
		return NULL;
	}

	for (l = bonobo_ui_node_children (node); l;
	     l = bonobo_ui_node_next (l)) {

		if (bonobo_ui_node_has_name (l, "type"))
			type = l;
		if (bonobo_ui_node_has_name (l, "value"))
			value = l;
	}
	if (!type || !value) {
		g_warning ("Missing type(%p) or value(%p) node under '%s'",
			   type, value, bonobo_ui_node_get_name (node));
		return NULL;
	}

	tc = decode_type (type, ev);

	g_return_val_if_fail (tc != NULL, NULL);

	block_size = ORBit_gather_alloc_info (tc);

	/*
	 * Uglier than your average butt cf.
	 * ORBit/src/corba_any/ORBit_demarshal_allocate_mem
	 */
	if (block_size) {
		retval = ORBit_alloc_2 (
			block_size,
			(ORBit_free_childvals) ORBit_free_via_TypeCode,
			GINT_TO_POINTER (1), sizeof(CORBA_TypeCode));
		*(CORBA_TypeCode *)((char *) retval - sizeof (ORBit_mem_info) -
				    sizeof (CORBA_TypeCode)) = 
			(CORBA_TypeCode) CORBA_Object_duplicate (
				(CORBA_Object) tc, ev);
	} else
		retval = NULL;

	any = CORBA_any__alloc ();
	any->_type = tc;
	any->_value = retval;

	decode_value (value, tc, &retval, ev);

	return any;
}
