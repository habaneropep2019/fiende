import _gnome, gnome

if not gnome.gnomelib_init_called:
	_gnome.gnomelib_init(gnome.app_id, gnome.app_version)
	gnome.gnomelib_init_called = 1

def get_string(path):
	return _gnome.gnome_config_get_string(path)
def get_translated_string(path):
	return _gnome.gnome_config_get_translated_string(path)
def get_int(path):
	return _gnome.gnome_config_get_int(path)
def get_float(path):
	return _gnome.gnome_config_get_float(path)
def get_bool(path):
	return _gnome.gnome_config_get_bool(path)
def get_vector(path):
	return _gnome.gnome_config_get_vector(path)
def set_string(path, val):
	_gnome.gnome_config_set_string(path, val)
def set_translated_string(path, val):
	_gnome.gnome_config_set_translated_string(path, val)
def set_int(path, val):
	_gnome.gnome_config_set_int(path, val)
def set_float(path, val):
	_gnome.gnome_config_set_float(path, val)
def set_bool(path, val):
	_gnome.gnome_config_set_bool(path, val)
def set_vector(path, val):
	_gnome.gnome_config_set_vector(path, val)

def has_section(path):
	return _gnome.gnome_config_has_section(path)
def section_contents(path):
	return _gnome.gnome_config_section_contents(path)
def enum_sections(path):
	return _gnome.gnome_config_enum_sections(path)

def private_get_string(path):
	return _gnome.gnome_config_private_get_string(path)
def private_get_translated_string(path):
	return _gnome.gnome_config_private_get_translated_string(path)
def private_get_int(path):
	return _gnome.gnome_config_private_get_int(path)
def private_get_float(path):
	return _gnome.gnome_config_private_get_float(path)
def private_get_bool(path):
	return _gnome.gnome_config_private_get_bool(path)
def private_get_vector(path):
	return _gnome.gnome_config_private_get_vector(path)
def private_set_string(path, val):
	_gnome.gnome_config_private_set_string(path, val)
def private_set_translated_string(path, val):
	_gnome.gnome_config_private_set_translated_string(path, val)
def private_set_int(path, val):
	_gnome.gnome_config_private_set_int(path, val)
def private_set_float(path, val):
	_gnome.gnome_config_private_set_float(path,val)
def private_set_bool(path, val):
	_gnome.gnome_config_private_set_bool(path, val)
def private_set_vector(path, val):
	_gnome.gnome_config_private_set_vector(path, val)

def private_has_section(path):
	return _gnome.gnome_config_private_has_section(path)
def private_section_contents(path):
	return _gnome.gnome_config_private_section_contents(path)
def private_enum_sections(path):
	return _gnome.gnome_config_private_enum_sections(path)

def drop_all():
	_gnome.gnome_config_drop_all()
def sync():
	_gnome.gnome_config_sync()

def sync_file(path):
	_gnome.gnome_config_sync_file(path)
def private_sync_file(path):
	_gnome.gnome_config_private_sync_file(path)

def drop_file(path):
	_gnome.gnome_config_drop_file(path)
def clean_file(path):
	_gnome.gnome_config_clean_file(path)
def clean_section(path):
	_gnome.gnome_config_clean_section(path)
def clean_key(path):
	_gnome.gnome_config_clean_key(path)
def private_drop_file(path):
	_gnome.gnome_config_private_drop_file(path)
def private_clean_file(path):
	_gnome.gnome_config_private_clean_file(path)
def private_clean_section(path):
	_gnome.gnome_config_private_clean_section(path)
def private_clean_key(path):
	_gnome.gnome_config_private_clean_key(path)

def push_prefix(path):
	_gnome.gnome_config_push_prefix(path)
def pop_prefix():
	_gnome.gnome_config_pop_prefix()
