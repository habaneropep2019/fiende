import sys

if sys.modules.has_key('gtk'):
	raise ImportError, "gnome.applet must be imported before gtk"
del sys

import _applet

import gnome
init_result = _applet.applet_widget_init(gnome.app_id, gnome.app_version)
gnome.gnome_init_called = 1
gnome.gnomelib_init_called = 1
del gnome
import _gnomeui
_gnomeui._register_types()
del _gnomeui

import gtk; _gtk = gtk; del gtk

# add our mainloop and mainquit implementations to GTK
def mainloop():
	_applet.applet_widget_gtk_main()
def mainquit(*args):
	_applet.applet_widget_gtk_main_quit()
_gtk.mainloop = mainloop
_gtk.mainquit = mainquit
del mainloop, mainquit

def panel_quit():
	_applet.applet_widget_panel_quit()

_obj2inst = _gtk._obj2inst

class AppletWidget(_gtk.GtkPlug):
	def __init__(self, goad_id="", _obj=None):
		if _obj: self._o = _obj; return
		self._o = _applet.applet_widget_new(goad_id)
	def __getattr__(self, attr):
		attrs = {
			'privcfgpath': _applet.applet_widget_get_privcfgpath,
			'globcfgpath': _applet.applet_widget_get_privcfgpath
		}
		if attrs.has_key(attr):
			return attrs[attr](self._o)
		return _gtk.GtkPlug.__getattr__(self, attr)
	def set_tooltip(self, text):
		_applet.applet_widget_set_tooltip(self._o, text)
	def set_widget_tooltip(self, widget, text):
		_applet.applet_widget_set_widget_tooltip(self._o, widget._o,
							 text)
	def add(self, child):
		_applet.applet_widget_add(self._o, child._o)
	def add_full(self, child, bind_events):
		_applet.applet_widget_add_full(self._o, child._o)
	def bind_events(self, widget):
		_applet.applet_widget_bind_events(self._o, widget._o)
	def remove(self):
		_applet.applet_widget_remove(self._o)
	def register_callback(self, name, menutext, func, *args):
		_applet.applet_widget_register_callback(self._o, name,
							menutext, func, args)
	def register_stock_callback(self, name, stock_type, menutext, func,
				    *args):
		_applet.applet_widget_register_stock_callback(self._o, name,
							      stock_type,
							      menutext, func,
							      args)
	def unregister_callback(self, name):
		_applet.applet_widget_unregister_callback(self._o, name)
	def register_callback_dir(self, name, menutext):
		_applet.applet_widget_register_callback_dir(self._o, name,
							    menutext)
	def register_stock_callback_dir(self, name, stock_type, menutext):
		_applet.applet_widget_register_stock_callback_dir(self._o,
								  name,
								  stock_type,
								  menutext)
	def unregister_callback_dir(self, name):
		_applet.applet_widget_unregister_callback_dir(self._o, name)
	def callback_set_sensitive(self, name, sensitive):
		_applet.applet_widget_callback_set_sensitive(self._o, name,
							     sensitive)
	def get_applet_count(self):
		return _applet.applet_widget_get_applet_count()
	def sync_config(self):
		_applet.applet_widget_sync_config(self._o)
	def get_panel_orient(self):
		return _applet.applet_widget_get_panel_orient(self._o)
	def get_panel_pixel_size(self):
		return _applet.applet_widget_get_panel_pixel_size(self._o)
	def get_free_space(self):
		return _applet.applet_widget_get_free_space(self._o)
	def send_position(self, enable):
		_applet.applet_widget_send_position(self._o, enable)
	def send_draw(self, enable):
		_applet.applet_widget_send_draw(self._o, enable)
	def queue_resize(self):
		_applet.applet_widget_queue_resize(self._o)
	def abort_load(self):
		_applet.applet_widget_abort_load(self._o)
_gtk._name2cls['AppletWidget'] = AppletWidget

# Panel orientation types ...
ORIENT_UP    = 0
ORIENT_DOWN  = 1
ORIENT_LEFT  = 2
ORIENT_RIGHT = 3

# panel sizes ...
PIXEL_SIZE_ULTRA_TINY = 12,
PIXEL_SIZE_TINY = 24,
PIXEL_SIZE_SMALL = 36,
PIXEL_SIZE_STANDARD = 48,
PIXEL_SIZE_LARGE = 64,
PIXEL_SIZE_HUGE = 80,
PIXEL_SIZE_RIDICULOUS = 128

# panel back types ...
PANEL_BACK_NONE   = 0
PANEL_BACK_COLOR  = 1
PANEL_BACK_PIXMAP = 2
