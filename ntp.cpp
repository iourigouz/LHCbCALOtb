//
//
#include <time.h>

#include "TFile.h"
#include "TTree.h"
#include "TH1D.h"
#include "TH1I.h"
#include "TTimeStamp.h"

#include "ntp.h"
#include "dimHist.h"

#include <dim/dis.hxx>

#include <dim/dim_common.h>
#include <dim/dic.hxx>

#include <errno.h>

int g_dimsrv_initialized=false;

int g_ievt=0;
int g_nped=0;
int g_nled=0;
int g_nsig=0;

bool g_startrun=false;
bool g_stoprun=false;
bool g_running=false;

char g_config[128]="";
char g_rootfilename[256]="";

bool g_exit=false;

int g_runnumber;

int g_print=0;

TFile* g_ROOTfile=(TFile*)0;
TTree* g_ROOTtree=(TTree*)0;

// data
int g_nTDCclear=0;

double g_t_tot, g_t0_tot;
double g_t, g_t0, g_tprev;
int g_pattern; // 1=PED, 2=LED, 4=SIG
int g_ADC[NADCCHAN];

int g_nTDC[NTDCCHAN];
int g_tTDC[NTDCCHAN][NTDCMAXHITS];
int g_tTDCtrig;

double g_x1, g_x2, g_x3, g_x4, g_y1, g_y2, g_y3, g_y4;

int g_nDT5742[NDT5742CHAN];
float* g_evdata742[NDT5742CHAN]; // intermediate destination for data pointers
int g_used742[NDT5742CHAN];          // flag =1 for used channels, 0 otherwise
float g_aDT5742[NDT5742CHAN][NDT5742SAMPL];

RUNPARAM g_rp;
// end data

//
char memdir[256]="";
char filedir[256]="";

// online histos
TH1D* g_hPMTHV=0;
TH1D* g_hULED=0;
TH1D* g_ht0=0;

TH1I *g_hpattern=0;
//
TProfile *g_hsumm_ADC_LED=0, *g_hsumm_ADC_PED=0, *g_hsumm_ADC_SIG=0;
TProfile *g_hsumm_DIG_LED=0, *g_hsumm_DIG_PED=0, *g_hsumm_DIG_SIG=0;
TProfile *g_hsumm_TDC_LED=0, *g_hsumm_TDC_SIG=0;
//
TH1I *g_hADC_LED[NADCCHAN]={0};
TH1I *g_hADC_PED[NADCCHAN]={0};
TH1I *g_hADC_SIG[NADCCHAN]={0};
TH1I *g_hdwc1x_LED=0, *g_hdwc1y_LED=0, *g_hdwc2x_LED=0, *g_hdwc2y_LED=0; 
TH1I *g_hdwc3x_LED=0, *g_hdwc3y_LED=0, *g_hdwc4x_LED=0, *g_hdwc4y_LED=0;
TH1I *g_hdwc1x_SIG=0, *g_hdwc1y_SIG=0, *g_hdwc2x_SIG=0, *g_hdwc2y_SIG=0; 
TH1I *g_hdwc3x_SIG=0, *g_hdwc3y_SIG=0, *g_hdwc4x_SIG=0, *g_hdwc4y_SIG=0;
TH1I *g_hDIG_LEDPED[NDT5742CHAN]={0};
TH1I *g_hDIG_LEDMAX[NDT5742CHAN]={0};
TH1I *g_hDIG_PEDPED[NDT5742CHAN]={0};
TH1I *g_hDIG_PEDMAX[NDT5742CHAN]={0};
TH1I *g_hDIG_SIGPED[NDT5742CHAN]={0};
TH1I *g_hDIG_SIGMAX[NDT5742CHAN]={0};

DIMSTAT g_d_status;
DIMHIST *g_d_pattern=0;
//
DIMSUMMARY *g_d_summ_ADC_LED=0, *g_d_summ_ADC_PED=0, *g_d_summ_ADC_SIG=0;
DIMSUMMARY *g_d_summ_DIG_LED=0, *g_d_summ_DIG_PED=0, *g_d_summ_DIG_SIG=0;
DIMSUMMARY *g_d_summ_TDC_LED=0, *g_d_summ_TDC_SIG=0;
//
DIMHIST* g_d_ADC_LED[NADCCHAN]={0};
DIMHIST* g_d_ADC_PED[NADCCHAN]={0};
DIMHIST* g_d_ADC_SIG[NADCCHAN]={0};
DIMHIST *g_d_dwc1x_LED=0, *g_d_dwc1y_LED=0, *g_d_dwc2x_LED=0, *g_d_dwc2y_LED=0; 
DIMHIST *g_d_dwc3x_LED=0, *g_d_dwc3y_LED=0, *g_d_dwc4x_LED=0, *g_d_dwc4y_LED=0;
DIMHIST *g_d_dwc1x_SIG=0, *g_d_dwc1y_SIG=0, *g_d_dwc2x_SIG=0, *g_d_dwc2y_SIG=0; 
DIMHIST *g_d_dwc3x_SIG=0, *g_d_dwc3y_SIG=0, *g_d_dwc4x_SIG=0, *g_d_dwc4y_SIG=0;
DIMHIST* g_d_DIG_LEDPED[NDT5742CHAN]={0};
DIMHIST* g_d_DIG_LEDMAX[NDT5742CHAN]={0};
DIMHIST* g_d_DIG_PEDPED[NDT5742CHAN]={0};
DIMHIST* g_d_DIG_PEDMAX[NDT5742CHAN]={0};
DIMHIST* g_d_DIG_SIGPED[NDT5742CHAN]={0};
DIMHIST* g_d_DIG_SIGMAX[NDT5742CHAN]={0};

DimService* g_s_status=0;
//
DimService* g_s_pattern=0;
//
DimService *g_s_summ_ADC_LED=0, *g_s_summ_ADC_PED=0, *g_s_summ_ADC_SIG=0;
DimService *g_s_summ_DIG_LED=0, *g_s_summ_DIG_PED=0, *g_s_summ_DIG_SIG=0;
DimService *g_s_summ_TDC_LED=0, *g_s_summ_TDC_SIG=0;
//
DimService* g_s_ADC_LED[NADCCHAN]={0};
DimService* g_s_ADC_PED[NADCCHAN]={0};
DimService* g_s_ADC_SIG[NADCCHAN]={0};
DimService *g_s_dwc1x_LED=0, *g_s_dwc1y_LED=0, *g_s_dwc2x_LED=0, *g_s_dwc2y_LED=0; 
DimService *g_s_dwc3x_LED=0, *g_s_dwc3y_LED=0, *g_s_dwc4x_LED=0, *g_s_dwc4y_LED=0;
DimService *g_s_dwc1x_SIG=0, *g_s_dwc1y_SIG=0, *g_s_dwc2x_SIG=0, *g_s_dwc2y_SIG=0; 
DimService *g_s_dwc3x_SIG=0, *g_s_dwc3y_SIG=0, *g_s_dwc4x_SIG=0, *g_s_dwc4y_SIG=0;
DimService* g_s_DIG_LEDPED[NDT5742CHAN]={0};
DimService* g_s_DIG_LEDMAX[NDT5742CHAN]={0};
DimService* g_s_DIG_PEDPED[NDT5742CHAN]={0};
DimService* g_s_DIG_PEDMAX[NDT5742CHAN]={0};
DimService* g_s_DIG_SIGPED[NDT5742CHAN]={0};
DimService* g_s_DIG_SIGMAX[NDT5742CHAN]={0};

DimCommand* g_cmnd;
//
// end online histos
// DIM stuff
char g_dimdns[128]="pclbcscalib01";
char g_servernam[128]="TB_DAQ";
char g_cmdNam[128]="TB_DAQ_CMD";
char g_statusNam[128]="TB_DAQ_STATUS";

static int g_dwc1left =-1;
static int g_dwc1right=-1;
static int g_dwc1up   =-1;
static int g_dwc1down =-1;
static int g_dwc2left =-1;
static int g_dwc2right=-1;
static int g_dwc2up   =-1;
static int g_dwc2down =-1;
static int g_dwc3left =-1;
static int g_dwc3right=-1;
static int g_dwc3up   =-1;
static int g_dwc3down =-1;
static int g_dwc4left =-1;
static int g_dwc4right=-1;
static int g_dwc4up   =-1;
static int g_dwc4down =-1;

void openROOTfile(const char* filenam, const RUNPARAM* rp){
  delete_histos();
  
  char nam[128],tit[128],fmt[128], nam1[128];
  if(!g_ROOTfile && !g_ROOTtree){
    strncpy(memdir, gDirectory->GetPath(), 240);
    g_ROOTfile=new TFile(filenam,"recreate");
    g_ROOTfile->cd();
    strncpy(filedir, gDirectory->GetPath(), 240);
    
    TTimeStamp ttst;
    g_t0=ttst.GetSec();
    g_ht0=new TH1D("t0","t0",3,-1.5,1.5);
    g_ht0->Fill(0.0,g_t0);
    
    g_tprev=g_t0;
    
    g_hPMTHV=new TH1D("PMTHV","PMTHV",216,0,216);
    for(int i=0; i<g_rp.nchans; ++i){
      int ihvchan=g_rp.HVchan[i];
      if(ihvchan>=0){
        g_hPMTHV->GetXaxis()->SetBinLabel(ihvchan+1,&g_rp.chnam[i][0]);
        g_hPMTHV->Fill(ihvchan+0.5,g_rp.HV[i]);
      }
    }
    
    g_hULED=new TH1D("ULED","ULED",216,0,216);
    for(int i=0; i<MAXLEDS; ++i){
      int iledchan=g_rp.LEDchan[i];
      if(iledchan>=0){
        char nm[32]; sprintf(nm,"LED%d",i);
        g_hULED->GetXaxis()->SetBinLabel(iledchan+1,nm);
        g_hULED->Fill(iledchan+0.5,g_rp.ULED[i]);
      }
    }
    
    int nh1i=0;
    g_hpattern=new TH1I("hpattern","pattern",16,0,16); 
    nh1i++;
    
    int nADCconns=0, nTDCconns=0, nDIGconns=0;
    for(int ich=0; ich<g_rp.nchans; ++ich){
      if(1==g_rp.datatype[ich])nADCconns++;
      else if(3==g_rp.datatype[ich])nDIGconns++;
    }
    
    g_dwc1left =g_rp.findch("dwc1left ");
    g_dwc1right=g_rp.findch("dwc1right");
    if(g_dwc1left>=0 && g_dwc1right>=0)nTDCconns++;
    g_dwc1up   =g_rp.findch("dwc1up   ");
    g_dwc1down =g_rp.findch("dwc1down ");
    if(g_dwc1up>=0 && g_dwc1down>=0)nTDCconns++;
    g_dwc2left =g_rp.findch("dwc2left ");
    g_dwc2right=g_rp.findch("dwc2right");
    if(g_dwc2left>=0 && g_dwc2right>=0)nTDCconns++;
    g_dwc2up   =g_rp.findch("dwc2up   ");
    g_dwc2down =g_rp.findch("dwc2down ");
    if(g_dwc2up>=0 && g_dwc2down>=0)nTDCconns++;
    g_dwc3left =g_rp.findch("dwc3left ");
    g_dwc3right=g_rp.findch("dwc3right");
    if(g_dwc3left>=0 && g_dwc3right>=0)nTDCconns++;
    g_dwc3up   =g_rp.findch("dwc3up   ");
    g_dwc3down =g_rp.findch("dwc3down ");
    if(g_dwc3up>=0 && g_dwc3down>=0)nTDCconns++;
    g_dwc4left =g_rp.findch("dwc4left ");
    g_dwc4right=g_rp.findch("dwc4right");
    if(g_dwc4left>=0 && g_dwc4right>=0)nTDCconns++;
    g_dwc4up   =g_rp.findch("dwc4up   ");
    g_dwc4down =g_rp.findch("dwc4down ");
    if(g_dwc4up>=0 && g_dwc4down>=0)nTDCconns++;
    
    gDirectory->mkdir("LED");
    gDirectory->cd("LED");
    //gDirectory->pwd();
    if(g_rp.ADC_used)g_hsumm_ADC_LED=new TProfile("summ_LED_ADC", "summary ADC LED",nADCconns, 0,nADCconns, 0,4096,"S");
    if(g_rp.TDC_used)g_hsumm_TDC_LED=new TProfile("summ_LED_TDC", "summary TDC LED",nTDCconns, 0,nTDCconns, -2e4,2e4,"S");
    if(g_rp.digitizer_used)g_hsumm_DIG_LED=new TProfile("summ_LED_DIG", "summary Digitizer LED",nDIGconns, 0,nDIGconns, -4096,4096,"S");
    int iADC=0, iTDC=0, iDIG=0;
    for(int ich=0; ich<g_rp.nchans; ++ich){
      if(1==g_rp.datatype[ich]){// ADC
        sprintf(nam,"%s_LED_ADC_%2.2d",&g_rp.chnam[ich][0],g_rp.datachan[ich]);
        sprintf(tit,"%s (ADC%2.2d) LED",&g_rp.chnam[ich][0],g_rp.datachan[ich]);
        g_hADC_LED[g_rp.datachan[ich]]=new TH1I(nam,tit,4096,0,4096); 
        nh1i++;
        g_hsumm_ADC_LED->GetXaxis()->SetBinLabel(iADC+1,&g_rp.chnam[ich][0]);
        iADC++;
      }
      else if(3==g_rp.datatype[ich]){// DIG
        sprintf(nam,"%s_LED_DIG_%2.2d_PED",&g_rp.chnam[ich][0],g_rp.datachan[ich]);
        sprintf(tit,"%s (DIG%2.2d) LED PED",&g_rp.chnam[ich][0],g_rp.datachan[ich]);
        g_hDIG_LEDPED[g_rp.datachan[ich]]=new TH1I(nam,tit,4096,0,4096);
        nh1i++;
        sprintf(nam,"%s_LED_DIG_%2.2d_MAX",&g_rp.chnam[ich][0],g_rp.datachan[ich]);
        sprintf(tit,"%s (DIG%2.2d) LED MAX",&g_rp.chnam[ich][0],g_rp.datachan[ich]);
        g_hDIG_LEDMAX[g_rp.datachan[ich]]=new TH1I(nam,tit,4096,0,4096);
        nh1i++;
        g_hsumm_DIG_LED->GetXaxis()->SetBinLabel(iDIG+1,&g_rp.chnam[ich][0]);
        iDIG++;
      }
    }
    iTDC=0;
    if(g_dwc1left>=0 && g_dwc1right>=0){
      g_hdwc1x_LED=new TH1I("dwc1x_LED_TDC","dwc1x LED",300,-75,75); 
      nh1i++;
      g_hsumm_TDC_LED->GetXaxis()->SetBinLabel(iTDC+1,"dwc1x");
      iTDC++;
    }
    if(g_dwc1up>=0   && g_dwc1down>=0 ){
      g_hdwc1y_LED=new TH1I("dwc1y_LED_TDC","dwc1y LED",300,-75,75); 
      nh1i++;
      g_hsumm_TDC_LED->GetXaxis()->SetBinLabel(iTDC+1,"dwc1y");
      iTDC++;
    }
    if(g_dwc2left>=0 && g_dwc2right>=0){
      g_hdwc2x_LED=new TH1I("dwc2x_LED_TDC","dwc2x LED",300,-75,75); 
      nh1i++;
      g_hsumm_TDC_LED->GetXaxis()->SetBinLabel(iTDC+1,"dwc2x");
      iTDC++;
    }
    if(g_dwc2up>=0   && g_dwc2down>=0 ){
      g_hdwc2y_LED=new TH1I("dwc2y_LED_TDC","dwc2y LED",300,-75,75); 
      nh1i++;
      g_hsumm_TDC_LED->GetXaxis()->SetBinLabel(iTDC+1,"dwc2y");
      iTDC++;
    }
    if(g_dwc3left>=0 && g_dwc3right>=0){
      g_hdwc3x_LED=new TH1I("dwc3x_LED_TDC","dwc3x LED",300,-75,75); 
      nh1i++;
      g_hsumm_TDC_LED->GetXaxis()->SetBinLabel(iTDC+1,"dwc3x");
      iTDC++;
    }
    if(g_dwc3up>=0   && g_dwc3down>=0 ){
      g_hdwc3y_LED=new TH1I("dwc3y_LED_TDC","dwc3y LED",300,-75,75); 
      nh1i++;
      g_hsumm_TDC_LED->GetXaxis()->SetBinLabel(iTDC+1,"dwc3y");
      iTDC++;
    }
    if(g_dwc4left>=0 && g_dwc4right>=0){
      g_hdwc4x_LED=new TH1I("dwc4x_LED_TDC","dwc4x LED",300,-75,75); 
      nh1i++;
      g_hsumm_TDC_LED->GetXaxis()->SetBinLabel(iTDC+1,"dwc4x");
      iTDC++;
    }
    if(g_dwc4up>=0   && g_dwc4down>=0 ){
      g_hdwc4y_LED=new TH1I("dwc4y_LED_TDC","dwc4y LED",300,-75,75); 
      nh1i++;
      g_hsumm_TDC_LED->GetXaxis()->SetBinLabel(iTDC+1,"dwc1Y");
      iTDC++;
    }
    gDirectory->cd("..");
    //gDirectory->pwd();
    
    
    gDirectory->mkdir("PED");
    gDirectory->cd("PED");
    //gDirectory->pwd();
    if(g_rp.ADC_used)g_hsumm_ADC_PED=new TProfile("summ_PED_ADC", "summary ADC PED",nADCconns, 0,nADCconns, 0,4096,"S");
    if(g_rp.digitizer_used)g_hsumm_DIG_PED=new TProfile("summ_PED_DIG", "summary Digitizer PED",nDIGconns, 0,nDIGconns, -4096,4096,"S");
    iADC=iDIG=0;
    for(int ich=0; ich<g_rp.nchans; ++ich){
      if(1==g_rp.datatype[ich]){// ADC
        sprintf(nam,"%s_PED_ADC_%2.2d",&g_rp.chnam[ich][0],g_rp.datachan[ich]);
        sprintf(tit,"%s (ADC%2.2d) PED",&g_rp.chnam[ich][0],g_rp.datachan[ich]);
        g_hADC_PED[g_rp.datachan[ich]]=new TH1I(nam,tit,4096,0,4096);
        nh1i++;
        g_hsumm_ADC_PED->GetXaxis()->SetBinLabel(iADC+1,&g_rp.chnam[ich][0]);
        iADC++;
      }
      else if(3==g_rp.datatype[ich]){// DIG
        sprintf(nam,"%s_PED_DIG_%2.2d_PED",&g_rp.chnam[ich][0],g_rp.datachan[ich]);
        sprintf(tit,"%s (DIG%2.2d) PED PED",&g_rp.chnam[ich][0],g_rp.datachan[ich]);
        g_hDIG_PEDPED[g_rp.datachan[ich]]=new TH1I(nam,tit,4096,0,4096);
        nh1i++;
        sprintf(nam,"%s_PED_DIG_%2.2d_MAX",&g_rp.chnam[ich][0],g_rp.datachan[ich]);
        sprintf(tit,"%s (DIG%2.2d) PED MAX",&g_rp.chnam[ich][0],g_rp.datachan[ich]);
        g_hDIG_PEDMAX[g_rp.datachan[ich]]=new TH1I(nam,tit,4096,0,4096);
        nh1i++;
        g_hsumm_DIG_PED->GetXaxis()->SetBinLabel(iDIG+1,&g_rp.chnam[ich][0]);
        iDIG++;
      }
    }
    gDirectory->cd("..");
    //gDirectory->pwd();

    gDirectory->mkdir("SIG");
    gDirectory->cd("SIG");
    //gDirectory->pwd();
    if(g_rp.ADC_used)g_hsumm_ADC_SIG=new TProfile("summ_SIG_ADC", "summary ADC SIG",nADCconns, 0,nADCconns, 0,4096,"S");
    if(g_rp.TDC_used)g_hsumm_TDC_SIG=new TProfile("summ_SIG_TDC", "summary TDC SIG",nTDCconns, 0,nTDCconns, -2e4,2e4,"S");
    if(g_rp.digitizer_used)g_hsumm_DIG_SIG=new TProfile("summ_SIG_DIG", "summary Digitizer SIG",nDIGconns, 0,nDIGconns, -4096,4096,"S");
    iADC=iDIG=0;
    for(int ich=0; ich<g_rp.nchans; ++ich){
      if(1==g_rp.datatype[ich]){// ADC
        sprintf(nam,"%s_SIG_ADC_%2.2d",&g_rp.chnam[ich][0],g_rp.datachan[ich]);
        sprintf(tit,"%s (ADC%2.2d) SIG",&g_rp.chnam[ich][0],g_rp.datachan[ich]);
        g_hADC_SIG[g_rp.datachan[ich]]=new TH1I(nam,tit,4096,0,4096);
        nh1i++;
        g_hsumm_ADC_SIG->GetXaxis()->SetBinLabel(iADC+1,&g_rp.chnam[ich][0]);
        iADC++;
      }
      else if(3==g_rp.datatype[ich]){// DIG
        sprintf(nam,"%s_SIG_DIG_%2.2d_PED",&g_rp.chnam[ich][0],g_rp.datachan[ich]);
        sprintf(tit,"%s (DIG%2.2d) SIG PED",&g_rp.chnam[ich][0],g_rp.datachan[ich]);
        g_hDIG_SIGPED[g_rp.datachan[ich]]=new TH1I(nam,tit,4096,0,4096);
        nh1i++;
        sprintf(nam,"%s_SIG_DIG_%2.2d_MAX",&g_rp.chnam[ich][0],g_rp.datachan[ich]);
        sprintf(tit,"%s (DIG%2.2d) SIG MAX",&g_rp.chnam[ich][0],g_rp.datachan[ich]);
        g_hDIG_SIGMAX[g_rp.datachan[ich]]=new TH1I(nam,tit,4096,0,4096);
        nh1i++;
        g_hsumm_DIG_SIG->GetXaxis()->SetBinLabel(iDIG+1,&g_rp.chnam[ich][0]);
        iDIG++;
      }
    }
    iTDC=0;
    if(g_dwc1left>=0 && g_dwc1right>=0){
      g_hdwc1x_SIG=new TH1I("dwc1x_SIG_TDC","dwc1x SIG",300,-75,75); 
      nh1i++;
      g_hsumm_TDC_SIG->GetXaxis()->SetBinLabel(iTDC+1,"dwc1x");
      iTDC++;
    }
    if(g_dwc1up>=0   && g_dwc1down>=0 ){
      g_hdwc1y_SIG=new TH1I("dwc1y_SIG_TDC","dwc1y SIG",300,-75,75); 
      nh1i++;
      g_hsumm_TDC_SIG->GetXaxis()->SetBinLabel(iTDC+1,"dwc1y");
      iTDC++;
    }
    if(g_dwc2left>=0 && g_dwc2right>=0){
      g_hdwc2x_SIG=new TH1I("dwc2x_SIG_TDC","dwc2x SIG",300,-75,75); 
      nh1i++;
      g_hsumm_TDC_SIG->GetXaxis()->SetBinLabel(iTDC+1,"dwc2x");
      iTDC++;
    }
    if(g_dwc2up>=0   && g_dwc2down>=0 ){
      g_hdwc2y_SIG=new TH1I("dwc2y_SIG_TDC","dwc2y SIG",300,-75,75); 
      nh1i++;
      g_hsumm_TDC_SIG->GetXaxis()->SetBinLabel(iTDC+1,"dwc2y");
      iTDC++;
    }
    if(g_dwc3left>=0 && g_dwc3right>=0){
      g_hdwc3x_SIG=new TH1I("dwc3x_SIG_TDC","dwc3x SIG",300,-75,75); 
      nh1i++;
      g_hsumm_TDC_SIG->GetXaxis()->SetBinLabel(iTDC+1,"dwc3x");
      iTDC++;
    }
    if(g_dwc3up>=0   && g_dwc3down>=0 ){
      g_hdwc3y_SIG=new TH1I("dwc3y_SIG_TDC","dwc3y SIG",300,-75,75); 
      nh1i++;
      g_hsumm_TDC_SIG->GetXaxis()->SetBinLabel(iTDC+1,"dwc3y");
      iTDC++;
    }
    if(g_dwc4left>=0 && g_dwc4right>=0){
      g_hdwc4x_SIG=new TH1I("dwc4x_SIG_TDC","dwc4x SIG",300,-75,75); 
      nh1i++;
      g_hsumm_TDC_SIG->GetXaxis()->SetBinLabel(iTDC+1,"dwc4x");
      iTDC++;
    }
    if(g_dwc4up>=0   && g_dwc4down>=0 ){
      g_hdwc4y_SIG=new TH1I("dwc4y_SIG_TDC","dwc4y SIG",300,-75,75); 
      nh1i++;
      g_hsumm_TDC_SIG->GetXaxis()->SetBinLabel(iTDC+1,"dwc1Y");
      iTDC++;
    }
    gDirectory->cd("..");
    //gDirectory->pwd();
    
    printf("\n   %s INFO: %d TH1I histograms created\n",__func__,nh1i);

    g_ROOTtree=new TTree("DATA", "DATA");
    
    g_ROOTtree->Branch("t",&g_t, "t/D");
    g_ROOTtree->Branch("pattern",&g_pattern, "pattern/I");
    g_ROOTtree->Branch("tTDCtrig",&g_tTDCtrig, "tTDCtrig/I");
  
    memset(g_used742,0,sizeof(g_used742));
    
    for(int i=0 ; i<g_rp.nchans; ++i){
      if(1==g_rp.datatype[i]){ // ADC
        sprintf(nam,"%s_A%2.2d",&g_rp.chnam[i][0],g_rp.datachan[i]);
        sprintf(fmt,"%s/I",nam);
        g_ROOTtree->Branch(nam,&g_ADC[g_rp.datachan[i]],fmt);
      }
      else if(2==g_rp.datatype[i]){ // TDC
        sprintf(nam,"%s_nt%2.2d",&g_rp.chnam[i][0],g_rp.datachan[i]);
        sprintf(fmt,"%s/I",nam);
        g_ROOTtree->Branch(nam,&g_nTDC[g_rp.datachan[i]],fmt);
        sprintf(nam1,"%s_tt%2.2d",&g_rp.chnam[i][0],g_rp.datachan[i]);
        sprintf(fmt,"%s[%s]/I",nam1,nam);
        g_ROOTtree->Branch(nam1,&g_tTDC[g_rp.datachan[i]][0],fmt);
      }
      else if(3==g_rp.datatype[i]){ // DIG
        int JCH=g_rp.datachan[i];
        if(g_rp.datachan[i]==20){
          JCH=8;// tr0 digitized by chip 0
          printf("tr0 digitized by chip 0 is connected to ch 8\n");
        }
        else if(g_rp.datachan[i]==21){
          JCH=17;// tr0 digitized by chip 1
          printf("tr0 digitized by chip 1 is connected to ch 17\n");
        }
        else if(g_rp.datachan[i]>7){
          JCH+=1; // accounts for tr0 which is inserted as #8
          printf("input %2d is connected to ch %d\n",g_rp.datachan[i],JCH);
        }
        g_used742[JCH]=1;
        sprintf(nam,"%s_nd%2.2d",&g_rp.chnam[i][0],g_rp.datachan[i]);
        sprintf(fmt,"%s/I",nam);
        g_ROOTtree->Branch(nam,&g_nDT5742[JCH],fmt);
        sprintf(nam1,"%s_ad%2.2d",&g_rp.chnam[i][0],g_rp.datachan[i]);
        sprintf(fmt,"%s[%s]/F",nam1,nam);
        g_ROOTtree->Branch(nam1,&g_aDT5742[JCH][0],fmt);
      }
    }
    if(g_dwc1left>=0 && g_dwc1right>=0)  g_ROOTtree->Branch("x1",&g_x1,"x1/D");
    if(g_dwc1up>=0 && g_dwc1down>=0)     g_ROOTtree->Branch("y1",&g_y1,"y1/D");
    if(g_dwc2left>=0 && g_dwc2right>=0)  g_ROOTtree->Branch("x2",&g_x2,"x2/D");
    if(g_dwc2up>=0 && g_dwc2down>=0)     g_ROOTtree->Branch("y2",&g_y2,"y2/D");
    if(g_dwc3left>=0 && g_dwc3right>=0)  g_ROOTtree->Branch("x3",&g_x3,"x3/D");
    if(g_dwc3up>=0 && g_dwc3down>=0)     g_ROOTtree->Branch("y3",&g_y3,"y3/D");
    if(g_dwc4left>=0 && g_dwc4right>=0)  g_ROOTtree->Branch("x4",&g_x4,"x4/D");
    if(g_dwc4up>=0 && g_dwc4down>=0)     g_ROOTtree->Branch("y4",&g_y4,"y4/D");
  }
}

void closeROOTfile(){
  //if(g_ROOTfile && g_ROOTtree){
  if(g_ROOTfile){
    g_ROOTfile->cd();
    
    if(g_hPMTHV)g_hPMTHV->Write();
    if(g_hULED)g_hULED->Write();
    if(g_ht0)g_ht0->Write();
    if(g_hpattern)g_hpattern->Write();
    
    gDirectory->cd("LED");
    for(int i=0; i<NADCCHAN; ++i) if(g_hADC_LED[i]) g_hADC_LED[i]->Write();
    if(g_hsumm_ADC_LED)g_hsumm_ADC_LED->Write();
    gDirectory->cd("../PED");
    for(int i=0; i<NADCCHAN; ++i) if(g_hADC_PED[i]) g_hADC_PED[i]->Write();
    if(g_hsumm_ADC_PED)g_hsumm_ADC_PED->Write();
    gDirectory->cd("../SIG");
    for(int i=0; i<NADCCHAN; ++i) if(g_hADC_SIG[i]) g_hADC_SIG[i]->Write();
    if(g_hsumm_ADC_SIG)g_hsumm_ADC_SIG->Write();
    
    gDirectory->cd("../LED");
    if(g_hdwc1x_LED) g_hdwc1x_LED->Write(); 
    if(g_hdwc1y_LED) g_hdwc1y_LED->Write(); 
    if(g_hdwc2x_LED) g_hdwc2x_LED->Write(); 
    if(g_hdwc2y_LED) g_hdwc2y_LED->Write(); 
    if(g_hdwc3x_LED) g_hdwc3x_LED->Write(); 
    if(g_hdwc3y_LED) g_hdwc3y_LED->Write(); 
    if(g_hdwc4x_LED) g_hdwc4x_LED->Write(); 
    if(g_hdwc4y_LED) g_hdwc4y_LED->Write(); 
    if(g_hsumm_TDC_LED)g_hsumm_TDC_LED->Write();
    gDirectory->cd("../SIG");
    if(g_hdwc1x_SIG) g_hdwc1x_SIG->Write(); 
    if(g_hdwc1y_SIG) g_hdwc1y_SIG->Write(); 
    if(g_hdwc2x_SIG) g_hdwc2x_SIG->Write(); 
    if(g_hdwc2y_SIG) g_hdwc2y_SIG->Write(); 
    if(g_hdwc3x_SIG) g_hdwc3x_SIG->Write(); 
    if(g_hdwc3y_SIG) g_hdwc3y_SIG->Write(); 
    if(g_hdwc4x_SIG) g_hdwc4x_SIG->Write(); 
    if(g_hdwc4y_SIG) g_hdwc4y_SIG->Write(); 
    if(g_hsumm_TDC_SIG)g_hsumm_TDC_SIG->Write();
    
    gDirectory->cd("../LED");
    for(int i=0; i<NDT5742CHAN; ++i){
      if(g_hDIG_LEDPED[i]) g_hDIG_LEDPED[i]->Write(); 
      if(g_hDIG_LEDMAX[i]) g_hDIG_LEDMAX[i]->Write(); 
    }
    if(g_hsumm_DIG_LED)g_hsumm_DIG_LED->Write();
    gDirectory->cd("../PED");
    for(int i=0; i<NDT5742CHAN; ++i){
      if(g_hDIG_PEDPED[i]) g_hDIG_PEDPED[i]->Write(); 
      if(g_hDIG_PEDMAX[i]) g_hDIG_PEDMAX[i]->Write(); 
    }
    if(g_hsumm_DIG_PED)g_hsumm_DIG_PED->Write();
    gDirectory->cd("../SIG");
    for(int i=0; i<NDT5742CHAN; ++i){
      if(g_hDIG_SIGPED[i]) g_hDIG_SIGPED[i]->Write(); 
      if(g_hDIG_SIGMAX[i]) g_hDIG_SIGMAX[i]->Write(); 
    }
    if(g_hsumm_DIG_SIG)g_hsumm_DIG_SIG->Write();
    
    gDirectory->cd("..");
    if(g_ROOTtree){
      g_ROOTtree->Write();
    }
    
    g_ROOTfile->Write();
    g_ROOTfile->Close();
    
    g_ROOTfile=(TFile*)0;
    g_ROOTtree=(TTree*)0;
    
    g_hPMTHV=g_hULED=g_ht0=0;
    g_hpattern=0;
    for(int i=0; i<NADCCHAN; ++i) g_hADC_LED[i]=g_hADC_PED[i]=g_hADC_SIG[i]=0;
    g_hdwc1x_LED=g_hdwc1y_LED=g_hdwc2x_LED=g_hdwc2y_LED=0;
    g_hdwc3x_LED=g_hdwc3y_LED=g_hdwc4x_LED=g_hdwc4y_LED=0;
    g_hdwc1x_SIG=g_hdwc1y_SIG=g_hdwc2x_SIG=g_hdwc2y_SIG=0;
    g_hdwc3x_SIG=g_hdwc3y_SIG=g_hdwc4x_SIG=g_hdwc4y_SIG=0;
    for(int i=0; i<NDT5742CHAN; ++i){
      g_hDIG_LEDPED[i]=g_hDIG_LEDMAX[i]=0;
      g_hDIG_PEDPED[i]=g_hDIG_PEDMAX[i]=0;
      g_hDIG_SIGPED[i]=g_hDIG_SIGMAX[i]=0; 
    }
    g_hsumm_ADC_LED=0;
    g_hsumm_ADC_PED=0;
    g_hsumm_ADC_SIG=0;
    g_hsumm_DIG_LED=0;
    g_hsumm_DIG_PED=0;
    g_hsumm_DIG_SIG=0;
    g_hsumm_TDC_LED=0;
    g_hsumm_TDC_SIG=0;
    g_dwc1left=g_dwc1right=g_dwc1up=g_dwc1down =-1;
    g_dwc2left=g_dwc2right=g_dwc2up=g_dwc2down =-1;
    g_dwc3left=g_dwc3right=g_dwc3up=g_dwc3down =-1;
    g_dwc4left=g_dwc4right=g_dwc4up=g_dwc4down =-1;
  }
}

double getped_742(int n, float* d){
  int nfirst=10;
  if(n<nfirst)return 0;
  
  double s=0;
  for(int i=0; i<nfirst; ++i)   s+=d[i]/nfirst;
  return s;
}

double getmax_742(int n, float* d){
  int nfirst=10;
  if(n<nfirst)return 0;
  
  double s=-1e9;
  for(int i=0; i<n; ++i) if(s<d[i])s=d[i];
  return s;
}

double getmin_742(int n, float* d){
  int nfirst=10;
  if(n<nfirst)return 0;
  
  double s=1e9;
  for(int i=0; i<n; ++i) if(s>d[i])s=d[i];
  return s;
}

int findbin(TH1* h, const char* t){ // returns bin number, 1...N+1, as in the TAxis convention
  TAxis* a=h->GetXaxis();
  int nbin=a->GetNbins();
  for(int i=1; i<nbin+2; ++i){
    const char* c=a->GetBinLabel(i);
    if(0==strcmp(c,t))return i;
  }
}

void fill_all(){
  if(g_ROOTfile && g_ROOTtree){
    TTimeStamp ttst;
    g_tprev=g_t;  // save the previous event time
    g_t=ttst.AsDouble()-g_t0;
    
    g_hpattern->Fill(g_pattern);
    
    for(int i=0; i<NADCCHAN; ++i){
      int ich=g_rp.findch("ADC",i); // entry number in runparams
      const char* nam=0;
      int ibin=0;
      if(ich>=0) {
        nam=&(g_rp.chnam[ich][0]); // channel name
        ibin=findbin(g_hsumm_ADC_PED, nam); // assuming all ADC summary histograms have same bin titles.
      }
      if(     g_rp.PEDpatt==g_pattern && 0!=g_hADC_PED[i]) {
        g_hADC_PED[i]->Fill(g_ADC[i]);
        g_hsumm_ADC_PED->Fill(ibin-0.5,g_ADC[i]);
      }
      else if(g_rp.LEDpatt==g_pattern && 0!=g_hADC_LED[i]) {
        g_hADC_LED[i]->Fill(g_ADC[i]);
        g_hsumm_ADC_LED->Fill(ibin-0.5,g_ADC[i]);
      }
      else if(g_rp.SIGpatt==g_pattern && 0!=g_hADC_SIG[i]) {
        g_hADC_SIG[i]->Fill(g_ADC[i]);
        g_hsumm_ADC_SIG->Fill(ibin-0.5,g_ADC[i]);
      }
    }
    
    for(int JCH=0; JCH<NDT5742CHAN; ++JCH){ // loop over DIG data, channels 0-17
      int i=JCH;                            // i is the channel # as in runparams
      if(8==JCH)i=20;
      else if(17==JCH)i=21;
      else if(JCH>=9 && JCH<=16)i=JCH-1;
      int ich=g_rp.findch("DIG",i);         // entry number in runparams
      const char *nam=0;
      int ibin=0;
      if(ich>=0) {
        nam=&(g_rp.chnam[ich][0]); // channel name
        ibin=findbin(g_hsumm_DIG_PED, nam); // assuming all DIG summary histograms have same bin titles.
      }
      if(g_used742[JCH]){
        double dped=getped_742(g_nDT5742[JCH],g_evdata742[JCH]);
        double dmax=getmax_742(g_nDT5742[JCH],g_evdata742[JCH]);
        double dmin=getmin_742(g_nDT5742[JCH],g_evdata742[JCH]);
        if(g_rp.PEDpatt==g_pattern){
          if(g_hDIG_PEDPED[i])g_hDIG_PEDPED[i]->Fill(dped);
          if(g_hDIG_PEDMAX[i])g_hDIG_PEDMAX[i]->Fill(dped-dmin);
          g_hsumm_DIG_PED->Fill(ibin-0.5,dped-dmin);
        }
        else if(g_rp.LEDpatt==g_pattern){
          if(g_hDIG_LEDPED[i])g_hDIG_LEDPED[i]->Fill(dped);     
          if(g_hDIG_LEDMAX[i])g_hDIG_LEDMAX[i]->Fill(dped-dmin);
          g_hsumm_DIG_LED->Fill(ibin-0.5,dped-dmin);
        }
        else if(g_rp.SIGpatt==g_pattern){
          if(g_hDIG_SIGPED[i])g_hDIG_SIGPED[i]->Fill(dped);     
          if(g_hDIG_SIGMAX[i])g_hDIG_SIGMAX[i]->Fill(dped-dmin);
          g_hsumm_DIG_SIG->Fill(ibin-0.5,dped-dmin);
        }
      }
    }
    
    g_x1=g_x2=g_x3=g_x4=g_y1=g_y2=g_y3=g_y4=-9999.;
    
    if(g_dwc1left>=0 && g_dwc1right>=0){
      int c1=g_rp.datachan[g_dwc1left];
      int c2=g_rp.datachan[g_dwc1right];
      TH1I* hLED=g_hdwc1x_LED;
      TH1I* hSIG=g_hdwc1x_SIG;
      int ibin=findbin(g_hsumm_TDC_SIG, "dwc1x");
      if(g_nTDC[c1]>0 && g_nTDC[c2]>0){
        g_x1=72./(g_rp.cx1[2]-g_rp.cx1[0])*(g_tTDC[c1][0]-g_tTDC[c2][0]-g_rp.cx1[1]);
        if(g_rp.LEDpatt==g_pattern && 0!=hLED){
          hLED->Fill(g_x1);
          g_hsumm_TDC_LED->Fill(ibin-0.5,g_x1);
        }
        else if(g_rp.SIGpatt==g_pattern && 0!=hSIG){
          hSIG->Fill(g_x1);
          g_hsumm_TDC_SIG->Fill(ibin-0.5,g_x1);
        }
      }
    }
    if(g_dwc2left>=0 && g_dwc2right>=0){
      int c1=g_rp.datachan[g_dwc2left];
      int c2=g_rp.datachan[g_dwc2right];
      TH1I* hLED=g_hdwc2x_LED;
      TH1I* hSIG=g_hdwc2x_SIG;
      int ibin=findbin(g_hsumm_TDC_SIG, "dwc2x");
      if(g_nTDC[c1]>0 && g_nTDC[c2]>0){
        g_x2= - 72./(g_rp.cx2[2]-g_rp.cx2[0])*(g_tTDC[c1][0]-g_tTDC[c2][0]-g_rp.cx2[1]); // NB inversion!
        if(g_rp.LEDpatt==g_pattern && 0!=hLED){
          hLED->Fill(g_x2);
          g_hsumm_TDC_LED->Fill(ibin-0.5,g_x2);
        }
        else if(g_rp.SIGpatt==g_pattern && 0!=hSIG){
          hSIG->Fill(g_x2);
          g_hsumm_TDC_SIG->Fill(ibin-0.5,g_x2);
        }
      }
    }
    if(g_dwc3left>=0 && g_dwc3right>=0){
      int c1=g_rp.datachan[g_dwc3left];
      int c2=g_rp.datachan[g_dwc3right];
      TH1I* hLED=g_hdwc3x_LED;
      TH1I* hSIG=g_hdwc3x_SIG;
      int ibin=findbin(g_hsumm_TDC_SIG, "dwc3x");
      if(g_nTDC[c1]>0 && g_nTDC[c2]>0){
        g_x3=72./(g_rp.cx3[2]-g_rp.cx3[0])*(g_tTDC[c1][0]-g_tTDC[c2][0]-g_rp.cx3[1]);
        if(g_rp.LEDpatt==g_pattern && 0!=hLED){
          hLED->Fill(g_x3);
          g_hsumm_TDC_LED->Fill(ibin-0.5,g_x3);
        }
        else if(g_rp.SIGpatt==g_pattern && 0!=hSIG){
          hSIG->Fill(g_x3);
          g_hsumm_TDC_SIG->Fill(ibin-0.5,g_x3);
        }
      }
    }
    if(g_dwc4left>=0 && g_dwc4right>=0){
      int c1=g_rp.datachan[g_dwc4left];
      int c2=g_rp.datachan[g_dwc4right];
      TH1I* hLED=g_hdwc4x_LED;
      TH1I* hSIG=g_hdwc4x_SIG;
      int ibin=findbin(g_hsumm_TDC_SIG, "dwc4x");
      if(g_nTDC[c1]>0 && g_nTDC[c2]>0){
        g_x4=72./(g_rp.cx4[2]-g_rp.cx4[0])*(g_tTDC[c1][0]-g_tTDC[c2][0]-g_rp.cx4[1]);
        if(g_rp.LEDpatt==g_pattern && 0!=hLED){
          hLED->Fill(g_x4);
          g_hsumm_TDC_LED->Fill(ibin-0.5,g_x4);
        }
        else if(g_rp.SIGpatt==g_pattern && 0!=hSIG){
          hSIG->Fill(g_x4);
          g_hsumm_TDC_SIG->Fill(ibin-0.5,g_x4);
        }
      }
    }
    
    if(g_dwc1up>=0 && g_dwc1down>=0){
      int c1=g_rp.datachan[g_dwc1down];
      int c2=g_rp.datachan[g_dwc1up];
      TH1I* hLED=g_hdwc1y_LED;
      TH1I* hSIG=g_hdwc1y_SIG;
      int ibin=findbin(g_hsumm_TDC_SIG, "dwc1y");
      if(g_nTDC[c1]>0 && g_nTDC[c2]>0){
        g_y1=72./(g_rp.cy1[2]-g_rp.cy1[0])*(g_tTDC[c1][0]-g_tTDC[c2][0]-g_rp.cy1[1]);
        if(g_rp.LEDpatt==g_pattern && 0!=hLED){
          hLED->Fill(g_y1);
          g_hsumm_TDC_LED->Fill(ibin-0.5,g_y1);
        }
        else if(g_rp.SIGpatt==g_pattern && 0!=hSIG){
          hSIG->Fill(g_y1);
          g_hsumm_TDC_SIG->Fill(ibin-0.5,g_y1);
        }
      }
    }
    if(g_dwc2up>=0 && g_dwc2down>=0){
      int c1=g_rp.datachan[g_dwc2down];
      int c2=g_rp.datachan[g_dwc2up];
      TH1I* hLED=g_hdwc2y_LED;
      TH1I* hSIG=g_hdwc2y_SIG;
      int ibin=findbin(g_hsumm_TDC_SIG, "dwc2y");
      if(g_nTDC[c1]>0 && g_nTDC[c2]>0){
        g_y2=72./(g_rp.cy2[2]-g_rp.cy2[0])*(g_tTDC[c1][0]-g_tTDC[c2][0]-g_rp.cy2[1]);
        if(g_rp.LEDpatt==g_pattern && 0!=hLED){
          hLED->Fill(g_y2);
          g_hsumm_TDC_LED->Fill(ibin-0.5,g_y2);
        }
        else if(g_rp.SIGpatt==g_pattern && 0!=hSIG){
          hSIG->Fill(g_y2);
          g_hsumm_TDC_SIG->Fill(ibin-0.5,g_y2);
        }
      }
    }
    if(g_dwc3up>=0 && g_dwc3down>=0){
      int c1=g_rp.datachan[g_dwc3down];
      int c2=g_rp.datachan[g_dwc3up];
      TH1I* hLED=g_hdwc3y_LED;
      TH1I* hSIG=g_hdwc3y_SIG;
      int ibin=findbin(g_hsumm_TDC_SIG, "dwc3y");
      if(g_nTDC[c1]>0 && g_nTDC[c2]>0){
        g_y3=72./(g_rp.cy3[2]-g_rp.cy3[0])*(g_tTDC[c1][0]-g_tTDC[c2][0]-g_rp.cy3[1]);
        if(g_rp.LEDpatt==g_pattern && 0!=hLED){
          hLED->Fill(g_y3);
          g_hsumm_TDC_LED->Fill(ibin-0.5,g_y3);
        }
        else if(g_rp.SIGpatt==g_pattern && 0!=hSIG){
          hSIG->Fill(g_y3);
          g_hsumm_TDC_SIG->Fill(ibin-0.5,g_y3);
        }
      }
    }
    if(g_dwc4up>=0 && g_dwc4down>=0){
      int c1=g_rp.datachan[g_dwc4down];
      int c2=g_rp.datachan[g_dwc4up];
      TH1I* hLED=g_hdwc4y_LED;
      TH1I* hSIG=g_hdwc4y_SIG;
      int ibin=findbin(g_hsumm_TDC_SIG, "dwc4y");
      if(g_nTDC[c1]>0 && g_nTDC[c2]>0){
        g_y4=72./(g_rp.cy4[2]-g_rp.cy4[0])*(g_tTDC[c1][0]-g_tTDC[c2][0]-g_rp.cy4[1]);
        if(g_rp.LEDpatt==g_pattern && 0!=hLED){
          hLED->Fill(g_y4);
          g_hsumm_TDC_LED->Fill(ibin-0.5,g_y4);
        }
        else if(g_rp.SIGpatt==g_pattern && 0!=hSIG){
          hSIG->Fill(g_y4);
          g_hsumm_TDC_SIG->Fill(ibin-0.5,g_y4);
        }
      }
    }
    
    g_ROOTtree->Fill();
  }
}

void delete_histos(){
  if(g_hPMTHV){ g_hPMTHV->Delete(); g_hPMTHV=0; }
  if(g_hULED){ g_hULED->Delete(); g_hULED=0; }
  if(g_ht0){ g_ht0->Delete(); g_ht0=0; }
  if(g_hpattern){ g_hpattern->Delete(); g_hpattern=0; }
  
  for(int i=0; i<NADCCHAN; ++i){
    if(g_hADC_LED[i]) { g_hADC_LED[i]->Delete(); g_hADC_LED[i]=0; }
    if(g_hADC_PED[i]) { g_hADC_PED[i]->Delete(); g_hADC_PED[i]=0; }
    if(g_hADC_SIG[i]) { g_hADC_SIG[i]->Delete(); g_hADC_SIG[i]=0; }
  }
  
  if(g_hsumm_ADC_PED) { g_hsumm_ADC_PED->Delete(); g_hsumm_ADC_PED=0; }
  if(g_hsumm_ADC_LED) { g_hsumm_ADC_LED->Delete(); g_hsumm_ADC_LED=0; }
  if(g_hsumm_ADC_SIG) { g_hsumm_ADC_SIG->Delete(); g_hsumm_ADC_SIG=0; }
  
  if(g_hdwc1x_LED) { g_hdwc1x_LED->Delete(); g_hdwc1x_LED=0; }
  if(g_hdwc1y_LED) { g_hdwc1y_LED->Delete(); g_hdwc1y_LED=0; }
  if(g_hdwc2x_LED) { g_hdwc2x_LED->Delete(); g_hdwc2x_LED=0; }
  if(g_hdwc2y_LED) { g_hdwc2y_LED->Delete(); g_hdwc2y_LED=0; }
  if(g_hdwc3x_LED) { g_hdwc3x_LED->Delete(); g_hdwc3x_LED=0; }
  if(g_hdwc3y_LED) { g_hdwc3y_LED->Delete(); g_hdwc3y_LED=0; }
  if(g_hdwc4x_LED) { g_hdwc4x_LED->Delete(); g_hdwc4x_LED=0; }
  if(g_hdwc4y_LED) { g_hdwc4y_LED->Delete(); g_hdwc4y_LED=0; }
  if(g_hdwc1x_SIG) { g_hdwc1x_SIG->Delete(); g_hdwc1x_SIG=0; }
  if(g_hdwc1y_SIG) { g_hdwc1y_SIG->Delete(); g_hdwc1y_SIG=0; }
  if(g_hdwc2x_SIG) { g_hdwc2x_SIG->Delete(); g_hdwc2x_SIG=0; }
  if(g_hdwc2y_SIG) { g_hdwc2y_SIG->Delete(); g_hdwc2y_SIG=0; }
  if(g_hdwc3x_SIG) { g_hdwc3x_SIG->Delete(); g_hdwc3x_SIG=0; }
  if(g_hdwc3y_SIG) { g_hdwc3y_SIG->Delete(); g_hdwc3y_SIG=0; }
  if(g_hdwc4x_SIG) { g_hdwc4x_SIG->Delete(); g_hdwc4x_SIG=0; }
  if(g_hdwc4y_SIG) { g_hdwc4y_SIG->Delete(); g_hdwc4y_SIG=0; }
  
  if(g_hsumm_TDC_LED) { g_hsumm_TDC_LED->Delete(); g_hsumm_TDC_LED=0; }
  if(g_hsumm_TDC_SIG) { g_hsumm_TDC_SIG->Delete(); g_hsumm_TDC_SIG=0; }
  
  for(int i=0; i<NDT5742CHAN; ++i){
    if(g_hDIG_LEDPED[i]) { g_hDIG_LEDPED[i]->Delete(); g_hDIG_LEDPED[i]=0; }
    if(g_hDIG_LEDMAX[i]) { g_hDIG_LEDMAX[i]->Delete(); g_hDIG_LEDMAX[i]=0; }
    if(g_hDIG_PEDPED[i]) { g_hDIG_PEDPED[i]->Delete(); g_hDIG_PEDPED[i]=0; }
    if(g_hDIG_PEDMAX[i]) { g_hDIG_PEDMAX[i]->Delete(); g_hDIG_PEDMAX[i]=0; }
    if(g_hDIG_SIGPED[i]) { g_hDIG_SIGPED[i]->Delete(); g_hDIG_SIGPED[i]=0; }
    if(g_hDIG_SIGMAX[i]) { g_hDIG_SIGMAX[i]->Delete(); g_hDIG_SIGMAX[i]=0; }
  }
  
  if(g_hsumm_DIG_PED) { g_hsumm_DIG_PED->Delete(); g_hsumm_DIG_PED=0; }
  if(g_hsumm_DIG_LED) { g_hsumm_DIG_LED->Delete(); g_hsumm_DIG_LED=0; }
  if(g_hsumm_DIG_SIG) { g_hsumm_DIG_SIG->Delete(); g_hsumm_DIG_SIG=0; }
}

void reset_histos(){
  if(g_hPMTHV)g_hPMTHV->Reset();
  if(g_hULED)g_hULED->Reset();
  if(g_ht0)g_ht0->Reset();
  if(g_hpattern)g_hpattern->Reset();
  
  for(int i=0; i<NADCCHAN; ++i){
    if(g_hADC_LED[i]) g_hADC_LED[i]->Reset();
    if(g_hADC_PED[i]) g_hADC_PED[i]->Reset();
    if(g_hADC_SIG[i]) g_hADC_SIG[i]->Reset();
  }
  
  if(g_hsumm_ADC_PED) g_hsumm_ADC_PED->Reset(); 
  if(g_hsumm_ADC_LED) g_hsumm_ADC_LED->Reset(); 
  if(g_hsumm_ADC_SIG) g_hsumm_ADC_SIG->Reset(); 
  
  if(g_hdwc1x_LED) g_hdwc1x_LED->Reset(); 
  if(g_hdwc1y_LED) g_hdwc1y_LED->Reset(); 
  if(g_hdwc2x_LED) g_hdwc2x_LED->Reset(); 
  if(g_hdwc2y_LED) g_hdwc2y_LED->Reset(); 
  if(g_hdwc3x_LED) g_hdwc3x_LED->Reset(); 
  if(g_hdwc3y_LED) g_hdwc3y_LED->Reset(); 
  if(g_hdwc4x_LED) g_hdwc4x_LED->Reset(); 
  if(g_hdwc4y_LED) g_hdwc4y_LED->Reset(); 
  if(g_hdwc1x_SIG) g_hdwc1x_SIG->Reset(); 
  if(g_hdwc1y_SIG) g_hdwc1y_SIG->Reset(); 
  if(g_hdwc2x_SIG) g_hdwc2x_SIG->Reset(); 
  if(g_hdwc2y_SIG) g_hdwc2y_SIG->Reset(); 
  if(g_hdwc3x_SIG) g_hdwc3x_SIG->Reset(); 
  if(g_hdwc3y_SIG) g_hdwc3y_SIG->Reset(); 
  if(g_hdwc4x_SIG) g_hdwc4x_SIG->Reset(); 
  if(g_hdwc4y_SIG) g_hdwc4y_SIG->Reset(); 
  
  if(g_hsumm_TDC_LED) g_hsumm_TDC_LED->Reset(); 
  if(g_hsumm_TDC_SIG) g_hsumm_TDC_SIG->Reset(); 

  for(int i=0; i<NDT5742CHAN; ++i){
    if(g_hDIG_LEDPED[i]) g_hDIG_LEDPED[i]->Reset(); 
    if(g_hDIG_LEDMAX[i]) g_hDIG_LEDMAX[i]->Reset(); 
    if(g_hDIG_PEDPED[i]) g_hDIG_PEDPED[i]->Reset(); 
    if(g_hDIG_PEDMAX[i]) g_hDIG_PEDMAX[i]->Reset(); 
    if(g_hDIG_SIGPED[i]) g_hDIG_SIGPED[i]->Reset(); 
    if(g_hDIG_SIGMAX[i]) g_hDIG_SIGMAX[i]->Reset(); 
  }
  if(g_hsumm_DIG_PED) g_hsumm_DIG_PED->Reset(); 
  if(g_hsumm_DIG_LED) g_hsumm_DIG_LED->Reset(); 
  if(g_hsumm_DIG_SIG) g_hsumm_DIG_SIG->Reset(); 
}

void delete_dimHists(){
  if(g_d_pattern){ delete g_d_pattern; g_d_pattern=0; }
  
  for(int i=0; i<NADCCHAN; ++i){
    if(g_d_ADC_LED[i]) { delete g_d_ADC_LED[i]; g_d_ADC_LED[i]=0; }
    if(g_d_ADC_PED[i]) { delete g_d_ADC_PED[i]; g_d_ADC_PED[i]=0; }
    if(g_d_ADC_SIG[i]) { delete g_d_ADC_SIG[i]; g_d_ADC_SIG[i]=0; }
  }
  
  if(g_d_summ_ADC_PED) { delete g_d_summ_ADC_PED; g_d_summ_ADC_PED=0; }
  if(g_d_summ_ADC_LED) { delete g_d_summ_ADC_LED; g_d_summ_ADC_LED=0; }
  if(g_d_summ_ADC_SIG) { delete g_d_summ_ADC_SIG; g_d_summ_ADC_SIG=0; }
  
  if(g_d_dwc1x_LED) { delete g_d_dwc1x_LED; g_d_dwc1x_LED=0; }
  if(g_d_dwc1y_LED) { delete g_d_dwc1y_LED; g_d_dwc1y_LED=0; }
  if(g_d_dwc2x_LED) { delete g_d_dwc2x_LED; g_d_dwc2x_LED=0; }
  if(g_d_dwc2y_LED) { delete g_d_dwc2y_LED; g_d_dwc2y_LED=0; }
  if(g_d_dwc3x_LED) { delete g_d_dwc3x_LED; g_d_dwc3x_LED=0; }
  if(g_d_dwc3y_LED) { delete g_d_dwc3y_LED; g_d_dwc3y_LED=0; }
  if(g_d_dwc4x_LED) { delete g_d_dwc4x_LED; g_d_dwc4x_LED=0; }
  if(g_d_dwc4y_LED) { delete g_d_dwc4y_LED; g_d_dwc4y_LED=0; }
  if(g_d_dwc1x_SIG) { delete g_d_dwc1x_SIG; g_d_dwc1x_SIG=0; }
  if(g_d_dwc1y_SIG) { delete g_d_dwc1y_SIG; g_d_dwc1y_SIG=0; }
  if(g_d_dwc2x_SIG) { delete g_d_dwc2x_SIG; g_d_dwc2x_SIG=0; }
  if(g_d_dwc2y_SIG) { delete g_d_dwc2y_SIG; g_d_dwc2y_SIG=0; }
  if(g_d_dwc3x_SIG) { delete g_d_dwc3x_SIG; g_d_dwc3x_SIG=0; }
  if(g_d_dwc3y_SIG) { delete g_d_dwc3y_SIG; g_d_dwc3y_SIG=0; }
  if(g_d_dwc4x_SIG) { delete g_d_dwc4x_SIG; g_d_dwc4x_SIG=0; }
  if(g_d_dwc4y_SIG) { delete g_d_dwc4y_SIG; g_d_dwc4y_SIG=0; }

  if(g_d_summ_TDC_LED) { delete g_d_summ_TDC_LED; g_d_summ_TDC_LED=0; }
  if(g_d_summ_TDC_SIG) { delete g_d_summ_TDC_SIG; g_d_summ_TDC_SIG=0; }
  
  for(int i=0; i<NDT5742CHAN; ++i){
    if(g_d_DIG_LEDPED[i]) { delete g_d_DIG_LEDPED[i]; g_d_DIG_LEDPED[i]=0; }
    if(g_d_DIG_LEDMAX[i]) { delete g_d_DIG_LEDMAX[i]; g_d_DIG_LEDMAX[i]=0; }
    if(g_d_DIG_PEDPED[i]) { delete g_d_DIG_PEDPED[i]; g_d_DIG_PEDPED[i]=0; }
    if(g_d_DIG_PEDMAX[i]) { delete g_d_DIG_PEDMAX[i]; g_d_DIG_PEDMAX[i]=0; }
    if(g_d_DIG_SIGPED[i]) { delete g_d_DIG_SIGPED[i]; g_d_DIG_SIGPED[i]=0; }
    if(g_d_DIG_SIGMAX[i]) { delete g_d_DIG_SIGMAX[i]; g_d_DIG_SIGMAX[i]=0; }
  }
  
  if(g_d_summ_DIG_PED) { delete g_d_summ_DIG_PED; g_d_summ_DIG_PED=0; }
  if(g_d_summ_DIG_LED) { delete g_d_summ_DIG_LED; g_d_summ_DIG_LED=0; }
  if(g_d_summ_DIG_SIG) { delete g_d_summ_DIG_SIG; g_d_summ_DIG_SIG=0; }
}

void delete_dimservices(){
  if(g_s_pattern){ delete g_s_pattern; g_s_pattern=0; }
  
  for(int i=0; i<NADCCHAN; ++i){
    if(g_s_ADC_LED[i]) { delete g_s_ADC_LED[i]; g_s_ADC_LED[i]=0; }
    if(g_s_ADC_PED[i]) { delete g_s_ADC_PED[i]; g_s_ADC_PED[i]=0; }
    if(g_s_ADC_SIG[i]) { delete g_s_ADC_SIG[i]; g_s_ADC_SIG[i]=0; }
  }
  
  if(g_s_summ_ADC_PED) { delete g_s_summ_ADC_PED; g_s_summ_ADC_PED=0; }
  if(g_s_summ_ADC_LED) { delete g_s_summ_ADC_LED; g_s_summ_ADC_LED=0; }
  if(g_s_summ_ADC_SIG) { delete g_s_summ_ADC_SIG; g_s_summ_ADC_SIG=0; }
  
  if(g_s_dwc1x_LED) { delete g_s_dwc1x_LED; g_s_dwc1x_LED=0; }
  if(g_s_dwc1y_LED) { delete g_s_dwc1y_LED; g_s_dwc1y_LED=0; }
  if(g_s_dwc2x_LED) { delete g_s_dwc2x_LED; g_s_dwc2x_LED=0; }
  if(g_s_dwc2y_LED) { delete g_s_dwc2y_LED; g_s_dwc2y_LED=0; }
  if(g_s_dwc3x_LED) { delete g_s_dwc3x_LED; g_s_dwc3x_LED=0; }
  if(g_s_dwc3y_LED) { delete g_s_dwc3y_LED; g_s_dwc3y_LED=0; }
  if(g_s_dwc4x_LED) { delete g_s_dwc4x_LED; g_s_dwc4x_LED=0; }
  if(g_s_dwc4y_LED) { delete g_s_dwc4y_LED; g_s_dwc4y_LED=0; }
  if(g_s_dwc1x_SIG) { delete g_s_dwc1x_SIG; g_s_dwc1x_SIG=0; }
  if(g_s_dwc1y_SIG) { delete g_s_dwc1y_SIG; g_s_dwc1y_SIG=0; }
  if(g_s_dwc2x_SIG) { delete g_s_dwc2x_SIG; g_s_dwc2x_SIG=0; }
  if(g_s_dwc2y_SIG) { delete g_s_dwc2y_SIG; g_s_dwc2y_SIG=0; }
  if(g_s_dwc3x_SIG) { delete g_s_dwc3x_SIG; g_s_dwc3x_SIG=0; }
  if(g_s_dwc3y_SIG) { delete g_s_dwc3y_SIG; g_s_dwc3y_SIG=0; }
  if(g_s_dwc4x_SIG) { delete g_s_dwc4x_SIG; g_s_dwc4x_SIG=0; }
  if(g_s_dwc4y_SIG) { delete g_s_dwc4y_SIG; g_s_dwc4y_SIG=0; }
  
  if(g_s_summ_TDC_LED) { delete g_s_summ_TDC_LED; g_s_summ_TDC_LED=0; }
  if(g_s_summ_TDC_SIG) { delete g_s_summ_TDC_SIG; g_s_summ_TDC_SIG=0; }
  
  for(int i=0; i<NDT5742CHAN; ++i){
    if(g_s_DIG_LEDPED[i]) { delete g_s_DIG_LEDPED[i]; g_s_DIG_LEDPED[i]=0; }
    if(g_s_DIG_LEDMAX[i]) { delete g_s_DIG_LEDMAX[i]; g_s_DIG_LEDMAX[i]=0; }
    if(g_s_DIG_PEDPED[i]) { delete g_s_DIG_PEDPED[i]; g_s_DIG_PEDPED[i]=0; }
    if(g_s_DIG_PEDMAX[i]) { delete g_s_DIG_PEDMAX[i]; g_s_DIG_PEDMAX[i]=0; }
    if(g_s_DIG_SIGPED[i]) { delete g_s_DIG_SIGPED[i]; g_s_DIG_SIGPED[i]=0; }
    if(g_s_DIG_SIGMAX[i]) { delete g_s_DIG_SIGMAX[i]; g_s_DIG_SIGMAX[i]=0; }
  }
  if(g_s_summ_DIG_PED) { delete g_s_summ_DIG_PED; g_s_summ_DIG_PED=0; }
  if(g_s_summ_DIG_LED) { delete g_s_summ_DIG_LED; g_s_summ_DIG_LED=0; }
  if(g_s_summ_DIG_SIG) { delete g_s_summ_DIG_SIG; g_s_summ_DIG_SIG=0; }
}

void create_dimHists(){
  int ndh=0;
  if(g_hpattern){ g_d_pattern=new dimHist(); fill_dimHist(g_d_pattern, g_hpattern); ndh++;}
  
  for(int i=0; i<NADCCHAN; ++i){
    if(g_hADC_LED[i]) { g_d_ADC_LED[i]=new dimHist(); fill_dimHist(g_d_ADC_LED[i],g_hADC_LED[i]);  ndh++;}
    if(g_hADC_PED[i]) { g_d_ADC_PED[i]=new dimHist(); fill_dimHist(g_d_ADC_PED[i],g_hADC_PED[i]);  ndh++;}
    if(g_hADC_SIG[i]) { g_d_ADC_SIG[i]=new dimHist(); fill_dimHist(g_d_ADC_SIG[i],g_hADC_SIG[i]);  ndh++;}
  }
  
  if(g_hsumm_ADC_PED) { g_d_summ_ADC_PED=new dimSummary(); g_d_summ_ADC_PED->fill_dimSummary(g_hsumm_ADC_PED); ndh++;}
  if(g_hsumm_ADC_LED) { g_d_summ_ADC_LED=new dimSummary(); g_d_summ_ADC_LED->fill_dimSummary(g_hsumm_ADC_LED); ndh++;}
  if(g_hsumm_ADC_SIG) { g_d_summ_ADC_SIG=new dimSummary(); g_d_summ_ADC_SIG->fill_dimSummary(g_hsumm_ADC_SIG); ndh++;}
  
  if(g_hdwc1x_LED) { g_d_dwc1x_LED=new dimHist(); fill_dimHist(g_d_dwc1x_LED,g_hdwc1x_LED);  ndh++;}
  if(g_hdwc1y_LED) { g_d_dwc1y_LED=new dimHist(); fill_dimHist(g_d_dwc1y_LED,g_hdwc1y_LED);  ndh++;}
  if(g_hdwc2x_LED) { g_d_dwc2x_LED=new dimHist(); fill_dimHist(g_d_dwc2x_LED,g_hdwc2x_LED);  ndh++;}
  if(g_hdwc2y_LED) { g_d_dwc2y_LED=new dimHist(); fill_dimHist(g_d_dwc2y_LED,g_hdwc2y_LED);  ndh++;}
  if(g_hdwc3x_LED) { g_d_dwc3x_LED=new dimHist(); fill_dimHist(g_d_dwc3x_LED,g_hdwc3x_LED);  ndh++;}
  if(g_hdwc3y_LED) { g_d_dwc3y_LED=new dimHist(); fill_dimHist(g_d_dwc3y_LED,g_hdwc3y_LED);  ndh++;}
  if(g_hdwc4x_LED) { g_d_dwc4x_LED=new dimHist(); fill_dimHist(g_d_dwc4x_LED,g_hdwc4x_LED);  ndh++;}
  if(g_hdwc4y_LED) { g_d_dwc4y_LED=new dimHist(); fill_dimHist(g_d_dwc4y_LED,g_hdwc4y_LED);  ndh++;}
  if(g_hdwc1x_SIG) { g_d_dwc1x_SIG=new dimHist(); fill_dimHist(g_d_dwc1x_SIG,g_hdwc1x_SIG);  ndh++;}
  if(g_hdwc1y_SIG) { g_d_dwc1y_SIG=new dimHist(); fill_dimHist(g_d_dwc1y_SIG,g_hdwc1y_SIG);  ndh++;}
  if(g_hdwc2x_SIG) { g_d_dwc2x_SIG=new dimHist(); fill_dimHist(g_d_dwc2x_SIG,g_hdwc2x_SIG);  ndh++;}
  if(g_hdwc2y_SIG) { g_d_dwc2y_SIG=new dimHist(); fill_dimHist(g_d_dwc2y_SIG,g_hdwc2y_SIG);  ndh++;}
  if(g_hdwc3x_SIG) { g_d_dwc3x_SIG=new dimHist(); fill_dimHist(g_d_dwc3x_SIG,g_hdwc3x_SIG);  ndh++;}
  if(g_hdwc3y_SIG) { g_d_dwc3y_SIG=new dimHist(); fill_dimHist(g_d_dwc3y_SIG,g_hdwc3y_SIG);  ndh++;}
  if(g_hdwc4x_SIG) { g_d_dwc4x_SIG=new dimHist(); fill_dimHist(g_d_dwc4x_SIG,g_hdwc4x_SIG);  ndh++;}
  if(g_hdwc4y_SIG) { g_d_dwc4y_SIG=new dimHist(); fill_dimHist(g_d_dwc4y_SIG,g_hdwc4y_SIG);  ndh++;}
  
  if(g_hsumm_TDC_LED) { g_d_summ_TDC_LED=new dimSummary(); g_d_summ_TDC_LED->fill_dimSummary(g_hsumm_TDC_LED); ndh++;}
  if(g_hsumm_TDC_SIG) { g_d_summ_TDC_SIG=new dimSummary(); g_d_summ_TDC_SIG->fill_dimSummary(g_hsumm_TDC_SIG); ndh++;}
  
  for(int i=0; i<NDT5742CHAN; ++i){
    if(g_hDIG_LEDPED[i]) { g_d_DIG_LEDPED[i]=new dimHist(); fill_dimHist(g_d_DIG_LEDPED[i],g_hDIG_LEDPED[i]); ndh++;}
    if(g_hDIG_LEDMAX[i]) { g_d_DIG_LEDMAX[i]=new dimHist(); fill_dimHist(g_d_DIG_LEDMAX[i],g_hDIG_LEDMAX[i]); ndh++;}
    if(g_hDIG_PEDPED[i]) { g_d_DIG_PEDPED[i]=new dimHist(); fill_dimHist(g_d_DIG_PEDPED[i],g_hDIG_PEDPED[i]); ndh++;}
    if(g_hDIG_PEDMAX[i]) { g_d_DIG_PEDMAX[i]=new dimHist(); fill_dimHist(g_d_DIG_PEDMAX[i],g_hDIG_PEDMAX[i]); ndh++;}
    if(g_hDIG_SIGPED[i]) { g_d_DIG_SIGPED[i]=new dimHist(); fill_dimHist(g_d_DIG_SIGPED[i],g_hDIG_SIGPED[i]); ndh++;}
    if(g_hDIG_SIGMAX[i]) { g_d_DIG_SIGMAX[i]=new dimHist(); fill_dimHist(g_d_DIG_SIGMAX[i],g_hDIG_SIGMAX[i]); ndh++;}
  }
  if(g_hsumm_DIG_PED) { g_d_summ_DIG_PED=new dimSummary(); g_d_summ_DIG_PED->fill_dimSummary(g_hsumm_DIG_PED); ndh++;}
  if(g_hsumm_DIG_LED) { g_d_summ_DIG_LED=new dimSummary(); g_d_summ_DIG_LED->fill_dimSummary(g_hsumm_DIG_LED); ndh++;}
  if(g_hsumm_DIG_SIG) { g_d_summ_DIG_SIG=new dimSummary(); g_d_summ_DIG_SIG->fill_dimSummary(g_hsumm_DIG_SIG); ndh++;}
  
  printf("   %s INFO: %d DIMhists created\n",__func__,ndh);
}

void update_dimHists(){
  if(!g_running)return;
  
  if(g_d_pattern) fill_dimHist(g_d_pattern, g_hpattern);
  
  for(int i=0; i<NADCCHAN; ++i){
    if(g_d_ADC_LED[i]) fill_dimHist(g_d_ADC_LED[i],g_hADC_LED[i]);
    if(g_d_ADC_PED[i]) fill_dimHist(g_d_ADC_PED[i],g_hADC_PED[i]);
    if(g_d_ADC_SIG[i]) fill_dimHist(g_d_ADC_SIG[i],g_hADC_SIG[i]);
  }
  
  if(g_d_summ_ADC_PED) g_d_summ_ADC_PED->fill_dimSummary(g_hsumm_ADC_PED);
  if(g_d_summ_ADC_LED) g_d_summ_ADC_LED->fill_dimSummary(g_hsumm_ADC_LED);
  if(g_d_summ_ADC_SIG) g_d_summ_ADC_SIG->fill_dimSummary(g_hsumm_ADC_SIG);
  
  if(g_d_dwc1x_LED) fill_dimHist(g_d_dwc1x_LED,g_hdwc1x_LED);
  if(g_d_dwc1y_LED) fill_dimHist(g_d_dwc1y_LED,g_hdwc1y_LED);
  if(g_d_dwc2x_LED) fill_dimHist(g_d_dwc2x_LED,g_hdwc2x_LED);
  if(g_d_dwc2y_LED) fill_dimHist(g_d_dwc2y_LED,g_hdwc2y_LED);
  if(g_d_dwc3x_LED) fill_dimHist(g_d_dwc3x_LED,g_hdwc3x_LED);
  if(g_d_dwc3y_LED) fill_dimHist(g_d_dwc3y_LED,g_hdwc3y_LED);
  if(g_d_dwc4x_LED) fill_dimHist(g_d_dwc4x_LED,g_hdwc4x_LED);
  if(g_d_dwc4y_LED) fill_dimHist(g_d_dwc4y_LED,g_hdwc4y_LED);
  if(g_d_dwc1x_SIG) fill_dimHist(g_d_dwc1x_SIG,g_hdwc1x_SIG);
  if(g_d_dwc1y_SIG) fill_dimHist(g_d_dwc1y_SIG,g_hdwc1y_SIG);
  if(g_d_dwc2x_SIG) fill_dimHist(g_d_dwc2x_SIG,g_hdwc2x_SIG);
  if(g_d_dwc2y_SIG) fill_dimHist(g_d_dwc2y_SIG,g_hdwc2y_SIG);
  if(g_d_dwc3x_SIG) fill_dimHist(g_d_dwc3x_SIG,g_hdwc3x_SIG);
  if(g_d_dwc3y_SIG) fill_dimHist(g_d_dwc3y_SIG,g_hdwc3y_SIG);
  if(g_d_dwc4x_SIG) fill_dimHist(g_d_dwc4x_SIG,g_hdwc4x_SIG);
  if(g_d_dwc4y_SIG) fill_dimHist(g_d_dwc4y_SIG,g_hdwc4y_SIG);
  
  if(g_d_summ_TDC_LED) g_d_summ_TDC_LED->fill_dimSummary(g_hsumm_TDC_LED);
  if(g_d_summ_TDC_SIG) g_d_summ_TDC_SIG->fill_dimSummary(g_hsumm_TDC_SIG);
  
  for(int i=0; i<NDT5742CHAN; ++i){
    if(g_d_DIG_LEDPED[i]) fill_dimHist(g_d_DIG_LEDPED[i],g_hDIG_LEDPED[i]);
    if(g_d_DIG_LEDMAX[i]) fill_dimHist(g_d_DIG_LEDMAX[i],g_hDIG_LEDMAX[i]);
    if(g_d_DIG_PEDPED[i]) fill_dimHist(g_d_DIG_PEDPED[i],g_hDIG_PEDPED[i]);
    if(g_d_DIG_PEDMAX[i]) fill_dimHist(g_d_DIG_PEDMAX[i],g_hDIG_PEDMAX[i]);
    if(g_d_DIG_SIGPED[i]) fill_dimHist(g_d_DIG_SIGPED[i],g_hDIG_SIGPED[i]);
    if(g_d_DIG_SIGMAX[i]) fill_dimHist(g_d_DIG_SIGMAX[i],g_hDIG_SIGMAX[i]);
  }
  
  if(g_d_summ_DIG_PED) g_d_summ_DIG_PED->fill_dimSummary(g_hsumm_DIG_PED);
  if(g_d_summ_DIG_LED) g_d_summ_DIG_LED->fill_dimSummary(g_hsumm_DIG_LED);
  if(g_d_summ_DIG_SIG) g_d_summ_DIG_SIG->fill_dimSummary(g_hsumm_DIG_SIG);
}

void create_dimservices(){
  int nds=0;
  if(g_d_pattern) {
    g_s_pattern=new DimService(g_d_pattern->name,g_d_pattern->fmat,g_d_pattern,sizeof(*g_d_pattern));
    nds++;
  }
  
  for(int i=0; i<NADCCHAN; ++i){
    if(g_d_ADC_LED[i]){
      g_s_ADC_LED[i]=new DimService(g_d_ADC_LED[i]->name,g_d_ADC_LED[i]->fmat,g_d_ADC_LED[i],sizeof(*g_d_ADC_LED[i]));
      nds++;
    }
    if(g_d_ADC_PED[i]){
      g_s_ADC_PED[i]=new DimService(g_d_ADC_PED[i]->name,g_d_ADC_PED[i]->fmat,g_d_ADC_PED[i],sizeof(*g_d_ADC_PED[i]));
      nds++;
    }
    if(g_d_ADC_SIG[i]){
      g_s_ADC_SIG[i]=new DimService(g_d_ADC_SIG[i]->name,g_d_ADC_SIG[i]->fmat,g_d_ADC_SIG[i],sizeof(*g_d_ADC_SIG[i]));
      nds++;
    }
  }
  
  if(g_d_summ_ADC_PED) { 
    g_s_summ_ADC_PED=new DimService(g_d_summ_ADC_PED->name,g_d_summ_ADC_PED->fmat,g_d_summ_ADC_PED,sizeof(*g_d_summ_ADC_PED)); 
    nds++;
  }
  if(g_d_summ_ADC_LED) { 
    g_s_summ_ADC_LED=new DimService(g_d_summ_ADC_LED->name,g_d_summ_ADC_LED->fmat,g_d_summ_ADC_LED,sizeof(*g_d_summ_ADC_LED)); 
    nds++;
  }
  if(g_d_summ_ADC_SIG) { 
    g_s_summ_ADC_SIG=new DimService(g_d_summ_ADC_SIG->name,g_d_summ_ADC_SIG->fmat,g_d_summ_ADC_SIG,sizeof(*g_d_summ_ADC_SIG)); 
    nds++;
  }
  
  if(g_d_dwc1x_LED){
    g_s_dwc1x_LED=new DimService(g_d_dwc1x_LED->name,g_d_dwc1x_LED->fmat,g_d_dwc1x_LED,sizeof(*g_d_dwc1x_LED));
    nds++;
  }
  if(g_d_dwc1y_LED){
    g_s_dwc1y_LED=new DimService(g_d_dwc1y_LED->name,g_d_dwc1y_LED->fmat,g_d_dwc1y_LED,sizeof(*g_d_dwc1y_LED));
    nds++;
  }
  if(g_d_dwc2x_LED){
    g_s_dwc2x_LED=new DimService(g_d_dwc2x_LED->name,g_d_dwc2x_LED->fmat,g_d_dwc2x_LED,sizeof(*g_d_dwc2x_LED));
    nds++;
  }
  if(g_d_dwc2y_LED){
    g_s_dwc2y_LED=new DimService(g_d_dwc2y_LED->name,g_d_dwc2y_LED->fmat,g_d_dwc2y_LED,sizeof(*g_d_dwc2y_LED));
    nds++;
  }
  if(g_d_dwc3x_LED){
    g_s_dwc3x_LED=new DimService(g_d_dwc3x_LED->name,g_d_dwc3x_LED->fmat,g_d_dwc3x_LED,sizeof(*g_d_dwc3x_LED));
    nds++;
  }
  if(g_d_dwc3y_LED){
    g_s_dwc3y_LED=new DimService(g_d_dwc3y_LED->name,g_d_dwc3y_LED->fmat,g_d_dwc3y_LED,sizeof(*g_d_dwc3y_LED));
    nds++;
  }
  if(g_d_dwc4x_LED){
    g_s_dwc4x_LED=new DimService(g_d_dwc4x_LED->name,g_d_dwc4x_LED->fmat,g_d_dwc4x_LED,sizeof(*g_d_dwc4x_LED));
    nds++;
  }
  if(g_d_dwc4y_LED){
    g_s_dwc4y_LED=new DimService(g_d_dwc4y_LED->name,g_d_dwc4y_LED->fmat,g_d_dwc4y_LED,sizeof(*g_d_dwc4y_LED));
    nds++;
  }
  if(g_d_dwc1x_SIG){
    g_s_dwc1x_SIG=new DimService(g_d_dwc1x_SIG->name,g_d_dwc1x_SIG->fmat,g_d_dwc1x_SIG,sizeof(*g_d_dwc1x_SIG));
    nds++;
  }
  if(g_d_dwc1y_SIG){
    g_s_dwc1y_SIG=new DimService(g_d_dwc1y_SIG->name,g_d_dwc1y_SIG->fmat,g_d_dwc1y_SIG,sizeof(*g_d_dwc1y_SIG));
    nds++;
  }
  if(g_d_dwc2x_SIG){
    g_s_dwc2x_SIG=new DimService(g_d_dwc2x_SIG->name,g_d_dwc2x_SIG->fmat,g_d_dwc2x_SIG,sizeof(*g_d_dwc2x_SIG));
    nds++;
  }
  if(g_d_dwc2y_SIG){
    g_s_dwc2y_SIG=new DimService(g_d_dwc2y_SIG->name,g_d_dwc2y_SIG->fmat,g_d_dwc2y_SIG,sizeof(*g_d_dwc2y_SIG));
    nds++;
  }
  if(g_d_dwc3x_SIG){
    g_s_dwc3x_SIG=new DimService(g_d_dwc3x_SIG->name,g_d_dwc3x_SIG->fmat,g_d_dwc3x_SIG,sizeof(*g_d_dwc3x_SIG));
    nds++;
  }
  if(g_d_dwc3y_SIG){
    g_s_dwc3y_SIG=new DimService(g_d_dwc3y_SIG->name,g_d_dwc3y_SIG->fmat,g_d_dwc3y_SIG,sizeof(*g_d_dwc3y_SIG));
    nds++;
  }
  if(g_d_dwc4x_SIG){
    g_s_dwc4x_SIG=new DimService(g_d_dwc4x_SIG->name,g_d_dwc4x_SIG->fmat,g_d_dwc4x_SIG,sizeof(*g_d_dwc4x_SIG));
    nds++;
  }
  if(g_d_dwc4y_SIG){
    g_s_dwc4y_SIG=new DimService(g_d_dwc4y_SIG->name,g_d_dwc4y_SIG->fmat,g_d_dwc4y_SIG,sizeof(*g_d_dwc4y_SIG));
    nds++;
  }
  
  if(g_d_summ_TDC_LED) { 
    g_s_summ_TDC_LED=new DimService(g_d_summ_TDC_LED->name,g_d_summ_TDC_LED->fmat,g_d_summ_TDC_LED,sizeof(*g_d_summ_TDC_LED)); 
    nds++;
  }
  if(g_d_summ_TDC_SIG) { 
    g_s_summ_TDC_SIG=new DimService(g_d_summ_TDC_SIG->name,g_d_summ_TDC_SIG->fmat,g_d_summ_TDC_SIG,sizeof(*g_d_summ_TDC_SIG)); 
    nds++;
  }
  
  for(int i=0; i<NDT5742CHAN; ++i){
    if(g_d_DIG_LEDPED[i]){
      g_s_DIG_LEDPED[i]=new DimService(g_d_DIG_LEDPED[i]->name,g_d_DIG_LEDPED[i]->fmat,g_d_DIG_LEDPED[i],sizeof(*g_d_DIG_LEDPED[i]));
      nds++;
    }
    if(g_d_DIG_LEDMAX[i]){
      g_s_DIG_LEDMAX[i]=new DimService(g_d_DIG_LEDMAX[i]->name,g_d_DIG_LEDMAX[i]->fmat,g_d_DIG_LEDMAX[i],sizeof(*g_d_DIG_LEDMAX[i]));
      nds++;
    }
    if(g_d_DIG_PEDPED[i]){
      g_s_DIG_PEDPED[i]=new DimService(g_d_DIG_PEDPED[i]->name,g_d_DIG_PEDPED[i]->fmat,g_d_DIG_PEDPED[i],sizeof(*g_d_DIG_PEDPED[i]));
      nds++;
    }
    if(g_d_DIG_PEDMAX[i]){
      g_s_DIG_PEDMAX[i]=new DimService(g_d_DIG_PEDMAX[i]->name,g_d_DIG_PEDMAX[i]->fmat,g_d_DIG_PEDMAX[i],sizeof(*g_d_DIG_PEDMAX[i]));
      nds++;
    }
    if(g_d_DIG_SIGPED[i]){
      g_s_DIG_SIGPED[i]=new DimService(g_d_DIG_SIGPED[i]->name,g_d_DIG_SIGPED[i]->fmat,g_d_DIG_SIGPED[i],sizeof(*g_d_DIG_SIGPED[i]));
      nds++;
    }
    if(g_d_DIG_SIGMAX[i]){
      g_s_DIG_SIGMAX[i]=new DimService(g_d_DIG_SIGMAX[i]->name,g_d_DIG_SIGMAX[i]->fmat,g_d_DIG_SIGMAX[i],sizeof(*g_d_DIG_SIGMAX[i]));
      nds++;
    }
  }
  
  if(g_d_summ_DIG_PED) { 
    g_s_summ_DIG_PED=new DimService(g_d_summ_DIG_PED->name,g_d_summ_DIG_PED->fmat,g_d_summ_DIG_PED,sizeof(*g_d_summ_DIG_PED)); 
    nds++;
  }
  if(g_d_summ_DIG_LED) { 
    g_s_summ_DIG_LED=new DimService(g_d_summ_DIG_LED->name,g_d_summ_DIG_LED->fmat,g_d_summ_DIG_LED,sizeof(*g_d_summ_DIG_LED)); 
    nds++;
  }
  if(g_d_summ_DIG_SIG) { 
    g_s_summ_DIG_SIG=new DimService(g_d_summ_DIG_SIG->name,g_d_summ_DIG_SIG->fmat,g_d_summ_DIG_SIG,sizeof(*g_d_summ_DIG_SIG)); 
    nds++;
  }
  
  printf("   %s INFO: %d DIM services created\n",__func__,nds);
}

bool server_started(){
  DimBrowser dbr;
  dbr.getServers();
  char *server, *node, *service, *format;
  while(dbr.getNextServer(server, node)){
    if(0==strcmp(server,g_servernam))return true;
  }
  return false;
}

void update_dimservices(){
  if(!g_running)return;
  
  if(g_s_pattern) g_s_pattern->updateService();

  for(int i=0; i<NADCCHAN; ++i){
    if(g_s_ADC_LED[i]) g_s_ADC_LED[i]->updateService();
    if(g_s_ADC_PED[i]) g_s_ADC_PED[i]->updateService();
    if(g_s_ADC_SIG[i]) g_s_ADC_SIG[i]->updateService();
  }
  
  if(g_s_summ_ADC_LED) g_s_summ_ADC_LED->updateService();
  if(g_s_summ_ADC_PED) g_s_summ_ADC_PED->updateService();
  if(g_s_summ_ADC_SIG) g_s_summ_ADC_SIG->updateService();
  
  if(g_s_dwc1x_LED) g_s_dwc1x_LED->updateService();
  if(g_s_dwc1y_LED) g_s_dwc1y_LED->updateService();
  if(g_s_dwc2x_LED) g_s_dwc2x_LED->updateService();
  if(g_s_dwc2y_LED) g_s_dwc2y_LED->updateService();
  if(g_s_dwc3x_LED) g_s_dwc3x_LED->updateService();
  if(g_s_dwc3y_LED) g_s_dwc3y_LED->updateService();
  if(g_s_dwc4x_LED) g_s_dwc4x_LED->updateService();
  if(g_s_dwc4y_LED) g_s_dwc4y_LED->updateService();
  if(g_s_dwc1x_SIG) g_s_dwc1x_SIG->updateService();
  if(g_s_dwc1y_SIG) g_s_dwc1y_SIG->updateService();
  if(g_s_dwc2x_SIG) g_s_dwc2x_SIG->updateService();
  if(g_s_dwc2y_SIG) g_s_dwc2y_SIG->updateService();
  if(g_s_dwc3x_SIG) g_s_dwc3x_SIG->updateService();
  if(g_s_dwc3y_SIG) g_s_dwc3y_SIG->updateService();
  if(g_s_dwc4x_SIG) g_s_dwc4x_SIG->updateService();
  if(g_s_dwc4y_SIG) g_s_dwc4y_SIG->updateService();
  
  if(g_s_summ_TDC_LED) g_s_summ_TDC_LED->updateService();
  if(g_s_summ_TDC_SIG) g_s_summ_TDC_SIG->updateService();

  for(int i=0; i<NDT5742CHAN; ++i){
    if(g_s_DIG_LEDPED[i]) (g_s_DIG_LEDPED[i])->updateService();
    if(g_s_DIG_LEDMAX[i]) (g_s_DIG_LEDMAX[i])->updateService();
    if(g_s_DIG_PEDPED[i]) (g_s_DIG_PEDPED[i])->updateService();
    if(g_s_DIG_PEDMAX[i]) (g_s_DIG_PEDMAX[i])->updateService();
    if(g_s_DIG_SIGPED[i]) (g_s_DIG_SIGPED[i])->updateService();
    if(g_s_DIG_SIGMAX[i]) (g_s_DIG_SIGMAX[i])->updateService();
  }
  
  if(g_s_summ_DIG_LED) g_s_summ_DIG_LED->updateService();
  if(g_s_summ_DIG_PED) g_s_summ_DIG_PED->updateService();
  if(g_s_summ_DIG_SIG) g_s_summ_DIG_SIG->updateService();
}

int dimsrv_init(){
  if(!g_dimsrv_initialized){
    int ires=dim_set_dns_node (g_dimdns);
    
    if(server_started()){
      printf("%s: DIM server is already running\n",__func__);
      return -1;
    }
    
    g_cmnd = new DimCommand(g_cmdNam, "C");
    create_dimservstatus();
    update_dimservstatus();
    
    DimServer::start(g_servernam);
    g_dimsrv_initialized=true;
  }
  
  return 0;
}

void dimsrv_createhistsvc(){
  create_dimHists();
  create_dimservices();
}

void dimsrv_deletehistsvc(){
  delete_dimservices();
  delete_dimHists();
}

void dimsrv_exit(){
  //delete_dimservices();
  //delete_dimHists();
  
  DimServer::stop();
  g_dimsrv_initialized=false;
}

void dimsrv_update(){
  if(!g_running)return;
  update_dimHists();
  update_dimservices();
}

const char* dimsrv_getcommand(){
  if(g_cmnd->getNext()) return g_cmnd->getString();
  else return 0;
}

void delete_dimservstatus(){
  if(g_s_status){delete g_s_status; g_s_status=0;}
}

void create_dimservstatus(){
  if(!g_s_status)g_s_status=new DimService(g_statusNam,g_d_status.fmat,&g_d_status,sizeof(g_d_status));
  else printf("%s WARNING trying to re-create DIM status service\n",__func__);
}

void update_dimservstatus(){
  g_d_status.irun=g_runnumber;
  g_d_status.running=g_running?1:0;
  g_d_status.ievt=g_ievt;
  g_d_status.nsig=g_nsig;
  g_d_status.nled=g_nled;
  g_d_status.nped=g_nped;
  g_d_status.starttime=g_t0;
  g_d_status.runtime=g_t;
  strncpy(g_d_status.conf,g_config,127); g_d_status.conf[127]=0;
  strncpy(g_d_status.fnam,g_rootfilename,255); g_d_status.fnam[255]=0;
  
  if(g_s_status)g_s_status->updateService();
}
