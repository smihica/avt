;;*****************************************************************************
;;*****************************************************************************
;;  FILENAME: I2CHW_1.asm
;;   Version: 1.90, Updated on 2012/3/2 at 9:14:43
;;  Generated by PSoC Designer 5.2.2551
;;
;;  DESCRIPTION: I2Cs User Module software implementation file
;;
;;  NOTE: User Module APIs conform to the fastcall16 convention for marshalling
;;        arguments and observe the associated "Registers are volatile" policy.
;;        This means it is the caller's responsibility to preserve any values
;;        in the X and A registers that are still needed after the API functions
;;        returns. For Large Memory Model devices it is also the caller's 
;;        responsibility to perserve any value in the CUR_PP, IDX_PP, MVR_PP and 
;;        MVW_PP registers. Even though some of these registers may not be modified
;;        now, there is no guarantee that will remain the case in future releases.
;;-----------------------------------------------------------------------------
;;  Copyright (c) Cypress Semiconductor 2012. All Rights Reserved.
;;*****************************************************************************
;;*****************************************************************************

include "m8c.inc"
include "memory.inc"
include "I2CHW_1Common.inc"
include "PSoCGPIOINT.inc"
include "PSoCAPI.inc"

;-----------------------------------------------
; include instance specific register definitions
;-----------------------------------------------

;-----------------------------------------------
;  Global Symbols
;-----------------------------------------------
;-------------------------------------------------------------------
;  Declare the functions global for both assembler and C compiler.
;
;  Note that there are two names for each API. First name is
;  assembler reference. Name with underscore is name refence for
;  C compiler.  Calling function in C source code does not require
;  the underscore.
;-------------------------------------------------------------------

export    I2CHW_1_ResumeInt
export   _I2CHW_1_ResumeInt
export    I2CHW_1_EnableInt
export   _I2CHW_1_EnableInt
export    I2CHW_1_ClearInt
export   _I2CHW_1_ClearInt
IF (I2CHW_1_MUM_SEL & (I2CHW_1_SLAVE | I2CHW_1_MMS))
export    I2CHW_1_EnableSlave
export   _I2CHW_1_EnableSlave
ENDIF
IF (I2CHW_1_MUM_SEL & (I2CHW_1_MSTR | I2CHW_1_MMS))
export    I2CHW_1_EnableMstr
export   _I2CHW_1_EnableMstr
ENDIF
export    I2CHW_1_Start
export   _I2CHW_1_Start
export    I2CHW_1_DisableInt
export   _I2CHW_1_DisableInt
IF (I2CHW_1_MUM_SEL & (I2CHW_1_SLAVE | I2CHW_1_MMS))
export    I2CHW_1_DisableSlave
export   _I2CHW_1_DisableSlave
ENDIF
IF (I2CHW_1_MUM_SEL & (I2CHW_1_MSTR | I2CHW_1_MMS))
export    I2CHW_1_DisableMstr
export   _I2CHW_1_DisableMstr
ENDIF
export    I2CHW_1_Stop
export   _I2CHW_1_Stop


AREA UserModules (ROM, REL)

.SECTION

;-----------------------------------------------------------------------------
;  FUNCTION NAME: I2CHW_1_Start
;
;  DESCRIPTION:
;   Initialize the I2CHW_1 I2C bus interface.
;
;-----------------------------------------------------------------------------
;
;  ARGUMENTS:
;
;  RETURNS: none
;
;  SIDE EFFECTS:
;    The A and X registers may be modified by this or future implementations
;    of this function.  The same is true for all RAM page pointer registers in
;    the Large Memory Model.  When necessary, it is the calling function's
;    responsibility to perserve their values across calls to fastcall16 
;    functions.
;          
 I2CHW_1_Start:
_I2CHW_1_Start:
    RAM_PROLOGUE RAM_USE_CLASS_1
    RAM_EPILOGUE RAM_USE_CLASS_1
    ret
.ENDSECTION

.SECTION
;-----------------------------------------------------------------------------
;  FUNCTION NAME: I2CHW_1_ResumeInt
;
;  DESCRIPTION:
;     reEnables SDA interrupt allowing start condition detection. 
;     Skips clearing INT_CLR3 by entering the EnableInt at ResumeIntEntry:.
;     Remember to call the global interrupt enable function by using
;     the macro: M8C_EnableGInt.
;-----------------------------------------------------------------------------
;  ARGUMENTS: none
;
;  RETURNS: none
;
;  SIDE EFFECTS:
;    The A and X registers may be modified by this or future implementations
;    of this function.  The same is true for all RAM page pointer registers in
;    the Large Memory Model.  When necessary, it is the calling function's
;    responsibility to perserve their values across calls to fastcall16 
;    functions.
;-----------------------------------------------------------------------------
 I2CHW_1_ResumeInt:
_I2CHW_1_ResumeInt:
    RAM_PROLOGUE RAM_USE_CLASS_1
    push A
    jmp ResumeIntEntry
    
;-----------------------------------------------------------------------------
;  FUNCTION NAME: I2CHW_1_EnableInt
;
;  DESCRIPTION:
;     Enables SDA interrupt allowing start condition detection. Remember to call the
;     global interrupt enable function by using the macro: M8C_EnableGInt.
;
;-----------------------------------------------------------------------------
;
;  ARGUMENTS: none
;
;  RETURNS: none
;
;  SIDE EFFECTS:
;    The A and X registers may be modified by this or future implementations
;    of this function.  The same is true for all RAM page pointer registers in
;    the Large Memory Model.  When necessary, it is the calling function's
;    responsibility to perserve their values across calls to fastcall16 
;    functions.
;-----------------------------------------------------------------------------
 I2CHW_1_EnableInt:
_I2CHW_1_EnableInt:
    RAM_PROLOGUE RAM_USE_CLASS_1
    ;first clear any pending interrupts
    push A
    mov A, reg[INT_CLR3]
    and A, ~I2CHW_1_INT_MASK
    mov reg[INT_CLR3], A
ResumeIntEntry:
    M8C_EnableIntMask I2CHW_1_INT_REG, I2CHW_1_INT_MASK
    pop A
    RAM_EPILOGUE RAM_USE_CLASS_1
    ret

.ENDSECTION

.SECTION
;-----------------------------------------------------------------------------
;  FUNCTION NAME: I2CHW_1_ClearInt
;
;  DESCRIPTION:
;     Clears only the I2C interrupt in the INT_CLR3 register.
;
;-----------------------------------------------------------------------------
;
;  ARGUMENTS: none
;
;  RETURNS: none
;
;  SIDE EFFECTS:
;    The A and X registers may be modified by this or future implementations
;    of this function.  The same is true for all RAM page pointer registers in
;    the Large Memory Model.  When necessary, it is the calling function's
;    responsibility to perserve their values across calls to fastcall16 
;    functions.
;          
 I2CHW_1_ClearInt:
_I2CHW_1_ClearInt:
    RAM_PROLOGUE RAM_USE_CLASS_1
    push A
    mov A, reg[INT_CLR3]
    and A, ~I2CHW_1_INT_MASK
    mov reg[INT_CLR3], A
    pop A
    RAM_EPILOGUE RAM_USE_CLASS_1
    ret
    
.ENDSECTION

IF (I2CHW_1_MUM_SEL & (I2CHW_1_MSTR | I2CHW_1_MMS))	
.SECTION
;-----------------------------------------------------------------------------
;  FUNCTION NAME: I2CHW_1_EnableMstr
;
;  DESCRIPTION:
;     Enables SDA interrupt allowing start condition detection. Remember to call the
;     global interrupt enable function by using the macro: M8C_EnableGInt.
;
;-----------------------------------------------------------------------------
;
;  ARGUMENTS: none
;
;  RETURNS: none
;
;  SIDE EFFECTS:
;    The A and X registers may be modified by this or future implementations
;    of this function.  The same is true for all RAM page pointer registers in
;    the Large Memory Model.  When necessary, it is the calling function's
;    responsibility to perserve their values across calls to fastcall16 
;    functions.
;          
 I2CHW_1_EnableMstr:
_I2CHW_1_EnableMstr:
    RAM_PROLOGUE RAM_USE_CLASS_1
	;;CDT 28399
	RAM_SETPAGE_CUR >I2CHW_1_bStatus
	and [I2CHW_1_bStatus], ~0x80 ;; ~I2CHW_1_ISR_ACTIVE
	RAM_SETPAGE_CUR >I2CHW_1_RsrcStatus
    and    [I2CHW_1_RsrcStatus], ~0x80;;~I2CHW_ISR_ACTIVE        ; Make sure internal control variables weren't corrupted previous to start.
    BitSetI2CHW_1_CFG I2C_M_EN                                       ;Enable SDA interupt
    RAM_EPILOGUE RAM_USE_CLASS_1
    ret

.ENDSECTION
ENDIF

IF (I2CHW_1_MUM_SEL & (I2CHW_1_SLAVE | I2CHW_1_MMS))
.SECTION
;-----------------------------------------------------------------------------
;  FUNCTION NAME: I2CHW_1_EnableSlave
;
;  DESCRIPTION:
;     Enables SDA interrupt allowing start condition detection. Remember to call the
;     global interrupt enable function by using the macro: M8C_EnableGInt.
;
;-----------------------------------------------------------------------------
;
;  ARGUMENTS: none
;
;  RETURNS: none
;
;  SIDE EFFECTS:
;    The A and X registers may be modified by this or future implementations
;    of this function.  The same is true for all RAM page pointer registers in
;    the Large Memory Model.  When necessary, it is the calling function's
;    responsibility to perserve their values across calls to fastcall16 
;    functions.
;          
 I2CHW_1_EnableSlave:
_I2CHW_1_EnableSlave:
    RAM_PROLOGUE RAM_USE_CLASS_1
    
    M8C_SetBank1 ;The SDA and SCL pins are setting to Hi-z drive mode
    and reg[I2CHW_1SDA_DriveMode_0_ADDR],~(I2CHW_1SDA_MASK|I2CHW_1SCL_MASK)
    or  reg[I2CHW_1SDA_DriveMode_1_ADDR], (I2CHW_1SDA_MASK|I2CHW_1SCL_MASK)
    M8C_SetBank0
    or  reg[I2CHW_1SDA_DriveMode_2_ADDR], (I2CHW_1SDA_MASK|I2CHW_1SCL_MASK)
   
    BitSetI2CHW_1_CFG I2C_S_EN                                       ;Enable SDA interrupt
    nop
    nop
    nop
    nop
    nop
   
    M8C_SetBank1 ;The SDA and SCL pins are restored to Open Drain Low drive mode
    or reg[I2CHW_1SDA_DriveMode_0_ADDR], (I2CHW_1SDA_MASK|I2CHW_1SCL_MASK)
    or reg[I2CHW_1SDA_DriveMode_1_ADDR], (I2CHW_1SDA_MASK|I2CHW_1SCL_MASK)
    M8C_SetBank0
    or reg[I2CHW_1SDA_DriveMode_2_ADDR], (I2CHW_1SDA_MASK|I2CHW_1SCL_MASK)
    
    RAM_EPILOGUE RAM_USE_CLASS_1
    ret

.ENDSECTION
ENDIF

.SECTION
;-----------------------------------------------------------------------------
;  FUNCTION NAME: I2CHW_1_DisableInt
;  FUNCTION NAME: I2CHW_1_Stop
;
;  DESCRIPTION:
;     Disables I2CHW_1 slave by disabling SDA interrupt
;
;-----------------------------------------------------------------------------
;
;  ARGUMENTS: none
;
;  RETURNS: none
;
;  SIDE EFFECTS:
;    The A and X registers may be modified by this or future implementations
;    of this function.  The same is true for all RAM page pointer registers in
;    the Large Memory Model.  When necessary, it is the calling function's
;    responsibility to perserve their values across calls to fastcall16 
;    functions.
;          
 I2CHW_1_DisableInt:
_I2CHW_1_DisableInt:
 I2CHW_1_Stop:
_I2CHW_1_Stop:
    RAM_PROLOGUE RAM_USE_CLASS_1
    M8C_DisableIntMask I2CHW_1_INT_REG, I2CHW_1_INT_MASK
    RAM_EPILOGUE RAM_USE_CLASS_1
    ret

.ENDSECTION

IF (I2CHW_1_MUM_SEL & (I2CHW_1_SLAVE | I2CHW_1_MMS))
.SECTION
;-----------------------------------------------------------------------------
;  FUNCTION NAME: I2CHW_1_DisableSlave
;
;  DESCRIPTION:
;     Disables I2CHW_1 slave by disabling SDA interrupt
;
;-----------------------------------------------------------------------------
;
;  ARGUMENTS: none
;
;  RETURNS: none
;
;  SIDE EFFECTS:
;    The A and X registers may be modified by this or future implementations
;    of this function.  The same is true for all RAM page pointer registers in
;    the Large Memory Model.  When necessary, it is the calling function's
;    responsibility to perserve their values across calls to fastcall16 
;    functions.
;          
 I2CHW_1_DisableSlave:
_I2CHW_1_DisableSlave:
    RAM_PROLOGUE RAM_USE_CLASS_1
    BitClrI2CHW_1_CFG I2C_S_EN                                       ;Disable the Slave
    RAM_EPILOGUE RAM_USE_CLASS_1
    ret

.ENDSECTION
ENDIF

IF (I2CHW_1_MUM_SEL & (I2CHW_1_MSTR | I2CHW_1_MMS))
.SECTION
;-----------------------------------------------------------------------------
;  FUNCTION NAME: I2CHW_1_DisableMstr
;
;  DESCRIPTION:
;     Disables I2CHW_1 slave by disabling SDA interrupt
;
;-----------------------------------------------------------------------------
;
;  ARGUMENTS: none
;
;  RETURNS: none
;
;  SIDE EFFECTS:
;    The A and X registers may be modified by this or future implementations
;    of this function.  The same is true for all RAM page pointer registers in
;    the Large Memory Model.  When necessary, it is the calling function's
;    responsibility to perserve their values across calls to fastcall16 
;    functions.
;          
 I2CHW_1_DisableMstr:
_I2CHW_1_DisableMstr:
    RAM_PROLOGUE RAM_USE_CLASS_1
    BitClrI2CHW_1_CFG I2C_M_EN                                       ;Disable the Master
    RAM_EPILOGUE RAM_USE_CLASS_1
   ret

.ENDSECTION
ENDIF

; End of File I2CHW_1.asm
