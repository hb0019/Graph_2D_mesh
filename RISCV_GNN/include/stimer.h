#ifndef __STIMER_H__
#define __STIMER_H__
#define TIMER_BASE 0x20130000
#define TIMER_DELTA 0x14

//REGS
#define TIMER_LoadCount(n)    (TIMER_BASE | 0x00 + (0x14*(n)))
#define TIMER_LoadCount2(n)   (TIMER_BASE | 0xb0 + (0x04*(n)))
#define TIMER_CurrentValue(n) (TIMER_BASE | 0x04 + (0x14*(n)))
#define TIMER_ControlReg(n)   (TIMER_BASE | 0x08 + (0x14*(n)))
#define TIMER_EOI(n)          (TIMER_BASE | 0x0C + (0x14*(n)))
#define TIMER_IntStatus(n)    (TIMER_BASE | 0x10 + (0x14*(n)))

#endif
