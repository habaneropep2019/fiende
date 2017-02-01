import _gnome, gnome

if not gnome.gnomelib_init_called:
	_gnome.gnomelib_init(gnome.app_id, gnome.app_version)
	gnome.gnomelib_init_called = 1

def type(filename):
	return _gnome.gnome_mime_type(filename)
def type_or_default(filename, default):
	return _gnome.gnome_mime_type_or_default(filename, default)
def type_of_file(existing_filename):
	return _gnome.gnome_mime_type_of_file(existing_filename)
def type_or_default_of_file(existing_filename, default):
	return _gnome.gnome_mime_type_or_default_of_file(existing_filename,
							 default)
def type_from_magic(filename):
	return _gnome.gnome_mime_type_from_magic(filename)

def get_value(mime_type, key):
	return _gnome.gnome_mime_get_value(mime_type, key)
def get_keys(mime_type):
	return _gnome.gnome_mime_get_keys(mime_type)
def program(mime_type):
	return _gnome.gnome_mime_program(mime_type)
def description(mime_type):
	return _gnome.gnome_mime_description(mime_type)
def nametemplate(mime_type):
	return _gnome.gnome_mime_nametemplate(mime_type)
def test(mime_type):
	return _gnome.gnome_mime_test(mime_type)
def composetyped(mime_type):
	return _gnome.gnome_mime_composetyped(mime_type)
def copiousoutput(mime_type, key):
	return _gnome.gnome_mime_copiousoutput(mime_type, key)
def needsterminal(mime_type, key):
	return _gnome.gnome_mime_needsterminal(mime_type, key)
