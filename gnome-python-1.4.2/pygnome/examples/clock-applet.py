import gnome.applet
import gtk
import time

def update_label():
	str = time.strftime("%a %b %d\n%I:%M %p", time.localtime(time.time()))
	label.set_text(str)
	return gtk.TRUE

app = gnome.applet.AppletWidget("python-clock")

frame = gtk.GtkFrame()
label = gtk.GtkLabel()
label.set_padding(4,4)
update_label()

frame.add(label)
label.show()

frame.show()
app.add(frame)
app.set_tooltip("Python clock")
app.show()

gtk.timeout_add(60, update_label)

gtk.mainloop()

