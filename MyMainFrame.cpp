#include <math.h>
#include <iostream>
#include <string>
#include <map>

#include "TROOT.h"
#include "TSystem.h"
#include "TRint.h"
//#include "TApplication.h"
#include "TGClient.h"
#include "TClass.h"
#include "TGGC.h"
#include "TGResourcePool.h"
#include "TGFont.h"
#include "TDirectory.h"
#include "TStyle.h"
#include "TGButton.h"
#include "Buttons.h"
#include "TGNumberEntry.h"
#include "TGComboBox.h"
#include "TGLabel.h"
#include "TTimer.h"
#include "TGFrame.h"
#include "TFrame.h"
#include "TRootEmbeddedCanvas.h"
#include "TGStatusBar.h"
#include "TCanvas.h"
#include "TF1.h"
#include "TRandom.h"
#include "TGraph.h"
#include "TAxis.h"
#include "TH2D.h"
#include "TH2F.h"
#include "TFile.h"
#include "TTimeStamp.h"

#include "dimHist.h"

#include "MyMainFrame.h"

#include "getVal.h"
#include "getComment.h"

#include "dic.hxx"
#include "dim_common.h"

// run status, commands, selections
bool g_running_prev=false;

bool g_startrun=false;
bool g_stoprun=false;
bool g_addcomment=false;

bool g_draw_hist=false;
bool g_auto_draw=false;
bool g_subtract_ref=false;
bool g_update_hists=false;

char g_patt_selected[32]="";
char g_type_selected[32]="";

char g_name_selected[256]="";
char g_name_sel_prev[256]="";

double g_t_startbutton_enabled=0;

// DIM stuff
DIMSTAT g_d_status;
//
DIMSUMMARY g_d_summ_ADC_LED, g_d_summ_ADC_PED, g_d_summ_ADC_SIG;
DIMSUMMARY g_d_summ_DIG_LED, g_d_summ_DIG_PED, g_d_summ_DIG_SIG;
DIMSUMMARY g_d_summ_TDC_LED, g_d_summ_TDC_SIG;
//
DIMHIST g_d_ADC_LED[NADCCHAN];
DIMHIST g_d_ADC_PED[NADCCHAN];
DIMHIST g_d_ADC_SIG[NADCCHAN];
DIMHIST g_d_dwc1x_LED, g_d_dwc1y_LED, g_d_dwc2x_LED, g_d_dwc2y_LED; 
DIMHIST g_d_dwc3x_LED, g_d_dwc3y_LED, g_d_dwc4x_LED, g_d_dwc4y_LED;
DIMHIST g_d_dwc1x_SIG, g_d_dwc1y_SIG, g_d_dwc2x_SIG, g_d_dwc2y_SIG; 
DIMHIST g_d_dwc3x_SIG, g_d_dwc3y_SIG, g_d_dwc4x_SIG, g_d_dwc4y_SIG;
DIMHIST g_d_DIG_LEDPED[2*N742CHAN];
DIMHIST g_d_DIG_LEDAMP[2*N742CHAN];
DIMHIST g_d_DIG_LEDWAV[2*N742CHAN];
DIMHIST g_d_DIG_PEDPED[2*N742CHAN];
DIMHIST g_d_DIG_PEDAMP[2*N742CHAN];
DIMHIST g_d_DIG_PEDWAV[2*N742CHAN];
DIMHIST g_d_DIG_SIGPED[2*N742CHAN];
DIMHIST g_d_DIG_SIGAMP[2*N742CHAN];
DIMHIST g_d_DIG_SIGWAV[2*N742CHAN];
// DIM dummies
DIMSTAT g_d_status_dummy;
DIMHIST g_d_hist_dummy;
DIMSUMMARY g_d_summ_dummy;
// DIM reference to subtract
DIMSTAT g_d_status_ref;
//
DIMSUMMARY g_d_summ_ADC_LED_ref, g_d_summ_ADC_PED_ref, g_d_summ_ADC_SIG_ref;
DIMSUMMARY g_d_summ_DIG_LED_ref, g_d_summ_DIG_PED_ref, g_d_summ_DIG_SIG_ref;
DIMSUMMARY g_d_summ_TDC_LED_ref, g_d_summ_TDC_SIG_ref;
//
DIMHIST g_d_ADC_LED_ref[NADCCHAN];
DIMHIST g_d_ADC_PED_ref[NADCCHAN];
DIMHIST g_d_ADC_SIG_ref[NADCCHAN];
DIMHIST g_d_dwc1x_LED_ref, g_d_dwc1y_LED_ref, g_d_dwc2x_LED_ref, g_d_dwc2y_LED_ref; 
DIMHIST g_d_dwc3x_LED_ref, g_d_dwc3y_LED_ref, g_d_dwc4x_LED_ref, g_d_dwc4y_LED_ref;
DIMHIST g_d_dwc1x_SIG_ref, g_d_dwc1y_SIG_ref, g_d_dwc2x_SIG_ref, g_d_dwc2y_SIG_ref; 
DIMHIST g_d_dwc3x_SIG_ref, g_d_dwc3y_SIG_ref, g_d_dwc4x_SIG_ref, g_d_dwc4y_SIG_ref;
DIMHIST g_d_DIG_LEDPED_ref[2*N742CHAN];
DIMHIST g_d_DIG_LEDAMP_ref[2*N742CHAN];
DIMHIST g_d_DIG_LEDWAV_ref[2*N742CHAN];
DIMHIST g_d_DIG_PEDPED_ref[2*N742CHAN];
DIMHIST g_d_DIG_PEDAMP_ref[2*N742CHAN];
DIMHIST g_d_DIG_PEDWAV_ref[2*N742CHAN];
DIMHIST g_d_DIG_SIGPED_ref[2*N742CHAN];
DIMHIST g_d_DIG_SIGAMP_ref[2*N742CHAN];
DIMHIST g_d_DIG_SIGWAV_ref[2*N742CHAN];
// end DIM stuff

ClassImp(MyMainFrame)

int add_logbook(const char* filenam, const char* config, const int irun, const char* state, const char* comment){
  FILE* f=fopen(filenam,"a");
  if(!f)return -1;

  char str[256];
  
  TTimeStamp tst;
  UInt_t year,month,day,hour,min,sec;
  tst.GetDate(kFALSE,0,&year,&month,&day);
  tst.GetTime(kFALSE,0,&hour,&min,&sec);
  
  fprintf(f," %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d %s_%2.2d %s\n",
          year,month,day, hour,min,sec, config,irun,state);
  
  int s=0;
  memset(str,0,sizeof(str));
  for(int i=0; i<(int)strlen(comment); ++i){
    if( ('\n'==comment[i]) || (s>250) ){
      str[s]=0;
      fprintf(f,"            %s\n",str);
      s=0;
      memset(str,0,sizeof(str));
    }
    else if(comment[i]>31)str[s++]=comment[i];
  }
  fprintf(f,"            %s\n",str);
  fclose(f);
  return 0;
}

int list_dimhists(){
  int nhists=0;
  
  if(g_d_summ_ADC_LED.isUsed()){nhists++; printf("%s\n",g_d_summ_ADC_LED.name);}
  if(g_d_summ_ADC_PED.isUsed()){nhists++; printf("%s\n",g_d_summ_ADC_PED.name);}
  if(g_d_summ_ADC_SIG.isUsed()){nhists++; printf("%s\n",g_d_summ_ADC_SIG.name);}
                                                                        
  if(g_d_summ_TDC_LED.isUsed()){nhists++; printf("%s\n",g_d_summ_TDC_LED.name);}
  if(g_d_summ_TDC_SIG.isUsed()){nhists++; printf("%s\n",g_d_summ_TDC_SIG.name);}
                                                                        
  if(g_d_summ_DIG_LED.isUsed()){nhists++; printf("%s\n",g_d_summ_DIG_LED.name);}
  if(g_d_summ_DIG_PED.isUsed()){nhists++; printf("%s\n",g_d_summ_DIG_PED.name);}
  if(g_d_summ_DIG_SIG.isUsed()){nhists++; printf("%s\n",g_d_summ_DIG_SIG.name);}
  
  for(int i=0; i<NADCCHAN; ++i){
    if(g_d_ADC_LED[i].isUsed()){nhists++; printf("%s\n",g_d_ADC_LED[i].name);}
    if(g_d_ADC_PED[i].isUsed()){nhists++; printf("%s\n",g_d_ADC_PED[i].name);}
    if(g_d_ADC_SIG[i].isUsed()){nhists++; printf("%s\n",g_d_ADC_SIG[i].name);}
  }
  
  if(g_d_dwc1x_LED.isUsed()){nhists++; printf("%s\n",g_d_dwc1x_LED.name);}
  if(g_d_dwc2x_LED.isUsed()){nhists++; printf("%s\n",g_d_dwc2x_LED.name);}
  if(g_d_dwc3x_LED.isUsed()){nhists++; printf("%s\n",g_d_dwc3x_LED.name);}
  if(g_d_dwc4x_LED.isUsed()){nhists++; printf("%s\n",g_d_dwc4x_LED.name);}
  if(g_d_dwc1y_LED.isUsed()){nhists++; printf("%s\n",g_d_dwc1y_LED.name);}
  if(g_d_dwc2y_LED.isUsed()){nhists++; printf("%s\n",g_d_dwc2y_LED.name);}
  if(g_d_dwc3y_LED.isUsed()){nhists++; printf("%s\n",g_d_dwc3y_LED.name);}
  if(g_d_dwc4y_LED.isUsed()){nhists++; printf("%s\n",g_d_dwc4y_LED.name);}
  if(g_d_dwc1x_SIG.isUsed()){nhists++; printf("%s\n",g_d_dwc1x_SIG.name);}
  if(g_d_dwc2x_SIG.isUsed()){nhists++; printf("%s\n",g_d_dwc2x_SIG.name);}
  if(g_d_dwc3x_SIG.isUsed()){nhists++; printf("%s\n",g_d_dwc3x_SIG.name);}
  if(g_d_dwc4x_SIG.isUsed()){nhists++; printf("%s\n",g_d_dwc4x_SIG.name);}
  if(g_d_dwc1y_SIG.isUsed()){nhists++; printf("%s\n",g_d_dwc1y_SIG.name);}
  if(g_d_dwc2y_SIG.isUsed()){nhists++; printf("%s\n",g_d_dwc2y_SIG.name);}
  if(g_d_dwc3y_SIG.isUsed()){nhists++; printf("%s\n",g_d_dwc3y_SIG.name);}
  if(g_d_dwc4y_SIG.isUsed()){nhists++; printf("%s\n",g_d_dwc4y_SIG.name);}
  
  for(int i=0; i<2*N742CHAN; ++i){
    if(g_d_DIG_LEDPED[i].isUsed()){nhists++; printf("%s\n",g_d_DIG_LEDPED[i].name);}
    if(g_d_DIG_LEDAMP[i].isUsed()){nhists++; printf("%s\n",g_d_DIG_LEDAMP[i].name);}
    if(g_d_DIG_LEDWAV[i].isUsed()){nhists++; printf("%s\n",g_d_DIG_LEDWAV[i].name);}
    if(g_d_DIG_PEDPED[i].isUsed()){nhists++; printf("%s\n",g_d_DIG_PEDPED[i].name);}
    if(g_d_DIG_PEDAMP[i].isUsed()){nhists++; printf("%s\n",g_d_DIG_PEDAMP[i].name);}
    if(g_d_DIG_PEDWAV[i].isUsed()){nhists++; printf("%s\n",g_d_DIG_PEDWAV[i].name);}
    if(g_d_DIG_SIGPED[i].isUsed()){nhists++; printf("%s\n",g_d_DIG_SIGPED[i].name);}
    if(g_d_DIG_SIGAMP[i].isUsed()){nhists++; printf("%s\n",g_d_DIG_SIGAMP[i].name);}
    if(g_d_DIG_SIGWAV[i].isUsed()){nhists++; printf("%s\n",g_d_DIG_SIGWAV[i].name);}
  }
  return nhists;
}

void reset_dimhists(){
  g_d_summ_ADC_LED.Reset();
  g_d_summ_ADC_PED.Reset();
  g_d_summ_ADC_SIG.Reset();
  g_d_summ_DIG_LED.Reset();
  g_d_summ_DIG_PED.Reset(); 
  g_d_summ_DIG_SIG.Reset();
  g_d_summ_TDC_LED.Reset();
  g_d_summ_TDC_SIG.Reset();
  //
  for(int i=0; i<NADCCHAN; ++i){
    g_d_ADC_LED[i].Reset();
    g_d_ADC_PED[i].Reset();
    g_d_ADC_SIG[i].Reset();
  }
  //
  g_d_dwc1x_LED.Reset();
  g_d_dwc1y_LED.Reset();
  g_d_dwc2x_LED.Reset();
  g_d_dwc2y_LED.Reset();
  g_d_dwc3x_LED.Reset();
  g_d_dwc3y_LED.Reset();
  g_d_dwc4x_LED.Reset();
  g_d_dwc4y_LED.Reset();
  g_d_dwc1x_SIG.Reset();
  g_d_dwc1y_SIG.Reset();
  g_d_dwc2x_SIG.Reset();
  g_d_dwc2y_SIG.Reset();
  g_d_dwc3x_SIG.Reset();
  g_d_dwc3y_SIG.Reset();
  g_d_dwc4x_SIG.Reset();
  g_d_dwc4y_SIG.Reset();
  //
  for(int i=0; i<2*N742CHAN; ++i){
    g_d_DIG_LEDPED[i].Reset();
    g_d_DIG_LEDAMP[i].Reset();
    g_d_DIG_LEDWAV[i].Reset();
    g_d_DIG_PEDPED[i].Reset();
    g_d_DIG_PEDAMP[i].Reset();
    g_d_DIG_PEDWAV[i].Reset();
    g_d_DIG_SIGPED[i].Reset();
    g_d_DIG_SIGAMP[i].Reset();
    g_d_DIG_SIGWAV[i].Reset();
  }
}

void* address_dimhist(char* type, char* patt, char* cell, int chan, char* suppl){
  void* ret=0;
  if(0==strcmp(cell,"summ"))return ret;  // return 0 if this is a dimsummary!
  
  if(0==strcmp(type,"ADC")){
    if(0==strcmp(patt,"PED")){
      if(chan>=0)ret=(void*)&g_d_ADC_PED[chan];
    }
    else if(0==strcmp(patt,"LED")){
      if(chan>=0)ret=(void*)&g_d_ADC_LED[chan];
    }
    else if(0==strcmp(patt,"SIG")){
      if(chan>=0)ret=(void*)&g_d_ADC_SIG[chan];
    }
  }
  else if(0==strcmp(type,"TDC")){
    if(0==strcmp(patt,"LED")){
      if(0==strcmp(cell,"dwc1x"))ret=(void*)&g_d_dwc1x_LED;
      if(0==strcmp(cell,"dwc2x"))ret=(void*)&g_d_dwc2x_LED;
      if(0==strcmp(cell,"dwc3x"))ret=(void*)&g_d_dwc3x_LED;
      if(0==strcmp(cell,"dwc4x"))ret=(void*)&g_d_dwc4x_LED;
      if(0==strcmp(cell,"dwc1y"))ret=(void*)&g_d_dwc1y_LED;
      if(0==strcmp(cell,"dwc2y"))ret=(void*)&g_d_dwc2y_LED;
      if(0==strcmp(cell,"dwc3y"))ret=(void*)&g_d_dwc3y_LED;
      if(0==strcmp(cell,"dwc4y"))ret=(void*)&g_d_dwc4y_LED;
    }
    else if(0==strcmp(patt,"SIG")){
      if(0==strcmp(cell,"dwc1x"))ret=(void*)&g_d_dwc1x_SIG;
      if(0==strcmp(cell,"dwc2x"))ret=(void*)&g_d_dwc2x_SIG;
      if(0==strcmp(cell,"dwc3x"))ret=(void*)&g_d_dwc3x_SIG;
      if(0==strcmp(cell,"dwc4x"))ret=(void*)&g_d_dwc4x_SIG;
      if(0==strcmp(cell,"dwc1y"))ret=(void*)&g_d_dwc1y_SIG;
      if(0==strcmp(cell,"dwc2y"))ret=(void*)&g_d_dwc2y_SIG;
      if(0==strcmp(cell,"dwc3y"))ret=(void*)&g_d_dwc3y_SIG;
      if(0==strcmp(cell,"dwc4y"))ret=(void*)&g_d_dwc4y_SIG;
    }
  }
  else if(0==strcmp(type,"DIG")){
    if(0==strcmp(patt,"PED")){
      if(0==strcmp(suppl,"PED")){
        if(chan>=0)ret=(void*)&g_d_DIG_PEDPED[chan];
      }
      else if(0==strcmp(suppl,"AMP")){
        if(chan>=0)ret=(void*)&g_d_DIG_PEDAMP[chan];
      }
      else if(0==strcmp(suppl,"WAV")){
        if(chan>=0)ret=(void*)&g_d_DIG_PEDWAV[chan];
      }
    }
    else if(0==strcmp(patt,"LED")){
      if(0==strcmp(suppl,"PED")){
        if(chan>=0)ret=(void*)&g_d_DIG_LEDPED[chan];
      }
      else if(0==strcmp(suppl,"AMP")){
        if(chan>=0)ret=(void*)&g_d_DIG_LEDAMP[chan];
      }
      else if(0==strcmp(suppl,"WAV")){
        if(chan>=0)ret=(void*)&g_d_DIG_LEDWAV[chan];
      }
    }
    else if(0==strcmp(patt,"SIG")){
      if(0==strcmp(suppl,"PED")){
        if(chan>=0)ret=(void*)&g_d_DIG_SIGPED[chan];
      }
      else if(0==strcmp(suppl,"AMP")){
        if(chan>=0)ret=(void*)&g_d_DIG_SIGAMP[chan];
      }
      else if(0==strcmp(suppl,"WAV")){
        if(chan>=0)ret=(void*)&g_d_DIG_SIGWAV[chan];
      }
    }
  }
  return ret;
}

void* address_ref_dimhist(char* type, char* patt, char* cell, int chan, char* suppl){
  void* ret=0;
  if(0==strcmp(cell,"summ"))return ret;  // return 0 if this is a dimsummary!
  
  if(0==strcmp(type,"ADC")){
    if(0==strcmp(patt,"PED")){
      if(chan>=0)ret=(void*)&g_d_ADC_PED_ref[chan];
    }
    else if(0==strcmp(patt,"LED")){
      if(chan>=0)ret=(void*)&g_d_ADC_LED_ref[chan];
    }
    else if(0==strcmp(patt,"SIG")){
      if(chan>=0)ret=(void*)&g_d_ADC_SIG_ref[chan];
    }
  }
  else if(0==strcmp(type,"TDC")){
    if(0==strcmp(patt,"LED")){
      if(0==strcmp(cell,"dwc1x"))ret=(void*)&g_d_dwc1x_LED_ref;
      if(0==strcmp(cell,"dwc2x"))ret=(void*)&g_d_dwc2x_LED_ref;
      if(0==strcmp(cell,"dwc3x"))ret=(void*)&g_d_dwc3x_LED_ref;
      if(0==strcmp(cell,"dwc4x"))ret=(void*)&g_d_dwc4x_LED_ref;
      if(0==strcmp(cell,"dwc1y"))ret=(void*)&g_d_dwc1y_LED_ref;
      if(0==strcmp(cell,"dwc2y"))ret=(void*)&g_d_dwc2y_LED_ref;
      if(0==strcmp(cell,"dwc3y"))ret=(void*)&g_d_dwc3y_LED_ref;
      if(0==strcmp(cell,"dwc4y"))ret=(void*)&g_d_dwc4y_LED_ref;
    }
    else if(0==strcmp(patt,"SIG")){
      if(0==strcmp(cell,"dwc1x"))ret=(void*)&g_d_dwc1x_SIG_ref;
      if(0==strcmp(cell,"dwc2x"))ret=(void*)&g_d_dwc2x_SIG_ref;
      if(0==strcmp(cell,"dwc3x"))ret=(void*)&g_d_dwc3x_SIG_ref;
      if(0==strcmp(cell,"dwc4x"))ret=(void*)&g_d_dwc4x_SIG_ref;
      if(0==strcmp(cell,"dwc1y"))ret=(void*)&g_d_dwc1y_SIG_ref;
      if(0==strcmp(cell,"dwc2y"))ret=(void*)&g_d_dwc2y_SIG_ref;
      if(0==strcmp(cell,"dwc3y"))ret=(void*)&g_d_dwc3y_SIG_ref;
      if(0==strcmp(cell,"dwc4y"))ret=(void*)&g_d_dwc4y_SIG_ref;
    }
  }
  else if(0==strcmp(type,"DIG")){
    if(0==strcmp(patt,"PED")){
      if(0==strcmp(suppl,"PED")){
        if(chan>=0)ret=(void*)&g_d_DIG_PEDPED_ref[chan];
      }
      else if(0==strcmp(suppl,"AMP")){
        if(chan>=0)ret=(void*)&g_d_DIG_PEDAMP_ref[chan];
      }
      else if(0==strcmp(suppl,"WAV")){
        if(chan>=0)ret=(void*)&g_d_DIG_PEDWAV_ref[chan];
      }
    }
    else if(0==strcmp(patt,"LED")){
      if(0==strcmp(suppl,"PED")){
        if(chan>=0)ret=(void*)&g_d_DIG_LEDPED_ref[chan];
      }
      else if(0==strcmp(suppl,"AMP")){
        if(chan>=0)ret=(void*)&g_d_DIG_LEDAMP_ref[chan];
      }
      else if(0==strcmp(suppl,"WAV")){
        if(chan>=0)ret=(void*)&g_d_DIG_LEDWAV_ref[chan];
      }
    }
    else if(0==strcmp(patt,"SIG")){
      if(0==strcmp(suppl,"PED")){
        if(chan>=0)ret=(void*)&g_d_DIG_SIGPED_ref[chan];
      }
      else if(0==strcmp(suppl,"AMP")){
        if(chan>=0)ret=(void*)&g_d_DIG_SIGAMP_ref[chan];
      }
      else if(0==strcmp(suppl,"WAV")){
        if(chan>=0)ret=(void*)&g_d_DIG_SIGWAV_ref[chan];
      }
    }
  }
  return ret;
}

void* address_dimsummary(char* type, char* patt, char* cell, int chan, char* suppl){
  void* ret=0;
  if(0!=strcmp(cell,"summ"))return ret;  // return 0 if this is NOT a dimsummary
  
  if(0==strcmp(type,"ADC")){
    if     (0==strcmp(patt,"PED")) ret=(void*)&g_d_summ_ADC_PED;
    else if(0==strcmp(patt,"LED")) ret=(void*)&g_d_summ_ADC_LED;
    else if(0==strcmp(patt,"SIG")) ret=(void*)&g_d_summ_ADC_SIG;
  }
  else if(0==strcmp(type,"TDC")){
    if     (0==strcmp(patt,"LED")) ret=(void*)&g_d_summ_TDC_LED;
    else if(0==strcmp(patt,"SIG")) ret=(void*)&g_d_summ_TDC_SIG;
  }
  else if(0==strcmp(type,"DIG")){
    if     (0==strcmp(patt,"PED")) ret=(void*)&g_d_summ_DIG_PED;
    else if(0==strcmp(patt,"LED")) ret=(void*)&g_d_summ_DIG_LED;
    else if(0==strcmp(patt,"SIG")) ret=(void*)&g_d_summ_DIG_SIG;
  }
  return ret;
}

void* address_ref_dimsummary(char* type, char* patt, char* cell, int chan, char* suppl){
  void* ret=0;
  if(0!=strcmp(cell,"summ"))return ret;  // return 0 if this is NOT a dimsummary
  
  if(0==strcmp(type,"ADC")){
    if     (0==strcmp(patt,"PED")) ret=(void*)&g_d_summ_ADC_PED_ref;
    else if(0==strcmp(patt,"LED")) ret=(void*)&g_d_summ_ADC_LED_ref;
    else if(0==strcmp(patt,"SIG")) ret=(void*)&g_d_summ_ADC_SIG_ref;
  }
  else if(0==strcmp(type,"TDC")){
    if     (0==strcmp(patt,"LED")) ret=(void*)&g_d_summ_TDC_LED_ref;
    else if(0==strcmp(patt,"SIG")) ret=(void*)&g_d_summ_TDC_SIG_ref;
  }
  else if(0==strcmp(type,"DIG")){
    if     (0==strcmp(patt,"PED")) ret=(void*)&g_d_summ_DIG_PED_ref;
    else if(0==strcmp(patt,"LED")) ret=(void*)&g_d_summ_DIG_LED_ref;
    else if(0==strcmp(patt,"SIG")) ret=(void*)&g_d_summ_DIG_SIG_ref;
  }
  return ret;
}

int parse_service_name(const char* servicename, char* type, char* patt, char* cell, int& chan, char* suppl){
  strncpy(type,"",3);
  strncpy(patt,"",3);
  strncpy(cell,"",3);
  chan=-1;
  strncpy(suppl,"",3);
  
  char str[256];
  strncpy(str,servicename,255); str[255]=0;
  char* s[10];
  int nel=readline_strings(str, '_',9,s);
  if(nel>0)strncpy(cell,s[0],15);
  if(nel>1)strncpy(patt,s[1],7);
  if(nel>2)strncpy(type,s[2],7);
  if(nel>3)sscanf(s[3],"%d",&chan);
  if(nel>4)strncpy(suppl,s[4],15);
  
  return nel;
}

int sendcom(const char *cd){
  char srv[32];
  strcpy(srv,"TB_DAQ_CMD");
  
  char cm[64];
  sprintf(cm,"%s",cd);
  
  printf("%s --> %s\n",cm,srv);
  return dic_cmnd_service(srv,cm,sizeof(cm));
}

void MyMainFrame::Do2Ref(){
  g_d_status_ref.Copy(g_d_status);
  
  if(g_d_summ_ADC_LED.isUsed())g_d_summ_ADC_LED_ref.Copy(g_d_summ_ADC_LED);
  if(g_d_summ_ADC_PED.isUsed())g_d_summ_ADC_PED_ref.Copy(g_d_summ_ADC_PED);
  if(g_d_summ_ADC_SIG.isUsed())g_d_summ_ADC_SIG_ref.Copy(g_d_summ_ADC_SIG);
  if(g_d_summ_DIG_LED.isUsed())g_d_summ_DIG_LED_ref.Copy(g_d_summ_DIG_LED);
  if(g_d_summ_DIG_PED.isUsed())g_d_summ_DIG_PED_ref.Copy(g_d_summ_DIG_PED); 
  if(g_d_summ_DIG_SIG.isUsed())g_d_summ_DIG_SIG_ref.Copy(g_d_summ_DIG_SIG);
  if(g_d_summ_TDC_LED.isUsed())g_d_summ_TDC_LED_ref.Copy(g_d_summ_TDC_LED);
  if(g_d_summ_TDC_SIG.isUsed())g_d_summ_TDC_SIG_ref.Copy(g_d_summ_TDC_SIG);
  //
  for(int i=0; i<NADCCHAN; ++i){
    if(g_d_ADC_LED[i].isUsed())g_d_ADC_LED_ref[i].Copy(g_d_ADC_LED[i]);
    if(g_d_ADC_PED[i].isUsed())g_d_ADC_PED_ref[i].Copy(g_d_ADC_PED[i]);
    if(g_d_ADC_SIG[i].isUsed())g_d_ADC_SIG_ref[i].Copy(g_d_ADC_SIG[i]);
  }
  //
  if(g_d_dwc1x_LED.isUsed())g_d_dwc1x_LED_ref.Copy(g_d_dwc1x_LED);
  if(g_d_dwc1y_LED.isUsed())g_d_dwc1y_LED_ref.Copy(g_d_dwc1y_LED);
  if(g_d_dwc2x_LED.isUsed())g_d_dwc2x_LED_ref.Copy(g_d_dwc2x_LED);
  if(g_d_dwc2y_LED.isUsed())g_d_dwc2y_LED_ref.Copy(g_d_dwc2y_LED);
  if(g_d_dwc3x_LED.isUsed())g_d_dwc3x_LED_ref.Copy(g_d_dwc3x_LED);
  if(g_d_dwc3y_LED.isUsed())g_d_dwc3y_LED_ref.Copy(g_d_dwc3y_LED);
  if(g_d_dwc4x_LED.isUsed())g_d_dwc4x_LED_ref.Copy(g_d_dwc4x_LED);
  if(g_d_dwc4y_LED.isUsed())g_d_dwc4y_LED_ref.Copy(g_d_dwc4y_LED);
  if(g_d_dwc1x_SIG.isUsed())g_d_dwc1x_SIG_ref.Copy(g_d_dwc1x_SIG);
  if(g_d_dwc1y_SIG.isUsed())g_d_dwc1y_SIG_ref.Copy(g_d_dwc1y_SIG);
  if(g_d_dwc2x_SIG.isUsed())g_d_dwc2x_SIG_ref.Copy(g_d_dwc2x_SIG);
  if(g_d_dwc2y_SIG.isUsed())g_d_dwc2y_SIG_ref.Copy(g_d_dwc2y_SIG);
  if(g_d_dwc3x_SIG.isUsed())g_d_dwc3x_SIG_ref.Copy(g_d_dwc3x_SIG);
  if(g_d_dwc3y_SIG.isUsed())g_d_dwc3y_SIG_ref.Copy(g_d_dwc3y_SIG);
  if(g_d_dwc4x_SIG.isUsed())g_d_dwc4x_SIG_ref.Copy(g_d_dwc4x_SIG);
  if(g_d_dwc4y_SIG.isUsed())g_d_dwc4y_SIG_ref.Copy(g_d_dwc4y_SIG);
  //
  for(int i=0; i<2*N742CHAN; ++i){
    if(g_d_DIG_LEDPED[i].isUsed())g_d_DIG_LEDPED_ref[i].Copy(g_d_DIG_LEDPED[i]);
    if(g_d_DIG_LEDAMP[i].isUsed())g_d_DIG_LEDAMP_ref[i].Copy(g_d_DIG_LEDAMP[i]);
    if(g_d_DIG_LEDWAV[i].isUsed())g_d_DIG_LEDWAV_ref[i].Copy(g_d_DIG_LEDWAV[i]);
    if(g_d_DIG_PEDPED[i].isUsed())g_d_DIG_PEDPED_ref[i].Copy(g_d_DIG_PEDPED[i]);
    if(g_d_DIG_PEDAMP[i].isUsed())g_d_DIG_PEDAMP_ref[i].Copy(g_d_DIG_PEDAMP[i]);
    if(g_d_DIG_PEDWAV[i].isUsed())g_d_DIG_PEDWAV_ref[i].Copy(g_d_DIG_PEDWAV[i]);
    if(g_d_DIG_SIGPED[i].isUsed())g_d_DIG_SIGPED_ref[i].Copy(g_d_DIG_SIGPED[i]);
    if(g_d_DIG_SIGAMP[i].isUsed())g_d_DIG_SIGAMP_ref[i].Copy(g_d_DIG_SIGAMP[i]);
    if(g_d_DIG_SIGWAV[i].isUsed())g_d_DIG_SIGWAV_ref[i].Copy(g_d_DIG_SIGWAV[i]);
  }
}

void MyMainFrame::DoAutoDraw(bool on){
  g_auto_draw=on;
}

void MyMainFrame::DoSubRef(bool on){
  g_subtract_ref=on;
}

void MyMainFrame::DoEnableStartRun(){
  bool currstat=tbStartRun->IsEnabled();
  if(currstat)  {
    tbStartRun->SetEnabled(kFALSE);
    tbEnable->SetText("Enable");
    g_t_startbutton_enabled=0;
  }
  else {
    tbStartRun->SetEnabled(kTRUE);
    tbEnable->SetText("Disable");
    TTimeStamp tst;
    g_t_startbutton_enabled=tst.AsDouble();
  }
}

void MyMainFrame::DoStartRun(){
  if(!g_startrun && !g_stoprun){
    const char* buttontext=tbStartRun->GetText()->GetString();
    char BUTTONTEXT[256]; memset(BUTTONTEXT,0,sizeof(BUTTONTEXT));
    for(int i=0; i<(int)sizeof(BUTTONTEXT) && (BUTTONTEXT[i]=toupper(buttontext[i])); ++i){;};
    
    if(0!=strstr(BUTTONTEXT,"START"))g_startrun=true;
    else if(0!=strstr(BUTTONTEXT,"STOP"))g_stoprun=true;
  }
}

void MyMainFrame::DoAddComment(){
  g_addcomment=true;
}

void MyMainFrame::DoSetPattType(Int_t wId, Int_t id){
  TGLBEntry* ePatt=cbPatt->GetSelectedEntry();
  if (ePatt->InheritsFrom(TGTextLBEntry::Class())) {
    const char* cPatt=((TGTextLBEntry*)ePatt)->GetText()->GetString();
    strcpy(g_patt_selected,cPatt);
    printf("%s: %s ",__func__,cPatt);
  }
  
  TGLBEntry* eType=cbType->GetSelectedEntry();
  if (eType->InheritsFrom(TGTextLBEntry::Class())) {
    const char* cType=((TGTextLBEntry*)eType)->GetText()->GetString();
    strcpy(g_type_selected,cType);
    printf("%s \n",cType);
  }
  
  g_update_hists=true;
  g_draw_hist=false;
  g_auto_draw=false;
  tbAutoDraw->SetState(kButtonUp,kFALSE);
}

void MyMainFrame::DoSetHist(Int_t wId, Int_t id){
  int nent=cbHist->GetNumberOfEntries();
  if(nent<=0){
    strcpy(g_name_selected,"");
    return;
  }
  
  TGLBEntry* eHist=cbHist->GetSelectedEntry();
  if(!eHist)return;
  
  if (eHist->InheritsFrom(TGTextLBEntry::Class())) {
    strncpy(g_name_selected,((TGTextLBEntry*)eHist)->GetText()->GetString(),255);; g_name_selected[255]=0;
    printf("%s(%d,%d): %s selected\n",__func__,wId,id,g_name_selected);
  }
  
  g_draw_hist=true;
  g_auto_draw=false;
  tbAutoDraw->SetState(kButtonUp,kFALSE);
}

void MyMainFrame::DoDrawHist(){
  g_draw_hist=true;
}

void MyMainFrame::drawHist()
{
  //printf("%s called, |%s| selected\n",__func__,g_name_selected);
  if(0==strlen(g_name_selected))return;
  
  TCanvas *c1 = fEcan->GetCanvas();
  c1->cd();
  
  char type[8],patt[8],cell[32],suppl[32];
  int chan=-1;
  parse_service_name(g_name_selected, type, patt, cell, chan, suppl);
  if(0==strcmp(cell,"summ")){
    void* addr=address_dimsummary(type, patt, cell, chan, suppl);
    void* addr_ref=address_ref_dimsummary(type, patt, cell, chan, suppl);
    if(addr && addr_ref){
      DIMSUMMARY* dsaddr=(DIMSUMMARY*)addr;
      DIMSUMMARY* dsaddr_ref=(DIMSUMMARY*)addr_ref;
      DIMSUMMARY ds;
      ds.Copy(*dsaddr);
      if(g_subtract_ref)ds.Subtract(*dsaddr_ref);
      //dsaddr->fill_TProfile(p0);
      bool newhist=ds.fill_TH1D(h0);
      //if(0!=strcmp(g_name_selected,g_name_sel_prev)){
      //printf("%s: new summary hist %s has %d bins\n",__func__,h0->GetName(), h0->GetXaxis()->GetNbins());
      if(newhist){
        gPad->SetRightMargin(0.17);
        gPad->SetLeftMargin(0.12);
        gPad->SetBottomMargin(0.12);
        gStyle->SetPalette(1);
        //p0->Draw();
        h0->Draw();
        h0->GetXaxis()->SetLabelSize(0.06);
      }
      c1->Modified();
      c1->Update();
    }
  }
  else {
    void* addr=address_dimhist(type, patt, cell, chan, suppl);
    void* addr_ref=address_ref_dimhist(type, patt, cell, chan, suppl);
    if(addr){
      DIMHIST* dhaddr=(DIMHIST*)addr;
      DIMHIST* dhaddr_ref=(DIMHIST*)addr_ref;
      DIMHIST dh;
      dh.Copy(*dhaddr);
      if(g_subtract_ref){
        if(0!=strcmp("WAV",suppl)) dh.Subtract(*dhaddr_ref);
      }
      dh.fill_TH1D(h0);
      if(0!=strcmp(g_name_selected,g_name_sel_prev)){
        gPad->SetRightMargin(0.17);
        gPad->SetLeftMargin(0.12);
        gPad->SetBottomMargin(0.12);
        gStyle->SetPalette(1);
        h0->Draw();
      }
      c1->Modified();
      c1->Update();
    }
  }
  strncpy(g_name_sel_prev, g_name_selected, 254); g_name_sel_prev[255]=0;
}

void MyMainFrame::fillRunStatus(){
  char str[256];
  
  DIMSTAT ds;
  ds.Copy(g_d_status);
  if(g_subtract_ref)ds.Subtract(g_d_status_ref);
  
  DIMSTAT* d=&ds;
  sprintf(str," RUN %12d",d->irun);
  lbNrun->SetText(str);
  
  UInt_t year,month,day,hour,min,sec;
  int stm=d->starttime;
  TTimeStamp tst((time_t)stm,0);
  tst.GetDate(kFALSE,0,&year,&month,&day);
  tst.GetTime(kFALSE,0,&hour,&min,&sec);
  sprintf(str," STARTED:      %2.2d:%2.2d:%2.2d",hour,min,sec);
  lbTbeg->SetText(str);
  
  int dur=d->runtime;
  hour=dur/3600;
  min=(dur/60)%60;
  sec=dur%60;
  sprintf(str," DURATION:     %2.2d:%2.2d:%2.2d",hour,min,sec);
  lbTdur->SetText(str);
  
  if(d->running>0)strcpy(str," STATUS:       RUNNING");
  else strcpy(str," STATUS:       STOPPED");
  lbRunStatus->SetText(str);
  if(d->running>0)tbStartRun->SetText("STOP");
  else tbStartRun->SetText("START");
  
  sprintf(str,"LED %6d",d->nled);
  lbNLED->SetText(str);
  sprintf(str,"PED %6d",d->nped);
  lbNPED->SetText(str);
  sprintf(str,"SIG %6d",d->nsig);
  lbNSIG->SetText(str);
  sprintf(str,"TOT %6d",d->ievt);
  lbNTOT->SetText(str);

  sprintf(str,"CONFIG:     %s",d->conf);
  lbConf->SetText(str);
  
  sprintf(str,"OUTPUT FILE:     %s",d->fnam);
  lbFile->SetText(str);
  
  if(d->running>0 && !g_running_prev){
    printf("Start run: update hists\n");
    g_update_hists=true;
  }
  g_running_prev= (d->running>0);
}

void MyMainFrame::fillHistCombo(const char* cPatt, const char* cType){
  int nhists=0;
  std::map<std::string,int> mh;
  std::map<std::string,int> ms;
  
  if(g_d_summ_ADC_LED.isUsed()){ms.insert(std::pair<std::string,int>(g_d_summ_ADC_LED.name, nhists)); nhists++;}
  if(g_d_summ_ADC_PED.isUsed()){ms.insert(std::pair<std::string,int>(g_d_summ_ADC_PED.name, nhists)); nhists++;}
  if(g_d_summ_ADC_SIG.isUsed()){ms.insert(std::pair<std::string,int>(g_d_summ_ADC_SIG.name, nhists)); nhists++;}
  
  if(g_d_summ_TDC_LED.isUsed()){ms.insert(std::pair<std::string,int>(g_d_summ_TDC_LED.name, nhists)); nhists++;}
  if(g_d_summ_TDC_SIG.isUsed()){ms.insert(std::pair<std::string,int>(g_d_summ_TDC_SIG.name, nhists)); nhists++;}
  
  if(g_d_summ_DIG_LED.isUsed()){ms.insert(std::pair<std::string,int>(g_d_summ_DIG_LED.name, nhists)); nhists++;}
  if(g_d_summ_DIG_PED.isUsed()){ms.insert(std::pair<std::string,int>(g_d_summ_DIG_PED.name, nhists)); nhists++;}
  if(g_d_summ_DIG_SIG.isUsed()){ms.insert(std::pair<std::string,int>(g_d_summ_DIG_SIG.name, nhists)); nhists++;}
  
  for(int i=0; i<NADCCHAN; ++i){
    if(g_d_ADC_LED[i].isUsed()){mh.insert(std::pair<std::string,int>(g_d_ADC_LED[i].name, nhists)); nhists++;}
    if(g_d_ADC_PED[i].isUsed()){mh.insert(std::pair<std::string,int>(g_d_ADC_PED[i].name, nhists)); nhists++;}
    if(g_d_ADC_SIG[i].isUsed()){mh.insert(std::pair<std::string,int>(g_d_ADC_SIG[i].name, nhists)); nhists++;}
  }
  
  if(g_d_dwc1x_LED.isUsed()){mh.insert(std::pair<std::string,int>(g_d_dwc1x_LED.name, nhists)); nhists++;}
  if(g_d_dwc2x_LED.isUsed()){mh.insert(std::pair<std::string,int>(g_d_dwc2x_LED.name, nhists)); nhists++;}
  if(g_d_dwc3x_LED.isUsed()){mh.insert(std::pair<std::string,int>(g_d_dwc3x_LED.name, nhists)); nhists++;}
  if(g_d_dwc4x_LED.isUsed()){mh.insert(std::pair<std::string,int>(g_d_dwc4x_LED.name, nhists)); nhists++;}
  if(g_d_dwc1y_LED.isUsed()){mh.insert(std::pair<std::string,int>(g_d_dwc1y_LED.name, nhists)); nhists++;}
  if(g_d_dwc2y_LED.isUsed()){mh.insert(std::pair<std::string,int>(g_d_dwc2y_LED.name, nhists)); nhists++;}
  if(g_d_dwc3y_LED.isUsed()){mh.insert(std::pair<std::string,int>(g_d_dwc3y_LED.name, nhists)); nhists++;}
  if(g_d_dwc4y_LED.isUsed()){mh.insert(std::pair<std::string,int>(g_d_dwc4y_LED.name, nhists)); nhists++;}
  if(g_d_dwc1x_SIG.isUsed()){mh.insert(std::pair<std::string,int>(g_d_dwc1x_SIG.name, nhists)); nhists++;}
  if(g_d_dwc2x_SIG.isUsed()){mh.insert(std::pair<std::string,int>(g_d_dwc2x_SIG.name, nhists)); nhists++;}
  if(g_d_dwc3x_SIG.isUsed()){mh.insert(std::pair<std::string,int>(g_d_dwc3x_SIG.name, nhists)); nhists++;}
  if(g_d_dwc4x_SIG.isUsed()){mh.insert(std::pair<std::string,int>(g_d_dwc4x_SIG.name, nhists)); nhists++;}
  if(g_d_dwc1y_SIG.isUsed()){mh.insert(std::pair<std::string,int>(g_d_dwc1y_SIG.name, nhists)); nhists++;}
  if(g_d_dwc2y_SIG.isUsed()){mh.insert(std::pair<std::string,int>(g_d_dwc2y_SIG.name, nhists)); nhists++;}
  if(g_d_dwc3y_SIG.isUsed()){mh.insert(std::pair<std::string,int>(g_d_dwc3y_SIG.name, nhists)); nhists++;}
  if(g_d_dwc4y_SIG.isUsed()){mh.insert(std::pair<std::string,int>(g_d_dwc4y_SIG.name, nhists)); nhists++;}
  
  for(int i=0; i<2*N742CHAN; ++i){
    if(g_d_DIG_LEDPED[i].isUsed()){mh.insert(std::pair<std::string,int>(g_d_DIG_LEDPED[i].name,nhists));nhists++;}
    if(g_d_DIG_LEDAMP[i].isUsed()){mh.insert(std::pair<std::string,int>(g_d_DIG_LEDAMP[i].name,nhists));nhists++;}
    if(g_d_DIG_LEDWAV[i].isUsed()){mh.insert(std::pair<std::string,int>(g_d_DIG_LEDWAV[i].name,nhists));nhists++;}
    if(g_d_DIG_PEDPED[i].isUsed()){mh.insert(std::pair<std::string,int>(g_d_DIG_PEDPED[i].name,nhists));nhists++;}
    if(g_d_DIG_PEDAMP[i].isUsed()){mh.insert(std::pair<std::string,int>(g_d_DIG_PEDAMP[i].name,nhists));nhists++;}
    if(g_d_DIG_PEDWAV[i].isUsed()){mh.insert(std::pair<std::string,int>(g_d_DIG_PEDWAV[i].name,nhists));nhists++;}
    if(g_d_DIG_SIGPED[i].isUsed()){mh.insert(std::pair<std::string,int>(g_d_DIG_SIGPED[i].name,nhists));nhists++;}
    if(g_d_DIG_SIGAMP[i].isUsed()){mh.insert(std::pair<std::string,int>(g_d_DIG_SIGAMP[i].name,nhists));nhists++;}
    if(g_d_DIG_SIGWAV[i].isUsed()){mh.insert(std::pair<std::string,int>(g_d_DIG_SIGWAV[i].name,nhists));nhists++;}
  }
  
  cbHist->RemoveAll();
  char type[8],patt[8],cell[32],suppl[32];
  int chan=-1;
  int nent=0;
  int ient_selected=-1;
  if(ms.size()>0){
    for(std::map<std::string,int>::iterator it=ms.begin(); it!=ms.end(); ++it){
      parse_service_name((it->first).c_str(), type, patt, cell, chan, suppl);
      if(0==strcmp(cPatt,patt) && 0==strcmp(cType,type)){
        if(0==strcmp(g_name_selected, (it->first).c_str()))ient_selected=nent;
        cbHist->AddEntry((it->first).c_str(),nent);
        nent++;
      }
    }
  }
  if(mh.size()>0){
    for(std::map<std::string,int>::iterator it=mh.begin(); it!=mh.end(); ++it){
      parse_service_name((it->first).c_str(), type, patt, cell, chan, suppl);
      if(0==strcmp(cPatt,patt) && 0==strcmp(cType,type)){
        if(0==strcmp(g_name_selected, (it->first).c_str()))ient_selected=nent;
        cbHist->AddEntry((it->first).c_str(),nent);
        nent++;
      }
    }
  }
  
  if(nent>0){
    if(ient_selected>=0){
      cbHist->Select(ient_selected,kFALSE);
    }
    else{
      cbHist->Select(0,kFALSE);
      DoSetHist(0,0);
    }
  }
  
  int siz=ms.size()+mh.size();
  printf("%s: %d hists found\n",__func__,siz);
}

Bool_t MyMainFrame::HandleTimer(TTimer* timer){
  if(timer!=fTimer) return kTRUE;
  
  DimBrowser dbr; 
  char *server, *node, *service, *format; 
  int typ;
  
  //bool tb_daq_server_present=false;
  
  char type[8],patt[8],cell[32],suppl[32],    str[128];
  int chan=-1;
  
  int nhists=0;
  std::map<std::string,int> mh;
  std::map<std::string,int> ms;
  
  //TTimeStamp tst1;
  //double t1=tst1.AsDouble();
  
  dbr.getServers(); 
  while(dbr.getNextServer(server, node))     { 
    if(0==strcmp(server,"TB_DAQ")){
      //tb_daq_server_present=true;
      dbr.getServerServices(server); 
      while( (typ=dbr.getNextServerService(service, format)) ) { // DIMSTAT
        if(0==strcmp("TB_DAQ_STATUS",service)){
          dic_info_service (service, ONCE_ONLY, 60, &g_d_status, sizeof(g_d_status), 
                            0, 0, &g_d_status_dummy, sizeof(g_d_status_dummy));
        }
        else if(0!=strstr(service,"summ_")){ // DIMSUMMARY
          parse_service_name(service, type, patt, cell, chan, suppl);
          void* addr=address_dimsummary(type, patt, cell, chan, suppl);
          dic_info_service (service, ONCE_ONLY, 60, addr, sizeof(DIMSUMMARY), 
                            0, 0, &g_d_summ_dummy, sizeof(g_d_summ_dummy));
          if(0==strcmp(g_patt_selected,patt) && 0==strcmp(g_type_selected,type)){
            ms.insert(std::pair<std::string,int>(service,nhists));
            nhists++;
          }
        }
        else{ // DIMHIST
          parse_service_name(service, type, patt, cell, chan, suppl);
          void* addr=address_dimhist(type, patt, cell, chan, suppl);
          dic_info_service (service, ONCE_ONLY, 60, addr, sizeof(DIMHIST), 
                            0, 0, &g_d_hist_dummy, sizeof(g_d_hist_dummy));
          if(0==strcmp(g_patt_selected,patt) && 0==strcmp(g_type_selected,type)){
            //printf("service %s, format %s\n",service,format);
            mh.insert(std::pair<std::string,int>(service,nhists));
            nhists++;
          }
        }
      }
    }
  } 
  TTimeStamp tst2;
  double t2=tst2.AsDouble();
  if(g_t_startbutton_enabled>0 && t2-g_t_startbutton_enabled>10) DoEnableStartRun();
  //printf("took %f s\n",t2-t1);
  
  fillRunStatus();
  
  if(g_update_hists) {
    cbHist->RemoveAll();
    fillHistCombo(g_patt_selected,g_type_selected);
    g_update_hists=false;
  }
  
  if(g_draw_hist || g_auto_draw){
    drawHist();
    g_draw_hist=false;
  }
  
  if(g_startrun){
    g_startrun=false;
    
    tbStartRun->SetEnabled(kFALSE);
    tbEnable->SetText("Enable");
    g_t_startbutton_enabled=0;
    
    g_auto_draw=false;
    tbAutoDraw->SetState(kButtonUp,kFALSE);
    
    char question[256], answer[256], comment[10240];
    sprintf(question,"CONFIG NAME?");
    sprintf(answer,"%s",g_d_status.conf);
    memset(comment,0,sizeof(comment));
    getVal *gV = new getVal(this, question, answer, comment);
    gV->Popup();
    
    if(0==strcmp(question,"OK")){
      reset_dimhists();
      int ndimhist=list_dimhists();
      printf("%s: %d dimhists found after total reset\n",__func__,ndimhist);
      
      //printf("your comment has %lu symbols\n",strlen(comment));
      //if(strlen(comment)>1)printf("%s\n",comment);
      
      add_logbook("/home/yuri/viewtb_comments.txt",g_d_status.conf,g_d_status.irun+1,"STARTING",comment);
      
      sprintf(str,"STARTRUN %s",answer);
      sendcom(str);
    }
  }
  
  if(g_stoprun){
    g_stoprun=false;
    
    tbStartRun->SetEnabled(kFALSE);
    tbEnable->SetText("Enable");
    g_t_startbutton_enabled=0;
    
    g_auto_draw=false;
    tbAutoDraw->SetState(kButtonUp,kFALSE);
    
    char question[256], answer[256], comment[10240],state[256];
    sprintf(question,"ARE YOU SURE TO STOP THE RUN?");
    sprintf(answer,"%s",g_d_status.conf);
    memset(comment,0,sizeof(comment));
    getVal *gV = new getVal(this, question, answer, comment);
    gV->Popup();
    
    if(0==strcmp(question,"OK")){
      //printf("your comment has %lu symbols\n",strlen(comment));
      //if(strlen(comment)>1)printf("%s\n",comment);
      
      sprintf(state,"STOPPING, %d signal, %d LED, %d PED",
              g_d_status.nsig,g_d_status.nled,g_d_status.nped);
      add_logbook("/home/yuri/viewtb_comments.txt",g_d_status.conf,g_d_status.irun,state,comment);
      
      sendcom("STOPRUN");
      for(int i=0; i<255 && (answer[i]=toupper(answer[i])); ++i){}
      if( (0==strcmp("STOPSRV",answer)) || (0==strcmp("EXITSRV",answer)) ){
        sendcom("EXITSRV");
      }
    }
  }
  
  if(g_addcomment){
    g_addcomment=false;
    
    char answer[256], comment[10240];
    memset(answer,0,sizeof(answer));
    memset(comment,0,sizeof(comment));
    getComment *gC = new getComment(this, answer, comment);
    gC->Popup();
    
    if(0==strcmp(answer,"OK")){
      //printf("your comment has %lu symbols\n",strlen(comment));
      //if(strlen(comment)>1)printf("%s\n",comment);
      add_logbook("/home/yuri/viewtb_comments.txt",g_d_status.conf,g_d_status.irun,"COMMENT",comment);
    }
  }  
  return kTRUE;
}

void MyMainFrame::EventInfo(Int_t event, Int_t px, Int_t py, TObject *selected)
{
  //  Writes the event status in the status bar parts

  const char *text0, *text1, *text3;
  char text2[50];
  text0 = selected->GetTitle();
  fStatusBar->SetText(text0,0);
  text1 = selected->GetName();
  fStatusBar->SetText(text1,1);

  sprintf(text2, "%d,%d", px, py);

  //printf("Event=%d\n",event);

  if (event == kKeyPress){
    sprintf(text2, "%c", (char) px);
  }
  else if(event == kButton1Up){
    printf("%s: Button 1 Up\n",__func__);
  }
  else if(event == kButton1Shift){
    printf("%s: Shift Button 1\n",__func__);
  }
  else if(event == kButton1Double){
    printf("%s: DoubleClick Button 1\n",__func__);
  }
  
  fStatusBar->SetText(text2,2);
  text3 = selected->GetObjectInfo(px,py);
  fStatusBar->SetText(text3,3);
}

MyMainFrame::MyMainFrame(const TGWindow *p, UInt_t w, UInt_t h)
  : TGMainFrame(p, w, h)
{
  h0=0; // histogram is not created for the moment
  
  // Create a horizontal frame containing the embedded canvas and frame with buttons to the right
  TGHorizontalFrame *hframc = new TGHorizontalFrame(this, 660, 460);

  // Create the embedded canvas
  fEcan = new TRootEmbeddedCanvas(0,hframc,600,450);
  Int_t wid = fEcan->GetCanvasWindowId();
  TCanvas *myc = new TCanvas("MyCanvas", 10,10,wid);
  myc->SetBorderMode(-1);
  fEcan->AdoptCanvas(myc);
  myc->Connect("ProcessedEvent(Int_t,Int_t,Int_t,TObject*)","MyMainFrame",this, 
               "EventInfo(Int_t,Int_t,Int_t,TObject*)");
  hframc->AddFrame(fEcan, new TGLayoutHints(kLHintsLeft | kLHintsExpandX | kLHintsExpandY ,0,0,1,1));
  
  // Create a vertical frame containing start/stop buttons and a run statistics
  TGVerticalFrame *hframv = new TGVerticalFrame(hframc, 50, 450);
  
  // fonts
  //TGFont *font18b = gClient->GetFont("-*-helvetica-bold-r-*-*-18-*-*-*-*-*-*-*");
  //TGFont *font16b = gClient->GetFont("-*-helvetica-bold-r-*-*-16-*-*-*-*-*-*-*");

  TGLabel* lbDummy=new TGLabel(hframv,"RUN CONTROL");
  //lbDummy->SetTextFont(font18b);
  hframv->AddFrame(lbDummy, new TGLayoutHints(kLHintsTop | kLHintsCenterX, 2, 2, 2, 20));
  
  tbEnable = new TGTextButton(hframv, "Enable", 11);
  //tbEnable->SetFont(font16b->GetFontStruct());
  tbEnable->Connect("Released()", "MyMainFrame", this, "DoEnableStartRun()");
  hframv->AddFrame(tbEnable, new TGLayoutHints(kLHintsTop|kLHintsCenterX, 2, 2, 2, 2));
  
  tbStartRun = new TGTextButton(hframv, "START", 12);
  //tbStartRun->SetFont(font16b->GetFontStruct());
  tbStartRun->Connect("Released()", "MyMainFrame", this, "DoStartRun()");
  hframv->AddFrame(tbStartRun, new TGLayoutHints(kLHintsTop|kLHintsCenterX, 2, 2, 2, 2));
  tbStartRun->SetEnabled(false);
  
  tbAddComment = new TGTextButton(hframv, "Add comment", 12);
  //tbStartRun->SetFont(font16b->GetFontStruct());
  tbAddComment->Connect("Released()", "MyMainFrame", this, "DoAddComment()");
  hframv->AddFrame(tbAddComment, new TGLayoutHints(kLHintsTop|kLHintsCenterX, 2, 2, 20, 2));
  tbAddComment->SetEnabled(true);
  
  lbNrun=new TGLabel(hframv," RUN  ----------------");
  //lbNrun->SetTextFont(font16b);
  lbNrun->SetTextJustify(kTextLeft|kTextTop);
  hframv->AddFrame(lbNrun, new TGLayoutHints(kLHintsTop | kLHintsLeft, 2, 2, 30, 2));
  
  lbTbeg=new TGLabel(hframv," STARTED:      ---:---:-------");
  //lbTbeg->SetTextFont(font16b);
  lbTbeg->SetTextJustify(kTextLeft|kTextTop);
  hframv->AddFrame(lbTbeg, new TGLayoutHints(kLHintsTop | kLHintsLeft, 2, 2, 2, 2));
  
  lbTdur=new TGLabel(hframv," DURATION:     ---:---:-------");
  //lbTdur->SetTextFont(font16b);
  lbTdur->SetTextJustify(kTextLeft|kTextTop);
  hframv->AddFrame(lbTdur, new TGLayoutHints(kLHintsTop | kLHintsLeft, 2, 2, 2, 2));
  
  lbRunStatus=new TGLabel(hframv," STATUS:       UNKNOWN");
  //lbRunStatus->SetTextFont(font16b);
  lbRunStatus->SetTextJustify(kTextLeft|kTextTop);
  hframv->AddFrame(lbRunStatus, new TGLayoutHints(kLHintsTop | kLHintsLeft, 2, 2, 2, 2));
  
  TGLabel* lbEvCount=new TGLabel(hframv,"----- Event count ------");
  lbEvCount->SetTextJustify(kTextLeft);
  hframv->AddFrame(lbEvCount, new TGLayoutHints(kLHintsTop | kLHintsLeft, 2, 2, 20, 2));
  
  lbNLED=new TGLabel(hframv," LED  ---------------");
  lbNLED->SetTextJustify(kTextLeft);
  hframv->AddFrame(lbNLED, new TGLayoutHints(kLHintsTop | kLHintsLeft, 2, 2, 2, 2));
  
  lbNPED=new TGLabel(hframv," PED  ---------------");
  lbNPED->SetTextJustify(kTextLeft);
  hframv->AddFrame(lbNPED, new TGLayoutHints(kLHintsTop | kLHintsLeft, 2, 2, 2, 2));
  
  lbNSIG=new TGLabel(hframv," SIG  ---------------");
  lbNSIG->SetTextJustify(kTextLeft);
  hframv->AddFrame(lbNSIG, new TGLayoutHints(kLHintsTop | kLHintsLeft, 2, 2, 2, 2));
  
  lbNTOT=new TGLabel(hframv," TOT  ---------------");
  lbNTOT->SetTextJustify(kTextLeft);
  hframv->AddFrame(lbNTOT, new TGLayoutHints(kLHintsTop | kLHintsLeft, 2, 2, 2, 2));
  
  lbConf=new TGLabel(hframv," CONFIG: ---------------------");
  lbConf->SetTextJustify(kTextLeft);
  hframv->AddFrame(lbConf, new TGLayoutHints(kLHintsTop | kLHintsLeft, 2, 2, 20, 2));
  
  hframc->AddFrame(hframv, new TGLayoutHints(kLHintsLeft, 0,0,1,1));
  
  AddFrame(hframc, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX | kLHintsExpandY,0,0,1,1));
  
  // status bar
  Int_t parts[] = {30, 10, 10, 50};
  fStatusBar = new TGStatusBar(this, 50, 10, kVerticalFrame);
  fStatusBar->SetParts(parts, 4);
  fStatusBar->Draw3DCorner(kFALSE);
  AddFrame(fStatusBar, new TGLayoutHints(kLHintsExpandX, 0, 0, 10, 0));
  
  // Create a horizontal frame containing pattern/type/histo select comboboxes
  TGHorizontalFrame *hframk = new TGHorizontalFrame(this, 600, 40);
  
  cbPatt=new TGComboBox(hframk, 31);
  hframk->AddFrame(cbPatt, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));
  cbPatt->AddEntry("PED",0);
  cbPatt->AddEntry("LED",1);
  cbPatt->AddEntry("SIG",2);
  cbPatt->Resize(50,20);
  cbPatt->Select(1,kFALSE);
  cbPatt->Connect("Selected(Int_t, Int_t)","MyMainFrame",this,"DoSetPattType(Int_t, Int_t)");
  
  cbType=new TGComboBox(hframk, 32);
  hframk->AddFrame(cbType, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));
  cbType->AddEntry("ADC",0);
  cbType->AddEntry("TDC",1);
  cbType->AddEntry("DIG",2);
  cbType->Resize(50,20);
  cbType->Select(0,kFALSE);
  cbType->Connect("Selected(Int_t, Int_t)","MyMainFrame",this,"DoSetPattType(Int_t, Int_t)");
  
  cbHist=new TGComboBox(hframk, 33);
  hframk->AddFrame(cbHist, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));
  cbHist->Connect("Selected(Int_t, Int_t)","MyMainFrame",this,"DoSetHist(Int_t, Int_t)");
  cbHist->AddEntry("-- no histos --",0);
  cbHist->Resize(240,20);
  cbHist->Select(0,kFALSE);
  
  tb2Ref = new TGTextButton(hframk, "->Ref", 35);
  tb2Ref->Connect("Released()", "MyMainFrame", this, "Do2Ref()");
  hframk->AddFrame(tb2Ref, new TGLayoutHints(kLHintsLeft, 2, 2, 2, 2));

  tbSubRef = new TGCheckButton(hframk, "-Ref",36);
  tbSubRef->Connect("Toggled(Bool_t)", "MyMainFrame", this, "DoSubRef(Bool_t)");
  hframk->AddFrame(tbSubRef, new TGLayoutHints(kLHintsLeft, 2, 2, 2, 2));

  tbAutoDraw = new TGCheckButton(hframk, "Auto",37);
  tbAutoDraw->Connect("Toggled(Bool_t)", "MyMainFrame", this, "DoAutoDraw(Bool_t)");
  hframk->AddFrame(tbAutoDraw, new TGLayoutHints(kLHintsRight, 100, 2, 2, 2));

  AddFrame(hframk, new TGLayoutHints(kLHintsLeft, 2, 2, 2, 2));
  
  TGHorizontalFrame *hframl = new TGHorizontalFrame(this, 200, 40);
  
  lbFile=new TGLabel(hframl," -- no output file --");
  lbFile->SetTextJustify(kTextLeft);
  hframl->AddFrame(lbFile, new TGLayoutHints(kLHintsExpandX | kLHintsLeft, 2, 2, 2, 2));
  
  AddFrame(hframl, new TGLayoutHints(kLHintsExpandX | kLHintsLeft, 2, 2, 2, 2));
  
  fTimer = new TTimer(1000);
  fTimer->SetObject(this);
  
  // Set a name to the main frame   
  SetWindowName("LHCb CALO beam test");
  MapSubwindows();
  
  // Initialize the layout algorithm via Resize()
  Resize(GetDefaultSize());
  
  // Map main frame
  MapWindow();
}

void MyMainFrame::Init(){
  gStyle->SetOptStat(1112210);
  
  int res=1;
  //res=dim_set_dns_node("pclbhcpmt02");
  //res=dim_set_dns_node("pclbcscalib01");
  if(res!=1){
    printf("Cannot connect to dns node!!!\n");
  }
  
  DoSetPattType(0,0);
  
  fTimer->TurnOn();
}

void MyMainFrame::CloseWindow()
{
  // Close Window.
  
  printf("Closing window\n");
  if (fTimer)           fTimer->TurnOff();
  
  DestroyWindow();
  
  printf("Exit application...\n");
  gApplication->Terminate(0);
}

MyMainFrame::~MyMainFrame()
{
  // Clean up main frame...
  Cleanup();
  //delete fEcan;
  //delete fTimer;
}

