(define-module (gnome ghttp)
  :use-module (gnome gnome)
  :use-module (gtk gtk)
  :use-module (gtk dynlink))

(merge-compiled-code "ghttp_init_glue" "libguileghttp")

