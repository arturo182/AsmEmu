;iloczyn skalarny z wektorami generowanymi kodem
;wektor pierwszy to same 2 a wektor drugi to liczby od 1 do 10

.data 8
wynik:	0
rozmiar:	10

;program
.code 30
start:		BRA stworzWektory
		CPA $10
		STO 1
		CPA $20
		STO 2
		CPA $0
		STO wynik

iteracja:		CPA rozmiar
		BRZ koniec
		CPA [1]
		MUL [2]
		ADD wynik
		STO wynik
		INC 1
		INC 2
		DEC rozmiar
		BRA iteracja

stworzWektory:	CPA rozmiar
		STO 5

wektor1:		CPA 5
		SUB rozmiar
		SUB rozmiar
		BRZ wektor2
		CPA $2
		STO [5]
		INC 5
		BRA wektor1

wektor2:		CPA 5
		SUB rozmiar
		SUB rozmiar
		SUB rozmiar
		BRZ 31
		CPA 5
		SUB rozmiar
		SUB rozmiar
		ADD $1
		STO [5]
		INC 5
		BRA wektor2

koniec:		HLT