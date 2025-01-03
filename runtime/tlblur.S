        # TLBlur vTLB/shadow page table update, called by instrumentation
        # 
        # args: %rdi = accessed address
        # %rax used as a scratch register
        __tlblur_pt_size = 0x1000000
        .global tlblur_tlb_update
tlblur_tlb_update:
        # Index in software TLB %rdi = ((%rdi - <enclave base>) >> 9) & (<size> - 1)
        lea     __ImageBase(%rip), %rax
        sub     %rax, %rdi
        shr     $12, %rdi
        shl     $3, %rdi
        and     $__tlblur_pt_size - 1, %rdi

        # Then update the software TLB
        leaq    __tlblur_shadow_pt(%rip), %rax
        add     %rax, %rdi

        # Increment global counter, and store it in %rax
        movq    __tlblur_global_counter(%rip), %rax

        # Update shadow page table by storing global counter
        movq    %rax, (%rdi)

        incq    __tlblur_global_counter(%rip)

        ret

        .data

        .global __tlblur_global_counter
__tlblur_global_counter:
        .quad   10000000

        .global __tlblur_global_counter_backup
__tlblur_global_counter_backup:
        .quad   0

        .global __tlblur_global_code_counter
__tlblur_global_code_counter:
        .quad   0

        .align 0x1000   # 4KiB
        .global __tlblur_shadow_pt
__tlblur_shadow_pt:
        .zero   __tlblur_pt_size
__tlblur_shadow_pt_end:

.section .tlblur.endtext, "ax", @progbits
.global __tlblur_endtext
__tlblur_endtext:
        nop
