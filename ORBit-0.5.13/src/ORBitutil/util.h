#ifndef UTIL_H
#define UTIL_H 1

#include <glib.h>

#define ORBIT_DEBUG 1

#ifdef ORBIT_DEBUG
#define ORBIT_DEBUG_NOTE(x) (x)
#else
#define ORBIT_DEBUG_NOTE(x)
#endif


#define BitTest(f, bit)  ((f) & (1<<(bit)))
#define BitSet(f, bit) ((f) |= (1<<(bit)))
#define BitClr(f, bit) ((f) &= ~(1<<(bit)))
/* Align an address upward to a boundary, expressed as a number of bytes.
   E.g. align to an 8-byte boundary with argument of 8.  */

/*
 *   (this + boundary - 1)
 *          &
 *    ~(boundary - 1)
 */

#define ALIGN_ADDRESS(this, boundary) \
  ((gpointer)((( ((unsigned long)(this)) + (((unsigned long)(boundary)) -1)) & (~(((unsigned long)(boundary))-1)))))


#include <ORBitutil/thread-safety.h>
#include <ORBitutil/trace.h>
#include <ORBitutil/compat.h>
#include <ORBitutil/os-specifics.h>

#endif
