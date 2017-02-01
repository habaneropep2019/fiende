%define  ver     1.4.0
%define  RELEASE 1
%define  rel     %{?CUSTOM_RELEASE} %{!?CUSTOM_RELEASE:%RELEASE}
%define  prefix  /usr

Summary: Sounds for GNOME events.
Name: gnome-audio
Version: %ver
Release: %rel
Copyright: Public domain
Group: System Environment/Libraries
Source: ftp://ftp.gnome.org/pub/GNOME/sources/gnome-audio-%{ver}.tar.gz
BuildRoot:/var/tmp/gnome-audio-guide-%{PACKAGE_VERSION}-root
URL: http://www.gnome.org
Docdir: %{prefix}/doc
BuildArchitectures: noarch

%package extra
Summary: foo
Group: Data/Media/Sound

%description
If you use the GNOME desktop environment, you may want to
install this package of complementary sounds.

%description extra
This package contains extra sound files useful for customizing the
sounds that the GNOME desktop environment makes.

%prep
%setup


%install
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT
make prefix=$RPM_BUILD_ROOT/%{prefix} install 

%clean
rm -rf $RPM_BUILD_ROOT

%changelog

* Tue Jan 26 1999 Michael Fulbright <drmike@redhat.com>
- version 0.99.4

* Thu Dec 17 1998 Michael Fulbright <drmike@redhat.com>

- first pass at a spec file

%files
%defattr(-, root, root)
%doc README
%{prefix}/share/sounds/shutdown1.wav
%{prefix}/share/sounds/startup3.wav
%{prefix}/share/sounds/panel
%{prefix}/share/sounds/gtk-events

#symlinks
%{prefix}/share/sounds/login.wav
%{prefix}/share/sounds/logout.wav

%files extra
%{prefix}/share/sounds/card_shuffle.wav
%{prefix}/share/sounds/phone.wav
%{prefix}/share/sounds/startup1.wav
%{prefix}/share/sounds/startup2.wav
