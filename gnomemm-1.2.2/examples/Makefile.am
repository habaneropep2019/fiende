
EXAMPLE_DIR = animator canvas clock dialogs dock druid gnome-hello iconlist mdi menu messagebox stock uiinfo

EXTRA_DIST = \
examples.conf.in \
gnome-hello/application.cc \
gnome-hello/application.h \
gnome-hello/hello.cc \
gnome-hello/Makefile \
gnome-hello/gnome-hello-logo.png \
dialogs/Makefile \
dialogs/dialogs.cc \
stock/Makefile \
stock/stock.cc \
animator/Makefile \
animator/animator.cc \
animator/email.png \
animator/gnome-default.png \
animator/tux-jump.png \
canvas/Makefile \
canvas/canvas.cc \
canvas/example.png \
canvas/canvas_events.cc \
canvas/canvas_affine.cc \
clock/Makefile \
clock/main.cc \
clock/clock-window.cc \
clock/clock-window.h \
dock/Makefile   \
dock/dock-window.h   \
dock/dock-window.cc   \
dock/main.cc     \
druid/Makefile     \
druid/dialog_druid.h     \
druid/dialog_druid.cc     \
druid/druid-window.h     \
druid/druid-window.cc     \
druid/main.cc \
iconlist/Makefile \
iconlist/icon1.png \
iconlist/icon2.png \
iconlist/iconlist.cc \
iconlist/iconlist2.cc \
mdi/Makefile   \
mdi/button.xpm   \
mdi/mdi-window.cc   \
mdi/main.cc     \
mdi/mdi-window.h    \
mdi/mdi.cc \
menu/Makefile \
menu/application.h \
menu/application.cc \
menu/main.cc \
messagebox/Makefile \
messagebox/main.cc \
messagebox/messagebox-window.cc \
messagebox/messagebox-window.h \
uiinfo/Makefile \
uiinfo/uiinfo1.cc \
uiinfo/uiinfo2.cc \
uiinfo/radio.cc 

examples:
	for subdir in $(EXAMPLE_DIR); do \
	(cd $$subdir; make all) ; \
	done

examples-clean:
	for subdir in $(EXAMPLE_DIR); do \
	(cd $$subdir; make clean) ; \
	done
	
