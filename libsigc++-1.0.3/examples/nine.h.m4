// Okay, so you want the signal system to expand to accept a 
// number of arguments not in the distribution.  You could hack the 
// library header files to support it, but then you would have to 
// distribute these with your program.  Further, that copy would fall 
// out of date, and probably wouldn't compile later.  Or you can request
// that we add one more argument to distribution.  This increasing the size
// of everyones library.  This would quickly get out of hand, So not a 
// good idea.
//
// Fortunately, you can just use the current macros provided and ask 
// them to build a header file for any number of arguments you require.  
// 
// Here is how to do it: (using Signal9 as an example)
//


// (1) Include the master template macros.
include(template.macros.m4)


// (2) Use minclude to take the macros from the standard distrubution.
//  minclude dumps all of the header files normal output.
//  (order doesn't matter)
minclude(slot.h.m4)
minclude(basic_signal.h.m4)
minclude(func_slot.h.m4)
minclude(object_slot.h.m4)


// (3) Place the usual C declarations, comments and such 
//   (Be warned:  m4 can interpret things as m4 macros if not 
//    careful and will eat braces.  Therefore, keep it to a minumum.)
#ifndef _NINE_H_
#define _NINE_H_


// (4) Call the macros with the number of arguments you require.
//   (order matters)

// (4a) Because this is a new signal type, an slot must be build
//   first.
SLOT(ARGS(P,9))

// (4b) Now we can make the signal.  
BASIC_SIGNAL(ARGS(P,9))

// (4c) Then define the slot types we wish to use.
//   (you don't need to make all of the slots, just the ones you plan to use)
FUNCTION_SLOT(ARGS(P,9))
OBJECT_SLOT(ARGS(P,9))

// (if you just need a new slot, say a Data slot with 3 callbacks for an
//  already defined signal type, skip steps 4a and 4b)
#endif


//
// (5) Run m4 against the macro file and redirect to your new header file
//   m4 -I/usr/local/include/sigc++/macros nine.h.m4 > nine.h
//
// (6) Include this new header file in your program.
//
// (7) Distribute only the m4 file, so that changes in the library are
//   reflected in your header file on the users system.
//


