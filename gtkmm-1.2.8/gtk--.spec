# ...for those missing static libraries...
#
# in order to build devel packages with static libs included you have to
# change '--disable-static' to '--enable-static' and uncomment the line
# containing the pattern ".../*.a" at the files section

%define  RELEASE 1
%define  rel     %{?CUSTOM_RELEASE} %{!?CUSTOM_RELEASE:%RELEASE}

Summary: 	A C++ interface for the GTK+ (a GUI library for X).
Name: 		gtkmm
Version: 	1.2.8
Release: 	%rel
Copyright: 	LGPL
Group: 		System Environment/Libraries
Packager:	Herbert Valerio Riedel <hvr@gnu.org>
Source: 	ftp://ftp.gtk.org/pub/gtk/gtk--/%{name}-%{version}.tar.gz
URL: 		http://gtkmm.sourceforge.net/
Prefix: 	/usr
BuildRoot: 	/var/tmp/%{name}-%{version}-root
Obsoletes:	Gtk--, gtk--
Requires:	gtk+, glib, libsigc++

%description
This package provides a C++ interface for GTK+ (the Gimp ToolKit) GUI
library.  The interface provides a convenient interface for C++
programmers to create GUIs with GTK+'s flexible object-oriented framework.
Features include type safe callbacks, widgets that are extensible using
inheritance and over 110 classes that can be freely combined to quickly
create complex user interfaces.


%package	devel
Summary: 	Headers for developing programs that will use Gtk--.
Group: 		Development/Libraries
Obsoletes:	Gtk---devel, gtk---devel
Requires:       %{name}, gtk+-devel, glib-devel, libsigc++-devel

%description    devel
This package contains the headers that programmers will need to develop
applications which will use Gtk--, the C++ interface to the GTK+
(the Gimp ToolKit) GUI library.

%prep
%setup -q

%build

# ...hope this can be removed soon
%ifarch alpha
	ARCH_FLAGS="--host=alpha-linux-gnu"
%endif

# Needed for snapshot releases.
if [ ! -f configure ]; then
	CFLAGS="$RPM_OPT_FLAGS" CXXFLAGS="$RPM_OPT_FLAGS" ./configure  $ARCH_FLA
	CFLAGS="$RPM_OPT_FLAGS" ./autogen.sh $ARCH_FLAGS \
		--prefix=%{prefix} \
		--disable-static \
		--enable-shared --enable-docs
else
	CFLAGS="$RPM_OPT_FLAGS" CXXFLAGS="$RPM_OPT_FLAGS" ./configure  $ARCH_FLA
	CFLAGS="$RPM_OPT_FLAGS" ./configure  $ARCH_FLAGS \
		--prefix=%{prefix} \
		--disable-maintainer-mode \
		--disable-static \
		--enable-shared --enable-docs
fi

if [ "$SMP" != "" ]; then
  make -j$SMP "MAKE=make -j$SMP"
  make
else
  make
fi

# since the tutorial and FAQ is not build by default
make -C docs/tutorial/
# disabled for now... since it requires other sgmtools
#make -C docs/FAQ/

%install
if [ -d $RPM_BUILD_ROOT ]; then rm -rf $RPM_BUILD_ROOT; fi
make DESTDIR=$RPM_BUILD_ROOT install

# replace examples.conf by a really simple one
(
  echo 'CXXBUILD = g++ -O2 $< -o $@ `gtkmm-config --cflags --libs` '
  echo 'CXXCOMPILE = g++ -O2 -o $@ `gtkmm-config --cflags` '
  echo 'CXXLINK = g++ -O2 -o $@ `gtkmm-config --libs`'
) > examples/examples.conf

rm examples/Makefile.am examples/Makefile.in examples/Makefile
rm examples/examples.conf.in

# strip down the docs 
find docs/ \
\( 	-name 'Makefile' -or	\
	-name 'Makefile.in' -or	\
	-name 'Makefile.am' -or	\
	-name '*.m4' -or	\
	-name 'html' -or	\
	-name 'header' -or 	\
	-name '*.h' 		\
\)	-exec rm -rf {} \;

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-, root, root)
%doc AUTHORS COPYING ChangeLog INSTALL NEWS README
%{prefix}/lib/*.so.*

%files  devel
%defattr(-, root, root)
%doc examples/ docs/  AUTHORS COPYING ChangeLog INSTALL NEWS README
%{prefix}/bin/*
%{prefix}/include/*.h
%{prefix}/include/gdk--
%{prefix}/include/gtk--
%{prefix}/lib/*.la
#{prefix}/lib/*.a # uncomment this one for static libs 
%{prefix}/lib/*.so
%{prefix}/lib/gtkmm
#{prefix}/man/man3/*
%{prefix}/share/aclocal/gtk--.m4

###########################################################################
%changelog
* Sat Mar 10 2001 Herbert Valerio Riedel <hvr@gnu.org>
- improved examples.conf

* Sun Feb 21 2001 Murray Cumming <murrayc@usa.net>
- Corrected URL.

* Sun May 21 2000 Herbert Valerio Riedel <hvr@gnu.org>
- fixed up documentation in gtkmm-devel

* Sun May 07 2000 Herbert Valerio Riedel <hvr@gnu.org>
- CXXFLAGS are set too

* Sun Feb 20 2000 Herbert Valerio Riedel <hvr@gnu.org>
- gnome-- and gtk-- are packaged separately

* Fri Jan 28 2000 Herbert Valerio Riedel <hvr@gnu.org>
- adapted to the new docs

* Sun Jan  2 2000 Herbert Valerio Riedel <hvr@gnu.org>
- examples should be makeable now

* Sun Dec 26 1999 Herbert Valerio Riedel <hvr@gnu.org>
- commented out manpages for now...

* Sat Dec 25 1999 Herbert Valerio Riedel <hvr@gnu.org>
- added dependancies on libsigc++

* Sat Nov  6 1999 Herbert Valerio Riedel <hvr@gnu.org>
- cleanup for 1.1.x
- changed rpm package name from Gtk-- to gtkmm
- removed that static hack

* Sat Oct 21 1999 Karl Einar Nelson <kenelson@ece.ucdavis.edu>
- Changed dist from Gtk--- to gtkmm-

* Sat Sep 11 1999 Herbert Valerio Riedel <hvr@gnu.org>
- added SMP support
- added custom release feature

* Sun Aug  1 1999 Herbert Valerio Riedel <hvr@gnu.org>
- Updated to gtk---1.1.x

* Thu Jul 29 1999 Herbert Valerio Riedel <hvr@gnu.org>
- Updated to gtk---1.0.x
- Merged in changes from redhat's gtk--.spec
- conditional build of static libraries by define 'STATIC'

* Thu May 10 1998 Bibek Sahu <scorpio@dodds.net>
- Upgraded to gtk---0.9.3

* Thu Apr 30 1998 Bibek Sahu <scorpio@dodds.net>
- Fixed problem with gtk---devel requiring libgtk-- (not gtk--).  Oops.

* Thu Apr 30 1998 Bibek Sahu <scorpio@dodds.net>
- Fixed problem with most of the headers not being included.

* Thu Apr 30 1998 Bibek Sahu <scorpio@dodds.net>
- Upgraded to gtk---0.9.1

* Tue Apr 28 1998 Bibek Sahu <scorpio@dodds.net>
- Fixed to build gtk-- and gtk---devel packages.

* Tue Apr 28 1998 Bibek Sahu <scorpio@dodds.net>
- First (s)rpm build.

