.data 0
podstawa:	3
wyk�adnik:	5
wynik:	0

.code 10
start:	CPA $1
	STO wynik
loop:	CPA wyk�adnik
	BRZ koniec
	DEC wyk�adnik
	CPA wynik
	MUL podstawa
	STO wynik
	BRA loop
koniec:	HLT