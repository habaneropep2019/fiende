import _gnome, gnome

if not gnome.gnomelib_init_called:
	_gnome.gnomelib_init(gnome.app_id, gnome.app_version)
	gnome.gnomelib_init_called = 1

def libdir_file(filename):
    return _gnome.gnome_libdir_file(filename)

def datadir_file(filename):
    return _gnome.gnome_datadir_file(filename)

def sound_file(filename):
    return _gnome.gnome_sound_file(filename)

def pixmap_file(filename):
    return _gnome.gnome_pixmap_file(filename)

def config_file(filename):
    return _gnome.gnome_config_file(filename)

def unconditional_libdir_file(filename):
    return _gnome.gnome_unconditional_libdir_file(filename)

def unconditional_datadir_file(filename):
    return _gnome.gnome_unconditional_datadir_file(filename)

def unconditional_sound_file(filename):
    return _gnome.gnome_unconditional_sound_file(filename)

def unconditional_pixmap_file(filename):
    return _gnome.gnome_unconditional_pixmap_file(filename)

def unconditional_config_file(filename):
    return _gnome.gnome_unconditional_config_file(filename)
