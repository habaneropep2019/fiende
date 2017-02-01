from gtk import *
from gnome.ui import *

win = GtkWindow()
win.connect('destroy', mainquit)
win.set_title('Canvas test')

canvas = GnomeCanvas()
canvas.set_size(300, 300)
win.add(canvas)
canvas.show()

canvas.root().add('line', points=(10,10, 90,10, 90,90, 10,90),
		  width_pixels=10, fill_color='blue')

win.show()

mainloop()

