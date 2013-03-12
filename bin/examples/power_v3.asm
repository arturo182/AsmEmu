.data 0
podstawa:	3
wyk³adnik:	5
wynik:	0

.code 10
start:	CPA $1
	STO wynik
loop:	CPA wyk³adnik
	BRZ koniec
	DEC wyk³adnik
	CPA wynik
	MUL podstawa
	STO wynik
	BRA loop
koniec:	HLT