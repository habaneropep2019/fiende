(define-module (gtk libglade)
  :use-module (gnome gnome)
  :use-module (gtk gtk)
  :use-module (gtk gdk)
  :use-module (gtk dynlink))

(merge-compiled-code "libglade_init_glue" "libguileglade")
