(define-module (gnome gnorba)
  :use-module (gtk dynlink)
  :use-module (gnome gnome))

(merge-compiled-code "gnome_gnorba_init_guile_glue" "libguilegnorba")

