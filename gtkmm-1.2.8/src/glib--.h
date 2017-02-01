// This is -*- C++ -*-
/* $Id: glib--.h,v 1.2 1998/12/03 20:18:58 hp Exp $ */
/* 
 * glib--.h
 *
 * Copyright (C) 1998 EMC Capital Management, Inc.
 *
 * Developed by Havoc Pennington <hp@emccta.com>.
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

// This file contains assorted GLib wrappers; no plans to be "complete,"
//  since the STL is often nicer anyway so most of glib is not useful.

#ifndef GLIB_MINUS_MINUS_H
#define GLIB_MINUS_MINUS_H

#include <string>
#include <glib.h>   // DO NOT add includes for Gdk or Gtk; this is separate.

class G_Date {
  GDate gdate;
public:
  G_Date() { g_date_clear(&gdate, 1); }
  G_Date(GDateDay d, GDateMonth m, GDateYear y) { 
    g_date_clear(&gdate, 1); 
    g_date_set_dmy(&gdate, d, m, y); 
  }
  G_Date(guint32 julian_day) {
    g_date_clear(&gdate, 1);  
    g_date_set_julian(&gdate, julian_day); 
  }
  ~G_Date() {}

  // Accessors
  
  bool valid() const { return g_date_valid(const_cast<GDate*>(&gdate)); }
  
  GDateWeekday weekday() const { return g_date_weekday(const_cast<GDate*>(&gdate)); }
  GDateDay     day() const     { return g_date_day(const_cast<GDate*>(&gdate)); }
  GDateMonth   month() const   { return g_date_month(const_cast<GDate*>(&gdate)); }
  GDateYear    year() const    { return g_date_year(const_cast<GDate*>(&gdate)); }
  guint32      julian() const  { return g_date_julian(const_cast<GDate*>(&gdate)); }
  guint        day_of_year() const { return g_date_day_of_year(const_cast<GDate*>(&gdate)); }
  guint        monday_week_of_year() const { return g_date_monday_week_of_year(const_cast<GDate*>(&gdate)); }
  guint        sunday_week_of_year() const { return g_date_sunday_week_of_year(const_cast<GDate*>(&gdate)); }
  
  bool is_first_of_month() const { return  g_date_is_first_of_month(const_cast<GDate*>(&gdate)); }
  bool is_last_of_month()  const { return  g_date_is_last_of_month(const_cast<GDate*>(&gdate)); }

  
  // Mutators

  void clear() { g_date_clear(&gdate, 1); }
  
  void set_parse(const gchar* text) { g_date_set_parse(&gdate, text); }
  void set_parse(const string & text) { g_date_set_parse(&gdate, text.c_str()); }
  
  void set_time   (GTime t)      { g_date_set_time(&gdate, t); }
  void set_month  (GDateMonth m)  { g_date_set_month(&gdate, m); }
  void set_day    (GDateDay   d)  { g_date_set_day(&gdate, d); }
  void set_year   (GDateYear  y)  { g_date_set_year(&gdate, y); }
  void set_dmy    (GDateDay   d,
                   GDateMonth m,
                   GDateYear  y)  { g_date_set_dmy(&gdate, d, m, y); }
  void set_julian (guint32 j)     { g_date_set_julian(&gdate, j); }

  void add_days   (guint ndays)   { g_date_add_days(&gdate, ndays); }
  void add_months (guint nmonths) { g_date_add_months(&gdate, nmonths); }
  void add_years  (guint nyears)  { g_date_add_years(&gdate, nyears); }

  void subtract_days   (guint ndays)   { g_date_subtract_days(&gdate, ndays); }
  void subtract_months (guint nmonths) { g_date_subtract_months(&gdate, nmonths); }
  void subtract_years  (guint nyears)  { g_date_subtract_years(&gdate, nyears); }

  // misc
  
  gsize strftime (gchar* s, gsize slen, const gchar* format) const { return g_date_strftime(s,slen,format,const_cast<GDate*>(&gdate)); }
  
  void to_struct_tm (struct tm* tm) const { g_date_to_struct_tm(const_cast<GDate*>(&gdate), tm); }

  // see also the < > == operators defined below
  int  compare(const G_Date& rhs) const { return g_date_compare(const_cast<GDate*>(&gdate),
                                                                const_cast<GDate*>(&rhs.gdate)); }

  // Static functions
  static bool valid_month  (GDateMonth   m) { return g_date_valid_month   (m); }
  static bool valid_year   (GDateYear    y) { return g_date_valid_year    (y); }
  static bool valid_day    (GDateDay     d) { return g_date_valid_day     (d); }
  static bool valid_weekday(GDateWeekday w) { return g_date_valid_weekday (w); }
  static bool valid_julian (guint32      j) { return g_date_valid_julian  (j); }
  static bool valid_dmy    (GDateDay     d,
                            GDateMonth   m,
                            GDateYear    y) { return g_date_valid_dmy(d,m,y); }

  static bool is_leap_year (GDateYear    y) { return g_date_is_leap_year(y); }

  static guint days_in_month (GDateMonth m, GDateYear y) { return g_date_days_in_month(m,y); }
  static guint monday_weeks_in_year(GDateYear y) { return g_date_monday_weeks_in_year(y); }
  static guint sunday_weeks_in_year(GDateYear y) { return g_date_sunday_weeks_in_year(y); }
  
  // C++ sugar

  G_Date(const G_Date & src) { gdate = src.gdate; }
  G_Date& operator=(const G_Date & src) { gdate = src.gdate; return *this; } // works for self-assign
  
  bool operator<(const G_Date & rhs) {  
    return (g_date_compare(const_cast<GDate*>(&gdate),
                           const_cast<GDate*>(&rhs.gdate)) < 0);
  }

  bool operator>(const G_Date & rhs) {  
    return (g_date_compare(const_cast<GDate*>(&gdate),
                           const_cast<GDate*>(&rhs.gdate)) > 0);
  }

  bool operator==(const G_Date & rhs) {  
    return (g_date_compare(const_cast<GDate*>(&gdate),
                           const_cast<GDate*>(&rhs.gdate)) == 0);
  }

  bool operator<=(const G_Date & rhs) {  
    return (*this < rhs) || (*this == rhs);
  }

  bool operator>=(const G_Date & rhs) {  
    return (*this > rhs) || (*this == rhs);
  }

};

#endif // GLIB_MINUS_MINUS_H

// $Id: glib--.h,v 1.2 1998/12/03 20:18:58 hp Exp $
