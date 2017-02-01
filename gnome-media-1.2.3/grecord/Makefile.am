SUBDIRS = src

EXTRA_DIST = \
	Soundrecorder.desktop.in README COPYING AUTHORS INSTALL \
	pixmaps/gnome_mixer.xpm grecord.keys.in grecord.mime \
	gnome-grecord.png

Applicationsdir = $(datadir)/gnome/apps/Multimedia
Applications_in_files = Soundrecorder.desktop.in 
Applications_DATA = $(Applications_in_files:.desktop.in=.desktop)
@XML_I18N_MERGE_DESKTOP_RULE@

Mimedir = $(datadir)/mime-info
Mime_DATA = grecord.keys grecord.mime

Icondir = $(datadir)/pixmaps
Icon_DATA = gnome-grecord.png

grecord.keys: grecord.keys.in Makefile
	sed -e 's^\@icondir\@^$(appicondir)^g' < $(srcdir)/grecord.keys.in > grecord.keys.tmp \
	&& mv grecord.keys.tmp grecord.keys 
