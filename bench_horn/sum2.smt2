(declare-rel inv (Int))
(declare-var x Int)
(declare-var x1 Int)

(declare-rel fail ())

(rule (=>
   (= x 0)  (inv x)
  )
)

(rule (=> 
    (and 
	(inv x) 
	(= x1 (+ 2 x))
    )
    (inv x1)
  )
)

(rule (=> (and (inv x) (= x1 (/ (* x (+ x 1)) 2))) fail))

(query fail :print-certificate true)

