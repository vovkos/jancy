;
; ported from: reactos\sdk\lib\crt\setjmp\amd64\setjmp.s
;

section .text

    JUMP_BUFFER_Frame  equ  0    ; 0x00
    JUMP_BUFFER_Rbx    equ  8    ; 0x08
    JUMP_BUFFER_Rsp    equ 16    ; 0x10
    JUMP_BUFFER_Rbp    equ 24    ; 0x18
    JUMP_BUFFER_Rsi    equ 32    ; 0x20
    JUMP_BUFFER_Rdi    equ 40    ; 0x28
    JUMP_BUFFER_R12    equ 48    ; 0x30
    JUMP_BUFFER_R13    equ 56    ; 0x38
    JUMP_BUFFER_R14    equ 64    ; 0x40
    JUMP_BUFFER_R15    equ 72    ; 0x48
    JUMP_BUFFER_Rip    equ 80    ; 0x50
    JUMP_BUFFER_Spare  equ 88    ; 0x58
    JUMP_BUFFER_Xmm6   equ 96    ; 0x60
    JUMP_BUFFER_Xmm7   equ 112   ; 0x70
    JUMP_BUFFER_Xmm8   equ 128   ; 0x80
    JUMP_BUFFER_Xmm9   equ 144   ; 0x90
    JUMP_BUFFER_Xmm10  equ 160   ; 0xa0
    JUMP_BUFFER_Xmm11  equ 176   ; 0xb0
    JUMP_BUFFER_Xmm12  equ 192   ; 0xc0
    JUMP_BUFFER_Xmm13  equ 208   ; 0xd0
    JUMP_BUFFER_Xmm14  equ 224   ; 0xe0
    JUMP_BUFFER_Xmm15  equ 240   ; 0xf0

;
;  int
;  jnc_setJmp(jmp_buf env);
;
;  \param   <rcx> - jmp_buf buffer
;  \return  0
;  \note    Sets up the jmp_buf
;

global jnc_setJmp
jnc_setJmp:

    ; .endprolog

    ; Load rsp as it was before the call into rax
    lea rax, [rsp + 8]
    ; Load return address into r8
    mov r8, [rsp]
    mov qword [rcx + JUMP_BUFFER_Frame], 0
    mov [rcx + JUMP_BUFFER_Rbx], rbx
    mov [rcx + JUMP_BUFFER_Rbp], rbp
    mov [rcx + JUMP_BUFFER_Rsi], rsi
    mov [rcx + JUMP_BUFFER_Rdi], rdi
    mov [rcx + JUMP_BUFFER_R12], r12
    mov [rcx + JUMP_BUFFER_R13], r13
    mov [rcx + JUMP_BUFFER_R14], r14
    mov [rcx + JUMP_BUFFER_R15], r15
    mov [rcx + JUMP_BUFFER_Rsp], rax
    mov [rcx + JUMP_BUFFER_Rip], r8
    movdqa [rcx + JUMP_BUFFER_Xmm6], xmm6
    movdqa [rcx + JUMP_BUFFER_Xmm7], xmm7
    movdqa [rcx + JUMP_BUFFER_Xmm8], xmm8
    movdqa [rcx + JUMP_BUFFER_Xmm9], xmm9
    movdqa [rcx + JUMP_BUFFER_Xmm10], xmm10
    movdqa [rcx + JUMP_BUFFER_Xmm11], xmm11
    movdqa [rcx + JUMP_BUFFER_Xmm12], xmm12
    movdqa [rcx + JUMP_BUFFER_Xmm13], xmm13
    movdqa [rcx + JUMP_BUFFER_Xmm14], xmm14
    movdqa [rcx + JUMP_BUFFER_Xmm15], xmm15
    xor rax, rax
    ret

;
;  void
;  jnc_longJmp(
;       jmp_buf env,
;       int value
;   );
;
;  \param    <rcx> - jmp_buf setup by _setjmp
;  \param    <rdx> - int     value to return.
;  \return   Doesn't return
;  \note     Non-local goto
;

global jnc_longJmp
jnc_longJmp:

    ; .endprolog

    ; FIXME: handle frame

    mov rbx, [rcx + JUMP_BUFFER_Rbx]
    mov rbp, [rcx + JUMP_BUFFER_Rbp]
    mov rsi, [rcx + JUMP_BUFFER_Rsi]
    mov rdi, [rcx + JUMP_BUFFER_Rdi]
    mov r12, [rcx + JUMP_BUFFER_R12]
    mov r13, [rcx + JUMP_BUFFER_R13]
    mov r14, [rcx + JUMP_BUFFER_R14]
    mov r15, [rcx + JUMP_BUFFER_R15]
    mov rsp, [rcx + JUMP_BUFFER_Rsp]
    mov r8, [rcx + JUMP_BUFFER_Rip]
    movdqa xmm6, [rcx + JUMP_BUFFER_Xmm6]
    movdqa xmm7, [rcx + JUMP_BUFFER_Xmm7]
    movdqa xmm8, [rcx + JUMP_BUFFER_Xmm8]
    movdqa xmm9, [rcx + JUMP_BUFFER_Xmm9]
    movdqa xmm10, [rcx + JUMP_BUFFER_Xmm10]
    movdqa xmm11, [rcx + JUMP_BUFFER_Xmm11]
    movdqa xmm12, [rcx + JUMP_BUFFER_Xmm12]
    movdqa xmm13, [rcx + JUMP_BUFFER_Xmm13]
    movdqa xmm14, [rcx + JUMP_BUFFER_Xmm14]
    movdqa xmm15, [rcx + JUMP_BUFFER_Xmm15]

    ; return param2 or 1 if it was 0
    mov rax, rdx
    test rax, rax
    jnz l2
    inc rax
l2: jmp r8
