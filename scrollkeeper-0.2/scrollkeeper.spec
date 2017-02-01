Summary:    ScrollKeeper is a cataloging system for documentation on open systems
Name:       scrollkeeper
Version:    0.2
Release:    1
Source0:    http://download.sourceforge.net/scrollkeeper/%{name}-%{version}.tar.gz
Copyright:  LGPL
Group:      System Environment/Base
BuildRoot:  %{_tmppath}/%{name}-buildroot
URL:        http://scrollkeeper.sourceforge.net/
Requires:   libxml


%description 
ScrollKeeper is a cataloging system for documentation. It manages
documentation metadata (as specified by the Open Source Metadata
Framework (OMF)) and provides a simple API to allow help browsers to
find, sort, and search the document catalog. It will also be able to
communicate with catalog servers on the Net to search for documents
which are not on the local system.

%prep
%setup

%build
%configure
if [ "$SMP" != "" ]; then
  make -j$SMP "MAKE=make -j$SMP"
else
  make
fi


%install
rm -rf $RPM_BUILD_ROOT
%makeinstall
# mkdir -p ${RPM_BUILD_ROOT}%{_datadir}/scrollkeeper/Templates
pushd ${RPM_BUILD_ROOT}%{_datadir}/scrollkeeper/Templates
rm -rf en && ln -sf C en
rm -rf no* && ln -sf nb no && ln -sf nn no_NY
for lang in bg_BG bg_BG.cp1251 de_AT en_AU en_GB en_SE en_UK en_US es_DO \
         es_ES es_GT es_HN es_MX es_PA es_PE es_SV ja_JP.eucJP pt_PT \
         sr_YU sv_SE zh_CN zh_CN.GB2312 zh_TW zh_TW.Big5 ; do
    l=${lang%_*}
    rm -rf $lang
    ln -sf $l $lang
done 
popd

%clean
rm -rf $RPM_BUILD_ROOT

%pre
if test -L %{_datadir}/scrollkeeper/Templates/pt_BR; then
	rm %{_datadir}/scrollkeeper/Templates/pt_BR
fi

%files
%defattr(-,root,root)
%doc COPYING COPYING-DOCS AUTHORS README ChangeLog NEWS INSTALL 
%{_datadir}/omf/scrollkeeper
%{_bindir}/*
%{_libdir}/*
%{_mandir}/man8/*
%{_datadir}/scrollkeeper
%{_datadir}/locale/*/*


%post
if [ $1 = 1 ]; then
  # There was previously no SK installed.
  # ie. make a new %{_localstatedir}/lib/scrollkeeper.
  # Blow away any old database that might be there, in case
  # the user installed from tarball in the past.
  rm -rf %{_localstatedir}/lib/scrollkeeper
  mkdir %{_localstatedir}/lib/scrollkeeper
  scrollkeeper-update -p %{_localstatedir}/lib/scrollkeeper
fi
if [ $1 = 2 ]; then
  # There was previously a SK installed.
  #  ie. don't make a new %{_localstatedir}/lib/scrollkeeper.
  # However, version 0.0.4 of SK did not properly create this
  # directory if a previous SK was installed, so just to be sure...
  rm -rf %{_localstatedir}/lib/scrollkeeper
  mkdir %{_localstatedir}/lib/scrollkeeper
  scrollkeeper-update -p %{_localstatedir}/lib/scrollkeeper
fi

%postun
if [ $1 = 0 ]; then
  # SK is being removed, not upgraded.  
  #  ie. erase {localstatedir}/lib/scrollkeeper.
  rm -rf %{_localstatedir}/lib/scrollkeeper
fi
#if [ $1 = 1 ]; then
#  # SK is being upgraded.  Do not erase {localstatedir}/lib/scrollkeeper.
#fi
#
# Make sure no junk is left in the old template directory, 
# since upgrade from 0.1.1 to 0.1.2 was broken and can leave
# files there.
rm -rf %{_datadir}/scrollkeeper/templates

%changelog
* Mon Mar 5 2001 Dan Mueth <dan@eazel.com>
- Added %postun to remove $datadir/scrollkeeper/templates
  to compensate for breakage in upgrade from 0.1.1 to 0.1.2

* Sun Mar 4 2001 Dan Mueth <dan@eazel.com>
- Added cleaner symbolic link section suggested by Karl 
  Eichwalder <keichwa@users.sourceforge.net>
- Have it blow away the database dir on first install, just
  in case an old tarball version had been installed
- Fixing the Source0 line at the top

* Tue Feb 15 2001 Dan Mueth <dan@eazel.com>
- added line to include the translations .mo file

* Tue Feb 06 2001 Dan Mueth <dan@eazel.com>
- fixed up pre and post installation scripts

* Tue Feb 06 2001 Laszlo Kovacs <laszlo.kovacs@sun.com>
- added all the locale directories and links for the template
  content list files

* Wed Jan 17 2001 Gregory Leblanc <gleblanc@cu-portland.edu>
- converted to scrollkeeper.spec.in

* Sat Dec 16 2000 Laszlo Kovacs <laszlo.kovacs@sun.com>
- help files added

* Fri Dec 8 2000 Laszlo Kovacs <laszlo.kovacs@sun.com>
- various small fixes added

* Thu Dec 7 2000 Laszlo Kovacs <laszlo.kovacs@sun.com>
- fixing localstatedir problem
- adding postinstall and postuninstall scripts

* Tue Dec 5 2000 Gregory Leblanc <gleblanc@cu-portland.edu>
- adding COPYING, AUTHORS, etc
- fixed localstatedir for the OMF files

* Fri Nov 10 2000 Gregory Leblanc <gleblanc@cu-portland.edu>
- Initial spec file created.


