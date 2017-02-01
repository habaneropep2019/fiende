#!/usr/bin/env python
import sys
import os
import urllib, urlparse
from gtk import *
from gnome.ui import *
from gtkhtml import *
import signal
signal.signal(signal.SIGINT, signal.SIG_DFL)

opener = urllib.URLopener()

# url history
history = []
# links for 'forward'
forward = []

def about(button):
	GnomeAbout('GtkHTML Test', '0.0',
		   'May be distributed under the terms of the GPL2',
		   ['Matt Wilson'],
		   ('This is a useless application demonstrating the\n'
                    'GtkHTML widget with python.  Based on html_demo.py by'
                    'James Henstridge')).show()

class HtmlWindow(GtkHTML):
	def __init__(self):
		GtkHTML.__init__(self)
	
	def load_url(self, html, url):
		if history: url = urllib.basejoin(history[-1], url)
		print "load_url", url
		history.append(url)
		handle = html.begin()
		self.request_url(html, url, handle)
		html.end(handle, HTML_STREAM_OK)

	def request_url(self, html, url, handle):
		url = urllib.basejoin(history[-1], url)
		print "Requesting url", url
		if url == 'blank':
			print "here"
			source = "<html><body>foobar</body></html>"
		elif os.path.exists(url):
			f = open(url)
			source = f.read()
		else:
			try:
				f = opener.open(url)
			except IOError, (num, error):
				GnomeErrorDialog(error).run()
				# html.end(handle, HTML_STREAM_ERROR)
				return
			headers = f.info()
			mime = headers.getheader('Content-type')
			source = f.read()
		html.write(handle, source)
		

	def anchor_track(self, html, info):
		if info.href:
			full_url = urlparse.urljoin(history[-1], info.href)
			status.set_text(full_url)
		else:
			status.set_text('')

	def activate(self, html, info):
		if info.href:
			url = urlparse.urljoin(history[-1], info.href)
			self.load_url(html, url)
			entry.set_text(url)
			status.set('')

	def entry_activate(self, entry, html):
		url = entry.get_text()
		self.load_url(html, url)
		del forward[:]

	def do_back(self, _b):
		forward.append(history[-1])
		del history[-1]
		url = history[-1]
		del history[-1]
		self.load_url(html, url)

	def do_forward(self, _b):
		if len(forward) == 0: return
		self.load_url(html, forward[-1])
		del forward[-1]

	def do_reload(self, _b):
		if len(history) == 0: return
		url = history[-1]
		del history[-1]
		self.load_url(html, url)

html = HtmlWindow()

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
	UIINFO_ITEM_STOCK('Back', 'Previous page', html.do_back, STOCK_PIXMAP_BACK),
	UIINFO_ITEM_STOCK('Forward', 'Next page', html.do_forward,
			  STOCK_PIXMAP_FORWARD),
	UIINFO_ITEM_STOCK('Reload', 'Reload current page', html.do_reload,
			  STOCK_PIXMAP_REFRESH)
]

win = GnomeApp("html_demo", "Python GtkHTML Test")
win.set_wmclass("gtk_html_test", "GtkHTMLTest")
win.connect('delete_event', mainquit)

vbox = GtkVBox(spacing=3)
vbox.set_border_width(2)
vbox.show()
win.set_contents(vbox)

entry = GtkEntry()
html.connect('url_requested', html.request_url)
html.connect('link_clicked', html.load_url)

entry.connect('activate', html.entry_activate, html)
vbox.pack_start(entry, expand=FALSE)
entry.show()

html.set_usize(400, 400)

sw = GtkScrolledWindow()
sw.set_policy(POLICY_AUTOMATIC, POLICY_AUTOMATIC)
sw.add(html)
vbox.pack_start(sw)

sep = GtkHSeparator()
vbox.pack_start(sep, expand=FALSE)

status = GtkLabel('')
status.set_justify(JUSTIFY_LEFT)
status.set_alignment(0.0, 0.5)
win.set_statusbar(status)

# This is at the end, since pixmap creation changes the default visual
win.create_menus(menus)
win.create_toolbar(toolbar)

html.load_empty()

win.show_all()

mainloop()
