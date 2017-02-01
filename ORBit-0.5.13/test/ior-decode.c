#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include <orb/orbit.h>

static void print_object_key(int len, guchar *buf);

int main(int argc, char **argv)
{
	CORBA_char *type_id, *host;
	CORBA_unsigned_short port;
	CDR_Codec *codec, *encaps_codec = NULL;
	CORBA_octet *buffer, *keybuffer, endian, iiop_major, iiop_minor;
	int i, j;
	CORBA_unsigned_long len, seq_len, misclen;
	CORBA_char *str;

	ORBit_Trace_setLevel(TraceLevel_Debug);
	ORBit_Trace_setModules(0xFFFFFFFF);
	printf("Tags known:\n");
	printf("TAG_INTERNET_IOP: 0x%x\n", IOP_TAG_INTERNET_IOP);
	printf("TAG_MULTIPLE_COMPONENTS: 0x%x\n", IOP_TAG_MULTIPLE_COMPONENTS);
	printf("TAG_ORBIT_SPECIFIC: 0x%x\n\n", IOP_TAG_ORBIT_SPECIFIC);

	if(argc!=2) {
		fprintf(stderr, "Usage: %s ior-string\n", argv[0]);
		exit(-1);
	}
	
	str=argv[1];

	if(strncmp(str, "IOR:", 4)) {
		fprintf(stderr, "Usage: %s ior-string\n", argv[0]);
		exit(-1);
	}

	codec = CDR_codec_init();
	len = strlen(str);

	if((len % 2) || len <= 4) {
		fprintf(stderr, "Invalid IOR string (not even number of digits, or less than 4 digits)\n");
		exit(-1);
	}

	codec->buf_len = (len-4)/2;
	buffer = g_new(CORBA_octet, codec->buf_len);

	if(!buffer) {
		fprintf(stderr, "Memory error\n");
		exit(-1);
	}

	codec->buffer=buffer;
	codec->readonly = TRUE;

	for(j = 0, i = 4; i < len; i+=2) {
		buffer[j++] = HEXOCTET(str[i], str[i+1]);
	};

	CDR_get_octet(codec, &endian);

	codec->data_endian = endian;

	CDR_get_string(codec, &type_id);
	printf("Object ID: %s\n", type_id);

	CDR_get_seq_begin(codec, &seq_len);
	printf("Profile count: %d\n", seq_len);
	printf("\n");

	for(i = 0; i < seq_len; i++) {
		IOP_ProfileId tag;

		if(!CDR_get_ulong(codec, &tag))
		  goto error_out;

		switch(tag) {
		case IOP_TAG_INTERNET_IOP:
			printf("Profile type: TAG_INTERNET_IOP\n");

			if(!CDR_get_ulong(codec, &misclen))
			  goto error_out;
			encaps_codec = CDR_codec_init();
			encaps_codec->buffer = g_new(CORBA_octet, misclen);
			if(!CDR_buffer_gets(codec, encaps_codec->buffer, misclen))
			  goto error_out;

			encaps_codec->buf_len = misclen;
			encaps_codec->readonly = CORBA_TRUE;
			if(!CDR_get_octet(encaps_codec, &endian))
			  goto error_out;

			encaps_codec->data_endian = endian;
			g_assert(endian <= 1);
			printf("Object endian: %s%s\n", endian?"Little":"Big", endian==encaps_codec->host_endian?"":" (converting)");

			if(!CDR_get_octet(encaps_codec, &iiop_major))
			  goto error_out;
			if(!CDR_get_octet(encaps_codec, &iiop_minor))
			  goto error_out;
			printf("IIOP version: %d.%d\n", iiop_major, iiop_minor);

			if(!CDR_get_string(encaps_codec, &host))
			  goto error_out;
			printf("Host: %s\n", host);

			if(!CDR_get_ushort(encaps_codec, &port))
			  goto error_out;
			printf("Port: %d\n", port);

			if(!CDR_get_seq_begin(encaps_codec, &misclen))
			  goto error_out;
			keybuffer = g_new(CORBA_octet, misclen);
			if(!CDR_buffer_gets(encaps_codec, keybuffer, misclen))
			  goto error_out;
			printf("Object key: \"");
			print_object_key(misclen, keybuffer);
			printf("\"\n");

			CDR_codec_free(encaps_codec);
			encaps_codec = NULL;
			printf("\n");
			break;

		case IOP_TAG_MULTIPLE_COMPONENTS:
			/* Just skip any multiple_components data, for now */
			printf("Skipping TAG_MULTIPLE_COMPONENTS\n");
			if(!CDR_get_ulong(codec, &misclen))
			  goto error_out;
			encaps_codec = CDR_codec_init();
			encaps_codec->buf_len = misclen;
			encaps_codec->buffer = g_new(CORBA_octet, misclen);
			encaps_codec->readonly = CORBA_TRUE;
			if(!CDR_buffer_gets(codec, encaps_codec->buffer, misclen))
			  goto error_out;
			CDR_codec_free(encaps_codec); encaps_codec = NULL;
			printf("\n");
			break;

		case IOP_TAG_ORBIT_SPECIFIC:
			printf("Profile type: TAG_ORBIT_SPECIFIC\n");

			if(!CDR_get_ulong(codec, &misclen))
			  goto error_out;
			encaps_codec = CDR_codec_init();

			encaps_codec->buffer = g_new(CORBA_octet, misclen);
			if(!CDR_buffer_gets(codec, encaps_codec->buffer, misclen))
			  goto error_out;

			encaps_codec->buf_len = misclen;
			encaps_codec->readonly = CORBA_TRUE;

			if(!CDR_get_octet(encaps_codec, &endian))
			  goto error_out;
			encaps_codec->data_endian = endian;
			printf("Object endian: %s%s\n", endian?"Little":"Big", endian==encaps_codec->host_endian?"":" (converting)");

			if(!CDR_get_octet(encaps_codec, &iiop_major))
			  goto error_out;
			if(!CDR_get_octet(encaps_codec, &iiop_minor))
			  goto error_out;
			printf("IIOP version: %d.%d\n", iiop_major, iiop_minor);

			if(!CDR_get_string(encaps_codec, &host))
			  goto error_out;
			printf("Socket path: %s\n", host);

			if(!CDR_get_ushort(encaps_codec, &port))
			  goto error_out;
			printf("IPv6 port: %d\n", port);

			if(!CDR_get_seq_begin(encaps_codec, &misclen))
			  goto error_out;
			keybuffer = g_new(CORBA_octet, misclen);
			if(!CDR_buffer_gets(encaps_codec, keybuffer, misclen))
			  goto error_out;
			printf("Object key: \"");
			print_object_key(misclen, keybuffer);
			printf("\"\n");

			CDR_codec_free(encaps_codec); encaps_codec = NULL;
			printf("\n");
			break;
		default:
			printf("Skipping unknown tag 0x%x\n", tag);

			/* Skip it */
			if(!CDR_get_ulong(codec, &misclen))
			  goto error_out;
			encaps_codec = CDR_codec_init();
			encaps_codec->buf_len = misclen;
			encaps_codec->buffer = g_new(CORBA_octet, misclen);
			encaps_codec->readonly = CORBA_TRUE;
			if(!CDR_buffer_gets(codec, encaps_codec->buffer, misclen))
			  goto error_out;
			CDR_codec_free(encaps_codec); encaps_codec = NULL;
			printf("\n");
			break;
		}
	}
 error_out:
	exit(0);
}

static void
print_object_key(int len, guchar *buf)
{
	int i;
	for(i = 0; i < len; i++) {
		guint intval;
		intval = buf[i];
		if(isprint(buf[i]) || (CLAMP(intval, 32, 127) == buf[i]))
			printf("%c", buf[i]);
		else
			printf(".");
	}
}

