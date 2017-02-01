(use-modules (gtk gtk))

(define titles #("name" "uid" "gid" "passwd" "gecos" "home" "shell"))

(define window (gtk-window-new 'toplevel))
(define scrolled-window (gtk-scrolled-window-new))
(define clist (gtk-clist-new-with-titles titles))
(gtk-container-add window scrolled-window)
(gtk-container-add scrolled-window clist)

(let loop ((pw (getpwent)))
  (cond (pw
	 (gtk-clist-append clist
			   (vector (passwd:name pw)
				   (number->string (passwd:uid pw))
				   (number->string (passwd:gid pw))
				   (passwd:passwd pw)
				   (passwd:gecos pw)
				   (passwd:dir pw)
				   (passwd:shell pw)))
	 (loop (getpwent)))))

(do ((i 0 (1+ i)))
    ((>= i (vector-length titles)))
  (gtk-clist-set-column-auto-resize clist i #t))

(gtk-widget-show-all window)

(gtk-standalone-main window)
