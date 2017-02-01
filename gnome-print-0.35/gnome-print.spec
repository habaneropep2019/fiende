# *** NOTE ****
# This gnome-print.spec file is not beeing maintaned by the gnome-print
# team. Patches are welcomed, but the only "official" gnome-print release
# is the tarball.

%define localstatedir /var/lib

Summary:        Gnome Print - Printing libraries for GNOME.
Name: 		gnome-print
Version: 	0.35
Release: 	0.1
Copyright: 	LGPL
Group: 		System Environment/Base
Source:         ftp://ftp.gnome.org/pub/GNOME/stable/sources/%{name}/%{name}-%{version}.tar.gz
BuildRoot: 	%{_tmpdir}/gnome-print-%{version}-root
PreReq:         ghostscript, urw-fonts, perl
PreReq:         ghostscript-fonts >= 4.03
Requires:       gtk+ >= 1.2.8
Requires:       gnome-libs >= 1.0
Requires:       libxml >= 1.8.5
BuildRequires:  gnome-libs-devel
BuildRequires:  gdk-pixbuf-devel >= 0.7.0
BuildRequires:  libxml-devel >= 1.8.5

%description
You should install the gnome-print package if you intend on using any of
the GNOME applications that can print. If you would like to develop GNOME
applications that can print you will also need to install the gnome-print
devel package.

%package devel
Summary:    Libraries and include files for developing GNOME applications.
Group: 	    Development/Libraries
Requires:   %{name} = %{version}

%description devel
You should install the gnome-print-devel package if you would like to 
develop GNOME applications that will use the GNOME printing facilities.
You don't need to install the gnome-print-devel package if you just want 
to use the GNOME desktop enviornment.

%prep
%setup -q

%build
# Needed for snapshot releases.
%ifarch alpha
  MYARCH_FLAGS="--host=alpha-redhat-linux"
%endif

if [ ! -f configure ]; then
    CFLAGS="$RPM_OPT_FLAGS" ./autogen.sh $MYARCH_FLAGS \
	--prefix=%{_prefix} --localstatedir=%{localstatedir} \
	--bindir=%{_bindir} --datadir=%{_datadir} --libdir=%{_libdir} \
	--includedir=%{_includedir} 
fi

CFLAGS="$RPM_OPT_FLAGS" ./configure $MYARCH_FLAGS --prefix=%{_prefix} \
    --localstatedir=%{localstatedir} --bindir=%{_bindir} \
    --datadir=%{_datadir} --libdir=%{_libdir} \
    --includedir=%{_includedir}


if [ "$SMP" != "" ]; then
    (make "MAKE=make -k -j $SMP"; exit 0)
    make
else
    make
fi

%install
rm -rf $RPM_BUILD_ROOT

make prefix=$RPM_BUILD_ROOT/%{_prefix} \
    localstatedir=$RPM_BUILD_ROOT/%{localstatedir} \
    bindir=$RPM_BUILD_ROOT/%{_bindir} \
    datadir=$RPM_BUILD_ROOT/%{_datadir} \
    libdir=$RPM_BUILD_ROOT/%{_libdir} \
    includedir=$RPM_BUILD_ROOT/%{_includedir} install 

# This is ugly
#

install -m 644 run-gnome-font-install $RPM_BUILD_ROOT%{_datadir}/fonts
cd fonts
install -m 644 *.font $RPM_BUILD_ROOT%{_datadir}/fonts

%clean
rm -rf $RPM_BUILD_ROOT

%post
/sbin/ldconfig

perl %{_datadir}/fonts/run-gnome-font-install \
	%{_bindir}/gnome-font-install %{_datadir} %{_datadir}

%postun 
/sbin/ldconfig

%files
%defattr(-, root, root)

%doc AUTHORS COPYING ChangeLog NEWS README
%{_bindir}/*
%{_libdir}/lib*.so.*
%{_datadir}/fonts/afms/adobe/*
%{_datadir}/fonts/fontmap2
%{_datadir}/fonts/*.font
%{_datadir}/fonts/run-gnome-font-install
%{_datadir}/locale/*/*/*
%config %{_datadir}/gnome-print/%{version}/profiles/PostscriptOptimized.profile
%config %{_datadir}/gnome-print/%{version}/profiles/pdf.profile
%config %{_datadir}/gnome-print/%{version}/profiles/fax-g3.profile
 
%files devel
%defattr(-, root, root)

%{_libdir}/lib*.so
%{_libdir}/*.a
%{_libdir}/*.la
%{_libdir}/*.sh
%{_includedir}/libgnomeprint


%changelog
* Fri Jun 22 2001 Gregory Leblanc <gleblanc@cu-portland.edu>
- reformatted the header with nicer indenting
- fixed all paths to use macros if possible
- added the rest of the option relocation options to configure and make install
- added localstatdir as a define
- added a PreReq: on perl
- used %{_tmpdir} in the BuildRoot line
- removed unnecessary defines
- used %name and %version in the Source line

* Mon Dec 11 2000 Chema Celorio <chema@celorio.com>
- Added note about this file not beeing maintaned and updated
  the old description.

* Sun Aug 01 1999 Gregory McLean <gregm@comstar.net>
- Undo my draconian uninstall stuff.

* Tue Jul 20 1999 Gregory McLean <gregm@comstar.net>
- Stab at cleaning up properly when we uninstall.

* Fri Jul 16 1999 Herbert Valerio Riedel <hvr@hvrlab.dhs.org>
- fixed typo in spec

* Wed Jul 14 1999 Gregory McLean <gregm@comstar.net>
- Added fonts to the spec.

* Mon Jul 05 1999 Gregory McLean <gregm@comstar.net>
- Fleshed out the descriptions.
