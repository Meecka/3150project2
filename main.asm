;
; Project.asm
;

    
	LDI R16, 0xFF
	LDI R17, 0x00
	OUT DDRA, R17 ; make all PA as input
	OUT PORTA, R17 ; Enable pull-up on PA
	OUT DDRD, R16 ; make all PD output
	LDI R16, 0x10
	OUT DDRE, R16 ; make all PE output
	OUT PORTD, R16
	LDI R16, 0xFF
	
	LDI R22, 1; subtract
	LDI R21, 25 ;max value
	LDI R20, 0b01111000
	OUT PORTD, R20
	start:
    ;switch 4 increment the top count
	SBIS PINA, 5 
	RCALL IncrementCount
	SBIS PINA, 7 ;goes in main loop 
	RCALL DecrementCount

    SBIS PINA, 0
    RCALL IncrementTop
	;switch 3 decrement
	SBIS PINA, 2
	RCALL DecrementTop
	
	MOV R23, R20
	COM R23
	RCALL flip
	OUT PORTD, R23
	OUT PORTA, R16
	RCALL DELAY
	RCALL CheckHighLow
	RJMP start
	
	
	CheckHighLow:
		CP R20, R21 
		BRGE HHigh
		CP R20, R17
		BRLT LLow
		ret
	HHigh: CP R20,R21
		  BREQ start
		  LDI R20, 0
		  RCALL buzz_one_point_five
		  ret
	LLow: MOV R20, R21
		  RCALL buzz_one
		  ret
		   	
	
	
	IncrementTop:
    INC R21
    ret
	
	
	
	DecrementTop:  
	SUB R21, R22
	ret


	DELAY: LDI R18, 25
L2:	LDI R19, 255
L4:	LDI R30, 155
L3: NOP
	NOP
	DEC R30
	BRNE L3
	DEC R19
	BRNE L4
	DEC R18
	BRNE L2
	RET

	flip:
	mov r24, r23
	ror r23  ; rotate right, pushing bit 0 out to C flag
	rol r24  ; rotate left, moving C flag into bit 0 and pushing bit 7 out to C flag
	ror r23
	rol r24
	ror r23
	rol r24
	ror r23
	rol r24
	ror r23
	andi r23, 0xF0 ; isolating bit 4 to 7
	andi r24, 0x0F ; isolating bit 0 to 3
	or r23, r24    ; combining
	swap r23       ; swapping the nibbles
	ret


	buzz_one_point_five:
	LDI R26, 10
Loop4:
	LDI R25, 100 
Loop3:
	SBI PORTE, 4
	RCALL Deley_one_point_five
	CBI PORTE, 4
	RCALL Deley_one_point_five
	dec R25
	BRNE Loop3
	dec R26
	BRNE Loop4
	ret
	
	Deley_one_point_five:
		LDI R27, 10
L8:	LDI R28, 132
L7: NOP
	DEC R28
	BRNE L7
	DEC R27
	BRNE L8
		RET

	buzz_one:
	LDI R26, 10
Loop2:
	LDI R25, 100 
Loop1:
	SBI PORTE, 4
	RCALL Deley_oneK
	CBI PORTE, 4
	RCALL Deley_oneK
	dec R25
	BRNE Loop1
	dec R26
	BRNE Loop2
	ret
	
	Deley_onek:
	LDI R27, 11
L6:	LDI R28, 181
L5: NOP
	DEC R28
	BRNE L5
	DEC R27
	BRNE L6
		RET



	IncrementCount: 

	INC R20 

	RET 

	DecrementCount: 

	DEC R20 

	RET 



