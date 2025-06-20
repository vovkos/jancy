	AREA |.text|, CODE, READONLY
	ALIGN 4

	EXPORT jnc_setJmp
	EXPORT jnc_longJmp

#define JUMP_BUFFER_Frame     0x00
#define JUMP_BUFFER_Reserved  0x08
#define JUMP_BUFFER_X19       0x10
#define JUMP_BUFFER_X20       0x18
#define JUMP_BUFFER_X21       0x20
#define JUMP_BUFFER_X22       0x28
#define JUMP_BUFFER_X23       0x30
#define JUMP_BUFFER_X24       0x38
#define JUMP_BUFFER_X25       0x40
#define JUMP_BUFFER_X26       0x48
#define JUMP_BUFFER_X27       0x50
#define JUMP_BUFFER_X28       0x58
#define JUMP_BUFFER_Fp        0x60
#define JUMP_BUFFER_Lr        0x68
#define JUMP_BUFFER_Sp        0x70
#define JUMP_BUFFER_Fpcr      0x78
#define JUMP_BUFFER_Fpsr      0x7c
#define JUMP_BUFFER_D8        0x80
#define JUMP_BUFFER_D9        0x88
#define JUMP_BUFFER_D10       0x90
#define JUMP_BUFFER_D11       0x98
#define JUMP_BUFFER_D12       0xa0
#define JUMP_BUFFER_D13       0xa8
#define JUMP_BUFFER_D14       0xb0
#define JUMP_BUFFER_D15       0xb8

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
	csel    x0,     x1,     x0,     ne
    ret

	END
