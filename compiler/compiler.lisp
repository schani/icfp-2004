(load "let-match.lisp")
(require 'utils)

;;; state management

(defvar *state-tmp-num* 0)
(defun make-state-tmp-name ()
  (let ((name (format nil "state_tmp_~A" *state-tmp-num*)))
    (incf *state-tmp-num*)
    name))

(defmacro with-state-fork (&rest body)
  `(let ((*state-tmp-num-save* *state-tmp-num*)
	 (*state-tmp-highest* nil))
    (let ((*result* (progn ,@body)))
      (setq *state-tmp-num* *state-tmp-highest*)
      *result*)))

(defmacro with-state-undo (&rest body)
  `(let ((*result* (progn ,@body)))
    (if *state-tmp-highest*
	(assert (= *state-tmp-num* *state-tmp-highest*))
	(setq *state-tmp-highest* *state-tmp-num*))
    (setq *state-tmp-num* *state-tmp-num-save*)
    *result*))

;;; macro expansion

(defun make-new-old-lists (arg-names args macro-call)
  (cond ((and (null arg-names)
	      (null args))
	 (values nil nil))
	((and (null arg-names)
	      (not (null args)))
	 (error "too many arguments in macro call ~A" macro-call))
	;; now we know arg-names is not null
	((eql (car arg-names) '&rest)
	 (values (list args) (list (cadr arg-names))))
	((null args)
	 (error "too few arguments in macro call ~A" macro-call))
	(t
	 (multiple-value-bind (rest-new rest-old)
	     (make-new-old-lists (cdr arg-names) (cdr args) macro-call)
	   (values (cons (car args) rest-new)
		   (cons (car arg-names) rest-old))))))

(defvar *lsg-macros* nil)
(defun add-lsg-macro (name arg-names body)
  (push (cons name
	      #'(lambda (&rest args)
		  (multiple-value-bind (new old)
		      (make-new-old-lists arg-names args (cons name args))
		    (subst-many new old body))))
	*lsg-macros*))

(defmacro deflsgmacro (name arg-names body)
  `(add-lsg-macro ',name ',arg-names ',body))

(defun lsg-macroexpand (body)
  (my-macroexpand body *lsg-macros*))

(defun unlispify (expr state)
  (let ((expr (lsg-macroexpand expr)))
    (if (and (listp expr)
	     (member (car expr) '(lisp lisp-nostate)))
	(unlispify (eval (if (eql (car expr) 'lisp)
			     (subst-many (mapcar #'cdr state) (mapcar #'car state) (cadr expr))
			     (cadr expr)))
		   state)
	expr)))

(defmacro with-unlispify (vars &rest body)
  (assert (> (length vars) 0))
  `(let ,(mapcar #'(lambda (var)
		     `(,var (unlispify ,var state)))
		 vars)
     ,@body))

;;; ir op creation

(defun stringify (name)
  (if (symbolp name) (symbol-name name) name))

(defun make-label-name (name state)
  (format nil "~A~{-~A~}" name (mapcar #'(lambda (s) (cdr s)) (reverse state))))

(defun make-tmp-label-name (state)
  (make-label-name (make-tmp-name) state))

;(defun redirect-name (name new-name)
;  (rplaca name (stringify new-name)))

;(defun ref-name (name)
;  (car name))

(defun make-label (name)
  (list (list 'label name)))

(defun make-goto (name)
  (make-sense 'here name name 'friend))

(defun make-sense (direction st1 st2 condition)
  (list (list 'sense direction st1 st2 condition)))

(defun make-move (st1 st2)
  (list (list 'move st1 st2)))

(defun make-pick-up (st1 st2)
  (list (list 'pick-up st1 st2)))

(defun make-flip (p st1 st2)
  (list (list 'flip p st1 st2)))

(defun make-mark (i st)
  (list (list 'mark i st)))

(defun make-unmark (i st)
  (list (list 'unmark i st)))

(defun make-drop (st)
  (list (list 'drop st)))

(defun make-turn (lr st)
  (list (list 'turn lr st)))

;;; bit field management

(defvar *bit-fields* nil)
(proclaim '(special *bit-fields*))

(defun bit-field-spec-num-values (spec)
  (case-match spec
	      ((field ?start ?stop)
	       (ash 1 (1+ (- stop start))))
	      ((bit ?bit)
	       2)
	      (t
	       (error "unknown bit field spec ~A" spec))))

(defun verify-bit-fields (bit-fields)
  (dolist (bit-field bit-fields)
    (destructuring-bind (name spec values)
	bit-field
      (case-match spec
	((field ?start ?stop)
	 (assert (and (>= start 0) (< start 6)
		      (>= stop 0) (< stop 6)
		      (>= stop start)))
	 (assert (or (eql values 'int)
		     (= (length values) (bit-field-spec-num-values spec)))))
	((bit ?bit)
	 (assert (and (>= bit 0) (< bit 6)))
	 (assert (= (length values) 2)))
	(t
	 (error "unknown bit field description ~A" bit-field))))))

;; returns a list (<first> <length> <values>)
(defun lookup-bit-field (name)
  (let ((bit-field (assoc name *bit-fields*)))
    (when (null bit-field)
      (error "unknown bit field ~A" name))
    (destructuring-bind (spec values)
	(cdr bit-field)
      (let* ((first (second spec))
	     (length (if (eql (first spec) 'field)
			 (1+ (- (third spec) first))
			 1))
	     (value (if (eql values 'int)
			(integers-upto (bit-field-spec-num-values spec))
			values)))
	(list first length value)))))

(defun lookup-bits (name value)
  (labels ((make-bits (value first length)
	     (if (<= length 0)
		 '()
		 (cons (cons first (oddp value)) (make-bits (ash value -1) (1+ first) (1- length))))))
    (destructuring-bind (first length values)
	(lookup-bit-field name)
      (let ((value (position value values)))
	(make-bits value first length)))))

;;; liesegang->ir compiler

;(defun compile-direction-case (bits la-name a-name ra-name u-name)
;  

(defun compile-condition (condition consequent-name alternative-name state)
  (labels ((sense-bit (direction bit consequent-name alternative-name)
	     (if (cdr bit)
		 (make-sense direction consequent-name alternative-name (list 'marker (car bit)))
		 (make-sense direction alternative-name consequent-name (list 'marker (car bit)))))
	   (sense-bits (direction bits consequent-name alternative-name)
	     (assert (not (null bits)))
	     (if (null (cdr bits))
		 (sense-bit direction (car bits) consequent-name alternative-name)
		 (let ((next-name (make-tmp-label-name state)))
		   (append (sense-bit direction (car bits) next-name alternative-name)
			   (make-label next-name)
			   (sense-bits direction (cdr bits) consequent-name alternative-name)))))
	   (compile (condition consequent-name alternative-name)
	     (case-match condition
	       ((sense ?direction (marker ?name ?value))
		(with-unlispify (direction name value)
		  (let ((bits (lookup-bits name value)))
		    (sense-bits direction bits consequent-name alternative-name))))
	       ((sense ?direction ?cond)
		(with-unlispify (direction cond)
		  (make-sense direction consequent-name alternative-name cond)))
	       ((move)
		(make-move consequent-name alternative-name))
	       ((pick-up)
		(make-pick-up consequent-name alternative-name))
	       ((flip ?p)
		(with-unlispify (p)
		  (make-flip p consequent-name alternative-name)))
	       ((not ?c)
		(with-unlispify (c)
		  (compile c alternative-name consequent-name)))
	       ((and ?a ?b)
		(with-unlispify (a b)
		  (let ((b-name (make-tmp-label-name state)))
		    (append (compile a b-name alternative-name)
			    (make-label b-name)
			    (compile b consequent-name alternative-name)))))
	       ((or ?a ?b)
		(with-unlispify (a b)
		  (let ((b-name (make-tmp-label-name state)))
		    (append (compile a consequent-name b-name)
			    (make-label b-name)
			    (compile b consequent-name alternative-name)))))
	       (t
		(error "cannot compile condition ~A" condition)))))
    (compile condition consequent-name alternative-name)))

(defun compile-progn (body target state)
  (cond ((null body)
	 (make-goto target))
	((null (cdr body))
	 (compile-to-ir (unlispify (car body) state) target state))
	(t
	 (let ((continue-name (make-tmp-label-name state)))
	   (append (compile-to-ir (unlispify (car body) state) continue-name state)
		   (make-label continue-name)
		   (compile-progn (cdr body) target state))))))

(defun modify-state (state name value)
  (cond ((null state)
	 (error "no variable ~A in state" name))
	((eql (caar state) name)
	 (cons (cons name value) (cdr state)))
	(t
	 (cons (car state) (modify-state (cdr state) name value)))))

(defun compile-to-ir (expr target state)
  (labels ((mark-bit (bit negate target)
	     (if (xor (cdr bit) negate)
		 (make-mark (car bit) target)
		 (make-unmark (car bit) target)))
	   (mark-bits (bits negate target)
	     (assert (not (null bits)))
	     (if (null (cdr bits))
		 (mark-bit (car bits) negate target)
		 (let ((next-name (make-tmp-label-name state)))
		   (append (mark-bit (car bits) negate next-name)
			   (make-label next-name)
			   (mark-bits (cdr bits) negate target)))))
	   (mark-field (name value negate target)
	     (let ((bits (lookup-bits name value)))
	       (mark-bits bits negate target)))
	   (compile (expr target state)
	     (let ((expr (lsg-macroexpand expr)))
	       (case-match expr
		 ((nop)
		  (make-goto target))

		 ((lisp ?sexp)
		  (compile (unlispify expr state) target state))

		 ((lisp-nostate ?sexp)
		  (compile (unlispify expr state) target state))

		 ((label ?name)
		  (with-unlispify (name)
		    (append (make-label (make-label-name name state))
			    (make-goto target))))

		 ((goto ?name)
		  (with-unlispify (name)
		    (make-goto (make-label-name name state))))

		 ((if ?condition ?consequent ?alternative)
		  (with-unlispify (condition consequent alternative)
		    (cond
		      ((eql condition t)
		       (compile consequent target state))
		      ((eql condition nil)
		       (compile alternative target state))
		      (t
		       (let ((consequent-name (make-tmp-label-name state))
			     (alternative-name (make-tmp-label-name state)))
			 (append (compile-condition condition consequent-name alternative-name state)
				 (make-label consequent-name)
				 (compile consequent target state)
				 (make-label alternative-name)
				 (compile alternative target state)))))))

		 ((while ?condition . ?body)
		  (with-unlispify (condition)
		    (let ((start-name (make-tmp-label-name state))
			  (body-name (make-tmp-label-name state)))
		      (append (make-label start-name)
			      (compile-condition condition body-name target state)
			      (make-label body-name)
			      (compile-progn body start-name state)))))

		 ((forever . ?body)
		  (let ((start-name (make-tmp-label-name state)))
		    (append (make-label start-name)
			    (compile-progn body start-name state))))

		 ((progn . ?body)
		  (compile-progn body target state))

		 ((with-state (?name ?values) . ?body)
		  (with-state-fork
		      (mappend #'(lambda (value)
				   (let ((new-state (cons (cons name value) state)))
				     (with-state-undo
					 (compile-progn body target new-state))))
			       values)))

		 ((setq ?name ?value)
		  (with-unlispify (name value)
		    (let ((set-name (make-state-tmp-name)))
		      (append (make-goto (make-label-name set-name (modify-state state name value)))
			      (make-label (make-label-name set-name state))
			      (make-goto target)))))

		 ((mark ?i)
		  (with-unlispify (i)
		    (make-mark i target)))

		 ((mark ?name ?value)
		  (with-unlispify (name value)
		    (mark-field name value nil target)))

		 ((unmark ?i)
		  (with-unlispify (i)
		    (make-unmark i target)))

		 ((unmark ?name ?value)
		  (with-unlispify (name value)
		    (mark-field name value t target)))

		 ((drop)
		  (make-drop target))

		 ((turn ?lr)
		  (with-unlispify (lr)
		    (make-turn lr target)))

		 (t
		  (compile-condition expr target target state))))))
    (compile expr target state)))

;;; ir->sm compiler

(defun make-label-alist (ir next-sn)
  (cond ((null ir)
	 '())
	((eql (caar ir) 'label)
	 (cons (cons (cadar ir) next-sn) (make-label-alist (cdr ir) next-sn)))
	(t
	 (make-label-alist (cdr ir) (1+ next-sn)))))

(defun sense-dir-name (dir)
  (let ((c (assoc dir '((here . "Here") (ahead . "Ahead")
			(left-ahead . "LeftAhead") (right-ahead . "RightAhead")))))
    (if c
	(cdr c)
      (error "unknown sense dir ~A" dir))))

(defun condition-name (condition)
  (case-match condition
    ((marker ?marker)
     (format nil "Marker ~A" marker))
    (t
     (let ((c (assoc condition '((friend . "Friend") (foe . "Foe")
				 (friend-with-food . "FriendWithFood")
				 (foe-with-food . "FoeWithFood")
				 (food . "Food") (rock . "Rock")
				 (foe-marker . "FoeMarker")
				 (home . "Home") (foe-home . "FoeHome")))))
       (if c
	   (cdr c)
	   (error "unknown condition ~A" condition))))))

(defun turn-name (turn)
  (let ((c (assoc turn '((left . "Left") (right . "Right")))))
    (if c
	(cdr c)
      (error "unknown turn ~A" turn))))

(defun ir-to-sm (ir)
  (let ((label-alist (make-label-alist ir 0)))
    (labels ((lookup (name)
	       (let ((a (assoc name label-alist :test #'equalp)))
		 (if a
		     (cdr a)
		     (error "unknown label ~A" name))))
	     (generate-sm (ir next-sn)
	       (cond
		 ((null ir)
		  "")
		 ((eql (caar ir) 'label)
		  (generate-sm (cdr ir) next-sn))
		 (t
		  (string-concat
		   (case-match (car ir)
		     ((sense ?direction ?st1 ?st2 ?condition)
		      (format nil "Sense ~A ~A ~A ~A"
			      (sense-dir-name direction)
			      (lookup st1) (lookup st2)
			      (condition-name condition)))
		     ((move ?st1 ?st2)
		      (format nil "Move ~A ~A"
			      (lookup st1) (lookup st2)))
		     ((pick-up ?st1 ?st2)
		      (format nil "PickUp ~A ~A"
			      (lookup st1) (lookup st2)))
		     ((flip ?p ?st1 ?st2)
		      (format nil "Flip ~A ~A ~A"
			      p (lookup st1) (lookup st2)))
		     ((mark ?i ?st)
		      (format nil "Mark ~A ~A"
			      i (lookup st)))
		     ((unmark ?i ?st)
		      (format nil "Unmark ~A ~A"
			      i (lookup st)))
		     ((drop ?st)
		      (format nil "Drop ~A"
			      (lookup st)))
		     ((turn ?lr ?st)
		      (format nil "Turn ~A ~A"
			      (turn-name lr) (lookup st)))
		     (t
		      (error "malformed ir op ~A" ir)))
		   (format nil "   ; state ~A~%" next-sn)
		   (generate-sm (cdr ir) (1+ next-sn)))))))
      (generate-sm ir 0))))

;;; compiler driver

(defun compile-to-string (expr bit-fields)
  (verify-bit-fields bit-fields)
  (let ((*bit-fields* bit-fields))
    (ir-to-sm (append (make-label (make-label-name "da-start" nil))
		      (compile-to-ir (lsg-macroexpand expr) "da-start" nil)
		      (make-goto (make-label-name "da-start" nil))))))

(defun compile-to-file (expr bit-fields file-name)
  (with-open-file (out file-name :direction :output :if-exists :supersede)
    (format out (compile-to-string expr bit-fields))))
