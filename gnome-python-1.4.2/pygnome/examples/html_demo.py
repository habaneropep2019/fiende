import sys
if sys.version[:3] == '1.4':
	import ni
import os
import urllib, urlparse
from gtk import *
from gnome.ui import *
from gnome.xmhtml import *

opener = urllib.URLopener()

# url history
history = []
# links for 'forward'
forward = []

def about(button):
	GnomeAbout('GtkXmHTML Test', '0.0',
		   'May be distributed under the terms of the GPL2',
		   ['James Henstridge'],
		   'This is a useless application demonstrating the\n' +
		   'GtkXmHTML widget with python.').show()

def load_url(url):
	if os.path.exists(url):
		f = open(url)
	else:
		f = opener.open(url)
		headers = f.info()
		mime = headers.getheader('Content-type')
		if mime: html.set_mime_type(mime)
	source = f.read()
	html.freeze()
	html.source(source)
	html.thaw()
	history.append(url)

def anchor_track(html, info):
	if info.href:
		full_url = urlparse.urljoin(history[-1], info.href)
		status.set_text(full_url)
	else:
		status.set_text('')

def activate(html, info):
	if info.href:
		url = urlparse.urljoin(history[-1], info.href)
		load_url(url)
		entry.set_text(url)
		status.set('')

def key_function(entry, event):
	url = entry.get_text()
	if event.keyval == GDK.Return:
		load_url(url)
		del forward[:]
		entry.emit_stop_by_name('key_press_event')

def do_back(_b):
	forward.append(history[-1])
	del history[-1]
	url = history[-1]
	del history[-1]
	load_url(url)
def do_forward(_b):
	if len(forward) == 0: return
	load_url(forward[-1])
	del forward[-1]
def do_reload(_b):
	if len(history) == 0: return
	url = history[-1]
	del history[-1]
	load_url(url)

file_menu = [
	UIINFO_ITEM_STOCK('Quit', None, mainquit, STOCK_MENU_QUIT),
]
help_menu = [
	UIINFO_ITEM_STOCK('About...', None, about, STOCK_MENU_ABOUT),
]
menus = [
	UIINFO_SUBTREE('File', file_menu),
	UIINFO_SUBTREE('Help', help_menu)
]

toolbar = [
	UIINFO_ITEM_STOCK('Back', 'Previous page', do_back, STOCK_PIXMAP_BACK),
	UIINFO_ITEM_STOCK('Forward', 'Next page', do_forward,
			  STOCK_PIXMAP_FORWARD),
	UIINFO_ITEM_STOCK('Reload', 'Reload current page', do_reload,
			  STOCK_PIXMAP_REFRESH)
]

win = GnomeApp("html_demo", "Python Gtk-XmHTML Test")
win.set_wmclass("gtk_xmhtml_test", "GtkXmHTMLTest")
win.connect('delete_event', mainquit)

vbox = GtkVBox(spacing=3)
vbox.set_border_width(2)
vbox.show()
win.set_contents(vbox)

entry = GtkEntry()
entry.connect('key_press_event', key_function)
vbox.pack_start(entry, expand=FALSE)
entry.show()

html = GtkXmHTML()
html.set_dithering(FALSE)  # this forces creation of CC
html.set_usize(400, 400)
html.set_allow_body_colors(TRUE)
html.connect("anchor_track", anchor_track)
html.connect("activate", activate)

html.freeze()
html.source('''<html><head><title>Dummy Document</title></head>
<body bgcolor="#ffffff"><H1>Pick a URL</H1>
Use the entry box to pick a document to view.
Currently image loading does not work.
Also you will find the links
aren't working.  This is because I haven't found
enough hooks to get everything working in gtk-xmhtml
</body></html>''')
html.thaw()
vbox.pack_start(html)
html.show()

sep = GtkHSeparator()
vbox.pack_start(sep, expand=FALSE)
sep.show()

status = GtkLabel('')
status.set_justify(JUSTIFY_LEFT)
status.set_alignment(0.0, 0.5)
win.set_statusbar(status)
status.show()

# This is at the end, since pixmap creation changes the default visual
win.create_menus(menus)
win.create_toolbar(toolbar)

win.show()

mainloop()

