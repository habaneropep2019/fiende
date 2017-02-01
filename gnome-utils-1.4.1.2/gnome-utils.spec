# Note that this is NOT a relocatable package
%define  ver      1.4.1.2
%define  RELEASE  1
%define  rel      %{?CUSTOM_RELEASE} %{!?CUSTOM_RELEASE:%RELEASE}

Summary: 	GNOME utility programs
Name: 		gnome-utils
Version: 	%ver
Release: 	%rel
Copyright: 	LGPL
Group: 		Applications/System
Source: 	ftp://ftp.gnome.org/pub/GNOME/stable/sources/gnome-utils-%{ver}.tar.gz
BuildRoot: 	/var/tmp/gnome-utils-%{PACKAGE_VERSION}-root
URL: 		http://www.gnome.org
Requires: 	gnome-libs >= 1.0.59
BuildRequires:  gnome-libs-devel >= 1.0.59, ORBit, gnome-core-devel
BuildRequires:  gtk+-devel >= 1.2.0, guile, e2fsprogs, e2fsprogs-devel
BuildRequires:  libgtop >= 1.0.0, libxml, libglade >= 0.11
Epoch:		1

%description
GNOME (GNU Network Object Model Environment) is a user-friendly set of
applications and desktop tools to be used in conjunction with a window
manager for the X Window System.  GNOME is similar in purpose and scope
to CDE and KDE, but GNOME is based completely on free software.

This package will install some GNOME utilities, such as the
calculator, search tool, and system information overview tool.

%prep
%setup -q

%build
%ifarch alpha
MYARCH_FLAGS=--host=alpha-redhat-linux
%endif

CFLAGS="$RPM_OPT_FLAGS" ./configure --prefix=%{_prefix} \
    --sysconfdir=%{_sysconfdir} --datadir=%{_datadir} \
    --bindir=%{_bindir} $MYARCH_FLAGS --enable-console-helper \
    --with-pam-prefix=/etc

if [ "$SMP" != "" ]; then
  make MAKE="make -j$SMP"
else
  make
fi

%install
[ -n "$RPM_BUILD_ROOT" -a "$RPM_BUILD_ROOT" != / ] && rm -rf $RPM_BUILD_ROOT

make prefix=$RPM_BUILD_ROOT%{_prefix} \
    sysconfdir=$RPM_BUILD_ROOT%{_sysconfdir} \
    bindir=$RPM_BUILD_ROOT%{_bindir} \
    sbindir=$RPM_BUILD_ROOT%{_sbindir} \
    datadir=$RPM_BUILD_ROOT%{_datadir} \
    PAM_PREFIX=$RPM_BUILD_ROOT/etc \
    install

%clean
[ -n "$RPM_BUILD_ROOT" -a "$RPM_BUILD_ROOT" != / ] && rm -rf $RPM_BUILD_ROOT

%files
%config /etc/pam.d/*
%config /etc/security/console.apps/*
%{_bindir}/*
%{_sbindir}/*
%{_sysconfdir}/CORBA/servers/*
%{_datadir}/applets/*/*
%{_datadir}/application-registry/*
%{_datadir}/gcolorsel
%{_datadir}/gnome-utils
%{_datadir}/gnome/apps/*/*
%{_datadir}/locale/*/*/*
%{_datadir}/gnome/help/*
%{_datadir}/gtt
%{_datadir}/idl/*
%{_datadir}/logview
%{_datadir}/mime-info/*
%{_datadir}/omf/*
%{_datadir}/pixmaps/*
%{_datadir}/stripchart

%doc AUTHORS COPYING ChangeLog NEWS README THANKS

%post
if which scrollkeeper-update>/dev/null 2>&1; then scrollkeeper-update; fi

%postun
if which scrollkeeper-update>/dev/null 2>&1; then scrollkeeper-update; fi


%changelog
* Sat Oct 20 2001 <jirka@5z.com>
- Fixes to the damn %files thing

* Thu Mar 29 2001 <dan@eazel.com>
- scrollkeeper stuff

* Wed Mar 14 2001 <jirka@5z.com>
- Fixes to the damn %files thing

* Mon Mar 12 2001 <gleblanc@peecee.linuxweasel.com> 
- macro fixes, removal of hard-coded paths.  removed automatic %files
  generation as we don't understand how that works.  some other
  beautifying work.

* Wed Sep 08 1999 Elliot Lee <sopwith@redhat.com>
- Updates from RHL package...

* Wed Sep 23 1998 Michael Fulbright <msf@redhat.com>
- Upgraded to 0.30

* Mon Apr  6 1998 Marc Ewing <marc@redhat.com>
- Integrate into gnome-utils CVS source tree
