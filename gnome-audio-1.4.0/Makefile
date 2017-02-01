VERSION=1.4.0
DD=gnome-audio-$(VERSION)

all: gnome-audio.spec

gnome-audio.spec: gnome-audio.spec.in
	sed "s/GNOME_AUDIO_VERSION/$(VERSION)/" gnome-audio.spec.in > gnome-audio.spec

install:
	$(MAKE) -C sounds install

dist: gnome-audio.spec
	make prefix=$(shell pwd)/$(DD) install
	mv $(DD)/share/sounds $(DD)/sounds
	rmdir $(DD)/share
	install -m644 Makefile README gnome-audio.spec.in gnome-audio.spec $(DD)
	install -m644 sounds/Makefile $(DD)/sounds
	tar -czvf $(DD).tar.gz $(DD)
	rm -rf $(DD)
