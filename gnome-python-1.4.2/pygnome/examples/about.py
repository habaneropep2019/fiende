# python-gnome-about
# Vincent Renardias <vincent@ldsol.com>, 1999/09/10

from gtk import *
from gnome.ui import *
from GDK import *

about = GnomeAbout('Title', 'Version', 'Copyright', ['Author1', 'Author2', '...'], 'Comments')

about.connect('destroy', mainquit)

about.show()

mainloop()

