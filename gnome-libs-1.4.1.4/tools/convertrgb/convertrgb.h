/*###################################################################*/
/*##                Image to Imlib raw rgb data Converter          ##*/
/*##                                                               ##*/
/*## This software falls under the GNU Public License. Please read ##*/
/*##              the COPYING file for more information            ##*/
/*###################################################################*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <glib.h>

#ifdef HAVE_LIBPNG
#include <png.h>
#endif

#ifdef HAVE_LIBPNG
unsigned char      *g_LoadPNG(FILE * f, int *w, int *h, int *t);
#endif

int gispng(char *file);
