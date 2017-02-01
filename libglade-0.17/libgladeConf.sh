prefix=/usr
exec_prefix=${prefix}

LIBGLADE_INCLUDEDIR="-I${prefix}/include/libglade-1.0 -I/usr/include/gnome-xml"
LIBGLADE_LIBDIR="-L${exec_prefix}/lib -rdynamic -L/usr/lib -L/usr/X11R6/lib"
LIBGLADE_LIBS="-lglade-gnome -lglade -L/usr/lib -lxml -lz -rdynamic -lgnomeui -lart_lgpl -lgdk_imlib -lSM -lICE -lgtk -lgdk -lgmodule -lXext -lX11 -lgnome -lgnomesupport -lesd -laudiofile -lm -ldb1 -lglib -ldl"
MODULE_VERSION="libglade-0.17"

