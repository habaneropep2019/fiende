;; -*- scheme -*-

(define-module (gtk dynlink-old)
  :use-module (gtk config)
  :use-module (ice-9 regex))

;; Alternative implementation of dynamic linking for compiled code
;; modules

;; The idea here is to not use the builtin mechanism for dyn-linking
;; at all, because it sucks.  It can't really deal with a more
;; complicated setup like more than one module in a shared library or
;; one compiled module using another.

;; The problems all stem from the fact that the shared library is
;; required to have a name that matches the module name and that it
;; does not work to load a shared library multiple times via different
;; names.  In addition, dlopen does not deal with version numbers,
;; etc.

;; I hope to use GNU dld in the future, which should provide a sane
;; dlopen abstraction on all supported platforms.

;; All other Guile-gtk related compiled code modules are supposed to
;; use this mechanism as well.  We use the special `gtk
;; %static-initfuncs%' module directory for storing information about
;; statically linked init functions.  You should never explicitely
;; request a module from this directory.

(define (pk . args)
  (write args (current-error-port))
  (newline)
  (car (last-pair args)))

(define this-module (current-module))

;; Split STR at its colons, resulting in a list of strings.

(define (split-colon-path str)
  (if (or (not str) (zero? (string-length str)))
      '()
      (let ((pos (string-index str #\:)))
	(if pos
	    (cons (substring str 0 pos)
		  (split-colon-path (substring str (1+ pos))))
	    (list str)))))

;; Extended version of `search-path': we also look for libraries with
;; trailing version numbers here.

(define (search-path-extended name path)
  (if (and (> (string-length name) 0) (eqv? (string-ref name 0) #\/))
      name
      (or-map (lambda (p)
		(let ((file-list (list))
		      (pattern (if (string-match "\.(so|la)$" name)
				   (string-append name "(-[0-9]+)*")
				   (string-append name "\.so*"))
			       ))
		  (and (file-exists? p)
		       (file-is-directory? p)
		       (let ((dirh (opendir p)))
			 (while (let ((file (readdir dirh)))
				  (if (not (eof-object? file))
				      (begin
					(set! file-list
					      (append file-list (list file)))
					#t)
				      #f))
				noop)))
		  (or-map (lambda (f)
			    (let ((match (string-match pattern f)))
			      (if match
				  (string-append p "/" (array-ref match 0))
				  #f)))
			  file-list)))
	      path)))

;; Try each element of PATH for NAME.

(define (search-path name path)
  (if (and (> (string-length name) 0) (eqv? (string-ref name 0) #\/))
      name
      (or
       (or-map (lambda (p)
		 (let ((full-name (string-append p "/" name)))
		   (if (file-exists? full-name)
		       full-name
		       #f)))
	       path)
       (search-path-extended name path)
       )
      )
  )

(define (default-lib-path)
  (append (split-colon-path (getenv "LD_LIBRARY_PATH"))
	  (list gtkconf-libdir "/usr/local/lib" "/usr/X11R6/lib" "/usr/lib" "/lib")))

;; Find a libtool information file.  We search PATH for "NAME.la" and
;; return the full path name.  When the file can not be found, return #f.

(define (find-libtool-library name path)
  (search-path (string-append name ".la") path))

;; Return the contents of NAME.la as an alist.  PATH is used to find
;; NAME.la.

(define-public (libtool-library-info name path)
  (define (split line)
    (let ((pos (string-index line #\=)))
      (if pos
	  (let ((key (substring line 0 pos))
		(value (substring line (1+ pos))))
	    (if (eqv? (string-ref value 0) #\')
		(set! value (substring value 1 (1- (string-length value)))))
	    (cons (string->symbol key) value))
	  #f)))
  (let ((full-name (find-libtool-library name path)))
    (if full-name
	(with-input-from-file full-name
	  (lambda ()
	    (let loop ((line (read-line))
		       (info '()))
	      (if (eof-object? line)
		  (reverse info)
		  (let ((i (split line)))
		    (if i
			(loop (read-line) (cons i info))
			(loop (read-line) info)))))))
	'())))

;;; Finding the shared object filename from a plain library name like
;;; "libm" that makes dlopen open that library.  On some systems it is
;;; enough to just return "libm.so", on other systems we have to
;;; search the path ourselves and maybe do magic stuff with version
;;; numbers.

;;(define (find-soname name path)
;;  (define (string-string str1 str2)
;;    (let ((len1 (string-length str1))
;;	  (len2 (string-length str2)))
;;      (let loop ((pos 0))
;;	(cond ((> (+ pos len2) len1)
;;	       #f)
;;	      ((string=? (substring str1 pos (+ pos len2)) str2)
;;	       pos)
;;	      (else
;;	       (loop (1+ pos)))))))
;;  (let ((full-name (pk 'full-name 
;;		       (search-path (string-append name ".so") path))))
;;    (and full-name
;;	 (let ((real-name (pk 'real-name (readlink full-name))))
;;	   (if real-name
;;	       (let ((pos (string-string real-name ".so.")))
;;		 (cond (pos
;;			(set! pos (+ pos 4))
;;			(do ()
;;			    ((or (> pos (string-length real-name))
;;				 (not (char-numeric? (string-ref real-name
;;								 pos)))))
;;			  (set! pos (1+ pos)))
;;			(pk 'soname (substring real-name 0 pos)))
;;		       (else
;;			real-name)))
;;	       full-name)))))

;; Return the soname of a shared object.  Originally I thought that we
;; have to be clever here because of versions and different names for
;; one library.  But I know think that dlopen is clever enough.  I'm
;; confused.

;; -------------------------------------------------------------------
;; [Followup to the comment above:]
;;
;; This is not the case on BSD-like systems (FreeBSD), so we need to
;; do the LD_LIBRARY_PATH search here - but I also want to use this
;; feature on systems where it is supported like Linux.
;;
;; So my idea was to do the LD_LIBRARY_PATH search here and pass both
;; the old name and the full pathname to %sgtk-dlopen - this function
;; is responsible for finding out which of them to use.
;;
;; Oct 24, 1998. Martin
;; -------------------------------------------------------------------

;; (define (find-soname name path)
;;   (string-append name ".so"))

(define-public (find-soname name path)
  (let ((full-name (search-path (string-append name ".so") path)))
    (if full-name full-name
	(begin (warn "Cannot find library" name) #f))))

;; We have to use our own dynamic-link function because Guile's
;; doesn't use the RTLD_GLOBAL flag.

(define (init-dlopen-funcs)
  (if (not (module-defined? this-module '%sgtk-dlopen))
      (save-module-excursion
       (lambda ()
	 (let ((full-name (string-append gtkconf-exec-prefix
					 "/lib/libguiledlopenhelper.so")))
	   (set-current-module this-module)
	   (dynamic-call 'sgtk_dlopenhelper_init 
			 (dynamic-link full-name)))))))

(define (sgtk-dlopen real-name full-name)
  (pk 'dlopening real-name full-name)
  (%sgtk-dlopen real-name full-name))

;; Find and link a library named NAME, searching PATH.  When PATH is
;; not specified, use a default search path.  NAME should be something
;; like "libfoo", without any extensions.  When NAME.la can not be
;; found, try NAME.so.  When NAME.so can't be found either, return #f.

(define-public (dlopen-libtool-library name . opt-path)
  (cond
   (gtkconf-libtool-is-winning
    (let* ((path (if (pair? opt-path) (car opt-path) (default-lib-path)))
	   (full-name (find-soname name path)))
      (init-dlopen-funcs)
      (sgtk-dlopen (string-append name ".so") full-name)))
   (else
    (let* ((path (if (pair? opt-path) (car opt-path) (default-lib-path)))
	   (info (libtool-library-info name path))
	   (dlname (assq 'dlname info))
	   (deps (assq 'dependency_libs info))
	   (libdir (assq 'libdir info))
	   (real-name (string-append name ".so")))
      (init-dlopen-funcs)
      (cond ((and dlname (> (string-length (cdr dlname)) 0))
	     (if deps (do-libtool-link (cdr deps)))
	     (let ((full-name (if libdir
				  (string-append (cdr libdir) "/" (cdr dlname))
				  (cdr dlname))))
	       (and full-name (sgtk-dlopen real-name full-name))))
	    (else
	     (let ((full-name (find-soname name path)))
	       (and full-name (sgtk-dlopen real-name full-name)))))))))
   
;; Return a procedure that returns the next space-separated word of
;; STR each time it is called.

(define (make-word-scanner str)
  (let ((pos 0)
	(len (string-length str)))
    (lambda ()
      (let loop ((start pos))
	(cond ((>= start len)
	       #f)
	      ((eqv? (string-ref str start) #\space)
	       (loop (1+ start)))
	      (else
	       (let loop ((stop start))
		 (if (or (>= stop len) (eqv? (string-ref str stop) #\space))
		     (begin
		       (set! pos stop)
		       (substring str start stop))
		     (loop (1+ stop))))))))))

;; Perform run-time linking of LIBS.  LIBS should be a string like you
;; would specify it to the system linker, like "-L/usrX11R6/lib -lX11".
;; Returns an unspecified value.

(define-public (do-libtool-link libs)
  (let ((next-word (make-word-scanner libs))
	(path (default-lib-path)))
    (define (option-arg word)
      (if (< (string-length word) 3)
	  (next-word)
	  (substring word 2)))
    (define (doit)
      (let ((word (next-word)))
	(if (and word (eqv? (string-ref word 0) #\-))
	    (let ((arg (option-arg word)))
	      (case (string-ref word 1)
		((#\l)
		 (let ((path path))
		   (doit) ; process rest first
		   (or (dlopen-libtool-library (string-append "lib" arg)
					       path)
		       (display (string-append "Can't link lib" arg
					       ", hope that's ok.\n")
				(current-error-port)))))
		((#\L)
		 (set! path (cons arg path))
		 (doit))
		(else
		 (display (string-append "Ignoring link option" word "\n")
			  (current-error-port))
		 (doit)))))))
    (doit)))

(define (update-registered-modules)
  (set! registered-modules 
	(append! (convert-c-registered-modules #f)
		 registered-modules)))

;; Merge some compiled code into the public interface of the current
;; module.  The compiled code will be initialized by calling a
;; function named INIT-FUNC in the library called LIBNAME.  When
;; INIT-FUNC is among the pre-loaded libraries, we use it.  Else we
;; try to dynamically link LIBNAME, which should be a shared library
;; built and installed with libtool.  LIBNAME should not include any
;; extension, like ".la" or ".so", or version number suffices.

(define-public (merge-compiled-code-old init-func libname)
  (let* ((module (current-module))
	 (interface (module-public-interface module)))
    ;; make the new primitives visible from within the current module.
    (module-use! module interface) ; XXX - is this safe?
    (save-module-excursion
     (lambda ()
       (update-registered-modules)
       (set-current-module interface)
       (let* ((modname (list 'gtk '%static-initfuncs%
			     (string->symbol init-func)))
	      (modinfo (or-map (lambda (modinfo)
				 (if (equal? (car modinfo) modname)
				     modinfo
				     #f))
			       registered-modules))
	      (init-func (if modinfo (cadr modinfo) init-func))
	      (lib       (if modinfo (caddr modinfo)
			     (or (dlopen-libtool-library libname)
				 (error "can't open library" libname)))))
	 (if (not modinfo)
	     (%sgtk-dlinit init-func lib)
	     (dynamic-call init-func #f)))))))

