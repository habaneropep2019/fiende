import _gnome, gnome

if not gnome.gnomelib_init_called:
	_gnome.gnomelib_init(gnome.app_id, gnome.app_version)
	gnome.gnomelib_init_called = 1

_gnome.gnome_triggers_init()

GTRIG_NONE      = 0
GTRIG_FUNCTION  = 1 # unsupported
GTRIG_COMMAND   = 2
GTRIG_MEDIAPLAY = 3

#def readfile(filename):
#	_gnome.gnome_triggers_readfile(filename)
def add_trigger(trig_type, trig_arg, trig_level, *supinfo):
	_gnome.gnome_triggers_add_trigger(trig_type, trig_arg, trig_level,
					  supinfo)
def vadd_trigger(trig_type, trig_arg, trig_level, supinfo):
	_gnome.gnome_triggers_add_trigger(trig_type, trig_arg, trig_level,
					  supinfo)
def do(msg, level, *supinfo):
	_gnome.gnome_triggers_do(msg, level, supinfo)
def vdo(msg, level, supinfo):
	_gnome.gnome_triggers_do(msg, level, supinfo)
