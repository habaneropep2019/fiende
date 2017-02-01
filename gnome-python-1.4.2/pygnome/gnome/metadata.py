import _gnome, gnome

if not gnome.gnomelib_init_called:
	_gnome.gnomelib_init(gnome.app_id, gnome.app_version)
	gnome.gnomelib_init_called = 1

# on write errors for the metadata, an IOError exception is thrown
# on metadata not found errors, an KeyError exception is thrown

def set(file, key, value):
	_gnome.gnome_metadata_set(file, key, value)
def remove(file, key):
	_gnome.gnome_metadata_remove(file, key)
def list(file):
	return _gnome.gnome_metadata_list(file)
def get(file, key):
	return _gnome.gnome_metadata_get(file, key)
def get_fast(file, key):
	return _gnome.gnome_metadata_get_fast(file, key)
def rename(file, to):
	_gnome.gnome_metadata_rename(file, to)
def copy(file, to):
	_gnome.gnome_metadata_copy(file, to)
def delete(file):
	_gnome.gnome_metadata_delete(file)
def regex_add(regex, key, value):
	_gnome.gnome_metadata_regex_add(regex, key, value)
def regex_remove(regex, key):
	_gnome.gnome_metadata_regex_remove(regex, key)
def type_add(type, key, value):
	_gnome.gnome_metadata_type_add(type, key, value)
def type_remove(type, key):
	_gnome.gnome_metadata_type_remove(type, key)
def lock():
	_gnome.gnome_metadata_lock()
def unlock():
	_gnome.gnome_metadata_unlock()
