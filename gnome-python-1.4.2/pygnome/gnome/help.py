import _gnome, gnome

if not gnome.gnomelib_init_called:
	_gnome.gnomelib_init(gnome.app_id, gnome.app_version)
	gnome.gnomelib_init_called = 1

def file_find_file(app, path):
	return _gnome.gnome_help_file_find_file(app, path)
def file_path(app, path):
	return _gnome.gnome_help_file_path(app, path)
def display(ref_name, ref_path):
	_gnome.gnome_help_display(ref_name, ref_path)
def goto(url):
	_gnome.gnome_help_goto(url)
