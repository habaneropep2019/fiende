# $Id: Makefile.am,v 1.20 2001/07/03 18:40:12 cactus Exp $

EXTRA_DIST = convert.m4 stage1.m4 gtkconvert.m4 list.m4 doc1.m4 parser.h

procdir = $(prefix)/lib/gtkmm/proc
proc_PROGRAMS = gtkmmproc 


gtkmmproc_SOURCES = parser.yy gtkmmproc.cc lexer.ll fixate.cc
noinst_HEADERS = gtkmmproc.h

m4includedir = $(prefix)/lib/gtkmm/proc
m4include_HEADERS = doc1.m4 stage1.m4 convert.m4 list.m4 gtkconvert.m4

YFLAGS = -d
CXXFLAGS +=
CPPFLAGS += -D MACRO_DIR=\"$(m4includedir)\" -D M4=\"@M4@\"

lexer: lexer.cc gtkmmproc.cc
	$(CXX) -D LEXER_TEST $CXXFLAGS lexer.cc gtkmmproc.cc -o lexer
fixate: fixate.cc 
	$(CXX) -D FIXATE_TEST fixate.cc -o fixate

