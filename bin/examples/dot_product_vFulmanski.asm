.data 0
v1:	1
	2
	3
	4
	5
	6
	7
	8
	9
	10

v2: 	2
	2
	2
	2
	2
	2
	2
	2
	2
	2

a_v1:	v1 
a_v2:	v2
result:	0
vec_len:	10 ;n - length of vector

.code 50
begin: 
	CPA vec_len
	BRZ end
	CPA [a_v1]
	MUL [a_v2]
	ADD result
	STO result
	INC a_v1
	INC a_v2
	DEC vec_len
	BRA begin
end: 	
	HLT