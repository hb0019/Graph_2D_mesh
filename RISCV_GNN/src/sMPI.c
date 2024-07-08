/*
 ============================================================================
 Name        : sMPI.c
 Author      : huangbin
 Version     :
 Copyright   : Your copyright notice
 Description : sMPI Functions
 ============================================================================
 */
#include <sMPI.h>
#include <stdarg.h>
#include <stdio.h>
#include "common.h"
#include "GCN_tool.h"

int print_str( char *s)
{
	while(*s)
	{
		putchar(*s++);
	}
	return 0;
}

// 目的路由
void send_raw_pkt(
    struct message_package *MessagePackage,
    unsigned int x_src,
    unsigned int y_src,
    unsigned int dest_x,
    unsigned int dest_y
){
    uint32_t rdata;
    uint32_t wdata;
    long long buff; // 64位数y

    // 读取操作
    rdata = read_reg32((reg32_t)LBR_BASE);//4000 0000
    // 触发CPU传输
    write_reg32((reg32_t)LBR_REG_ADDR(TRANS_BASE_ADDR), 0x08);//4000 C030

    /*******************第一拍*******************/
    // 包头
    buff = RAW_PACKAGE_HEAD(x_src, y_src, x_dst, y_dst);
    // 包头写入SRAM低32位
    // 1100 0000 0000 0000 0000 0000 0000 0000
    write_reg32((reg32_t)LBR_SRAM_ADDR(LBR_TRANS_SRAM_0), (unsigned int)buff);//4000 A000        
    // 包头写入SRAM高32位
    // 1010 0000 0000 0000 1100 0000 0000 0000
    write_reg32((reg32_t)(LBR_SRAM_ADDR(LBR_TRANS_SRAM_0)+4), (buff >> 32));//4000 A004

    /*******************第二拍*******************/
    // 自定义结构体包
    // 写入layer_index
    write_reg32((reg32_t)(LBR_SRAM_ADDR(LBR_TRANS_SRAM_0)+8), (unsigned int)MessagePackage->layer_index);
    // 写入node_index
    write_reg32((reg32_t)(LBR_SRAM_ADDR(LBR_TRANS_SRAM_0)+12), (unsigned int)MessagePackage->node_index);
    
    /*******************第三拍*******************/
    // 写入node_feature
    write_reg32((reg32_t)(LBR_SRAM_ADDR(LBR_TRANS_SRAM_0)+16), (unsigned int)MessagePackage->node->n);
    /*******************第四拍*******************/
    for (unsigned int i=0; i<MessagePackage->node->n; i++){
        write_reg32((reg32_t)(LBR_SRAM_ADDR(LBR_TRANS_SRAM_0)+24+i*4), MessagePackage->node->mat[i]);
    }

    // 特定操作，包深度描述写入Router_Trans
    write_reg32((reg32_t)LBR_REG_ADDR(TRANS_BASE_ADDR), WRITE_CPU_TRANS_DATA_INFO(0, MessagePackage->node->n+3));//4000 C030
    
    return;
}

void rec_raw_pkt(
    struct message_package *MessagePackage,
    uint32_t sram_addr,
    uint32_t rdata_rcv_len
){
    if (rdata_rcv_len < 3){
        printf("error: rdata_rcv_len < 4!\n");
        return;
    }
    MessagePackage->layer_index = readMemory32(sram_addr, 0); 
    MessagePackage->node_index = readMemory32(sram_addr, 4); 
    MessagePackage->node->n = readMemory32(sram_addr, 8);

    for (uint32_t i=3; i<rdata_rcv_len; i++){
        // 读RECV的SRAM
        MessagePackage->node->mat[i] = readMemory32(sram_addr, 16+i*4);
        // rdata_low = readMemory32(sram_addr, i*8 + );        
        // rdata_high = readMemory32(sram_addr, i*8 + 4 + );    
    }
}

int sMPI_router_trans_test(
//  const void *buf,         //要发送的数据
    int type,                //包的类型
    int dest,                //目的地
    int depth                //数据深度
    )
{
	// int a;					//未填充的buff和dest操作
	// if(dest)
	// {
	// 	a = sizeof(buf);
	// }
    uint32_t rdata;
    uint32_t wdata;
    long long y = 0xA000000000000000; // 64位数y
    long long mask;
    long long extended_x;

    // 读取操作
    rdata = read_reg32((reg32_t)LBR_BASE);//4000 0000
    
    // 写入操作
    if(type == spike){
        //
    }
    else if(type == nonspike)
    {
        // 触发CPU传输
        write_reg32((reg32_t)LBR_REG_ADDR(TRANS_BASE_ADDR), 0x08);//4000 C030             

    /*******************第一拍*******************/
        switch(dest)
        {
            case sMPI_East:
                mask = ~(((1ULL << 13) - 1) << 28);
                extended_x = (long long)depth << (64 - 13);
                y = (y & mask) | extended_x;
                break;

            case sMPI_West:
                mask = ~(((1ULL << 13) - 1) << 28);
                extended_x = (long long)(-depth) << (64 - 13);
                y = (y & mask) | extended_x;
                break;

            case sMPI_North:
                mask = ~(((1ULL << 13) - 1) << 42);
                extended_x = (long long)depth << (64 - 13);
                y = (y & mask) | extended_x;
                break;
                
            case sMPI_South:
                mask = ~(((1ULL << 13) - 1) << 42);
                extended_x = (long long)(-depth) << (64 - 13);
                y = (y & mask) | extended_x;
                break;
        }

        // 写入SRAM低32位
        // 1100 0000 0000 0000 0000 0000 0000 0000
        write_reg32((reg32_t)LBR_SRAM_ADDR(LBR_TRANS_SRAM_0), (unsigned int)y);//4000 A000        

        // 写入SRAM高32位
        // 1010 0000 0000 0000 1100 0000 0000 0000
        write_reg32((reg32_t)(LBR_SRAM_ADDR(LBR_TRANS_SRAM_0)+4), (y >> 32));//4000 A004
    
        // 数据处理
        wdata = rdata & 0x00000FFF;                      

    /*******************第二拍*******************/
        // 写入SRAM低32位
        write_reg32((reg32_t)(LBR_SRAM_ADDR(LBR_TRANS_SRAM_0)+8), wdata);//4000 A008

        //读取操作
        rdata = read_reg32((reg32_t)(LBR_BASE+4));//4000 0004

        // 数据处理
        wdata = rdata & 0x00000FFF;    

        //写入SRAM高32位
        write_reg32((reg32_t)(LBR_SRAM_ADDR(LBR_TRANS_SRAM_0)+12), wdata);//4000 A00C

        // 另一个特定操作
        write_reg32((reg32_t)LBR_REG_ADDR(TRANS_BASE_ADDR), 0x00004050);//4000 C030

    }
    return 0;
}
