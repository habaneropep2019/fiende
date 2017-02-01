/*###################################################################*/
/*##              Image to Imlib raw rgb data Converter            ##*/
/*##                                                               ##*/
/*## This software falls under the GNU Public License. Please read ##*/
/*##              the COPYING file for more information            ##*/
/*###################################################################*/

#include "convertrgb.h"

#define CRGB_VERSION "0.2.1"

static int efficient;
static int dimensions;
static int append;
static char *file_for_output;
static char *suffix;
static char *variable_name;

static int is_file(char *s)
{
   struct stat st;

   if ((!s)||(!*s)) return 0;
   if (stat(s,&st)<0) return 0;
   if (S_ISREG(st.st_mode)) return 1;
   return 0;
}

static void convert(char *real_file)
{
	FILE *f, *sf;
	char outfile[4096];
	char data_var[4096];
	char cmd[4096];
	unsigned char buf[16];
	char *ptr, *file;
	int col = 0;
	int c = 1;
	int w, h, t;

        ptr = strrchr (real_file, '/');
        file = ptr ? ptr+1 : real_file;

	if (file_for_output)
		{
		strcpy(outfile, file_for_output);
		}
	else
		{
		strcpy (outfile, file);
		ptr = outfile + strlen(outfile);
		while (ptr > &outfile[0] && ptr[0] != '.') ptr--;
		ptr[0] = '\0';
		if (suffix)
			strcpy (ptr, suffix);
		else
			strcpy (ptr, ".c");
		}

	if (variable_name)
		strcpy(data_var, variable_name);
	else
		{
		strcpy(data_var, file);
		ptr = data_var + strlen(data_var);
		while (ptr > &data_var[0] && ptr[0] != '.') ptr--;
		strcpy (ptr, "_rgb");
		}

	if (strcmp(file, outfile) == 0)
		{
		printf("Input and output files cannot be the same!\n");
		return;
		}


	if (gispng(real_file))
		{
#ifdef HAVE_LIBPNG
		unsigned char *d;
		int x, y;

		sf = fopen(real_file, "rb");
		if (!sf)
			{
			return;
			}

		d = g_LoadPNG(sf, &w, &h, &t);

		if (!d) return;

		if (append)
			f = fopen(outfile,"a");
		else
			f = fopen(outfile,"w");
	
		if (!f)
			{
			printf("unable to open output file: %s\n", outfile);
			free(d);
			return;
			}

		fprintf(f, "/* Imlib raw rgb data file created by convertrgb */\n\n");

		if (dimensions)
			{
			fprintf(f, "static const int %s_width  = %d;\n", data_var, w);
			fprintf(f, "static const int %s_height = %d;\n", data_var, h);
			if (t)
				fprintf(f, "static const GdkImlibColor %s_alpha  = { 255, 0, 255, 0 };\n", data_var);
			else
				fprintf(f, "static const GdkImlibColor %s_alpha  = { -1, -1, -1, 0 };\n", data_var);
			}

		fprintf(f, "static const unsigned char %s[] = {\n", data_var);

		for (y=0;y < h; y++)
			for (x=0;x < w; x++)
				{
				unsigned int r, g, b;
				int l;
				l = (( y * w) + x ) * 3;
				r = d[l];
				l++;
				g = d[l];
				l++;
				b = d[l];
				if (!efficient)
					fprintf(f, "0x%.2x, 0x%.2x, 0x%.2x", r, g, b);
				else
					fprintf(f, "%d,%d,%d", r, g, b);
				col++;
				if (y != h -1 || x != w - 1)
					fprintf(f, ",");
				if (col > 3)
					{
					col = 0;
					fprintf(f , "\n");
					}
				else
					{
					if (!efficient) fprintf(f, " ");
					}
				}

		if (col == 0)
			fprintf (f, "};\n");
		else
			fprintf (f, "\n};\n");

		fclose(f);
		free(d);
#endif
		}
	else
		{
		sprintf(cmd, "convert %s rgba:-", real_file);

		sf = popen(cmd, "r");
		if (!sf) return;

		if (append)
			f = fopen(outfile,"a");
		else
			f = fopen(outfile,"w");
	
		if (!f)
			{
			pclose(sf);
			printf("unable to open output file: %s\n", outfile);
			return;
			}

		fprintf(f, "/* Imlib raw rgb data created by convertrgb %s */\n\n", CRGB_VERSION);

		/* FIXME dimensions are broken here:
		if (dimensions)
			{
			fprintf(f, "static const int %s_width  = %d;\n", data_var, w);
			fprintf(f, "static const int %s_height = %d;\n", data_var, h);
			}
		*/

		fprintf(f, "static const unsigned char %s[] = {\n", data_var);

		while (fread(buf, sizeof(unsigned char), 8, sf) == 8)
			{
			unsigned int r, g, b, a;

			/* most images don't go beyond 24 bits, but just in case: */
			if (buf[0] == buf[1])
				r = buf[0];
			else
				r = (buf[0] * buf[1]) / 255;

			if (buf[2] == buf[3])
				g = buf[2];
			else
				g = (buf[2] * buf[3]) / 255;
	
			if (buf[4] == buf[5])
				b = buf[4];
			else
				b = (buf[4] * buf[5]) / 255;

			if (buf[6] == buf[7])
				a = buf[6];
			else
				a = (buf[6] * buf[7]) / 255;
			/* imlib data uses 255,0,255 for tranparency */
			if (a < 128)
				{
				r = 255;
				g = 0;
				b = 255;
				}

			if (c != 1)
				{
				fprintf(f, ",");
				if (col > 3)
					{
					col = 0;
					fprintf(f , "\n");
					}
				else
					{
					if (!efficient) fprintf(f, " ");
					}
				}

			if (!efficient)
				fprintf(f, "0x%.2x, 0x%.2x, 0x%.2x", r, g, b);
			else
				fprintf(f, "%d,%d,%d", r, g, b);
			col++;
			c++;
			}

		pclose(sf);
	
		fprintf (f, "\n};\n");

		fclose(f);
		}
}

int main (int argc, char *argv[])
{
	efficient = 0;
	dimensions = 1;
	append = 0;
	file_for_output = NULL;
	suffix = NULL;
	variable_name = NULL;

	if (argc > 1)
		{
		int i = 1;
		while (i < argc)
			{
			char *file = argv[i];
			if (strcmp(file, "--efficient") == 0 || strcmp(file, "-e") == 0)
				efficient = 1;
			else if (strcmp(file, "--nodim") == 0 || strcmp(file, "-n") == 0)
				dimensions = 0;
			else if (strncmp(file, "-o", 2) == 0)
				{
				if (append || file_for_output || suffix)
					{
					printf("Cannot Specify multiple output methods of -a, -o, or -s\n");
					return 1;
					}
				if (strlen(file) > 3)
					{
					file_for_output = g_strdup (file + 3);
					}
				else
					{
					printf("No output file specified for -o option\n");
					return 1;
					}
				}
			else if (strncmp(file, "-a", 2) == 0)
				{
				if (append || file_for_output || suffix)
					{
					printf("Cannot Specify multiple output methods of -a, -o, or -s\n");
					return 1;
					}
				if (strlen(file) > 3)
					{
					append = 1;
					file_for_output = g_strdup (file + 3);
					}
				else
					{
					printf("No output file specified for -a option\n");
					return 1;
					}
				}
			else if (strncmp(file, "-s", 2) == 0)
				{
				if (append || file_for_output || suffix)
					{
					printf("Cannot Specify multiple output methods of -a, -o, or -s\n");
					return 1;
					}
				if (strlen(file) > 3)
					{
					append = 1;
					suffix = g_strdup (file + 3);
					}
				else
					{
					printf("No suffix specified for -s option\n");
					return 1;
					}
				}
			else if (strncmp(file, "-v", 2) == 0)
				{
				if (variable_name)
					{
					printf("Cannot -v more than once before each input file\n");
					return 1;
					}
				if (strlen(file) > 3)
					{
					variable_name = g_strdup (file + 3);
					}
				else
					{
					printf("No name specified for -v option\n");
					return 1;
					}
				}
			else if (is_file(file))
				{
				convert(file);
				if (file_for_output && !append)
					{
					g_free(file_for_output);
					file_for_output = NULL;
					}
				if (variable_name)
					{
					g_free(variable_name);
					variable_name = NULL;
					}
				}
			else
				printf("Error, file not found: %s\n", file);
			i++;
			}
		}
	else
		{
		printf ("Image to Imlib raw rgb data Converter                 Version %s\n", CRGB_VERSION);
		printf ("This program is released under the terms of the GNU public license.\n");
		printf ("Command line params:\n\n");
		printf ("      convertrgb [-e] [-o=fn|-a=fn|-s=sn] [-v=vn] inputfile [inputfile] ...\n\n");
		printf ("   -e, --efficient      Use smallest format possible to save space\n");
		printf ("   -n, --nodim          Suppress output of image dimensions\n");
		printf ("   -o=[fn]              Output to file named fn , default is .c extension\n");
		printf ("                          to replace extension of inputfile\n");
		printf ("   -a=[fn]              Like -o, but appends to file named fn\n");
		printf ("   -s=[sn]              Suffix for output filename\n");
		printf ("                        (eg: 'convertrgb -s=.rgb logo.png'sets output file\n");
		printf ("                         to 'logo.rgb')\n");
		printf ("   -v=[vn]              The variable name to use for the next image.\n");
		}

	return 0;
}

