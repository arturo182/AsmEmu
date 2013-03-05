;potêgowanie
;komórka pocz¹tkowa 10
;wynik w 3

;dane
1	3 ;podstawa
2	5 ;wyk³adnik
4	1 ;do dekrementacji

;program
10	CPA 4
11	STO 3
12	CPA 3
13	MUL 1
14	STO 3
15	BRZ 21
16	CPA 2
17	SUB 4
18	STO 2
19	BRZ 21
20	BRA 12
21 	HLT