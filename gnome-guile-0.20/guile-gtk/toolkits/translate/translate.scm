;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Ariel Rios               ;;
;; ariel@arcavia.com        ;;
;; http://erin.netpedia.net ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;; Released under GPL
;; Please Refer to the file COPYING

;; Translator Gtk+ header files unto guile-gtk defs file.

(for-each load (list "case.scm" "support.scm"))

(define (guile-gtk-grammar-enum-val str)
  (letrec ((enum                         
	    (lambda (ls word param n)
	      (cond 
	       ((equal? #\} (car ls)) 
		(list (reverse (cons (apply string-append (reverse word)) param)) n))
	       ((char-whitespace? (car ls)) (enum (cdr ls) word param (1+ n)))
	       ((equal? #\, (car ls)) (enum (cdr ls) '() (cons (apply string-append (reverse word)) param) (1+ n))) 
	       (else (enum (cdr ls) (cons (string (car ls)) word) param (1+ n)))))))
    (enum (cdr (member #\{ (string->list str))) '() '() 0)))

(define (guile-gtk-grammar-enum-write-single param)
  (let ((ls (string->list param)))
    (string-append "   ("  
		   (list->string  
		    (map char-downcase (string->list (substring param (length (member #\_ (reverse ls))) (length ls))))) 
		   " " param ")\n")))

(define (guile-gtk-grammar-enum-write param name)
  (string-append "(define-enum " name "\n"
		 (apply string-append (map guile-gtk-grammar-enum-write-single param)) ")\n"))

(define (guile-gtk-grammar-enum-name str)
  (let* ((ls (string->list str))     
	 (rl (reverse ls))
	 (i  (- (length ls) (1- (length (member #\_ ls)))))
	 (j  (1- (length (member #\} rl))))
	 (k  (1- (length (member #\{ rl)))))
    (substring str (if (> i k) (1+ j) i) (if (> i k)  (1- (length (member #\; rl))) k))))

(define (guile-gtk-grammar-enum str)
  (guile-gtk-grammar-enum-write (car (guile-gtk-grammar-enum-val str)) (guile-gtk-grammar-enum-name str)))

(define (guile-gtk-grammar-val-trim str)
  (list->string (map (lambda (x) (case x ((#\*) #\space) ((#\_) #\-) (else x))) (string->list str))))

(define (guile-gtk-grammar-val-param-guile ls)
  (map (lambda(x)  
	 (case* x 
		(("char"  "char *" "const gchar") "string")
		(("void" x) "")
		(("gint" "gint8" "gint16" "gint32") "int")
		(("guint" "guint8" "guint16" "guint32") "uint")
		(("double" "gdouble") "double")
		(("gboolean") "bool")
		(("glong") "long")
		(("gulong") "ulong")
		(("gfloat") "float")
 		(else x)))
       ls))

(define (guile-gtk-grammar-val-param-list str)
  (letrec ((val
	    (lambda (ls word obj param)
	      (cond
	       ((equal? #\) (car ls)) (reverse (cons (cons (apply string-append (reverse word)) obj) param)))
	       ((equal? #\, (car ls)) (val (cdr ls) '() '()
					   (cons (reverse (cons (apply string-append (reverse word)) obj)) param)))
	       (else (val (cdr ls) (cons (string (car ls)) word) obj param))))))
    (val (cdr (member #\( (string->list str))) '() '() '())))

(define (guile-gtk-grammar-val-param ls)
  (letrec ((split 
	    (lambda (ls out param)
	      (cond 
	       ((null? ls) (cdr (reverse (cons (apply string (reverse out)) (cons " " param)))))
	       ((char-whitespace? (car ls)) (split (cdr ls) '() (cons (apply string (reverse out)) (cons " " param))))
	       (else (split (cdr ls) (cons (car ls) out) param))))))
    (map (lambda (x) (apply string-append x))
	 (map guile-gtk-grammar-val-param-guile
	      (map (lambda (x) (map guile-gtk-grammar-val-trim x))
		   (map  (lambda (x) (split (string->list (car x)) '() '())) ls) ))) ) )

(define (guile-gtk-grammar-val-str ls) (apply string-append (map (lambda (x) (string-append "(" x ")\n")) ls)))

(define (guile-gtk-grammar-val-types-only-one-space str)
  (let ((n (string-length str)))
    (letrec ((doer
	      (lambda (s k)
		(cond 
		 ((= n k) s)
		 ((and (> k 0) (eqv? (string-ref str k) (string-ref str (1- k))) (eqv? #\space (string-ref str k)))
		  (doer s (1+ k)))
		 (else (doer (string-append s (string (string-ref str k))) (1+ k)))))))
      (doer "" 0))))
							    
(define (guile-gtk-grammar-val-types str)
  (guile-gtk-grammar-val-types-only-one-space
   (guile-gtk-grammar-val-str (guile-gtk-grammar-val-param (guile-gtk-grammar-val-param-list str)))))

(define (guile-gtk-grammar-val-func-return str)
  (let ((str (substring str 0 (- (string-length str) (length (member #\space (string->list str)))))))
    (guile-gtk-grammar-val-trim
     (case* str
	    (("char" "char *""const char" "const gchar") "string")
	    (("gint" "gint8" "gint16" "gint32") "int")
	    (("guint" "guint8" "guint16" "guint32") "uint")
	    (("double" "gdouble") "double")
	    (("gboolean") "bool")
	    (("glong") "long")
	    (("gulong") "ulong")
	    (("gfloat") "float")
	    (("void") "none")
	    (else str)))))
  
(define (guile-gtk-grammar-val-func-name str)
  (guile-gtk-grammar-val-types-only-one-space
   (substring str (- (string-length str) (length (member #\space (string->list str)))) (- (string-length str) (length (member #\( (string->list str)))))))

(define (guile-gtk-grammar-func-final-format str)
  (let ((n (string-length str)))
    (letrec ((doer 
	      (lambda (s k)
		(cond 
		 ((= n k) s)
		 ((and (> k 0) (eqv? #\space (string-ref str k)) (eqv? #\( (string-ref str (1- k))))
		  (doer s (1+ k)))
		 ((and (> k 0) (< (1+ k) n)(eqv? #\newline (string-ref str k)) (eqv? #\) (string-ref str (1+ k))))
		  (doer s (1+ k))) 
		 (else (doer (string-append s (string (string-ref str k))) (1+ k)))))))
      (doer "" 0))))

(define (guile-gtk-grammar-func str)
  (string-append "(define-func" 
		 (guile-gtk-grammar-val-func-name str)   "\n  "           
		 (guile-gtk-grammar-val-func-return str) "\n  ("
		 (guile-gtk-grammar-val-types str)       "))\n" ))

(define (guile-gtk-grammar-object-name str)
  (substring str (1+ (- (string-length str) (length (member #\_ (string->list str))))) (- (string-length str) (length (member #\{  (string->list str))) 1)))

(define (guile-gtk-grammar-object-type-all str)
  (substring str (1+ (- (string-length str) (length (member #\{ (string->list str))))) (- (string-length str) (length (member #\; (string->list str))) 0)))

(define (guile-gtk-grammar-object-find-index func? str)
  (let ((k (string-length str)))
    (letrec ((index 
	      (lambda (i)
		(cond
		 ((= i k) -1)
		 ((func? (string-ref str i)) i)
		 (else (index (1+ i)))))))
      (index 0))))

(define (guile-gtk-grammar-object-type str)
  (let* ((str (guile-gtk-grammar-object-type-all str))
	 (i (guile-gtk-grammar-object-find-index char-alphabetic? str))
	 (k (guile-gtk-grammar-object-find-index char-whitespace? (substring str i (string-length str)))))
    (substring str i (+ k i))))

(define (guile-gtk-grammar-object str)
  (guile-gtk-grammar-func-final-format (string-append "(define-object " (guile-gtk-grammar-object-name str) " (" (guile-gtk-grammar-object-type str) "))\n")))

(define (guile-gtk-grammar-split-at-semicolon str)
  (letrec ((split 
	    (lambda (s word ls)
	      (cond 
	       ((null? s) (reverse ls))
	       ((equal? (car s) #\;) (split (cdr s) '() (cons (list->string (reverse (cons #\; word))) ls)))
	       (else (split (cdr s) ((lambda (t) (if (equal? t #\newline) word (cons t word))) (car s))  ls))))))
    (split (string->list str) '() '())))
	      
(define-macro (define-apply-func name func)
  `(define (,name obj)
     (apply string-append (map ,func (if (list? obj) obj (guile-gtk-grammar-split-at-semicolon obj)))))) 

(define-apply-func object guile-gtk-grammar-object)
(define-apply-func enum guile-gtk-grammar-enum)
(define-apply-func func guile-gtk-grammar-func)

(define regexp-func (make-regexp "[A-z_]+[ *]*[ ]+[*A-Z_a-z\t]+[ ]+[(][ (),*A-Z a-z\t\n]+;"))
(define regexp-obj  (make-regexp "struct[ _]+[A-Za-z]+[ \t\n]+[{][- \t\nA-Za-z0-9;_/*<>:().',]+};"))
(define regexp-enum (make-regexp "typedef enum[ \t\n]*[{][- \t\nA-Za-z0-9;_/*<>:().',]+}[A-Za-z \t\n]+;"))

(define (regexp-do-list rx str)
  (letrec ((do-list 
	    (lambda (l k)
	      (call-with-current-continuation
	       (lambda (break)
		 (let* ((v (regexp-exec rx str k))
			(ls (if v (vector->list v) #f)))
		   (if (not ls)
		       (break (reverse l)))
		   (do-list (cons (substring str (caadr ls) (cdr (cadr ls))) l) (cdr (cadr ls)))))))))
    (do-list '() 0)))

(define (translate rx func file path out)
  (let ((str (read-string (string-append path "/" file))))
    (for-each (lambda (x) (write-char x out)) (string->list (func (regexp-do-list rx str))))))

(define (translate-all file path) 
  (let ((out (open-output-file (string-append file ".defs"))))
    (for-each (lambda (x) (write-char x out)) (string->list ";; -*- scheme -*-\n\n"))
    (for-each (lambda (x y) (translate x y file path out)) (list regexp-enum regexp-obj regexp-func) (list enum object func))
    (close out)))

(define (main dir) (for-each (lambda (x) (translate-all x dir)) (get-header-files dir)))
