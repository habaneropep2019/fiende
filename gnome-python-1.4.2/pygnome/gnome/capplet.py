import sys

if sys.modules.has_key('gtk'):
	raise ImportError, "gnome.capplet must be imported before gtk"
del sys

import _capplet

import gnome
init_result = _capplet.gnome_capplet_init(gnome.app_id, gnome.app_version)
gnome.gnome_init_called = 1
gnome.gnomelib_init_called = 1
del gnome
import _gnomeui
_gnomeui._register_types()
del _gnomeui

import gtk; _gtk = gtk; del gtk

# add our mainloop implementations to GTK
def mainloop():
	_capplet.capplet_gtk_main()
_gtk.mainloop = mainloop
del mainloop

class CappletWidget(_gtk.GtkPlug):
	def __init__(self, _obj=None):
		if _obj: self._o = _obj; return
		self._o = _capplet.capplet_widget_new()
	def state_changed(self, undoable=_gtk.TRUE):
		_capplet.capplet_widget_state_changed(self._o, undoable)
_gtk._name2cls['CappletWidget'] = CappletWidget
