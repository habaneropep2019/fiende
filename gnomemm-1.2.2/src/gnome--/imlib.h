/*
 * Gdk-- imlib.h
 *
 * Copyright 1998 Karl E. Nelson <kenelson@ece.ucdavis.edu>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#ifndef _GDKMM_IMLIB_H_
#define _GDKMM_IMLIB_H_

#include <gdk--.h>
#include <gdk_imlib.h>

class Gdk_Imlib
{
public:
  /* forward definition for internal classes */
  class Border		:public Gdk_Obj<GdkImlibBorder> {};
  class Color		:public Gdk_Obj<GdkImlibColor> {};
  class Color_Modifier	:public Gdk_Obj<GdkImlibColorModifier> {};
  class Image;
  class Save_Info		:public Gdk_Obj<GdkImlibSaveInfo> {};
  class Init_Params		:public Gdk_Obj<GdkImlibInitParams> {};
  class Modifier_Map		:public Gdk_Obj<GdkImlibModifierMap> {};
  struct Enums;

  /* Imlib Object (We use this a bit like a namespace) */
  Gdk_Imlib()
    {
      gdk_imlib_init();
    }

  Gdk_Imlib(Init_Params &p)
    {
      gdk_imlib_init_params(p);
    }

  gint get_render_type()
    {
      return gdk_imlib_get_render_type();
    }

  void set_render_type(gint rend_type)
    {
      gdk_imlib_set_render_type(rend_type);
    }

  gint load_colors(const Gtk::string& file)
    {
      return gdk_imlib_load_colors(const_cast<gchar*>(file.c_str()));
    }   

  gint best_color_match(gint &r,gint &g,gint &b)
    {
      return gdk_imlib_best_color_match(&r,&g,&b);
    }

  void best_color_get(Gdk_Color &c)
    {
      gdk_imlib_best_color_get(c.gdkobj());
    }

  gint get_fallback()
    {
      return gdk_imlib_get_fallback();
    }

  void set_fallback(gint fallback)
    {
      gdk_imlib_set_fallback(fallback);
    }

  Gdk_Visual get_visual()
    {
      return gdk_imlib_get_visual();
    }

  Gdk_Colormap get_colormap()
    {
      return gdk_imlib_get_colormap();
    }

  gchar  *get_sysconfig()
    {
      return gdk_imlib_get_sysconfig();
    }

  void get_cache_info (int &cache_pixmaps, int &cache_images)
    {
      gdk_imlib_get_cache_info (&cache_pixmaps,&cache_images);
    }

  void set_cache_info (int cache_pixmaps, int cache_images)
    {
      gdk_imlib_set_cache_info (cache_pixmaps,cache_images);
    }
 
  /* Free resources */
  void free_colors()
    {
      gdk_imlib_free_colors();
    }

  void free_pixmap(Gdk_Pixmap &pixmap)
    {
      gdk_imlib_free_pixmap(pixmap);
    }

  void free_bitmap(Gdk_Bitmap &bitmap)
    {
      gdk_imlib_free_bitmap(bitmap);
    }

  gint                gdk_imlib_data_to_pixmap(char **data, GdkPixmap ** pmap, GdkBitmap ** mask);

};

struct Gdk_Imlib::Enums
{
  enum PARAMS 
  {
    VISUALID		= PARAMS_VISUALID,
    PALETTEFILE	= PARAMS_PALETTEFILE,
    SHAREDMEM		= PARAMS_SHAREDMEM,
    SHAREDPIXMAPS	= PARAMS_SHAREDPIXMAPS,
    PALETTEOVERRIDE	= PARAMS_PALETTEOVERRIDE,
    REMAP		= PARAMS_REMAP,
    FASTRENDER	= PARAMS_FASTRENDER,
    HIQUALITY		= PARAMS_HIQUALITY,
    DITHER		= PARAMS_DITHER,
    IMAGECACHESIZE	= PARAMS_IMAGECACHESIZE,
    PIXMAPCACHESIZE	= PARAMS_PIXMAPCACHESIZE
  };
  enum PAGE_SIZE
  {
    EXECUTIVE    = PAGE_SIZE_EXECUTIVE,
    LETTER       = PAGE_SIZE_LETTER,
    LEGAL        = PAGE_SIZE_LEGAL,
    A4           = PAGE_SIZE_A4,
    A3           = PAGE_SIZE_A3,
    A5           = PAGE_SIZE_A5,
    FOLIO        = PAGE_SIZE_FOLIO
  };
  enum RT
  {
    PLAIN_PALETTE       = RT_PLAIN_PALETTE,
    PLAIN_PALETTE_FAST  = RT_PLAIN_PALETTE_FAST,
    DITHER_PALETTE      = RT_DITHER_PALETTE,
    DITHER_PALETTE_FAST = RT_DITHER_PALETTE_FAST,
    PLAIN_TRUECOL       = RT_PLAIN_TRUECOL,
    DITHER_TRUECOL      = RT_DITHER_TRUECOL
  };
};

class Gdk_Imlib::Image:public Gdk_Handle<GdkImlibImage> 
{
public:
  void destroy()
    {
      gdk_imlib_destroy_image(obj_);
    }
  void kill()
    {
      gdk_imlib_kill_image(obj_);
    }
private:
  void ref()
    {}
  void unref()
    {destroy();}

public:
  Image()
    :Gdk_Handle<GdkImlibImage>(0) {}

  Image(const Image &im)
    :Gdk_Handle<GdkImlibImage>(0)
  {
    if(im.gdkobj() != NULL)
      clone(im);
  }

  Image(const Gtk::string &file)
    :Gdk_Handle<GdkImlibImage>(0) 
    {
      load(file);
    }

  Image(const gchar * const *data)
    :Gdk_Handle<GdkImlibImage>(0) 
    {
      create_from_xpm_data(data);
    }

  ~Image()
    {
      unref();
    }

  Image& operator = (const Image &im)
    {
      clone(im);
      return *this;
    }

  void load(const Gtk::string& file)
    {
      unref();
      obj_=gdk_imlib_load_image(const_cast<gchar*>(file.c_str()));
      ref();
      if(!obj_) create_from_xpm_data(Gdk_Pixmap::defaultPixmap);
    }

  void create_from_data(unsigned char *data,
			unsigned char *alpha, 
			gint w, gint h)
    {
      unref();
      obj_=gdk_imlib_create_image_from_data(data,alpha,w,h);
      ref();
    }

  void clone(const Image &im)
    {
      unref();
      obj_=gdk_imlib_clone_image(gdk_const_cast(im));
      ref();
    }

  void clone_scaled(const Image &im, int w, int h)
    {
      unref();
      obj_=gdk_imlib_clone_scaled_image(gdk_const_cast(im),w,h);
      ref();
    }
 
  // should be const char * const *data
  void create_from_xpm_data(const gchar* const *data)
    {
      unref();
      obj_=gdk_imlib_create_image_from_xpm_data(const_cast<gchar**>(data));
      ref();
    }

  void crop_and_clone(const Image &im, int x, int y, int w, int h)
    {
      unref();
      obj_=gdk_imlib_crop_and_clone_image(gdk_const_cast(im),x,y,w,h);
      ref();
    }

  void create_from_drawable(Gdk_Drawable &gwin,
			    Gdk_Bitmap   &gmask,
			    int x, int y, int width, int height)
    {
      unref();
      obj_=gdk_imlib_create_image_from_drawable(gwin,gmask,x,y,width,height);
      ref();
    }

  void inlined_png_to_image(unsigned char *data, int data_size)
    {
      unref();
      obj_=gdk_imlib_inlined_png_to_image(data,data_size);
    }

  /* Methods */
  gint render(gint width, gint height)
    {
      return gdk_imlib_render(*this,width,height);
    }

  Gdk_Pixmap copy_image()
    {
      return gdk_imlib_copy_image(*this);
    }

  Gdk_Bitmap copy_mask()
    {
      return gdk_imlib_copy_mask(*this);
    }

  Gdk_Pixmap move_image()
    {
      return gdk_imlib_move_image(*this);
    }

  Gdk_Bitmap move_mask()
    {
      return gdk_imlib_move_mask(*this);
    }

  void get_border(Border &border) const
    {
      gdk_imlib_get_image_border(const_cast<GdkImlibImage*>(gdkobj()),border);
    }

  void set_border(Border &border)
    {
      gdk_imlib_set_image_border(*this,border);
    }

  void get_shape(Color &color) const
    {
      gdk_imlib_get_image_shape(const_cast<GdkImlibImage*>(gdkobj()),color);
    }

  void set_shape(Color &color)
    {
      gdk_imlib_set_image_shape(*this,color);
    }

  gint save_to_eim(const Gtk::string& file) const
    {
      return gdk_imlib_save_image_to_eim(const_cast<GdkImlibImage*>(gdkobj()),
					 const_cast<char*>(file.c_str()));
    }

  gint add_to_eim(const Gtk::string& file) const
    {
      return gdk_imlib_add_image_to_eim(const_cast<GdkImlibImage*>(gdkobj()),
					const_cast<char*>(file.c_str()));
    }

  gint save(const Gtk::string &file, Save_Info &info) const
    {
      return gdk_imlib_save_image(const_cast<GdkImlibImage*>(gdkobj()),
				  const_cast<gchar*>(file.c_str()),info);
    }

  gint save_to_ppm(const Gtk::string& file) const
    {
      return gdk_imlib_save_image_to_ppm(const_cast<GdkImlibImage*>(gdkobj()),
					 const_cast<char*>(file.c_str()));
    }


  /* Modifiers */
  void set_modifier(Color_Modifier &mod)
    {
      gdk_imlib_set_image_modifier(*this,mod);
    }

  void set_red_modifier(Color_Modifier &mod)
    {
      gdk_imlib_set_image_red_modifier(*this,mod);
    }
  
  void set_green_modifier(Color_Modifier &mod)
    {
      gdk_imlib_set_image_green_modifier(*this,mod);
    }

  void set_blue_modifier(Color_Modifier &mod)
    {
      gdk_imlib_set_image_blue_modifier(*this,mod);
    }

  void get_modifier(Color_Modifier &mod) const
    {
      gdk_imlib_get_image_modifier(const_cast<GdkImlibImage*>(gdkobj()),mod);
    }

  void get_red_modifier(Color_Modifier &mod) const
    {
      gdk_imlib_get_image_red_modifier(const_cast<GdkImlibImage*>(gdkobj()),mod);
    }

  void get_green_modifier(Color_Modifier & mod) const
    {
      gdk_imlib_get_image_green_modifier(const_cast<GdkImlibImage*>(gdkobj()),mod);
    }

  void get_blue_modifier(Color_Modifier &mod) const
    {
      gdk_imlib_get_image_blue_modifier(const_cast<GdkImlibImage*>(gdkobj()),mod);
    }

  /* Curve methods - mod[] is 256 */
  void set_red_curve(unsigned char mod[])
    {
      gdk_imlib_set_image_red_curve(*this,mod);
    }

  void set_green_curve(unsigned char mod[])
    {
      gdk_imlib_set_image_green_curve(*this,mod);
    }

  void set_blue_curve(unsigned char mod[])
    {
      gdk_imlib_set_image_blue_curve(*this,mod);
    }

  void get_red_curve(unsigned char mod[]) const
    {
      gdk_imlib_get_image_red_curve(const_cast<GdkImlibImage*>(gdkobj()),mod);
    }

  void get_green_curve(unsigned char mod[]) const
    {
      gdk_imlib_get_image_green_curve(const_cast<GdkImlibImage*>(gdkobj()),mod);
    }

  void get_blue_curve(unsigned char mod[]) const
    {
      gdk_imlib_get_image_blue_curve(const_cast<GdkImlibImage*>(gdkobj()),mod);
    }
    
  void apply_modifiers_to_rgb()
    {
      gdk_imlib_apply_modifiers_to_rgb(*this);
    }

  /* Manipulations */
  void changed()
    {
      gdk_imlib_changed_image(*this);
    }

  //: Makes the image a background for a window
  // (This must be a window, not a drawable)
  void apply(Gdk_Window &p) const
    {
      gdk_imlib_apply_image(const_cast<GdkImlibImage*>(gdkobj()),p);
    }

  void paste(Gdk_Drawable &p, gint x, gint y, gint w, gint h) const
    {
      gdk_imlib_paste_image(const_cast<GdkImlibImage*>(gdkobj()),p,x,y,w,h);
    }

  void paste_border(Gdk_Drawable &p, gint x, gint y, gint w, gint h) const
    {
      gdk_imlib_paste_image_border(const_cast<GdkImlibImage*>(gdkobj()),p,x,y,w,h);
    }

  void flip_horizontal()
    {
      gdk_imlib_flip_image_horizontal(*this);
    }

  void flip_vertical()
    {
      gdk_imlib_flip_image_vertical(*this);
    }

  void rotate(gint d)
    {
      gdk_imlib_rotate_image(*this,d);
    }

  void crop(gint x, gint y, gint w, gint h)
    {
      gdk_imlib_crop_image(*this,x,y,w,h);
    }

  gint rgb_width() const
    {
      return gdkobj()->rgb_width;
    }

  gint rgb_height() const
    {
      return gdkobj()->rgb_height;
    }

};

/* these still need work */

#endif // _GDKMM_IMLIB_H_
