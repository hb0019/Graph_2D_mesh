#ifndef __DMA_H__
#define __DMA_H__
#define DMAC_BASE 0x20200000
#define DMAC_CH_ADDR_DELTA 0x100

//REGS
#define DMAC_CFGREG                  (DMAC_BASE+0x10 )
#define DMAC_CHENREG                 (DMAC_BASE+0x18 )
#define DMAC_INTSTATUSREG            (DMAC_BASE+0x30 )

#define DMAC_CH1_CFG                 (DMAC_BASE+0x120)
#define DMAC_CH1_SAR                 (DMAC_BASE+0x100)
#define DMAC_CH1_DAR                 (DMAC_BASE+0x108)
#define DMAC_CH1_BLOCK_TS            (DMAC_BASE+0x110)
#define DMAC_CH1_CTL                 (DMAC_BASE+0x118)
#define DMAC_CH1_INTSTATUS_ENABLEREG (DMAC_BASE+0x180)
#define DMAC_CH1_INTSIGNAL_ENABLEREG (DMAC_BASE+0x190)
#define DMAC_CH1_INTSTATUS           (DMAC_BASE+0x188)
#define DMAC_CH1_INTCLEARREG         (DMAC_BASE+0x198)

#define DMAC_CH2_CFG                 (DMAC_BASE+0x220)
#define DMAC_CH2_SAR                 (DMAC_BASE+0x200)
#define DMAC_CH2_DAR                 (DMAC_BASE+0x208)
#define DMAC_CH2_BLOCK_TS            (DMAC_BASE+0x210)
#define DMAC_CH2_CTL                 (DMAC_BASE+0x218)
#define DMAC_CH2_INTSTATUS_ENABLEREG (DMAC_BASE+0x280)
#define DMAC_CH2_INTSIGNAL_ENABLEREG (DMAC_BASE+0x290)
#define DMAC_CH2_INTSTATUS           (DMAC_BASE+0x288)
#define DMAC_CH2_INTCLEARREG         (DMAC_BASE+0x298)

#define DMAC_CH3_CFG                 (DMAC_BASE+0x320)
#define DMAC_CH3_SAR                 (DMAC_BASE+0x300)
#define DMAC_CH3_DAR                 (DMAC_BASE+0x308)
#define DMAC_CH3_BLOCK_TS            (DMAC_BASE+0x310)
#define DMAC_CH3_CTL                 (DMAC_BASE+0x318)
#define DMAC_CH3_INTSTATUS_ENABLEREG (DMAC_BASE+0x380)
#define DMAC_CH3_INTSIGNAL_ENABLEREG (DMAC_BASE+0x390)
#define DMAC_CH3_INTSTATUS           (DMAC_BASE+0x388)
#define DMAC_CH3_INTCLEARREG         (DMAC_BASE+0x398)

#define DMAC_CH4_CFG                 (DMAC_BASE+0x420)
#define DMAC_CH4_SAR                 (DMAC_BASE+0x400)
#define DMAC_CH4_DAR                 (DMAC_BASE+0x408)
#define DMAC_CH4_BLOCK_TS            (DMAC_BASE+0x410)
#define DMAC_CH4_CTL                 (DMAC_BASE+0x418)
#define DMAC_CH4_INTSTATUS_ENABLEREG (DMAC_BASE+0x480)
#define DMAC_CH4_INTSIGNAL_ENABLEREG (DMAC_BASE+0x490)
#define DMAC_CH4_INTSTATUS           (DMAC_BASE+0x488)
#define DMAC_CH4_INTCLEARREG         (DMAC_BASE+0x498)


//single channel common para init, ch id=1,2,3,4
//test void dmac_ch_init(uint32_t ch_id, uint32_t src_addr, uint32_t dst_addr, uint32_t tran_num, uint32_t src_width, uint32_t dst_width);
//init dmac int en &channel en
//test void dmac_ch_en_init(uint32_t ch1_en, uint32_t ch2_en, uint32_t ch3_en, uint32_t ch4_en);

#endif
