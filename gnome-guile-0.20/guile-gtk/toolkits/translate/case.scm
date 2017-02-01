(use-modules (ice-9 syncase))

(define-syntax case*
  (syntax-rules (else)
    ((case* (key ...)
       clauses ...)
     (let ((atom-key (key ...)))
       (case* atom-key clauses ...)))
    ((case* key
       (else result1 result2 ...))
     (begin result1 result2 ...))
    ((case* key
       ((atoms ...) result1 result2 ...))
     (if (member key '(atoms ...))
         (begin result1 result2 ...)))
    ((case* key
       ((atoms ...) result1 result2 ...)
       clause clauses ...)
     (if (member key '(atoms ...))
         (begin result1 result2 ...)
         (case* key clause clauses ...)))))

;; 

(define (do-read l in)
  (if (eof-object? (peek-char in))
      (begin (close in )(apply string (reverse l)))
      (do-read (cons (read-char in) l) in)))

(define (read-string file)
  (let ((in (open-input-file file)))
    (do-read '() in)))



