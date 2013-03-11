;dane
dzielna:	20
dzielnik:	7
reszta:	0
wynik:	0

;program
start:		CPA dzielnia
		SUB dzielnik
		BRN resztaKoniec
		INC wynik
		STO dzielna
		BRZ koniec
		BRA start
resztaKoniec:	CPA dzielna
		STO reszta
koniec:		HLT