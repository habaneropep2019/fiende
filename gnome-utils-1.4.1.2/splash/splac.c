#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "splash.h"

int main(int argc, char *argv[])
{
	struct splash_message sm;
	
	if(argc!=3)
	{
		fprintf(stderr,"%s: command data.\n", argv[0]);
		exit(1);
	}
	if(strcmp(argv[1], "title")==0)
	{
		strcpy(sm.data, argv[2]);
		sm.type=SM_TYPE_TITLE;
	}
	else if(strcmp(argv[1], "image")==0)
	{
		strcpy(sm.data, argv[2]);
		sm.type=SM_TYPE_IMAGE;
	}
	else
		fprintf(stderr, "%s: unknown command '%s'.\n", argv[0], argv[1]);
		
	if(write(3, &sm, sizeof(sm))!=sizeof(sm))
		fprintf(stderr, "%s: can't communicate with splash.\n",
			argv[0]);
}

