(drop)

(if cond conse alt)


(if (move)
    (progn
      (drop)
      (turn left))
  (turn right))


(while (sense ahead food)
  (move)
  (turn left))
