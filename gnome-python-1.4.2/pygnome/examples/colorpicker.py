# python-gnome-colorpicker
# Vincent Renardias <vincent@ldsol.com>, 1999/09/10

from gtk import *
from gnome.ui import *
from GDK import *

window = GtkWindow(type='toplevel')
window.connect('destroy', mainquit)
window.set_usize(50, 50)

color = GnomeColorPicker()
window.add(color)

window.show_all()

mainloop()

