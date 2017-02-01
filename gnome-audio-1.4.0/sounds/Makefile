prefix=$(shell gnome-config --prefix)

LN_S=ln -sf
INSTALL_DATA=install -c -m 444 
# Which one is more portable
# MKINSTALLDIRS=install -d
MKINSTALLDIRS=mkdir -p

all:

install: install-files install-default-links

install-files:
	for I in *.wav */*.wav ; do \
		$(MKINSTALLDIRS) $(prefix)/share/sounds/`dirname $$I`; \
		$(INSTALL_DATA) $$I $(prefix)/share/sounds/$$I; \
	done

install-default-links:
	$(LN_S) startup3.wav $(prefix)/share/sounds/login.wav
	$(LN_S) shutdown1.wav $(prefix)/share/sounds/logout.wav
