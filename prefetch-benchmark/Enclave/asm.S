/*
 *  This file is part of the SGX-Step enclave execution control framework.
 *
 *  Copyright (C) 2017 Jo Van Bulck <jo.vanbulck@cs.kuleuven.be>,
 *                     Raoul Strackx <raoul.strackx@cs.kuleuven.be>
 *
 *  SGX-Step is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  SGX-Step is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with SGX-Step. If not, see <http://www.gnu.org/licenses/>.
 */
    .section .data
    .global dummy_array

dummy_array:
    .fill 0x100, 1, 0x55 

    .text
    .global enclave_call
    .type enclave_call,@function
enclave_call:
    xor %r9, %r9
1: 
    cmp %r9, %rsi
    je 2f    
    
    ud2
    # Execute the RDTSC instruction
    rdtsc

    # Combine the high and low parts into a 64-bit integer
    shlq $32, %rdx       # Shift the high 32 bits left by 32 positions
    orq %rdx, %rax       # Combine the high and low parts into RAX

    # Store the 64-bit TSC value into rdtsc_value
    movq %rax, (%rdi)

    inc %r9
    jmp 1b

2:
    ret
