(load "compiler.lisp")

(defparameter *bit-fields*
  '((home (field 0 2) int)
    (state (bit 3) (good bad))
    (heusl (field 4 5) (a b c d))))

(defparameter *heusl-ant*
  '(if (sense ahead food)
       (progn
	 (move)
	 (pick-up))
     (progn
       (turn left)
       (move))))

(defparameter *faz-ant*
  '(while (move)
     (move)))

(compile-to-file *heusl-ant* *bit-fields* "heusl.ant")
(compile-to-file *faz-ant* *bit-fields* "faz.ant")
