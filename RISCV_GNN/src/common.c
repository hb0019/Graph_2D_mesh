#include "stdint.h"
#include "common.h"

void write_reg32(uint32_t addr,uint32_t data){
  *((volatile uint32_t*)(addr)) = data;
}


uint32_t read_reg32(uint32_t addr){
  uint32_t rdata;
  rdata = *((volatile uint32_t*)(addr));
  return rdata;
}

uint16_t read_reg16(uint32_t addr){
  uint16_t rdata;
  rdata = *((volatile uint32_t*)(addr));
  return rdata;
}

void sim_exit(){
  //exit command
  write_reg32(TUBE_ADDR,0xFFFF0000);
  while(1);
}

void error(){
 //issue error command
  write_reg32(TUBE_ADDR,0xEEEE0000);
  while(1);
}

void enable_cache(){
//FIXME    asm(
//FIXME    "//enable cache\n"
//FIXME    "//****************************************************\n"
//FIXME    "// Step1:invalid all cache\n"
//FIXME    "//****************************************************\n"
//FIXME    "lrw r1, 0xe000f004   //set CIR register address\n"
//FIXME    "ld.w r2, (r1, 0x0)   \n"
//FIXME    "bseti r2, 0x0        //set the 0 bit :invalid all\n"
//FIXME    "st.w r2, (r1,0)      //begin to invalid all cache\n"
//FIXME    "//****************************************************\n"
//FIXME    "// Step2:set CRCRx to control cache region\n"
//FIXME    "//****************************************************\n"
//FIXME    "lrw  r1, 0x00000021 //set rom 0-128K as cacheable\n"
//FIXME    "lrw  r2, 0xe000f008  //CRCR0 addr,\n"
//FIXME    "st.w r1,(r2,0)       //begin to set cache region\n"
//FIXME    "lrw  r1, 0x0010001F //set sram 0-64K as cacheable\n"
//FIXME    "lrw  r2, 0xe000f00C  //CRCR1 addr,\n"
//FIXME    "st.w r1,(r2,0)       //begin to set cache region\n"
//FIXME    "//****************************************************\n"
//FIXME    "// Step3:enable cache\n"
//FIXME    "//**************************************************** \n"
//FIXME    "lrw   r2, 0xe000f000 \n"
//FIXME    "ld.w  r1, (r2,0)\n"
//FIXME    "bseti r1, 0           //cache enable\n"
//FIXME    "bclri r1, 1           //set INST and DATA all cacheble\n"
//FIXME    "bseti r1, 2           //set write back \n"
//FIXME    "bseti r1, 4           //set cache can be writen\n"
//FIXME    "bclri r1, 5           //set write non-allocate\n"
//FIXME    "st.w  r1, (r2,0)      //store the value to register\n"
//FIXME    );


}
