
# python-gnome-calculator
# Vincent Renardias <vincent@ldsol.com>, 1999/09/10

from gtk import *
from gnome.ui import *
from GDK import *

window = GtkWindow(type='toplevel')
window.connect('destroy', mainquit)

calc = GnomeCalculator()
window.add(calc)

window.show_all()

mainloop()

