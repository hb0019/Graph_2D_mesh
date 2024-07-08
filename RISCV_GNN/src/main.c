/*
 ============================================================================
 Name        : main.c
 Author      : huangbin
 Version     :
 Copyright   : Your copyright notice
 Description : struct type for router test
 ============================================================================
 */
#include <sMPI.h>
#include <stdlib.h>
#include <stdint.h>
#include "GCN_tool.h"
#include "pic.h"
#include "lbii.h"
#include "commmon.h"

//中断的flag
uint8_t router_rcv_int_flag = 0;

// 片上特殊存储结构体
struct message_package rec_MessagePackage;
struct message_package send_MessagePackage;
struct node_feature node_feature_data; // 节点特征结构体


//定义中断函数
void router_intr(){	
	writeReg32_cpu_rcv_mask_down();				// 使能router阻塞接收数据
	// 读取数据
	load_data();
	aggregator(&rec_MessagePackage, &node_feature_data);
	// 触发中断后续处理程序
	router_rcv_int_flag = 1;
	writeReg32_cpu_rcv_mask_high();				// 使能router开放接收数据
}

void load_data(){
	uint32_t rdata_rcv_len;
	uint32_t rdata_rcv_avail;
	uint32_t sram_addr;
	uint32_t rdata;

	// 读取raw包描述
	rdata = readReg32_cpu_rcv();	//4000 C034
	rdata_rcv_avail = (rdata>>9) & 0x00000003;  // 判断生效sram的id
    rdata_rcv_len = (rdata) & 0x000001ff;       // raw_data有效长度

	if (rdata_rcv_avail == 1){
		sram_addr = LBR_SRAM_ADDR(LBR_RECV_SRAM_0);
	} else if (rdata_rcv_avail == 2 || rdata_rcv_avail == 3){
		sram_addr = LBR_SRAM_ADDR(LBR_RECV_SRAM_1);
	} else {
		sram_addr = 0;
		print('error\n');
		continue;
	}

	rec_raw_pkt(&rec_MessagePackage, sram_addr, rdata_rcv_len);

}


int main(void){
	// 清除main的中断
	write_reg32(0x1040000c, 0);
	// init the router interrupt
	enable_int(INT_ROUTER);

	// 测试数据
	uint16_t layer_dimension[2] = {8, 8};  
	int node_num = 10;
	node_data_create(layer_dimension, &node_feature_data, node_num);

	
	int node_list[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
	message(&send_MessagePackage, &node_feature_data, node_list, 10, 0);

	while (1){
		// 等待中断
		if (router_rcv_int_flag){
			router_rcv_int_flag = 0;
			uint8_t layer_index = rec_MessagePackage->layer_index;
    		uint16_t node_index = rec_MessagePackage->node_index;
			if (node_feature_data->degree[layer_index][node_index]!=0){
				int node_list[1] = {node_index};
				message(&send_MessagePackage, &node_feature_data, node_list, 1, 0);
			}
		} 
	}

	return 0;
}

