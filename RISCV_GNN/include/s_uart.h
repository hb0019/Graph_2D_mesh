#ifndef __UART_H__
#define __UART_H__
#include "datatype.h"
#define UART_BASE 0x20120000
#define UART_ADDR_DELTA 0x60000
//#define UART1_BASE 0x20180000
//REGS
#define UART_FCR(n)   (UART_BASE+0x8 + (0x60000*(n)))
#define UART_LCR(n)   (UART_BASE+0xC + (0x60000*(n)))
#define UART_DLL(n)   (UART_BASE+0x0 + (0x60000*(n)))
#define UART_DLH(n)   (UART_BASE+0x4 + (0x60000*(n)))
#define UART_IER(n)   (UART_BASE+0x4 + (0x60000*(n)))
#define UART_MCR(n)   (UART_BASE+0x10+ (0x60000*(n)))
#define UART_IIR(n)   (UART_BASE+0x8 + (0x60000*(n)))
#define UART_THR(n)   (UART_BASE+0x0 + (0x60000*(n)))
/*
#define UART1_FCR(n)   (UART1_BASE+0x8 + (0x100000*(n)))
#define UART1_LCR(n)   (UART1_BASE+0xC + (0x100000*(n)))
#define UART1_DLL(n)   (UART1_BASE+0x0 + (0x100000*(n)))
#define UART1_DLH(n)   (UART1_BASE+0x4 + (0x100000*(n)))
#define UART1_IER(n)   (UART1_BASE+0x4 + (0x100000*(n)))
#define UART1_MCR(n)   (UART1_BASE+0x10+ (0x100000*(n)))
#define UART1_IIR(n)   (UART1_BASE+0x8 + (0x100000*(n)))
#define UART1_THR(n)   (UART1_BASE+0x0 + (0x100000*(n)))
*/
void uart_init(uint32_t uart_id , uint32_t rx_en, uint32_t tx_en);
#endif

