#! /bin/sh
exec guile-gtk -e main -s $0 $*
!#
;; Time-stamp: <1998-03-15 21:34:42 szi>
;;
;; Copyright (C) 1997, 1998, 1999 Marius Vollmer
;; 
;; Test-Gtk for Guile-Gtk
;;
;; This program should do the same like the test-gtk program, which
;; comes with Gtk.
;;

(use-modules (gtk gtk)
	     (gtk gdk))

(if (not (defined? 'gtk-scrolled-window-add-with-viewport))
    (define gtk-scrolled-window-add-with-viewport gtk-container-add))

(define call/cc call-with-current-continuation)

(define-macro (with-return . forms)
  `(call/cc (lambda (return) ,@forms)))

(define (gtk-widget-visible? w)
  (memq 'visible (gtk-widget-flags w)))

(define (make-dialog-creator inner-creator)
  (let ((window #f))
    (lambda ()
      (if window
	  (pk (gtk-widget-flags window)))
      (cond ((not window)
	     (set! window (inner-creator))
	     (gtk-signal-connect window "destroy" 
				 (lambda () (set! window #f)))))
      (if (gtk-widget-visible? window)
	  (gtk-widget-destroy window)
	  (gtk-widget-show window)))))

(define (make-creator title inner-creator)
  (define (outer-creator)
    (let ((window (gtk-window-new 'toplevel))
	  (box1 (gtk-vbox-new #f 0))
	  (box2 (gtk-hbox-new #f 10))
	  (box3 (gtk-vbox-new #f 10))
	  (separator (gtk-hseparator-new))
	  (close (gtk-button-new-with-label "close"))
	  (buttons '()))
      (gtk-window-set-title window title)
      (gtk-container-border-width window 0)
      (gtk-container-add window box1)
      (gtk-widget-show box1)

      (gtk-box-pack-start box1 box3 #t #t 0)
      (gtk-container-border-width box3 20)
      (gtk-widget-show box3)
      (set! buttons (inner-creator box3))
	       
      (gtk-box-pack-start box1 separator #f #t 0)
      (gtk-widget-show separator)
      (gtk-container-border-width box2 10)
      (gtk-box-pack-start box1 box2 #f #t 0)
      (gtk-widget-show box2)
      (gtk-signal-connect close "clicked" 
			  (lambda () (gtk-widget-destroy window)))
      (gtk-box-pack-start box2 close #t #t 0)
      (gtk-widget-set-flags close '(can-default))
      (gtk-widget-grab-default close)
      (gtk-widget-show close)
      (for-each (lambda (bt)
		  (let ((b (gtk-button-new-with-label (car bt))))
		    ; (gtk-widget-set-flags b '(can-default))
		    (gtk-box-pack-start box2 b #t #t 0)
		    (gtk-signal-connect b "clicked" (cadr bt))
		    (gtk-widget-show b)))
		buttons)
      window))
  (make-dialog-creator outer-creator))

(define-macro (define-test sym title . forms)
  `(define ,sym (make-creator ,title 
			      (lambda (outer-box) ,@forms))))

(define-macro (define-dialog-test sym . forms)
  `(define ,sym (make-dialog-creator (lambda () ,@forms))))

(define-test create-button-box "button box"
  (let* ((frame-horz (gtk-frame-new "Horizontal Button Boxes"))
	 (frame-vert (gtk-frame-new "Vertical Button Boxes"))
 	 (vbox (gtk-vbox-new #f 0))
 	 (hbox (gtk-hbox-new #f 0)))

    (define (create-bbox horizontal title spacing child_w child_h layout)
      (let* ((frame (gtk-frame-new title))
	     (bbox (if horizontal 
		       (gtk-hbutton-box-new)
		       (gtk-vbutton-box-new))))
	(gtk-container-border-width bbox 5)
	(gtk-container-add frame bbox)
	(gtk-button-box-set-layout bbox layout)
	(gtk-button-box-set-spacing bbox spacing)
	(gtk-button-box-set-child-size bbox child_w child_h)
	(gtk-container-add bbox (gtk-button-new-with-label "OK"))
	(gtk-container-add bbox (gtk-button-new-with-label "Cancel"))
	(gtk-container-add bbox (gtk-button-new-with-label "Help"))
	frame))
	     		   
    (gtk-box-pack-start outer-box frame-horz #t #t 10)
    (gtk-container-add frame-horz vbox)
    (gtk-container-border-width vbox 10)
    (gtk-box-pack-start vbox (create-bbox #t "Spread" 40 85 20 'spread) #t #t 0)
    (gtk-box-pack-start vbox (create-bbox #t "Edge" 40 85 20 'edge) #t #t 5)
    (gtk-box-pack-start vbox (create-bbox #t "Start" 40 85 20 'start) #t #t 5)
    (gtk-box-pack-start vbox (create-bbox #t "End" 40 85 20 'end) #t #t 5)
    (gtk-widget-show-all frame-horz)

    (gtk-box-pack-start outer-box frame-vert #t #t 10)
    (gtk-container-add frame-vert hbox)
    (gtk-container-border-width hbox 10)
    (gtk-box-pack-start hbox (create-bbox #f "Spread" 30 85 20 'spread) #t #t 0)
    (gtk-box-pack-start hbox (create-bbox #f "Edge" 30 85 20 'edge) #t #t 5)
    (gtk-box-pack-start hbox (create-bbox #f "Start" 30 85 20 'start) #t #t 5)
    (gtk-box-pack-start hbox (create-bbox #f "End" 30 85 20 'end) #t #t 5)
    (gtk-widget-show-all frame-vert)
    '()))

(define-test create-clist "clist"
  (let* ((titles #("auto resize" "not resizeable" "max width 100" "min width 50"
		   "hide column" "Title 5" "Title 6" "Title 7" "Title 8"  "Title 9"
		   "Title 10" "Title 11" "Title 12" "Title 13" "Title 14" "Title 15"
		   "Title 16" "Title 17" "Title 18" "Title 19"))
	 (box1 (gtk-vbox-new #f 0))
	 (box2 (gtk-hbox-new #f 10))
	 (clist (gtk-clist-new-with-titles titles))
	 (scrolled-win (gtk-scrolled-window-new))
	 (button (gtk-button-new-with-label "Add 1,000 Rows With Pixmaps"))
	 (menu (gtk-menu-new))
	 (omenu (gtk-option-menu-new))
	 (sort-column 0)
	 (sort-ascending #t))

    (gtk-container-add outer-box box1)
    (gtk-container-border-width box2 10)
    (gtk-box-pack-start box1 box2 #f #f 0)
    (gtk-scrolled-window-set-policy scrolled-win 'automatic 'automatic)
    (gtk-container-add scrolled-win clist)
    (gtk-signal-connect clist "click_column" 
			(lambda (column) 
			  (if (= column 4) 
			      (gtk-clist-set-column-visibility clist column #f)
			      (if (= column sort-column)
				  (begin 
				    (set! sort-ascending (not sort-ascending))
				    (if sort-ascending
					(gtk-clist-set-sort-type clist 'ascending)
					(gtk-clist-set-sort-type clist 'descending))
				    (gtk-clist-sort clist))
				  (begin
				    (set! sort-column column)
				    (gtk-clist-set-sort-column clist sort-column))))))

    (gtk-box-pack-start box2 button #t #t 0)
    (gtk-widget-set-sensitive button #f) ;needs gtk-clist-set-pixtext
				      
    (set! button (gtk-button-new-with-label "Add 10,000 Rows"))
    (gtk-box-pack-start box2 button #t #t 0)
    (gtk-signal-connect button "clicked" 
			(lambda ()
			  (let ((x #("CListRow 0" "Right" "Center" "Column 3" 
				     "Column 4" "Column 5" "Column 6" "Column 7" 
				     "Column 8" "Column 9" "Column 10" "Column 11" 
				     "Column 12" "Column 13" "Column 14" "Column 15" 
				     "Column 16" "Column 17" "Column 18" "Column 19")))
			    (gtk-clist-freeze clist)
			       (do ((i 0 (1+ i))) ((= i 10000))
				 (vector-set! x 0 
					      (string-append 
					       "CListRow " (number->string i)))
				 (gtk-clist-append clist x))

			       (gtk-clist-thaw clist))))

    (set! button (gtk-button-new-with-label "Clear List"))
    (gtk-box-pack-start box2 button #t #t 0)
    (gtk-signal-connect button "clicked" (lambda () (gtk-clist-clear clist)))

    (set! button (gtk-button-new-with-label "Remove Row"))
    (gtk-box-pack-start box2 button #t #t 0)
    (gtk-signal-connect button "clicked" ;needs access to focus_row
			(lambda () (gtk-clist-remove clist 0)))

    (set! box2 (gtk-hbox-new #f 10))
    (gtk-box-pack-start box1 box2 #f #f 0)
    (gtk-container-border-width box2 10)
    
    (set! button (gtk-button-new-with-label "Insert Row"))
    (gtk-box-pack-start box2 button #t #t 0)
    (gtk-signal-connect button "clicked" ;needs access to focus_row  
			(lambda () (gtk-clist-prepend clist 
					     #("This" "is" "an" "inserted" "row" 
					       "This" "is" "an" "inserted" "row" 
					       "This" "is" "an" "inserted" "row" 
					       "This" "is" "an" "inserted" "row" ))))

    (set! button (gtk-button-new-with-label "Show Title Buttons"))
    (gtk-box-pack-start box2 button #t #t 0)
    (gtk-signal-connect button "clicked" 
			(lambda () (gtk-clist-column-titles-show clist)))

    (set! button (gtk-button-new-with-label "Hide Title Buttons"))
    (gtk-box-pack-start box2 button #t #t 0)
    (gtk-signal-connect button "clicked" 
			(lambda () (gtk-clist-column-titles-hide clist)))

    (set! button (gtk-button-new-with-label "Warning Test"))
    (gtk-box-pack-start box2 button #t #t 0)
    (gtk-widget-set-sensitive button #f) ;no idea

    (set! box2 (gtk-hbox-new #f 10))
    (gtk-box-pack-start box1 box2 #f #f 0)
    (gtk-container-border-width box2 10)

    (set! button (gtk-button-new-with-label "Undo last selection"))
    (gtk-box-pack-start box2 button #t #t 0)
    (gtk-signal-connect button "clicked" 
			(lambda () (gtk-clist-undo-selection clist)))
    (gtk-box-pack-start box2 (gtk-label-new "Selection Mode :") #f #f 0)

    (set! button (gtk-radio-menu-item-new-with-label-from-widget #f "Single"))
    (gtk-menu-append menu button)
    (gtk-widget-show button)
    (gtk-signal-connect button "activate" 
			(lambda () (gtk-clist-set-selection-mode clist 'single)))
    (set! button (gtk-radio-menu-item-new-with-label-from-widget button "Browse"))
    (gtk-menu-append menu button)
    (gtk-widget-show button)
    (gtk-signal-connect button "activate"
			(lambda () (gtk-clist-set-selection-mode clist 'browse)))
    (set! button (gtk-radio-menu-item-new-with-label-from-widget button "Multiple"))
    (gtk-menu-append menu button)
    (gtk-widget-show button)
    (gtk-signal-connect button "activate"
			(lambda () (gtk-clist-set-selection-mode clist 'multiple)))
    (set! button (gtk-radio-menu-item-new-with-label-from-widget button "Extended"))
    (gtk-menu-append menu button)
    (gtk-widget-show button)
    (gtk-signal-connect button "activate"
			(lambda () (gtk-clist-set-selection-mode clist 'extended)))
    (gtk-option-menu-set-menu omenu menu)
    (gtk-box-pack-start box2 omenu #f #f 0)

    (set! box2 (gtk-hbox-new #f 10))
    (gtk-container-border-width box2 10)
    (gtk-box-pack-start box1 box2 #t #t 0)
    (gtk-clist-set-row-height clist 18)
    (gtk-widget-set-usize clist -1 300)
    (do ((i 0 (1+ i))) ((= i 20)) 
      (gtk-clist-set-column-width clist i 80))
    (gtk-clist-set-column-auto-resize clist 0 #t)
    (gtk-clist-set-column-resizeable clist 1 #f)
    (gtk-clist-set-column-max-width clist 2 100)
    (gtk-clist-set-column-min-width clist 3 50)
    (gtk-clist-set-selection-mode clist 'extended)
    (gtk-clist-set-column-justification clist 1 'right)
    (gtk-clist-set-column-justification clist 2 'center)
    (let ((x #("CListRow 0" "Right" "Center" "Column 3" "Column 4"
				"Column 5" "Column 6" "Column 7" "Column 8" "Column 9" 
				"Column 10" "Column 11" "Column 12" "Column 13" 
				"Column 14" "Column 15" "Column 16" "Column 17" 
				"Column 18" "Column 19")) )
    (do ((i 0 (1+ i))) ((= i 10))
      (vector-set! x 0 (string-append "CListRow " (number->string i)))
      (gtk-clist-append clist x)) )

    (gtk-container-border-width scrolled-win 5)
    (gtk-box-pack-start box2 scrolled-win #t #t 0)
    (gtk-widget-show-all box1)
    '()))
	 
(define-test create-buttons "buttons"
  (let*                  ; name      x y clicked
      ((button-template '(("button1" 0 0 1)
			  ("button2" 1 1 2)
			  ("button3" 2 2 3)
			  ("button4" 0 2 4)
			  ("button5" 2 0 5)
			  ("button6" 1 2 6)
			  ("button7" 1 0 7)
			  ("button8" 2 1 8)
			  ("button9" 0 1 0)))
	(toggle-button (lambda (b)
			 (if (gtk-widget-visible? b)
			     (gtk-widget-hide b)
			     (gtk-widget-show b))
			 (if break-iteration (break-iteration #f))))

	(table (gtk-table-new 3 3 #f))
	(buttons (map (lambda (bt)
			(gtk-button-new-with-label (car bt)))
		      button-template)))

    (gtk-table-set-row-spacings table 5)
    (gtk-table-set-col-spacings table 5)
    (gtk-container-border-width table 0)
    (gtk-box-pack-start outer-box table #t #t 0)
    (gtk-widget-show table)
    
    (for-each 
     (lambda (b bt)
       (let ((x (cadr bt))
	     (y (caddr bt))
	     (clicked (list-ref buttons (cadddr bt))))
	 (gtk-signal-connect b "clicked" 
			     (lambda () (toggle-button clicked)))
	 ;; (gtk-signal-connect b "enter_notify_event"
	 ;;		     (lambda (ev) (pk 'enter ev) #f))
	 (gtk-table-attach table b x (+ 1 x) y (+ 1 y)
			   '(expand fill) '(expand fill)
			   0 0)
	 (gtk-widget-show b)))
     buttons button-template)

    '()))

(define (make-three-buttons outer-box maker)
  (map (lambda (l)
	 (let ((b (maker l)))
	   (gtk-box-pack-start outer-box b #t #t 0)
	   (gtk-widget-show b)
	   b))
       '("button1" "button2" "button3")))
  
(define-test create-toggle-buttons "toggle buttons"
  (make-three-buttons outer-box gtk-toggle-button-new-with-label)
  '())

(define-test create-check-buttons "check buttons"
  (make-three-buttons outer-box gtk-check-button-new-with-label)
  '())

(define-test create-radio-buttons "radio buttons"
  (let ((group #f))
    (define (radio-maker label)
      (set! group (gtk-radio-button-new-with-label group label))
      group)
    (make-three-buttons outer-box radio-maker))
  '())

(define-test create-reparent "reparent"
  (let ((hbox (gtk-hbox-new #f 5))
	(label (gtk-label-new "Hello World")))
    (define (make-frame title init-label)
      (let ((frame (gtk-frame-new title))
	    (box (gtk-vbox-new #f 5))
	    (button (gtk-button-new-with-label "switch")))
	(gtk-box-pack-start hbox frame #t #t 0)
	(gtk-widget-show frame)
	(gtk-container-border-width box 5)
	(gtk-container-add frame box)
	(gtk-widget-show box)
	(gtk-signal-connect button "clicked" 
			    (lambda () (gtk-widget-reparent label box)))
	(gtk-box-pack-start box button #f #t 0)
	(gtk-widget-show button)
	(cond (init-label
	       (gtk-box-pack-start box label #f #t 0)
	       (gtk-widget-show label)))))

    (gtk-box-pack-start outer-box hbox #t #t 0)
    (gtk-widget-show hbox)
    (make-frame "Frame 1" #t)
    (make-frame "Frame 2" #f)
    '()))

(define-test create-pixmap "pixmap"
  (let* ((button (gtk-button-new))
	 (pixmap (gtk-pixmap-new "test.xpm" button))
	 (label (gtk-label-new "Pixmap test"))
	 (hbox (gtk-hbox-new #f 0)))
    (gtk-box-pack-start outer-box button #f #f 0)
    (gtk-widget-show button)
    (gtk-container-border-width hbox 2)
    (gtk-container-add hbox pixmap)
    (gtk-container-add hbox label)
    (gtk-container-add button hbox)
    (gtk-widget-show pixmap)
    (gtk-widget-show label)
    (gtk-widget-show hbox)
    '()))

(define tooltips #f)

(define-test create-tooltips "tooltips"
  (if (not tooltips)
      (set! tooltips (gtk-tooltips-new)))
  (let ((buttons  (make-three-buttons outer-box
				      gtk-toggle-button-new-with-label)))
    (for-each (lambda (b tip)
		(gtk-tooltips-set-tip tooltips b tip ""))
	      buttons
	      '("This is button 1"
		"This is button 2"
		"This is button 3. This is also a really long tooltip which probably won't fit on a single line and will therefore need to be wrapped. Hopefully the wrapping will work correctly."))
    '()))
	
(define (make-menu depth)
  (if (< depth 1)
      #f
      (let ((menu (gtk-menu-new))
	    (submenu (make-menu (1- depth)))
	    (group #f))
	(do ((i 0 (1+ i))
	     (j 1 (1+ j)))
	    ((= i 6))
	  (let ((menuitem (gtk-radio-menu-item-new-with-label group
			   (string-append "item " (number->string depth)
					  " - " (number->string j)))))
	    (set! group menuitem)
	    (gtk-menu-append menu menuitem)
	    (gtk-widget-show menuitem)
	    (if submenu
		(gtk-menu-item-set-submenu menuitem submenu))))
	menu)))
      
(define-test create-menus "menus"
  (let ((menubar (gtk-menu-bar-new))
	(menu (make-menu 2))
	(optionmenu (gtk-option-menu-new))
	(fixed (gtk-fixed-new)))
    (gtk-box-pack-start outer-box menubar #f #t 0)
    (gtk-widget-show menubar)
    (for-each (lambda (label)
		(let ((menuitem (gtk-menu-item-new-with-label label)))
		  (gtk-menu-item-set-submenu menuitem menu)
		  (gtk-menu-bar-append menubar menuitem)
		  (gtk-widget-show menuitem)))
	      '("test" "foo" "bar"))
    (gtk-option-menu-set-menu optionmenu (make-menu 1))
    (gtk-option-menu-set-history optionmenu 4)
    (gtk-box-pack-start outer-box optionmenu #t #t 0)
    (gtk-widget-show optionmenu)

    (gtk-widget-set-usize fixed 30 20)
    (gtk-signal-connect fixed "button_press_event"
                      (lambda (e)
                        (if (= (gdk-event-button e) 1)
                            (gtk-menu-popup menu #f #f
                                            (gdk-event-button e)
                                            (gdk-event-time e)))))
    (gtk-box-pack-start outer-box fixed #t #t 0)
    (gtk-widget-show fixed)

    '()))

(define-test create-scrolled-windows "scrolled windows"
  (let* ((scrolled-win (gtk-scrolled-window-new))
	 (viewport (gtk-viewport-new
		    (gtk-scrolled-window-get-hadjustment scrolled-win)
		    (gtk-scrolled-window-get-vadjustment scrolled-win)))
	 (table (gtk-table-new 20 20 #f))
	 (button #f))
    (gtk-container-border-width outer-box 0)
    (gtk-container-border-width scrolled-win 10)
    (gtk-scrolled-window-set-policy scrolled-win 'automatic 'automatic)
    (gtk-box-pack-start outer-box scrolled-win #t #t 0)
    (gtk-table-set-row-spacings table 10)
    (gtk-table-set-col-spacings table 10)
    (gtk-scrolled-window-add-with-viewport scrolled-win table)
    ;;(gtk-container-set-focus-hadjustment 
    ;;  table (gtk-scrolled-window-get-hadjustment scrolled-win))
    ;;(gtk-container-set-focus-vadjustment 
    ;;  table (gtk-scrolled-window-get-vadjustment scrolled-win))
    (do ((i 0 (1+ i))) ((= i 20))
      (do ((j 0 (1+ j))) ((= j 20))
	(set! button (gtk-button-new-with-label 
		      (string-append "button (" (number->string i) ","
				     (number->string j) ")" (string #\newline))))
	(gtk-table-attach-defaults table button i (1+ i) j (1+ j))))
    (gtk-widget-show-all scrolled-win)
    '()))

(define-test create-entry "entry"
  (let* ((box1 (gtk-vbox-new #f 0))
	 (box2 (gtk-vbox-new #f 10))
	 (entry (gtk-entry-new))
	 (cb (gtk-combo-new))
	 (check1 #f)
	 (check2 #f)
	 (check3 #f))
    (gtk-container-add outer-box box1)
    (gtk-container-border-width box2 10)
    (gtk-box-pack-start box1 box2 #t #t 0)
    (gtk-entry-set-text entry "hello world")
    (gtk-entry-select-region entry 0 5)
    (gtk-box-pack-start box2 entry #t #t 0)
    (gtk-combo-set-popdown-strings cb 
				  #("item0"
				    "item1 item1"
				    "item2 item2 item2"
				    "item3 item3 item3 item3"
				    "item4 item4 item4 item4 item4"
				    "item5 item5 item5 item5 item5 item5"
				    "item6 item6 item6 item6 item6"
				    "item7 item7 item7 item7"
				    "item8 item8 item8"
				    "item9 item9"))
    (gtk-entry-set-text (gtk-combo-entry cb) "hello world")
    (gtk-entry-select-region (gtk-combo-entry cb) 0 -1)
    (gtk-box-pack-start box2 cb #t #t 0)

    (set! check1 (gtk-check-button-new-with-label "Editable"))
    (gtk-box-pack-start box2 check1 #f #t 0)
    (gtk-signal-connect check1 "toggled"
			(lambda ()
			  (gtk-entry-set-editable 
			   entry (gtk-toggle-button-active check1))))
    (gtk-toggle-button-set-state check1 #t)

    (set! check2 (gtk-check-button-new-with-label "Visible"))
    (gtk-box-pack-start box2 check2 #f #t 0)
    (gtk-signal-connect check2 "toggled"
			(lambda ()
			  (gtk-entry-set-visibility 
			   entry (gtk-toggle-button-active check2))))
    (gtk-toggle-button-set-state check2 #t)

    (set! check3 (gtk-check-button-new-with-label "Sensitive"))
    (gtk-box-pack-start box2 check3 #f #t 0)
    (gtk-signal-connect check3 "toggled"
			(lambda ()
			  (gtk-widget-set-sensitive 
			   entry (gtk-toggle-button-active check3))))
    (gtk-toggle-button-set-state check3 #t)
    
    (gtk-widget-show-all box1)
    '()))

(define-test create-list "list"
  (let ((list-items '("hello" 
		      "world"
		      "blah"
		      "foo"
		      "bar"
		      "argh"
		      "spencer"
		      "is a"
		      "wussy"
		      "programmer"))
	(scrolled-win (gtk-scrolled-window-new))
	(lyst (gtk-list-new))
	(add (gtk-button-new-with-label "add"))
	(remove (gtk-button-new-with-label "remove")))
	     
    (gtk-scrolled-window-set-policy scrolled-win 'automatic 'automatic)
    (gtk-box-pack-start outer-box scrolled-win #t #t 0)
    (gtk-widget-show scrolled-win)

    (gtk-list-set-selection-mode lyst 'multiple)
    (gtk-list-set-selection-mode lyst 'browse)
    (gtk-container-add scrolled-win lyst)
    (gtk-widget-show lyst)

    (for-each (lambda (i)
		(let ((list-item (gtk-list-item-new-with-label i)))
		  (gtk-container-add lyst list-item)
		  (gtk-widget-show list-item)))
	      list-items)
    
    (gtk-signal-connect add "clicked" (lambda () (pk 'add)))
    (gtk-box-pack-start outer-box add #f #t 0)
    (gtk-widget-show add)

    (gtk-signal-connect remove "clicked" (lambda () (pk 'remove)))
    (gtk-box-pack-start outer-box remove #f #t 0)
    (gtk-widget-show remove)

    (gtk-signal-connect lyst "select_child" 
			(lambda (child)
			  (pk 'selected child
			      (gtk-list-child-position lyst child))))

    ; unselect_child is never emited...
    (gtk-signal-connect lyst "unselect_child" (lambda (child)
					      (pk 'unselected child)))

    '()))

(define-dialog-test create-color-selection
  (let ((window #f)
	(button #f))
    (gtk-preview-set-install-cmap #t)
    (gtk-widget-push-visual (gtk-preview-get-visual))
    (gtk-widget-push-colormap (gtk-preview-get-cmap))
    
    (set! window (gtk-color-selection-dialog-new
		  "color selection dialog"))
    ;; (let ((colorsel (gtk-color-selection-dialog-colorsel window)))
    ;; (gtk-signal-connect colorsel "color_changed" 
    ;;		  (lambda () (pk 'changed))))
    (set! button (gtk-button-new-with-label "Green"))
    (gtk-signal-connect 
     button "clicked"
     (lambda () (gtk-color-selection-set-color 
		 (gtk-color-selection-dialog-colorsel window)
		 (gdk-color-parse "green"))))
    (gtk-widget-show button)
    (gtk-box-pack-start (gtk-color-selection-dialog-main-vbox window) 
			button #f #f 0)
    (gtk-signal-connect
     (gtk-color-selection-dialog-ok-button window)
     "clicked" (lambda () 
		 (let ((color (gtk-color-selection-get-color
			       (gtk-color-selection-dialog-colorsel window))))
		   (pk (gdk-color-red color) 
		       (gdk-color-green color)
		       (gdk-color-blue color)))))
    (gtk-signal-connect 
     (gtk-color-selection-dialog-cancel-button window)
     "clicked" (lambda () (gtk-widget-destroy window)))
    
    (gtk-widget-pop-colormap)
    (gtk-widget-pop-visual)
    window))

(define-dialog-test create-font-selection 
  (let* ((window (gtk-font-selection-dialog-new "font selection dialog")))
    (gtk-font-selection-dialog-set-preview-text window "Set from Scheme!")
    (gtk-signal-connect 
     (gtk-font-selection-dialog-cancel-button window)
     "clicked" (lambda () (gtk-widget-destroy window)))
    (gtk-signal-connect
     (gtk-font-selection-dialog-ok-button window)
     "clicked"
     (lambda () (pk (gtk-font-selection-dialog-get-font-name window))))
    window))

(define-dialog-test create-file-selection 
  (let* ((window (gtk-file-selection-new "file selection"))
	 (button #f))
    (gtk-signal-connect
     (gtk-file-selection-ok-button window)
     "clicked" (lambda () (pk 'ok)))
    (gtk-signal-connect 
     (gtk-file-selection-cancel-button window)
     "clicked" (lambda () (gtk-widget-destroy window)))

    (gtk-file-selection-hide-fileop-buttons window)

    (set! button (gtk-button-new-with-label "Hide Fileops"))
    (gtk-signal-connect 
     button "clicked"
     (lambda () (gtk-file-selection-hide-fileop-buttons window)))
    (gtk-box-pack-start (gtk-file-selection-action-area window)
			button #f #f 0)
    (gtk-widget-show button)

    (set! button (gtk-button-new-with-label "Show Fileops"))
    (gtk-signal-connect 
     button "clicked"
     (lambda () (gtk-file-selection-show-fileop-buttons window)))
    (gtk-box-pack-start (gtk-file-selection-action-area window)
			button #f #f 0)
    (gtk-widget-show button)
    
    window))

(define-dialog-test create-dialog 
  (let* ((dialog (gtk-dialog-new))
	 (button #f)
	 (label #f))
    (gtk-window-set-title dialog "dialog")
    (gtk-container-border-width dialog 0)
    (gtk-widget-set-usize dialog 200 100)

    (set! button (gtk-button-new-with-label "OK"))
    (gtk-signal-connect 
     button "clicked" 
     (lambda () (gtk-widget-destroy dialog)))
    (gtk-widget-set-flags button '(can-default))
    (gtk-box-pack-start (gtk-dialog-action-area dialog)
			button #t #t 0)
    (gtk-widget-grab-default button)
    (gtk-widget-show button)

    (set! button (gtk-button-new-with-label "Toggle"))
    (gtk-signal-connect 
     button "clicked" 
     (lambda () 
       (if (not label)
	   (begin
	     (set! label (gtk-label-new "Dialog Test"))
	     (gtk-misc-set-padding label 10 10)
	     (gtk-box-pack-start (gtk-dialog-vbox dialog)
				 label #t #t 0)
	     (gtk-widget-show label))
	   (begin
	     (gtk-widget-destroy label)
	     (set! label #f)))))
    (gtk-widget-set-flags button '(can-default))
    (gtk-box-pack-start (gtk-dialog-action-area dialog)
			button #t #t 0)
    (gtk-widget-show button)
    dialog))

(define-test create-range-controls "range controls"
  (let* ((adjustment (gtk-adjustment-new 0.0 0.0 101.0 0.1 1.0 1.0))
	 (scale (gtk-hscale-new adjustment))
	 (scrollbar (gtk-hscrollbar-new adjustment)))

    (gtk-widget-set-usize scale 150 30)
    (gtk-range-set-update-policy scale 'delayed)
    (gtk-scale-set-digits scale 2)
    (gtk-scale-set-draw-value scale #t)
    (gtk-box-pack-start outer-box scale #t #t 0)
    (gtk-widget-show scale)

    (gtk-range-set-update-policy scrollbar 'continuous)
    (gtk-box-pack-start outer-box scrollbar #t #t 0)
    (gtk-widget-show scrollbar)

    '()))

(define-dialog-test create-rulers 
  (let* ((window (gtk-window-new 'toplevel))
	 (table (gtk-table-new 2 2 #f))
	 (hruler #f)
	 (vruler #f))
    (gtk-window-set-policy window #t #t #f)
    (gtk-window-set-title window "rulers")
    (gtk-widget-set-usize window 300 300)
    (gtk-widget-set-events window
			   '(pointer-motion-mask
			     pointer-motion-hint-mask))
    (gtk-container-border-width window 0)

    (gtk-container-add window table)
    (gtk-widget-show table)

    (set! hruler (gtk-hruler-new))
    (gtk-ruler-set-metric hruler 'centimeters)
    (gtk-ruler-set-range hruler 100 0 0 20)
    (gtk-table-attach table hruler 1 2 0 1 '(expand fill) '(fill) 0 0)
    (gtk-widget-show hruler)

    (set! vruler (gtk-vruler-new))
    (gtk-ruler-set-range vruler 5 15 0 20)
    (gtk-table-attach table vruler 0 1 1 2 '(fill) '(expand fill) 0 0)
    (gtk-widget-show vruler)

    (gtk-signal-connect 
     window "motion_notify_event"
     (lambda (ev) 
       (gtk-widget-event hruler ev)
       (gtk-widget-event vruler ev)))
    window))

(define-test create-text "text"
  (let* ((t (gtk-text-new #f #f)))
    (gtk-box-pack-start outer-box t #t #t 0)
    (gtk-widget-show t)

    (gtk-widget-realize t)
    (gtk-text-insert t #f "white" "green" "Hallo!\n" -1)
    (gtk-text-insert t #f "yellow" "red" "Huhu!\n" -1)
    (gtk-text-thaw t)

    '()))

(define-test create-notebook "notebook" 
  (let ((notebook (gtk-notebook-new)))
    (gtk-notebook-set-tab-pos notebook 'top)
    (gtk-box-pack-start outer-box notebook #t #t 0)
    (gtk-widget-show notebook)

    (do ((i 1 (1+ i)))
	((= i 6))
      (let* ((text (string-append "Page " (number->string i)))
	     (frame (gtk-frame-new text))
	     (label1 (gtk-label-new text))
	     (label2 (gtk-label-new text)))
	(gtk-container-border-width frame 10)
	(gtk-widget-set-usize frame 200 150)
	(gtk-widget-show frame)
	(gtk-container-add frame label1)
	(gtk-widget-show label1)
	(gtk-notebook-append-page notebook frame label2)))

    `(("next" ,(lambda () 
		 (gtk-notebook-next-page notebook)))
      ("prev" ,(lambda ()
		 (gtk-notebook-prev-page notebook)))
      ("rotate" ,(lambda ()
		   (gtk-notebook-set-tab-pos 
		    notebook
		    (cdr (assq (gtk-notebook-tab-pos notebook)
			       '((top . left)
				 (left . bottom)
				 (bottom . right)
				 (right . top))))))))))

(define-test create-progress-bar "progress"
  (let* ((timer #f)
	 (label (gtk-label-new "progress..."))
	 (pbar (gtk-progress-bar-new)))
    
    (define timeout
      (let ((val 0))
	(lambda ()
	  (set! val (+ 0.02 (if (>= val 1) 0 val)))
	  (gtk-progress-bar-update pbar val)
	  #t)))

    (gtk-signal-connect pbar "destroy" (lambda () 
					 (gtk-timeout-remove timer)
					 (set! timer #f)))

    (gtk-misc-set-alignment label 0 0.5)
    (gtk-box-pack-start outer-box label #f #t 0)
    (gtk-widget-show label)
    (gtk-widget-set-usize pbar 200 20)
    (gtk-box-pack-start outer-box pbar #t #t 0)
    (gtk-widget-show pbar)

    (set! timer (gtk-timeout-add 100 timeout))
    '()))

(define create-color-preview #f)
(define create-gray-preview #f)
(define-test create-gamma-curve "gamma curve"
  (let* ((curve (gtk-gamma-curve-new)))
    (gtk-container-add outer-box curve)
    (gtk-widget-show-all curve)
    (gtk-curve-set-range (gtk-gamma-curve-curve curve) 0 255 0 255)
    (gtk-curve-set-gamma (gtk-gamma-curve-curve curve) 2)
    '()))
    
(define (make-counter outer-box adder remover)
  (let ((tag #f)
	(count 0)
	(label (gtk-label-new "")))
    (define (set-label)
      (gtk-label-set label
		     (string-append "count: " (number->string count))))
    (define (tick)
      (set! count (1+ count))
      (set-label))
    (define (start)
      (if (not tag) (set! tag (adder tick))))
    (define (stop)
      (if tag (remover tag))
      (set! tag #f))
    (define (shot)
      (start)
      (gtk-timeout-add 1001 (lambda () (stop) #f)))
    (gtk-signal-connect label "destroy" stop)

    (set-label)
    (gtk-misc-set-padding label 10 10)
    (gtk-box-pack-start outer-box label #t #t 0)
    (gtk-widget-show label)

    `(("shot"  ,shot)
      ("start" ,start)
      ("stop"  ,stop))))

(define-test create-timeout-test "Timeout Test"
  (make-counter outer-box
		(lambda (p) (gtk-timeout-add 100 p)) gtk-timeout-remove))

(define-test create-idle-test "Idle Test"
  (make-counter outer-box gtk-idle-add gtk-idle-remove))

(define create-test #f)

(define (yes-or-no? question)
  (let ((d (gtk-dialog-new))
	(l (gtk-label-new question))
	(y (gtk-button-new-with-label "Yes"))
	(n (gtk-button-new-with-label "No"))
	(answer #f))

    (gtk-window-set-title d "Yes or No?")
    (gtk-misc-set-alignment l 0.5 0.5)
    (gtk-box-pack-start (gtk-dialog-vbox d) l #f #f 0)
    (gtk-box-pack-start (gtk-dialog-action-area d) y #t #t 3)
    (gtk-box-pack-start (gtk-dialog-action-area d) n #t #t 3)
    (gtk-widget-show-all d)

    (gtk-signal-connect d "delete_event"
			(lambda (ev)
			  (gtk-main-quit) #t))
    (gtk-signal-connect n "clicked"
			(lambda () 
			  (gtk-widget-destroy d)
			  (gtk-main-quit)))
    (gtk-signal-connect y "clicked"
			(lambda () 
			  (set! answer #t)
			  (gtk-widget-destroy d)
			  (gtk-main-quit)))

    (gtk-grab-add d)
    (gtk-main)
    (gtk-grab-remove d)

    answer))

(define (create-main-window)
  (define buttons
    `(("button box" ,create-button-box)
      ("buttons" ,create-buttons)
      ("toggle buttons" ,create-toggle-buttons)
      ("check buttons" ,create-check-buttons)
      ("radio buttons" ,create-radio-buttons)
      ("clist" ,create-clist)
      ("reparent" ,create-reparent)
      ("pixmap" ,create-pixmap)
      ("tooltips" ,create-tooltips)
      ("menus" ,create-menus)
      ("scrolled windows" ,create-scrolled-windows)
      ("drawing areas" #f)
      ("entry" ,create-entry)
      ("list" ,create-list)
      ("color selection" ,create-color-selection)
      ("font selection", create-font-selection)
      ("file selection" ,create-file-selection)
      ("dialog" ,create-dialog)
      ("miscellaneous" #f)
      ("range controls" ,create-range-controls)
      ("rulers" ,create-rulers)
      ("text" ,create-text)
      ("notebook" ,create-notebook)
      ("progress bar" ,create-progress-bar)
      ("preview color" ,create-color-preview)
      ("preview gray" ,create-gray-preview)
      ("gamma curve" ,create-gamma-curve)
      ("test timeout" ,create-timeout-test)
      ("test idle" ,create-idle-test)
      ("test" ,create-test)))

  (let* ((window (gtk-window-new 'toplevel))
	(box1   (gtk-vbox-new #f 0))
	(box2   (gtk-vbox-new #f 0))
	(label (gtk-label-new "Guile/Gtk+"))
	(scrolled-win (gtk-scrolled-window-new))
	(viewport (gtk-viewport-new 
		   (gtk-scrolled-window-get-hadjustment scrolled-win)
		   (gtk-scrolled-window-get-vadjustment scrolled-win)))
	(separator (gtk-hseparator-new))
	(box3   (gtk-vbox-new #f 10))
	(gc-but (gtk-button-new-with-label "gc"))
	(button (gtk-button-new-with-label "close")))
    
    (gtk-widget-set-name window "main window")
    (gtk-widget-set-usize window 200 400)
    (gtk-widget-set-uposition window 20 20)
    (gtk-container-add window box1)
    (gtk-box-pack-start box1 label #f #f 0)
    (gtk-container-border-width scrolled-win 10)
    (gtk-scrolled-window-set-policy scrolled-win 'automatic 'automatic)
    (gtk-box-pack-start box1 scrolled-win #t #t 0)
    (gtk-container-border-width box2 10)
    (gtk-scrolled-window-add-with-viewport scrolled-win box2)
    
    (for-each (lambda (b)
		(let ((button (gtk-button-new-with-label (car b))))
		  (if (cadr b)
		      (gtk-signal-connect button "clicked" (cadr b))
		      (gtk-widget-set-sensitive button #f))
		  (gtk-box-pack-start box2 button #t #t 0)))
	      buttons)
       
    (gtk-box-pack-start box1 separator #f #t 0)
    (gtk-container-border-width box3 10)
    (gtk-box-pack-start box1 box3 #f #t 0)
    (gtk-signal-connect gc-but "clicked" gc)
    (gtk-box-pack-start box3 gc-but #t #t 0)
    (gtk-signal-connect button "clicked" gtk-exit)
    (gtk-box-pack-start box3 button #t #t 0)
    (gtk-widget-set-flags button '(can-default))
    (gtk-widget-grab-default button)

    (gtk-signal-connect window "delete_event" 
			(lambda (ev) 
			  (if (yes-or-no? "Quit?")
			      (if (gtk-standalone?) 
				  (gtk-exit)
				  (gtk-widget-destroy window))
			      #t)))
    
    (gtk-widget-show-all window)))

(define break-iteration #f)
(define (breakable-iteration)
  (with-return
   (set! break-iteration return)
   (gtk-main-iteration)))

(gtk-rc-parse "testgtkrc")
(create-main-window)

(if (gtk-standalone?) (gtk-main))

; Local Variables:
; mode: scheme
; End:
