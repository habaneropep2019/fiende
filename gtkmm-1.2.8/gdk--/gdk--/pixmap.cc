#include <gdk--/pixmap.h>
#include <gdk--/window.h>
#include <gdk--/colormap.h>
#include <gdk--/bitmap.h>
#include <iostream>
 

GTKMM_USING_STD_STRING

void Gdk_Pixmap::ref()
{
  if (obj_) 
    gdk_pixmap_ref(obj_);
}

void Gdk_Pixmap::unref()
{
  if (obj_) 
    gdk_pixmap_unref(obj_);
  obj_=0;
}

Gdk_Pixmap::Gdk_Pixmap():Gdk_Drawable(0) 
{
}

Gdk_Pixmap::Gdk_Pixmap(GdkPixmap *pixmap):Gdk_Drawable(pixmap)
{
  ref();
}

Gdk_Pixmap::Gdk_Pixmap(Gdk_Pixmap const &pixmap):Gdk_Drawable(pixmap)
{
  ref();
}

Gdk_Pixmap::~Gdk_Pixmap()
{
  release();
}

Gdk_Pixmap& Gdk_Pixmap::operator = (const Gdk_Pixmap& h)
  {
    if (h.obj_==obj_) return *this;
    unref();
    obj_=h.obj_;
    ref();
    return *this;
  }


/*****************************************************************/

void Gdk_Pixmap::create(const Gdk_Drawable  &window,
			gint        width,
			gint        height,
			gint        depth)
{
  unref();
  obj_=gdk_pixmap_new(gdk_const_cast(window),width,height,depth);
  // gdk_pixmap_new sets the refcount to 1 already, no need to
  // do it again here.
  //ref();
}

Gdk_Pixmap::Gdk_Pixmap(const Gdk_Drawable &window,
		       gint        width, 
		       gint        height,
		       gint        depth):Gdk_Drawable(0)
{
  create(window,width,height,depth);
}

void Gdk_Pixmap::create(
			gint        width,
			gint        height,
			gint        depth)
{
  unref();
  obj_=gdk_pixmap_new(0,width,height,depth);
  // gdk_pixmap_new puts refcount to 1 already, no need to do it here.
  //ref();
}

Gdk_Pixmap::Gdk_Pixmap(
		       gint        width, 
		       gint        height,
		       gint        depth):Gdk_Drawable(0)
{
  create(width,height,depth);
}

/****************************************************************/

void Gdk_Pixmap::create_from_data(
				  const Gdk_Drawable  &window,
				  const gchar *data,
				  gint        width,
				  gint        height,
				  gint        depth,
				  const Gdk_Color   &fg,
				  const Gdk_Color   &bg)
{
  unref();
  obj_=gdk_pixmap_create_from_data(gdk_const_cast(window),
				   const_cast<gchar*>(data),width,height,depth,
				   gdk_const_cast(fg),
				   gdk_const_cast(bg));  
  // no need to ref() it here.
  //ref();
}

Gdk_Pixmap::Gdk_Pixmap(const Gdk_Drawable  &window,
		       const gchar      *data,
		       gint        width,
		       gint        height,
		       gint        depth,
		       const Gdk_Color  &fg,
		       const Gdk_Color  &bg):Gdk_Drawable(0)
{
  create_from_data(window,data,width,height,depth,fg,bg);
}
    
void Gdk_Pixmap::create_from_xpm(
				 const Gdk_Drawable  &window,
				 const Gdk_Color   &transparent_color,
				 const Gtk::string      &filename)
{
  unref();
  obj_=gdk_pixmap_create_from_xpm(gdk_const_cast(window),0,
				  gdk_const_cast(transparent_color),
				  const_cast<gchar*>(filename.c_str()));
  if(!obj_)
    {
      Gdk_Bitmap dummy;
      warn_about_xpm_file_problem(filename);
      create_from_xpm_d(window, dummy, transparent_color, defaultPixmap);
    }
      
  // no need to ref() it here, gdk should do it.
  //ref();
}

void Gdk_Pixmap::create_from_xpm(
				 const Gdk_Drawable  &window,
				 Gdk_Bitmap  &mask,
				 const Gdk_Color   &transparent_color,
				 const Gtk::string      &filename)
{
  unref();
  GdkBitmap *bit;
  obj_=gdk_pixmap_create_from_xpm(gdk_const_cast(window),&bit,
				  gdk_const_cast(transparent_color),
				  const_cast<gchar*>(filename.c_str()));
  if(!obj_)
    {
      warn_about_xpm_file_problem(filename);
      create_from_xpm_d(window, mask, transparent_color, defaultPixmap);
    }
  else
    mask=Gdk_Bitmap(bit);

  // no need to ref it here.
  //ref();
}
    
Gdk_Pixmap::Gdk_Pixmap(const Gdk_Drawable  &window,
		       const Gdk_Color   &transparent_color,
		       const Gtk::string      &filename):Gdk_Drawable(0)
{
  create_from_xpm(window,transparent_color,filename);
}
 
Gdk_Pixmap::Gdk_Pixmap(const Gdk_Drawable  &window,
		       Gdk_Bitmap  &bitmap,
		       const Gdk_Color   &transparent_color,
		       const Gtk::string      &filename):Gdk_Drawable(0)
{
  create_from_xpm(window,bitmap,transparent_color,filename);
}  

void Gdk_Pixmap::create_colormap_from_xpm(
					  const Gdk_Drawable   &window,
					  Gdk_Colormap &colormap,
					  Gdk_Bitmap   &bitmap,
					  const Gdk_Color    &transparent_color,
					  const Gtk::string &filename)
{
  unref();
  GdkBitmap *bit;
  obj_=gdk_pixmap_colormap_create_from_xpm(gdk_const_cast(window),
					   colormap,&bit,
					   gdk_const_cast(transparent_color),
					   const_cast<gchar*>(filename.c_str()));

  if(!obj_)
    {
      warn_about_xpm_file_problem(filename);
      create_from_xpm_d(window, bitmap, transparent_color, defaultPixmap);
    }
  else
    bitmap=Gdk_Bitmap(bit);
}


Gdk_Pixmap::Gdk_Pixmap(const Gdk_Drawable   &window,
		       Gdk_Colormap &colormap,
		       Gdk_Bitmap   &bitmap,
		       const Gdk_Color    &transparent_color,
		       const Gtk::string &filename):Gdk_Drawable(0)
{
  create_colormap_from_xpm(window, colormap, bitmap,
			   transparent_color,filename);
}
 
void Gdk_Pixmap::create_from_xpm_d(const Gdk_Drawable &window, 
				   Gdk_Bitmap       &bitmap,
				   const Gdk_Color  &transparent_color,
				   const gchar * const *data)
{
  unref();
  GdkBitmap  *bit;
  obj_=gdk_pixmap_create_from_xpm_d(gdk_const_cast(window),&bit,
				    gdk_const_cast(transparent_color),
				    const_cast<gchar**>(data));
  // no need to ref it here.
  //ref();
  bitmap=Gdk_Bitmap(bit);
}

void Gdk_Pixmap::create_colormap_from_xpm_d (const Gdk_Drawable   &window,
					     Gdk_Colormap &colormap,
					     Gdk_Bitmap  &mask,
					     const Gdk_Color &transparent_color,
					     const gchar * const *data)
{
  unref();
  GdkBitmap  *bit;
  obj_=gdk_pixmap_colormap_create_from_xpm_d(gdk_const_cast(window),
					     colormap,
					     &bit,gdk_const_cast(transparent_color),
					     const_cast<gchar**>(data));
  // no need to ref here?
  //   ref();
  mask=Gdk_Bitmap(bit);
}

void
Gdk_Pixmap::warn_about_xpm_file_problem(const Gtk::string &filename)
{
  g_warning("Couldn't create pixmap out of file '%s' - using internal default",filename.c_str());
}


static const char *const noimage_xpm[] = {
  "20 20 4 1",
    " 	c None",
    ".	c #000000",
    "+	c #E24444",
    "@	c #FFFFFF",
    "....................",
    ".++@@@@@@@@@@@@@@++.",
    ".+++@@@@@@@@@@@@+++.",
    ".@+++@@@@@@@@@@+++@.",
    ".@@+++@@@@@@@@+++@@.",
    ".@@@+++@@@@@@+++@@@.",
    ".@@@@+++@@@@+++@@@@.",
    ".@@@@@+++@@+++@@@@@.",
    ".@@@@@@++++++@@@@@@.",
    ".@@@@@@@++++@@@@@@@.",
    ".@@@@@@@++++@@@@@@@.",
    ".@@@@@@++++++@@@@@@.",
    ".@@@@@+++@@+++@@@@@.",
    ".@@@@+++@@@@+++@@@@.",
    ".@@@+++@@@@@@+++@@@.",
    ".@@+++@@@@@@@@+++@@.",
    ".@+++@@@@@@@@@@+++@.",
    ".+++@@@@@@@@@@@@+++.",
    ".++@@@@@@@@@@@@@@++.",
    "...................."};

const char* const* Gdk_Pixmap::defaultPixmap=noimage_xpm;

