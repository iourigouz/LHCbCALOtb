#pragma once

#include <stdint.h>
#include <cctype>
#include <cstdio>
#include <cstring>

#define NADCCHAN 24
#define NTDCCHAN 32
#define NTDCMAXHITS 10
#define NDT5742CHAN 20
#define NDT5742SAMPL 2048

#define MAXCHANS 100
#define MAXNAMELENGTH 32

#define MAXLEDS 2

// VME modules

extern uint32_t VME_ADC1;     // 0xC30000   ADC1, LECROY 1182
extern uint32_t VME_ADC2;     // 0xC10000   ADC2, LECROY 1182
extern uint32_t VME_ADC3;     // 0xC20000   ADC3, LECROY 1182
extern uint32_t VME_CORBO;    // 0xF00000   CORBO = CES RCB 8047
extern uint32_t VME_CRB_CH;   // 0x000000   CORBO main channel
extern uint32_t VME_CRB_CH2;  // 0x000001   CORBO secondary channel (pulse gen etc)
extern uint32_t VME_V259;     // 0xC00000   pattern unit
extern uint32_t VME_V1290;    // 0xCC0000   CAEN TDC with NIM inputs
extern uint32_t VME_V260;     // 0x00DD00   CAEN scaler
extern uint32_t VME_V812;     // 0x880000   CAEN V812 constant fraction discriminator #1
extern uint32_t VME_V812_2;   // 0x990000   CAEN V812 constant fraction discriminator #2
extern uint32_t VME_CRB_IRQ;  // 3          VME IRQ ised in CORBO
extern uint32_t VME_CRB_VEC;  // 0x85       interrupt vector ised in CORBO

// connections


class runParam {
 public:
  bool init;
  double printperiod;
  int HVmaster, HVslave;
  
  int PEDpatt, LEDpatt, SIGpatt;

  int nLEDs;
  int LEDchan[MAXLEDS];
  double ULED[MAXLEDS];
  
  uint32_t vme_adc1, vme_adc2, vme_adc3;
  uint32_t vme_corbo, vme_crb_ch, vme_crb_ch2, vme_crb_irq, vme_crb_vec;
  uint32_t vme_v259, vme_v1290, vme_v260, vme_v812, vme_v812_2;
  
  int nchans;
  char chnam[MAXCHANS][MAXNAMELENGTH];
  int HVchan[MAXCHANS];
  double HV[MAXCHANS];
  int datatype[MAXCHANS];  // 1=ADC; 2=TDC; 3=DIG
  int datachan[MAXCHANS];
  
 public:
  runParam();
  
  void reset();  
  void setChanHV(char* nam, int ich, double v);
  void setLED(int iLED, int ich, double v);
  void setChanDataConn(char* nam, char* typ, int ich);
  void write(const char* fnam);
  void read(const char* fnam);
  int findch(const char* nam);
};

typedef runParam RUNPARAM;

extern RUNPARAM g_rp;
