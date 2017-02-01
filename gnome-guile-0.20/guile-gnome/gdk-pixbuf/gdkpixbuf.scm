(define-module (gtk gdkpixbuf)
  :use-module (gtk gtk)
  :use-module (gtk dynlink))

(merge-compiled-code "libgdkpixbuf_init_glue" "libguilegdkpixbuf")

