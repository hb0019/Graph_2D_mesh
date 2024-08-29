/*
 ============================================================================
 Name        : matrix_tool.h
 Author      : huangbin
 Version     :
 Copyright   : Your copyright notice
 Description : matrix Headfile
 ============================================================================
 */
#ifndef MATRIX_TOOL_H
#define MATRIX_TOOL_H

#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include "top_env.h"

//NPU基础地址
#define NPU_BASE_0              0x30000000
#define NPU_BASE_1              0x31000000
#define NPU_BASE_2              0x32000000
#define NPU_BASE_3              0x33000000
#define NPU_SRAM_BASE           0x5e0000
//NPU_CSR调用地址
#define NPU_CSR_ADDR(base, n)   (base | n)

//寄存器名称
#define CSR_RUN                 0x00000038

//npu_mode
#define BIC_MODE                0b00
#define GEMM_MODE               0b01
#define GEMPM_MODE              0b10
#define GEMA_MODE               0b11

//npu sram 大小
#define NPU_SRAM_SIZE           4096

//npu_irq
#define NPU_IRQ(base)           (read_reg32((reg32_t)NPU_CSR_ADDR(base, 0x00000048))) & 0x80000000
//set npu Model
#define set_npu_mode(base, mode)  (write_reg32((reg32_t)NPU_CSR_ADDR(base, 0x04), ((read_reg32((reg32_t)NPU_CSR_ADDR(base, 0x04)))&0xfffffe7f)|(mode<<7)))

struct bpu_matrix{
    uint32_t staddr;
    uint32_t length;
};

struct addr_offset{
    uint32_t a_offset;
    uint32_t b_offset;
};

static void npu_nop(int npu_base){
    while (!(NPU_IRQ(npu_base))){
    }
    return;
}

void GEM_csr_init(int npu_base){
    // staddr 表示偏移地址，左移5位后在加上基地址为实际的物理地址
    //model_CFG0
    PISA_W32(REG32(NPU_CSR_ADDR(npu_base, 0x00)), 0x81fe00ff);      
    PISA_W32(REG32(NPU_CSR_ADDR(npu_base, 0x04)), 0x80);            // [40:39] npu_mode 
    //model_CFG1
    PISA_W32(REG32(NPU_CSR_ADDR(npu_base, 0x08)), 0xfff404da);      // [19:10] matrix_calcu245_staddr=0x101=257; [9:0] matrix_calcu245_length=0xda=218 
    PISA_W32(REG32(NPU_CSR_ADDR(npu_base, 0x0c)), 0x1f);            
    //model_CFG2
    PISA_W32(REG32(NPU_CSR_ADDR(npu_base, 0x10)), NPU_CSR_ADDR(npu_base, NPU_SRAM_BASE));      // [31:0]  syn_update_base_addr
    PISA_W32(REG32(NPU_CSR_ADDR(npu_base, 0x14)), NPU_CSR_ADDR(npu_base, NPU_SRAM_BASE));      // [63:32] syn_base_addr 
    //model_CFG3
    PISA_W32(REG32(NPU_CSR_ADDR(npu_base, 0x18)), 0x632040e);       // [19:10] matrix_calcu1_staddr=0x81=129; [9:0] matrix_calcu1_length=0xe=14
    PISA_W32(REG32(NPU_CSR_ADDR(npu_base, 0x1c)), 0x0);             
    //model_CFG4
    PISA_W32(REG32(NPU_CSR_ADDR(npu_base, 0x20)), 0xfff);           // [39:20] matrix_A_staddr=0x0=0; [19:0] matrix_A_length=0xfff=4095
    PISA_W32(REG32(NPU_CSR_ADDR(npu_base, 0x24)), 0x8041100);       // [59:50] matrix_calcu3_staddr=0x1=513; [49:40] matrix_calcu3_length=0x11=17
    //HW_CFG0
    PISA_W32(REG32(NPU_CSR_ADDR(npu_base, 0x28)), 0x320100);
    PISA_W32(REG32(NPU_CSR_ADDR(npu_base, 0x2c)), 0x802008);
    //MAT_CFG
    PISA_W32(REG32(NPU_CSR_ADDR(npu_base, 0x30)), 0x9000007);       // [31:12] matrix_C_staddr=0x9000=36864; [11:0] matrix_C_length=0x7=7
    PISA_W32(REG32(NPU_CSR_ADDR(npu_base, 0x34)), 0x1000fff);       // [63:44] matrix_B_staddr=0x1000=4096; [43:32] matrix_B_length=0x7ff=2047
    //CSR_RUN
    PISA_W32(REG32(NPU_CSR_ADDR(npu_base, 0x38)), 0x0);             // [16] matrix_A_offset_clr; [15] matrix_B_offset_clr; [14] matrix_C_offset_clr; [13] ncu_addr_clr; 
                                                                    // [4] step_en; [3] matrix_calcu_s1_en; [2] matrix_calcu_s2_en; [1] matrix_calcu_s3_en; [0] bic_sim_start
    PISA_W32(REG32(NPU_CSR_ADDR(npu_base, 0x3c)), 0x1);             // [41:32] step_num=0x1=1
    return;
}

struct addr_offset data_load(int npu_base, char type, uint32_t *matrix_A, int A_col, int A_row, uint32_t *matrix_B, int B_col, int B_row){
    struct addr_offset addr_offset;
    uint32_t offset = 0;
    // 导入matrix_A
    for (int i=0; i<A_col; i++){
        for (int j=0; j<A_row; j++){
            int offset_addr = NPU_SRAM_BASE+offset;
            PISA_W32(REG32(NPU_CSR_ADDR(npu_base, offset_addr)), *(uint32_t *)(matrix_A + i*(A_row) + j));
            offset = offset + 4;
        }
        // 维度不足8的填充0值
        // if (A_row%8!=0){
        //     for (int k=0; k<8-A_row%8; k++){
        //         int offset_addr = NPU_SRAM_BASE+offset;
        //         PISA_W32(REG32(NPU_CSR_ADDR(npu_base, offset_addr)), 0x0);
        //         offset = offset + 4;
        //     }
        // }
    }
    addr_offset.a_offset = offset;
    if (type != 'M'){
        for (int i=0; i<B_col; i++){
            for (int j=0; j<B_row; j++){
                int offset_addr = NPU_SRAM_BASE+offset;
                PISA_W32(REG32(NPU_CSR_ADDR(npu_base, offset_addr)), *(uint32_t *)(matrix_B + i*(B_row) + j));
                offset = offset + 4;
            }
            // 维度不足8的填充0值
            // if (A_row%8!=0){
            //     for (int k=0; k<8-A_row%8; k++){
            //         int offset_addr = NPU_SRAM_BASE+offset;
            //         PISA_W32(REG32(NPU_CSR_ADDR(npu_base, offset_addr)), 0x0);
            //         offset = offset + 4;
            //     }
            // }
        }
    } else {
        // 转置
        for (int i=0; i<B_row; i++){
            for (int j=0; j<B_col; j++){
                int offset_addr = NPU_SRAM_BASE+offset;
                PISA_W32(REG32(NPU_CSR_ADDR(npu_base, offset_addr)), *(uint32_t *)(matrix_B + j*(B_row) + i));
                offset = offset + 4;
            }
            // TODO: 维度不足8的填充0值
        }
    }
    addr_offset.b_offset = offset;
    return addr_offset;
}

// 通用矩阵加
struct bpu_matrix GEMA_run(int npu_base, uint32_t matrix_A_staddr, uint32_t matrix_A_length, uint32_t matrix_B_staddr, uint32_t matrix_B_length){
    struct bpu_matrix bpu_matrix_C;

    if (matrix_A_length != matrix_B_length){
        printf("Error: matrix_A_length != matrix_B_length\n");
        return bpu_matrix_C;
    }
    int csr_value = 0x00000000;
    // set mode
    set_npu_mode(npu_base, GEMA_MODE);
    // set SMT staddr
    PISA_W32(REG32(NPU_CSR_ADDR(npu_base, 0x08)), 0xfffc040e);      // [19:10] matrix_calcu245_staddr=0x301=769; [9:0] matrix_calcu245_length=0xe=14 
    PISA_W32(REG32(NPU_CSR_ADDR(npu_base, 0x18)), 0x632040e);       // [19:10] matrix_calcu1_staddr=0x81=129; [9:0] matrix_calcu1_length=0xe=14 
   
    // set matrix
    bpu_matrix_C.staddr = matrix_B_staddr + matrix_B_length + 1;
    bpu_matrix_C.length = matrix_B_length;
    int step_num=1;
    if (matrix_A_length >= NPU_SRAM_SIZE){
        step_num = matrix_A_length / NPU_SRAM_SIZE + 1;
        matrix_A_length = NPU_SRAM_SIZE;
        matrix_B_length = NPU_SRAM_SIZE;
    }

    csr_value = 0x00000000;
    csr_value = csr_value | (matrix_A_length & 0x000fffff) | ((matrix_A_staddr<<20) & 0xfff00000);
    PISA_W32(REG32(NPU_CSR_ADDR(npu_base, 0x20)), csr_value);       // [39:20] matrix_A_staddr; [19:0] matrix_A_length

    csr_value = 0x00000000;
    csr_value = csr_value | (matrix_B_length & 0x00000fff) | ((matrix_B_staddr<<12) & 0xfffff000);
    PISA_W32(REG32(NPU_CSR_ADDR(npu_base, 0x34)), csr_value);       // [63:44] matrix_B_staddr; [43:32] matrix_B_length

    csr_value = 0x00000000;
    csr_value = csr_value | (matrix_B_length & 0x00000fff) | ((bpu_matrix_C.staddr<<12) & 0xfffff000);
    PISA_W32(REG32(NPU_CSR_ADDR(npu_base, 0x30)), csr_value);       // [31:12] matrix_C_staddr; [11:0] matrix_C_length

    // enable csr
    csr_value = 0x00000000;
    csr_value = csr_value | 0x201d; //[13] ncu_addr_clr [4] step_en; [3] matrix_calcu_s1_en; [2] matrix_calcu_s2_en; [1] matrix_calcu_s3_en; [0] bic_sim_start 
    PISA_W32(REG32(NPU_CSR_ADDR(npu_base, 0x3c)), step_num);       // [41:32] step_num=0x1=1
    PISA_W32(REG32(NPU_CSR_ADDR(npu_base, 0x38)), csr_value);      // [16] matrix_a_offset_clr; [15] matrix_b_offset_clr; [14] matrix_c_offset_clr;

    npu_nop(npu_base);

    return bpu_matrix_C;
}

// 通用矩阵点乘
struct bpu_matrix GEMPM_run(int npu_base, uint32_t matrix_A_staddr, uint32_t matrix_A_length, uint32_t matrix_B_staddr, uint32_t matrix_B_length){
    struct bpu_matrix bpu_matrix_C;
    if (matrix_A_length != matrix_B_length){
        printf("Error: matrix_A_length != matrix_B_length\n");
        return bpu_matrix_C;
    }
    int csr_value = 0x00000000;
    // set mode
    set_npu_mode(npu_base, GEMPM_MODE);
    // set SMT staddr
    PISA_W32(REG32(NPU_CSR_ADDR(npu_base, 0x08)), 0xfffa040e);      // [19:10] matrix_calcu245_staddr=0x281=641; [9:0] matrix_calcu245_length=0xe=14
    PISA_W32(REG32(NPU_CSR_ADDR(npu_base, 0x18)), 0x632040e);       // [19:10] matrix_calcu1_staddr=0x81=129; [9:0] matrix_calcu1_length=0xe=14
    
    // set matrix
    bpu_matrix_C.staddr = matrix_B_staddr + matrix_B_length + 1;
    bpu_matrix_C.length = matrix_B_length;
    int step_num=1;
    if (matrix_A_length >= NPU_SRAM_SIZE){
        step_num = matrix_A_length / NPU_SRAM_SIZE + 1;
        matrix_A_length = NPU_SRAM_SIZE;
        matrix_B_length = NPU_SRAM_SIZE;
    }

    csr_value = 0x00000000;
    csr_value = csr_value | (matrix_A_length & 0x000fffff) | ((matrix_A_staddr<<20) & 0xfff00000);
    PISA_W32(REG32(NPU_CSR_ADDR(npu_base, 0x20)), csr_value);       // [39:20] matrix_A_staddr; [19:0] matrix_A_length

    csr_value = 0x00000000;
    csr_value = csr_value | (matrix_B_length & 0x00000fff) | ((matrix_B_staddr<<12) & 0xfffff000);
    PISA_W32(REG32(NPU_CSR_ADDR(npu_base, 0x34)), csr_value);       // [63:44] matrix_B_staddr; [43:32] matrix_B_length

    csr_value = 0x00000000;
    csr_value = csr_value | (matrix_B_length & 0x00000fff) | ((bpu_matrix_C.staddr<<12) & 0xfffff000);
    PISA_W32(REG32(NPU_CSR_ADDR(npu_base, 0x30)), csr_value);       // [31:12] matrix_C_staddr; [11:0] matrix_C_length

    //enable csr
    csr_value = 0x00000000;
    csr_value = csr_value | 0x201d; // [4] step_en; [3] matrix_calcu_s1_en; [2] matrix_calcu_s2_en; [1] matrix_calcu_s3_en; [0] bic_sim_start 
    PISA_W32(REG32(NPU_CSR_ADDR(npu_base, 0x3c)), step_num);       // [41:32] step_num=0x1=1
    PISA_W32(REG32(NPU_CSR_ADDR(npu_base, 0x38)), csr_value);

    npu_nop(npu_base);

    return bpu_matrix_C;
}

// 通用矩阵乘
struct bpu_matrix GEMM_run(int npu_base, uint32_t matrix_A_staddr, uint32_t matrix_B_staddr, uint32_t matrix_C_staddr, uint32_t matrix_length, uint32_t iteration, uint32_t step_num){
    struct bpu_matrix bpu_matrix_C;
    int csr_value = 0x00000000;
    // set mode
    set_npu_mode(npu_base, GEMM_MODE);
    // set SMT staddr
    PISA_W32(REG32(NPU_CSR_ADDR(npu_base, 0x08)), 0xfff404da);      // [19:10] matrix_calcu245_staddr=0x101=257; [9:0] matrix_calcu245_length=0xda=218
    PISA_W32(REG32(NPU_CSR_ADDR(npu_base, 0x18)), 0x632040e);       // [19:10] matrix_calcu1_staddr=0x81=129; [9:0] matrix_calcu1_length=0xe=14
    PISA_W32(REG32(NPU_CSR_ADDR(npu_base, 0x24)), 0x8041100);       // [59:50] matrix_calcu3_staddr=0x1=513; [49:40] matrix_calcu3_length=0x11=17

    csr_value = 0x00000000;
    csr_value = csr_value | (matrix_length & 0x000fffff) | ((matrix_A_staddr<<20) & 0xfff00000);
    PISA_W32(REG32(NPU_CSR_ADDR(npu_base, 0x20)), csr_value);       // [39:20] matrix_A_staddr; [19:0] matrix_A_length

    csr_value = 0x00000000;
    csr_value = csr_value | (matrix_length & 0x00000fff) | ((matrix_B_staddr<<12) & 0xfffff000);
    PISA_W32(REG32(NPU_CSR_ADDR(npu_base, 0x34)), csr_value);       // [63:44] matrix_B_staddr; [43:32] matrix_B_length

    bpu_matrix_C.staddr = matrix_C_staddr;
    bpu_matrix_C.length = 7;

    csr_value = 0x00000000;
    csr_value = csr_value | (bpu_matrix_C.length & 0x00000fff) | ((bpu_matrix_C.staddr<<12) & 0xfffff000);
    PISA_W32(REG32(NPU_CSR_ADDR(npu_base, 0x30)), csr_value);       // [31:12] matrix_C_staddr; [11:0] matrix_C_length

    // TODO: iteration and step_num set!!!!!!!!!
    // enable csr 
    for (int i=0; i<iteration; i++){
        // sim1 load_A and B_offset_clr
        csr_value = 0x00000000;
        csr_value = csr_value | 0xa019;     // [15] matrix_B_offset_clr; [13] ncu_addr_clr
        // [4] step_en; [3] matrix_calcu_s1_en; [2] matrix_calcu_s2_en; [1] matrix_calcu_s3_en; [0] bic_sim_start
        PISA_W32(REG32(NPU_CSR_ADDR(npu_base, 0x3c)), 0x1);       // [41:32] step_num=0x1=1
        PISA_W32(REG32(NPU_CSR_ADDR(npu_base, 0x38)), csr_value);
        
        npu_nop(npu_base);

        // sim2 calculation
        csr_value = 0x00000000;
        csr_value = csr_value | 0x2017;     // [4] step_en; [3] matrix_calcu_s1_en; [2] matrix_calcu_s2_en; [1] matrix_calcu_s3_en; [0] bic_sim_start
        PISA_W32(REG32(NPU_CSR_ADDR(npu_base, 0x3c)), step_num);       // [41:32] step_num=0x1=1
        PISA_W32(REG32(NPU_CSR_ADDR(npu_base, 0x38)), csr_value);

        npu_nop(npu_base);
    }
    
    // 更新计算结果的数据长度
    bpu_matrix_C.length = iteration * step_num * 8 - 1;

    return bpu_matrix_C;
}

void GEMA(int npu_base, uint32_t *matrix_A, int A_col, int A_row, uint32_t *matrix_B, int B_col, int B_row){
    // 矩阵尺寸规划与数据搬运
    if (A_col!=B_col || A_row!=B_row){
        PISA_INFO("Error: A_col!=B_col or A_row!=B_row\n");
        return;
    }
    struct addr_offset addr_offset = data_load(npu_base, 'A', matrix_A, A_col, A_row, matrix_B, B_col, B_row);
    // 矩阵计算
    struct bpu_matrix bpu_matrix_C = GEMA_run(npu_base, 0, (addr_offset.a_offset>>5)-1, addr_offset.a_offset>>5, ((addr_offset.b_offset-addr_offset.a_offset)>>5)-1);
    PISA_SHOW("bpu_matrix_C.staddr = %d\n", bpu_matrix_C.staddr);
    PISA_SHOW("bpu_matrix_C.length = %d\n", bpu_matrix_C.length);
    return;
}

void GEMPM(int npu_base, uint32_t *matrix_A, int A_col, int A_row, uint32_t *matrix_B, int B_col, int B_row){
    // 矩阵尺寸规划与数据搬运
    if (A_col!=B_col || A_row!=B_row){
        PISA_INFO("Error: A_col!=B_col or A_row!=B_row\n");
        return;
    }
    struct addr_offset addr_offset = data_load(npu_base, 'P', matrix_A, A_col, A_row, matrix_B, B_col, B_row);
    // 矩阵计算
    struct bpu_matrix bpu_matrix_C = GEMPM_run(npu_base, 0, (addr_offset.a_offset>>5)-1, addr_offset.a_offset>>5, ((addr_offset.b_offset-addr_offset.a_offset)>>5)-1);
    PISA_SHOW("bpu_matrix_C.staddr = %d\n", bpu_matrix_C.staddr);
    PISA_SHOW("bpu_matrix_C.length = %d\n", bpu_matrix_C.length);
    return;
}

void GEMM(int npu_base, uint32_t *matrix_A, int A_col, int A_row, uint32_t *matrix_B, int B_col, int B_row){
    // 矩阵尺寸规划与数据搬运
    if (A_row!=B_col){
        PISA_INFO("Error: A_row!=B_col\n");
        return;
    }
    struct addr_offset addr_offset = data_load(npu_base, 'M', matrix_A, A_col, A_row, matrix_B, B_col, B_row);
    // TODO: 矩阵计算
    uint32_t iteration = A_col>>3;
    uint32_t step_num = B_row>>3;
    struct bpu_matrix bpu_matrix_C = GEMM_run(npu_base, 0, addr_offset.a_offset>>5, addr_offset.b_offset>>5, A_row-1, iteration, step_num);
    return;
}


#endif



