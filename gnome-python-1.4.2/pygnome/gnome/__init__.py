__all__ = ['tty',     # libgtktty
	   'xmhtml',  # libgtk-xmhtml
	   'ui',      # libgnomeui
	   'zvt',     # libzvt
	   'applet',  # panel applets
	   'capplet', # control center capplets
	   'config', 'file', 'help', 'history',
	   'metadata', 'mime', 'score', 'url', 'util'] # libgnome

# set app_id and app_version where other modules can find it easily ...

import sys, string
if len(sys.argv) > 0:
	app_id = sys.argv[0][string.rfind(sys.argv[0], '/')+1:]
	if app_id[-3:] == '.py':
		app_id = app_id[:-3]
	elif app_id[-4:] in ('.pyc', '.pyo'):
		app_id = app_id[:-4]
else:
	app_id = 'unknown'
del sys, string

app_version = "0.0"

gnomelib_init_called = 0
gnome_init_called = 0
