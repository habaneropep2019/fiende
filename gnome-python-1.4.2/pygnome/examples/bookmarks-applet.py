#!/usr/bin/env python
#
# This is an applet that displays your Netscape 4.x bookmarks on its context
# menu.  When you select a bookmark from the context menu, it will load it
# in netscape (or whatever other browser you have set to run when
# gnome_url_show is called).
#
import os
filename = os.path.join(os.environ['HOME'], '.netscape', 'bookmarks.html')
# Or for mozilla bookmarks ...
#filename = os.path.join(os.environ['HOME'], '.mozilla', 'bookmarks.html')

import gnome.applet
import gnome.url
from gtk import *
from gnome.ui import *
from sgmllib import SGMLParser

class BookmarkParser(SGMLParser):
	"""Parses a Netscape 4.x style bookmarks file"""

	def __init__(self, verbose=0):
		SGMLParser.__init__(self, verbose)
		self.title = None
		self.data = ""
		self.rootmenu = None
		self.menuheader = None
		self.is_menuheader = 0
		self.treestack = []

		self.item_title = None
		self.item_href = None

	def handle_data(self, data):
		self.data = self.data + data

	def start_title(self, attrs): self.data = ""
	def end_title(self): self.title = self.data

	def start_dl(self, attrs):
		newmenu = []
		if len(self.treestack) > 0:
			self.treestack[-1].append((self.item_title, newmenu))
			self.treestack.append(newmenu)
		else:
			self.treestack.append(newmenu)
			self.rootmenu = newmenu
		if self.is_menuheader: self.menuheader = newmenu
	def end_dl(self):
		del self.treestack[-1]
		
	def start_h3(self, attrs):
		self.data = ""
		self.is_menuheader = 0
		for tag, value in attrs:
			if tag == 'menuheader': self.is_menuheader = 1
	def end_h3(self): self.item_title = self.data

	def start_a(self, attrs):
		self.data = ""
		for tag, value in attrs:
			if tag == 'href':
				self.item_href = value
	def end_a(self):
		self.treestack[-1].append((self.data, self.item_href))

def parseBookmarks(fname):
	"""parses a bookmarks file, and returns the parse tree"""
	fp = open(fname)
	p = BookmarkParser()
	p.feed(fp.read())
	fp.close()
	p.close()
	return (p.title, p.menuheader or p.rootmenu)

def url_callback(applet, url):
	gnome.url.show(url)
def register_callbacks(app, list, prefix="bookmarks"):
	num = 0
	for title, href in list:
		key = prefix + "/menu" + str(num)
		if type(href) == type([]):
			app.register_stock_callback_dir(key,
							STOCK_MENU_BOOK_OPEN,
							title)
			register_callbacks(app, href, key)
		else:
			app.register_stock_callback(key, STOCK_MENU_JUMP_TO,
						    title, url_callback, href)
		num = num + 1
def fill_menu(app, title, menu):
	app.set_tooltip(title)
	app.register_stock_callback_dir("bookmarks", STOCK_MENU_BOOK_OPEN,
					"Bookmarks")
	register_callbacks(app, menu)

app = gnome.applet.AppletWidget("bookmarks")

frame = GtkFrame()
frame.set_shadow_type(SHADOW_ETCHED_IN)
label = GtkLabel("Netscape\nBookmarks")
label.set_padding(4,4)
frame.add(label)
label.show()
frame.show()

app.add(frame)
app.show()

title, menu = parseBookmarks(filename)
fill_menu(app, title, menu)

mainloop()

