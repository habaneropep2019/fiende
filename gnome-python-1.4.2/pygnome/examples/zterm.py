#!/usr/bin/env python
# to run this script type:
# zterm.py [prog [args ...]]
# if no program name given, spawm a python interpreter

import sys
if sys.version[:3] == '1.4':
	import ni
from gtk import *
from gnome.zvt import *
import os

def child_died_event(zvt):
	sys.exit(0)

def main():
	font_name = "-misc-fixed-medium-r-normal--20-200-75-75-c-100-*-*"

	win = GtkWindow()
	win.connect("delete_event", mainquit)
	win.set_title("ZTerm")
	win.set_policy(FALSE, TRUE, TRUE);

	hbox = GtkHBox()
	win.add(hbox)
	hbox.show()
	
	term = ZvtTerm(80, 25)
	term.set_scrollback(50)
	term.set_font_name(font_name);
	term.connect("child_died", child_died_event)
	hbox.pack_start(term)
	term.show()

	scroll = GtkVScrollbar(term.adjustment)
	hbox.pack_start(scroll, expand=FALSE)
	scroll.show()

	charwidth = term.charwidth
	charheight = term.charheight
	win.set_geometry_hints(geometry_widget=term,
			       min_width=2*charwidth, min_height=2*charheight,
			       base_width=charwidth,  base_height=charheight,
			       width_inc=charwidth,   height_inc=charheight)
	win.show()

	pid = term.forkpty()
	if pid == -1:
		print "Couldn't fork"
		sys.exit(1)
	if pid == 0:
		argv = sys.argv[1:]
		if not argv:
			os.execv('/usr/bin/env', ['/usr/bin/env', 'python'])
		else:
			os.execvp(argv[0], argv)
		print "Couldn't exec"
		os._exit(1)
	# this is executed by parent process only:
	mainloop()

if __name__ == '__main__': main()

