(use-modules (gnome gnome)
	     (gtk gtk))

(define (callback info)
  (lambda args
    (apply pk 'callback info args)
    #t))
  

(gnome-init-hack "uiinfo" #f '())

(let* ((file-menu `((item "New..."
			  :callback ,(callback 'new)
			  :stock-pixmap "New")
		    (item "Open"
			  :callback ,(callback 'open)
			  :stock-pixmap "Open")
		    (item "Save"
			  :callback ,(callback 'save)
			  :stock-pixmap "Save")
		    (item "Save As..."
			  :callback ,(callback 'save-as)
			  :stock-pixmap "Save As")
		    (separator)
		    (item "Exit"
			  :callback ,gtk-main-quit
			  :stock-pixmap "Quit")))
       (edit-menu `((item "Cut"
			  :callback ,(callback 'cut)
			  :stock-pixmap "Cut")
		    (item "Copy"
			  :callback ,(callback 'copy)
			  :stock-pixmap "Copy")
		    (item "Paste"
			  :callback ,(callback 'paste)
			  :stock-pixmap "Paste")))
       (conf-menu `((new "New..." :callback ,(callback 'c-new))
		    (open ,(callback 'c-open))
		    (save ,(callback 'c-save))
		    (save-as ,(callback 'c-save-as))
		    (revert ,(callback 'c-revert))
		    (print ,(callback 'c-print))
		    (print-setup ,(callback 'c-print-setup))
		    (close ,(callback 'c-close))
		    (exit ,(callback 'c-exit))
		    (cut ,(callback 'c-cut))
		    (copy ,(callback 'c-copy))
		    (paste ,(callback 'c-paste))
		    (clear ,(callback 'c-clear))
		    (undo ,(callback 'c-undo))
		    (redo ,(callback 'c-redo))
		    (find ,(callback 'c-find))
		    (find-again ,(callback 'c-find-again))
		    (replace ,(callback 'c-replace))
		    (properties ,(callback 'c-properties))
		    (preferences ,(callback 'c-preferences))
		    (about ,(callback 'c-about))
		    (select-all ,(callback 'c-select-all))
		    (new-window ,(callback 'c-new-window))
		    (pause-game ,(callback 'c-pause-game))
		    (restart-game ,(callback 'c-restart-game))
		    (undo-move ,(callback 'c-undo-move))
		    (redo-move ,(callback 'c-redo-move))
		    (hint ,(callback 'c-hint))
		    (scores ,(callback 'c-scores))
		    (end-game ,(callback 'c-end-game))))
       (test-menu `((radioitems ((item "One")       ; This is
				 (item "Two")       ; the magical
				 (item "Three")))   ; verse for
		    (separator)                     ; calling the
		    (item "Run GC" :callback ,gc))) ; evil collector
       (menu `((subtree "File" :items ,file-menu)
	       (subtree "Edit" :items ,edit-menu)
	       (subtree "Conf" :items ,conf-menu)
	       (subtree "Test" :items ,test-menu)))
       (app (gnome-app-new "uiinfo_demo" "UIInfo Demo")))
  (gnome-app-create-menus app menu)
  ;; or alternatively
  ;; (let ((uiinfos (pk 'infos (gnome-uiinfos-intern menu))))
  ;;   (twiddle-uiinfos uiinfos)
  ;;   (gnome-app-create-menus app uiinfos))
  (gtk-widget-show-all app)
  (gtk-standalone-main app))
