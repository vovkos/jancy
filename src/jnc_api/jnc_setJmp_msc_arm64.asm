	AREA |.text|, CODE, READONLY
	ALIGN 4

	EXPORT jnc_setJmp
	EXPORT jnc_longJmp

JUMP_BUFFER_Frame     EQU 0x00
JUMP_BUFFER_Reserved  EQU 0x08
JUMP_BUFFER_X19       EQU 0x10
JUMP_BUFFER_X20       EQU 0x18
JUMP_BUFFER_X21       EQU 0x20
JUMP_BUFFER_X22       EQU 0x28
JUMP_BUFFER_X23       EQU 0x30
JUMP_BUFFER_X24       EQU 0x38
JUMP_BUFFER_X25       EQU 0x40
JUMP_BUFFER_X26       EQU 0x48
JUMP_BUFFER_X27       EQU 0x50
JUMP_BUFFER_X28       EQU 0x58
JUMP_BUFFER_Fp        EQU 0x60
JUMP_BUFFER_Lr        EQU 0x68
JUMP_BUFFER_Sp        EQU 0x70
JUMP_BUFFER_Fpcr      EQU 0x78
JUMP_BUFFER_Fpsr      EQU 0x7c
JUMP_BUFFER_D8        EQU 0x80
JUMP_BUFFER_D9        EQU 0x88
JUMP_BUFFER_D10       EQU 0x90
JUMP_BUFFER_D11       EQU 0x98
JUMP_BUFFER_D12       EQU 0xa0
JUMP_BUFFER_D13       EQU 0xa8
JUMP_BUFFER_D14       EQU 0xb0
JUMP_BUFFER_D15       EQU 0xb8

;
;  int
;  jnc_setJmp(jmp_buf env);
;
;  \param   <x0> - jmp_buf buffer
;  \return  0
;  \note    Sets up the jmp_buf
;

jnc_setJmp
	str     x29,    [x0, #JUMP_BUFFER_Frame]
	mov     x1,     #0
	str     x1,     [x0, #JUMP_BUFFER_Reserved]

	stp     x19,    x20,    [x0, #JUMP_BUFFER_X19]
	stp     x21,    x22,    [x0, #JUMP_BUFFER_X21]
	stp     x23,    x24,    [x0, #JUMP_BUFFER_X23]
	stp     x25,    x26,    [x0, #JUMP_BUFFER_X25]
	stp     x27,    x28,    [x0, #JUMP_BUFFER_X27]

	str     x29,    [x0, #JUMP_BUFFER_Fp]
	str     x30,    [x0, #JUMP_BUFFER_Lr]
	mov     x1,     sp
	str     x1,     [x0, #JUMP_BUFFER_Sp]

	mrs     x1,     fpcr
	str     w1,     [x0, #JUMP_BUFFER_Fpcr]
	mrs     x1,     fpsr
	str     w1,     [x0, #JUMP_BUFFER_Fpsr]

	str     d8,     [x0, #JUMP_BUFFER_D8]
	str     d9,     [x0, #JUMP_BUFFER_D9]
	str     d10,    [x0, #JUMP_BUFFER_D10]
	str     d11,    [x0, #JUMP_BUFFER_D11]
	str     d12,    [x0, #JUMP_BUFFER_D12]
	str     d13,    [x0, #JUMP_BUFFER_D13]
	str     d14,    [x0, #JUMP_BUFFER_D14]
	str     d15,    [x0, #JUMP_BUFFER_D15]

	mov     x0,     #0
	ret

;
;  void
;  jnc_longJmp(
;       jmp_buf env,
;       int value
;   );
;
;  \param    <x0> - jmp_buf setup by _setjmp
;  \param    <x1> - int value to return.
;  \return   Doesn't return
;  \note     Non-local goto
;

jnc_longJmp
    ldp     x19,    x20,    [x0, #JUMP_BUFFER_X19]
    ldp     x21,    x22,    [x0, #JUMP_BUFFER_X21]
    ldp     x23,    x24,    [x0, #JUMP_BUFFER_X23]
    ldp     x25,    x26,    [x0, #JUMP_BUFFER_X25]
    ldp     x27,    x28,    [x0, #JUMP_BUFFER_X27]

    ldr     x29,    [x0, #JUMP_BUFFER_Fp]
    ldr     x30,    [x0, #JUMP_BUFFER_Lr]
    ldr     x2,     [x0, #JUMP_BUFFER_Sp]
    mov     sp,     x2

    ldr     w2,     [x0, #JUMP_BUFFER_Fpcr]
    msr     fpcr,   x2
    ldr     w2,     [x0, #JUMP_BUFFER_Fpsr]
    msr     fpsr,   x2

    ldr     d8,     [x0, #JUMP_BUFFER_D8]
    ldr     d9,     [x0, #JUMP_BUFFER_D9]
    ldr     d10,    [x0, #JUMP_BUFFER_D10]
    ldr     d11,    [x0, #JUMP_BUFFER_D11]
    ldr     d12,    [x0, #JUMP_BUFFER_D12]
    ldr     d13,    [x0, #JUMP_BUFFER_D13]
    ldr     d14,    [x0, #JUMP_BUFFER_D14]
    ldr     d15,    [x0, #JUMP_BUFFER_D15]

    cmp     x1,     #0
	mov     x0,     #1
	cselne   x0,     x1,     x0
    ret

	END
