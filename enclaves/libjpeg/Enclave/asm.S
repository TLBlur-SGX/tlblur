    .text
    .global all_zeros
    .align 0x1000   /* 4KiB */
all_zeros:
    retq   
    .space 0x1000   /* 4KiB */

    .text
    .global islow
    .align 0x1000   /* 4KiB */
islow:
    retq  
    .space 0x1000   /* 4KiB */
