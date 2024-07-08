#ifndef __IOCFG_H__
#define __IOCFG_H__
#define IOCFG_BASE 0x20020000

//REGS
#define IOCFG(n)    (IOCFG_BASE | (0x4*(n)))

#endif
