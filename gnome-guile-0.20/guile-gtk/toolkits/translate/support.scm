;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Ariel Rios               ;;
;; ariel@arcavia.com        ;;
;; http://erin.netpedia.net ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;; Released under GPL
;; Please Refer to the file COPYING

(define (c-header? str) ((lambda (k)(if (<= k 2) #f (equal? ".h" (substring str (- k 2) k)))) (string-length str)))

(define (get-header-files arg)
  (call-with-current-continuation
   (lambda (break)
     (if (not (and (access? arg R_OK) (file-is-directory? arg)))
	 (break #f))
     (let((dir (opendir arg)))
       (letrec ((director
		 (lambda (ls)
		   (let ((obj (readdir dir)))
		     (if (eof-object? obj) 
			 (reverse ls)
			 (director (if (c-header? obj) (cons obj ls) ls))))))) 
	 (director '()))))))



