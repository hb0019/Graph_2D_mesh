# SPDX-License-Identifier: Apache-2.0
# Copyright 2020 Western Digital Corporation or its affiliates.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
// startup code to support HLL programs

//#include "defines.h"
#include <bits.h>
.macro TRAP_ENTRY
  addi sp, sp, -16*REGBYTES

  STORE x1,  0*REGBYTES(sp)
  STORE x5,  1*REGBYTES(sp)
  STORE x6,  2*REGBYTES(sp)
  STORE x7,  3*REGBYTES(sp)
  STORE x10, 4*REGBYTES(sp)
  STORE x11, 5*REGBYTES(sp)
  STORE x12, 6*REGBYTES(sp)
  STORE x13, 7*REGBYTES(sp)
  STORE x14, 8*REGBYTES(sp)
  STORE x15, 9*REGBYTES(sp)
  STORE x16, 10*REGBYTES(sp)
  STORE x17, 11*REGBYTES(sp)
  STORE x28, 12*REGBYTES(sp)
  STORE x29, 13*REGBYTES(sp)
  STORE x30, 14*REGBYTES(sp)
  STORE x31, 15*REGBYTES(sp)
.endm

#restore caller registers
.macro TRAP_EXIT
## Remain in M-mode after mret
#  li t0, MSTATUS_MPP
#  csrs mstatus, t0

  LOAD x1,  0*REGBYTES(sp)
  LOAD x5,  1*REGBYTES(sp)
  LOAD x6,  2*REGBYTES(sp)
  LOAD x7,  3*REGBYTES(sp)
  LOAD x10, 4*REGBYTES(sp)
  LOAD x11, 5*REGBYTES(sp)
  LOAD x12, 6*REGBYTES(sp)
  LOAD x13, 7*REGBYTES(sp)
  LOAD x14, 8*REGBYTES(sp)
  LOAD x15, 9*REGBYTES(sp)
  LOAD x16, 10*REGBYTES(sp)
  LOAD x17, 11*REGBYTES(sp)
  LOAD x28, 12*REGBYTES(sp)
  LOAD x29, 13*REGBYTES(sp)
  LOAD x30, 14*REGBYTES(sp)
  LOAD x31, 15*REGBYTES(sp)

  addi sp, sp, 16*REGBYTES
  mret
.endm

.macro enable_interrupt id
enable_interrupt_\@:
  li tp, (0xe0002000+(\id << 2))  //RV_PIC_MEIE
  li t0, 1
  sw t0, 0(tp)
.endm


.macro set_threshold threshold
set_threshold_\@:
  li tp, 0xe0003004  //RV_PIC_MEIPT
  li t0, \threshold
  sw t0, 0(tp)
.endm

.macro set_priority id,priority
set_priority_\@:
  li tp, (0xe0000000 + (\id << 2))  //RV_PIC_MEIPL
  li t0, \priority
  sw t0, 0(tp)
.endm

.section .text.init
.global _start
_start:
// Region 0x1 (Address 0x1000_1000 ~ 0x1fff_ffff, Boot QSPI Flash)  -> Both No
// Region 0x2 (Address 0x2000_0000 ~ 0x2fff_ffff, External Devices) -> Sideeffect Yes
// Region 0xF (Address 0xf000_0000 ~ 0xffff_ffff, DCCM)             -> Both No
// Other Region -> Cacheable Yes
        li t0, 0x15555561
        csrw 0x7c0, t0

        la sp, STACK

/*        //change sys clk to 500MHz
        li tp, 0x2000000C  
        li t1, 0x80000000 
        lw t0, 0(tp)
        or t0,t0,t1
        sw t0, 0(tp)
*/
    //copy data section
    la t0, __erodata
    la t1, __data_start__
    la t2, __bss_start__

    sub t2,t2, t1
    li tp, 0
    beq t2, tp , L_loop0_done

L_loop0:
	lw tp, 0(t0)
	sw tp, 0(t1)
	addi t0,t0, 4
	addi t1,t1, 4
    li tp, 4
	sub  t2,t2, tp
    li tp, 0
	bne t2,tp,L_loop0
	
L_loop0_done:
//clean BSS
	lw t0, __bss_start__
	lw t1, __bss_end__
	
	li tp, 0
	sub t1,t1, t0
	beq t1, tp,L_loop1_done
	
L_loop1:
	li tp, 0
	sw tp, 0(t0)
	addi t0,t0, 4
	li tp, 4
	sub t1,t1,tp
	li tp, 0
	bne t1,tp, L_loop1
	
L_loop1_done:
	li t0, 0
	li t1, 0
	li t2, 0
	li tp, 0





        la t0, trap_entry
        csrw mtvec, t0
        call main
  .align 10

trap_entry:
  TRAP_ENTRY

  csrr a0, mcause
  csrr a1, mepc
  call handle_trap
  csrw mepc, a0

  TRAP_EXIT

.weak handle_trap
handle_trap:
1:
  j 1b


.global _finish
_finish:
        la t0, tohost
        li t1, 0xff
        sb t1, 0(t0) // DemoTB test termination
        li t1, 1
        sw t1, 0(t0) // Whisper test termination
        beq x0, x0, _finish
        .rept 10
        nop
        .endr

.section .data.io
.global tohost
tohost: .word 0

