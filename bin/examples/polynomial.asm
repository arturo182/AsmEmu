;f(x) = ax^3 + bx^2 + cx + d
;komórka startowa 10
;wynik w 4

5	2 ; x
6	3 ; a
7	4 ; b
8	5 ; c
9	6 ; d

;schemat hornera
;((a * x + b) * x + c) * x + d

10	CPA 6;		3
11	MUL 5;		3 * 2
12	ADD 7;		3 * 2 + 4
13	MUL 5;		(3 * 2 + 4) * 2
14	ADD 8;		(3 * 2 + 4) * 2 + 5
15	MUL 5;		((3 * 2 + 4) * 2 + 5) * 2
16	ADD 9;		((3 * 2 + 4) * 2 + 5) * 2 + 6
17	STO 4
18	HLT