(define-module (gtk threads)
  :use-module (gtk gtk)
  :use-module (gtk gdk)
  :use-module (ice-9 threads))

(export gtk-threads-handler? gtk-threads-ensure-handler)

(define handler-running? #f)

(define (gtk-threads-handler?)
  handler-running?)

(define (gtk-threads-ensure-handler)
  (if (not handler-running?)
      (begin-thread
       (dynamic-wind
	   (lambda ()
	     (gdk-threads-enter)
	     (set! handler-running? #t))
	   (lambda ()
	     (gtk-main))
	   (lambda ()
	     (set! handler-running? #f)
	     (gdk-threads-leave))))))
