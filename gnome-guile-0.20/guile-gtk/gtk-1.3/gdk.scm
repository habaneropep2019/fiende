(define-module (gtk-1.3 gdk)
  :use-module (gtk dynlink))

;; Merge compiled code stubs into the interface of the current module.
;; The idea here is to not use the builtin mechanism for dyn-linking
;; at all, because it sucks.  It can't really deal with a more
;; complicated setup like more than one module in a shared library or
;; one compiled module using another.

;; The problems all stem from the fact that the shared library is
;; required to have a name that matches the module name and that it
;; does not work to load a shared library multiple times via different
;; names.  We avoid this problem here by making all details explicit.

;; All other Guile-gtk related compiled code modules are supposed to
;; use this mechanism as well, and build-guile-gtk has some support
;; for it.  We use the special `gtk static-initfuncs' hierarchy for
;; storing information about statically linked init functions.  You
;; should never explicitely request such a module.

(define-public gdk-major-version 1)
(define-public gdk-minor-version 3)
(gtk-version-set "1.3")

(merge-compiled-code "sgtk_init_gtk_gdk_glue" "libguilegtk-1.3")
