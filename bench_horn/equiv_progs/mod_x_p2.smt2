(declare-rel inv (Int Int Int))
(declare-var x Int)
(declare-var x1 Int)
(declare-var y Int)
(declare-var y1 Int)
(declare-var z Int)
(declare-var z1 Int)
(declare-var len Int)

(declare-rel fail ())

(rule (=>
    (and  (= z 0) ) (inv x y z)
  )
)

(rule (=> 
    (and 
	(inv x y z) 
	(= z1 (mod (mod y x) x))
        (= x1 x)
        (= y1 y)
    )
    (inv x1 y1 z1)
  )
)

(rule (=> (and (inv x y z) (not (= y (* 2 x)))) fail))

(query fail :print-certificate true)

