(define-module (gtk repl)
  :use-module (gtk gtk)
  :use-module (gtk gdk)
  :use-module (gtk threads))

;; A event driven repl

(define eof-object (with-input-from-string "" read))

(define-public repl-error-stack car)
(define-public repl-error-args cadr)

(define-public (make-event-repl read eval print error-reporter)
  (let ((the-last-stack #f)
	(stack-saved?   #f)

	(buffer  "")
	(bufpos  0)
	(readeof #f))

    (define (save-stack)
      (cond (stack-saved?)
	    ((not (memq 'debug (debug-options-interface)))
	     (set! the-last-stack #f)
	     (set! stack-saved? #t))
	    (else
	     (set! the-last-stack (make-stack #t lazy-dispatch 4))
	     (set! stack-saved? #t))))

    (define (lazy-dispatch . args)
      (save-stack)
      (apply throw args))
    
    (define (catch-stacked thunk handler)
      (set! stack-saved? #f)
      (start-stack #t
		   (catch #t
			  (lambda ()
			    (lazy-catch #t
					thunk
					lazy-dispatch))
			  (lambda args
			    (if (= (length args) 5)
				(handler 
				 (list (if stack-saved?
					   the-last-stack #f)
				       args))
				(apply throw args))))))

    (define (bufeof?)
      (>= bufpos (string-length buffer)))

    (define (discardbuf)
      (set! buffer (substring buffer bufpos))
      (set! bufpos 0))

    (define bufport (make-soft-port
		     (vector #f #f #f
			     (lambda ()
			       (cond ((bufeof?)
				      (set! readeof #t)
				      #f)
				     (else
				      (let ((ch (string-ref buffer bufpos)))
					(set! bufpos (1+ bufpos))
					ch))))
			     #f)
		     "r"))

    (define (tryread)
      (set! readeof #f)
      (set! bufpos 0)
      (let ((val
	     (catch-stacked
	      (lambda () (read bufport))
	      (lambda (data)
		;; when READ gets an error but has consumed the whole
		;; buffer, we assume it is some kind of `premature end
		;; of input` condition.
		(cond ((not readeof)
		       (error-reporter data)
		       (discardbuf)))
		eof-object))))
	(if (not (eof-object? val))
	    (discardbuf))
	val))

    (define (evalbuf)
      (let loop ((form (tryread)))
	(if (not (eof-object? form))
	    (let* ((throw-args #f)
		   (ans (catch-stacked
			 (lambda () (eval form))
			 (lambda args (set! throw-args args)))))
	      (if throw-args
		  (apply error-reporter throw-args)
		  (print ans))
	      (loop (tryread))))))
      
    (lambda (op . args)
      (case op
	((input)
	 (set! buffer (string-append buffer (car args)))
	 (evalbuf))
	((pending?)
	 (not (bufeof?)))))))

(define-public (repl-input repl str)
  (repl 'input str))

(define-public (repl-pending? repl)
  (repl 'pending?))

(define-public (repl-display-error data . opt-port)
  (let ((port (if (null? opt-port) (current-error-port) (car opt-port))))
    (apply display-error (repl-error-stack data) port 
	   (cdr (repl-error-args data)))))

(define-public (repl-display-backtrace data . opt-port)
  (let ((port (if (null? opt-port) (current-error-port) (car opt-port))))
    (if (repl-error-stack data)
	(display-backtrace (repl-error-stack data) port))))

;; The Gtk repl that doesn't use threads.

(define-public (gtk-event-repl)
  (define inport (current-input-port))
  (define outport (current-output-port))

  (define unspecified (if #f #f))
  (define (prompt)
    (display "gtk> " outport)
    (force-output outport))
  (define (print val)
    (cond ((not (eq? unspecified val))
	   (write val outport)
	   (newline outport)))
    (prompt))
  (define (report data)
    (repl-display-backtrace data outport)
    (repl-display-error data outport)
    (prompt))
  (define (nonblocking-read port)
    (let loop ((res '()))
      (if (char-ready? port)
	  (let ((ch (read-char port)))
	    (if (eof-object? ch)
		(if (null? res)
		    ch 
		    (apply string (reverse res)))
		(loop (cons ch res))))
	  (apply string (reverse res)))))

  (let ((repl (make-event-repl read eval print report)))
    (gtk-input-add inport
		   '(read)
		   (lambda (source condition)
		     (catch 'quit
			    (lambda ()
			      (let ((str (nonblocking-read inport)))
				(if (eof-object? str)
				    (gtk-exit)
				    (repl-input repl str))))
			    (lambda (key . args)
			      (gtk-exit (if (null? args) 0 (car args)))))))
    (prompt)
    (gtk-main)))

;; The default Gtk repl.

(define-public (gtk-repl)
  (cond
   ((feature? 'threads)
    (save-module-excursion
     (lambda ()
       (define-module (guile-user)
	 :use-module (gtk gtk)
	 :use-module (gtk gdk))))
    (gtk-threads-ensure-handler)
    (add-hook! before-read-hook gdk-flush)
    (top-repl))
   (else
    (gtk-event-repl))))
