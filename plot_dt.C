#include "TROOT.h"

void time_range(TGraph* tg, double lev, double& t1, double& t2){// suppose points are ordered in x
  t1=-9999;
  t2=-9999;
  if(!tg)return;
  if(lev<0.01 || lev>0.99)return;
    
  int np=tg->GetN();  
  double* xd=tg->GetX();
  double* yd=tg->GetY();
  
  float x[30000],y[30000];
  for(int i=0; i<np; ++i){
    x[i]=xd[i];
    y[i]=yd[i];
  }

  time_lev(np,x,y,lev,t1,t2);
}

void time_lev(int np, float* x, float* y, double lev, double& t1){// suppose points are ordered in x
  t1=-9999;
  if(lev<0.01 || lev>0.99)return;
    
  double ymin=999999, ymax=-999999;
  for(int i=0; i<np; ++i){
    if(ymin>y[i])ymin=y[i];
    if(ymax<y[i])ymax=y[i];
  }
  if(ymax<=ymin) return;
  if(ymax<0 || ymin<0 || ymax>4095 || ymin>4095) return;
  
  double lvl = ymax - lev*(ymax-ymin);
  
  int i1=-1;
  for(int i=0; i<np-1; ++i){
    if(i1<0 && y[i]>lvl && y[i+1]<lvl)i1=i;
  }
  
  if(i1>=0){
    t1=x[i1]+(x[i1+1]-x[i1])*(y[i1]-lvl)/(y[i1]-y[i1+1]);
    double t_max=x[i1]+(x[i1+1]-x[i1])*(y[i1]-ymax)/(y[i1]-y[i1+1]);
    double s0=0, s1=0, s2=0;
    for(int i=0; i<i1; ++i){
      if(x[i]>(t_max-100) && x[i]<t_max){
        s0+=1;
        s1+=y[i];
        s2+=y[i]*y[i];
      }
    }
    if(s0>10){
      lvl = s1/s0 - lev*(s1/s0-ymin);
      i1=-1;
      for(int i=0; i<np-1; ++i){
        if(i1<0 && y[i]>lvl && y[i+1]<lvl)i1=i;
      }
      t1=x[i1]+(x[i1+1]-x[i1])*(y[i1]-lvl)/(y[i1]-y[i1+1]);
    }
  }
}

int plot_dt(double level=0.5)
{
  TH1I* hdt=(TH1I*)gDirectory->FindObjectAny("dt");
  if(hdt)hdt->Delete();
  hdt=new TH1I("dt","dt",5000,380,430);
  
  int patt;
  double xx[1200],yy[1200];
  float x[1200],y[1200],ytrig[1200];
  for(int j=0; j<1024; ++j)x[j]=j;
  
  DATA->SetBranchAddress("pattern",&patt);
  DATA->SetBranchAddress("mcp1_ad00",y);
  DATA->SetBranchAddress("tr0dig0_ad20",ytrig);
  
  int nev=DATA->GetEntries();
  for(int iev=0; iev<nev; ++iev){
    if(0==iev%1000)printf("%6d\n",iev);
    //printf("%6d\r",iev);
    DATA->GetEvent(iev);
    if(1==patt){
      double tsig,ttrig;
      time_lev(1024,x,y,level,tsig);
      time_lev(1024,x,ytrig,level,ttrig);
      if(tsig>0 && ttrig>0){
        hdt->Fill(tsig-ttrig);
      }
    }
  }
  printf("Total of %d events\n",nev);
  
  TCanvas* cdt=(TCanvas*)gROOT->FindObjectAny("cdt");
  if(!cdt){
    cdt=new TCanvas("cdt","cdt",600,600);
  }
  cdt->cd();
  dt->Draw();
  
  return nev;
}

static jevt=0;

int plot_ev(int iev=-1)
{
  if(iev<0)iev=jevt;
  jevt+=1;
  char nam[128];
  sprintf(nam,"Event %6d\n",iev);
  
  int nev=DATA->GetEntries();
  if(iev<0)iev=0;
  if(iev>nev-1)iev=nev-1;
  
  double xx[1200],yy[1200];
  float y[1200],ytrig[1200];
  for(int j=0; j<1024; ++j)xx[j]=j;
  
  DATA->SetBranchAddress("mcp1_ad00",y);
  DATA->SetBranchAddress("tr0dig0_ad20",ytrig);
  
  DATA->GetEvent(iev);
  
  for(int j=0; j<1024; ++j)yy[j]=y[j];
  TGraph* g=new TGraph(1024,xx,yy);
  g->SetTitle(nam);
  g->SetBit(kCanDelete);
  for(int j=0; j<1024; ++j)yy[j]=ytrig[j];
  TGraph* gtrig=new TGraph(1024,xx,yy);
  gtrig->SetTitle(nam);
  gtrig->SetBit(kCanDelete);
  
  TCanvas* cev=(TCanvas*)gROOT->FindObjectAny("cev");
  if(!cev){
    cev=new TCanvas("cev","cev",600,600);
    cev->Divide(1,2);
  }
  cev->cd(1);
  g->Draw("ALP");
  cev->cd(2);
  gtrig->Draw("ALP");
  
  return 0;
}
