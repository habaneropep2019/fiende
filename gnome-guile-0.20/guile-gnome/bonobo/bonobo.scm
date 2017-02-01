(define-module (gnome bonobo)
  :use-module (gtk dynlink))

(merge-compiled-code "bonobo_init_glue" "libguilebonobo")



