import _gnome, gnome

if not gnome.gnomelib_init_called:
	_gnome.gnomelib_init(gnome.app_id, gnome.app_version)
	gnome.gnomelib_init_called = 1

_gnome.gnome_score_init(gnome.app_id)

def log(score, level, high_to_low):
	_gnome.gnome_score_log(score, level, high_to_low)
def get_notable(gamename=None, level=None):
	return _gnome.gnome_score_get_notable(gamename, level)
