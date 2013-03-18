;Przyk³ad prezentuje uzycie funkcji ekranowych
;Wybierz Widok -> Ekran by pokazaæ ekran
.code 0
loop:
	CPA [DI + str]
	BRZ end
	SCR [DI + str]
	INC DI

	;Podkreœlamy "hell"
	CPA DI
	SUB $28
	BRZF redFont

	CPA DI
	SUB $32
	BRZF whiteFont

	BRA loop

redFont:
	SCRB $1
	SCRF $0
	BRA loop

whiteFont:
	SCRB $0
	SCRF $7
	BRA loop

end:
	HLT
.data 18
str: DAT "Lorem ipsum dolor sit amet.\nHello world, this is a long text\n:)\n\narturo182\0"