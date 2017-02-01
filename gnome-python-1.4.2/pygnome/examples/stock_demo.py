import sys
if sys.version[:3] == '1.4':
	import ni

from gtk import *
from gnome.ui import *

def message_dlg_clicked(widget, button):
	if button == 0:
		mainquit()

def message_dlg(widget, event=None):
	box = GnomeMessageBox("Reallly quit?", "question",
			      STOCK_BUTTON_YES, STOCK_BUTTON_NO)
	box.connect("clicked", message_dlg_clicked)
	box.set_modal(TRUE)
	box.show()
	return TRUE

def prop_apply(box, n):
	if n != -1:
		mbox = GnomeMessageBox("Applied changes on page #%d" % (n,),
				       "info", STOCK_BUTTON_OK)
		mbox.show()

def prop_dlg(widget):
	box = GnomePropertyBox()
	w = GtkButton("Click me (Page #1)")
	w.connect("clicked", box.changed)
	w.show()
	label = GtkLabel("Page #1")
	label.show()
	box.append_page(w, label)
	w = GtkButton("Click me (Page #2)")
	w.connect("clicked", box.changed)
	w.show()
	label = GtkLabel("Page #2")
	label.show()
	box.append_page(w, label)
	box.connect('apply', prop_apply)
	box.show()

def create_menu():
	file_menu = [
		UIINFO_ITEM_STOCK('New...', None, None, STOCK_MENU_NEW),
		UIINFO_ITEM_STOCK('Open...', None, None, STOCK_MENU_OPEN),
		UIINFO_ITEM_STOCK('Save', None, None, STOCK_MENU_SAVE),
		UIINFO_ITEM_STOCK('Save as...', None, None, STOCK_MENU_SAVE_AS),
		UIINFO_SEPARATOR,
		UIINFO_ITEM_STOCK('Print...', None, None, STOCK_MENU_PRINT),
		UIINFO_ITEM_STOCK('Setup Page...', None, None, STOCK_MENU_BLANK),
		UIINFO_SEPARATOR,
		UIINFO_ITEM_STOCK('Quit', None, message_dlg, STOCK_MENU_QUIT)
	]
	edit_menu = [
		UIINFO_ITEM_STOCK('Undo', None, None, STOCK_MENU_UNDO),
		UIINFO_ITEM_STOCK('Redo', None, None, STOCK_MENU_BLANK),
		UIINFO_SEPARATOR,
		UIINFO_ITEM_STOCK('Delete', None, None, STOCK_MENU_BLANK),
		UIINFO_ITEM_STOCK('Cut', None, None, STOCK_MENU_CUT),
		UIINFO_ITEM_STOCK('Copy', None, None, STOCK_MENU_COPY),
		UIINFO_ITEM_STOCK('Paste', None, None, STOCK_MENU_PASTE),
		UIINFO_SEPARATOR,
		UIINFO_ITEM_STOCK('Properties...', None, prop_dlg, STOCK_MENU_PROP),
		UIINFO_ITEM_STOCK('Preferences...', None, None, STOCK_MENU_PREF),
		UIINFO_SEPARATOR,
		UIINFO_ITEM_STOCK('Scores...', None, None, STOCK_MENU_SCORES)
	]
	help_menu = [
		UIINFO_ITEM_STOCK('About', None, None, STOCK_MENU_ABOUT)
	]
	menu_info = [
		UIINFO_SUBTREE('File', file_menu),
		UIINFO_SUBTREE('Edit', edit_menu),
		UIINFO_SUBTREE('Help', help_menu)
	]
	return menu_info

def create_toolbar():
	toolbar_info = [
		UIINFO_ITEM_STOCK('New', None, None, STOCK_PIXMAP_NEW),
		UIINFO_SEPARATOR,
		UIINFO_ITEM_STOCK('Open', None, None, STOCK_PIXMAP_OPEN),
		UIINFO_ITEM_STOCK('Save', None, None, STOCK_PIXMAP_SAVE),
		UIINFO_ITEM_STOCK('Save as', None, None, STOCK_PIXMAP_SAVE_AS),
		UIINFO_SEPARATOR,
		UIINFO_ITEM_STOCK('Cut', None, None, STOCK_PIXMAP_CUT),
		UIINFO_ITEM_STOCK('Copy', None, None, STOCK_PIXMAP_COPY),
		UIINFO_ITEM_STOCK('Paste', None, None, STOCK_PIXMAP_PASTE),
		UIINFO_SEPARATOR,
		UIINFO_ITEM_STOCK('Props', None, prop_dlg, STOCK_PIXMAP_PROPERTIES)
	]
	return toolbar_info

def toggle_button(button):
	wid = button.children()[0]
	text = button.get_data('user_data')
	if text == STOCK_PIXMAP_TIMER_STOP:
		wid.set_icon(STOCK_PIXMAP_TIMER)
		button.set_data('user_data', STOCK_PIXMAP_TIMER)
	else:
		wid.set_icon(STOCK_PIXMAP_TIMER_STOP)
		button.set_data('user_data', STOCK_PIXMAP_TIMER_STOP)

def fill_table(win, table):
	w = GnomeStock(STOCK_PIXMAP_HELP)
	w.show()
	table.attach(w, 0,1, 0,1)
	w = GtkLabel("Help")
	w.show()
	table.attach(w, 0,1, 2,3)

	w = GnomeStock(STOCK_PIXMAP_SEARCH)
	w.show()
	table.attach(w, 1,2, 0,1)
	w = GnomeStock(STOCK_MENU_SEARCH)
	w.show()
	table.attach(w, 1,2, 1,2)
	w = GtkLabel('Search')
	w.show()
	table.attach(w, 1,2, 2,3)

	w = GnomeStock(STOCK_PIXMAP_PRINT)
	w.show()
	table.attach(w, 2,3, 0,1)
	w = GtkLabel('Print')
	w.show()
	table.attach(w, 2,3, 2,3)

	w = GnomeStock(STOCK_PIXMAP_BACK)
	w.show()
	table.attach(w, 3,4, 0,1)
	w = GnomeStock(STOCK_MENU_BACK)
	w.show()
	table.attach(w, 3,4, 1,2)
	w = GtkLabel('Backward')
	w.show()
	table.attach(w, 3,4, 2,3)

	w = GnomeStock(STOCK_PIXMAP_FORWARD)
	w.show()
	table.attach(w, 4,5, 0,1)
	w = GnomeStock(STOCK_MENU_FORWARD)
	w.show()
	table.attach(w, 4,5, 1,2)
	w = GtkLabel('Forward')
	w.show()
	table.attach(w, 4,5, 2,3)

	w = GnomeStock(STOCK_PIXMAP_PREFERENCES)
	w.show()
	table.attach(w, 5,6, 0,1)
	w = GtkLabel('Preferences')
	w.show()
	table.attach(w, 5,6, 2,3)

	button = GtkButton()
	wid = GnomeStock(STOCK_PIXMAP_TIMER_STOP)
	wid.show()
	button.add(wid)
	button.show()
	table.attach(button, 0,1, 3,4)
	button.connect('clicked', toggle_button)
	button.set_data('user_data', STOCK_PIXMAP_TIMER_STOP)

	button = GtkButton()
	wid = GnomeStock(STOCK_PIXMAP_TIMER)
	wid.show()
	button.add(wid)
	button.show()
	table.attach(button, 0,1, 3,4)
	button.connect('clicked', toggle_button)
	button.set_data('user_data', STOCK_PIXMAP_TIMER)

	w = GnomeStock(STOCK_PIXMAP_MAIL)
	w.show()
	table.attach(w, 2,3, 3,4)
	w = GnomeStock(STOCK_MENU_MAIL)
	w.show()
	table.attach(w, 2,3, 4,5)
	w = GtkLabel('Mail')
	w.show()
	table.attach(w, 2,3, 5,6)

	w = GnomeStock(STOCK_PIXMAP_MAIL_RCV)
	w.show()
	table.attach(w, 3,4, 3,4)
	w = GnomeStock(STOCK_MENU_MAIL_RCV)
	w.show()
	table.attach(w, 3,4, 4,5)
	w = GtkLabel('Receive Mail')
	w.show()
	table.attach(w, 3,4, 5,6)

	w = GnomeStock(STOCK_PIXMAP_MAIL_SND)
	w.show()
	table.attach(w, 4,5, 3,4)
	w = GnomeStock(STOCK_MENU_MAIL_SND)
	w.show()
	table.attach(w, 4,5, 4,5)
	w = GtkLabel('Send Mail')
	w.show()
	table.attach(w, 4,5, 5,6)

def main():
	win = GnomeApp('stock_demo', 'PyGnome Stock Test')
	win.set_wmclass('stock_test', 'PyGnomeStockTest')
	win.connect('delete_event', message_dlg)
	win.connect('destroy', mainquit)

	win.create_menus(create_menu())
	win.create_toolbar(create_toolbar())

	vbox = GtkVBox(spacing=5)
	vbox.set_border_width(5)
	vbox.show()
	frame = GtkFrame("Other Icons")
	frame.show()
	vbox.pack_start(frame)
	table = GtkTable(1, 1, FALSE)
	table.set_border_width(3)
	table.show()
	frame.add(table)
	fill_table(win, table)

	hbox = GtkHBox(spacing=3)
	hbox.show()
	vbox.pack_end(hbox, expand=FALSE)

	w = GnomeStockButton(STOCK_BUTTON_OK)
	w.show()
	hbox.pack_start(w, fill=FALSE)
	w = GnomeStockButton(STOCK_BUTTON_APPLY)
	w.show()
	hbox.pack_start(w, fill=FALSE)
	w = GnomeStockButton(STOCK_BUTTON_CANCEL)
	w.show()
	hbox.pack_start(w, fill=FALSE)
	w = GnomeStockButton(STOCK_BUTTON_YES)
	w.show()
	hbox.pack_start(w, fill=FALSE)
	w = GnomeStockButton(STOCK_BUTTON_NO)
	w.show()
	hbox.pack_start(w, fill=FALSE)
	w = GnomeStockButton(STOCK_BUTTON_HELP)
	w.show()
	hbox.pack_start(w, fill=FALSE)
	w = GnomeStockButton(STOCK_BUTTON_CLOSE)
	w.show()
	w.connect('clicked', message_dlg)
	hbox.pack_start(w, fill=FALSE)
	

	win.set_contents(vbox)
	win.show()
	mainloop()

if __name__ == '__main__': main()
