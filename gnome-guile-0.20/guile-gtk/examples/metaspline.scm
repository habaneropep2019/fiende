#! /usr/local/bin/guile-gtk -s
!#

;;;
;;; Meta-Spline
;;;

;; Allow the user to edit a Bezier spline with parameters as used in
;; Metafont.  Normally, you would use a GtkCanvas for this, but I'll
;; do it the hard way to test out the guile-gtk low-level Gdk support.
;; And my ability to understand a Knuthonian algorithm.

(define-module (metaspline)
  :use-module (gtk gtk)
  :use-module (gtk gdk)
  :use-module (ice-9 common-list))

;;; Utilities

;; This is a little cheesy.  The fix would be to make internal defines
;; behave like letrec*, if you know what I mean.

(defmacro define-struct (tag . fields)
  (let* ((field-syms (map (lambda (f)
			    (if (pair? f) (car f) f))
			  fields))
	 (make-parms (remove-if pair? fields))
	 (make-args (map (lambda (f)
			   (if (pair? f) (cadr f) f))
			 fields))
	 (type (make-record-type tag field-syms)))
    `(begin
       (define ,(symbol-append 'make- tag)
	 (let ((maker (record-constructor ',type)))
	   (lambda ,make-parms
	     (maker ,@make-args))))
       (define ,(symbol-append tag '?) (record-predicate ',type))
       ,@(map (lambda (f)
		`(define ,(symbol-append tag '- f)
		   (record-accessor ',type ',f)))
	      field-syms)
       ,@(map (lambda (f)
		`(define ,(symbol-append tag '-set- f '!)
		   (record-modifier ',type ',f)))
	      field-syms))))

;;; Tinytalk in 40 lines Scheme...

(define (set-dispatcher! obj proc)
  (set-object-property! obj 'dispatcher proc))

(define (get-dispatcher obj)
  (object-property obj 'dispatcher))

(define (send obj . msg)
  (let ((dispatcher (get-dispatcher obj)))
    (if dispatcher 
	(apply dispatcher msg)
	(error "Not an object: " obj))))

(defmacro define-dispatcher (obj . cases)
  `(let* ((obj ,obj)
	  (parent (get-dispatcher ,obj))
	  (send-super (lambda (op . args)
			(if parent
			    (apply parent op args)
			    (error "object has no super class")))))
     (set-dispatcher! obj
		      (lambda (op . args)
			(case op
			  ,@(map (lambda (c)
				   (let ((proto (car c))
					 (body (cdr c)))
				     `((,(car proto))
				       (apply (lambda ,(cdr proto) ,@body)
					      args))))
				 cases)
			  (else
			   (if parent
			       (apply parent op args)
			       (error "no method for: " op))))))))

(defmacro define-methods (class . methods)
  `(begin ,@(map (lambda (m)
		   `(define (,(symbol-append class '- (car m)) ,@(cdr m))
		      (send ,(cadr m) ',(car m) ,@(cddr m))))
		 methods)))

(define make-point cons)
(define point-x car)
(define point-y cdr)
(define (point-sub p1 p2)
  (make-point (- (point-x p1) (point-x p2))  (- (point-y p1) (point-y p2))))
(define (point-add p1 p2)
  (make-point (+ (point-x p1) (point-x p2))  (+ (point-y p1) (point-y p2))))
(define (point-dist p1 p2) 
  (sqrt (+ (expt (- (point-x p1) (point-x p2)) 2)
	   (expt (- (point-y p1) (point-y p2)) 2))))

;;; The drawing board

(define (drawing-board-new)
  (let ((widget (gtk-drawing-area-new))
	(configures '())
	(updates '())
	(window #f)
	(width #f)
	(height #f)
	(back-gc #f)
	(need-configure #f))

    (define (configure)
      (if (not back-gc)
	  (let ((style (gtk-widget-style widget)))
	    (set! back-gc (gtk-style-bg-gc style 'normal))))
      (for-each (lambda (c) (c window width height)) configures))

    (define (update)
      (gdk-draw-rectangle window back-gc #t 0 0 width height)
      (for-each (lambda (u) (u)) updates))
    
    (define (realize)
      (set! window (gtk-widget-window widget))
      (if need-configure (configure)))

    (define (configure-event ev)
      (if ev
	  (begin
	    (set! width (gdk-event-configure-width ev))
	    (set! height (gdk-event-configure-height ev))
	    (if window 
		(configure)
		(set! need-configure #t)))))

    (define (expose-event ev)
      (update))
    
    (gtk-signal-connect widget "realize" realize)
    (gtk-signal-connect widget "configure_event" configure-event)
    (gtk-signal-connect widget "expose_event" expose-event)

    (define-dispatcher widget
      ((add-configure proc)
       (set! configures (cons proc configures)))
      ((rem-configure proc)
       (set! configures (delq! proc configures)))
      ((add-update proc)
       (set! updates (cons proc updates)))
      ((rem-update proc)
       (set! updates (delq! proc updates)))
      ((update)
       (if window
	   (update))))

    widget))

(define-methods drawing-board
  (add-configure board proc)
  (rem-configure board proc)
  (add-update board proc)
  (rem-update board proc)
  (update board))

;;; The handle box.  Turns a drawing board into something that can
;;; push handles around.

(define (handle-box-new)
  (let ((board #f)
	(window #f)
	(fore-gc #f)
	(active-gc #f)
	(width #f) 
	(height #f)

	(settled #t)
	(settled-timer #f)
	(settle-notify '())

	(all-handles '())
	(active-handle #f)
	(dragged-handle #f))

    (define (configure wnd w h)
      (if (not fore-gc)
	  (let ((style (gtk-widget-style board)))
	    (set! fore-gc (gtk-style-fg-gc style 'normal))
	    (set! active-gc (gdk-gc-new wnd))
	    (gdk-gc-set-foreground active-gc "red3")))
      (set! window wnd)
      (set! width w)
      (set! height h))

    (define-struct handle posf placef )
    (define (handle-pos h) ((handle-posf h)))
    (define (handle-place! h p) ((handle-placef h) p))
    (define (handle-hit? h p)
      (< (point-dist p (handle-pos h)) 5))

    (define (draw-handle h)
      (let* ((p (handle-pos h))
	     (x (point-x p))
	     (y (point-y p)))
	(gdk-draw-rectangle window (if (eq? h active-handle) active-gc fore-gc)
			    #t (+ x -2) (+ y -2) 5 5)))

    (define (settled-timeout)
      (set! settled-timer #f)  ;; timeouts can't reliably be removed
			       ;; while the callback runs, it seems.
      (set-settled #t)
      #f)

    (define (set-settled f)
      (if settled-timer
	  (gtk-timeout-remove settled-timer))
      (if (not f)
	  (set! settled-timer (gtk-timeout-add 150 settled-timeout)))
      (cond ((not (eq? f settled))
	     (set! settled f)
	     (for-each (lambda (s) (s settled)) settle-notify))))

    (define (button-press ev)
      (if (= (gdk-event-button ev) 1)
	  (let* ((p (make-point (gdk-event-x ev) (gdk-event-y ev)))
		 (h (or-map (lambda (h) (and (handle-hit? h p) h))
			    all-handles)))
	    (set! active-handle h)
	    (set! dragged-handle h)
	    (drawing-board-update board))))

    (define (motion-notify ev)
      (cond (dragged-handle
	     (set-settled #f)
	     (handle-place! dragged-handle 
			    (make-point (gdk-event-x ev) (gdk-event-y ev))))))
    

    (define (button-release ev)
      (cond ((= (gdk-event-button ev) 1)
	     (set! dragged-handle #f)
	     (set-settled #t)
	     (drawing-board-update board))))

    (define (add-handle h)
      (set! all-handles (cons h all-handles)))

    (define (rem-handle h)
      (set! all-handles (delq! h all-handles)))

    (define (add-settle-notify p)
      (set! settle-notify (cons p settle-notify)))

    (define (update)
      (for-each draw-handle all-handles))

    (define-struct HandleBox) ; dummy

    (let ((box (make-HandleBox)))
      (define-dispatcher box

        ((set-board b)
	 (set! board b)
	 (drawing-board-add-configure board configure)
	 (drawing-board-add-update board update)
	 
	 (gtk-widget-set-events board '(exposure-mask
					button-press-mask
					button-release-mask
					button1-motion-mask))
	 (gtk-signal-connect board "button_press_event" button-press)
	 (gtk-signal-connect board "button_release_event" button-release)
	 (gtk-signal-connect board "motion_notify_event" motion-notify))

	((add-handle posf placef)
	 (let ((h (make-handle posf placef)))
	   (add-handle h)
	   h))
	((rem-handle h)
	 (rem-handle h))

	((dragging?)
	 dragged-handle)
	((add-settle-notify proc)
	 (add-settle-notify proc)))

      box)))

(define-methods handle-box
  (set-board box board)
  (add-handle box posf placef)
  (rem-handle box handle)
  (dragging? box)
  (add-settle-notify box proc))

;;; Data structures for paths

;; type is one of endpoint, explicit, given, curl, open.

(define-struct knot 
  pos 
  control- control+
  left-tension right-tension
  left-type right-type
  left-curl right-curl
  left-given right-given)


(define (make-start-knot pos)
  (make-knot
   pos
   #f #f
   1.0 1.0
   'endpoint 'curl
   #f 1.0
   #f #f))

(define (make-mid-knot pos)
  (make-knot
   pos
   #f #f
   1.0 1.0
   'open 'open
   #f #f
   #f #f))

(define (make-end-knot pos)
  (make-knot
   pos
   #f #f
   1.0 1.0
   'curl 'endpoint
   1.0 #f
   #f #f))

;;; The spline drawer

(define (spline-drawer-new)
  (let ((board #f)
	(box #f)
	(window #f)
	(fore-gc #f)
	(back-gc #f)
	(spline-gc #f)
	(control-gc #f)
	(width #f) (height #f)

	(spline '())
	(bezier-steps 5))

    (define (configure wnd w h)
      (if (not fore-gc)
	  (let ((style (gtk-widget-style board)))
	    (set! fore-gc (gtk-style-fg-gc style 'normal))
	    (set! spline-gc (gdk-gc-new wnd))
	    (gdk-gc-set-foreground spline-gc "black")
	    (gdk-gc-set-line-attributes spline-gc 2 'solid 'round 'round)
	    (set! control-gc (gdk-gc-new wnd))
	    (gdk-gc-set-foreground control-gc "black")
	    (gdk-gc-set-line-attributes control-gc 0 'on-off-dash 'round 
					'round)))
      (set! window wnd)
      (set! width w)
      (set! height h))

    ;; knots

    (define (draw-knot k)
      (if (knot-control- k)
	  (gdk-draw-lines window control-gc (list (knot-pos k)
						  (knot-control- k))))
      (if (knot-control+ k)
	  (gdk-draw-lines window control-gc (list (knot-pos k)
						  (knot-control+ k)))))

    (define (draw-knots knots)
      (for-each draw-knot knots))

    ;; Bezier curves

    (define (bezier-point knots t)
      (let* ((seg (inexact->exact (floor t)))
	     (t (- t seg))
	     (k1 (list-ref knots seg))
	     (k2 (list-ref knots (1+ seg))))
	(bezier-point1 (knot-pos k1) (knot-control+ k1)
		       (knot-control- k2) (knot-pos k2)
		       t)))

    (define (bezier-point1 p1 p2 p3 p4 t)
      (define (scalar-cubic s1 s2 s3 s4 t)
	(inexact->exact
	 (+ (* (expt (- 1 t) 3) s1)
	    (* 3 t (expt (- 1 t) 2) s2)
	    (* 3 t t (- 1 t) s3)
	    (* t t t s4))))
      (make-point (scalar-cubic (point-x p1) (point-x p2)
				(point-x p3) (point-x p4) t)
		  (scalar-cubic (point-y p1) (point-y p2)
				(point-y p3) (point-y p4) t)))

    (define (draw-bezier knots)
      (cond ((null? knots)
	     #t)
	    ((not (null? (cdr knots)))
	     (let ((k1 (car knots))
		   (k2 (cadr knots)))
	       (draw-bezier1 window spline-gc
			     (knot-pos k1) (knot-control+ k1)
			     (knot-control- k2) (knot-pos k2))
	       (draw-bezier (cdr knots))))))

    (define (draw-bezier1 window gc p1 p2 p3 p4)
      (let* ((points (make-vector (1+ bezier-steps))))
	(vector-set! points 0 p1)
	(do ((i 1 (1+ i)))
	    ((= i bezier-steps))
	  (vector-set! points i (bezier-point1 p1 p2 p3 p4 
					       (/ i bezier-steps))))
	(vector-set! points bezier-steps p4)
	(gdk-draw-lines window gc points)))

    (define (update)
      (draw-bezier spline)
      (draw-knots spline))

    (define-struct Spline)  ; dummy

    (let ((sp (make-Spline)))
      (define-dispatcher sp

	((set-board b)
	 (set! board b)
	 (drawing-board-add-configure board configure)
	 (drawing-board-add-update board update))

	((set-handle-box b)
	 (set! box b)
	 (handle-box-add-settle-notify box
				       (lambda (f) 
					 (set! bezier-steps (if f 30 7))
					 (drawing-board-update board))))

	((set-knots kn)
	 (set! spline kn)
	 (drawing-board-update board)))

      sp)))

(define-methods spline-drawer
  (set-board spline board)
  (set-handle-box spline box)
  (set-knots spline knots))

;;; The spline controller - implements the Metafont algorithm for
;;; choosing control points

(define (spline-controller-new)
  (let ((board #f)
	(box #f)
	(knots '())
	(handles '())
	(selected-knot #f))

    (define (rem-handles)
      (for-each (lambda (h) (handle-box-rem-handle box h)) handles)
      (set! handles '()))

    (define (update-handles)
      (rem-handles)
      (for-each (lambda (k)
		  (define (pos) (knot-pos k))
		  (define (pos-) (knot-control- k))
		  (define (pos+) (knot-control+ k))
		  (define (place p)
		    (let ((dp (point-sub p (knot-pos k))))
		      (knot-set-pos! k p)
		      (if (knot-control- k)
			  (knot-set-control-! k (point-add (knot-control- k)
							   dp)))
		      (if (knot-control+ k)
			  (knot-set-control+! k (point-add (knot-control+ k)
							   dp)))
		      (update-choices)))
		  (define (place- p)
		    (knot-set-control-! k p)
		    (update-choices))
		  (define (place+ p)
		    (knot-set-control+! k p)
		    (update-choices))
		  (set! handles
			(cons (handle-box-add-handle box pos place)
			      handles))
		  (if (eq? (knot-left-type k) 'explicit)
		      (set! handles
			    (cons (handle-box-add-handle box pos- place-)
				  handles)))
		  (if (eq? (knot-right-type k) 'explicit)
		      (set! handles
			    (cons (handle-box-add-handle box pos+ place+)
				  handles))))
		knots))

    (define (make-choices)
      ;; How low can you go.
      (for-each (lambda (k)
		  (if (not (eq? (knot-left-type k) 'endpoint))
		      (knot-set-control-! k (point-add (knot-pos k)
						      (make-point -20 0))))
		  (if (not (eq? (knot-right-type k) 'endpoint))
		      (knot-set-control+! k (point-add (knot-pos k)
						      (make-point 20 0)))))
		knots))

    (define (update-choices)
      (make-choices)
      (drawing-board-update board))

    (define (make-type-frame label)
      (let ((f (gtk-frame-new label))
	    (vbox (gtk-vbox-new #f 2))
	    (group #f))
	(gtk-container-add f vbox)
	(for-each (lambda (t)
		    (set! group (gtk-radio-button-new-with-label 
				 group (symbol->string t)))
		    (gtk-box-pack-start vbox group #f #f)
		    (gtk-signal-connect group "clicked"
					(lambda () (pk 'type t))))
		  '(explicit given curl open))
	f))
	
    (let ((widget (gtk-hbox-new #f 2))
	  (left-type-frame (make-type-frame "left type"))
	  (right-type-frame (make-type-frame "right type")))
      (gtk-box-pack-start widget left-type-frame #f #f)
      (gtk-box-pack-end widget right-type-frame #f #f)
		
      (define-dispatcher widget
	((set-board  b)
	 (set! board b))
	((set-handle-box b)
	 (rem-handles)
	 (set! box b)
	 (update-handles))
	((set-knots k)
	 (set! knots k)
	 (update-handles)
	 (make-choices)))

      widget)))

(define-methods spline-controller
  (set-board cntrl board)
  (set-handle-box cntrl box)
  (set-knots cntrl knots))

;;; Main

(let ((toplevel (gtk-window-new 'toplevel))
      (vbox (gtk-vbox-new #f 1))
      (board (drawing-board-new))
      (box (handle-box-new))
      (drawer (spline-drawer-new))
      (controller (spline-controller-new))
      (quit-b (gtk-button-new-with-label "Quit")))

  (spline-drawer-set-handle-box drawer box)
  (spline-controller-set-handle-box controller box)
  (handle-box-set-board box board)
  (spline-drawer-set-board drawer board)
  (spline-controller-set-board controller board)

  (gtk-container-add toplevel vbox)
  (gtk-box-pack-start vbox board #t #t)
  (gtk-box-pack-start vbox controller #f #f)
  (gtk-box-pack-start vbox quit-b #f #f)

  (let ((kn (list (make-start-knot (make-point 100 50))
		  (make-mid-knot (make-point 200 50))
		  (make-end-knot (make-point 300 50)))))
    (spline-controller-set-knots controller kn)
    (spline-drawer-set-knots drawer kn))

  (gtk-signal-connect quit-b "clicked"
		      (lambda () (gtk-widget-destroy toplevel)))

  (gtk-drawing-area-size board 400 200)
  (gtk-widget-show-all toplevel)
  (gtk-standalone-main toplevel))
