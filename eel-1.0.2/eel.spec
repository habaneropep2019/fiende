# Note that this is NOT a relocatable package
%define name		eel
%define ver		1.0.2
%define RELEASE		0_cvs_0
%define rel		%{?CUSTOM_RELEASE} %{!?CUSTOM_RELEASE:%RELEASE}
%define prefix		/usr
%define sysconfdir	/etc

Name:		%name
Vendor:		GNOME
Distribution:	CVS
Summary:	Eazel Extensions Library
Version: 	%ver
Release: 	%rel
Copyright: 	GPL
Group:		System Environment/Libraries
Source: 	%{name}-%{ver}.tar.gz
URL: 		http://nautilus.eazel.com/
BuildRoot:	/var/tmp/%{name}-%{ver}-root
Docdir: 	%{prefix}/doc
Requires:	glib >= 1.2.9
Requires:	gtk+ >= 1.2.9
Requires:	libxml >= 1.8.10
Requires:	gnome-libs >= 1.2.11
Requires:	gnome-vfs >= 1.0
Requires:	gdk-pixbuf >= 0.10.0
Requires:	freetype >= 2.0.1
Requires:	libpng
Requires:	GConf >= 0.12
Requires:	oaf >= 0.6.5
Requires:	librsvg >= 1.0.0

BuildRequires:	glib-devel >= 1.2.9
BuildRequires:	gtk+-devel >= 1.2.9
BuildRequires:	libxml-devel >= 1.8.10
BuildRequires:	gnome-libs-devel >= 1.2.11
BuildRequires:	GConf-devel >= 0.12
BuildRequires:	oaf-devel >= 0.6.5
BuildRequires:	gnome-vfs-devel >= 1.0
BuildRequires:	gdk-pixbuf-devel >= 0.10.0
BuildRequires:	libpng-devel
BuildRequires:	librsvg-devel >= 1.0.0

%description
Eazel Extensions Library

%package devel
Summary:	Libraries and include files for developing with Eel.
Group:		Development/Libraries
Requires:	%name = %{PACKAGE_VERSION}

%description devel
This package provides the necessary development libraries and include
files to allow you to develop with Eel.

%changelog
* Wed Apr 04 2000 Ramiro Estrugo <ramiro@eazel.com>
- created this thing

%prep
%setup

%build
%ifarch alpha
	MYARCH_FLAGS="--host=alpha-redhat-linux"
%endif

LC_ALL=""
LINGUAS=""
LANG=""
export LC_ALL LINGUAS LANG

## Warning!  Make sure there are no spaces or tabs after the \ 
## continuation character, or else the rpm demons will eat you.
CFLAGS="$RPM_OPT_FLAGS" ./configure $MYARCH_FLAGS --prefix=%{prefix} \
	--sysconfdir=%{sysconfdir}

make -k
make check

%install
[ -n "$RPM_BUILD_ROOT" -a "$RPM_BUILD_ROOT" != / ] && rm -rf $RPM_BUILD_ROOT
make -k prefix=$RPM_BUILD_ROOT%{prefix} sysconfdir=$RPM_BUILD_ROOT%{sysconfdir} install
for FILE in "$RPM_BUILD_ROOT/bin/*"; do
	file "$FILE" | grep -q not\ stripped && strip $FILE
done

%clean
[ -n "$RPM_BUILD_ROOT" -a "$RPM_BUILD_ROOT" != / ] && rm -rf $RPM_BUILD_ROOT

%post
if ! grep %{prefix}/lib /etc/ld.so.conf > /dev/null ; then
	echo "%{prefix}/lib" >> /etc/ld.so.conf
fi
/sbin/ldconfig

%postun -p /sbin/ldconfig

%files

%defattr(0555, bin, bin)
%doc AUTHORS COPYING COPYING.LIB ChangeLog NEWS README
%{_libdir}/*.so*

%defattr (0444, bin, bin)
%{_datadir}/locale/*/LC_MESSAGES/*.mo
%{_datadir}/eel/fonts/urw/*.dir
%{_datadir}/eel/fonts/urw/*.pfb
%{_datadir}/eel/fonts/urw/*.afm
%{_datadir}/eel/fonts/urw/*.pfm

%files devel

%defattr(0555, bin, bin)
%{_libdir}/*.la
%{_libdir}/*.sh
%{_bindir}/eel-config

%defattr(0444, bin, bin)
%{_includedir}/eel-1/eel/*.h
