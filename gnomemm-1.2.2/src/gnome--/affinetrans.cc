// -*- C++ -*-
/* $Id: affinetrans.cc,v 1.4 2001/03/08 15:47:16 murrayc Exp $ */

/* affinetrans.h
 * 
 * Copyright (C) 1999 The Gnome-- Development Team
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
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <affinetrans.h>
#include <libgnomeui/gnome-canvas.h>

namespace Gnome
{

namespace Art
{

AffineTrans::AffineTrans(gdouble scale = 1.0)
{
  trans_[0] = scale;
  trans_[1] = 0.0;
  trans_[2] = 0.0;
  trans_[3] = scale;
  trans_[4] = 0.0;
  trans_[5] = 0.0;
}

AffineTrans::AffineTrans(double aff[])
{
	trans_[0] = aff[0];
	trans_[1] = aff[1];
	trans_[2] = aff[2];
	trans_[3] = aff[3];
	trans_[4] = aff[4];
	trans_[5] = aff[5];
}

AffineTrans::AffineTrans(const AffineTrans& src)
{
  operator=(src);
}

AffineTrans::~AffineTrans()
{
}

AffineTrans& AffineTrans::operator=(const AffineTrans& src)
{
  for(unsigned int i = 0; i < 6; i++)
  {
    trans_[i] = src.trans_[i];
  }

  return *this;
}

double&
AffineTrans::operator[](unsigned int idx)
{
  if(idx > 5)
    {
      g_warning("AffineTrans::operator[] called with idx > 5");
      return trans_[5]; //0; //Can't convert 0 to double& - throw exception?
    }
  return trans_[idx];
}

const double&
AffineTrans::operator[](unsigned int idx) const
{
  if(idx > 5)
    {
      g_warning("AffineTrans::operator[] const called with idx > 5");
      return trans_[5]; //0; //Can't convert 0 to double& - throw exception?
    }
  return trans_[idx];
}

Point AffineTrans::apply_to(const Point& p) const
{
  Point result;
  art_affine_point(result.obj(), p.obj(), trans_);
  return result;
}
  
Point AffineTrans::operator*(const Point& p) const
{
  return apply_to(p);
}

AffineTrans
AffineTrans::operator*(const AffineTrans& aff2)
{
  AffineTrans result;
  art_affine_multiply(result.obj(), obj(), aff2.obj());
  return result;
}

gboolean AffineTrans::operator==(AffineTrans& other)
{
  return art_affine_equal(trans_, other.obj());
}
 
gboolean AffineTrans::operator!=(AffineTrans& other)
{
  return !art_affine_equal(trans_, other.obj());
}
                        
void AffineTrans::invert()
{
  art_affine_invert(trans_, trans_);
}
  
void AffineTrans::flip(gboolean horiz, gboolean vert)
{
  art_affine_flip(trans_, trans_, horiz, vert);
}

gboolean AffineTrans::rectilinear() const
{
  return art_affine_rectilinear(trans_);
}

gdouble AffineTrans::expansion() const
{
  return art_affine_expansion(trans_);
}

AffineTrans const &
AffineTrans::operator*=(AffineTrans& other)
{
  art_affine_multiply(obj(), obj(), other.obj());
  return *this;
}


AffineTrans AffineTrans::identity()
{
  AffineTrans tmp;
	art_affine_identity(tmp.obj());
	return tmp;
}

AffineTrans
AffineTrans::scaling(gdouble s)
{
  return scaling(s, s);
}

AffineTrans
AffineTrans::scaling(gdouble sx, gdouble sy)
{
	AffineTrans tmp;
	art_affine_scale(tmp.obj(), sx, sy);
	return tmp;
}

AffineTrans
AffineTrans::rotation(gdouble theta)
{
	AffineTrans tmp;
	art_affine_rotate(tmp.obj(), theta);
	return tmp;
}

AffineTrans
AffineTrans::translation(gdouble dx, gdouble dy)
{
	AffineTrans tmp;
	art_affine_translate(tmp.obj(), dx, dy);
	return tmp;
}

AffineTrans
AffineTrans::translation(const Point& p)
{
	AffineTrans tmp;
	art_affine_translate(tmp.obj(), p.get_x(), p.get_y());
	return tmp;
}


AffineTrans
AffineTrans::shearing(gdouble theta)
{
	AffineTrans tmp;
	art_affine_shear(tmp.obj(), theta);
	return tmp;
}

gdouble* AffineTrans::obj()
{
  return trans_;
} 

const gdouble* AffineTrans::obj() const
{
  return trans_;
}

Gtk::string AffineTrans::to_string() const
{
  char pchStr[128];
  pchStr[127] = 0; //Just in case art_affine_to_string doesn't work properly.
  art_affine_to_string(pchStr, obj());
  return Gtk::string(pchStr);
}

} //namespace Art

} //namespace Gnome

std::ostream& operator<<(std::ostream& out, const Gnome::Art::AffineTrans& aff)
{
  return out << aff.to_string();
}
