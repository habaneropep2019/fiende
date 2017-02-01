struct splash_message
{
	unsigned int type;
	unsigned int ndata[3];
	unsigned char data[1024];
};


#define SM_TYPE_QUIT	1
#define SM_TYPE_TITLE	2
#define SM_TYPE_IMAGE	3
