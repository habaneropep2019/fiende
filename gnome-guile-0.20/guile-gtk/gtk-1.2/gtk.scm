(define-module (gtk-1.2 gtk)
  :use-module (gtk dynlink))
;; Uncomment when let-multi is uncommented.
;;  :use-module (ice-9 syncase)

(define-public gtk-major-version 1)
(define-public gtk-minor-version 2)
(gtk-version-set "1.2")

(merge-compiled-code "sgtk_init_gtk_gtk_glue" "libguilegtk-1.2")

(define-public (gtk-update)
  (cond ((> (gtk-events-pending) 0)
	 (gtk-main-iteration)
	 (gtk-update))))

(define-public (gtk-standalone-main toplevel)
  (cond ((gtk-standalone?)
	 (gtk-signal-connect toplevel "destroy" gtk-exit)
	 (gtk-main))))

;; Macro for multiple return values
;;
;; The function, which should return 2 or more values does not return
;; the values, but takes the recipient as an argument function and
;; calls it with the return values as input arguments:
;;
;; (define (give-two-1 taker) (taker 5 7))
;; (define (give-two-2 taker) (taker 11 13))
;;
;; To get a list with the return values one can write:
;;
;; (give-two-1 list) => (5 7)
;; (give-two-2 list) => (11 13)
;;
;; To use it like a normal let one can write:
;;
;; (let-multi (((a b) give-two-1)
;;             ((c d) give-two-2))
;;   (+ a b c d))                  => 36

;; Using this complicates our life -- it requires a relatively new
;; version of Guile.  Since we don't actually use it, for now we
;; comment it out.
;(define-syntax let-multi
;  (syntax-rules ()
;    ((let-multi (((name ...) giver)) body1 body2 ...)
;     (giver (lambda (name ...)
;	      body1 body2 ...)))
;    ((let-multi (((name1 ...) giver1)
;		 ((name2 ...) giver2) ...) body1 body2 ...)
;     (giver1 (lambda (name1 ...)
;	       (let-multi (((name2 ...) giver2) ...)
;			  body1 body2 ...))))))

; William D Clinger wrote:
; 
; If MIT Scheme 7.4.2 uses R5RS multiple values, and
; graphics-coordinate-limits returns two values x and y,
; then you would write something like
; 
;   (call-with-values
;    (lambda ()
;      (graphics-coordinate-limits ...))
;    (lambda (x y)
;      ; x and y are the values returned by graphics-coordinate-limits
;      ...))

;; Some aliases and quickies

(define-public gtk-radio-menu-item-new 
  gtk-radio-menu-item-new-from-widget)
(define-public gtk-radio-menu-item-new-with-label
  gtk-radio-menu-item-new-with-label-from-widget)
(define-public gtk-radio-button-new
  gtk-radio-button-new-from-widget)
(define-public gtk-radio-button-new-with-label
  gtk-radio-button-new-with-label-from-widget)
(define-public (gtk-idle-add proc)
  (gtk-idle-add-full 0 proc))

;; The error reporter

(define-public gtk-show-error
  (let ((window #f)
	(text #f))
    (lambda (msg)
      (cond ((not window)
	     (set! window (gtk-window-new 'toplevel))
	     (set! text (gtk-text-new #f #f))
	     (let* ((vscroll (gtk-vscrollbar-new (gtk-text-vadj text)))
		    (close (gtk-button-new-with-label "Close"))
		    (hbox (gtk-hbox-new #f 1))
		    (vbox (gtk-vbox-new #f 3)))

	       (gtk-container-add window vbox)
	       (gtk-box-pack-start vbox hbox #t #t 0)
	       (gtk-box-pack-start hbox text #t #t 0)
	       (gtk-box-pack-start hbox vscroll #f #t 0)
	       (gtk-box-pack-start vbox close #f #t 0)
	       (gtk-window-set-title window "guile-gtk error messages")
	       (gtk-widget-set-usize window 320 200)
	       (gtk-window-set-policy window #t #t #f)
	       (gtk-signal-connect close "clicked"
				   (lambda () (gtk-widget-destroy window)))
	       (gtk-signal-connect window "destroy"
				   (lambda () 
				     (set! window #f)
				     (set! text #f)))
	       (gtk-widget-show-all window))))
      (gtk-text-insert text #f #f #f msg -1))))

(define (call-with-error-catching thunk)
  (let ((the-last-stack #f)
	(stack-saved? #f))
    
    (define (handle-error key args)
      (let ((text (call-with-output-string
		   (lambda (cep)
		     (if the-last-stack
			 (display-backtrace the-last-stack cep)
			 (display "no backtrace available.\n" cep))
		     (apply display-error the-last-stack cep args)))))
	(gtk-show-error text)
	#f))

    (define (save-stack)
      (cond (stack-saved?)
	    ((not (memq 'debug (debug-options-interface)))
	     (set! the-last-stack #f)
	     (set! stack-saved? #t))
	    (else
	     (set! the-last-stack (make-stack #t lazy-dispatch 4))
	     (set! stack-saved? #t))))

    (define (lazy-dispatch key . args)
      (save-stack)
      (apply throw key args))

    (start-stack #t
		 (catch #t
			(lambda ()
			  (lazy-catch #t
				      thunk
				      lazy-dispatch))
			(lambda (key . args)
			  (if (= (length args) 4)
			      (handle-error key args)
			      (apply throw key args)))))))

(define-macro (with-error-catching . body)
  `(call-with-error-catching (lambda () ,@body)))

(gtk-callback-trampoline (lambda (proc args)
			   (with-error-catching
			    (apply proc args))))
