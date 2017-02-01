#! /bin/sh
exec guile-gtk -s "$0" "$@"
!#
;; Time-stamp: <1998-03-15 21:18:16 szi>
;;
;; Copyright (C) 1997, 1998, 1999 Marius Vollmer
;; 
;; This program is free software; you can redistribute it and/or
;; modify it under the terms of the GNU General Public License as
;; published by the Free Software Foundation; either version 2, or (at
;; your option) any later version.
;; 
;; This program is distributed in the hope that it will be useful, but
;; WITHOUT ANY WARRANTY; without even the implied warranty of
;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
;; General Public License for more details.
;; 
;; You should have received a copy of the GNU General Public License
;; along with this software; see the file COPYING.  If not, write to
;; the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139,
;; USA.
;;
;;
;; Calc - a HP48 like calculator
;;
;; This is a small demo application of the Guile-gtk bindings, written
;; for the Gnome and Guile workshop at the IN congress in Aachen.
;;
;; It implements a simple desktop calculator in the famous HP
;; style. Altho it is quite simple, it is very powerful.  The complete
;; Scheme programming language is available to the user, both to
;; extend its functionality and while using it interactively.
;;
;; * Basic concepts
;;
;; The calculator maintains a stack of arbitray Scheme values and an
;; entry field where the user can type in new values.  Below the entry
;; field is an array of buttons that can be configured to the liking
;; of the user.
;;
;; The specification of such a button is quite straightforward: You
;; have to give it a label (a string), a Scheme function that performs
;; the desired operation, the number of argument this function takes
;; from the stack, and what to do with the return value.  The rest
;; happens automatically.  The stack is popped and pushed, etc.
;;
;; Buttons are grouped into a `row', and rows are grouped into a
;; `panel`.  Finally, when constructing a specific calculator, you
;; pass a list of panels that should appear in the calculator window.
;;
;; Typically, a panel groups buttons that belong to one theme, like
;; transcendetal functions, stack operations, functions for doing
;; calculus, or financial operations.  Even the digit pad is such a
;; panel with nothing special about it.
;;
;; When the calculator app starts, it reads the file "~/.calcrc" and
;; evaluates it as Scheme code.  The main purpose of this file is to
;; set up the preferred button configuration.  You can define new
;; panels and/or rearrange the existing ones.
;;
;; * Specifics
;; 
;; The panel configuration for the calculator is defined by the
;; variable "calc-panels".  To setup your own configuration, you can
;; set it in your "~/.calcrc" file.  There are some predefined panels
;; that you can use.  They are:
;;
;;   calc-arith-ops:  A row with "+-" "+" "-" "*" and "/"
;;
;;   calc-trans-ops:  Some rows with trigonometric functions and
;;                    their inverses.
;;
;;   calc-stack-ops:  Functions for manipulating the stack, like
;;                    "drop", "dup" and "swap".
;;
;;   calc-digit-pad:  The ten digits plus the arith-ops and some
;;                    goodies.
;;
;; You have to set "calc-panels" to a list of all the panels you want
;; to have in your calculator, like this
;;
;;   (set! calc-panels (list calc-arith-ops calc-trans-ops))
;;
;; This would give you only the artihmetic operations and some
;; scientific functions.  There would be nothing else, not even a
;; digit pad.
;;
;; To define your own panels, you can use the "calc-panel" construct.
;; This is actually a macro, so the normal rules of argument
;; evaluation do not apply to it.  The new syntax is:
;;
;;   (calc-panel ROW...)
;;
;; with
;;
;;   ROW -> (BUTTON...)
;;
;;   BUTTON -> (LABEL FUNC N-ARGS [RETURN-ACTION])
;; 
;; This will construct a new panel that consists of the specified ROWs
;; and return it.  You can then bind the return value to a variable or
;; use it directly.
;;
;; Each ROW is a list of BUTTONs and each BUTTON is specified by a
;; list of four elements.  That's quite a mouthful of lists, but this
;; is LISP, after all...
;;
;; The LABEL of a button is simply the string that will appear on the
;; screen.  LABEL will not be evaluated.  This means that you can only
;; put constant strings there and not variables.  If this is not what
;; you want, you can construct the panel description on your own with
;; `list', etc, but we don't explain it here.
;;
;; The FUNC is a Scheme function that will carry out the operation.
;; Unlike LABEL, FUNC *will* be evaluated by calc-panel, because that
;; is almost always what you want.  This means that you can simply
;; name functions by their name or construct new ones with `lambda'.
;; See below how this function is invoked.  When FUNC throws an error,
;; it is captured by the calculator and the stack is restored to its
;; previous state.
;;
;; N-ARGS indicates how many arguments this function takes.  When
;; N-ARGS is a number, the calculator will pop this many items off its
;; stack and pass them to FUNC.  When N-ARGS is the symbol `all', the
;; function will receive the complete stack as its arguments.  Note:
;; it will not be called with a single argument that is a list of all
;; stack items.  Rather it receives as many arguments as there are
;; stack items.  The first argument to FUNC is the last-popped stack
;; item.  When N-ARGS is the symbol `entry', it receives the current
;; value of the entry field as a string.  N-ARGS is not evaluated.
;;
;; RETURN-ACTION is a symbol that indicates what happens with the
;; return value.  It should be one of:
;;
;;   push:      Take the return value and push it onto the stack.
;;
;;   push-list: The return value is a list.  Push all its elements
;;              onto the stack, beginning with the first list element.
;;
;;   set-entry: Set the entry field to the returned string.
;;
;; Specifying RETURN-ACTION is optional.  When it is left out, it
;; defaults to `push'.  RETURN-ACTION is not evaluated.
;;
;; To recap: LABEL, N-ARGS and RETURN-ACTION are not evaluted, but
;; FUNC is.
;;
;; * Examples
;;
;; This is a panel that only contains one row with one button.  The
;; button takes the top two items in the stack, adds them as numbers
;; and pushes the result.
;;
;;   (define my-plus-panel (calc-panel (("+" + 2)))
;;
;; There is only one ROW, and the row contains only one BUTTON.  The
;; RETURN-ACTION of the BUTTON has been defaulted to `push'.
;;
;; The `+' right after the "+" label is the Scheme variable that
;; carries the normal addition function.
;;
;; The next one is slightly more involved.  It is a row of two buttons.
;; One swaps the two top elements of the stack, the other clears the
;; stack.
;;
;;   (define my-stack-ops 
;;     (calc-panel (("swap" (lambda (a b) (list b a)) 2 push-list)
;;                  ("clear" (lambda args '()) #t push-list))))
;;
;; Note that we are using `lambda' here to construct the functions in
;; place.
;;
;; `Swap' takes two arguments and returns them in a list that will be
;; pushed element-by-element.  `Clear' takes the whole stack as its
;; arguments, but ignores it and pushes nothing, thus leaving the
;; stack empty.
;;
;; For examples about the entry field, see the definition of
;; calc-digit-pad below.
;;
;;
;; * Implementation
;;
;; We put the calculator into its own module to protect us from
;; overwriting builtin functions.

(define-module (calc)
  :use-module (gtk gtk))

(if (not (defined? 'gtk-scrolled-window-add-with-viewport))
    (define gtk-scrolled-window-add-with-viewport gtk-container-add))

;; First some utility functions.  The first saves us from writing a
;; million times "gtk-widget-show".  It is like gtk-widget-show but
;; takes any number of widgets and shows them all.

(define (gtk-widget-show-multi . widgets)
  (for-each gtk-widget-show widgets))

;; This one loads a file from the users home directory, if it exists.

(define (load-rcfile name)
  (let ((file (string-append (getenv "HOME") "/" name)))
    (if (file-exists? file)
	(load file))))

;; Ok now, this is the meat of the calculator.  MAKE-CALCULATOR
;; constructs a new toplevel window and poulates it with widgets,
;; connects to their signals and then shows everything.  PANELS is the
;; list of panels that should be displayed below the entry field.

(define (make-calculator panels)

  ;; First, we create some widgets and store references to them in
  ;; local variables.
  (let ((window (gtk-window-new 'toplevel))
	(box (gtk-vbox-new #f 0))
	(scrolled-win (gtk-scrolled-window-new))
	(list-widget (gtk-list-new))
	(entry (gtk-entry-new))
	(echo (gtk-label-new " "))

	;; This is the caluclator stack.  It is simply a list of
        ;; arbitrary values, with the top of the stack in the first
        ;; element.  It starts out empty.
	(stack '()))
  
    ;; Some handy operations to work on the stack.  I think you can
    ;; figure them out.
    (define (push val) (set! stack (cons val stack)))
    (define (pop) (let ((val (car stack))) (set! stack (cdr stack)) val))

    ;; This pops N elements from the stack and returns a list of them.
    ;; The first element of this list is the stack item that was
    ;; popped last.  When there are fewer than N items on the stack,
    ;; we throw an error.
    (define (pop-n n)
      (if (< (length stack) n)
	  (throw 'too-few-arguments)
	  (do ((i n (1- i))
	       (s '() (cons (pop) s)))
	      ((zero? i) s))))

    ;; Make our list-widget show the stack.  The list-widget is
    ;; cleared and refilled each time.  This is slow, but should be
    ;; okay for now.
    (define (redisplay)

      ;; A line of the stack display is composed of a hbox that packs
      ;; a numeric label at the left and the value at the right.  We
      ;; print to a string to get the printed representation of the
      ;; value.
      (define (make-stack-line num val) 
	(let ((numlab (gtk-label-new (string-append (number->string num) ":")))
	      (vallab (gtk-label-new (call-with-output-string
				      (lambda (port) (write val port)))))
	      (box (gtk-hbox-new #f 2))
	      (item (gtk-list-item-new)))
	  (gtk-box-pack-start box numlab #f #f 0)
	  (gtk-box-pack-end box vallab #t #t 0)
	  (gtk-misc-set-alignment vallab 1.0 0.0)
	  (gtk-container-add item box)
	  (gtk-widget-show-multi numlab vallab box item)
	  item))

      ;; Now clear the list-widget and insert a stack-line for every
      ;; stack item.
      (gtk-list-clear-items list-widget 0 -1)
      (do ((i 1 (1+ i))
	   (s stack (cdr s)))
	  ((null? s))
	(gtk-list-prepend-item list-widget (make-stack-line i (car s)))))

    ;; This sets the echo area of the calculator to TEXT.
    (define set-echo
      (let ((last-text ""))
	(lambda (text)
	  ;; Avoid changing the label when the text would not change.
	  ;; Gtk does a lot of size negotiating when the label is
	  ;; changed, which leads to visible flicker.
	  (cond ((not (string=? last-text text))
		 (set! last-text text)
		 (gtk-label-set-text echo text))))))

    ;; Construct a concise error message out of KEY and ARGS.  For now
    ;; we just return the KEY as a string and print a more verbose
    ;; description to stderr.
    (define (construct-error-message key args)
      (if (= (length args) 4)
	  (apply display-error #f (current-error-port) args))
      (symbol->string key))

    ;; This gets called when the text in the entry field should be
    ;; pushed on the stack.  The text is filtered thru `read' so that
    ;; you can enter arbitray Scheme data in the field, like numbers,
    ;; strings, symbols or lists.  ACTIVATE-ENTRY takes one argument
    ;; that says whether to duplicate the topmost stack item when the
    ;; entry is empty.  It defaults to `#t'.  The entry is cleared
    ;; after pushing the value.
    (define (activate-entry . opt-dup)
      (define (read-all port)
	(let loop ((res '())
		   (val (read port)))
	  (if (eof-object? val)
	      (reverse! res)
	      (loop (cons val res) (read port)))))
      (let ((vals (call-with-input-string (gtk-entry-get-text entry)
					  read-all)))
	(if (null? vals)
	    (if (and (or (null? opt-dup) (car opt-dup)) (not (null? stack)))
		(push (car stack)))
	    (for-each push vals))
	(redisplay)
	(gtk-entry-set-text entry "")
	(set-echo "")))

    ;; Do whatever ACTION says to do with VAL.
    (define (perform-action val action)
      (case action 
	((push)      (push val))
	((push-list) (for-each push val))
	((set-entry) (gtk-entry-set-text entry val))
	(else        (error "bad action"))))

    ;; Return a new function that is suitable as a signal handler on a
    ;; panel button.  That new function prepares the arguments
    ;; according to N-ARGS, invokes FUNC and then performs ACTION on
    ;; the return value.
    (define (make-op func n-args action)
      (lambda ()

	;; Push the entry field prior to collecting the arguments, but
        ;; only when we are taking them from the stack.
	(if (not (eq? action 'set-entry))
	    (activate-entry #f))

	;; Save the stack so that we can restore it later.
	(let ((saved-stack stack))

	  ;; Now, collect all arguments, invoke the real function with
	  ;; them and stuff the return value, all while catching
	  ;; errors.
	  (catch #t
		 ;; This is the normal action.
		 (lambda ()
		   (set-echo "")
		   (let ((args (cond ((number? n-args)
				      (pop-n n-args))
				     ((eq? 'all n-args)
				      (pop-n (length stack)))
				     ((eq? 'entry n-args)
				      (list (gtk-entry-get-text entry)))
				     (else
				      (error "bad arg spec" n-args)))))
		     (perform-action (apply func args) action)))

		 ;; We come here when we have caught an error.
		 (lambda (key . args)
		   (set-echo (construct-error-message key args))
		   (set! stack saved-stack))))
	
	;; Update the stack display.
	(redisplay)))

    ;; Construct a hbox full of buttons according to SPECS.  SPECS is
    ;; a list of button specifications 
    ;;
    ;;   (LABEL FUNC N-ARGS RETURN-ACTION),
    ;; 
    ;; very similar to the BUTTON template above under "Specifics".
    ;; But here it is simply a list of values, not syntax with special
    ;; evaluation rules.  All the evaluation or not-evaluation takes
    ;; place when executing the `calc-panel' form.  See below.
    (define (make-button-row specs)
      (let ((box (gtk-hbox-new #t 2)))
	(for-each (lambda (s)
		    (let ((b (gtk-button-new-with-label (car s))))
		      (gtk-signal-connect b "clicked" (apply make-op (cdr s)))
		      (gtk-box-pack-start box b #t #t 0)
		      (gtk-widget-show b)))
		  specs)
	(gtk-widget-show box)
	box))

    ;; Now setup the GUI.  This is fairly basic Gtk stuff, just like
    ;; you would write it in C.
    (gtk-window-set-title window "Calc")
    (gtk-scrolled-window-set-policy scrolled-win 'automatic 'always)
    (gtk-scrolled-window-add-with-viewport scrolled-win list-widget)
    (gtk-misc-set-alignment echo 0.0 0.5)
    (gtk-widget-set-usize scrolled-win 200 120)

    ;; Pack the widgets from top to bottom.
    (gtk-container-add window box)
    (gtk-box-pack-start box scrolled-win #t #t 0)
    (gtk-box-pack-start box entry #f #f 0)

    ;; Here we loop thru all panels, and (within each panel) thru all
    ;; rows.  Each row is in the format expected by `make-button-row'
    ;; so we can simply pass it thru.
    (for-each (lambda (p)
		(for-each (lambda (r)
			    (let ((br (make-button-row r)))
			      (gtk-box-pack-start box br #f #f 1)))
			  p)
		(let ((sep (gtk-hseparator-new)))
		  (gtk-box-pack-start box sep #f #f 1)
		  (gtk-widget-show sep)))
	      panels)

    ;; Pack the echo area at the bottom.
    (gtk-box-pack-end box echo #f #f 0)

    ;; Finally, connect to some signals, show the whole mess and we
    ;; are done.
    (gtk-signal-connect entry "activate" activate-entry)
    (if (gtk-standalone?) (gtk-signal-connect window "destroy" gtk-exit))
    (gtk-widget-show-multi echo entry list-widget box scrolled-win window)))


;; This is the definition of the CALC-PANEL macro.  It looks quite
;; involved and was indeed not exactly trivial to get right.  A better
;; macro system like the syntax-case macros that are now included with
;; Guile should definitely be able to help here.
;;
;; Anyway, all it does is to construct a Scheme expression that --
;; when evaluated -- in turn constructs a list that can be used as a
;; panel description.  A panel description is a list of rows, and a
;; row is a list of button specifications.  A button spec is a list
;; of four elements:
;;
;;   (LABEL FUNC N-ARGS RETURN-ACTION)
;; 
;; This is very similar to the input syntax of `calc-panel' (see above
;; under "Specifics") but not the same.  For one, `calc-panel'
;; provides the default value of `push' for the RETURN-ACTION if it is
;; missing in the input; and it handles the funky evaluation business.
;; LABEL, N-ARGS and RETURN-ACTION are quoted in the constructed
;; Scheme expression, but FUNC is not.
;; 
;; Thus, the expression
;;
;;  (calc-panel (("+" + 2)))
;;
;; expands into
;;
;;  (list (list (list '"+" + '2 'push)))

(define-macro (calc-panel . rows)
  (let ((row-exprs (map (lambda (r)
			  `(list ,@(map (lambda (s)
					  `(list ',(car s)
						 ,(cadr s)
						 ',(caddr s)
						 ',(if (null? (cdddr s))
						       'push
						       (cadddr s))))
					r)))
			rows)))
    `(list ,@row-exprs)))


;; Some ready-made panels.
;;
;; The arithmetic operators are straight forward.  Maybe "+-" is
;; interesting: It is just the builtin `-' function with only one
;; argument, whereas the minus operator is this same builtin function
;; but with two arguments.

(define calc-arith-ops
  (calc-panel
   (("+-" - 1) ("+" + 2) ("-" - 2) ("*" * 2) ("/" / 2))))

;; These should be easy to understand.

(define calc-trans-ops
  (calc-panel
   (("sin" sin 1) ("cos" cos 1) ("tan" tan 1) ("exp" exp 1))
   (("asin" asin 1) ("acos" acos 1) ("atan" atan 1) ("log" log 1))
   (("sinh" sinh 1) ("cosh" cosh 1) ("tanh" tanh 1) ("expt" expt 2))))

;; The stack operations are a little bit more involved, but not
;; really.  Note that the "eval" button gives us a complete Scheme
;; interpreter in our calculator.  (We should do some module magic
;; here to protect us from evil evals.)  When RMS's visions come true
;; about Guile supporting many popular syntaxes and semantics, we
;; might get as easily a Phyton interpreter, or Perl, or TCL, or Java,
;; or what-you-have.

(define calc-stack-ops
  (calc-panel
   (("drop"  (lambda (a) '()) 1 push-list)
    ("swap"  (lambda (a b) (list b a)) 2 push-list)
    ("dup"   (lambda (a) (list a a)) 1 push-list)
    ("clear" (lambda args '()) all push-list)
    ("eval"  eval 1))))

;; The digit-pad has to implement insertion/deletion in the entry
;; field.  We need some helper functions for that.  INS produces an
;; insertion function that just tacks the argument given to INS onto
;; the old entry field text.  DEL removes the last char of the text,
;; when there is one.

(define (ins string)
  (lambda (entry)
    (string-append entry string)))

(define (del entry)
  (let ((len (string-length entry)))
    (if (> len 0)
	(substring entry 0 (1- len))
	entry)))

;; Can you figure out the "get" function?

(define calc-digit-pad
  (calc-panel
   (("+-" - 1)
    ("get" (lambda (a) 
	     (with-output-to-string (lambda () (write a))))
	   1 set-entry)
    ("ENTER" (lambda () '()) 0 push-list)
    ("DEL" del entry set-entry))
   (("7" (ins "7") entry set-entry)
    ("8" (ins "8") entry set-entry)
    ("9" (ins "9") entry set-entry) ("/" / 2))
   (("4" (ins "4") entry set-entry)
    ("5" (ins "5") entry set-entry)
    ("6" (ins "6") entry set-entry) ("*" * 2))
   (("1" (ins "1") entry set-entry)
    ("2" (ins "2") entry set-entry)
    ("3" (ins "3") entry set-entry) ("-" - 2))
   (("0" (ins "0") entry set-entry)
    ("." (ins ".") entry set-entry)
    ("SPC" (ins " ") entry set-entry) ("+" + 2))))

;; Predefine CALC-PANELS with some useful panels.  Note that the
;; arith-ops are not included because they are also on the digit-pad.

(define calc-panels (list calc-stack-ops calc-trans-ops calc-digit-pad))

;; Now load the user configuration file and use whatever ends up in
;; `calc-panels' to construct the calculator.

(load-rcfile ".calcrc")
(make-calculator calc-panels)

(if (gtk-standalone?) (gtk-main))

; Local Variables:
; mode: scheme
; End:
