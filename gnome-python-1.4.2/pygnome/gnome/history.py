import _gnome, gnome

if not gnome.gnomelib_init_called:
	_gnome.gnomelib_init(gnome.app_id, gnome.app_version)
	gnome.gnomelib_init_called = 1

def recently_used(filename, filetype, creator, desc):
	_gnome.gnome_history_recently_used(filename, filetype, creator, desc)
def get_recently_used():
	return _gnome.gnome_history_get_recently_used()
