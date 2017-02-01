SUBDIRS = testgtk--

EXAMPLE_DIR = arrow aspectframe base buttonbox buttons clist ctree drawingarea entry eventbox filesel fixed frame helloworld helloworld2 idle label layout list menu notebook packbox packer paned pixmap progressbar rangewidgets radiobuttons rulers scribble-simple scrolledwin selection spinbutton statusbar table text tictactoe timeout toolbar wheelbarrow
# tree calendar gtkdial 

EXTRA_DIST= \
examples.conf.in			\
arrow/Makefile				\
arrow/arrow.cc				\
arrow/direction.cc			\
aspectframe/Makefile			\
aspectframe/aspectframe.cc		\
base/Makefile				\
base/base.cc				\
buttonbox/Makefile			\
buttonbox/buttonbox.cc			\
buttons/Makefile			\
buttons/buttons.cc			\
buttons/info.xpm			\
calendar/Makefile			\
calendar/gcalendar.cc			\
clist/Makefile				\
clist/clist.cc				\
ctree/Makefile				\
ctree/ctree.cc				\
dialog/Makefile				\
dialog/dialog1.cc			\
dialog/dialog2.cc			\
dialog/dialog3.cc			\
drawingarea/Makefile			\
drawingarea/drawingarea.cc		\
entry/Makefile				\
entry/entry.cc				\
eventbox/Makefile			\
eventbox/eventbox.cc			\
filesel/Makefile			\
filesel/filesel.cc			\
fixed/Makefile				\
fixed/fixed.cc				\
frame/Makefile				\
frame/frame.cc				\
gtkdial/Makefile			\
gtkdial/dial_test.cc			\
gtkdial/gtkdial.cc			\
gtkdial/gtkdial.h			\
helloworld/Makefile			\
helloworld/helloworld.cc		\
helloworld2/Makefile			\
helloworld2/helloworld2.cc		\
idle/Makefile                           \
idle/idle.cc                            \
input/Makefile                          \
input/input.cc                          \
label/Makefile				\
label/label.cc				\
layout/Makefile				\
layout/layoutarea.cc			\
list/Makefile				\
list/list.cc				\
menu/Makefile				\
menu/menu.cc				\
menu/menu2.cc				\
notebook/Makefile			\
notebook/notebook.cc			\
packbox/Makefile			\
packbox/packbox.cc			\
packer/Makefile				\
packer/packer.cc			\
paned/Makefile				\
paned/paned.cc				\
pixmap/Makefile				\
pixmap/pixmap.cc			\
progressbar/Makefile			\
progressbar/progressbar.cc		\
progressbar/progressbar2.cc		\
radiobuttons/Makefile			\
radiobuttons/radiobuttons.cc		\
rangewidgets/Makefile			\
rangewidgets/rangewidgets.cc		\
rulers/Makefile				\
rulers/rulers.cc			\
scribble-simple/Makefile		\
scribble-simple/scribble-simple.cc	\
scrolledwin/Makefile			\
scrolledwin/scrolledwin.cc		\
selection/Makefile			\
selection/gettargets.cc			\
selection/setselection.cc		\
spinbutton/Makefile			\
spinbutton/spinbutton.cc		\
statusbar/Makefile			\
statusbar/statusbar.cc			\
table/Makefile				\
table/table.cc				\
text/Makefile				\
text/text.cc				\
tictactoe/Makefile			\
tictactoe/tictactoe.cc			\
tictactoe/tictactoe.h			\
tictactoe/ttt_test.cc			\
timeout/Makefile                        \
timeout/timeout.cc                      \
toolbar/Makefile                        \
toolbar/toolbar.cc                      \
tree/Makefile				\
tree/tree.cc				\
wheelbarrow/Makefile			\
wheelbarrow/wheelbarrow.cc		


INCLUDES = -I$(top_srcdir)/src -I$(top_builddir)/src -I$(top_builddir)/src/gtk-- \
           -I$(top_srcdir)/gdk-- \
           $(GTK_CFLAGS) \
           $(SIGC_CFLAGS)

LDADD = $(top_builddir)/src/gtk--/libgtkmm.la \
        $(top_builddir)/gdk--/gdk--/libgdkmm.la $(GTK_LIBS) $(SIGC_LIBS)

# ctreetest removed since no source exists. 
noinst_PROGRAMS = 

examples:
	for subdir in $(EXAMPLE_DIR); do \
	(cd $$subdir; make all) ; \
	done 

examples-clean:
	for subdir in $(EXAMPLE_DIR); do \
	(cd $$subdir; make clean) ; \
	done 

