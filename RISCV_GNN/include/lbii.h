/*
 ============================================================================
 Name        : lbii.h
 Author      : Randolph
 Version     :
 Copyright   : Your copyright notice
 Description : lbii Headfile
 ============================================================================
 */
#ifndef __LBII_H__
#define __LBII_H__

//调用地址
#define LBR_REG_ADDR(n)                       (LBR_BASE_REG_ADDR | n)
#define LBR_SRAM_ADDR(n)                      (LBR_BASE | n)
//寄存器基础地址定义
#define LBR_BASE                              0x40000000
#define LBR_BASE_REG_ADDR                     0x4000C000

//CPU_TRANS和CPU_RCV寄存器地址定义
#define TRANS_BASE_ADDR                       0x30
#define RECV_BASE_ADDR                        0x34

//Others
#define LBR_DST                               0x00
#define LBR_SRC                               0x04
#define LBR_LOCAL                             0x08
#define LB_XY_SRC_CPU                         0x0C
#define SP_DATA_LOW                           0x10
#define SP_DATA_HIGH                          0x14
#define LBR_BASE_ADDR                         0x18
#define LBR_XY_SRC                            0x20
#define CPU_CODE                              0x24
#define CPU_STATUS                            0x28
#define TEST_REG                              0x2C // Use this for node's static ID?

//TRANS硬件SRAM地址
#define LBR_TRANS_SRAM_0                      0xA000
#define LBR_TRANS_SRAM_1                      0xB000

//RECV硬件SRAM地址
#define LBR_RECV_SRAM_0                 	  0x8000
#define LBR_RECV_SRAM_1                       0x9000

//硬件可操作地址空间
#define RISCV_SRAM_BASE                       0x30600000
#define NPU0_BASE                             0x30000000
#define NPU1_BASE                             0x31000000
#define NPU2_BASE                             0x32000000
#define NPU3_BASE                             0x33000000
//硬件调用地址
#define RISCV_SRAM_BASE_ADDR(n)               (RISCV_SRAM_BASE + n*sizeof(uint32_t))


#define readReg32_cpu_rcv() (read_reg32((reg32_t)LBR_REG_ADDR(RECV_BASE_ADDR)))
#define writeReg32_cpu_rcv(value) (write_reg32((reg32_t)LBR_REG_ADDR(RECV_BASE_ADDR), value))
#define writeReg32_cpu_rcv_mask_down() (write_reg32((reg32_t)LBR_REG_ADDR(RECV_BASE_ADDR), readReg32_cpu_rcv() & 0xfffff7ff))
#define writeReg32_cpu_rcv_mask_high() (write_reg32((reg32_t)LBR_REG_ADDR(RECV_BASE_ADDR), readReg32_cpu_rcv() | 0x00000800))
#define readMemory32_cpu_rcv_sram0(addr) (read_reg32(LBR_SRAM_ADDR(LBR_RECV_SRAM_0) + addr))
#define readMemory32_cpu_rcv_sram1(addr) (read_reg32(LBR_SRAM_ADDR(LBR_RECV_SRAM_1) + addr))
#define readMemory32(addr, offset) (read_reg32((reg32_t)addr + offset))



#endif