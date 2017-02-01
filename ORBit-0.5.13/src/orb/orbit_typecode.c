#include "orbit.h"
#include "orbit_typecode.h"
#include "cdr.h"
#include "corba_typecode_type.h"
#include <IIOP/giop-msg-buffer.h>
#include "../IIOP/iiop-endianP.h"

#if 0
#define CORBA_Object_release(x, y) ({ g_message(__FILE__ ":%d Releasing object %#x from %d", __LINE__, \
x, ORBIT_ROOT_OBJECT(x)->refs); CORBA_Object_release(x, y); })
#define CORBA_Object_duplicate(x, y) ({ g_message(__FILE__ ":%d Duping object %#x from %d", __LINE__, \
x, ORBIT_ROOT_OBJECT(x)->refs); CORBA_Object_duplicate(x, y); })
#endif

typedef struct{
	CORBA_TypeCode tc;
	guint index; 
}TCRecursionNode;

typedef struct{
	GSList* prior_tcs; /* Could be a hash table by typecode */
	guint current_idx; /* The "top-level" index of the start of the current codec */
}TCEncodeContext;

typedef struct{
	GSList* prior_tcs; /* Could be a hash table by offset */
	guint current_idx;
}TCDecodeContext;



static void tc_enc(CORBA_TypeCode t, CDR_Codec* c, TCEncodeContext* ctx);
static void tc_dec(CORBA_TypeCode* t, CDR_Codec* c, TCDecodeContext* ctx);
static void tc_enc_tk_objref(CORBA_TypeCode t, CDR_Codec* c, TCEncodeContext* ctx);
static void tc_dec_tk_objref(CORBA_TypeCode t, CDR_Codec* c, TCDecodeContext* ctx);
static void tc_enc_tk_sequence(CORBA_TypeCode t, CDR_Codec* c, TCEncodeContext* ctx);
static void tc_dec_tk_sequence(CORBA_TypeCode t, CDR_Codec* c, TCDecodeContext* ctx);
static void tc_enc_tk_string(CORBA_TypeCode t, CDR_Codec* c, TCEncodeContext* ctx);
static void tc_dec_tk_string(CORBA_TypeCode t, CDR_Codec* c, TCDecodeContext* ctx);
static void tc_enc_tk_struct(CORBA_TypeCode t, CDR_Codec* c, TCEncodeContext* ctx);
static void tc_dec_tk_struct(CORBA_TypeCode t, CDR_Codec* c, TCDecodeContext* ctx);
static void tc_enc_tk_union(CORBA_TypeCode t, CDR_Codec* c, TCEncodeContext* ctx);
static void tc_dec_tk_union(CORBA_TypeCode t, CDR_Codec* c, TCDecodeContext* ctx);
static void tc_enc_tk_enum(CORBA_TypeCode t, CDR_Codec* c, TCEncodeContext* ctx);
static void tc_dec_tk_enum(CORBA_TypeCode t, CDR_Codec* c, TCDecodeContext* ctx);
static void tc_enc_tk_alias(CORBA_TypeCode t, CDR_Codec* c, TCEncodeContext* ctx);
static void tc_dec_tk_alias(CORBA_TypeCode t, CDR_Codec* c, TCDecodeContext* ctx);
static void tc_enc_tk_except(CORBA_TypeCode t, CDR_Codec* c, TCEncodeContext* ctx);
static void tc_dec_tk_except(CORBA_TypeCode t, CDR_Codec* c, TCDecodeContext* ctx);
static void tc_enc_tk_array(CORBA_TypeCode t, CDR_Codec* c, TCEncodeContext* ctx);
static void tc_dec_tk_array(CORBA_TypeCode t, CDR_Codec* c, TCDecodeContext* ctx);
static void tc_enc_tk_fixed(CORBA_TypeCode t, CDR_Codec* c, TCEncodeContext* ctx);
static void tc_dec_tk_fixed(CORBA_TypeCode t, CDR_Codec* c, TCDecodeContext* ctx);
static void tc_enc_tk_wstring(CORBA_TypeCode t, CDR_Codec* c, TCEncodeContext* ctx);
static void tc_dec_tk_wstring(CORBA_TypeCode t, CDR_Codec* c, TCDecodeContext* ctx);



typedef void
(*CORBA_TypeCodeEncoder)(CORBA_TypeCode t,
		   CDR_Codec* c,
		   TCEncodeContext* ctx);

typedef void
(*CORBA_TypeCodeDecoder)(CORBA_TypeCode t,
		   CDR_Codec* c,
		   TCDecodeContext* ctx);


typedef enum{
	TK_EMPTY,
	TK_SIMPLE,
	TK_COMPLEX
} TkType;

typedef struct{
	TkType type;
	CORBA_TypeCodeEncoder encoder;
	CORBA_TypeCodeDecoder decoder;
} TkInfo;

static const TkInfo tk_info[CORBA_tk_last]={
	{TK_EMPTY, NULL, NULL}, /* tk_null */
	{TK_EMPTY, NULL, NULL}, /* tk_void */
	{TK_EMPTY, NULL, NULL}, /* tk_short */
	{TK_EMPTY, NULL, NULL}, /* tk_long */
	{TK_EMPTY, NULL, NULL}, /* tk_ushort */
	{TK_EMPTY, NULL, NULL}, /* tk_ulong */
	{TK_EMPTY, NULL, NULL}, /* tk_float */
	{TK_EMPTY, NULL, NULL}, /* tk_double */
	{TK_EMPTY, NULL, NULL}, /* tk_boolean */
	{TK_EMPTY, NULL, NULL}, /* tk_char */
	{TK_EMPTY, NULL, NULL}, /* tk_octet */
	{TK_EMPTY, NULL, NULL}, /* tk_any */
	{TK_EMPTY, NULL, NULL}, /* tk_TypeCode */
        {TK_EMPTY, NULL, NULL}, /* tk_Principal */
	{TK_COMPLEX, tc_enc_tk_objref, tc_dec_tk_objref}, /* tk_objref */
	{TK_COMPLEX, tc_enc_tk_struct, tc_dec_tk_struct}, /* tk_struct */
        {TK_COMPLEX, tc_enc_tk_union, tc_dec_tk_union}, /* tk_union */
        {TK_COMPLEX, tc_enc_tk_enum, tc_dec_tk_enum}, /* tk_enum */
        {TK_SIMPLE, tc_enc_tk_string, tc_dec_tk_string}, /* tk_string */
        {TK_COMPLEX, tc_enc_tk_sequence, tc_dec_tk_sequence}, /* tk_sequence */
        {TK_COMPLEX, tc_enc_tk_array, tc_dec_tk_array}, /* tk_array */
        {TK_COMPLEX, tc_enc_tk_alias, tc_dec_tk_alias}, /* tk_alias */
        {TK_COMPLEX, tc_enc_tk_except, tc_dec_tk_except}, /* tk_except */
        {TK_EMPTY, NULL, NULL}, /* tk_longlong */
        {TK_EMPTY, NULL, NULL}, /* tk_ulonglong */
        {TK_EMPTY, NULL, NULL}, /* tk_longdouble */
        {TK_EMPTY, NULL, NULL}, /* tk_wchar */
	{TK_SIMPLE, tc_enc_tk_wstring, tc_dec_tk_wstring}, /* tk_wstring */
	{TK_SIMPLE, tc_enc_tk_fixed, tc_dec_tk_fixed} /* tk_fixed */
};

void ORBit_encode_CORBA_TypeCode(CORBA_TypeCode t, GIOPSendBuffer* buf)
{
        CDR_Codec codec_d;
	CDR_Codec* codec = &codec_d;
	TCEncodeContext ctx;
	GSList* l;
	CORBA_octet codecbuf[2048];

	CDR_codec_init_static(codec);

	codec->wptr = 0;
	codec->buffer = codecbuf;
	codec->release_buffer = FALSE;
	codec->buf_len = 2048;
	codec->data_endian=FLAG_ENDIANNESS;

	ctx.current_idx=0;
	ctx.prior_tcs=NULL;
	tc_enc(t, codec, &ctx);
	for(l=ctx.prior_tcs;l;l=l->next)
		g_free(l->data);
	g_slist_free(ctx.prior_tcs);
	giop_message_buffer_do_alignment(GIOP_MESSAGE_BUFFER(buf), 4);
	giop_send_buffer_append_mem_indirect(buf,
					     codec->buffer,
					     codec->wptr);
}

void ORBit_decode_CORBA_TypeCode(CORBA_TypeCode* t, GIOPRecvBuffer* buf)
{
        CDR_Codec codec_d;
	CDR_Codec* codec = &codec_d;
	TCDecodeContext ctx;
	GSList* l;

	CDR_codec_init_static(codec);
	buf->cur = ALIGN_ADDRESS(buf->cur, 4);
	codec->buffer=buf->cur;
	codec->release_buffer=CORBA_FALSE;
	codec->readonly=CORBA_TRUE;
	codec->buf_len =		/* hope this is correct */
	  ((guchar *)buf->message_body) +
	  GIOP_MESSAGE_BUFFER(buf)->message_header.message_size
	  - ((guchar *)buf->cur);
		
	codec->data_endian=GIOP_MESSAGE_BUFFER(buf)->message_header.flags & 1;

	ctx.current_idx=0;
	ctx.prior_tcs=NULL;
	tc_dec(t, codec, &ctx);
	for(l=ctx.prior_tcs;l;l=l->next)
		g_free(l->data);
	g_slist_free(ctx.prior_tcs);
	buf->cur = ((guchar *)buf->cur) + codec->rptr;
}
	

/* Encode a typecode to a codec, possibly recursively */

static void tc_enc(CORBA_TypeCode tc,
	       CDR_Codec* codec,
	       TCEncodeContext* ctx)
{
	TCRecursionNode* node;
	const TkInfo* info;
	GSList* l;
	CORBA_octet codecbuf[2048];
	CDR_Codec encaps_d;
	CDR_Codec* encaps = &encaps_d;

	for(l=ctx->prior_tcs;l;l=l->next){
		TCRecursionNode* node=l->data;
		/* CORBA_CORBA_TypeCode_equal might save space, but is slow.. */
		if(node->tc==tc){
			CDR_put_ulong(codec, CORBA_tk_recursive);
			CDR_put_long(codec,
				     node->index
				     -ctx->current_idx
				     -codec->wptr);
			return;
		}
	}

	/* All right, this isn't a previously met type. So record it. */
	/* NOTE: put kind before recording index so alignment is dealt with! */
	CDR_put_ulong(codec, tc->kind);

	node=g_new(TCRecursionNode, 1);
	node->tc=tc;
	node->index=ctx->current_idx+codec->wptr - 4; /* -4 for kind */
	ctx->prior_tcs=g_slist_prepend(ctx->prior_tcs, node);
	
	info=&tk_info[tc->kind];
	switch(info->type){
		guint tmp_index;
	case TK_EMPTY:
		break;
	case TK_COMPLEX:
		tmp_index=ctx->current_idx;
		ctx->current_idx+=codec->wptr+4; /* +4 for the length */
		CDR_codec_init_static(encaps);
		encaps->wptr = 0;
		encaps->buffer = codecbuf;
		encaps->release_buffer = FALSE;
		encaps->buf_len = 2048;
		encaps->data_endian=FLAG_ENDIANNESS;
		CDR_put_octet(encaps, FLAG_ENDIANNESS);
		(info->encoder)(tc, encaps, ctx);
		CDR_put_ulong(codec, encaps->wptr);
		/* Now this is a time hog */ 
		CDR_put_octets(codec, encaps->buffer, encaps->wptr);
		ctx->current_idx=tmp_index;
		break;
	case TK_SIMPLE:
		(info->encoder)(tc, codec, ctx);
	}
}

static void
ORBit_TypeCode_release (gpointer obj, CORBA_Environment *ev)
{
  /* we will initialize the TC_ constants with a negative refcount */
  if(ORBIT_ROOT_OBJECT(obj)->refs >= 0) {
    ORBIT_ROOT_OBJECT_UNREF(obj);

    if(ORBIT_ROOT_OBJECT(obj)->refs <= 0) {
      CORBA_TypeCode tc = obj;
      int i;

      g_free((gpointer)tc->name);
      g_free((gpointer)tc->repo_id);

      for(i = 0; i < tc->sub_parts; i++) {
	if(tc->subnames)
	  g_free((gpointer)tc->subnames[i]);

	if(tc->subtypes)
	  CORBA_Object_release((CORBA_Object)tc->subtypes[i], ev);

	if(tc->sublabels)
	  CORBA_any__free(&tc->sublabels[i], NULL, TRUE);
      }

      g_free(tc->subnames);
      g_free(tc->subtypes);
      g_free(tc->sublabels);

      if(tc->discriminator)
	CORBA_Object_release((CORBA_Object)tc->discriminator, ev);

      memset (obj, 0xa, sizeof (struct CORBA_TypeCode_struct));

      g_free(obj);
    }

  }
}

const ORBit_RootObject_Interface ORBit_TypeCode_epv = {
  &ORBit_TypeCode_release
};

static void tc_dec(CORBA_TypeCode* t, CDR_Codec* c, TCDecodeContext* ctx)
{
	CORBA_TCKind kind;
	CORBA_TypeCode tc;
	const TkInfo* info;
	TCRecursionNode* node;
	CDR_Codec encaps_d;
	CDR_Codec* encaps = &encaps_d;

	CDR_get_ulong(c, &kind);

	if(kind==CORBA_tk_recursive){
		CORBA_long offset;
		GSList* l;
		CDR_get_ulong(c, &offset);
		for(l=ctx->prior_tcs;l;l=l->next){
			node=l->data;
			/* NOTE: below, -4 is b/c we already read offset */
			if(node->index==ctx->current_idx+c->rptr+offset-4){
				CORBA_Object_duplicate((CORBA_Object)node->tc, NULL);
				*t=node->tc;
				return;
			}
		}
		ORBit_Trace(TraceMod_ORB, TraceLevel_Error,
			    "tc_dec: Invalid CORBA_TypeCode recursion offset "
			    "in input buffer\n");
		g_assert_not_reached();
	}
		
	ORBit_Trace(TraceMod_TC, TraceLevel_Debug, "codec->host_endian: %d, codec->data_endian: %d\n", c->host_endian, c->data_endian);
	ORBit_Trace(TraceMod_TC, TraceLevel_Debug, "kind: %d, CORBA_tk_last: %d\n", kind, CORBA_tk_last);
	g_assert(kind<CORBA_tk_last);

	node=g_new(TCRecursionNode, 1);
	node->index=ctx->current_idx+c->rptr-4; /* -4 for the TCKind */
	info=&tk_info[kind];
	
	tc=g_new0(struct CORBA_TypeCode_struct, 1);

	/* Passing in NULL for CORBA_Environment is patently dangerous. */
	ORBit_pseudo_object_init((ORBit_PseudoObject)tc,
				 ORBIT_PSEUDO_TYPECODE, NULL);
	ORBit_RootObject_set_interface((ORBit_RootObject)tc,
				       (ORBit_RootObject_Interface *)&ORBit_TypeCode_epv,
				       NULL);
	CORBA_Object_duplicate((CORBA_Object)tc, NULL);
	tc->kind=kind;
	switch(info->type){
		guint tmp_index;
		CORBA_octet o;

	case TK_EMPTY:
		break;

	case TK_COMPLEX:
		tmp_index=ctx->current_idx;
		CDR_codec_init_static(encaps);
		CDR_get_ulong(c, &encaps->buf_len);
		ctx->current_idx+=c->rptr;
		encaps->buffer=&c->buffer[c->rptr];
		encaps->release_buffer=CORBA_FALSE;
		CDR_get_octet(encaps, &o);
		encaps->data_endian=o;
		(info->decoder)(tc, encaps, ctx);
		c->rptr += encaps->buf_len;
		ctx->current_idx=tmp_index;
		break;
	case TK_SIMPLE:
		(info->decoder)(tc, c, ctx);
		break;
	}
	node->tc=tc;
	ctx->prior_tcs=g_slist_prepend(ctx->prior_tcs, node);
	*t=tc;
}



static void tc_enc_tk_objref(CORBA_TypeCode t, CDR_Codec* c, TCEncodeContext* ctx)
{
	CDR_put_string(c, t->repo_id);
	CDR_put_string(c, t->name);
}

static void tc_dec_tk_objref(CORBA_TypeCode t, CDR_Codec* c, TCDecodeContext* ctx)
{
	CDR_get_string(c, (char **)&t->repo_id);
	CDR_get_string(c, (char **)&t->name);
}

static void tc_enc_tk_sequence(CORBA_TypeCode t, CDR_Codec* c, TCEncodeContext* ctx)
{
	tc_enc(*t->subtypes, c, ctx);
	CDR_put_ulong(c, t->length);
}

static void tc_dec_tk_sequence(CORBA_TypeCode t, CDR_Codec* c, TCDecodeContext* ctx)
{
	t->subtypes=g_new(CORBA_TypeCode, 1);
	tc_dec(&t->subtypes[0], c, ctx);
	t->sub_parts=1;
	CDR_get_ulong(c, &t->length);
}

static void tc_enc_tk_string(CORBA_TypeCode t, CDR_Codec* c, TCEncodeContext* ctx)
{
	CDR_put_ulong(c, t->length);
}

static void tc_dec_tk_string(CORBA_TypeCode t, CDR_Codec* c, TCDecodeContext* ctx)
{
	CDR_get_ulong(c, &t->length);
}

static void tc_enc_tk_struct(CORBA_TypeCode t, CDR_Codec* c, TCEncodeContext* ctx)
{
	CORBA_unsigned_long i;
	CDR_put_string(c, t->repo_id);
	CDR_put_string(c, t->name);
	CDR_put_ulong(c, t->sub_parts);
	for(i=0;i<t->sub_parts;i++){
		CDR_put_string(c, t->subnames[i]);
		tc_enc(t->subtypes[i], c, ctx);
	}
}
 
static void tc_dec_tk_struct(CORBA_TypeCode t, CDR_Codec* c, TCDecodeContext* ctx)
{
	CORBA_unsigned_long i;
	CDR_get_string(c, (char **)&t->repo_id);
	CDR_get_string(c, (char **)&t->name);
	CDR_get_ulong(c, &t->sub_parts);
	t->subnames=g_new(const char*, t->sub_parts);
	t->subtypes=g_new(CORBA_TypeCode, t->sub_parts);
	for(i=0;i<t->sub_parts;i++){
		CDR_get_string(c, (char **)&t->subnames[i]);
		tc_dec(&t->subtypes[i], c, ctx);
	}
}	

#define UNION_MEMBERS(dir)					\
	MEMBER_LOOPER_##dir(ulong, long, long);			\
    case CORBA_tk_enum: /* fall through */			\
	MEMBER_LOOPER_##dir(ulong, unsigned_long, ulong);	\
	MEMBER_LOOPER_##dir(octet, boolean, boolean);		\
	MEMBER_LOOPER_##dir(octet, char, char);			\
	MEMBER_LOOPER_##dir(ushort, short, short);		\
	MEMBER_LOOPER_##dir(ushort, unsigned_short, ushort);	\
	MEMBER_LOOPER_##dir(ulong_long, long_long, longlong);	\
	MEMBER_LOOPER_##dir(ulong_long, unsigned_long_long, ulonglong);	\
	/* MEMBER_LOOPER_##dir(wchar, wchar, wchar); */


static void tc_enc_tk_union(CORBA_TypeCode t, CDR_Codec* c, TCEncodeContext* ctx)
{
	CORBA_unsigned_long i;
	CDR_put_string(c, t->repo_id);
	CDR_put_string(c, t->name);
	tc_enc(t->discriminator, c, ctx);
	CDR_put_long(c, t->default_index);
	CDR_put_ulong(c, t->sub_parts);
	i=t->sub_parts;
	/* Thank goodness the discriminator types are rather limited,
	   we can do the marshalling inline.. */
#define MEMBER_LOOPER_ENC(putname, typename, tkname) \
	case CORBA_tk_##tkname: \
		for(i=0;i<t->sub_parts;i++){ \
			CDR_put_##putname(c, *(CORBA_##typename*) \
				      (t->sublabels[i]._value)); \
			CDR_put_string(c, t->subnames[i]); \
			tc_enc(t->subtypes[i], c, ctx); \
		} \
		break

	switch(t->discriminator->kind){
	UNION_MEMBERS(ENC);
	default:
		ORBit_Trace(TraceMod_ORB, TraceLevel_Error,
			    "tc_enc_tk_union: Illegal union discriminator "
			    "type %s\n", t->discriminator->name);
	}
}

static void tc_dec_tk_union(CORBA_TypeCode t, CDR_Codec* c, TCDecodeContext* ctx)
{
	CORBA_unsigned_long i;
	CDR_get_string(c, (char **)&t->repo_id);
	CDR_get_string(c, (char **)&t->name);
	tc_dec(&t->discriminator, c, ctx);
	CDR_get_ulong(c, &t->default_index);
	CDR_get_ulong(c, &t->sub_parts);

	t->sublabels=g_new(CORBA_any, t->sub_parts);
	t->subnames=g_new(const char*, t->sub_parts);
	t->subtypes=g_new(CORBA_TypeCode, t->sub_parts);

#define MEMBER_LOOPER_DEC(getname, typename, tkname) \
    case CORBA_tk_##tkname: \
	for(i=0;i<t->sub_parts;i++){ 	\
	    t->sublabels[i]._type = (CORBA_TypeCode) \
	      CORBA_Object_duplicate((CORBA_Object)t->discriminator, NULL); \
	    t->sublabels[i]._value = ORBit_alloc(sizeof(CORBA_##typename), NULL, NULL); \
	    t->sublabels[i]._release = CORBA_TRUE; \
	    CDR_get_##getname(c, t->sublabels[i]._value); \
	    CDR_get_string(c, (char **)&t->subnames[i]); \
	    tc_dec(&t->subtypes[i], c, ctx); \
	} \
	break

	switch(t->discriminator->kind){
	UNION_MEMBERS(DEC);
	default:
	    /* XXX: what is correct error handling */
	    g_assert(!"Not yet implemented.");
	}
}

static void tc_enc_tk_enum(CORBA_TypeCode t, CDR_Codec* c, TCEncodeContext* ctx)
{
	CORBA_unsigned_long i;
	CDR_put_string(c, t->repo_id);
	CDR_put_string(c, t->name);
	CDR_put_ulong(c, t->sub_parts);
	for(i=0;i<t->sub_parts;i++)
		CDR_put_string(c, t->subnames[i]);
}

static void tc_dec_tk_enum(CORBA_TypeCode t, CDR_Codec* c, TCDecodeContext* ctx)
{
	CORBA_unsigned_long i;
	CDR_get_string(c, (char **)&t->repo_id);
	CDR_get_string(c, (char **)&t->name);
	CDR_get_ulong(c, &t->sub_parts);
	t->subnames=g_new(const char*, t->sub_parts);
	for(i=0;i<t->sub_parts;i++)
		CDR_get_string(c, (char **)&t->subnames[i]);
}

static void tc_enc_tk_alias(CORBA_TypeCode t, CDR_Codec* c, TCEncodeContext* ctx)
{
	CDR_put_string(c, t->repo_id);
	CDR_put_string(c, t->name);
	tc_enc(*t->subtypes, c, ctx);
}

static void tc_dec_tk_alias(CORBA_TypeCode t, CDR_Codec* c, TCDecodeContext* ctx)
{
	CDR_get_string(c, (char **)&t->repo_id);
	CDR_get_string(c, (char **)&t->name);
	t->subtypes=g_new(CORBA_TypeCode, 1);
	t->sub_parts=1;
	tc_dec(t->subtypes, c, ctx);
}


static void tc_enc_tk_except(CORBA_TypeCode t, CDR_Codec* c, TCEncodeContext* ctx)
{
	gulong i;
	CDR_put_string(c, t->repo_id);
	CDR_put_string(c, t->name);
	CDR_put_ulong(c, t->sub_parts);
	for(i=0;i<t->sub_parts;i++){
		CDR_put_string(c, t->subnames[i]);
		tc_enc(t->subtypes[i], c, ctx);
	}
}

static void tc_dec_tk_except(CORBA_TypeCode t, CDR_Codec* c, TCDecodeContext* ctx)
{
	gulong i;
	CDR_get_string(c, (char **)&t->repo_id);
	CDR_get_string(c, (char **)&t->name);
	CDR_get_ulong(c, &t->sub_parts);
	t->subtypes=g_new(CORBA_TypeCode, t->sub_parts);
	t->subnames=g_new(const char*, t->sub_parts);
	for(i=0;i<t->sub_parts;i++){
		CDR_get_string(c, (char **)&t->subnames[i]);
		tc_dec(&t->subtypes[i], c, ctx);
	}
}

static void tc_enc_tk_array(CORBA_TypeCode t, CDR_Codec* c, TCEncodeContext* ctx)
{
	tc_enc(*t->subtypes, c, ctx);
	CDR_put_ulong(c, t->length);
}

static void tc_dec_tk_array(CORBA_TypeCode t, CDR_Codec* c, TCDecodeContext* ctx)
{
	t->subtypes=g_new(CORBA_TypeCode, 1);
	tc_dec(t->subtypes, c, ctx);
	CDR_get_ulong(c, &t->length);
	t->sub_parts=1;
}

static void tc_enc_tk_wstring(CORBA_TypeCode t, CDR_Codec* c, TCEncodeContext* ctx)
{
	g_assert(!"Not yet implemented.");
}

static void tc_dec_tk_wstring(CORBA_TypeCode t, CDR_Codec* c, TCDecodeContext* ctx)
{
	g_assert(!"Not yet implemented.");
}

static void tc_enc_tk_fixed(CORBA_TypeCode t, CDR_Codec* c, TCEncodeContext* ctx)
{
	g_assert(!"Not yet implemented.");
}

static void tc_dec_tk_fixed(CORBA_TypeCode t, CDR_Codec* c, TCDecodeContext* ctx)
{
	g_assert(!"Not yet implemented.");
}
