# python-gnome-dial
# Vincent Renardias <vincent@ldsol.com>, 1999/09/10

from gtk import *
from gnome.ui import *
from GDK import *

def quit_app(foo):
	#app.set_float("DialValue=", adjustment) 	
	window.hide()
	window.destroy()
	mainquit

app = GnomeApp('gnomedial')
#app.config_get_float("DialValue=")

window = GtkWindow(type='toplevel')
window.connect('destroy', mainquit)
window.set_title('range controls')
window.set_border_width(10)

box1 = GtkVBox(0,10)
window.add(box1)
box1.show()

box2 = GtkVBox(0,10)
box2.set_border_width(0)
box1.pack_start(box2, 1, 1, 0)
box2.show

adjustment = GtkAdjustment(0.0, 0.0, 101.0, 0.1, 1.0, 1.0)

scale = GtkHScale(adjustment)
scale.set_usize(150,30)
scale.set_update_policy('delayed')
scale.set_digits(1)
scale.set_draw_value(1)
box2.pack_start(scale, 1, 1, 0)
scale.show()

scrollbar = GtkHScrollbar(adjustment)
scrollbar.set_update_policy('continuous')
box2.pack_start(scrollbar, 1, 1, 0)
scrollbar.show()

dial = GtkDial(adjustment)
dial.set_update_policy('continuous')
box2.pack_start(dial, 1, 1, 0)
dial.show()

separator = GtkHSeparator()
box1.pack_start(separator, 0, 1, 0)
separator.show()

box2 = GtkVBox(0, 10)
box2.set_border_width(10)
box1.pack_start(box2, 0, 1, 0)
box2.show()

button = GtkButton('close')
#button.connect('clicked', lambda foo: window.destroy())
button.connect('clicked', quit_app)
box2.pack_start(button, 1, 1, 0)
button.set_flags(CAN_DEFAULT)
button.grab_default()

button.show()

window.show_all()

mainloop()

