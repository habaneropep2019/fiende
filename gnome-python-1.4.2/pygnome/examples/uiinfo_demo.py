import sys
if sys.version[:3] == "1.4":
	import ni

from gtk import *
from gnome.ui import *

def about(item):
	win = GnomeAbout('Test', '0.0', 'GPL', ['James Henstridge'],
			 'a simple program to test pygimp')
	win.show()
def cb(item):
	print item

file_menu = [
	(APP_UI_ITEM, 'New...', None, cb, None,
	 APP_PIXMAP_STOCK, STOCK_MENU_NEW, 0, 0),
	(APP_UI_ITEM, 'Open', None, cb, None,
	 APP_PIXMAP_STOCK, STOCK_MENU_OPEN, 0, 0),
	(APP_UI_ITEM, 'Save', None, cb, None,
	 APP_PIXMAP_STOCK, STOCK_MENU_SAVE, 0, 0),
	(APP_UI_ITEM, 'Save As', None, cb, None,
	 APP_PIXMAP_STOCK, STOCK_MENU_SAVE_AS, 0, 0),
	UIINFO_SEPARATOR,
	(APP_UI_ITEM, 'Exit', None, mainquit, None,
	 APP_PIXMAP_STOCK, STOCK_MENU_EXIT, 0, 0)
]
edit_menu = [
	(APP_UI_ITEM, 'Cut', None, cb, None,
	 APP_PIXMAP_STOCK, STOCK_MENU_CUT, 0, 0),
	(APP_UI_ITEM, 'Copy', None, cb, None,
	 APP_PIXMAP_STOCK, STOCK_MENU_COPY, 0, 0),
	(APP_UI_ITEM, 'Paste', None, cb, None,
	 APP_PIXMAP_STOCK, STOCK_MENU_PASTE, 0, 0),
]
test_menu = [
	UIINFO_ITEM('an item before radio list', None, None, None),
	UIINFO_SEPARATOR,
	UIINFO_RADIOLIST([
		UIINFO_ITEM('one', None, None, None),
		UIINFO_ITEM('two', None, None, None),
		UIINFO_ITEM('three', None, None, None)
	]),
	UIINFO_ITEM('An item after radio list', None, None, None)
]

help_menu = [
	UIINFO_HELP('sample-help'),
	UIINFO_SEPARATOR,
	(APP_UI_ITEM, 'About', None, about, None,
	 APP_PIXMAP_STOCK, STOCK_MENU_ABOUT, 0, 0),
]
menu = [
	UIINFO_SUBTREE('File', file_menu),
	UIINFO_SUBTREE('Edit', edit_menu),
	UIINFO_SUBTREE('Test', test_menu),
	UIINFO_SUBTREE('Help', help_menu)
]

app = GnomeApp('uiinfo_demo', 'UIInfo Demo')
app.connect('delete_event', mainquit)
app.create_menus(menu)
app.show()

mainloop()

