#ifndef __WDT_H__
#define __WDT_H__
#define WDT_BASE 0x20150000

//REGS
#define WDT_CR   (WDT_BASE | 0x00 )
#define WDT_TORR (WDT_BASE | 0x04 )
#define WDT_CCVR (WDT_BASE | 0x08 )
#define WDT_CRR  (WDT_BASE | 0x0C )
#define WDT_STAT (WDT_BASE | 0x10 )
#define WDT_EOI  (WDT_BASE | 0x14 )

#endif
