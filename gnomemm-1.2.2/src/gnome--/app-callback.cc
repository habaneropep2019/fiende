/* 
 * Copyright 2000 Karl Nelson
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
 *
 */

#include <gnome--/app-callback.h>

namespace Gnome {
 
void reply_call(gint i,void* data)
  {
    if (data)
      ((ReplyHolder*)data)->cb_.call(i);
  }

void reply_destroy(void* data)
  {
    delete (ReplyHolder*)data;
  }

void add_reply(Gnome::Dialog* w,ReplyHolder* rh)
  {
    if (!w)    
      delete rh;
    else
      w->set_data_full("gnomemm-dialog-cb",rh,&reply_destroy);
  }

void string_reply_call(const gchar* i,void* data)
  {
    if (data)
      ((StringHolder*)data)->cb_.call(i?i:"");
  }

void string_reply_destroy(void* data)
  {
    delete (StringHolder*)data;
  }

void add_reply(Gnome::Dialog* w,StringHolder* rh)
  {
    if (!w)
      delete rh;
    else
      w->set_data_full("gnomemm-dialog-cb",rh,&string_reply_destroy);
  }

}
