#pragma once

#include <stdint.h>
#include <cctype>
#include <cstdio>
#include <cstring>

#define NADCCHAN 24
#define NTDCCHAN 32
#define NTDCMAXHITS 10
#define N742CHAN 36
#define N742SAMPL 2048
#define TR0DIG0CHAN 32
#define TR0DIG1CHAN 33
#define TR1DIG0CHAN 34
#define TR1DIG1CHAN 35

#define MAXCHANS 100
#define MAXNAMELENGTH 32

#define MAXLEDS 2

// VME modules

extern uint32_t VME_ADC1;     // 0xC30000   ADC1, LECROY 1182
extern uint32_t VME_ADC2;     // 0xC10000   ADC2, LECROY 1182
extern uint32_t VME_ADC3;     // 0xC20000   ADC3, LECROY 1182
extern uint32_t VME_CORBO;    // 0xF00000   CORBO = CES RCB 8047
extern uint32_t VME_CRB_CH;   // 0x000000   CORBO main channel (IRQ)
extern uint32_t VME_CRB_CH2;  // 0x000001   CORBO secondary channel (pulse gen etc)
extern uint32_t VME_CRB_CH3;  // 0x000002   CORBO channel used for a counter
extern uint32_t VME_CRB_CH4;  // 0x000003   CORBO channel used for a counter
extern uint32_t VME_CRB_IRQ;  // 3          VME IRQ ised in CORBO
extern uint32_t VME_CRB_VEC;  // 0x85       interrupt vector ised in CORBO
extern uint32_t VME_V259;     // 0xC00000   pattern unit
extern uint32_t VME_V1290;    // 0xCC0000   CAEN TDC with NIM inputs
extern uint32_t VME_V260;     // 0x00DD00   CAEN scaler
extern uint32_t VME_V812;     // 0x880000   CAEN V812 constant fraction discriminator #1
extern uint32_t VME_V812_2;   // 0x990000   CAEN V812 constant fraction discriminator #2

// connections


class runParam {
 public:
  bool init;
  double printperiod, writeperiod, cmdperiod, updperiod;
  
  int write_ntp; // whether to write a root tree with data
  int write_bin; // whether to write a binary file with data
  
  int PEDpatt, LEDpatt, SIGpatt;
  double PEDperiod, LEDperiod; // in seconds!!!

  int nLEDs;
  int LEDchan[MAXLEDS];
  double ULED[MAXLEDS];
  
  uint32_t vme_adc1, vme_adc2, vme_adc3;
  uint32_t vme_corbo, vme_crb_ch, vme_crb_ch2, vme_crb_ch3, vme_crb_ch4, vme_crb_irq, vme_crb_vec;
  uint32_t vme_v259, vme_v1290, vme_v260, vme_v812, vme_v812_2;
  
  int vme_conetnode, dig_conetnode, dig2_conetnode;
  
  char HVIP[MAXNAMELENGTH];
  int nHVchans;
  int HVchan[MAXCHANS];
  char HVname[MAXCHANS][MAXNAMELENGTH];
  double HV[MAXCHANS];
  
  int nchans;
  char chnam[MAXCHANS][MAXNAMELENGTH];
  int datatype[MAXCHANS];  // 1=ADC; 2=TDC; 3=DIG
  int datachan[MAXCHANS];
  // polarity for presentation. meaningful only for DIG
  // negative (default): A=ped-min
  // positive: A=max-ped
  int polarity[MAXCHANS];  // 777-positive, otherwise negative
  int dig_PED_summ;         // DIG PED summary plot: 0 -> AMP, otherwise PED
  int dig_adjust_offsets;    // requests dig pedestal adjust at start of run (0->NO, !=0->YES)
  int dig2_adjust_offsets;    // requests dig pedestal adjust at start of run (0->NO, !=0->YES)
  int dig_use_correction; // whether to use or not the factory corrections for digitizer#1
  int dig2_use_correction; // whether to use or not the factory corrections for digitizer#2
  double dig_posttrigger; // The posttrigger delay value, in % of the window (204.8 ns), default=5
  double dig2_posttrigger; // The posttrigger value for digitizer#2
  int dig_frequency; // The digitizer sampling frequency code: 0->5GHz, 1->2.5GHz, 2->1GHz, 3->0.75GHz
  int dig2_frequency; // The digitizer #2 frequency code
  char dig_calib_path[128];
  char dig2_calib_path[128];
  
  bool ADC1_used, ADC2_used, ADC3_used, ADC_used;
  bool TDC_used;
  bool digitizer_used;
  bool digitizer2_used;
  int used742[MAXCHANS];
  
  int evbuflen;
  int evoffset[MAXCHANS];
  
  // DWC calibration
  double cx1[3],cy1[3],cx2[3],cy2[3],cx3[3],cy3[3],cx4[3],cy4[3];
  
  //---------------digitizer calibration coefficients of Vincenzo ------------
  double dig_p0[2*N742CHAN][1024]; // coefficients for cell calibration
  double dig_p1[2*N742CHAN][1024]; // coefficients for cell calibration
  double dig_p2[2*N742CHAN][1024]; // coefficients for cell calibration
  double dig_pa0[2*N742CHAN][1024]; // coefficients for sample calibration
  double dig_pa1[2*N742CHAN][1024]; // coefficients for sample calibration
  double dig_pa2[2*N742CHAN][1024]; // coefficients for sample calibration
  double dig_timev[2*N742CHAN][1024];// cell time widths
  int dig_calibs_inited[2*N742CHAN]; // initialized or not
  
 public:
  runParam();
  
  void reset();  
  void setChanHV(char* nam, int ich, double v);
  void setLED(int iLED, int ich, double v);
  void setChanDataConn(char* nam, char* typ, int ich);
  void writestream(FILE* f);
  void write(const char* fnam);
  void readstream(FILE* f);
  void read(const char* fnam);
  int findch(const char* nam);
  int findch(const char* typ, int ch);

  int read_digitizer_calibs(const char* path, int ifreq, int JCH);
  void digitizer_calib_init();
  void digitizer2_calib_init();

};

typedef runParam RUNPARAM;

extern RUNPARAM g_rp;

int i2JCH(int i);
int JCH2i(int JCH);

