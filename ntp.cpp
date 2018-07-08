//
//
#include <time.h>

#include "TFile.h"
#include "TTree.h"
#include "TH1D.h"
#include "TH1I.h"
#include "TTimeStamp.h"

#include "ntp.h"

TFile* g_ROOTfile=(TFile*)0;
TTree* g_ROOTtree=(TTree*)0;

// data
int g_nclear;

double g_t, g_t0;
int g_pattern; // 1=PED, 2=LED, 4=SIG
int g_ADC[NADCCHAN];

int g_nTDC[NTDCCHAN];
int g_tTDC[NTDCCHAN][NTDCMAXHITS];
int g_tTDCtrig;

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
TH1D* g_hPMTHV;
TH1D* g_hULED;
TH1D* g_ht0;

TH1I* g_hpattern;
//
TH1I* g_hADC_LED[NADCCHAN];
TH1I* g_hADC_PED[NADCCHAN];
TH1I* g_hADC_SIG[NADCCHAN];
TH1I *g_hdwc1x_LED, *g_hdwc1y_LED, *g_hdwc2x_LED, *g_hdwc2y_LED; 
TH1I *g_hdwc3x_LED, *g_hdwc3y_LED, *g_hdwc4x_LED, *g_hdwc4y_LED;
TH1I *g_hdwc1x_SIG, *g_hdwc1y_SIG, *g_hdwc2x_SIG, *g_hdwc2y_SIG; 
TH1I *g_hdwc3x_SIG, *g_hdwc3y_SIG, *g_hdwc4x_SIG, *g_hdwc4y_SIG;
TH1I* g_hDIG_LEDPED[NDT5742CHAN];
TH1I* g_hDIG_LEDMAX[NDT5742CHAN];
TH1I* g_hDIG_PEDPED[NDT5742CHAN];
TH1I* g_hDIG_PEDMAX[NDT5742CHAN];
TH1I* g_hDIG_SIGPED[NDT5742CHAN];
TH1I* g_hDIG_SIGMAX[NDT5742CHAN];
//
// end online histos

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
    
    g_hpattern=new TH1I("hpattern","pattern",16,0,16);
    
    g_dwc1left =g_rp.findch("dwc1left ");
    g_dwc1right=g_rp.findch("dwc1right");
    g_dwc1up   =g_rp.findch("dwc1up   ");
    g_dwc1down =g_rp.findch("dwc1down ");
    g_dwc2left =g_rp.findch("dwc2left ");
    g_dwc2right=g_rp.findch("dwc2right");
    g_dwc2up   =g_rp.findch("dwc2up   ");
    g_dwc2down =g_rp.findch("dwc2down ");
    g_dwc3left =g_rp.findch("dwc3left ");
    g_dwc3right=g_rp.findch("dwc3right");
    g_dwc3up   =g_rp.findch("dwc3up   ");
    g_dwc3down =g_rp.findch("dwc3down ");
    g_dwc4left =g_rp.findch("dwc4left ");
    g_dwc4right=g_rp.findch("dwc4right");
    g_dwc4up   =g_rp.findch("dwc4up   ");
    g_dwc4down =g_rp.findch("dwc4down ");
    
    gDirectory->mkdir("LED");
    gDirectory->cd("LED");
    gDirectory->pwd();
    for(int ich=0; ich<g_rp.nchans; ++ich){
      if(1==g_rp.datatype[ich]){// ADC
        sprintf(nam,"%s_LEDADC%2.2d",&g_rp.chnam[ich][0],g_rp.datachan[ich]);
        sprintf(tit,"%s (ADC%2.2d) LED",&g_rp.chnam[ich][0],g_rp.datachan[ich]);
        g_hADC_LED[g_rp.datachan[ich]]=new TH1I(nam,tit,4096,0,4096);
      }
      else if(3==g_rp.datatype[ich]){// DIG
        sprintf(nam,"%s_LED_DIG%2.2dPED",&g_rp.chnam[ich][0],g_rp.datachan[ich]);
        sprintf(tit,"%s (DIG%2.2d) LED PED",&g_rp.chnam[ich][0],g_rp.datachan[ich]);
        g_hDIG_LEDPED[g_rp.datachan[ich]]=new TH1I(nam,tit,4096,0,4096);
        sprintf(nam,"%s_LED_DIG%2.2dMAX",&g_rp.chnam[ich][0],g_rp.datachan[ich]);
        sprintf(tit,"%s (DIG%2.2d) LED MAX",&g_rp.chnam[ich][0],g_rp.datachan[ich]);
        g_hDIG_LEDMAX[g_rp.datachan[ich]]=new TH1I(nam,tit,4096,0,4096);
      }
    }
    if(g_dwc1left>=0 && g_dwc1right>=0)g_hdwc1x_LED=new TH1I("dwc1x_LED","dwc1x_LED",300,-15000,15000);
    if(g_dwc1up>=0   && g_dwc1down>=0 )g_hdwc1y_LED=new TH1I("dwc1y_LED","dwc1y_LED",300,-15000,15000);
    if(g_dwc2left>=0 && g_dwc2right>=0)g_hdwc2x_LED=new TH1I("dwc2x_LED","dwc2x_LED",300,-15000,15000);
    if(g_dwc2up>=0   && g_dwc2down>=0 )g_hdwc2y_LED=new TH1I("dwc2y_LED","dwc2y_LED",300,-15000,15000);
    if(g_dwc3left>=0 && g_dwc3right>=0)g_hdwc3x_LED=new TH1I("dwc3x_LED","dwc3x_LED",300,-15000,15000);
    if(g_dwc3up>=0   && g_dwc3down>=0 )g_hdwc3y_LED=new TH1I("dwc3y_LED","dwc3y_LED",300,-15000,15000);
    if(g_dwc4left>=0 && g_dwc4right>=0)g_hdwc4x_LED=new TH1I("dwc4x_LED","dwc4x_LED",300,-15000,15000);
    if(g_dwc4up>=0   && g_dwc4down>=0 )g_hdwc4y_LED=new TH1I("dwc4y_LED","dwc4y_LED",300,-15000,15000);
    gDirectory->cd("..");
    gDirectory->pwd();
    
    
    gDirectory->mkdir("PED");
    gDirectory->cd("PED");
    gDirectory->pwd();
    for(int ich=0; ich<g_rp.nchans; ++ich){
      if(1==g_rp.datatype[ich]){// ADC
        sprintf(nam,"%s_PEDADC%2.2d",&g_rp.chnam[ich][0],g_rp.datachan[ich]);
        sprintf(tit,"%s (ADC%2.2d) PED",&g_rp.chnam[ich][0],g_rp.datachan[ich]);
        g_hADC_PED[g_rp.datachan[ich]]=new TH1I(nam,tit,4096,0,4096);
      }
      else if(3==g_rp.datatype[ich]){// DIG
        sprintf(nam,"%s_PED_DIG%2.2dPED",&g_rp.chnam[ich][0],g_rp.datachan[ich]);
        sprintf(tit,"%s (DIG%2.2d) PED PED",&g_rp.chnam[ich][0],g_rp.datachan[ich]);
        g_hDIG_PEDPED[g_rp.datachan[ich]]=new TH1I(nam,tit,4096,0,4096);
        sprintf(nam,"%s_PED_DIG%2.2dMAX",&g_rp.chnam[ich][0],g_rp.datachan[ich]);
        sprintf(tit,"%s (DIG%2.2d) PED MAX",&g_rp.chnam[ich][0],g_rp.datachan[ich]);
        g_hDIG_PEDMAX[g_rp.datachan[ich]]=new TH1I(nam,tit,4096,0,4096);
      }
    }
    gDirectory->cd("..");
    gDirectory->pwd();

    gDirectory->mkdir("SIG");
    gDirectory->cd("SIG");
    gDirectory->pwd();
    for(int ich=0; ich<g_rp.nchans; ++ich){
      if(1==g_rp.datatype[ich]){// ADC
        sprintf(nam,"%s_SIGADC%2.2d",&g_rp.chnam[ich][0],g_rp.datachan[ich]);
        sprintf(tit,"%s (ADC%2.2d) SIG",&g_rp.chnam[ich][0],g_rp.datachan[ich]);
        g_hADC_SIG[g_rp.datachan[ich]]=new TH1I(nam,tit,4096,0,4096);
      }
      else if(3==g_rp.datatype[ich]){// DIG
        sprintf(nam,"%s_SIG_DIG%2.2dPED",&g_rp.chnam[ich][0],g_rp.datachan[ich]);
        sprintf(tit,"%s (DIG%2.2d) SIG PED",&g_rp.chnam[ich][0],g_rp.datachan[ich]);
        g_hDIG_SIGPED[g_rp.datachan[ich]]=new TH1I(nam,tit,4096,0,4096);
        sprintf(nam,"%s_SIG_DIG%2.2dMAX",&g_rp.chnam[ich][0],g_rp.datachan[ich]);
        sprintf(tit,"%s (DIG%2.2d) SIG MAX",&g_rp.chnam[ich][0],g_rp.datachan[ich]);
        g_hDIG_SIGMAX[g_rp.datachan[ich]]=new TH1I(nam,tit,4096,0,4096);
      }
    }
    if(g_dwc1left>=0 && g_dwc1right>=0)g_hdwc1x_SIG=new TH1I("dwc1x_SIG","dwc1x_SIG",300,-15000,15000);
    if(g_dwc1up>=0   && g_dwc1down>=0 )g_hdwc1y_SIG=new TH1I("dwc1y_SIG","dwc1y_SIG",300,-15000,15000);
    if(g_dwc2left>=0 && g_dwc2right>=0)g_hdwc2x_SIG=new TH1I("dwc2x_SIG","dwc2x_SIG",300,-15000,15000);
    if(g_dwc2up>=0   && g_dwc2down>=0 )g_hdwc2y_SIG=new TH1I("dwc2y_SIG","dwc2y_SIG",300,-15000,15000);
    if(g_dwc3left>=0 && g_dwc3right>=0)g_hdwc3x_SIG=new TH1I("dwc3x_SIG","dwc3x_SIG",300,-15000,15000);
    if(g_dwc3up>=0   && g_dwc3down>=0 )g_hdwc3y_SIG=new TH1I("dwc3y_SIG","dwc3y_SIG",300,-15000,15000);
    if(g_dwc4left>=0 && g_dwc4right>=0)g_hdwc4x_SIG=new TH1I("dwc4x_SIG","dwc4x_SIG",300,-15000,15000);
    if(g_dwc4up>=0   && g_dwc4down>=0 )g_hdwc4y_SIG=new TH1I("dwc4y_SIG","dwc4y_SIG",300,-15000,15000);
    gDirectory->cd("..");
    gDirectory->pwd();

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
        if(JCH>7)JCH+=1; // account for tr0 which is inserted as #8
        g_used742[JCH]=1;
        sprintf(nam,"%s_nd%2.2d",&g_rp.chnam[i][0],g_rp.datachan[i]);
        sprintf(fmt,"%s/I",nam);
        g_ROOTtree->Branch(nam,&g_nDT5742[JCH],fmt);
        sprintf(nam1,"%s_ad%2.2d",&g_rp.chnam[i][0],g_rp.datachan[i]);
        sprintf(fmt,"%s[%s]/F",nam1,nam);
        g_ROOTtree->Branch(nam1,&g_aDT5742[JCH][0],fmt);
      }
    }
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
    
    for(int i=0; i<NADCCHAN; ++i){
      if(g_hADC_LED[i]) g_hADC_LED[i]->Write();
      //else printf("%s: g_hADC_LED[%d]=0\n",__func__,i);
      if(g_hADC_PED[i]) g_hADC_PED[i]->Write();
      //else printf("%s: g_hADC_PED[%d]=0\n",__func__,i);
      if(g_hADC_SIG[i]) g_hADC_SIG[i]->Write();
      //else printf("%s: g_hADC_SIG[%d]=0\n",__func__,i);
    }
    
    if(g_hdwc1x_LED) g_hdwc1x_LED->Write(); 
    if(g_hdwc1y_LED) g_hdwc1y_LED->Write(); 
    if(g_hdwc2x_LED) g_hdwc2x_LED->Write(); 
    if(g_hdwc2y_LED) g_hdwc2y_LED->Write(); 
    if(g_hdwc3x_LED) g_hdwc3x_LED->Write(); 
    if(g_hdwc3y_LED) g_hdwc3y_LED->Write(); 
    if(g_hdwc4x_LED) g_hdwc4x_LED->Write(); 
    if(g_hdwc4y_LED) g_hdwc4y_LED->Write(); 
    if(g_hdwc1x_SIG) g_hdwc1x_SIG->Write(); 
    if(g_hdwc1y_SIG) g_hdwc1y_SIG->Write(); 
    if(g_hdwc2x_SIG) g_hdwc2x_SIG->Write(); 
    if(g_hdwc2y_SIG) g_hdwc2y_SIG->Write(); 
    if(g_hdwc3x_SIG) g_hdwc3x_SIG->Write(); 
    if(g_hdwc3y_SIG) g_hdwc3y_SIG->Write(); 
    if(g_hdwc4x_SIG) g_hdwc4x_SIG->Write(); 
    if(g_hdwc4y_SIG) g_hdwc4y_SIG->Write(); 
    
    for(int i=0; i<NDT5742CHAN; ++i){
      if(g_hDIG_LEDPED[i]) g_hDIG_LEDPED[i]->Write(); 
      if(g_hDIG_LEDMAX[i]) g_hDIG_LEDMAX[i]->Write(); 
      if(g_hDIG_PEDPED[i]) g_hDIG_PEDPED[i]->Write(); 
      if(g_hDIG_PEDMAX[i]) g_hDIG_PEDMAX[i]->Write(); 
      if(g_hDIG_SIGPED[i]) g_hDIG_SIGPED[i]->Write(); 
      if(g_hDIG_SIGMAX[i]) g_hDIG_SIGMAX[i]->Write(); 
    }
    
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

void fill_all(){
  if(g_ROOTfile && g_ROOTtree){
    TTimeStamp ttst;
    g_t=ttst.AsDouble()-g_t0;
    
    g_hpattern->Fill(g_pattern);
    
    for(int i=0; i<NADCCHAN; ++i){
      if(     g_rp.PEDpatt==g_pattern && 0!=g_hADC_PED[i]) g_hADC_PED[i]->Fill(g_ADC[i]);
      else if(g_rp.LEDpatt==g_pattern && 0!=g_hADC_LED[i]) g_hADC_LED[i]->Fill(g_ADC[i]);
      else if(g_rp.SIGpatt==g_pattern && 0!=g_hADC_SIG[i]) g_hADC_SIG[i]->Fill(g_ADC[i]);
    }
    
    for(int i=0; i<NDT5742CHAN; ++i){
      if(g_used742[i]){
        double dped=getped_742(g_nDT5742[i],g_evdata742[i]);
        double dmax=getmax_742(g_nDT5742[i],g_evdata742[i]);
        double dmin=getmin_742(g_nDT5742[i],g_evdata742[i]);
        if(g_rp.PEDpatt==g_pattern){
          if(g_hDIG_PEDPED[i])g_hDIG_PEDPED[i]->Fill(dped);
          if(g_hDIG_PEDMAX[i])g_hDIG_PEDPED[i]->Fill(dped-dmin);
        }
        else if(g_rp.LEDpatt==g_pattern){
          if(g_hDIG_LEDPED[i])g_hDIG_LEDPED[i]->Fill(dped);     
          if(g_hDIG_LEDMAX[i])g_hDIG_LEDPED[i]->Fill(dped-dmin);
        }
        else if(g_rp.SIGpatt==g_pattern){
          if(g_hDIG_SIGPED[i])g_hDIG_SIGPED[i]->Fill(dped);     
          if(g_hDIG_SIGMAX[i])g_hDIG_SIGPED[i]->Fill(dped-dmin);
        }
      }
    }
    
    if(g_dwc1left>=0 && g_dwc1right>=0){
      if(g_nTDC[g_dwc1left]>0 && g_nTDC[g_dwc1right]>0){
        if(g_rp.LEDpatt==g_pattern && 0!=g_hdwc1x_LED)g_hdwc1x_LED->Fill(g_tTDC[g_dwc1left][0]-g_tTDC[g_dwc1right][0]);
        else if(g_rp.SIGpatt==g_pattern && 0!=g_hdwc1x_SIG)g_hdwc1x_SIG->Fill(g_tTDC[g_dwc1left][0]-g_tTDC[g_dwc1right][0]);
      }
    }
    if(g_dwc2left>=0 && g_dwc2right>=0){
      if(g_nTDC[g_dwc2left]>0 && g_nTDC[g_dwc2right]>0){
        if(g_rp.LEDpatt==g_pattern && 0!=g_hdwc2x_LED)g_hdwc2x_LED->Fill(g_tTDC[g_dwc2left][0]-g_tTDC[g_dwc2right][0]);
        else if(g_rp.SIGpatt==g_pattern && 0!=g_hdwc2x_SIG)g_hdwc2x_SIG->Fill(g_tTDC[g_dwc2left][0]-g_tTDC[g_dwc2right][0]);
      }
    }
    if(g_dwc3left>=0 && g_dwc3right>=0){
      if(g_nTDC[g_dwc3left]>0 && g_nTDC[g_dwc3right]>0){
        if(g_rp.LEDpatt==g_pattern && 0!=g_hdwc3x_LED)g_hdwc3x_LED->Fill(g_tTDC[g_dwc3left][0]-g_tTDC[g_dwc3right][0]);
        else if(g_rp.SIGpatt==g_pattern && 0!=g_hdwc3x_SIG)g_hdwc3x_SIG->Fill(g_tTDC[g_dwc3left][0]-g_tTDC[g_dwc3right][0]);
      }
    }
    if(g_dwc4left>=0 && g_dwc4right>=0){
      if(g_nTDC[g_dwc4left]>0 && g_nTDC[g_dwc4right]>0){
        if(g_rp.LEDpatt==g_pattern && 0!=g_hdwc4x_LED)g_hdwc4x_LED->Fill(g_tTDC[g_dwc4left][0]-g_tTDC[g_dwc4right][0]);
        else if(g_rp.SIGpatt==g_pattern && 0!=g_hdwc4x_SIG)g_hdwc4x_SIG->Fill(g_tTDC[g_dwc4left][0]-g_tTDC[g_dwc4right][0]);
      }
    }
    
    if(g_dwc1up>=0 && g_dwc1down>=0){
      if(g_nTDC[g_dwc1up]>0 && g_nTDC[g_dwc1down]>0){
        if(g_rp.LEDpatt==g_pattern && 0!=g_hdwc1y_LED)g_hdwc1y_LED->Fill(g_tTDC[g_dwc1up][0]-g_tTDC[g_dwc1down][0]);
        else if(g_rp.SIGpatt==g_pattern && 0!=g_hdwc1y_SIG)g_hdwc1y_SIG->Fill(g_tTDC[g_dwc1up][0]-g_tTDC[g_dwc1down][0]);
      }
    }
    if(g_dwc2up>=0 && g_dwc2down>=0){
      if(g_nTDC[g_dwc2up]>0 && g_nTDC[g_dwc2down]>0){
        if(g_rp.LEDpatt==g_pattern && 0!=g_hdwc2y_LED)g_hdwc2y_LED->Fill(g_tTDC[g_dwc2up][0]-g_tTDC[g_dwc2down][0]);
        else if(g_rp.SIGpatt==g_pattern && 0!=g_hdwc2y_SIG)g_hdwc2y_SIG->Fill(g_tTDC[g_dwc2up][0]-g_tTDC[g_dwc2down][0]);
      }
    }
    if(g_dwc3up>=0 && g_dwc3down>=0){
      if(g_nTDC[g_dwc3up]>0 && g_nTDC[g_dwc3down]>0){
        if(g_rp.LEDpatt==g_pattern && 0!=g_hdwc3y_LED)g_hdwc3y_LED->Fill(g_tTDC[g_dwc3up][0]-g_tTDC[g_dwc3down][0]);
        else if(g_rp.SIGpatt==g_pattern && 0!=g_hdwc3y_SIG)g_hdwc3y_SIG->Fill(g_tTDC[g_dwc3up][0]-g_tTDC[g_dwc3down][0]);
      }
    }
    if(g_dwc4up>=0 && g_dwc4down>=0){
      if(g_nTDC[g_dwc4up]>0 && g_nTDC[g_dwc4down]>0){
        if(g_rp.LEDpatt==g_pattern && 0!=g_hdwc4y_LED)g_hdwc4y_LED->Fill(g_tTDC[g_dwc4up][0]-g_tTDC[g_dwc4down][0]);
        else if(g_rp.SIGpatt==g_pattern && 0!=g_hdwc4y_SIG)g_hdwc4y_SIG->Fill(g_tTDC[g_dwc4up][0]-g_tTDC[g_dwc4down][0]);
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

  for(int i=0; i<NDT5742CHAN; ++i){
    if(g_hDIG_LEDPED[i]) { g_hDIG_LEDPED[i]->Delete(); g_hDIG_LEDPED[i]=0; }
    if(g_hDIG_LEDMAX[i]) { g_hDIG_LEDMAX[i]->Delete(); g_hDIG_LEDMAX[i]=0; }
    if(g_hDIG_PEDPED[i]) { g_hDIG_PEDPED[i]->Delete(); g_hDIG_PEDPED[i]=0; }
    if(g_hDIG_PEDMAX[i]) { g_hDIG_PEDMAX[i]->Delete(); g_hDIG_PEDMAX[i]=0; }
    if(g_hDIG_SIGPED[i]) { g_hDIG_SIGPED[i]->Delete(); g_hDIG_SIGPED[i]=0; }
    if(g_hDIG_SIGMAX[i]) { g_hDIG_SIGMAX[i]->Delete(); g_hDIG_SIGMAX[i]=0; }
  }
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

  for(int i=0; i<NDT5742CHAN; ++i){
    if(g_hDIG_LEDPED[i]) g_hDIG_LEDPED[i]->Reset(); 
    if(g_hDIG_LEDMAX[i]) g_hDIG_LEDMAX[i]->Reset(); 
    if(g_hDIG_PEDPED[i]) g_hDIG_PEDPED[i]->Reset(); 
    if(g_hDIG_PEDMAX[i]) g_hDIG_PEDMAX[i]->Reset(); 
    if(g_hDIG_SIGPED[i]) g_hDIG_SIGPED[i]->Reset(); 
    if(g_hDIG_SIGMAX[i]) g_hDIG_SIGMAX[i]->Reset(); 
  }
}

