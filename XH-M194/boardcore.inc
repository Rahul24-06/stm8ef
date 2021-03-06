; XH-M194:
; - 7S-LED 4digit clock-style display w/ 74HC595 shift register
; - 7S-LED MPX PD7, PD3, PD2, PD0
; - Clock-style second indicator dots PD1/SWIM
; - STM8S10S4 PD1/SWIM single pin communication
; - DS1302 RTC PA6, PB6, PB7

;       BOARDINIT  ( -- )
;       Init board GPIO (except COM ports)

BOARDINIT:

        ; XH-M194 STM8S105S4 init GPIO
        MOV     PA_DDR,#0b01110110 ; -kFE-DC- LED green k:CLK-RTC
        MOV     PA_CR1,#0b01110110

        MOV     PB_DDR,#0b01000000 ; dc<^v>LR keys c:CE-RTC d:DAT-RTC
        MOV     PB_CR1,#0b01000000

        MOV     PC_DDR,#0b11101110 ; 123-56A- LED red, green
        MOV     PC_CR1,#0b11101110

        MOV     PD_DDR,#0b11111101 ; thd123-4 t:STCP h:SHCP d:DS
        MOV     PD_CR1,#0b11111101 ; thd123-4 1:D1 2:D2 3:D3 4:D4

        MOV     PE_DDR,#0b00100000 ; --B----- LED green
        MOV     PE_CR1,#0b00100000

        MOV     PG_DDR,#0b00000001 ; -------4 LED red
        MOV     PG_CR1,#0b00000001

        .ifne   HAS_OUTPUTS
        CLRW    Y
        JP      YOUTSTOR
        .else
        RET
        .endif

;===============================================================

;      Dummy labels for PSIM interrupts declared in main.c

        .ifne   PSIM-PORTA
;       Dummy label for _EXTIA_IRQHandler
_EXTI0_IRQHandler:
        .endif

        .ifne   PSIM-PORTB
;       Dummy label for _EXTIB_IRQHandler
_EXTI1_IRQHandler:
        .endif

        .ifne   PSIM-PORTC
;       Dummy label for _EXTIC_IRQHandler
_EXTI2_IRQHandler:
        .endif

        .ifne   PSIM-PORTD
;       Dummy label for _EXTID_IRQHandler
_EXTI3_IRQHandler:
        .endif

        .ifne   PSIM-PORTE
;       Dummy label for _EXTIE_IRQHandler
_EXTI4_IRQHandler:
        .endif

;===============================================================

        .ifne   HAS_LED7SEG

;       LED_MPX driver ( -- )
;       Code called from ISR for LED MPX

        .db     2

LED_MPX:
        ;       turn off all digits
        LD      A,#0b11100010   ; Digits ...123.4
        AND     A,PD_ODR
        LD      PD_ODR,A

        LD      A,TICKCNT+1
        AND     A,#3            ; 4 digits MPX
        PUSH    A               ; digit #

        ; clock dots shared with the console
        LDW     Y,TIMSERIAL     ; decrement COM use timer
        JREQ    LEDMPXDOTS
        DECW    Y               ; active COM: don't change PNRX properties
        LDW     TIMSERIAL,Y
        JRA     LEDMPXGO

LEDMPXDOTS:
        MOV     TIMRELOAD,#1    ; Reset TIMSERIAL reload value
        BTJF    PD_DDR,#PNRX,LEDDOTS
        BRES    PD_DDR,#PNRX
        BRES    PD_CR1,#PNRX
        BSET    PD_CR2,#PNRX

LEDDOTS:                        ; CFG "CFGDOTS" digit # test used for colon dots
        CP      A,#2
        JRNE    LEDMPXGO
        TNZ     LED7FIRST+1     ; dots: decimal point of 2nd digit
        JRPL    LEDMPXGO
        BRES    PD_ODR,#PNRX    ; clock dots shared with the console
        BRES    PD_CR2,#PNRX
        BSET    PD_CR1,#PNRX
        BSET    PD_DDR,#PNRX

LEDMPXGO:
        CLRW    X
        LD      XL,A
        LD      A,(LED7FIRST,X)
        CPL     A               ; common anode

        ;       LED segment clock out
        PUSH    A               ; LED pattern (2,SP)
        LD      A,#8            ; 8 clock pulses for 74HC595
        PUSH    A               ; set counter (1,SP)

LED_CLK:
        SLL     (2,SP)
        BCCM    PD_ODR,#5       ; 74HC595 serial data in
        BRES    PD_ODR,#6       ; 74HC595 serial clock
        BSET    PD_ODR,#6
        DEC     (1,SP)
        JRNE    LED_CLK

        BSET    PD_DDR,#7       ; 74HC595 parallel clock
        POPW    X               ; discard counter and LED pattern
        BRES    PD_DDR,#7

        ;       Set active LED digit
        POP     A               ; digit #

        TNZ     A
        JRNE    1$
        BSET    PD_ODR,#4       ; Digit 1...
        JRA     LEDMPXEND

1$:     CP      A,#1
        JRNE    2$
        BSET    PD_ODR,#3       ; Digit .2..
        JRA     LEDMPXEND

2$:     CP      A,#2
        JRNE    3$
        BSET    PD_ODR,#2       ; Digit ..3.
        JRA     LEDMPXEND

3$:     CP      A,#3
        JRNE    LEDMPXEND
        BSET    PD_ODR,#0       ; Digit ...4
        ; fall through

LEDMPXEND:
        RET
        .endif



;===============================================================

        .ifne   HAS_OUTPUTS
        RamWord OUTPUTS         ; "OUT", e.g. relays, LEDs, etc. (16 bit)

;       OUT!  ( c -- )
;       Put c to board outputs, storing a copy in OUTPUTS
        .dw     LINK

        LINK =  .
        .db     (4)
        .ascii  "OUT!"
OUTSTOR:
        CALL    YFLAGS
YOUTSTOR:
        LDW     OUTPUTS,Y
        LD      A,YH
        RRC     A
        BCCM    PC_ODR,#1       ; A: R1 LED green
        RRC     A
        BCCM    PE_ODR,#5       ; B: R2 LED green
        RRC     A
        BCCM    PA_ODR,#1       ; C: R3 LED green
        RRC     A
        BCCM    PA_ODR,#2       ; D: R4 LED green
        RRC     A
        BCCM    PA_ODR,#4       ; E: R5 LED green
        RRC     A
        BCCM    PA_ODR,#5       ; F: R6 LED green
        LD      A,YL
        RRC     A
        BCCM    PC_ODR,#7       ; 1: R1 LED red
        RRC     A
        BCCM    PC_ODR,#6       ; 2: R2 LED red
        RRC     A
        BCCM    PC_ODR,#5       ; 3: R3 LED red
        RRC     A
        BCCM    PG_ODR,#0       ; 4: R4 LED red
        RRC     A
        BCCM    PC_ODR,#3       ; 5: R5 LED red
        RRC     A
        BCCM    PC_ODR,#2       ; 6: R6 LED red
        RET
        .endif

;===============================================================

        .ifne   HAS_KEYS
;       BKEY  ( -- c )     ( TOS STM8: -- A,Z,N )
;       Read board key state as a bitfield
        .dw     LINK

        LINK =  .
        .db     (4)
        .ascii  "BKEY"
BKEY:
        ; Keys "R" (0x01), "L" (0x02), ">" (0x04)
        ;      "v" (0x08), "^" (0x10), "<" (0x20)
        LD      A, PB_IDR
        CPL     A
        AND     A, #0x3F
3$:     JP      ASTOR

;       BKEYC  ( -- c )   ( TOS STM8: -- A,Z,N )
;       Read and translate board dependent key bitmap into char

BKEYCHAR:
        CALLR   BKEY
        JREQ    1$
        ADD     A,#'@'
        LD      (1,X),A
1$:     RET

       .endif
