#!@GNOMEG@ -s
;; -*- scheme -*-
!#

;; Cromagnon: GNOME crontab manager
;; by Aldy Hernandez (aldy@uaa.edu)

(use-modules (gnome gnome) (gtk gtk))

(define-macro (gnome-pixmap pm)
  `(gnome-unconditional-pixmap-file ,pm))

(define-macro (cm-internal-clear-entries)
  `(gtk-list-clear-items cm-entries-list 0 (length crontab-entries)))

(define (gtk-widget-show-multi . list)
  (for-each gtk-widget-show list))

(define cm-main-scrolled-widget #f)

(define cm-entries-list '())

(define cm-toolbar #f)

;; 0 based integer representing current selected entry in list
(define cm-current-entry 0)

;;  [description, minute, hour, day, month day-of-week, command]
(define crontab-entries #f)

;; spacing width for each crontab entry (in points (whatever that is))
(define crontab-entries-spacing
  #(250 40 40 40 45 50 -1))

(define cm-modified? #f)

(define day-of-week
  '(("Sunday" "0")
    ("Monday" "1")
    ("Tuesday" "2")
    ("Wednesday" "3")
    ("Thursday" "4")
    ("Friday" "5")
    ("Saturday" "6")))

(define (nil-proc) #t)

(define (cm-internal-close-window x)
  #f)


;; toolbar routines

(define (gnome-create-toolbar upper-level . rest)
  (let ((hbox (gtk-hbox-new #f 0))
	(mode (if (not (null? rest))
		  (car rest)
		  #f)))
    (if mode
	(case mode
	  ((container-add) (gtk-container-add upper-level hbox))
	  ((pack-start) (gtk-box-pack-start upper-level hbox #f #f 0))
	  ((pack-end) (gtk-box-pack-end upper-level hbox #f #f 0)))
	(gtk-box-pack-start upper-level hbox #f #f 0)) ;; default

    (gtk-container-border-width hbox 2)
    (gtk-widget-show hbox)
    ;; a ``toolbar'' list includes (toolbar hbox list-of-items):
    ;;   the actual toolbar (an hbox), the parent hbox that contains the
    ;;   toolbar, and a list of toolbar items (label, pixmap, function)
    ;;
    ;; We need a separate hbox to embed the toolbar in, so we can
    ;; destroy and recreate the toolbar at will.
    (list #f hbox '())))

;; Destroy toolbar *permanently*
(define (gnome-destroy-toolbar toolbar)
  (gtk-widget-destroy (cadr toolbar))	; destroy hbox
  '())

(define (gnome-toolbar-add toolbar label pixmap func)
  (let ((toolbar-items (caddr toolbar)))
    ;; i'm sure append is not the best way to cons at the end VVVVVVV
    (set-car! (cddr toolbar) (append toolbar-items
				      (list (list label pixmap func))))))

;; This is what actually displays the toolbar buttons
(define (gnome-toolbar-style toolbar mode)
  ;; no sense changing style if toolbar has been destroyed
  (if (not (null? toolbar))
      (let ((m-icons (or (eq? mode 'icons) (eq? mode 'text-and-icons)))
	    (m-text  (or (eq? mode 'text) (eq? mode 'text-and-icons))))
	(if (and (car toolbar) ;; destroy old toolbar so we can redraw it
		 (not (gtk-object-destroyed (car toolbar))))
	    (gtk-widget-destroy (car toolbar)))
	(if (not (eq? mode 'nothing))
	    (begin
	      ;; make new toolbar
	      (set-car! toolbar (gtk-hbox-new #f 0))
	      (gtk-container-border-width (car toolbar) 3)
	      (gtk-box-pack-start (cadr toolbar) (car toolbar) #f #f 3)

	      (for-each (lambda (tb-entry)
			  (let* ((button (gtk-button-new))
				 (vbox (gtk-vbox-new #f 0))
				 (label (if (and m-text (car tb-entry))
					    (gtk-label-new (car tb-entry))
					    #f))
				 (pixmap (if (and m-icons (cadr tb-entry))
					     (gtk-pixmap-new (cadr tb-entry) button)
					     #f)))

			    ;; if programmer is an idiot...
			    (if (and (eq? mode 'icons) (not pixmap))
				(set! label (gtk-label-new "undefined")))

			    (gtk-container-add button vbox)
			    (gtk-container-border-width button 1)
			    (if pixmap
				(begin
				  (gtk-box-pack-start vbox pixmap #f #f 0)
				  (gtk-widget-show pixmap)))
			    (if label
				(begin
				  (gtk-box-pack-start vbox label #f #f 0)
				  (gtk-widget-show label)))
			    (gtk-box-pack-start (car toolbar) button #f #f 0)
			    (gtk-widget-show-multi button vbox)
			    (gtk-signal-connect button "clicked" (caddr tb-entry))))
			(caddr toolbar))
	      (gtk-widget-show (car toolbar)))))))

;; end of toolbar routines

(define (cm-refresh-entries-list)
  (for-each (lambda (x)
	      (let ((list-item (gtk-list-item-new))
		    (box (gtk-hbox-new #f 0)))

		(gtk-container-add list-item box)
		(gtk-widget-show box)

		;; pack each crontab subentry into an hbox inside a list
		(let ((c 0))
		  (for-each (lambda (field)
			      (let ((l (gtk-label-new
					(string-append "" field))))
;				(gtk-misc-set-alignment l 1.0 0)
;; why doesn't this align to the left or right??  ^^^^^^^^^^^^??
				(gtk-box-pack-start box l #f #f 0)
				(gtk-widget-show l)
				(gtk-widget-set-usize
				 l
				 (vector-ref crontab-entries-spacing c)
				 -1))
			      (set! c (1+ c)))
			    x))

		(gtk-container-add cm-entries-list list-item)
		(gtk-widget-show list-item)))
	    crontab-entries))

(define (cm-quit)
  (if cm-modified?
      (cm-are-you-sure "Quit without saving?" gtk-main-quit)
      (gtk-main-quit)))

(define (cm-add-other)
  (cm-internal-modify
   (list "" "" "" "" "" "" "") 'add-other))

(define (cm-save)
  (set! cm-modified? #f)
  (for-each (lambda (x)
	      (display (string-append "#" (car x) "\n"))
	      (for-each (lambda (x)
			  (display x)
			  (display " "))
			(cdr x))
	      (display "\n\n"))
	    crontab-entries))

(define (cm-delete)
  (cm-are-you-sure "Delete entry?" cm-internal-delete))

;; Delete current entry from list, and refresh screen
(define (cm-internal-delete)
  (let ((old-entry-num cm-current-entry))
    (set! cm-modified? #t)
    (cm-internal-clear-entries)
    (cm-delete-entry cm-current-entry)

    (cm-refresh-entries-list)
    (gtk-list-select-item cm-entries-list
			  (if (> old-entry-num 0)
			      (if (>= old-entry-num (length crontab-entries))
				  (1- old-entry-num)
				  0)
			      old-entry-num))
    ))

;; delete entry #n from crontab-entries
(define (cm-delete-entry n)
  ;; This whole func is ugly.  I really need to learn Scheme.
  (cond ((= 0 n)
	 (set! crontab-entries (cdr crontab-entries))) ;; delete head
	((= n (1- (length crontab-entries))) ;; delete tail
	 (set-cdr! (list-tail crontab-entries (1- n)) '()))
	(#t
	 (let* ((entry (list-tail crontab-entries n))
		(entry-cdr (cadr entry)))
	   (set-cdr! entry '())
	   (set-car! entry entry-cdr)))))

;; cm-internal-modify is a modal window because things get really hairy
;; if we try to modify things that are being deleted (or updated).
;; Basically I'm lazy.
(define (cm-internal-modify defaults mode)
  (let ((w (gtk-window-new 'toplevel))
	(vbox (gtk-vbox-new #f 10))
	(hbox (gtk-hbox-new #f 0))
	(separator (gtk-hseparator-new))
	(ok (gtk-button-new-with-label "  Ok  "))
	(cancel (gtk-button-new-with-label "Cancel"))
	(frame (gtk-frame-new "Modify"))
	(toolbar-frame (gtk-frame-new "Toolbar"))
	(toolbar '())
	(entry-desc (gtk-entry-new))
	(entry-min (gtk-entry-new))
	(entry-hour (gtk-entry-new))
	(entry-day (gtk-entry-new))
	(entry-month (gtk-entry-new))
	(entry-week-day (gtk-entry-new))
	(entry-command (gtk-entry-new)))
    (gtk-window-set-title w "Modify crontab entry")
    (gtk-signal-connect w "delete_event"
			(lambda (x)
			  (gtk-main-quit) #t))
    (gtk-container-border-width vbox 10)
    (gtk-container-add w vbox)

    ;; put toolbar in between two separators-- boy this is ugly
    (let ((s1 (gtk-hseparator-new))
	  (s2 (gtk-hseparator-new)))
      (gtk-box-pack-start vbox s1 #t #t 0)
      (set! toolbar (gnome-create-toolbar vbox 'container-add))
      (gnome-toolbar-add toolbar "Minute" (gnome-pixmap "gnome-cromagnon-small.xpm") nil-proc)
      (gnome-toolbar-add toolbar "Hourly" (gnome-pixmap "gnome-cromagnon-small.xpm") nil-proc)
      (gnome-toolbar-add toolbar "Daily" (gnome-pixmap "gnome-cromagnon-small.xpm") nil-proc)
      (gnome-toolbar-add toolbar "Weekly" (gnome-pixmap "gnome-cromagnon-small.xpm") nil-proc)
      (gnome-toolbar-add toolbar "Monthly" (gnome-pixmap "gnome-cromagnon-small.xpm") nil-proc)
      (gnome-toolbar-style toolbar 'text-and-icons)
      (gtk-box-pack-start vbox s2 #t #t 0)
      (gtk-widget-show-multi s1 s2))

    (gtk-box-pack-start vbox frame #f #t 0)

    ;; data entry frame
    (gtk-widget-set-usize entry-desc 250 -1)
    (gtk-widget-set-usize entry-min 50 -1)
    (gtk-widget-set-usize entry-hour 50 -1)
    (gtk-widget-set-usize entry-day 50 -1)
    (gtk-widget-set-usize entry-month 50 -1)
    (gtk-widget-set-usize entry-week-day 50 -1)
    (gtk-widget-set-usize entry-command -1 -1)
    (gtk-container-add
     frame
     (gnome-make-filled-vbox
      #t 3
      (gnome-boxed-widget
       #f #f 10
       (gnome-make-filled-hbox
	#f 3
	(gnome-boxed-widget (gtk-label-new "Description"))
	(gnome-boxed-widget #t #t 10 entry-desc)))
      (gnome-boxed-widget
       (gnome-make-filled-hbox
	#f 3
	(gnome-boxed-widget (gtk-label-new "Hour"))
	(gnome-boxed-widget entry-hour)
	(gnome-boxed-widget (gtk-label-new "Minute"))
	(gnome-boxed-widget entry-min)))
      (gnome-boxed-widget
       (gnome-make-filled-hbox
	#f 3
	(gnome-boxed-widget (gtk-label-new "Day"))
	(gnome-boxed-widget entry-day)
	(gnome-boxed-widget (gtk-label-new "Month  "))
	(gnome-boxed-widget entry-month)
	(gnome-boxed-widget (gtk-label-new "Week day"))
	(gnome-boxed-widget entry-week-day)))
      (gnome-boxed-widget
       #f #f 10
       (gnome-make-filled-hbox
	#f 3
	(gnome-boxed-widget (gtk-label-new "Command"))
	(gnome-boxed-widget #t #t 10 entry-command)))))

    ;; set defaults
    (map (lambda (entry text)
	   (gtk-entry-set-text entry (string-append "" text))
	   (gtk-entry-set-position entry 0))
	 (list entry-desc entry-min entry-hour entry-day entry-month entry-week-day entry-command) defaults)

    ;; ok/cancel section
    (gtk-box-pack-start vbox separator #f #t 0)
    (gtk-box-pack-start vbox hbox #f #t 0)
    (gtk-box-pack-start hbox ok #f #t 0)
    (gtk-box-pack-end hbox cancel #f #t 0)
    (gtk-signal-connect cancel "clicked"
			(lambda () (gtk-widget-destroy w)))
    (gtk-signal-connect ok "clicked"
			(lambda ()
			  (set! cm-modified? #t)
			  (let ((entry-backup cm-current-entry)
				(entry
				 (list (gtk-entry-get-text entry-desc)
				       (gtk-entry-get-text entry-min)
				       (gtk-entry-get-text entry-hour)
				       (gtk-entry-get-text entry-day)
				       (gtk-entry-get-text entry-month)
				       (gtk-entry-get-text entry-week-day)
				       (gtk-entry-get-text entry-command))))
			    (cond
			     ((eq? mode 'modify) ;; modify in-place
			      (begin
				(set-car! (list-tail crontab-entries
						     cm-current-entry) entry)
				(cm-internal-clear-entries)
				(cm-refresh-entries-list)
				(set! cm-current-entry entry-backup)))
			     ((eq? mode 'add-other) ;; append to entries list
			      (begin
				(cm-add-entry entry)
				(set! cm-current-entry
				      (1- (length crontab-entries))))))
			    (gtk-list-select-item cm-entries-list
						  cm-current-entry)
			    (gtk-widget-destroy w))))
    (gtk-widget-set-flags cancel '(can-default))
    (gtk-widget-grab-default cancel)

    (gtk-widget-show-multi w vbox frame ok cancel hbox toolbar-frame)
    (gtk-grab-add w)
    (gtk-main)
    (gtk-grab-remove w)))

(define (cm-are-you-sure mesg yes-function)
  (let ((w (gtk-dialog-new))
	(vbox (gtk-vbox-new #f 0))
	(hbox (gtk-hbox-new #f 0))
	(yes (gtk-button-new-with-label "Yes"))
	(no (gtk-button-new-with-label "No"))
	(label (gtk-label-new mesg)))
    (gtk-window-set-title w "Are you sure?")
    (gtk-widget-set-usize w 150 -1)
    (gtk-box-pack-start (gtk-dialog-vbox w) vbox #t #t 0)
    (gtk-container-border-width vbox 10)
    (gtk-box-pack-start vbox label #f #t 0)
    (gtk-box-pack-start (gtk-dialog-action-area w) yes #t #t 0)
    (gtk-box-pack-start (gtk-dialog-action-area w) no #t #t 0)
    (gtk-widget-set-flags no '(can-default))
    (gtk-widget-grab-default no)
    (gtk-signal-connect no "clicked"
			(lambda () (gtk-widget-destroy w)))
    (gtk-signal-connect yes "clicked"
			(lambda ()
			  (gtk-widget-destroy w)
			  (yes-function)))
    (gtk-widget-show-multi no yes label vbox w)))

;; insert crontab entry into current list position
(define (cm-add-entry entry)
  (set! crontab-entries (append crontab-entries (list entry)))
  (cm-internal-clear-entries)
  (cm-refresh-entries-list)
  (set! cm-current-entry (1- (length crontab-entries)))
  (gtk-list-select-item cm-entries-list cm-current-entry))

(define (cm-add type desc-pre desc-post)
  (let ((w (gtk-window-new 'toplevel))
	;; do we need to prompt for HH:MM??
	(need-hr-min? (member type '(days months day-of-month week-day)))
	(vbox (gtk-vbox-new #f 10))
	(frame-data-entry (gtk-frame-new "Add"))
	(entry-general (gtk-entry-new))
	(entry-command (gtk-entry-new))
	(entry-desc (gtk-entry-new))
	(entry-hr (gtk-entry-new))
	(entry-min (gtk-entry-new))
	(menu-week-days (gtk-option-menu-new))
	(week-day "Sunday")		;; default
	(separator (gtk-hseparator-new))
	(hbox (gtk-hbox-new #f 0))
	(ok (gtk-button-new-with-label "  Ok  "))
	(cancel (gtk-button-new-with-label "Cancel")))

    (gtk-window-set-title w "Add crontab entry")
    (gtk-signal-connect w "delete_event" cm-internal-close-window)
    (gtk-container-border-width vbox 10)
    (gtk-container-add w vbox)
    (gtk-box-pack-start vbox frame-data-entry #f #t 0)

    ;; set up menu-week-days if special case of ``week-day''
    (if (eq? type 'week-day)
	(let ((this-menu (gtk-menu-new)))
	  (for-each (lambda (x)
		      (let ((item (gtk-menu-item-new-with-label x)))
			(gtk-menu-append this-menu item)
			(gtk-widget-show item)
			(gtk-signal-connect item "activate"
					    (lambda ()
					      (set! week-day x)))))
		    (list "Sunday" "Monday" "Tuesday" "Wednesday" "Thursday" "Friday"))
	  (gtk-option-menu-set-menu menu-week-days this-menu)))

    ;; data entry frame
    (gtk-widget-set-usize entry-general 30 -1)
    (gtk-widget-set-usize entry-hr 30 -1)
    (gtk-widget-set-usize entry-min 30 -1)
    (gtk-container-add
     frame-data-entry
     (gnome-make-filled-vbox
      #t 3
      (gnome-boxed-widget
       (gnome-make-filled-hbox
	#f 3
	(gnome-boxed-widget (gtk-label-new "Description"))
	(gnome-boxed-widget #t #t 10 entry-desc)))
      (gnome-boxed-widget
       (gnome-make-filled-hbox
	#f 3
	(gnome-boxed-widget (gtk-label-new desc-pre))
	(gnome-boxed-widget (if (eq? type 'week-day)
				menu-week-days
				entry-general))
	(gnome-boxed-widget (gtk-label-new desc-post))))
      (if need-hr-min?
	  (gnome-boxed-widget
	   (gnome-make-filled-hbox
	    #f 3
	    (gnome-boxed-widget (gtk-label-new "Hour"))
	    (gnome-boxed-widget entry-hr)
	    (gnome-boxed-widget (gtk-label-new "Minute"))
	    (gnome-boxed-widget entry-min)
	    (gnome-boxed-widget #f #f 5 (gtk-label-new "(military time)"))))
	  #f)
      (gnome-boxed-widget
       #f #f 10
       (gnome-make-filled-hbox
	#f 3
	(gnome-boxed-widget (gtk-label-new "Command"))
	(gnome-boxed-widget #t #t 10 entry-command)))))

    (gtk-box-pack-start vbox separator #f #t 0)
    (gtk-box-pack-start vbox hbox #f #t 0)
    (gtk-box-pack-start hbox ok #f #t 0)
    (gtk-box-pack-end hbox cancel #f #t 0)

    (gtk-signal-connect cancel "clicked"
			(lambda () (gtk-widget-destroy w)))

    (gtk-signal-connect ok "clicked"
			(lambda ()
			  (let ((data (gtk-entry-get-text entry-general))
				(hr (if need-hr-min?
					(gtk-entry-get-text entry-hr)
					#f))
				(min (if need-hr-min?
					 (gtk-entry-get-text entry-min)))
				(command (gtk-entry-get-text entry-command))
				(desc (gtk-entry-get-text entry-desc)))
			    (set! cm-modified? #t)
			    (cm-add-entry
			     (case type
			       ((minutes) (list
					   desc
					   (string-append "*/" data)
					   "*" "*" "*" "*"
					   command))
			       ((hours) (list
					 desc "01"
					 (string-append "*/" data)
					 "*" "*" "*"
					 command))
			       ((days) (list
					desc min hr
					(string-append "*/" data)
					"*" "*"
					command))
			       ((months) (list
					  desc min hr "1"
					  (string-append "*/" data)
					  "*"
					  command))
			       ((day-of-month) (list
						desc min hr data "*" "*"
						command))
			       ((week-day) (list
					    desc min hr "*" "*"
					    (cadr (assoc week-day day-of-week))
					    command)))))
			  (gtk-widget-destroy w)))

    (gtk-widget-set-flags cancel '(can-default))
    (gtk-widget-grab-default cancel)
    (gtk-widget-show-multi w vbox frame-data-entry separator hbox ok cancel)
  ))

(define cm-main-menu
  `(("File" ("Save" ,cm-save)
	    ("Revert" ,(lambda ()
			 (cm-are-you-sure "Restore old crontab?"
					  (lambda ()
					    (cm-internal-clear-entries)
					    (set! crontab-entries (cm-read-and-parse-crontab-entries))
					    (cm-refresh-entries-list)))))
	    ("Exit" ,cm-quit))
    ("Edit" ("Add every x minutes" ,(lambda ()
				      (cm-add 'minutes "Every" "minutes")))
	    ("Add every x hours" ,(lambda ()
				    (cm-add 'hours "Every" "hours")))
	    ("Add every x days" ,(lambda ()
				   (cm-add 'days "Every" "days")))
	    ("Add every x months" ,(lambda ()
				     (cm-add 'months "Every" "months")))
	    ("Add on the x of each month" ,(lambda ()
					     (cm-add 'day-of-month
						     "On the" "of each month")))
	    ("Add every x of the week" ,(lambda ()
					  (cm-add 'week-day
						  "Every" "of the week")))
	    ("Add other" ,cm-add-other)
	    ("Modify" ,(lambda ()
			 (cm-internal-modify
			  (list-ref crontab-entries cm-current-entry) 'modify)))
	    ("Delete" ,cm-delete))
    ("Toolbar" ("Hide" ,(lambda ()
			  (gnome-toolbar-style cm-toolbar 'nothing)))
	       ("Show as text" ,(lambda ()
				  (gnome-toolbar-style cm-toolbar 'text)))
	       ("Show as icons" ,(lambda ()
				   (gnome-toolbar-style cm-toolbar 'icons)))
	       ("Show as text and icons" ,(lambda ()
					    (gnome-toolbar-style
					     cm-toolbar 'text-and-icons))))
    ("Help" ("About" ,(lambda ()
			(let ((w (gtk-window-new 'toplevel)))
			  (gtk-window-set-title w "About")
			  (gtk-container-add w (cm-create-about-panel))
			  (gtk-container-border-width w 50)
			  (gtk-signal-connect w "delete_event" cm-internal-close-window)
			  (gtk-widget-show w)))))))

(define (cm-create-main-menu)
  (let ((box (gtk-vbox-new #f 0))
	(menubar (gtk-menu-bar-new))
	(menu (gtk-menu-new))
	(separator (gtk-hseparator-new)))

    (gtk-box-pack-start box separator #f #t 0)
    (gtk-widget-show separator)

    (gtk-widget-show box)
    (gtk-box-pack-start box menubar #f #t 0)
    (gtk-widget-show menubar)

    ;; set up toolbar
    (set! cm-toolbar (gnome-create-toolbar box))
    (gnome-toolbar-add cm-toolbar "Modify" (gnome-pixmap "gnome-cromagnon-small.xpm") nil-proc)
    (gnome-toolbar-add cm-toolbar "Add" (gnome-pixmap "gnome-cromagnon-small.xpm") cm-add-other)
    (gnome-toolbar-add cm-toolbar "Delete" (gnome-pixmap "gnome-cromagnon-small.xpm") cm-delete)
    (gnome-toolbar-add cm-toolbar "Exit" (gnome-pixmap "gnome-cromagnon-small.xpm") cm-quit)
    (gnome-toolbar-style cm-toolbar 'text-and-icons)

    ;; header buttons
    ;; --boy these are ugly
    (let ((header-box (gtk-hbox-new #f 0))
	  (c 0))
      (for-each (lambda (x)
		  (set! x (gtk-button-new-with-label x))
		  (gtk-box-pack-start header-box x #f #f 0)
		  (gtk-widget-set-usize
		   x (vector-ref crontab-entries-spacing c) -1)
		  (gtk-widget-show x)
		  (set! c (1+ c)))
		(list "Description" "Min" "Hour" "Day" "Month" "Wk day" "Command            "))
      (gtk-box-pack-start box header-box #f #f 0)
      (gtk-widget-show header-box))

    ;; pulldown menus
    (for-each (lambda (menu-option)
		(let ((menuitem (gtk-menu-item-new-with-label (car menu-option)))
		      (pulldowns (cdr menu-option))
		      (menu (gtk-menu-new)))
		  (if (procedure? (cadr menu-option)) ;; no pulldowns?
		      ;; no pulldowns, just the main option
		      (gtk-signal-connect menuitem "activate" (cadr menu-option))
		      ;; yey, lots of pulldowns
		      (begin
			(for-each (lambda (n)
				    (let ((m (gtk-menu-item-new-with-label (car n))))
				      (gtk-menu-append menu m)
				      (gtk-signal-connect m "activate" (cadr n))
				      (gtk-widget-show m))) (cdr menu-option))
			(gtk-menu-item-set-submenu menuitem menu)))
		  (gtk-menu-bar-append menubar menuitem)
		  (gtk-widget-show menuitem))) cm-main-menu)

    ;; scrolled crontab entries
    (let ((scroll (gtk-scrolled-window-new #f #f)))
      (gtk-scrolled-window-set-policy scroll 'automatic 'always)
      (gtk-box-pack-start box scroll #t #t 0) ; scroll --> box
      (gtk-container-border-width scroll 5)

      (set! cm-main-scrolled-widget scroll) ; save globally for later
      (set! cm-entries-list (gtk-list-new)) ; ibid
      (gtk-list-set-selection-mode cm-entries-list 'multiple)
      (gtk-list-set-selection-mode cm-entries-list 'browse)
      (gtk-container-add scroll cm-entries-list)
      (gtk-widget-show cm-entries-list)
      (cm-refresh-entries-list)

      (gtk-signal-connect cm-entries-list "select_child"
			  (lambda (child)
			    (set! cm-current-entry (gtk-list-child-position cm-entries-list child))))

      (gtk-widget-show scroll))

	  
  box))

;; Make a list of crontab entries in the form of:
;;  ("crontab entry" "garbage preeding it" "garbage preceeding it, etc")
;;
;; The so called garbage are comments/whitespace that someone added
;; manually.  Hopefully that won't happen ;-)
;;
;; Notice we put the entry at the top and the comments and whitespace at
;; the bottom (for easy access).
;;
(define (cm-read-raw-crontab-entries)
  (let ((port (open-input-pipe "crontab -l")))
    (do ((line (read-line port) (read-line port))
	 (one-entry '())
	 (l '()))
	((eof-object? line) l)
      (if (string-match "^[A-Za-z0-9*]" line)
	  (begin
	    (set! l (cons (cons line (reverse one-entry)) l))
	    (set! one-entry '()))
	  (set! one-entry (cons line one-entry))))))

(define (cm-substring-match->list matches)
  (do ((i 1 (1+ i))
       (l (1- (vector-length matches)))
       (result '()))
      ((>= i l) (reverse result))
    (set! result (cons (match:substring matches i) result))))

(define (cm-read-and-parse-crontab-entries)
  (let ((re (make-regexp
	     "^\([^ 	]+\)[ 	]+\([^ 	]+\)[ 	]+\([^ 	]+\)[ 	]+\([^ 	]+\)[ 	]+\([^ 	]+\)[ 	]+\([^#]*\)"))
	(entries '()))
    (for-each (lambda (x)
		(if (not (regexp-exec re (car x)))
		    (if (string-match "^[A-Za-z0-9]+=" (car x))
			(set! entries (cons (list (car x)) entries))
			#f)		;; throw away, we don't know thee
		    (let ((entry (cm-substring-match->list (regexp-exec re (car x))))
			  (desc (list-ref x (1- (length x)))))
		      ;; put desc at the beginning
		      ;; (desc is the last line previous to the crontab entry
		      (set! entry (cons (substring desc 1 (string-length desc)) entry))
		      (set-cdr! (list-tail x (- (length x) 2)) '())	; chop desc
		      (set! entries (cons entry entries)))))
	      (cm-read-raw-crontab-entries))
    entries))

(define (cm-create-about-panel)
  (let ((alignment (gtk-alignment-new 0.5 0.5 0.0 0.0)))
    (gtk-container-add
     alignment
     (gnome-make-filled-vbox
      #f 3
      (gnome-boxed-widget (gtk-label-new "CroMagnon: Crontab Manager"))
      (gnome-boxed-widget (gtk-label-new "Version 0.0.0"))
      (gnome-boxed-widget (gtk-label-new "Aldy Hernandez <aldy@uaa.edu>"))))
    (gtk-widget-show alignment)
    alignment))

(define (main)
  (set! crontab-entries (cm-read-and-parse-crontab-entries))

  ;; create top-level window
  (let ((window (gtk-window-new 'toplevel)))
    (gtk-signal-connect window "delete_event"
			(lambda (x)
			  (gtk-exit)))
    (gtk-window-set-title window "CroMagnon: GNOME Crontab Manager")
    (gtk-container-border-width window 0)
    (gtk-widget-set-usize window 600 300)
    (gtk-container-add window (cm-create-main-menu))
    (gtk-widget-show window)))

(gnome-init-hack "cromagnon" (lambda (option arg) #f) '())
(launch-gnome main)

;; hbox: homogeneous
;; pack: expand fill padding

