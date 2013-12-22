;;*****************************************************************************
;;*****************************************************************************
;;  FILENAME:   UART_2.inc
;;  Version: 5.3, Updated on 2013/5/19 at 10:44:49
;;  Generated by PSoC Designer 5.4.2946
;;
;;  DESCRIPTION:  Assembler declarations for the UART user module.
;;-----------------------------------------------------------------------------
;;  Copyright (c) Cypress Semiconductor 2013. All Rights Reserved.
;;*****************************************************************************
;;*****************************************************************************

;--------------------------------------------------
; Constants for UART_2 API's
;--------------------------------------------------

;mask value for global int reg bit for TX instance
UART_2_TX_INT_MASK:                          equ 0x40
UART_2_TX_INT_REG:                           equ 0x0e1  ;TX interrupt address
UART_2_RX_INT_MASK:                          equ 0x80  ;mask value for global int reg bit for RX instance
UART_2_RX_INT_REG:                           equ 0x0e1  ;RX interrupt address

UART_2_RXBUF_ENABLE:                         equ 1

; Interrupt control masks
UART_2_ENABLE_RX_INT:                        equ 0x01
UART_2_DISABLE_RX_INT:                       equ 0x00
UART_2_ENABLE_TX_INT:                        equ 0x02
UART_2_DISABLE_TX_INT:                       equ 0x00

UART_2_INT_MODE_TX_REG_EMPTY:                equ 0x00
UART_2_INT_MODE_TX_COMPLETE:                 equ 0x01

UART_2_RX_IGNORE_BELOW:                      equ 0x20
UART_2_RX_BUFFER_SIZE:                       equ 0x10
UART_2_CMD_TERM:                             equ 0x3e
UART_2_DELIMITER:                            equ 0x20
UART_2_BACKSPACE_ENABLE:                     equ 0x0

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;       WARNING WARNING WARNING
; The following equates are for backwards
; compatibility only and should not be used
; for new designs.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;------------------------------------
;  Parity masks
;------------------------------------
UART_PARITY_NONE:       equ   00h
UART_PARITY_EVEN:       equ   02h
UART_PARITY_ODD:        equ   06h

;------------------------------------
;  TX Status Register masks
;------------------------------------
UART_TX_COMPLETE:       equ   20h
UART_TX_BUFFER_EMPTY:   equ   10h

;------------------------------------
;  RX Status Register masks
;------------------------------------
UART_RX_ACTIVE:         equ   10h
UART_RX_COMPLETE:       equ   08h
UART_RX_PARITY_ERROR:   equ   80h
UART_RX_OVERRUN_ERROR:  equ   40h
UART_RX_FRAMING_ERROR:  equ   20h
UART_RX_NO_ERROR:       equ   E0h
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;             END WARNING
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;



;------------------------------------
;  Parity masks
;------------------------------------
UART_2_PARITY_NONE:                     equ   00h
UART_2_PARITY_EVEN:                     equ   02h
UART_2_PARITY_ODD:                      equ   06h

;------------------------------------
;  TX Status Register masks
;------------------------------------
UART_2_TX_COMPLETE:                     equ   20h
UART_2_TX_BUFFER_EMPTY:                 equ   10h

;------------------------------------
;  RX Status Register masks
;------------------------------------
UART_2_RX_ACTIVE:                       equ   10h
UART_2_RX_COMPLETE:                     equ   08h
UART_2_RX_REG_FULL:                     equ   08h
UART_2_RX_PARITY_ERROR:                 equ   80h
UART_2_RX_OVERRUN_ERROR:                equ   40h
UART_2_RX_FRAMING_ERROR:                equ   20h
UART_2_RX_ERROR:                        equ   E0h
UART_2_RX_ENABLE:                       equ   01h

UART_2_RX_BUF_ERROR:                    equ   F0h  ; Mask for any Rx that may occur.
UART_2_RX_BUF_OVERRUN:                  equ   10h  ; This indicates the software buffer has
                                                        ; been over run.
UART_2_RX_BUF_CMDTERM:                  equ   01h  ; Command terminator has been received.

UART_2_RX_NO_DATA:                      equ   01h
UART_2_RX_NO_ERROR:                     equ   E0h


;--------------------------------------------------
; Registers Address Constants for UART_2
;--------------------------------------------------
;---------------------------------
;  Registers used by TX
;---------------------------------
UART_2_TX_CONTROL_REG:  equ 3bh            ; Control register
UART_2_TX_SHIFT_REG:    equ 38h               ; TX Shift Register register
UART_2_TX_BUFFER_REG:   equ 39h               ; TX Buffer Register
UART_2_TX_FUNC_REG: equ 38h             ; Function register
UART_2_TX_INPUT_REG:    equ 39h            ; Input register
UART_2_TX_OUTPUT_REG:   equ 3ah           ; Output register

;---------------------------------
;  Registers used by RX
;---------------------------------
UART_2_RX_CONTROL_REG:  equ 3fh            ; Control register
UART_2_RX_SHIFT_REG:    equ 3ch               ; RX Shift Register register
UART_2_RX_BUFFER_REG:   equ 3eh               ; RX Buffer Register
UART_2_RX_FUNC_REG: equ 3ch             ; Function register
UART_2_RX_INPUT_REG:    equ 3dh            ; Input register
UART_2_RX_OUTPUT_REG:   equ 3eh           ; Output register

; end of file UART_2.inc