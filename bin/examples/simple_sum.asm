;sumujemy 6, 7 i 8, wynik w 9
;komórka pocz¹tkowa: 10

;dane
6	10
7	20
8	30

;komórka	intrukcja		liczbowo		ACU
;---------------------------------------------------------------------
10	CPA 6; 		1006		10
11	ADD 7;		3007		10 + 20
12	ADD 8;		3008		10 + 20 + 30
13	STO 9;		2009		bez zmian
14	HLT;		0000		

