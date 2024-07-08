/*
 ============================================================================
 Name        : sMPI.c
 Author      : Randolph
 Version     :
 Copyright   : Your copyright notice
 Description : Interrupt configuration
 ============================================================================
 */
#include "stdint.h"
#include "pic.h"
#include "common.h"

/*****New******/
void __attribute__((weak))router_intr();
void __attribute__((weak))apb_uart_intr();
void __attribute__((weak))apb_timer_0_intr();

//void __attribute__((weak))axi_dmac_intr();
//void __attribute__((weak))apb_timer_1_intr();
//void __attribute__((weak))apb_timer_2_intr();
//void __attribute__((weak))apb_timer_3_intr();
//void __attribute__((weak))apb_spi_mst_intr();
//void __attribute__((weak))apb_i2c_intr();
//void __attribute__((weak))apb_gpio0_intr();
//void __attribute__((weak))apb_i2s_intr();
//void __attribute__((weak))ext_ssi_intr();
//void __attribute__((weak))apb_uart1_intr();
//void __attribute__((weak))otg_intr();
//void __attribute__((weak))sec_trng_alarm_intr();
//void __attribute__((weak))sec_trng_intr();
//void __attribute__((weak))sec_pka_intr();
//void __attribute__((weak))sec_spacc_intr();
//void __attribute__((weak))gmac0_sbd_tx_0_intr();
//void __attribute__((weak))gmac0_sbd_tx_1_intr();
//void __attribute__((weak))gmac0_sbd_rx_intr();
//void __attribute__((weak))gmac0_sbd_intr();
//void __attribute__((weak))gmac0_pmt_intr();
//void __attribute__((weak))gmac0_lpi_intr();
//void __attribute__((weak))apb_smbus_intr();
//void __attribute__((weak))sd_wakeup_intr();
//void __attribute__((weak))sd_intr();
//void __attribute__((weak))accntlm_idle_intr();
//void __attribute__((weak))accntlm_pause_intr();
//void __attribute__((weak))boot_ssi_intr();
//void __attribute__((weak))gmac1_pmt_intr();
//void __attribute__((weak))gmac1_lpi_intr();
//void __attribute__((weak))gmac1_mci_intr();
//void __attribute__((weak))c2h_close_conn_rsp_intr();
//void __attribute__((weak))c2h_open_conn_rsp_intr();
//void __attribute__((weak))c2h_port_listen_rsp_intr();
//void __attribute__((weak))c2h_rt_timeout_intr();
//void __attribute__((weak))c2h_rx_conn_closed_intr();
//void __attribute__((weak))c2h_wtm_ovf_intr();
//void __attribute__((weak))c2h_udp_wtm_ovf_intr();
//void __attribute__((weak))c2h_rx_conn_open_intr();
//void __attribute__((weak))c2h_udp_conn_open_intr();
//void __attribute__((weak))c2h_udp_conn_close_intr();
 
void pic_enable_interrupt(uint32_t int_num) {
  write_reg32(RV_PIC_MEIE(int_num), 0x1);
}

void pic_disable_interrupt(uint32_t int_num) {
  write_reg32(RV_PIC_MEIE(int_num), 0x0);
}

void pic_set_threshold(uint32_t prior) {
  write_reg32(RV_PIC_MEIPT, prior);
}

void pic_set_priority(uint32_t int_num, uint32_t prior) {
  write_reg32(RV_PIC_MEIPL(int_num), prior);
}

void enable_int(uint32_t int_num) {
  // Disable interrupt globally
  __asm__("csrrci x0, mstatus, 0x8;\n");
  // Disable external interrupt
  __asm__("li a0,(1<<11);\n");
  pic_set_threshold(1);
  pic_set_priority(int_num, 7);
  __asm__("csrrc zero,mie,a0;\n");
  pic_enable_interrupt(int_num);
  // Enable external interrupt
  __asm__(
    "li a0,(1<<11);\n"
    "csrrs zero,mie,a0;\n"
  );
  // Enable interrupt globally
  __asm__("csrrs x0, mstatus, 0x8;\n");
}

void disable_int(uint32_t int_num) {
  // Disable interrupt globally
  __asm__("csrrci x0, mstatus, 0x8;\n");
  // Disable external interrupt
  __asm__("li a0,(1<<11);\n");
  pic_set_threshold(1);
  pic_set_priority(int_num, 7);
  __asm__("csrrc zero,mie,a0;\n");
  pic_disable_interrupt(int_num);
  // Enable external interrupt
  __asm__(
    "li a0,(1<<11);\n"
    "csrrs zero,mie,a0;\n"
  );
  // Enable interrupt globally
  __asm__("csrrs x0, mstatus, 0x8;\n");
}

uint32_t handle_trap(uint32_t mcause, uint32_t epc) {
  if (0){
    // External Machine-Level interrupt from PLIC
  } else if ((mcause & MCAUSE_INT) && ((mcause & MCAUSE_CAUSE) == IRQ_M_EXT)) {
    handle_m_ext_interrupt();
    // External Machine-Level interrupt from PLIC
  } else if ((mcause & MCAUSE_INT) && ((mcause & MCAUSE_CAUSE) == IRQ_M_TIMER)){
    //handle_m_time_interrupt();
  }
  //else {
  //  write(1, "trap\n", 5);
  //  _exit(1 + mcause);
  //}
  return epc;
}

void handle_m_ext_interrupt() {
  write_csr(0xBCA, 1);                           // meicpct
  uint32_t int_num = (read_csr(0xFC8) >> 2) - 1; // meihap
//  printf("\nint_num is %x\n", int_num);          // For debug
  switch (int_num) {
  	/*******New*********/
    case INT_ROUTER			  :
      router_intr();
      break;
    /*******************/
//    case INT_APB_UART         :
//      apb_uart_intr();
//      break;
//    case INT_APB_TIMER_0      :
//      apb_timer_0_intr();
//      break;
//    case INT_AXI_DMAC         :
//      axi_dmac_intr();
//      break;
//    case INT_APB_TIMER_1      :
//      apb_timer_1_intr();
//      break;
//    case INT_APB_TIMER_2      :
//      apb_timer_2_intr();
//      break;
//    case INT_APB_TIMER_3      :
//      apb_timer_3_intr();
//      break;
//    case INT_APB_SPI_MST      :
//      apb_spi_mst_intr();
//      break;
//    case INT_APB_I2C          :
//      apb_i2c_intr();
//      break;
//    case INT_APB_GPIO0        :
//      apb_gpio0_intr();
//      break;
//    case INT_APB_I2S          :
//      apb_i2s_intr();
//      break;
//    case INT_EXT_SSI          :
//      ext_ssi_intr();
//      break;
//    case INT_APB_UART1        :
//      apb_uart1_intr();
//      break;
//    case INT_OTG               :
//      otg_intr();
//      break;
//    case INT_SEC_TRNG_ALARM   :
//      sec_trng_alarm_intr();
//      break;
//    case INT_SEC_TRNG         :
//      sec_trng_intr();
//      break;
//    case INT_SEC_PKA          :
//      sec_pka_intr();
//      break;
//    case INT_SEC_SPACC        :
//      sec_spacc_intr();
//      break;
//    case INT_GMAC0_SBD_TX_0   :
//      gmac0_sbd_tx_0_intr();
//      break;
//    case INT_GMAC0_SBD_TX_1   :
//      gmac0_sbd_tx_1_intr();
//      break;
//    case INT_GMAC0_SBD_RX     :
//      gmac0_sbd_rx_intr();
//      break;
//    case INT_GMAC0_SBD        :
//      gmac0_sbd_intr();
//      break;
//    case INT_GMAC0_PMT        :
//      gmac0_pmt_intr();
//      break;
//    case INT_GMAC0_LPI        :
//      gmac0_lpi_intr();
//      break;
//    case INT_APB_SMBUS        :
//      apb_smbus_intr();
//      break;
//    case INT_SD_WAKEUP        :
//      sd_wakeup_intr();
//      break;
//    case INT_SD               :
//      sd_intr();
//      break;
//    case INT_ACCNTLM_IDLE         :
//      accntlm_idle_intr();
//      break;
//    case INT_ACCNTLM_PAUSE        :
//      accntlm_pause_intr();
//      break;
//    case INT_BOOT_SSI             :
//      boot_ssi_intr();
//      break;
//    case INT_GMAC1_PMT            :
//      gmac1_pmt_intr();
//      break;
//    case INT_GMAC1_LPI            :
//      gmac1_lpi_intr();
//      break;
//    case INT_GMAC1_MCI            :
//      gmac1_mci_intr();
//      break;
//    case INT_C2H_CLOSE_CONN_RSP   :
//      c2h_close_conn_rsp_intr();
//      break;
//    case INT_C2H_OPEN_CONN_RSP    :
//      c2h_open_conn_rsp_intr();
//      break;
//    case INT_C2H_PORT_LISTEN_RSP  :
//      c2h_port_listen_rsp_intr();
//      break;
//    case INT_C2H_RT_TIMEOUT       :
//      c2h_rt_timeout_intr();
//      break;
//    case INT_C2H_RX_CONN_CLOSED   :
//      c2h_rx_conn_closed_intr();
//      break;
//    case INT_C2H_WTM_OVF          :
//      c2h_wtm_ovf_intr();
//      break;
//    case INT_C2H_UDP_WTM_OVF      :
//      c2h_udp_wtm_ovf_intr();
//      break;
//    case INT_C2H_RX_CONN_OPEN     :
//      c2h_rx_conn_open_intr();
//      break;
//    case INT_C2H_UDP_OPEN_CONN    :
//      c2h_udp_conn_open_intr();
//      break;
//    case INT_C2H_UDP_CLOSE_CONN   :
//      c2h_udp_conn_close_intr();
//      break;
    default:
      break;
  }
}


