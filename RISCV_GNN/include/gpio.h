#ifndef __GPIO_H__
#define __GPIO_H__
#define GPIO_BASE 0x20100000

//regs
#define GPIO_SWPORTA_DR      (GPIO_BASE | 0x00 )
#define GPIO_SWPORTA_DDR     (GPIO_BASE | 0x04 )
#define GPIO_INTEN           (GPIO_BASE | 0x30 )
#define GPIO_INTMASK         (GPIO_BASE | 0x34 )
#define GPIO_INTTYPE_LEVEL   (GPIO_BASE | 0x38 )
#define GPIO_INT_POLARITY    (GPIO_BASE | 0x3C )
#define GPIO_INTSTATUS       (GPIO_BASE | 0x40 )
#define GPIO_RAW_INTSTATUS   (GPIO_BASE | 0x44 )
#define GPIO_DEBOUNCE        (GPIO_BASE | 0x48 )
#define GPIO_PORTA_EOI       (GPIO_BASE | 0x4C )
#define GPIO_EXT_PORTA       (GPIO_BASE | 0x50 )
#define GPIO_LS_SYNC         (GPIO_BASE | 0x60 )
#define GPIO_ID_CODE         (GPIO_BASE | 0x64 )
#define GPIO_INT_BOTHEDGE    (GPIO_BASE | 0x68 )
#define GPIO_VER_ID_CODE     (GPIO_BASE | 0x6C )
#define GPIO_CONFIG_REG2     (GPIO_BASE | 0x70 )
#define GPIO_CONFIG_REG1     (GPIO_BASE | 0x74 )

#endif
