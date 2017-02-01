;; A small calendar application for Guile-gtk
;;This is a simple calendar built using the GtkCalendar Widget
;;It actually does nothing but being a Calendar.

(use-modules (gtk gtk))

(define (calendar-example)
 (let ((window (gtk-window-new 'toplevel))
       (calendar (gtk-calendar-new)))
 (gtk-container-add window calendar)
 (gtk-widget-show-all window) 
 (gtk-standalone-main window) )) 

(calendar-example)

;;Ariel Rios
