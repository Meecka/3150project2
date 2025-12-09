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
	SBIS PINA, 5; start beginning 
	RCALL IncrementCount; go to IncrementCount
	SBIS PINA, 7 ;goes in main loop 
	RCALL DecrementCount; go to DecrementCount

    SBIS PINA, 0
    RCALL IncrementTop; go to IncrementTop
	;switch 3 decrement
	SBIS PINA, 2
	RCALL DecrementTop; go to DecrementTop
	
	MOV R23, R20
	COM R23
	RCALL flip; got to flip
	OUT PORTD, R23
	OUT PORTA, R16
	RCALL DELAY; go to DELAY
	RCALL CheckHighLow; go to CheckHighLow
	RJMP start; return to start
	
	
	CheckHighLow:
		CP R20, R21; CheckHighLow start
		BRGE HHigh; go to HHigh if R21 is greater than or equal to R20
		CP R20, R17
		BRLT LLow; go to LLow if R21 is less than R20
		ret; CheckHighLow end
	HHigh: CP R20,R21; HHigh start
		  BREQ start; go to start if R20 and R21 equal
		  LDI R20, 0; if R20 and R21 unequal then set R20 to 0
		  RCALL buzz_one_point_five; go to buzz_one_point_five
		  ret; HHigh end
	LLow: MOV R20, R21; LLow start
		  RCALL buzz_one; go to buzz_one
		  ret; HHigh end
		   	
	
	
	IncrementTop:
    INC R21; IncrementTop start
    ret; IncrementTop end
	
	
	
	DecrementTop:  
	SUB R21, R22; DecrementTop start
	ret; DecrementTop end


	DELAY: LDI R18, 25; Delay start
L2:	LDI R19, 255; L2 start
L4:	LDI R30, 155; L4 start
L3: NOP; L3 start
	NOP
	DEC R30
	BRNE L3; L3 return if not zero
	DEC R19
	BRNE L4; L4 return if R19 not zero
	DEC R18
	BRNE L2; L2 return if R18 not zero
	RET; Delay end

	flip:
	mov r24, r23; flip start
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
	ret; flip end


	buzz_one_point_five:
	LDI R26, 10; buzz_one_point_five start
Loop4:
	LDI R25, 100; Loop4 start
Loop3:
	SBI PORTE, 4; Loop3 start
	RCALL Deley_one_point_five; go to Deley_one_point_five
	CBI PORTE, 4
	RCALL Deley_one_point_five; go to Deley_one_point_five
	dec R25
	BRNE Loop3; return to Loop 3 if R25 not zero
	dec R26
	BRNE Loop4; return to Loop 4 if R26 not zero
	ret; buzz_one_point_five end
	
	Deley_one_point_five:
		LDI R27, 10; Deley_one_point_five start
L8:	LDI R28, 132; L8 start
L7: NOP; L7 start
	DEC R28
	BRNE L7; return to L7 if R28 not zero
	DEC R27
	BRNE L8; return to L7 if R27 not zero
		RET; Deley_one_point_five end

	buzz_one:
	LDI R26, 10; buzz_one start
Loop2:
	LDI R25, 100; Loop2 start
Loop1:
	SBI PORTE, 4; Loop1 start
	RCALL Deley_oneK; go to Deley_oneK
	CBI PORTE, 4
	RCALL Deley_oneK; go to Deley_oneK
	dec R25
	BRNE Loop1; return to Loop1 if R25 not zero
	dec R26
	BRNE Loop2; return to Loop2 if R26 not zero
	ret; buzz_one end
	
	Deley_onek:
	LDI R27, 11; Deley_onek start
L6:	LDI R28, 181; L6 start
L5: NOP; L5 start
	DEC R28
	BRNE L5; return to L5 if not R28 zero
	DEC R27
	BRNE L6; return to L6 if not R27 zero
		RET; Deley_onek end



	IncrementCount: 

	INC R20; IncrementCount start

	RET; IncrementCount end

	DecrementCount: 

	DEC R20; DecrementCount start

	RET; DecrementCount end



