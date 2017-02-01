gnomedir = $(pythondir)/gnome

if WITH_APPLETS
applet_py = applet.py
endif
if WITH_CAPPLET
capplet_py = capplet.py
endif

gnome_PYTHON = __init__.py config.py file.py help.py history.py metadata.py \
	mime.py score.py triggers.py url.py ui.py uiconsts.py xmhtml.py \
	zvt.py affine.py util.py $(applet_py) $(capplet_py)

# so they don't get left behind
EXTRA_DIST = applet.py capplet.py