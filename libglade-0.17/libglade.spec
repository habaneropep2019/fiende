Summary: libglade library
Name: libglade
Version: 0.17
Release: 1
Copyright: LGPL
Group: System Environment/Libraries
Source: ftp://ftp.gnome.org/pub/GNOME/stable/sources/libglade/libglade-%{version}.tar.gz
BuildRoot: /var/tmp/%{name}-root
Packager: James Henstridge
URL: http://www.gnome.org

Requires: gtk+
Requires: libxml >= 1.3

%description
This library allows you to load user interfaces in your program, which are
stored externally.  This allows alteration of the interface without
recompilation of the program.

The interfaces can also be edited with GLADE.

%package devel
Summary: Libraries, includes, etc to develop libglade applications
Group: Development/Libraries
Requires: libglade gtk+-devel libxml-devel

%description devel
Libraries, include files, etc you can use to develop libglade applications.


%changelog

* Sun Nov  1 1998 James Henstridge <james@daa.com.au>

- Updated the dependencies of the devel package, so users must have gtk+-devel.

* Sun Oct 25 1998 James Henstridge <james@daa.com.au>

- Initial release 0.0.1

%prep
%setup

%build
%ifarch alpha
	MYARCH_FLAGS="--host=alpha-redhat-linux"
%endif

CFLAGS="$RPM_OPT_FLAGS" ./configure $MYARCH_FLAGS --prefix=%{_prefix}

if [ "$SMP" != "" ]; then
	make -k -j$SMP check "MAKE=make -k -j$SMP check"
else
	make -k check
fi

%install
[ -n "$RPM_BUILD_ROOT" -a "$RPM_BUILD_ROOT" != / ] && rm -rf $RPM_BUILD_ROOT

make -k prefix=$RPM_BUILD_ROOT%{_prefix} install


%clean
[ -n "$RPM_BUILD_ROOT" -a "$RPM_BUILD_ROOT" != / ] && rm -rf $RPM_BUILD_ROOT

%post
/sbin/ldconfig

%postun
/sbin/ldconfig

%files
%defattr(-, root, root)

%doc AUTHORS ChangeLog NEWS README COPYING
%{_prefix}/lib/lib*.so.*

%files devel
%defattr(-, root, root)

%{_prefix}/bin/*
%{_prefix}/lib/lib*.so
%{_prefix}/lib/*a
%{_prefix}/include/libglade-1.0/glade/*
%{_prefix}/share/aclocal/*
%{_prefix}/lib/libgladeConf.sh
%{_prefix}/lib/pkgconfig/*.pc

%doc test-libglade.c
%doc *.glade
%doc %{_prefix}/share/gnome/html/libglade/*
