# Note that this is NOT a relocatable package
%define ver      1.2.3
%define  RELEASE 1
%define  rel     %{?CUSTOM_RELEASE} %{!?CUSTOM_RELEASE:%RELEASE}
%define prefix   /usr

Summary: GNOME media programs
Name: gnome-media
Version: %ver
Release: %rel
Copyright: LGPL
Group: X11/Libraries
Source: ftp://ftp.gnome.org/pub/GNOME/sources/gnome-media-%{ver}.tar.gz
BuildRoot: /var/tmp/gnome-media-%{PACKAGE_VERSION}-root
Obsoletes: gnome

URL: http://www.gnome.org
Docdir: %{prefix}/doc
Requires: gnome-libs >= 0.99.8
Summary(es): Programas multimedia de GNOME
Summary(fr): Programmes multimédia de GNOME

%description
GNOME media programs.

GNOME is the GNU Network Object Model Environment.  That's a fancy
name but really GNOME is a nice GUI desktop environment.  It makes
using your computer easy, powerful, and easy to configure.

%description -l es
Programas multimedia GNOME.

GNOME (GNU Network Object Model Environment) es un entorno gráfico
orientado escritorio. Con él el uso de su computadora es más fácil,
agradable y eficaz.

Este paquete contiene varios juegos para el entorno Gnome.

%description -l fr
Programmes multimédia GNOME.

GNOME (GNU Network Object Model Environment) est un environnement graphique
de type bureau. Il rends l'utilisation de votre ordinateur plus facile,
agréable et eficace, et est facile à configurer.

%changelog

* Sat Nov 21 1998 Pablo Saratxaga <srtxg@chanae.alphanet.ch>

- added spanish and french translations for rpm

* Wed Sep 23 1998 Michael Fulbright <msf@redhat.com>

- Updated to 0.30 release

* Mon Mar 16 1998 Marc Ewing <marc@redhat.com>

- Integrate into gnome-media CVS source tree

%prep
%setup

%build
# Needed for snapshot releases.
if [ ! -f configure ]; then
%ifarch alpha
  CFLAGS="$RPM_OPT_FLAGS" ./autogen.sh --host=alpha-redhat-linux --prefix=%prefi
x 
%else
  CFLAGS="$RPM_OPT_FLAGS" ./autogen.sh --prefix=%prefix 
%endif
else
%ifarch alpha
  CFLAGS="$RPM_OPT_FLAGS" ./configure --host=alpha-redhat-linux --prefix=%prefix
%else
  CFLAGS="$RPM_OPT_FLAGS" ./configure --prefix=%prefix 
%endif
fi

if [ "$SMP" != "" ]; then
  (make "MAKE=make -k -j $SMP"; exit 0)
  make
else
  make
fi

%install
rm -rf $RPM_BUILD_ROOT

make prefix=$RPM_BUILD_ROOT%{prefix} install

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-, root, root)

%doc AUTHORS COPYING ChangeLog NEWS README
%{prefix}/bin/*
%{prefix}/share/locale/*/*/*
%config %{prefix}/share/gnome/cddb-submit-methods
%{prefix}/share/gnome/apps/*
%{prefix}/share/pixmaps/*

