;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Implrementacija asemblerskih prekidnih rutina za tajmer i AD konverziju
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

			.cdecls C,LIST,"msp430.h"       ; Ukljucivanje header fajla uredjaja

			.ref	current_state           ; Referenca na promenljivu current_state
			.ref	ad_result               ; Reference na promenljivu ad_result

; TIMER0A0 ISR
			.text
CCR0ISR		push.w	R8						; Cuvanje R8 i R9 na steku
			push.w	R9
            mov.w   #0000h, R11				; Praznjenje registra R11
			mov.b   current_state, R11      ; Pomeranje vrednosti current_state u R11
			cmp.b 	#005h, R11              ; Provera da li smo u stanju SELECT_SYMBOL (5)
			jnz		timer_exit
		    bis.w   #ADC12SC,&ADC12CTL0     ; Startovanje AD konverzija ako jesmo u stanju SELECT_SYMBOL
timer_exit  pop.w	R9						; Vracanje registara R8 i R9
			pop.w	R8
			reti


; ADC12 ISR
            .text
ADC12ISR    push.w  R8                            ; Cuvanje R8 i R9 na steku
            push.w  R9
            mov.w   #0000h, R13                   ; Praznjenje registra R13
            cmp     #ADC12IV_ADC12IFG0,&ADC12IV   ; Proverava se da li je trigerovan interrupt za AD konverziju
            jnz     adc_exit
            mov.w   ADC12MEM0,R13				  ; Ako jeste, iz registra MEM0 prepisuje se vrednost u registar R13
            rra		R13                           ; 1. rotacija registra R13
            rra		R13                           ; 2. rotacija registra R13
            rra		R13                           ; 3. rotacija registra R13
            rra		R13                           ; 4. rotacija registra R13
            rra		R13                           ; 5. rotacija registra R13
            rra		R13                           ; 6. rotacija registra R13
            rra		R13                           ; 7. rotacija registra R13
            rra		R13                           ; 8. rotacija registra R13 ---> dobili smo 4 najvisa bita na najnizim pozicijama, kao shift u levo za 8 mesta
            and     #000fh, R13					  ; And-ovanje sa maskom koja uzima samo najniza 4 bita
           	mov.w   R13, ad_result				  ; Prebacivanje rezultata iz registra R13 u promenljivu ad_result
adc_exit    pop.w   R9                            ; Vracanje registara R8 i R9
            pop.w   R8
            reti

; Vektori
			.sect	.int54
			.short	CCR0ISR

			.sect	.int55
			.short	ADC12ISR
