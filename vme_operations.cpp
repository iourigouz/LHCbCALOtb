#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <inttypes.h>
#include <stdint.h>
#include <unistd.h>

#include <time.h>
#include <sys/time.h>

#include "CAENVMElib.h"
#include "V1290N.h"

#include <math.h>
#include <string.h>
#include <ctype.h>

#include <vector>
#include <algorithm>
#include <utility>

#include "runparam.h"
#include "ntp.h"
#include "vme_operations.h"

bool g_ADC1_installed=true;
bool g_ADC2_installed=true;
bool g_ADC3_installed=true;
bool g_TDC_installed=true;

bool g_vme_initialized=false;

int32_t       BHandle;

//----------------------V260 functions ---------------------
int V260_init(const uint32_t base){
  int res=cvSuccess;

  res=V260_clear_scalers(base);
  if(cvSuccess!=res) return res;
	
  res=V260_disable_interrupt(base);
  if(cvSuccess!=res) return res;
	
  res=V260_clear_inhibit(base);
  if(cvSuccess!=res) return res;
	
  return cvSuccess;
}

int V260_clear_scalers(const uint32_t base){
  uint32_t addr=0x50;
  //uint32_t v=0xFFFF;
  //return write_reg2(base, addr, v);
  uint16_t v=(uint16_t)0xFFFF;
  return CAENVME_WriteCycle(BHandle, base+addr, &v, cvA24_S_DATA, cvD16);
}

int V260_set_inhibit(const uint32_t base){
  uint32_t addr=0x52;
  //uint32_t v=0xFFFF;
  //return write_reg2(base, addr, v);
  uint16_t v=(uint16_t)0xFFFF;
  return CAENVME_WriteCycle(BHandle, base+addr, &v, cvA24_S_DATA, cvD16);
}

int V260_clear_inhibit(const uint32_t base){
  uint32_t addr=0x54;
  //uint32_t v=0xFFFF;
  //return write_reg2(base, addr, v);
  uint16_t v=(uint16_t)0xFFFF;
  return CAENVME_WriteCycle(BHandle, base+addr, &v, cvA24_S_DATA, cvD16);
}

int V260_increment(const uint32_t base){
  uint32_t addr=0x56;
  //uint32_t v=0xFFFF;
  //return write_reg2(base, addr, v);
  uint16_t v=(uint16_t)0xFFFF;
  return CAENVME_WriteCycle(BHandle, base+addr, &v, cvA24_S_DATA, cvD16);
}

int V260_disable_interrupt(const uint32_t base){
  uint32_t addr=0xA;
  //uint32_t v=0xFFFF;
  //return write_reg2(base, addr, v);
  uint16_t v=(uint16_t)0xFFFF;
  return CAENVME_WriteCycle(BHandle, base+addr, &v, cvA24_S_DATA, cvD16);
}

int V260_enable_interrupt(const uint32_t base){
  uint32_t addr=0x8;
  //uint32_t v=0xFFFF;
  //return write_reg2(base, addr, v);
  uint16_t v=(uint16_t)0xFFFF;
  return CAENVME_WriteCycle(BHandle, base+addr, &v, cvA24_S_DATA, cvD16);
}

int V260_set_interrupt(const uint32_t base, uint32_t vector){
  uint32_t addr=0x4;
  //uint32_t v=vector&0xFF;
  //return write_reg2(base, addr, v);
  uint16_t v=(uint16_t)(vector&0xFF);
  return CAENVME_WriteCycle(BHandle, base+addr, &v, cvA24_S_DATA, cvD16);
}

int V260_read(const uint32_t base, uint32_t chan, uint32_t &count){// read one counter, 24 bit
  int res=cvSuccess;
	
  uint32_t addr_MSB=0x10+chan*4;
  uint32_t addr_LSB=addr_MSB+2;
  uint32_t count_LSB=0, count_MSB=0;
	
  res=read_reg2(base, addr_LSB, count_LSB);
  if(cvSuccess!=res)return res;	
  res=read_reg2(base, addr_MSB, count_MSB);
  if(cvSuccess!=res)return res;
	
  count = (count_LSB & 0xFFFF) | (count_MSB<<16);
  return cvSuccess;
}

//-------------------end V260 functions -----------------------
//--------------------- V812 functions --------------------

int V812_init(const uint32_t base, int* CFDthr=0){
  int res=cvSuccess;

  unsigned int v;

  res=V812_write_inh_patt(base, 0xFFFF);
  if(cvSuccess!=res) return res;

  for(int ich=0; ich<16; ++ich){
    v=30;
    if(CFDthr)v=CFDthr[ich]&0xFF;
    res=V812_write_threshold(base, ich, v);
    if(cvSuccess!=res) return res;
  }

  v=40;
  if(CFDthr)v=CFDthr[16]&0xFF;
  res=V812_write_width1(base, v);
  if(cvSuccess!=res) return res;

  v=40;
  if(CFDthr)v=CFDthr[17]&0xFF;
  res=V812_write_width2(base, v);
  if(cvSuccess!=res) return res;

  v=10;
  if(CFDthr)v=CFDthr[18]&0xFF;
  res=V812_write_deadtime1(base, v);
  if(cvSuccess!=res) return res;

  v=10;
  if(CFDthr)v=CFDthr[19]&0xFF;
  res=V812_write_deadtime2(base, v);
  if(cvSuccess!=res) return res;

  res=V812_write_maj_thr(base, 19); // corresponds to MAJ threshold=2, see user manual
  if(cvSuccess!=res) return res;

  return cvSuccess;
}

int V812_write_threshold(const uint32_t base, uint32_t chan, uint32_t val){
  uint32_t addr=(chan&0x0F)<<1;
  //uint32_t val=(thr&0xFF);
  //return write_reg2(base, addr, val);
  uint16_t v=(uint16_t)(val&0xFF);
  return CAENVME_WriteCycle(BHandle, base+addr, &v, cvA24_S_DATA, cvD16);
}

int V812_write_width1(const uint32_t base, uint32_t val){ // ch 0-7
  uint32_t addr=0x40;
  //uint32_t v=(val&0xFF);
  //return write_reg2(base, addr, v);
  uint16_t v=(uint16_t)(val&0xFF);
  return CAENVME_WriteCycle(BHandle, base+addr, &v, cvA24_S_DATA, cvD16);
}

int V812_write_width2(const uint32_t base, uint32_t val){ // ch 8-15
  uint32_t addr=0x42;
  //uint32_t v=(val&0xFF);
  //return write_reg2(base, addr, v);
  uint16_t v=(uint16_t)(val&0xFF);
  return CAENVME_WriteCycle(BHandle, base+addr, &v, cvA24_S_DATA, cvD16);
}

int V812_write_deadtime1(const uint32_t base, uint32_t val){ // ch 0-7
  uint32_t addr=0x44;
  //uint32_t v=(val&0xFF);
  // return write_reg2(base, addr, v);
  uint16_t v=(uint16_t)(val&0xFF);
  return CAENVME_WriteCycle(BHandle, base+addr, &v, cvA24_S_DATA, cvD16);
}

int V812_write_deadtime2(const uint32_t base, uint32_t val){ // ch 8-15
  uint32_t addr=0x46;
  //uint32_t v=(val&0xFF);
  //return write_reg2(base, addr, v);
  uint16_t v=(uint16_t)(val&0xFF);
  return CAENVME_WriteCycle(BHandle, base+addr, &v, cvA24_S_DATA, cvD16);
}

int V812_write_maj_thr(const uint32_t base, uint32_t val){
  uint32_t addr=0x48;
  //uint32_t v=(val&0xFF);
  //return write_reg2(base, addr, v);
  uint16_t v=(uint16_t)(val&0xFF);
  return CAENVME_WriteCycle(BHandle, base+addr, &v, cvA24_S_DATA, cvD16);
}

int V812_write_inh_patt(const uint32_t base, uint32_t val){
  uint32_t addr=0x4A;
  //uint32_t v=(val&0xFFFF);
  //return write_reg2(base, addr, v);
  uint16_t v=(uint16_t)(val&0xFFFF);
  return CAENVME_WriteCycle(BHandle, base+addr, &v, cvA24_S_DATA, cvD16);
}

int V812_send_test_pulse(const uint32_t base){
  uint32_t addr=0x4A;
  //uint32_t v=0xFFFF;
  //return write_reg2(base, addr, v);
  uint16_t v=0xFFFF;
  return CAENVME_WriteCycle(BHandle, base+addr, &v, cvA24_S_DATA, cvD16);
}

//-------------------end V812 functions -----------------------
//--------------------- V1290 functions --------------------

int write_reg2(const uint32_t base, const uint32_t addr, uint32_t& data){
  uint16_t d=data&0xFFFF;
  return CAENVME_WriteCycle(BHandle, base+addr, &d, cvA24_S_DATA, cvD16);
}
int  read_reg2(const uint32_t base, const uint32_t addr, uint32_t& data){
  uint16_t d;
  int ret = CAENVME_ReadCycle(BHandle, base+addr, &d, cvA24_S_DATA, cvD16);
  data=d;
  return ret;
}
int write_reg4(const uint32_t base, const uint32_t addr, uint32_t& data){
  return CAENVME_WriteCycle(BHandle, base+addr, &data, cvA24_S_DATA, cvD32);
}
int  read_reg4(const uint32_t base, const uint32_t addr, uint32_t& data){
  return CAENVME_ReadCycle(BHandle, base+addr, &data, cvA24_S_DATA, cvD32);
}

int V1290_read_statreg(const uint32_t base, uint16_t& statreg){
  return CAENVME_ReadCycle(BHandle, V1290_STATUS_ADD, &statreg, cvA24_S_DATA, cvD16);
}

int V1290_read_ctrlreg(const uint32_t base, uint16_t& ctrlreg){
  return CAENVME_ReadCycle(BHandle, V1290_CONTROL_ADD, &ctrlreg, cvA24_S_DATA, cvD16);
}

int V1290_write_ctrlreg(const uint32_t base, uint16_t& ctrlreg){
  return CAENVME_WriteCycle(BHandle, V1290_CONTROL_ADD, &ctrlreg, cvA24_S_DATA, cvD16);
}

int V1290_read_dummy32reg(const uint32_t base, uint32_t& ctrlreg){
  return CAENVME_ReadCycle(BHandle, V1290_CONTROL_ADD, &ctrlreg, cvA24_S_DATA, cvD32);
}

int V1290_write_dummy32reg(const uint32_t base, uint32_t& ctrlreg){
  return CAENVME_WriteCycle(BHandle, V1290_CONTROL_ADD, &ctrlreg, cvA24_S_DATA, cvD32);
}

int V1290_read_micro(const uint32_t base, uint16_t &datum){
  int ret=0;
  uint16_t micro_hnd=0;
  
  uint32_t addr_hnd=base+V1290_MICRO_HND_ADD;
  while(0==(micro_hnd & V1290_MICRO_HND_READOK_MSK)){// check READ_OK
    if(cvSuccess!=(ret=CAENVME_ReadCycle(BHandle, addr_hnd, &micro_hnd, cvA24_S_DATA, cvD16)))
      return ret;
  }
  
  uint32_t addr=base+V1290_MICRO_ADD;
  return CAENVME_ReadCycle(BHandle, addr, &datum, cvA24_S_DATA, cvD16);
}

int V1290_write_micro(const uint32_t base, uint16_t &datum){
  int ret=0;
  uint16_t micro_hnd=0;
  
  uint32_t addr_hnd=base+V1290_MICRO_HND_ADD;
  while(0==(micro_hnd & V1290_MICRO_HND_WRITEOK_MSK)){// check WRITE_OK
    if(cvSuccess!=(ret=CAENVME_ReadCycle(BHandle, addr_hnd, &micro_hnd, cvA24_S_DATA, cvD16)))
      return ret;
  }
  
  uint32_t addr=base+V1290_MICRO_ADD;
  return CAENVME_WriteCycle(BHandle, addr, &datum, cvA24_S_DATA, cvD16);
}

int V1290_opcode(const uint32_t base, int opcode, int nR, uint16_t* R, int nW, uint16_t* W){
  int ret=0;
  
  uint16_t cod=(uint16_t) opcode & 0xFFFF;
  if(cvSuccess!=(ret=V1290_write_micro(base, cod)) ){ // write opcode
    printf("%s: error in V1290_write_micro, opcode %d\n",__func__,cod);
    return ret;
  }

  if( (0!=nR) && R){
    for(int i=0; i<nR; ++i){
      if(cvSuccess != (ret=V1290_read_micro(base, R[i])) ) return ret; // write datum
    }
  }
  else if( (0!=nW) && W){
    for(int i=0; i<nW; ++i){
      if(cvSuccess != (ret=V1290_write_micro(base, W[i])) ) return ret; // write datum
    }
  }
}

int V1290_clear(const uint32_t base){
  uint32_t addr=base+V1290_SW_CLEAR_ADD;
  uint16_t data=1;
  return CAENVME_WriteCycle(BHandle, addr, &data, cvA24_S_DATA, cvD16);
}

int V1290_count_reset(const uint32_t base){
  uint32_t addr=base+V1290_SW_EVENT_RESET_ADD;
  uint16_t data=1;
  return CAENVME_WriteCycle(BHandle, addr, &data, cvA24_S_DATA, cvD16);
}  

int V1290_read_buffer(const uint32_t base, int maxdata, int &ndata, uint32_t *data){
  uint32_t addr=base + V1290_OUT_BUFFER_ADD;
  uint32_t datum=0;
  bool done=false;
  int ret=cvSuccess;
  
  ndata=0;
  while(!done){
    if(cvSuccess!=(ret=CAENVME_ReadCycle(BHandle, addr, &datum, cvA24_S_DATA, cvD32))){
      printf("%s: error reading data\n",__func__);
      return ret;
    }
    if(ndata>=maxdata)ndata=maxdata-1;
    data[ndata++]=datum;
    addr+=sizeof(uint32_t);
    if(V1290_IS_GLOBAL_TRAILER(datum) || V1290_IS_FILLER(datum))done=true;
  }
  return cvSuccess;
}

void V1290_print_buffer(int ndata, uint32_t *data){
  printf("\n Word count = %d\n",ndata);
  for(int i=0; i<ndata; ++i){
    if(V1290_IS_GLOBAL_HEADER(data[i])){
      printf("Global Header: Event count=0x%X, GEO=0x%X\n", (data[i]&0x07FFFFE0)>>5, data[i]&0x1F);
    }
    else if(V1290_IS_TDC_HEADER(data[i])){
      printf("TDC Header: TDC# = 0x%X, Event ID = 0x%X, Bunch ID = 0x%X\n", 
             (data[i]&0x03000000)>>24, (data[i]&0x00FFF000)>>12, data[i]&0x00000FFF);
    }
    else if(V1290_IS_TDC_MEASURE(data[i])){
      printf("TDC Measure: edge = 0x%X, channel = 0x%X, meas = 0x%X\n", 
             (data[i]&0x0C000000)>>26, (data[i]&0x03E00000)>>21, data[i]&0x001FFFFF);
    }
    else if(V1290_IS_TDC_TRAILER(data[i])){
      printf("TDC Trailer: TDC# = 0x%X, Event ID = 0x%X, Bunch ID = 0x%X\n", 
             (data[i]&0x03000000)>>24, (data[i]&0x00FFF000)>>12, data[i]&0x00000FFF);
    }
    else if(V1290_IS_TDC_ERROR(data[i])){
      printf("TDC Error: TDC# = 0x%X, Error Flags = 0x%X\n", (data[i]&0x03000000)>>24, data[i]&0x00007FFF);
    }
    else if(V1290_IS_GLOBAL_TRIGGER_TIME (data[i])){
      printf("Global Trigger Time: Tag = 0x%X\n", data[i]&0x07FFFFFF);
    }
    else if(V1290_IS_GLOBAL_TRAILER(data[i])){
      printf("Global Trailer: Trigger Lost = %d, Ovfl = %d, TDC Err = %d, Word count=0x%X, GEO=0x%X\n", 
             (data[i]&0x04000000)>>26, (data[i]&0x02000000)>>25, (data[i]&0x01000000)>>24, (data[i]&0x001FFFE0)>>24, data[i]&0x1F);
    }
    else{
      printf("Unknown: 0x%X\n",data[i]);
    }
  }
}

void t_sort(int nt, int *t){
  if(nt<=1)return;

  std::vector<int> tims(t, t+nt);
  std::sort(tims.begin(), tims.end());
  for(int j=0; j<nt; ++j)t[j]=tims[j];
}

void V1290_unpack_buffer(int ndata, uint32_t *data){
  memset(&g_nTDC[0], 0, sizeof(g_nTDC));
  memset(&g_tTDC[0][0], 0, sizeof(g_tTDC));
  g_tTDCtrig=0;

  for(int i=0; i<ndata; ++i){
    if(V1290_IS_TDC_MEASURE(data[i])){
      int tim=(int)data[i]&0x001FFFFF;
      int wch=(int)(data[i]&0x03E00000)>>21;
      int edge=(int)(data[i]&0x0C000000)>>26;
      //if(edge)continue;	// take only leading edges
      if(g_nTDC[wch]<NTDCMAXHITS) g_tTDC[wch][g_nTDC[wch]++]=tim;
    }
    else if(V1290_IS_GLOBAL_TRIGGER_TIME(data[i])){
      uint32_t tTDCtrig = data[i] & V1290_GLB_TRG_TIME_TAG_MSK;
      g_tTDCtrig=tTDCtrig;
    }
  }

  for(int ich=0; ich<NTDCCHAN; ++ich){
    t_sort(g_nTDC[ich], &g_tTDC[ich][0]);
  }
}

int V1290_init(const uint32_t base){
  int status;
  
  uint16_t R[256], W[256];
  uint32_t d;
  
  d=0x1UL;
  if(cvSuccess != (status=write_reg2(base, V1290_MOD_RESET_ADD, d) ) ) return status; // reset module
  //printf("TDC module reset sent\n");
  usleep(200000);
  
  uint16_t ctr=0;
  V1290_read_ctrlreg(base, ctr);
  ctr|=V1290_CTRL_TRIGGER_TIME_TAG_ENABLE_MSK;
  V1290_write_ctrlreg(base, ctr);                      // enabling trigger time tag
  
  V1290_opcode(base, V1290_DIS_ALL_CH_OPCODE, 0, &R[0], 0, &W[0]); // disable all channels
  
  V1290_clear(base);                                               // clear module
  
  V1290_opcode(base, V1290_EN_HEAD_TRAILER_OPCODE, 0, &R[0], 0, &W[0]); // enable TDC header and trailer
  
  V1290_opcode(base, V1290_TRG_MATCH_OPCODE, 0, &R[0], 0, &W[0]); // set trigger matching
  
  W[0]=60;
  V1290_opcode(base, V1290_SET_WIN_WIDTH_OPCODE, 0, &R[0], 1, &W[0]); // set matching window width: 250 ns, for the beginning
  
  W[0]=0xFFE4;//0xFFE4=-28; 0xFFEC=-20; 0xFFD8=-40 (default)
  V1290_opcode(base, V1290_SET_WIN_OFFSET_OPCODE, 0, &R[0], 1, &W[0]); // set matching window offset: -500 ns
  
  //W[0]=8;
  //V1290_opcode(base, V1290_SET_SW_MARGIN_OPCODE, 0, &R[0], 1, &W[0]); // search margin: this is default
  
  //W[0]=4;
  //V1290_opcode(base, V1290_SET_REJ_MARGIN_OPCODE, 0, &R[0], 1, &W[0]); // reject margin: this is default
  
  V1290_opcode(base, V1290_EN_SUB_TRG_OPCODE, 0, &R[0], 0, &W[0]); // enable trigger subtraction
  
  W[0]=2;                                                           // 1-trailing, 2-leading, 3-both
  V1290_opcode(base, V1290_SET_DETECTION_OPCODE, 0, &R[0], 1, &W[0]); // edge detection
  
  //V1290_opcode(base, V1290_EN_CHANNEL_OPCODE|0x00, 0, &R[0], 0, &W[0]); // enable channel 0
  V1290_opcode(base, V1290_EN_ALL_CH_OPCODE, 0, &R[0], 0, &W[0]); // enable all channels
  
  V1290_opcode(base, V1290_READ_DEAD_TIME_OPCODE, 1, &R[0], 0, &W[0]); // read dead time
  printf("channel dead time = 0x%X\n",R[0]);
  
  //W[0]=3;
  //V1290_opcode(base, V1290_SET_DEAD_TIME_OPCODE, 0, &R[0], 1, &W[0]); // set dead time
  
  //V1290_opcode(base, V1290_READ_DEAD_TIME_OPCODE, 1, &R[0], 0, &W[0]); // read dead time
  //printf("channel dead time = 0x%X\n",R[0]);
  
  return status;
}

int V1290_test(const uint32_t base){
  int status;

  uint16_t R[256], W[256];
  uint32_t d;

  uint16_t statreg=0;
  if(cvSuccess != (status=V1290_read_statreg(base,statreg) ) ) return status;
  printf("statreg=0x%X\n",statreg);

  uint16_t ctrlreg=0;
  if(cvSuccess != (status=V1290_read_ctrlreg(base,ctrlreg) ) ) return status;
  printf("ctrlreg=0x%X\n",ctrlreg);

  uint32_t dummy32=0xAABBCCDD;
  if(cvSuccess != (status=V1290_write_dummy32reg(base,dummy32) ) ) return status;
  if(cvSuccess != (status=V1290_read_dummy32reg(base,dummy32) ) ) return status;
  printf("dummy32=0x%X\n",dummy32);

  d=0x1UL;
  if(cvSuccess != (status=write_reg2(base, V1290_MOD_RESET_ADD, d) ) ) return status;
  printf("TDC module reset sent\n");
  usleep(200000);

  V1290_opcode(base, V1290_TRG_MATCH_OPCODE, 0, &R[0], 0, &W[0]);

  W[0]=2;
  V1290_opcode(base, V1290_SET_WIN_WIDTH_OPCODE, 0, &R[0], 1, &W[0]);

  W[0]=0xFFFC;
  V1290_opcode(base, V1290_SET_WIN_OFFSET_OPCODE, 0, &R[0], 1, &W[0]);

  W[0]=8;
  V1290_opcode(base, V1290_SET_SW_MARGIN_OPCODE, 0, &R[0], 1, &W[0]);

  W[0]=4;
  V1290_opcode(base, V1290_SET_REJ_MARGIN_OPCODE, 0, &R[0], 1, &W[0]);

  V1290_opcode(base, V1290_EN_SUB_TRG_OPCODE, 0, &R[0], 0, &W[0]);

  V1290_opcode(base, V1290_READ_ACQ_MOD_OPCODE, 1, &R[0], 0, &W[0]);
  printf("acquisition mode = 0x%X\n", R[0]);

  V1290_opcode(base, V1290_READ_TRG_CONF_OPCODE, 5, &R[0], 0, &W[0]);
  printf(" trigger configuration: 0x%X 0x%X 0x%X 0x%X 0x%X\n", R[0],R[1],R[2],R[3],R[4]);

  W[0]=2;
  V1290_opcode(base, V1290_SET_DETECTION_OPCODE, 0, &R[0], 1, &W[0]);

  V1290_opcode(base, V1290_READ_DETECTION_OPCODE, 1, &R[0], 0, &W[0]);
  printf("detection = 0x%X\n",R[0]);

  V1290_opcode(base, V1290_READ_HEAD_TRAILER_OPCODE, 1, &R[0], 0, &W[0]);
  printf("header/trailer = 0x%X\n",R[0]);

  V1290_opcode(base, V1290_READ_FIFO_SIZE_OPCODE, 1, &R[0], 0, &W[0]);
  printf("FIFO = 0x%X\n",R[0]);

  V1290_opcode(base, V1290_DIS_ALL_CH_OPCODE, 0, &R[0], 0, &W[0]);
  V1290_opcode(base, V1290_EN_CHANNEL_OPCODE|0x01, 0, &R[0], 0, &W[0]);
  V1290_opcode(base, V1290_EN_CHANNEL_OPCODE|0x00, 0, &R[0], 0, &W[0]);
  V1290_opcode(base, V1290_READ_EN_PATTERN_OPCODE, 1, &R[0], 0, &W[0]);
  printf("enable pattern = 0x%X\n",R[0]);

  V1290_opcode(base, V1290_READ_FIFO_SIZE_OPCODE, 1, &R[0], 0, &W[0]);
  printf("FIFO = 0x%X\n",R[0]);

  V1290_opcode(base, V1290_READ_TDC_ID_OPCODE|0x00, 2, &R[0], 0, &W[0]);
  printf("TDC #0 ID = 0x%4X%4X; ",R[1],R[0]);

  V1290_opcode(base, V1290_READ_TDC_ID_OPCODE|0x01, 2, &R[0], 0, &W[0]);
  printf("TDC #0 ID = 0x%4X%4X\n",R[1],R[0]);

  V1290_opcode(base, V1290_READ_MICRO_REV_OPCODE, 1, &R[0], 0, &W[0]);
  printf("MICRO firmware revision = 0x%04X\n",R[0]);

  if(cvSuccess != (status=read_reg2(base, V1290_FW_REV_ADD, d) ) ) return status;
  printf("V1290 firmware revision = 0x%04X\n",d);
	
  return status;
}

//---------------- End V1290 TDC functions --------------------------------
//---------------- V259 functions --------------------------------------

int V259_read(const uint32_t base, uint16_t& data){
  uint32_t addr=base+0x22;
  return CAENVME_ReadCycle(BHandle, addr, &data, cvA24_S_DATA, cvD16);
}

int V259_clear(const uint32_t base){
  uint32_t addr=base+0x20;
  uint16_t data=0;
  return CAENVME_WriteCycle(BHandle, addr, &data, cvA24_S_DATA, cvD16);
}

//---------------- End V259 functions --------------------------------
//---------------- CORBO functions --------------------------------------

int CORBO_init(int base){
  int ret=0;
  // disable all channels
  for(int ch=0; ch<4; ++ch){
    if(cvSuccess!=(ret=CORBO_write_CSR(base, ch, 0xFF)) ){ // disable all inputs
      printf("%s: error %d in CORBO_write_CSR\n", __func__,ret);
      return -ch*10-1;
    }
    if(cvSuccess!=(ret=CORBO_set_IRQ(base, ch, 0, 0x00, 0x00)) ){ // disable all event irq
      printf("%s: error %d while disabling event IRQ ch %d\n", __func__,ret,ch);
      return -ch*10-2;
    }
    if(cvSuccess!=(ret=CORBO_set_IRQ(base, ch, 1, 0x00, 0x00)) ){ // disable all timeout irq
      printf("%s: error %d while disabling timeout IRQ ch %d\n", __func__,ret,ch);
      return -ch*10-3;
    }
  }
  
  if(cvSuccess!=(ret=CORBO_write_CSR(base, VME_CRB_CH, 0xEC)) ){ // enable main channel, test input
    printf("%s: error %d in CORBO_write_CSR, ch %d, CSR=0xEC\n", __func__,ret,VME_CRB_CH);
    return -101;
  }
  if(cvSuccess!=(ret=CORBO_setbusy(base, VME_CRB_CH)) ){ // set it busy
    printf("%s: error %d in CORBO_setbusy, ch %d\n", __func__,ret,VME_CRB_CH);
    return -102;
  }
  if(cvSuccess!=(ret=CORBO_write_CSR(base, VME_CRB_CH, 0x00)) ){ // enable main channel for work
    printf("%s: error %d in CORBO_write_CSR, ch %d, CSR=0x00\n", __func__,ret,VME_CRB_CH);
    return -103;
  }
  uint16_t csr=0;
  if(cvSuccess!=(ret=CORBO_read_CSR(base, VME_CRB_CH, csr)) ){ // check CSR
    printf("%s: error %d in CORBO_read_CSR, ch %d\n", __func__,ret,VME_CRB_CH);
    return -104;
  }
  if(0x00!=(csr&0xFF)){
    printf("WARNING: bad csrL = 0x%x for work, CORBO ch %d\n", (csr&0xFF), VME_CRB_CH);
    return -105;
  }
  
  if(VME_CRB_CH2!=VME_CRB_CH){// the channel for pulse generation
    if(cvSuccess!=(ret=CORBO_write_CSR(base, VME_CRB_CH2, 0xEC)) ){ // enable main channel, test input
      printf("%s: error %d in CORBO_write_CSR, ch %d, CSR=0xEC\n", __func__,ret,VME_CRB_CH2);
      return -101;
    }
    if(cvSuccess!=(ret=CORBO_setbusy(base, VME_CRB_CH2)) ){ // set it busy
      printf("%s: error %d in CORBO_write_CSR, ch %d, CSR=0xEC\n", __func__,ret,VME_CRB_CH2);
      return -102;
    }
    uint16_t csr2=0;
    if(cvSuccess!=(ret=CORBO_read_CSR(base, VME_CRB_CH2, csr2)) ){ // check CSR
      printf("%s: error %d in CORBO_read_CSR, ch %d\n", __func__,ret,VME_CRB_CH2);
      return -104;
    }
    if(0xEC!=(csr2&0xFF)){
      printf("WARNING: bad csrL = 0x%x for pulser, CORBO ch %d\n", (csr2&0xFF), VME_CRB_CH2);
      return -105;
    }
  }
  return 0;
}

int CORBO_set_IRQ(const uint32_t base, int ch, int type, uint16_t cr, uint16_t vr){
  if(ch<0 || ch>3) return -1; // channels 0-3
  if(type!=0) type=1;         // type=0: event; type=1; timeout
  int ret=0;
  // cr - control
  uint32_t addr_cr = base + 0x30 + 2*ch + 0x10*type;
  uint16_t data_cr = cr&0xFF;
  if(data_cr>0)data_cr|=0x10; // set Enable IRQ if needed (i.e. if cr>0)
  if(cvSuccess!=(ret=CAENVME_WriteCycle(BHandle, addr_cr, &data_cr, cvA24_S_DATA, cvD16))){
    printf("%s: error %d writing CR: %d to chan %d\n", __func__,ret,cr,ch);
    return -1;
  }
  // vr - vec
  uint32_t addr_vr=addr_cr+8;
  uint16_t data_vr=vr&0xFF;
  if(cvSuccess!=(ret=CAENVME_WriteCycle(BHandle, addr_vr, &data_vr, cvA24_S_DATA, cvD16))){
    printf("%s: error %d writing VR: %d to chan %d\n", __func__,ret,vr,ch);
    return -1;
  }
  return 0;
}

int CORBO_get_IRQ(const uint32_t base, int ch, int type, uint16_t &cr, uint16_t &vr){
  if(ch<0 || ch>3) return -1; // channels 0-3
  if(type!=0) type=1;         // type=0: event; type=1; timeout
  int ret=0;
  // cr - control
  uint32_t addr_cr = base + 0x30 + 2*ch + 0x10*type;
  if(cvSuccess!=(ret=CAENVME_ReadCycle(BHandle, addr_cr, &cr, cvA24_S_DATA, cvD16))){
    printf("%s: error %d reading CR: %d to chan %d\n", __func__,ret,cr,ch);
    return -1;
  }
  // vr - vec
  uint32_t addr_vr=addr_cr+8;
  if(cvSuccess!=(ret=CAENVME_ReadCycle(BHandle, addr_vr, &vr, cvA24_S_DATA, cvD16))){
    printf("%s: error %d reading VR: %d to chan %d\n", __func__,ret,vr,ch);
    return -1;
  }
  cr&=0xFF;
  vr&=0xFF;
  return 0;
}

int CORBO_setbusy(const uint32_t base, int ch){ 
  if(ch<0 || ch>3) return -1;
  uint16_t csr=0,csr1=0,csr2=0;
  int ret=0;
  
  if(cvSuccess!=(ret=CORBO_read_CSR(base, ch, csr))){
    printf("%s: error in CORBO_read_CSR: %d\n", __func__,ret);
    return -1;
  }
  if(0!=(csr&1)){
    printf("%s: channel is disabled\n",__func__);
    return -2;
  }
  
  csr1=(csr&0xFC) | 0x0C; // modify csr: enable channel, busy mode, test input
  
  if(csr1!=csr){
    if(cvSuccess!=(ret=CORBO_write_CSR(base, ch, csr1))){ // write modified csr
      printf("%s: error %d in CORBO_write_CSR\n", __func__,ret);
      return -3;
    }
  }
  
  if(cvSuccess!=(ret=CORBO_test(base, ch))){ // write TEST to set busy
    printf("%s: error in CORBO_test: %d\n", __func__,ret);
    return -4;
  }
  
  csr2 = (csr & 0xFC) ; // final csr: initial csr + enable channel + busy mode
  
  if(csr2!=csr1){
    if(cvSuccess!=(ret=CORBO_write_CSR(base, ch, csr2))){ // restore old input selection
      printf("%s: error %d in CORBO_write_CSR\n", __func__,ret);
      return -5;
    }
  }
  
  return 0;
}

int CORBO_test(const uint32_t base, int ch){ // write TEST to set busy
  if(ch<0 || ch>3) return -1;
  uint16_t data = 0xFF;
  uint32_t addr=base+0x50+2*ch;
  return CAENVME_WriteCycle(BHandle, addr, &data, cvA24_S_DATA, cvD16);
}

int CORBO_clear(const uint32_t base, int ch){ 
  if(ch<0 || ch>3) return -1;
  uint16_t data = 0x00;
  uint32_t addr=base+0x58+2*ch;
  return CAENVME_WriteCycle(BHandle, addr, &data, cvA24_S_DATA, cvD16);
}

int CORBO_read_CSR(const uint32_t base, int ch, uint16_t& data){
  if(ch<0 || ch>3) return -1;
  uint32_t addr=base+2*ch;// CSR
  return CAENVME_ReadCycle(BHandle, addr, &data, cvA24_S_DATA, cvD16);
}

int CORBO_write_CSR(const uint32_t base, const int ch, uint16_t data){
  if(ch<0 || ch>3) return -1;
  data &= 0xFF; // only csrL is writable
  uint32_t addr=base+2*ch;// CSR
  return CAENVME_WriteCycle(BHandle, addr, &data, cvA24_S_DATA, cvD16);
}

//---------------- End CORBO functions --------------------------------
//---------------- ADC functions --------------------------------------

int ADC_write_CSR(const uint32_t base, uint16_t& data){
  data |= 0x4; // I want always front panel gate
  uint32_t addr=base+0;// CSR
  return CAENVME_WriteCycle(BHandle, addr, &data, cvA24_S_DATA, cvD16);
}

int ADC_clear(const uint32_t base){
  uint16_t csr_clear=0x0104;
  return ADC_write_CSR(base, csr_clear);
}

int ADC_read_CSR(const uint32_t base, uint16_t& data){
  uint32_t addr=base+0;// CSR
  return CAENVME_ReadCycle(BHandle, addr, &data, cvA24_S_DATA, cvD16);
}

int ADC_read_data(const uint32_t base, int& evcnt, uint16_t* data){
  int res;
  uint16_t csr;
  int csrcnt;
  int bytecnt=0;
  
  char iadc[8];
  
  if(VME_ADC1==base)strcpy(iadc,"ADC1");
  else if(VME_ADC2==base)strcpy(iadc,"ADC2");
  else if(VME_ADC3==base)strcpy(iadc,"ADC3");
  
  short fifo;
  res=CAENVME_GetFIFOMode(BHandle, &fifo);
  if(cvSuccess!=res){printf("%s %s: Error in CAENVME_GetFIFOMode\n", __func__,iadc);return res;}
  
  if(0==fifo){
    res=CAENVME_SetFIFOMode(BHandle, 1);
    if(cvSuccess!=res){printf("%s %s: Error in CAENVME_SetFIFOMode\n", __func__,iadc);return res;}
    
    res=CAENVME_GetFIFOMode(BHandle, &fifo);
    if(cvSuccess!=res){printf("%s %s: Error in CAENVME_GetFIFOMode\n", __func__,iadc);return res;}
    if(0==fifo){printf("%s %s: cannot disable fifo mode\n", __func__,iadc);return cvGenericError;}
  }
  
  for(csrcnt=0; csrcnt<100; ++csrcnt){
    res=ADC_read_CSR(base, csr);
    if(cvSuccess!=res){
      printf("%s %s: Error in ADC_read_CSR\n", __func__,iadc);
      goto clearadc;
    }
    if(0==(csr&2))break;
  }
  
  if(csrcnt>98){
    printf("%s %s: ADC error, conversion too long\n", __func__,iadc);
    res=cvGenericError;
    goto clearadc;
  }
  
  if( 0==(csr&0xF8) ){
    printf("%s %s: ADC buffer overflow\n", __func__,iadc);
    res=cvGenericError;
    goto clearadc;
  }
  
  evcnt=(csr&0xFF)>>4;
  if(1!=evcnt){
    printf("%s %s: %d events in the buffer\n", __func__,iadc,evcnt);
    goto clearadc;
  }
    
  
  res=CAENVME_BLTReadCycle(BHandle,base+0x100,(unsigned char*)data,16*evcnt,cvA24_S_BLT,cvD16,&bytecnt);
  if(cvSuccess!=res){
    printf("%s %s: Error in CAENVME_BLTReadCycle, code %d\n", __func__,iadc,res);
    goto clearadc;
  }
  else if(16*evcnt!=bytecnt){
    printf("%s %s: wrong byte count = %d\n", __func__,iadc, bytecnt);
    goto clearadc;
  }
  
 clearadc:
  
  int resclear=ADC_clear(base);
  if(cvSuccess!=resclear){
    printf("%s %s: Error clear ADC %d\n", __func__, iadc, resclear);
  }
  
  return res;
}

int ADC_read_data1(const uint32_t base, int& evcnt, uint16_t* data){
  int res;
  
  char iadc[8];
  if(VME_ADC1==base)strcpy(iadc,"ADC1");
  else if(VME_ADC2==base)strcpy(iadc,"ADC2");
  else if(VME_ADC3==base)strcpy(iadc,"ADC3");
  
  uint16_t csr;
  int csrcnt;
  for(csrcnt=0; csrcnt<100; ++csrcnt){
    res=ADC_read_CSR(base, csr);
    if(cvSuccess!=res){printf("%s %s: Error in ADC_read_CSR\n", __func__,iadc);return res;}
    if(0==(csr&2))break;
  }
  
  if(csrcnt>98){
    printf("%s %s: ADC error, conversion too long\n", __func__,iadc);
    return cvGenericError;
  }
  
  if( 0==(csr&0xF8) ){
    printf("%s %s: ADC buffer overflow\n", __func__,iadc);
    return cvGenericError;
  }
  
  evcnt=(csr&0xFF)>>4;
  
  for(int ia=0; ia<8*evcnt; ++ia){
    uint32_t ad=base+0x100+2*ia;
    res=CAENVME_ReadCycle(BHandle,ad,(unsigned char*)&data[ia],cvA24_S_DATA,cvD16);
    if(cvSuccess!=res){printf("%s %s: Error in CAENVME_ReadCycle\n", __func__,iadc); return res;}
  }
  
  res=ADC_clear(base);
  if(cvSuccess!=res){printf("%s %s: Error clear ADC, iadc\n", __func__);return res;}

  return cvSuccess;
}

//-------- End ADC functions ---------------------------------------------------------------------

int vme_init(int pulse_len){
  CVBoardTypes  VMEBoard=cvV2718;
  short         Link=0;
  short         Device=0;
  int res;
  
  g_ADC1_installed=true;
  g_ADC2_installed=true;
  g_ADC3_installed=true;
  
  if(VME_ADC1<=0)g_ADC1_installed=false;
  if(VME_ADC2<=0)g_ADC2_installed=false;
  if(VME_ADC3<=0)g_ADC3_installed=false;
  
  if(!g_vme_initialized){
    res=CAENVME_Init(VMEBoard, Link, Device, &BHandle);
    if(cvSuccess!=res){printf("%s: Error in CAENVME_Init\n", __func__); return res;}
    g_vme_initialized=true;
  }
  else{
    res=CAENVME_DeviceReset(BHandle);
    if(cvSuccess!=res){printf("%s: Error in CAENVME_DeviceReset\n", __func__); return res;}
  }
  
  res=CAENVME_SetOutputConf(BHandle, cvOutput0, cvDirect, cvActiveHigh, cvMiscSignals);
  if(cvSuccess!=res){printf("%s: Error in CAENVME_SetOutputConf cvOutput0\n", __func__); return res;}
  
  res=CAENVME_SetOutputConf(BHandle, cvOutput1, cvDirect, cvActiveHigh, cvMiscSignals);
  if(cvSuccess!=res){printf("%s: Error in CAENVME_SetOutputConf cvOutput1\n", __func__); return res;}
  
  res=CAENVME_SetOutputConf(BHandle, cvOutput2, cvDirect, cvActiveHigh, cvMiscSignals);
  if(cvSuccess!=res){printf("%s: Error in CAENVME_SetOutputConf cvOutput2\n", __func__); return res;}
  
  res=CAENVME_SetOutputConf(BHandle, cvOutput3, cvDirect, cvActiveHigh, cvMiscSignals);
  if(cvSuccess!=res){printf("%s: Error in CAENVME_SetOutputConf cvOutput3\n", __func__); return res;}
  
  res=CAENVME_SetPulserConf(BHandle, cvPulserA, 20, pulse_len, cvUnit25ns, 1, cvManualSW, cvManualSW);
  if(cvSuccess!=res){printf("%s: Error in CAENVME_SetPulserConf\n", __func__); return res;}
  
  res=CAENVME_SetPulserConf(BHandle, cvPulserB, 20, pulse_len, cvUnit25ns, 1, cvManualSW, cvManualSW);
  if(cvSuccess!=res){printf("%s: Error in CAENVME_SetPulserConf\n", __func__);return res;}
  
  res=V259_clear(VME_V259);
  if(cvSuccess!=res){printf("%s: Error clear V259\n", __func__);return res;}
  
  if(g_ADC1_installed && g_rp.ADC1_used){
    res=ADC_clear(VME_ADC1);
    if(cvSuccess!=res){
      printf("%s: Error clear ADC1: probably not installed\n", __func__);
      g_ADC1_installed=false;
    }
  }
  
  if(g_ADC2_installed && g_rp.ADC2_used){
    res=ADC_clear(VME_ADC2);
    if(cvSuccess!=res){
      printf("%s: Error clear ADC2: probably not installed\n", __func__);
      g_ADC2_installed=false;
    }
  }
  
  if(g_ADC3_installed && g_rp.ADC3_used){
    res=ADC_clear(VME_ADC3);
    if(cvSuccess!=res){
      printf("%s: Error clear ADC3: probably not installed\n", __func__);
      g_ADC3_installed=false;
    }
  }
  
  if(g_TDC_installed && g_rp.TDC_used){
    res=V1290_init(VME_V1290);
    if(cvSuccess!=res){
      printf("%s: Error init TDC: probably not installed\n", __func__);
      g_TDC_installed=false;
    }
  }
  
  res=CORBO_init(VME_CORBO);
  if(cvSuccess!=res){printf("%s: Error init CORBO\n", __func__);return res;}
  
  return 0;
}

int vme_pulse(int pattern){
  if(pattern<0 || pattern>1) return 0;
  
  CVPulserSelect puls=cvPulserA;
  if(0==pattern)puls=cvPulserB;
  
  int res=CAENVME_StartPulser(BHandle, puls);
  if(cvSuccess!=res)printf("%s: Error in CAENVME_StartPulser\n", __func__);
  return res;
}

int vme_pulseLED(){
  int res=CAENVME_StartPulser(BHandle, cvPulserA);
  if(cvSuccess!=res)printf("%s: Error in CAENVME_StartPulser\n", __func__);
  return res;
}

int vme_pulsePED(){
  int res=CAENVME_StartPulser(BHandle, cvPulserB);
  if(cvSuccess!=res)printf("%s: Error in CAENVME_StartPulser\n", __func__);
  return res;
}

int vme_read_pattern(){
  int res=cvSuccess;
  uint16_t pat;
  
  res=V259_read(VME_V259,pat);
  if(cvSuccess!=res){
    printf("%s: error in V259_read\n",__func__);
    return res;
  }
  
  res=V259_clear(VME_V259);
  if(cvSuccess!=res){
    printf("%s: error in V259_clear\n",__func__);
    return res;
  }
  
  g_pattern=pat;
  return res;
}

int vme_readTDC(){
  if(g_TDC_installed && g_rp.TDC_used){
    int ndata=0;
    uint32_t data[2048];
    int res=V1290_read_buffer(VME_V1290, 1024, ndata, data);
    if(cvSuccess!=res){
      V1290_print_buffer(ndata,data);
      ndata=0;
      g_nTDCclear++;
      //CORBO_clear(VME_CORBO,VME_CRB_CH);
      memset(g_nTDC,0,sizeof(g_nTDC));
      memset(g_tTDC,0,sizeof(g_tTDC));
      g_tTDCtrig=0;
      return -1;
    }
    //print_buffer_TDC(ndata,data);
    V1290_unpack_buffer(ndata,data);
  }
  return 0;
}

int vme_readADC(){
  uint16_t data[16];
  int evcnt;
  int res=0;
  
  if(g_ADC1_installed && g_rp.ADC1_used){
    res=ADC_read_data(VME_ADC1, evcnt, data);
    if(0!=res){
      printf("%s: error reading ADC1\n",__func__);
      return -1;
    }
    
    if(1==evcnt){
      for(int i=0; i<8; ++i)g_ADC[i]=data[i];
    }
    else if(evcnt<=0){printf("%s: no data from ADC1\n",__func__);return -1;}
    else if(evcnt>1) {printf("%s: %d ev in ADC1\n",__func__,evcnt);return -1;}
  }
  
  if(g_ADC2_installed && g_rp.ADC2_used){
    res=ADC_read_data(VME_ADC2, evcnt, data);
    if(0!=res){
      g_ADC2_installed=false;
      return 0;
    }
    
    if(1==evcnt){
      for(int i=8; i<16; ++i)g_ADC[i]=data[i-8];
    }
    else if(evcnt<=0){printf("%s: no data from ADC2\n",__func__);return -1;}
    else if(evcnt>1) {printf("%s: %d ev in ADC2\n",__func__,evcnt);return -1;}
  }

  if(g_ADC3_installed && g_rp.ADC3_used){
    res=ADC_read_data(VME_ADC3, evcnt, data);
    //res=ADC_read_data1(VME_ADC3, evcnt, data);
    if(0!=res){
      g_ADC3_installed=false;
      return 0;
    }
    
    if(1==evcnt){
      for(int i=16; i<24; ++i)g_ADC[i]=data[i-16];
    }
    else if(evcnt<=0){printf("%s: no data from ADC3\n",__func__);return -1;}
    else if(evcnt>1) {printf("%s: %d ev in ADC3\n",__func__,evcnt);return -1;}
  }

  return 0;
}

int vme_close(){
  int res;
  
  if(g_vme_initialized){
    res=CAENVME_End(BHandle);
    if(cvSuccess!=res){printf("%s: Error in CAENVME_End\n", __func__); return res;}
    g_vme_initialized=false;
  }
  
  return 0;
}

CVIRQLevels cvirq(uint32_t irq){
  if(1==irq)return cvIRQ1;
  else if(2==irq)return cvIRQ2;
  else if(3==irq)return cvIRQ3;
  else if(4==irq)return cvIRQ4;
  else if(5==irq)return cvIRQ5;
  else if(6==irq)return cvIRQ6;
  else if(7==irq)return cvIRQ7;
  else {
    printf("WARNING invalid IRQ level %d!!!\n",irq);
    return cvIRQ1;
  };
}

int vme_start(){
  int c_res=0;
  
  if( cvSuccess!=(c_res=CORBO_set_IRQ(VME_CORBO,VME_CRB_CH,0, 0x00, 0x00)) ){
    printf("%s: CORBO disable event IRQ ch %d returns %d\n",__func__,VME_CRB_CH,c_res);
    return c_res;
  }
  usleep(100000);
  
  if( cvSuccess!=(c_res=CORBO_setbusy(VME_CORBO,VME_CRB_CH)) ){
    printf("%s: CORBO set busy ch %d returns %d\n",__func__,VME_CRB_CH,c_res);
    return c_res;
  }
  usleep(100000);
  
  if(g_ADC1_installed && g_rp.ADC1_used){
    if( cvSuccess!=(c_res=ADC_clear(VME_ADC1)) ){
      printf("%s: clear ADC1 returns %d\n",__func__,c_res);
      return c_res;
    }
  }
  
  if(g_ADC2_installed && g_rp.ADC2_used){
    if( cvSuccess!=(c_res=ADC_clear(VME_ADC2)) ){
      printf("%s: clear ADC2 returns %d\n",__func__,c_res);
      return c_res;
    }
  }
  
  if(g_ADC3_installed && g_rp.ADC3_used){
    if( cvSuccess!=(c_res=ADC_clear(VME_ADC3)) ){
      printf("%s: clear ADC3 returns %d\n",__func__,c_res);
      return c_res;
    }
  }
  
  if( cvSuccess!=(c_res=V259_clear(VME_V259)) ){
    printf("%s: clear V259 returns %d\n",__func__,c_res);
    return c_res;
  }
  
  if( cvSuccess!=(c_res=V1290_clear(VME_V1290)) ){
    printf("%s: clear TDC returns %d\n",__func__,c_res);
    return c_res;
  }
  
  if( cvSuccess!=(c_res=V1290_count_reset(VME_V1290)) ){
    printf("%s: count reset TDC returns %d\n",__func__,c_res);
    return c_res;
  }
  
  //    if( cvSuccess!=(c_res=V260_clear_scalers(VME_V260)) ){
  //            printf("clear scalers V260: returns %d\n",c_res);
  //    }
  
  //CVIRQLevels mask_enable=cvirq(VME_CRB_IRQ);
  //if(cvSuccess!=(c_res=CAENVME_IRQEnable(BHandle, cvirq(VME_CRB_IRQ)))){; // level 3
  if(cvSuccess!=(c_res=CAENVME_IRQEnable(BHandle, 0xFFFFFFFF))){; // level 3
    printf("%s: Cannot enable VME IRQ\n",__func__);
    return c_res;
  }

  if( cvSuccess!=(c_res=CORBO_set_IRQ(VME_CORBO,VME_CRB_CH,0, VME_CRB_IRQ, VME_CRB_VEC)) ){ // level 3, vector 0x85
    printf("%s: CORBO enable event IRQ ch %d returns %d\n",__func__,VME_CRB_CH,c_res);
    return c_res;
  }
  
  uint16_t cr=0, vr=0;
  if( cvSuccess!=(c_res=CORBO_get_IRQ(VME_CORBO,VME_CRB_CH,0, cr, vr)) ){
    printf("%s: CORBO enable event IRQ ch %d returns %d\n",__func__,VME_CRB_CH,c_res);
    return c_res;
  }
  else printf("%s: CORBO IRQ written/read back: cr 0x%X / 0x%X; vr 0x%X / 0x%X\n",
              __func__, VME_CRB_IRQ, cr, VME_CRB_VEC, vr);
  
  if( cvSuccess!=(c_res=CORBO_clear(VME_CORBO,VME_CRB_CH)) ){
    printf("%s: CORBO clear ch %d returns %d\n",__func__,VME_CRB_CH,c_res);
    return c_res;
  }
  
  return c_res;
}

int vme_stop(){
  int c_res=0;

  if( cvSuccess!=(c_res=CORBO_set_IRQ(VME_CORBO,VME_CRB_CH,0, 0x00, 0x00)) ){
    printf("%s: CORBO disable event IRQ ch %d returns %d\n",__func__,VME_CRB_CH,c_res);
    return c_res;
  }
  usleep(100000);

  if( cvSuccess!=(c_res=CORBO_setbusy(VME_CORBO,VME_CRB_CH)) ){
    printf("%s: CORBO_set busy ch %d returns %d\n",__func__,VME_CRB_CH,c_res);
    return c_res;
  }
  usleep(100000);
  
  if( cvSuccess!=(c_res=ADC_clear(VME_ADC1)) ){
    printf("%s: clear ADC1 returns %d\n",__func__,c_res);
    return c_res;
  }
  
  if(g_ADC2_installed){
    if( cvSuccess!=(c_res=ADC_clear(VME_ADC2)) ){
      printf("%s: clear ADC2 returns %d\n",__func__,c_res);
      return c_res;
    }
  }
  
  if(g_ADC3_installed){
    if( cvSuccess!=(c_res=ADC_clear(VME_ADC3)) ){
      printf("%s: clear ADC3 returns %d\n",__func__,c_res);
      return c_res;
    }
  }
  
  return c_res;
}

int diftimeval(timeval tv1, timeval tv2){
  int dif_sec=tv2.tv_sec - tv1.tv_sec;
  int dif_usec=tv2.tv_usec - tv1.tv_usec;
  return 1000000*dif_sec + dif_usec;
}

int vme_wait(uint32_t timeout){
  int ret=0;
  unsigned char msk1=0, msk2=0;
  
  struct timeval start_time;
  struct timeval check_time;
  gettimeofday(&start_time, NULL);
  
  while(true){
    if(cvSuccess!=(ret=CAENVME_IRQCheck(BHandle, &msk1))){
      printf("%s: Cannot check IRQ\n",__func__);
      return ret;
    }
    if(msk1 & cvirq(VME_CRB_IRQ)){
      ret=0; 
      break;
    }
    gettimeofday(&check_time, NULL);
    int dt=diftimeval(start_time, check_time);
    if(dt>timeout*1000){
      return 111;
    }
  }

  //while( !(msk1 & cvirq(VME_CRB_IRQ)) ){
  //  if(cvSuccess!=(ret=CAENVME_IRQCheck(BHandle, &msk1))){
  //    printf("%s: Cannot check IRQ\n",__func__);
  //    return ret;
  //  }
  //}
  //ret=CAENVME_IRQWait(BHandle, cvirq(VME_CRB_IRQ), timeout);
  //ret=CAENVME_IRQWait(BHandle, 0xFFFFFFFF, timeout);
  //
  //if(cvSuccess!=ret){
  //  if (cvTimeoutError!=ret){
  //    printf("%s: %d returned\n",__func__,ret);
  //    return ret;
  //  }
  //  else return 111;
  //}
  //
  // then check irq AFTER WAIT
  //if(cvSuccess!=(ret=CAENVME_IRQCheck(BHandle, &msk2))){
  //  printf("%s: Cannot check IRQ\n",__func__);
  //  return ret;
  //}
  //
  //printf("%s: active interrupt before CAENVME_IRQWait: 0x%X; after: 0x%X\n",__func__,msk1,msk2);
  
  uint32_t vec=0;
  if(cvSuccess!=(ret=CAENVME_IACKCycle(BHandle, cvirq(VME_CRB_IRQ), &vec, cvD32))){; // level 3
    printf("%s Cannot acknowledge IRQ\n",__func__);
    return ret;
  }
  else if(VME_CRB_VEC!=(vec&0xFF)){
    printf("%s WARNING wrong interrupt vector %d\n",__func__,vec);
    return 112;
  }
  return 0;
}

int vme_wait0(uint32_t timeout){
  int ret=0;

  ret=CAENVME_IRQWait(BHandle, cvirq(VME_CRB_IRQ), timeout);
  
  uint32_t vec=0;
  if(cvSuccess!=(ret=CAENVME_IACKCycle(BHandle, cvirq(VME_CRB_IRQ), &vec, cvD32))){; // level 3
    printf("%s Cannot acknowledge IRQ\n",__func__);
    return ret;
  }
  else if(VME_CRB_VEC!=(vec&0xFF)){
    printf("%s WARNING wrong interrupt vector %d\n",__func__,vec);
    return 112;
  }
  return 0;
}

int vme_clearCORBO(){
    CORBO_clear(VME_CORBO,VME_CRB_CH);
}
