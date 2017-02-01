Summary: GNOME PostScript viewer
Name: 		ggv
Version:	1.0.2
Release:         0.1
License:         GPL
Group: 	         Applications/Publishing
Source:		ftp://ftp.gnome.org/pub/GNOME/sources/%{name}/%{name}-%{version}.tar.gz
BuildRoot: 	/var/tmp/%{name}-%{version}-root
URL: 		http://www.gnome.org/
BuildRequires:   gnome-libs-devel, gnome-print-devel, oaf-devel
BuildRequires:   gtk+-devel >= 1.2.0
BuildRequires:   bonobo-devel >= 0.30

%description
ggv allows you to view PostScript documents, and print ranges
of pages.

%prep
%setup -q

%build

# libtool workaround for alphalinux
%ifarch alpha
  ARCH_FLAGS="--host=alpha-redhat-linux"
%endif

# Needed for snapshot releases.
if [ ! -f configure ]; then
    CFLAGS="$RPM_OPT_FLAGS" ./autogen.sh $ARCH_FLAGS --prefix=%{_prefix} \
	--sysconfdir=%{_sysconfdir} --datadir=%{_datadir} \
	--bindir=%{_bindir}
else
    CFLAGS="$RPM_OPT_FLAGS" ./configure $ARCH_FLAGS --prefix=%{_prefix} \
        --sysconfdir=%{_sysconfdir} --datadir=%{_datadir} \
        --bindir=%{_bindir}
fi

if [ "$SMP" != "" ]; then
  (make "MAKE=make -k -j $SMP"; exit 0)
  make
else
  make
fi

%install
rm -rf $RPM_BUILD_ROOT

make prefix=$RPM_BUILD_ROOT/%{_prefix} \
    sysconfdir=$RPM_BUILD_ROOT/%{_sysconfdir} \
    datadir=$RPM_BUILD_ROOT/%{_datadir} \
    bindir=$RPM_BUILD_ROOT/%{_bindir} install

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-, root, root)

%doc AUTHORS COPYING ChangeLog NEWS README TODO
%{_bindir}/*
%{_datadir}/gnome/apps/Graphics/*
%{_datadir}/gnome/help/ggv
%{_datadir}/locale/*/*/*
%{_datadir}/oaf/*
%{_datadir}/omf/ggv
%{_datadir}/pixmaps/*


%changelog
* Fri May 25 2001 Gregory Leblanc <gleblanc@localhost.localdomain>
- removed unnecessary %defines
- fixed %files section
- made %setup quiet
- added bindir to configure and install
- replaced Copyright with License

* Tue Feb 20 2001 Gregory Leblanc <gleblanc@cu-portland.edu>
- removing hard-coded paths, and cleaning macros

* Fri Aug 27 1999 Karl Eichwalder <ke@suse.de>
- Added more %doc files
- Fixed the spelling of PostScript and the Source entry

* Sat Aug 21 1999 Herbert Valerio Riedel <hvr@hvrlab.dhs.org>
- Actualized spec file

* Thu Aug 13 1998 Marc Ewing <marc@redhat.com>
- Initial spec file copied from gnome-graphics
