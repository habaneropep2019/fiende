# ...for those missing static libraries...
#
# in order to build devel packages with static libs included you have to
# change '--disable-static' to '--enable-static' and uncomment the line
# containing the pattern ".../*.a" at the files section

%define  RELEASE 1
%define  rel     %{?CUSTOM_RELEASE} %{!?CUSTOM_RELEASE:%RELEASE}

Summary: 	A C++ interface for Gnome libs (a GUI library for X).
Name: 		gnomemm
Version: 	1.2.2
Release: 	%rel
Copyright: 	LGPL
Group: 		System Environment/Libraries
Packager:	Herbert Valerio Riedel <hvr@gnu.org>
Source: 	ftp://download.sourceforge.net/gtkmm/%{name}-%{version}.tar.gz
URL: 		http://gtkmm.sourceforge.net/
Prefix: 	/usr
BuildRoot: 	/var/tmp/%{name}-%{version}-root
Requires:	gnome-libs, gtkmm

%description
This package provides a C++ interface for GnomeUI.  It is a subpackage
of the Gtk-- project.  The interface provides a convenient interface for C++
programmers to create Gnome GUIs with GTK+'s flexible object-oriented 
framework.

%package	devel
Summary: 	Headers for developing programs that will use Gnome--.
Group: 		Development/Libraries
Requires:       %{name}, gnome-libs-devel

%description    devel
This package contains the headers that programmers will need to develop
applications which will use Gnome--, part of Gtk-- the C++ interface to 
the GTK+ (the Gimp ToolKit) GUI library.

%prep
%setup -q

%build

# ...hope this can be removed soon
%ifarch alpha
	ARCH_FLAGS="--host=alpha-linux-gnu"
%endif

# Needed for snapshot releases.
if [ ! -f configure ]; then
	CFLAGS="$RPM_OPT_FLAGS" ./autogen.sh $ARCH_FLAGS \
		--prefix=%{prefix} \
		--disable-static \
		--enable-shared --enable-docs
else
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

%install
if [ -d $RPM_BUILD_ROOT ]; then rm -rf $RPM_BUILD_ROOT; fi
make DESTDIR=$RPM_BUILD_ROOT install

# replace examples.conf by a really simple one
(
  echo 'CXXBUILD = g++ -O2 $< -o $@ `gnome-config gnomemm --cflags --libs` '
  echo 'CXXCOMPILE = g++ -O2 -c $< -o $@ `gnome-config gnomemm --cflags` '
  echo 'CXXLINK = g++ -O2 -o $@ `gnome-config gnomemm --libs` '
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
#{prefix}/bin/*
%{prefix}/include/*.h
%{prefix}/include/gnome--
%{prefix}/lib/*.la
#{prefix}/lib/*.a # uncomment this one for static libs 
%{prefix}/lib/*.sh
%{prefix}/lib/*.so
#{prefix}/lib/gtkmm
#{prefix}/man/man3/*
#{prefix}/share/aclocal/gtk--.m4
%{prefix}/share/aclocal/gnome--.m4

###########################################################################
%changelog
* Tue Mar 20 2001 Eric Bourque <ericb@computer.org>
- added gnome--.m4 to files devel section

* Sat Mar 10 2001 Herbert Valerio Riedel <hvr@gnu.org>
- improved examples.conf
- fixed example build problems

* Thu May 11 2000 Herbert Valerio Riedel <hvr@gnu.org>
- removed lib/gtkmm from files section
- removed empty obsolete tags

* Sun Jan 30 2000 Karl Einar Nelson <kenelson@sourceforge.net>
- adapted from gtk--.spec
