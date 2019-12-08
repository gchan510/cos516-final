(declare-rel inv (Int))
(declare-var y Int)

(declare-rel fail ())

(rule (=> true (inv y)))
(rule (=> (inv y) (inv y)))
(rule (=> (not (inv y)) fail))

(query fail :print-certificate true)
