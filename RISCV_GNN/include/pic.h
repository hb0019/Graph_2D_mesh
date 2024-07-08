#ifndef __PIC_H__
#define __PIC_H__

#include <stdint.h>

#define RV_PIC_BASE_ADDR       0xe0000000

#define RV_PIC_MEIPL(n)        (RV_PIC_BASE_ADDR | 0x0004 + (0x04*(n)))
#define RV_PIC_MEIE(n)         (RV_PIC_BASE_ADDR | 0x2004 + (0x04*(n)))
#define RV_PIC_MEIPT           (RV_PIC_BASE_ADDR | 0x3004)

//#define INT_C2H_RX_CONN_OPEN    47
//#define INT_C2H_UDP_WTM_OVF     46
//#define INT_C2H_WTM_OVF         45
//#define INT_C2H_RX_CONN_CLOSED  44
//#define INT_C2H_RT_TIMEOUT      43
//#define INT_C2H_PORT_LISTEN_RSP 42
//#define INT_C2H_OPEN_CONN_RSP   41
//#define INT_C2H_CLOSE_CONN_RSP  40
//#define INT_C2H_UDP_OPEN_CONN   39
//#define INT_C2H_UDP_CLOSE_CONN  38
//#define INT_GMAC1_MCI           37
//#define INT_GMAC1_LPI           36
//#define INT_GMAC1_PMT           35
//#define INT_BOOT_SSI            28
//#define INT_ACCNTLM_IDLE        27
//#define INT_ACCNTLM_PAUSE       26
//#define INT_SD                  25
//#define INT_SD_WAKEUP           24
//#define INT_APB_SMBUS           23
//#define INT_GMAC0_LPI           22
//#define INT_GMAC0_PMT           21
//#define INT_GMAC0_SBD           20
//#define INT_GMAC0_SBD_RX        19
//#define INT_GMAC0_SBD_TX_1      18
//#define INT_GMAC0_SBD_TX_0      17
//#define INT_SEC_SPACC           16
//#define INT_SEC_PKA             15
//#define INT_SEC_TRNG            14
//#define INT_SEC_TRNG_ALARM      13
//#define INT_OTG                 12
//#define INT_APB_UART1           11
//#define INT_EXT_SSI             10
//#define INT_APB_I2S             9
//#define INT_APB_GPIO0           8
//#define INT_APB_I2C             7
//#define INT_APB_SPI_MST         6
//#define INT_APB_TIMER_3         5
//#define INT_APB_TIMER_2         4
//#define INT_APB_TIMER_1         3
//#define INT_APB_TIMER_0         2
//#define INT_APB_UART            1
//#define INT_AXI_DMAC            0

/*****New Define******/
// #define INT_APB_TIMER_0         2
// #define INT_APB_UART            1
#define INT_ROUTER				   32


#define MCAUSE_INT         0x80000000
#define MCAUSE_CAUSE       0x7FFFFFFF
#define IRQ_S_SOFT   1
#define IRQ_H_SOFT   2
#define IRQ_M_SOFT   3
#define IRQ_S_TIMER  5
#define IRQ_H_TIMER  6
#define IRQ_M_TIMER  7
#define IRQ_S_EXT    9
#define IRQ_H_EXT    10
#define IRQ_M_EXT    11
#define IRQ_COP      12
#define IRQ_HOST     13


/* ================================================================================ */
/* ================       Device Specific Peripheral Section       ================ */
/* ================================================================================ */

#define CONFIG_TIMER_NUM    1  //ORI 4
#define CONFIG_USART_NUM    2  //ori 1
#define CONFIG_GPIO_NUM     8
#define CONFIG_GPIO_PIN_NUM 18


#define read_csr(reg) ({ unsigned long __tmp; asm volatile ("csrr %0, " #reg : "=r"(__tmp)); __tmp; })

#define write_csr(reg, val) ({ asm volatile ("csrw " #reg ", %0" :: "i"(val));})

#define write_csr_ns(reg, val) ({ asm volatile ("csrw " #reg ", %0" :: "r"(val));})

void enable_int(uint32_t int_num);
void disable_int(uint32_t int_num);
void handle_m_ext_interrupt();

#endif
