/* PNG loader copied from Imlib 1.8 (load.c)
 * with a few modifications 
 */

#include "convertrgb.h"

static int transp;

#ifdef HAVE_LIBPNG
unsigned char      *
g_LoadPNG(FILE * f, int *w, int *h, int *t)
{
   png_structp         png_ptr;
   png_infop           info_ptr;
   unsigned char      *data, *ptr, **lines, *ptr2, r, g, b, a;
   int                 i, x, y, bit_depth, color_type, interlace_type;
   png_uint_32         ww, hh;

   /* Init PNG Reader */
   transp = 0;
   png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
   if (!png_ptr)
      return NULL;

   info_ptr = png_create_info_struct(png_ptr);
   if (!info_ptr)
     {
	png_destroy_read_struct(&png_ptr, NULL, NULL);
	return NULL;
     }

   if (setjmp(png_ptr->jmpbuf))
     {
	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
	return NULL;
     }

   if (info_ptr->color_type == PNG_COLOR_TYPE_RGB_ALPHA)
     {
	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
	return NULL;
     }
   png_init_io(png_ptr, f);
   /* Read Header */
   png_read_info(png_ptr, info_ptr);
   png_get_IHDR(png_ptr, info_ptr, &ww, &hh, &bit_depth, &color_type, &interlace_type,
		NULL, NULL);
  *w = ww;
  *h = hh;
   /* Setup Translators */
   if (color_type == PNG_COLOR_TYPE_PALETTE)
     png_set_expand(png_ptr);
   png_set_strip_16(png_ptr);
   png_set_packing(png_ptr);
   if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
      png_set_expand(png_ptr);
   png_set_filler(png_ptr, 0xff, PNG_FILLER_AFTER);
   data = malloc(*w ** h * 3);
   if (!data)
     {
	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
	return NULL;
     }
   lines = (unsigned char **)malloc(*h * sizeof(unsigned char *));

   if (lines == NULL)
     {
	free(data);
	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
	return NULL;
     }
   for (i = 0; i < *h; i++)
     {
	if ((lines[i] = malloc(*w * (sizeof(unsigned char) * 4))) == NULL)
	  {
	     int                 n;

	     free(data);
	     for (n = 0; n < i; n++)
		free(lines[n]);
	     free(lines);
	     png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
	     return NULL;
	  }
     }
   png_read_image(png_ptr, lines);
   png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
   ptr = data;
   if ((color_type == PNG_COLOR_TYPE_GRAY) ||
       (color_type == PNG_COLOR_TYPE_GRAY_ALPHA))
     {
	for (y = 0; y < *h; y++)
	  {
	     ptr2 = lines[y];
	     for (x = 0; x < *w; x++)
	       {
		  r = *ptr2++;
		  a = *ptr2++;
		  if (a < 128)
		    {
		       *ptr++ = 255;
		       *ptr++ = 0;
		       *ptr++ = 255;
		       transp = 1;
		    }
		  else
		    {
		       *ptr++ = r;
		       *ptr++ = r;
		       *ptr++ = r;
		    }
	       }
	  }
     }
   else
     {
	for (y = 0; y < *h; y++)
	  {
	     ptr2 = lines[y];
	     for (x = 0; x < *w; x++)
	       {
		  r = *ptr2++;
		  g = *ptr2++;
		  b = *ptr2++;
		  a = *ptr2++;
		  if (a < 128)
		    {
		       *ptr++ = 255;
		       *ptr++ = 0;
		       *ptr++ = 255;
		       transp = 1;
		    }
		  else
		    {
		       if ((r == 255) && (g == 0) && (b == 255))
			  r = 254;
		       *ptr++ = r;
		       *ptr++ = g;
		       *ptr++ = b;
		    }
	       }
	  }
     }
   for (i = 0; i < *h; i++)
      free(lines[i]);
   free(lines);
   *t = transp;
   return data;
}
#endif /* HAVE_LIBPNG */

int
gispng(char *file)
{
#ifdef HAVE_LIBPNG
   FILE               *f;
   unsigned char       buf[8];

   f = fopen(file, "rb");
   if (!f)
      return 0;
   fread(buf, 1, 8, f);
   fclose(f);
   return (int)png_check_sig(buf, 8);
#else
   return 0;
#endif
}
