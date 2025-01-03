    .global long_func
    .align 0x1000
long_func:
    .cfi_startproc
    xor %rax, %rax
    .rept 5000
        inc %rax
    .endr
    ret
    .cfi_endproc

    .text
    __num_iterations = 1000000
    .global long_loop
    .align 0x1000
long_loop:
    .cfi_startproc
    xor %rax, %rax
1:
    inc %rax
    jmp 2f
    .rept 6000
        nop
    .endr
2:
    cmpq $__num_iterations, %rax
    jl 1b
    ret
    .cfi_endproc
