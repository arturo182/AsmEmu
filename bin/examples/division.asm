;dzielenie z reszt¹

.data 0
dzielna:	20
dzielnik:	7
reszta:	0
wynik:	0

.code 10
start:		CPA dzielna
		SUB dzielnik
		BRN resztaKoniec
		INC wynik
		STO dzielna
		BRZ koniec
		BRA start
resztaKoniec:	CPA dzielna
		STO reszta
koniec:		HLT