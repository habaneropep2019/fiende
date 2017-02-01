(define-module (gtk dynlink)
  :use-module (gtk config)
  :use-module (gtk dynlink-old))

;; Merge some compiled code into the public interface of the current
;; module.  The compiled code will be initialized by calling a
;; function named INIT-FUNC in the library called LIBNAME.

(define-public (merge-compiled-code init-func libname)
  (cond 
   (gtkconf-using-ltdl
    (let* ((module (current-module))
	   (interface (module-public-interface module)))
      ;; make the new primitives visible from within the current module.
      (module-use! module interface) ; XXX - is this safe?
      (save-module-excursion
       (lambda ()
	 (set-current-module interface)
	 (dynamic-call init-func (dynamic-link libname))))))
   (else
    (merge-compiled-code-old init-func libname))))

(define default-module-prefix 
  (string->symbol (string-append "gtk-" gtkconf-default-version)))
(define module-prefix #f)

(define-public (gtk-version-set version)
  (let ((prefix (string->symbol (string-append "gtk-" version))))
    (if (and module-prefix (not (eq? prefix module-prefix)))
	(error "Can't mix" module-prefix 'and prefix)
	(begin
	  (set! gtkconf-version version)
	  (set! module-prefix prefix)))))

(define-public (gtk-version-alias suffix)
  (if (not module-prefix)
      (set! module-prefix default-module-prefix))
  (let* ((mod-name (list module-prefix suffix))
	 (mod-iface (resolve-interface mod-name)))
    (or mod-iface
	(error "no such module" mod-name))
    (set-module-public-interface! (current-module) mod-iface)))
