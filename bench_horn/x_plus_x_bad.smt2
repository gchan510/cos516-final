(declare-rel inv (Int Int))
(declare-var x Int)
(declare-var x1 Int)
(declare-var y Int)
(declare-var y1 Int)
(declare-var len Int)

(declare-rel fail ())

(rule (=>
    (and  (= y 0) ) (inv x y)
  )
)

(rule (=> 
    (and 
	(inv x y) 
	(= y1 (+ (+ x x) 1))
        (= x1 x)
    )
    (inv x1 y1)
  )
)

(rule (=> (and (inv x y) (not (= y (* 2 x)))) fail))

(query fail :print-certificate true)

