%define nam guile-gtk
%define ver 0.19
%define rel 1
%define prefix /usr

Name: guile-gtk
Summary: Glue code that makes Gtk+ accessible from Guile
Version: %ver
Release: %rel
Copyright: GPL
Group: Development/Languages
Source: %{nam}-%{ver}.tar.gz
BuildRoot: /tmp/%{nam}-%{ver}-build
Packager: Jason Cao <jcao@users.sourceforge.net>
URL: http://www.ping.de/sites/zagadka/guile-gtk/


%description
  This is glue code to make GTK+ accessible from Guile.  It provides a
  convenient way for scheme/guile programmers to develop applications
  with a graphical user interface.

%prep
%setup

%build
CFLAGS="$RPM_OPT_FLAGS" ./configure --prefix=%prefix
make

%install
make prefix=$RPM_BUILD_ROOT%{prefix} \
ROOT=$RPM_BUILD_ROOT \
sitedir=$RPM_BUILD_ROOT%{prefix}/share/guile/site \
schemedir=$RPM_BUILD_ROOT%{prefix}/share/guile \
install 

%clean
rm -rf $RPM_BUILD_ROOT

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%defattr(-, root, root)

%doc AUTHORS COPYING ChangeLog NEWS README  


%{prefix}/bin/*
%{prefix}/include/*
%{prefix}/lib/*
%{prefix}/share/guile-gtk/*
%{prefix}/share/guile/gtk/*.scm
%{prefix}/share/guile/gtk-1.2/*.scm

%changelog
* Sat Jul 22 2000 Jason Cao <jcao@users.sourceforge.net>
- Change the spec for guile-gtk-0.19	

* Mon Oct  4 1999 Ariel Rios <jarios@usa.net>
- Changed the spec for guile-gtk main distribution.

* Tue Sep 28 1999 Greg J. Badros <gjb@cs.washington.edu>
- Link to guile-1.3.4, use new CVS that has Marius's proxy bug fix.

* Wed Sep 1 1999 Greg J. Badros <gjb@cs.washington.edu>
- Link to guile-1.3.3, so bump to release 2.

* Thu Aug 26 1999 Greg J. Badros <gjb@cs.washington.edu>
- Built from CVS especially for the Scwm-0.99.2 on RH6.

* Fri Jul 30 1999 Ariel Rios <jarios@usa.net>
- This rpm was built from cvs especially for the Linux PPP 6.0 distribution
