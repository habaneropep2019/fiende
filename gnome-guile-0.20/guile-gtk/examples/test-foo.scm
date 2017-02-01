
;; We load the module manually because Guile wont find the uninstalled
;; version.

(load "foo.scm")
(use-modules (gtk foo) (gtk gtk))

(let ((w (gtk-window-new 'toplevel)))
  (gtk-container-add w (foo-new))
  (gtk-widget-show-all w)
  (gtk-standalone-main w))
