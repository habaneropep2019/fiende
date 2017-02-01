(define-module (gtk foo)
  :use-module (gtk dynlink))

(merge-compiled-code "foo_init_glue" "libfoo")
