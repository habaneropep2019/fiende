Name:		oaf
Summary:	Object activation framework for GNOME
Version: 	0.6.8
Release: 	1
License: 	LGPL and GPL
Group:		System Environment/Libraries
Source: 	ftp://ftp.gnome.org/pub/GNOME/unstable/sources/%{name}/%{name}-%{version}.tar.gz
URL: 		http://www.gnome.org/
BuildRoot:	%{_tmpdir}/%{name}-%{version}-root

%description
OAF is an object activation framework for GNOME. It uses ORBit.

%package devel
Summary:	Libraries and include files for OAF
Group:		Development/Libraries
Requires:	%{name} = %{version}

%description devel
Development headers and libraries for OAF.

%prep
%setup -q

%build
%ifarch alpha
	MYARCH_FLAGS="--host=alpha-redhat-linux"
%endif

LC_ALL=""
LINGUAS=""
LANG=""
export LC_ALL LINGUAS LANG

CFLAGS="$RPM_OPT_FLAGS" ./configure $MYARCH_FLAGS \
	--enable-more-warnings --prefix=%{_prefix} \
	--sysconfdir=%{_sysconfdir} --bindir=%{_bindir} \
	--libdir=%{_libdir} --datadir=%{_datadir} \
	--includedir=%{_includedir}

make -k

%install
[ -n "$RPM_BUILD_ROOT" -a "$RPM_BUILD_ROOT" != / ] && rm -rf $RPM_BUILD_ROOT

make -k prefix=$RPM_BUILD_ROOT/%{_prefix} \
    sysconfdir=$RPM_BUILD_ROOT/%{_sysconfdir} \
    bindir=$RPM_BUILD_ROOT/%{_bindir} \
    libdir=$RPM_BUILD_ROOT/%{_libdir} \
    datadir=$RPM_BUILD_ROOT/%{_datadir} \
    includedir=$RPM_BUILD_ROOT/%{_includedir} install

%find_lang %name

%clean
[ -n "$RPM_BUILD_ROOT" -a "$RPM_BUILD_ROOT" != / ] && rm -rf $RPM_BUILD_ROOT

%post -p /sbin/ldconfig
  
%postun -p /sbin/ldconfig

%files -n %{name}.lang
%defattr(0555, root, root)

%doc AUTHORS COPYING ChangeLog NEWS README
%config %{_sysconfdir}/oaf
%{_bindir}/*
%{_libdir}/*.so.*

%defattr (0444, root, root)
%{_datadir}/idl/*.idl
%{_datadir}/oaf/*.oafinfo

%files devel

%defattr(0555, bin, bin)
%dir %{prefix}/include/liboaf
%{_libdir}/*.la
%{_libdir}/*.so
%{_libdir}/*.sh

%defattr(0444, bin, bin)
%{_includedir}/liboaf
%{_datadir}/aclocal/*.m4


%changelog
* Sun Aug 26 2001 Gregory Leblanc <gleblanc@linuxweasel.com>
- remove some unnecessary %defines
- used the %find_lang macro for i18n.  Makes many translators happy
- replaced %{prefix}/lib with %{_libdir} in the files section
- simplified files section
- replaced %{prefix}/bin with %{_bindir} in the files section
- added bindir, libdir, datadir, includedir to the configure and make install stages
- made %post script not bother to check if the correct path is already in /etc/ld.so.conf
- removed explicit stripping of binaries.  RPM does this automagically
- made configure and make install stages use RPMs built-in location macros
- made the setup step quiet
- added a description for the devel package
- made -devel not obsolete itself (not sure why it needed to in the first place)
- removed explicit definition of DocDir
- fixed BuildRoot
- fixed Source URL
- move ChangeLog to the end of the file (so that it's easier to read)
- moved some files into the -devel rpm
- changed default ownerships to be root, although we're still not quite decided on this one

* Tue Aug 29 2000 Maciej Stachowiak <mjs@eazel.com>
- corrected Copyright field and renamed it to License

* Sun May 21 2000 Ross Golder <rossigee@bigfoot.com>
- created spec file (based on bonobo.spec.in)
