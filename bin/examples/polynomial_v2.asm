;4 * 2^2 + 2 * 2^1 + 3 = 16 + 4 + 3 = 27

.data 0
;wielomian
varX:	2

varA:	4
varB:	2
varC:	3
wsp:	1

powA:	2
powB:	1
wyk:	5

superWynik:	0 
licznik:	2 ;ilosc wspolczynnikow

;potegowanie
podstawa:	0
wykladnik:	0
wynik:	0

.code 20
start:		CPA varX
		STO podstawa
		CPA licznik
		STO wykladnik
		BRA potegaStart
loop:		CPA wynik
		MUL [4]
		INC wsp
		INC wyk
		ADD superWynik
		STO superWynik
		DEC licznik
		CPA licznik
		BRN koniec
		BRA start
koniec:		CPA [4]
		ADD superWynik
		STO superWynik
		HLT


potegaStart:	CPA $1
		STO wynik
potegaLoop:		CPA wykladnik
		BRZ potegaKoniec
		DEC wykladnik
		CPA wynik
		MUL podstawa
		STO wynik
		BRA potegaLoop
potegaKoniec:	BRA loop