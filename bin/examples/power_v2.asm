;potêgowanie
;komórka pocz¹tkowa 10
;wynik w 3

;dane
1	1003 ;podstawa (-3)
	5 ;wyk³adnik
4	1 ;do dekrementacji

;program
10	CPA 4
	STO 3
	CPA 3
	MUL 1
	STO 3
	BRZ 21
	CPA 2
	SUB 4
	STO 2
	BRZ 21
	BRA 12
 	HLT