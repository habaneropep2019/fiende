(define-module (gnome gnome)
  :use-module (gtk dynlink)
  :use-module (gtk gtk))       ;; this also causes libguilegtk to be linked.

(merge-compiled-code "gnome_init_guile_glue" "libguilegnome")

; FIXME: do all of these really need to be here?

(define-public (launch-gnome fn)
  ; call to gtk-main moved to gnome-guile.c (federico)
  (fn)
  (gtk-main))

(define-public (gtk-widget-show-multi . list)
  (for-each gtk-widget-show list))

(define (internal-make-filled-box boxfunc homogeneous padding widgets)
  (let ((box (boxfunc homogeneous padding))
	(state 'pack-start))
    (for-each (lambda (thing)
		(cond
		 ((eq? thing 'pack-start) (set! state 'pack-start) )
		 ((eq? thing 'pack-end) (set! state 'pack-end))
		 ((and (list? thing) (eq? 'boxed-widget (car thing)))
		  (case state
		    ((pack-start) (gtk-box-pack-start box
						      (cadr thing)
						      (caddr thing)
						      (cadddr thing)
						      (car (cddddr 
thing)))
				  (gtk-widget-show (cadr thing)))
		    ((pack-end) (gtk-box-pack-end box
						  (cadr thing)
						  (caddr thing)
						  (cadddr thing)
						  (car (cddddr thing)))
				(gtk-widget-show (cadr thing)))))
		 (thing (error 'wrong-type-arg thing))))
	      widgets)
    (gtk-widget-show box)
    box))

(define-public (gnome-make-filled-hbox homogeneous padding . widgets)
  (internal-make-filled-box gtk-hbox-new homogeneous padding widgets))

(define-public (gnome-make-filled-vbox homogeneous padding . widgets)
  (internal-make-filled-box gtk-vbox-new homogeneous padding widgets))

(defmacro-public gnome-boxed-widget-internal (expand fill padding widget)
  `(list 'boxed-widget ,widget ,expand ,fill ,padding))

(defmacro-public gnome-boxed-widget args
  (if (boolean? (car args))
      `(gnome-boxed-widget-internal ,@args)
      `(gnome-boxed-widget-internal #f #f 0 ,@args)))

(define-public (make-gnome-window title menu-def make-contents make-statusline)

  (define window (gtk-window-new 'toplevel))
  (define main-vbox (gtk-vbox-new #f 0))
  (define menubar (gtk-menu-bar-new))
  (define contents-frame (gtk-frame-new #f))
  (define statusbar-separator (gtk-hseparator-new))
  (define statusbar-hbox (gtk-hbox-new #f 0))
  
  ;;
  ;; Create a menu from a menu-definition
  ;;
  (define (make-menu! menubar menu-def)
    (defmacro nil-if-exception (expr)
      `(catch #t (lambda () ,expr)
	      (lambda args '())))
    (define (cadr* l)
      (nil-if-exception (cadr l)))
    (for-each 
     (lambda (menu)
       (let ((menuitem (gtk-menu-item-new-with-label (car menu)))
	     (submenu (gtk-menu-new))
	     (items (map (lambda (item action)
			   (let ((menuitem 
				  (gtk-menu-item-new-with-label item)))
			     (gtk-signal-connect 
			      menuitem "activate" 
			      (if (null? action)
				  (lambda () 
				    (display "undefined!\n"))
				  action))
			     menuitem))
			 (map car (cdr menu))
			 (map cadr* (cdr menu)))))
	 (for-each (lambda (item) (gtk-menu-append submenu item)) items)
	 (for-each (lambda (item) (gtk-widget-show item)) items)
	 (gtk-menu-item-set-submenu menuitem submenu)
	 (gtk-menu-bar-append menubar menuitem)
	 (gtk-widget-show menuitem)))
     menu-def))

  ; init the main window
  (gtk-window-set-title window title)
  (gtk-container-border-width window 0)
  (gtk-container-add window main-vbox)
  (gtk-widget-show main-vbox)
    
  ; add the menubar and the menus
  (gtk-box-pack-start main-vbox menubar #f #t 0)
  (gtk-widget-show menubar)
  (make-menu! menubar menu-definition)
  
  ; create a container with a border for the contents
  (gtk-frame-set-shadow-type contents-frame 'none)
  (gtk-container-border-width contents-frame 5)
  (gtk-box-pack-start main-vbox contents-frame #t #t 0)
  (gtk-widget-show contents-frame)

  ; create the contents of the main windows
  (make-contents contents-frame)

  ; create a seperation line for the status bar
  (gtk-box-pack-start main-vbox statusbar-separator #f #t 0)
  (gtk-widget-show statusbar-separator)

  ; create the hbox for the status bar
  (gtk-box-pack-start main-vbox statusbar-hbox #f #t 0)
  (gtk-widget-show statusbar-hbox)

  ; create the statubar
  (make-statusline statusbar-hbox)

  ; show me what i want
  (gtk-widget-show window)

  (define (self . args)
    (if (null? args)
	(throw 'wrong-number-of-args)
	(case (car args)
	  ((get-window) window)
	  ((get-menubar) menubar)
	  ((get-contents-frame) contents-frame)
	  ((get-statusbar-hbox) statusbar-hbox)
	  )))
  self)

;;
;; Read a file and display it in a window. Use this to show the GPL:
;;
;; (gnome-show-file "COPYING" "GNU Public License"
;;                  "-*-lucidatypewriter-medium-*-*-*-12-*-*-*-*-*-*-*"
;;                  600 800)
;;
(define-public (gnome-show-file file title font width height)
  (let* ((window (gtk-window-new 'toplevel))
         (vbox (gtk-vbox-new #f 0))
         (table (gtk-table-new 2 2 #f))
         (vadj (gtk-adjustment-new 0.0 0.0 101.0 0.1 1.0 1.0))
         (text (gtk-text-new #f vadj))
         (vscrollbar (gtk-vscrollbar-new vadj))
         (hbox (gtk-hbox-new #f 0))
         (close-button (gtk-button-new-with-label "Close"))
         (port (open-input-file file)))

    (letrec ((insert-file (lambda ()
                            (let ((line (read-line port 'split)))
                              (if (not (eof-object? (cdr line)))
                                  (let ((str (string-append (car line) "\n")))
                                    (gtk-text-insert text font #f #f str -1)
                                    (insert-file))
                                  (close-input-port port))))))

      ; create window and vbox
      (gtk-window-set-title window title)
      (gtk-widget-set-usize window width height)
      (gtk-container-add window vbox)
      (gtk-widget-show vbox)

      ; create table
      (gtk-table-set-row-spacing table 0 2)
      (gtk-table-set-col-spacing table 0 2)
      (gtk-box-pack-start vbox table #t #t 0)
      (gtk-widget-show table)

      ; attache text and scrollbar
      (gtk-table-attach-defaults table text 0 1 0 1)
      (gtk-widget-show text)
      (gtk-table-attach table vscrollbar 1 2 0 1
                        '(fill) '(expand fill) 0 0)
      (gtk-widget-show vscrollbar)

      ; insert text file
      (gtk-text-freeze text)
      (gtk-widget-realize text)
      (insert-file)
      (gtk-text-thaw text)

      ; create close button
      (gtk-box-pack-start vbox hbox #f #f 10)
      (gtk-widget-show hbox)
      (gtk-box-pack-start hbox close-button #f #f 10)
      (gtk-signal-connect close-button "clicked"
                          (lambda () (gtk-widget-destroy window)))
      (gtk-widget-show close-button)

      ; show it
      (gtk-widget-show window))))

(or (feature? 'gettext)
    (begin
      ;; Usage: (gettext String ?Domain? ?Category?)
      ;; Optional args turn it into a call to dgettext or dcgettext.
      (define (gettext string . args) string)
      ;; Usage just like the C function.
      (define (textdomain string) string)
      ;; Usage like the C function; if the second arg is not given
      ;; then NULL is passed to bindtextdomain().
      (define (bindtextdomain string . args) string)))


;;; gnome-make-expander-button name state . widgets
;;;   name is the name to use for the label 
;;;     (" >>" or " <<" will be appended depending on the state)
;;;   state is the initial state, #f for collapsed, #t for expanded
;;;   widgets is the list of widgets initially under the expander's
;;;   control.
;;;
;;;   Creates an "expander button", a button that makes some widgets
;;;   show or hide in the current dialog. It is very useful for making
;;;   expandable dialogs that show some basic options by default but
;;;   can be expanded to show more. The object returned can be treated
;;;   as a gtk-button in all respects, but some extra procedures
;;;   (below) work on it.

(define-public (gnome-make-expander-button name state . widgets)
  (let* ((state #f)
	 (expand-name (string-append name " >>"))
	 (collapse-name (string-append name " <<"))
	 (label (gtk-label-new expand-name))
	 (button (gtk-button-new))
	 (set-state! (lambda (new-state)
		       (cond
			(new-state
			 (gtk-label-set label collapse-name)
			 (map gtk-widget-show widgets))
			(else 
			 (gtk-label-set label expand-name)
			 (map gtk-widget-hide widgets)))
		       (set! state new-state)))
	 (handler (lambda ()
		    (set-state! (not state))))
	 )
    
    (gtk-container-add button label)
    (gtk-widget-show label)
    
    (set-state! state)

    (gtk-signal-connect button "clicked" handler)

    (set-object-property! button 'gnome-expander-button? #t)
    (set-object-property! button 'gnome-expander-button-get-state
			  (lambda () state))
    (set-object-property! button 'gnome-expander-button-set-state
			  set-state!)
    (set-object-property! button 'gnome-expander-button-add-widgets
			  (lambda (ws)
			    (map
			     (lambda (widget)
			       (cond 
				((not (memq widget widgets))
				 (set! widgets (cons widget widgets))
				 (if state
				     (gtk-widget-show widget)
				     (gtk-widget-hide widget)))))
			     ws)))
    (set-object-property! button 'gnome-expander-button-remove-widgets
			  (lambda (ws)
			    (map
			     (lambda (widget)
			       (if (not (memq widget widgets))
				   (set! widgets (delq! widget widgets))))
			     ws)))
    button))

;;; gnome-expander-button? object
;;;   true if object is a gnome-expander-button (gtk-button? should also
;;;   be true of it).

(define-public (gnome-expander-button? object)
  (object-property object 'gnome-expander-button?))

;;; gnome-expander-button-get-state expander-button
;;;   retrieves the current state of the given expander button

(define-public (gnome-expander-button-get-state expander-button)
  (cond
   ((object-property expander-button 'gnome-expander-button-get-state)
    => (lambda (proc) (proc)))
   (else (error 'wrong-type-arg expander-button))))

;;; gnome-expander-button-set-state expander-button new-state
;;;   sets the state of the given expander button to new-state

(define-public (gnome-expander-button-set-state expander-button new-state)
  (cond
   ((object-property expander-button 'gnome-expander-button-set-state)
    => (lambda (proc) (proc new-state)))
   (else (error 'wrong-type-arg expander-button))))

;;; gnome-expander-button-add-widgets expander-button . widgets
;;;   adds wdigets to the control of the expander-button. The
;;;   widgets are hidden or shown to match the current state.

(define-public (gnome-expander-button-add-widgets expander-button . widgets)
  (cond
   ((object-property expander-button 'gnome-expander-button-add-widgets)
    => (lambda (proc) (proc widgets)))
   (else (error 'wrong-type-arg expander-button))))

;;; gnome-expander-button-remove-widgets expander-button . widgets
;;;   removes wdigets from the control of the expander-button. The
;;;   widgets remains in the hide/show state they were in until they are
;;;   explicitly hidden or shown.

(define-public (gnome-expander-button-remove-widgets expander-button . widgets)
  (cond
   ((object-property expander-button 'gnome-expander-button-remove-widgets)
    => (lambda (proc) (proc widgets)))
   (else (error 'wrong-type-arg expander-button))))

;;; UIInfo translator

;; Simple minded translation.  Everything is turned into a
;; configurable item.

(gnome-uiinfo-set-translator
 (lambda (item)
   (let ((defkey (let ((first-val (cadr item)))
		   (if (or (symbol? first-val) (keyword? first-val))
		       #f
		       (if (eq? (car item) 'new)
			   #:label
			   #:callback)))))
     `(configurable #:subtype ,(car item)
		    ,@(if defkey 
			  (cons defkey (cdr item))
			  (cdr item))))))
