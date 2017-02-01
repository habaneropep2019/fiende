#SUBDIRS = tutorial translated_from_gtk

INCLUDES = -I$(top_srcdir)/src -I$(top_builddir)/src -I$(top_builddir)/src/gtk-- \
           -I$(top_srcdir)/gdk-- \
           $(GTK_CFLAGS) $(SIGC_CFLAGS)

LDADD = $(top_builddir)/src/gtk--/libgtkmm.la \
        $(top_builddir)/gdk--/gdk--/libgdkmm.la $(GTK_LIBS) $(SIGC_LIBS)

noinst_PROGRAMS = testgtk-- testdnd

testdnd_SOURCES = testdnd.C


testgtk___SOURCES =         \
	ButtonBoxTest.h     \
	ButtonBoxTest.C     \
	ButtonTest.h        \
	ButtonTest.C        \
	CheckButtonTest.h   \
	CheckButtonTest.C   \
	CListTest.h         \
	CListTest.C         \
	FileSelectionTest.h \
	FileSelectionTest.C \
	HandleBoxTest.h     \
	HandleBoxTest.C     \
	PaneTest.h          \
	PaneTest.C          \
	PixmapTest.h        \
	PixmapTest.C        \
	RadioButtonTest.h   \
	RadioButtonTest.C   \
	ReparentDemo.h      \
	ReparentDemo.C      \
	SavedPosition.h     \
	SavedPosition.C     \
	StatusbarTest.h     \
	StatusbarTest.C     \
	ToggleButtonTest.h  \
	ToggleButtonTest.C  \
	ToolbarTest.h       \
	ToolbarTest.C       \
	TooltipsTest.h      \
	TooltipsTest.C      \
	TreeTest.h          \
	TreeTest.C          \
	TestFixture.h       \
	TestFixture.C       \
	testgtk--.C 


EXTRA_DIST= 3DRings.xpm     FilesQueue.xpm  Modeller.xpm    test.xpm circles.xbm gtk_mini.xpm



