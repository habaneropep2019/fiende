import sys
if sys.version[:3] == '1.4':
	import ni

from gtk import *
from gnome.ui import *

def clicked(_button):
	print "Button clicked"

def xrandom_widgets(ted):
	for i in range(4):
		name = "Button-%d" % (i,)
		l = GtkButton(name)
		l.show()
		ted.add(l, name)
		l.connect('clicked', clicked)
	l = GtkLabel("This is a windowless widget")
	l.show()
	ted.add(l, "Label-0")
	l = GtkEntry()
	l.show()
	ted.add(l, "Entry-0")

def main():
	w = GtkWindow()
	w.connect('delete_event', mainquit)
	ted = GtkTed("DialogName")
	ted.show()
	w.add(ted)

	xrandom_widgets(ted)
	ted.prepare()
	w.show()
	mainloop()

if __name__ == '__main__': main()

