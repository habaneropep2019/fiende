%define pygtk_ver   0.6.9
%define pygnome_ver 1.4.2

Summary: The sources for the PyGTK and PyGNOME Python extension modules.
Name: gnome-python
Version: %{pygnome_ver}
Release: 1
Source: ftp://ftp.gnome.org/pub/GNOME/stable/sources/gnome-python/gnome-python-%{pygnome_ver}.tar.gz
Copyright: LGPL
Group: System Environment/Libraries
BuildRoot: /var/tmp/gnome-python-root
Packager: James Henstridge <james@daa.com.au>
Requires: gtk+ >= 1.2.8
Requires: gnome-libs >= 1.2.0
Requires: pygtk = %{pygtk_ver}

%description
The gnome-python package contains the source packages for the Python
bindings for GTK+ and GNOME (PyGTK and PyGNOME, respectively). 

PyGTK is an extension module for Python that provides access to the
GTK+ widget set. Just about anything (within reason) you can write in
C with GTK+, you can write in Python with PyGTK, but with all the
benefits of Python.

PyGNOME is an extension module for Python that provides access to the
base GNOME libraries, so you have access to more widgets, a simple
configuration interface, and metadata support.

%package -n pygtk
Version: %{pygtk_ver}
Summary: Python bindings for the GTK+ widget set.
Group: Development/Languages
Requires: glib, imlib, python >= 1.5.2
Requires: gtk+ >= 1.2.8

%description -n pygtk
PyGTK is an extension module for Python that gives you access to the
GTK+ widget set.  Just about anything you can write in C with GTK+ you
can write in Python with PyGTK (within reason), but with all the
benefits of Python. PyGTK provides an object-oriented interface at a
slightly higher level than the C interface. The PyGTK interface does
all of the type casting and reference counting that you would have to
do yourself using the C API.

Install pygtk if you need Python bindings for the GTK+ widget set.

%package -n pygtk-glarea
Version: %{pygtk_ver}
Summary: A wrapper for the GtkGLArea widget for use with PyGTK
Group: Development/Languages
Requires: pygtk = %{pygtk_ver}

%description -n pygtk-glarea
This module contains a wrapper for the GtkGLArea widget, which allows you
to display OpenGL output inside your pygtk program.  It needs a set of
Python OpenGL bindings such as PyOpenGL to actually do any OpenGL rendering.

%package -n pygtk-libglade
Version: %{pygtk_ver}
Summary: A wrapper for the libglade library for use with PyGTK
Group: Development/Languages
Requires: pygtk = %{pygtk_ver}

%description -n pygtk-libglade
This module contains a wrapper for the libglade library.  Libglade is a
library similar to the pyglade module, except that it is written in C (so
is faster) and is more complete.

%package -n pygtk-devel
Version: %{pygtk_ver}
Summary: files needed to build wrappers for GTK+ addon libraries
Group: Development/Languages
Requires: pygtk = %{pygtk_ver}

%description -n pygtk-devel
This package contains files required to build wrappers for GTK+ addon
libraries so that they interoperate with pygtk.

%package -n pygnome-libglade
Version: %{pygnome_ver}
Summary: GNOME support for the libglade python wrapper
Group: Development/Languages
Requires: pygnome = %{pygnome_ver}
Requires: pygtk-libglade = %{pygtk_ver}

%description -n pygnome-libglade
This module contains GNOME support to suppliment the libglade python
wrapper.  Libglade is a library similar to the pyglade module, except
that it is written in C (so is faster) and is more complete.

%package -n pygnome
Version: %{pygnome_ver}
Summary: Python bindings for the GNOME libraries.
Group: Development/Languages
Requires: pygtk = %{pygtk_ver}
Requires: gnome-libs

%description -n pygnome
PyGNOME is an extension module for python that gives you access to the
base GNOME libraries.  This means you have access to more widgets, simple
configuration interface, metadata support and many other features.

Install pygnome if you need Python bindings for the GNOME libraries.

%package -n pygnome-applet
Version: %{pygnome_ver}
Summary: Python bindings for GNOME Panel applets.
Group: Development/Languages
Requires: pygnome = %{pygnome_ver}

%description -n pygnome-applet
This module contains a wrapper that allows GNOME Panel applets to be
written in Python.

%package -n pygnome-capplet
Version: %{pygnome_ver}
Summary: Python bindings for GNOME Panel applets.
Group: Development/Languages
Requires: pygnome = %{pygnome_ver}

%description -n pygnome-capplet
This module contains a wrapper that allows GNOME Control Center
capplets to be in Python.

%package -n pygnome-devel
Version: %{pygnome_ver}
Summary: files that are useful for wrapping GNOME addon libraries
Group: Development/Languages
Requires: pygtk-devel = %{pygtk_ver}, pygnome = %{pygnome_ver}

%description -n pygnome-devel
This package contains files required to build wrappers for GTK+ addon
libraries so that they interoperate with pygtk.

%prep
%setup
CFLAGS="$RPM_OPT_FLAGS" ./configure --prefix=%{_prefix}

%build
make

%install
rm -rf $RPM_BUILD_ROOT
make DESTDIR=$RPM_BUILD_ROOT install

%files -n pygtk
%{_prefix}/lib/python?.?/site-packages/gtk.py*
%{_prefix}/lib/python?.?/site-packages/GtkExtra.py*
%{_prefix}/lib/python?.?/site-packages/GTK.py*
%{_prefix}/lib/python?.?/site-packages/GDK.py*
%{_prefix}/lib/python?.?/site-packages/GdkImlib.py*
%{_prefix}/lib/python?.?/site-packages/pyglade/*.py*

%{_prefix}/lib/python?.?/site-packages/_gtkmodule.so
%{_prefix}/lib/python?.?/site-packages/_gdkimlibmodule.so
%{_prefix}/lib/python?.?/site-packages/gdkpixbufmodule.so

%doc pygtk/AUTHORS pygtk/NEWS pygtk/README pygtk/MAPPING pygtk/ChangeLog
%doc pygtk/description.py pygtk/examples

%files -n pygtk-glarea
%{_prefix}/lib/python?.?/site-packages/gtkgl.py*
%{_prefix}/lib/python?.?/site-packages/_gtkglmodule.so

%files -n pygtk-libglade
%{_prefix}/lib/python?.?/site-packages/libglade.py*
%{_prefix}/lib/python?.?/site-packages/_libglademodule.so

%files -n pygtk-devel
%{_prefix}/bin/pygtk-codegen-1.2
%dir %{_prefix}/include/pygtk
%{_prefix}/include/pygtk/*.h
%dir %{_prefix}/share/pygtk
%dir %{_prefix}/share/pygtk/1.2
%dir %{_prefix}/share/pygtk/1.2/codegen
%dir %{_prefix}/share/pygtk/1.2/defs
%{_prefix}/share/pygtk/1.2/codegen/*
%{_prefix}/share/pygtk/1.2/defs/gtk.defs
%{_prefix}/share/pygtk/1.2/defs/gtkbase.defs
%{_prefix}/share/pygtk/1.2/defs/gtkcontainers.defs
%{_prefix}/share/pygtk/1.2/defs/gtkdata.defs
%{_prefix}/share/pygtk/1.2/defs/gtkdnd.defs
%{_prefix}/share/pygtk/1.2/defs/gtkedit.defs
%{_prefix}/share/pygtk/1.2/defs/gtkenums.defs
%{_prefix}/share/pygtk/1.2/defs/gtkgl.defs
%{_prefix}/share/pygtk/1.2/defs/gtklists.defs
%{_prefix}/share/pygtk/1.2/defs/gtkmenus.defs
%{_prefix}/share/pygtk/1.2/defs/gtkmisc.defs
%{_prefix}/share/pygtk/1.2/defs/gtkranges.defs
%{_prefix}/share/pygtk/1.2/defs/libglade.defs

%files -n pygnome
%dir %{_prefix}/lib/python?.?/site-packages/gnome
%{_prefix}/lib/python?.?/site-packages/gettext.py*
%{_prefix}/lib/python?.?/site-packages/gnome/__init__.py*
%{_prefix}/lib/python?.?/site-packages/gnome/affine.py*
%{_prefix}/lib/python?.?/site-packages/gnome/config.py*
%{_prefix}/lib/python?.?/site-packages/gnome/file.py*
%{_prefix}/lib/python?.?/site-packages/gnome/help.py*
%{_prefix}/lib/python?.?/site-packages/gnome/history.py*
%{_prefix}/lib/python?.?/site-packages/gnome/metadata.py*
%{_prefix}/lib/python?.?/site-packages/gnome/mime.py*
%{_prefix}/lib/python?.?/site-packages/gnome/score.py*
%{_prefix}/lib/python?.?/site-packages/gnome/triggers.py*
%{_prefix}/lib/python?.?/site-packages/gnome/ui.py*
%{_prefix}/lib/python?.?/site-packages/gnome/uiconsts.py*
%{_prefix}/lib/python?.?/site-packages/gnome/url.py*
%{_prefix}/lib/python?.?/site-packages/gnome/util.py*
%{_prefix}/lib/python?.?/site-packages/gnome/xmhtml.py*
%{_prefix}/lib/python?.?/site-packages/gnome/zvt.py*

%{_prefix}/lib/python?.?/site-packages/_gnomemodule.so
%{_prefix}/lib/python?.?/site-packages/_gnomeuimodule.so
%{_prefix}/lib/python?.?/site-packages/_zvtmodule.so
%{_prefix}/lib/python?.?/site-packages/_gtkxmhtmlmodule.so

%doc AUTHORS NEWS README ChangeLog
%doc pygnome/examples

%files -n pygnome-libglade
%{_prefix}/lib/python?.?/site-packages/_gladegnomemodule.so

# the following are only included if gnome-core and control-center were
# installed during the build
%files -n pygnome-applet
%{_prefix}/lib/python?.?/site-packages/_appletmodule.so
%{_prefix}/lib/python?.?/site-packages/gnome/applet.py*

%files -n pygnome-capplet
%{_prefix}/lib/python?.?/site-packages/_cappletmodule.so
%{_prefix}/lib/python?.?/site-packages/gnome/capplet.py*

%files -n pygnome-devel
%{_prefix}/share/pygtk/1.2/defs/applet.defs
%{_prefix}/share/pygtk/1.2/defs/capplet.defs
%{_prefix}/share/pygtk/1.2/defs/gnome-enums.defs
%{_prefix}/share/pygtk/1.2/defs/gnome.defs
%{_prefix}/share/pygtk/1.2/defs/gnomeui.defs
%{_prefix}/share/pygtk/1.2/defs/gtkhtml.defs
%{_prefix}/share/pygtk/1.2/defs/xmhtml.defs
%{_prefix}/share/pygtk/1.2/defs/zvt.defs

%changelog
* Fri Jan 21 2000 Matt Wilson <msw@redhat.com>
- added pygnome-libglade subpackage

* Wed Jan  5 2000 Matt Wilson <msw@redhat.com>
- split applet and capplet modules into their own package

