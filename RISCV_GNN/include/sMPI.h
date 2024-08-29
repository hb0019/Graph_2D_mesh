/*
 ============================================================================
 Name        : sMPI.h
 Author      : Randolph
 Version     :
 Copyright   : Your copyright notice
 Description : sMPI Headfile
 ============================================================================
 */
#ifndef __SMPI_H__
#define __SMPI_H__

#include "GCN_tool.h"

// //调用地址
// #define LBR_REG_ADDR(n)                       (LBR_BASE_REG_ADDR | n)
// #define LBR_SRAM_ADDR(n)                      (LBR_BASE | n)
// //寄存器基础地址定义
// #define LBR_BASE                              0x40000000
// #define LBR_BASE_REG_ADDR                     0x4000C000

// //CPU_TRANS和CPU_RCV寄存器地址定义
// #define TRANS_BASE_ADDR                       0x30
// #define RECV_BASE_ADDR                        0x34 

// //Others
// #define LBR_DST                               0x00
// #define LBR_SRC                               0x04
// #define LBR_LOCAL                             0x08
// #define LB_XY_SRC_CPU                         0x0C
// #define SP_DATA_LOW                           0x10
// #define SP_DATA_HIGH                          0x14
// #define LBR_BASE_ADDR                         0x18
// #define LBR_XY_SRC                            0x20
// #define CPU_CODE                              0x24
// #define CPU_STATUS                            0x28
// #define TEST_REG                              0x2C  

// //TRANS硬件SRAM地址
// #define LBR_TRANS_SRAM_0                      0xA000
// #define LBR_TRANS_SRAM_1                      0xffff

// //RECV硬件SRAM地址
// #define LBR_RECV_SRAM_0                 	  0x8000
// #define LBR_RECV_SRAM_1                       0x9000

#define sMPI_East							  0
#define sMPI_South                            1
#define sMPI_West                             2
#define sMPI_North							  3
#define sMPI_Local                            4

#define spike                                 0
#define nonspike                              1
#define RAW_TAG                               1

#define SPIKE_PACKAGE                         0
#define RAW_PACKAGE                           1
#define READ_PACKAGE                          2
#define WRITE_PACKAGE                         3
#define SIM_END_PACKAGE                       4

#define PKG_ADDR(addr, offset)                ((addr&0x3FFF)<<offset)
#define RAW_PACKAGE_HEAD(x_src, y_src, x_dst, y_dst) ((RAW_TAG<<61)|PKG_ADDR(y_dst, 42)|PKG_ADDR(x_dst, 28)|PKG_ADDR(y_src, 14)|PKG_ADDR(x_src, 0))

#define WRITE_CPU_TRANS_DATA_INFO(sram, depth) ((sram<<14)|(depth<<5)|0x10)

//基本测试收发
int sMPI_router_trans_test(
//  const void *buf,         //要发送的数据
    int type,                //包的类型
    int dest,                //目的地
	int depth				 //深度
    );

void send_raw_pkt(
    struct message_package *MessagePackage,
    unsigned int x_src,
    unsigned int y_src,
    unsigned int dest_x,
    unsigned int dest_y
);

void rec_raw_pkt(
    struct message_package *MessagePackage,
    uint32_t sram_addr,
    uint32_t rdata_rcv_len
);

//sMPI基本APIs
// int sMPI_Init(
//     int *argc, 
//     char ***argv);

// int sMPI_Comm_size(
//     sMPI_Comm comm, 
//     int *size);

// int sMPI_Comm_rank(
//     sMPI_Comm comm, 
//     int *rank);

// int sMPI_Send(
//     const void *buf, 
//     int count, 
//     sMPI_Datatype datatype, 
//     int dest, 
//     int tag, 
//     sMPI_Comm comm);

// int sMPI_Recv(
//     void *buf, 
//     int count, 
//     sMPI_Datatype datatype, 
//     int source, 
//     int tag, 
//     sMPI_Comm comm, 
//     sMPI_Status *status);

// int sMPI_Finalize(
//     void);

#endif
