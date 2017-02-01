(define-module (gnome zvt)
  :use-module (gtk dynlink))

(merge-compiled-code "zvt_init_glue" "libguilezvt")