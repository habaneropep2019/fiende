(use-modules (gtk gtk) (gtk gdk))

(define div quotient)

(define (arc-drawer-new width height start-adj extent-adj)
  (let ((widget (gtk-drawing-area-new))
	(pixmap #f) (window #f)
	(fore-gc #f) (back-gc #f) (handle-gc #f)
	(start #f) (extent #f) (need-update #t)
	(pi (* 2 (acos 0)))
	(poly '((10 . 10) (20 . 10) (10 . 20) (50 . 50)))
	(use-backing #f))

    (define (realize)
      (set! window (gtk-widget-window widget))
      (let ((style (gtk-widget-style widget)))
	(set! fore-gc (gtk-style-fg-gc style 'normal))
	(set! back-gc (gtk-style-bg-gc style 'normal)))
      (configure #f))

    (define (configure ev)
      (cond (ev
	     (set! width (gdk-event-configure-width ev))
	     (set! height (gdk-event-configure-height ev))))
      (cond (window
	     (set! pixmap (if use-backing
			      (gdk-pixmap-new window width height)
			      window))
	     (set! need-update #t)
	     (set! handle-gc (gdk-gc-new pixmap))
	     (gdk-gc-set-foreground handle-gc "red3"))))

    (define (expose ev)
      (cond (use-backing
	     (if need-update (update))
	     (gdk-draw-pixmap window back-gc pixmap 0 0 0 0 width height))
	    (else
	     (update))))
    
    (define (draw-handle x y)
      (gdk-draw-rectangle pixmap handle-gc #t
			  (+ x -2)
			  (+ y -2)
			  4 4))
    (define (draw-poly)
      (gdk-draw-polygon pixmap fore-gc #t poly)
      (for-each (lambda (p) (draw-handle (car p) (cdr p))) poly))
      
    (define (draw-arc)
      (let ((dx 5) (dy 5) (w (- width 10)) (h (- height 10)))
	(define (draw-arc-handle angle)
	  (define (->rad x) (* x (/ pi (* 180 64))))
	  (let ((x (inexact->exact (* 0.5 w (cos (->rad angle)))))
		(y (inexact->exact (* -0.5 h (sin (->rad angle))))))
	    (draw-handle (+ (div w 2) dx x) (+ (div h 2) dy y))))
      
	(gdk-draw-arc pixmap fore-gc #f
		      dx dy w h (remainder start (* 360 64)) extent)
	(draw-arc-handle start)
	(draw-arc-handle (+ start extent))))

    (define button1-motion-handler #f)
    (define (button1-motion ev)
      (if button1-motion-handler (button1-motion-handler ev)))
    (define (button-release ev)
      (cond ((= (gdk-event-button ev) 1)
	     (set! button1-motion-handler #f))))

    (define (drag-poly ev)
      (define (find-poly-handle x y)
	(or-map (lambda (p)
		  (and (< (abs (- x (car p))) 4)
		       (< (abs (- y (cdr p))) 4)
		       p))
		poly))
      (let ((handle (find-poly-handle (gdk-event-x ev) (gdk-event-y ev))))
	(cond (handle
	       (set! button1-motion-handler
		     (lambda (ev)
		       (let ((x (gdk-event-x ev))
			     (y (gdk-event-y ev)))
			 (set-car! handle x)
			 (set-cdr! handle y)
			 (update))))))))

    (define (update)
      (set! start (inexact->exact (* (gtk-adjustment-value start-adj) 64)))
      (set! extent (inexact->exact (* (gtk-adjustment-value extent-adj) 64)))
      (cond (window
	     (gdk-draw-rectangle pixmap back-gc #t 0 0 width height)
	     (draw-arc)
	     (draw-poly)
	     (set! need-update #f)
	     (if use-backing (expose #f)))))

    (define (pk-event ev)
      (pk (gdk-event-type ev) (gdk-event-x ev) (gdk-event-y ev)))

    (gtk-signal-connect widget "button_press_event" drag-poly)
    (gtk-signal-connect widget "button_release_event" button-release)
    (gtk-signal-connect widget "motion_notify_event" button1-motion)
    (gtk-signal-connect widget "realize" realize)
    (gtk-signal-connect widget "expose_event" expose)
    (gtk-signal-connect widget "configure_event" configure)
    (gtk-signal-connect start-adj "value_changed" update)
    (gtk-signal-connect extent-adj "value_changed" update)
    (gtk-drawing-area-size widget width height)
    (gtk-widget-set-events widget '(exposure-mask
				    button-press-mask
				    button-release-mask
				    button1-motion-mask
				    key-press-mask))

    (lambda (op . args)
      (case op
	((widget)
	 widget)
	((use-backing)
	 (set! use-backing (car args))
	 (configure #f))))))

(let* ((window (gtk-window-new 'toplevel))
       (vbox   (gtk-vbox-new #f 5))
       (start-adj (gtk-adjustment-new 360.0 0.0 721.0 1.0 1.0 1.0))
       (start-scl (gtk-hscale-new start-adj))
       (extent-adj (gtk-adjustment-new 180.0 0.0 361.0 1.0 1.0 1.0))
       (extent-scl (gtk-hscale-new extent-adj))
       (arc    (arc-drawer-new 150 150 start-adj extent-adj))
       (backing-button (gtk-check-button-new-with-label "Use backing pixmap"))
       (close  (gtk-button-new-with-label "close")))

  (gtk-container-add window vbox)
  (gtk-box-pack-start vbox (arc 'widget) #t #t 0)
  (gtk-box-pack-end vbox close #f #f 0)
  (gtk-box-pack-end vbox backing-button #f #f 0)
  (gtk-box-pack-end vbox extent-scl #f #f 0)
  (gtk-box-pack-end vbox start-scl #f #f 0)
  (gtk-signal-connect backing-button "clicked"
		      (lambda ()
			(arc 'use-backing
			     (gtk-widget-get backing-button 'active))))
  (gtk-signal-connect close "clicked" (lambda () (gtk-widget-destroy window)))
  (gtk-scale-set-draw-value start-scl #f)
  (gtk-scale-set-draw-value extent-scl #f)
  (gtk-widget-show-all window)
  (gtk-standalone-main window))
