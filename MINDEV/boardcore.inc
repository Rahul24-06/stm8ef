; STM8S103F3P6 "Minimal System Board" STM8S device dependent routines


;       BOARDINIT  ( -- )
;       Init board GPIO (except COM ports)

BOARDINIT:
        ; Board I/O initialization
        BSET    PB_DDR,#5
        BSET    PB_CR1,#5

        .ifne   HAS_OUTPUTS
        CLR     A
        JRA     AOUTSTOR
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

LED_MPX:
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
        INCW    X
        LD      A,(X)
        INCW    X
AOUTSTOR:
        LD      OUTPUTS+1,A
        RRC     A
        CCF
        BCCM    PB_ODR,#5       ; PB5 LED
        RET
        .endif

;===============================================================

        .ifne   HAS_KEYS

;       BKEY  ( -- f )     ( TOS STM8: -- A,Z,N )
;       Read board key state as a bitfield

        .dw     LINK

        LINK =  .
        .db     (4)
        .ascii  "BKEY"
BKEY:
        CLR     A
        JP      ASTOR


;       BKEYC  (  -- c )   ( TOS STM8: -- A,Z,N )
;       Read and translate board dependent key bitmap into char

BKEYCHAR:
        JRA     BKEY            ; Dummy: get "no key" and leave it as it is

       .endif

