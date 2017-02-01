import _gnome, gnome

if not gnome.gnomelib_init_called:
	_gnome.gnomelib_init(gnome.app_id, gnome.app_version)
	gnome.gnomelib_init_called = 1

def convert_stream(fd, fromtype, totype):
	return _gnome.gnome_file_convert_stream(fd, fromtype, totype)
def convert(filename, fromtype, totype):
	return _gnome.gnome_file_convert(filename, fromtype, totype)
