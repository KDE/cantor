(in-package :maxima)
#+clisp (defvar *old-suppress-check-redefinition* 
	      custom:*suppress-check-redefinition*)
#+clisp (setf custom:*suppress-check-redefinition* t)
(setf *alt-display2d* 'cantor-latex-print)
(setf *alt-display1d* 'cantor-regular-print)
(setf *prompt-prefix* "<prompt>")
(setf *prompt-suffix* "</prompt>")
;(setf *general-display-prefix* "DISPLAY_PREFIX")
(setf *maxima-prolog* "Hello World")
(setf *maxima-epilog* "Bye!")

;inchar:%I$
;outchar:%O$


;#-gcl(setf *debug-io* (make-two-way-stream *standard-input* *standard-output*))
;#+(or cmu sbcl scl)
;(setf *terminal-io* (make-two-way-stream *standard-input* *standard-output*))

;; Small changes to mactex.lisp for interfacing with TeXmacs
;; Andrey Grozin, 2001-2006

;(defun main-prompt ()
;  (format () "~A(~A~D) ~A" *prompt-prefix* 
;    (tex-stripdollar $inchar) $linenum *prompt-suffix*))

(declare-top
	 (special lop rop ccol $gcprint $inchar)
	 (*expr tex-lbp tex-rbp))
(defconstant texport *standard-output*)

(defun tex-stripdollar (x)
  (let ((s (quote-% (maybe-invert-string-case (symbol-name (stripdollar x))))))
    (if (> (length s) 1)
      (concatenate 'string "\\mathrm{" s "}")
      s)))

(defprop mtimes ("\\*") texsym)


(defun cantor-latex-print (x)
  (princ "<result>")
  (princ "<text>")
  (linear-displa x )
  (princ "</text>")

  (let ((ccol 1))
    (mapc #'myprinc
        (tex x '("<latex>") '("</latex>") 'mparen 'mparen)))

  (princ "</result>")
)

(defun cantor-regular-print (x)
  (princ "<result>")
  (princ "<text>")
  (linear-displa x)
  (princ "</text>")
  (princ "</result>")
)


#+clisp (setf custom:*suppress-check-redefinition*
	      *old-suppress-check-redefinition*)
