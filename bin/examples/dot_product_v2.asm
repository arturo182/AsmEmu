;iloczyn skalarny
;wynik w 8

;d³ugoœæ wektorów
9	10

;wektor 1
10	2
11	2
12	2
13	2
14	2
15	2
16	2
17	2
18	2
19	2

;wektor 2
20	1
21	2
22	3
23	4
24	5
25	6
26	7
27	8
28	9
29	10

;program
start:	CPA $10
	STO 1
	CPA $20
	STO 2
	CPA $0
	STO 8
	CPA 9
	BRZ koniec
	CPA [1]
	MUL [2]
	ADD 8
	STO 8
	INC 1
	INC 2
	DEC 9
	BRA 36
koniec:	HLT