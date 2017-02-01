#include <gdk--/font.h>


GTKMM_USING_STD_STRING;

void Gdk_Font::ref()
  {
   if (obj_) 
     gdk_font_ref(obj_);
  }

void Gdk_Font::unref()
  {
   if (obj_) 
     gdk_font_unref(obj_);
   obj_=0;
  }

Gdk_Font::Gdk_Font():Gdk_Handle<GdkFont>(0) 
  {}

Gdk_Font::Gdk_Font(GdkFont *font):Gdk_Handle<GdkFont>(font)
  {
   ref();
  } 

void Gdk_Font::release()
  {
    unref();
    obj_=0;
  }

Gdk_Font& Gdk_Font::operator = (const Gdk_Font& h)
  {
    if (h.obj_==obj_) return *this;
    unref();
    obj_=h.obj_;
    ref();
    return *this;
  }
  
Gdk_Font::Gdk_Font(const Gdk_Font& font):Gdk_Handle<GdkFont>(font)
  {
   ref();
  }

Gdk_Font::Gdk_Font(const Gtk::string &font_name):Gdk_Handle<GdkFont>(0)
  {
   load(font_name);
  }

Gdk_Font::~Gdk_Font()
  {
   unref();
  }

void Gdk_Font::create(const Gtk::string &font_name)
  {
   load (font_name);
  }

Gdk_Font& Gdk_Font::load(const Gtk::string &font_name)
  {
   unref();
   obj_=gdk_font_load(font_name.c_str());
   return *this;
  }

Gdk_Font& Gdk_Font::set_load(const Gtk::string &fontset_name)
  {
   unref();
   obj_=gdk_fontset_load(const_cast<gchar*>(fontset_name.c_str()));
   return *this;
  }

gint Gdk_Font::font_id() const
  {
   return gdk_font_id(gdk_const_cast(*this));
  }

gint Gdk_Font::equal(const Gdk_Font  &font) const
  {
   return gdk_font_equal(obj_,gdk_const_cast(font));
  }

gint Gdk_Font::ascent() const
  {
    return obj_->ascent;
  }

gint Gdk_Font::descent() const
  {
    return obj_->descent;
  }

gint Gdk_Font::height() const
  {
   return obj_->ascent + obj_->descent;
  }

gint Gdk_Font::string_width(const Gtk::string &str) const
  {
   return gdk_string_width(*this,str.c_str());
  }

gint Gdk_Font::text_width(const char* text,int length) const
  {
   return gdk_text_width(*this,text,length);
  }

gint Gdk_Font::char_width(gchar     character) const
  {
   return gdk_char_width(*this,character);
  }

gint Gdk_Font::string_measure(const Gtk::string &str) const
  {
   return gdk_string_measure(*this,str.c_str());
  }

gint Gdk_Font::text_measure(const char* text,int length) const
  {
   return gdk_text_measure(*this,text,length);
  }

gint Gdk_Font::char_measure(gchar     character) const
  {
   return gdk_char_measure(*this,character);
  }
#if GDK_VERSION_GT(1,0)
gint Gdk_Font::string_height(const Gtk::string &str) const
  {
   return gdk_string_height(*this,str.c_str());
  }

gint Gdk_Font::text_height(const char* text,int length) const
  {
   return gdk_text_height(*this,text,length);
  }

gint Gdk_Font::char_height(gchar character) const
  {
   return gdk_char_height(*this,character);
  }

void Gdk_Font::string_extents (const Gtk::string &str,
gint        &lbearing,
gint        &rbearing,
gint        &width,
gint        &ascent,
gint        &descent) const
  {
   gdk_string_extents(*this,str.c_str(),&lbearing,&rbearing,&width,
     &ascent,&descent);
  }

void Gdk_Font::text_extents(const char* text, 
                  gint length,
                  gint &lbearing,
                  gint &rbearing,
                  gint &width,
                  gint &ascent,
                  gint &descent) const
  {
   gdk_text_extents(*this,text,length,&lbearing,&rbearing,&width,
     &ascent,&descent);
  }

/* These wrappers are quit raw.  We need to review them later */
gint Gdk_Font::text_width_wc(const GdkWChar* text,gint text_length) const
  {
   return gdk_text_width_wc(*this,text,text_length);
  }

gint Gdk_Font::char_width_wc(GdkWChar character) const
  {
   return gdk_char_width_wc(*this,character);
  }

void Gdk_Font::text_extents_wc( const GdkWChar *text,                              
                      gint            text_length,
                      gint           &lbearing,
                      gint           &rbearing,
                      gint           &width,
                      gint           &ascent,
                      gint           &descent) const
  {
   gdk_text_extents_wc(*this,text,text_length,&lbearing,&rbearing,
     &width,&ascent,&descent);
  }

#endif

