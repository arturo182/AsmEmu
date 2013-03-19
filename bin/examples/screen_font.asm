.code 0
start:
	CPA $32
	STO DI
	
loop:
	SCR DI
	INC DI
	
	CPA DI
	SUB $126
	BRZ end
	BRA loop
	
end:
	HLT