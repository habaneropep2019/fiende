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

#ifndef GNOMEMM_APP_CALLBACK_H
#define GNOMEMM_APP_CALLBACK_H
#include <gnome--/dialog.h>

namespace Gnome {
 
// With replies
struct ReplyHolder 
  {
    ReplyCallback cb_;
    ReplyHolder(ReplyCallback cb) : cb_(cb) {}
  };

struct StringHolder
  {
    StringCallback cb_;
    StringHolder(StringCallback cb) : cb_(cb) {}
  };

void add_reply(Gnome::Dialog* w,ReplyHolder* rh);
void add_reply(Gnome::Dialog* w,StringHolder* rh);
void reply_call(gint i,void* data);
void string_reply_call(const gchar* i,void* data);

} /* namespace Gnome */

#endif /* GNOMEMM_APP_CALLBACK_H */
