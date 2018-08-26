#pragma once

#include <cstring>
#include "TH1D.h"
#include "TH1F.h"
#include "TH1I.h"
#include "TProfile.h"

typedef struct dimHist{
  int nbins, ndummy;
  double xmin, xmax;
  double nentries,mean,rms;
  char fmat[64];
  char name[256];
  char title[256];
  double cont[5000];
  
  dimHist(){
    this->Reset();
  }
  
  void Reset(){
    nbins=0;
    ndummy=0xAAAA;
    xmin=xmax=nentries=mean=rms=0;
    memset(fmat,0,sizeof(fmat));
    strncpy(fmat,"I:1;D:5;C:64;C:256;C:256;D:5000",63);
    memset(name,0,sizeof(name));
    memset(title,0,sizeof(title));
    memset(cont,0,sizeof(cont));
  };
  
  bool isUsed(){
    if(strlen(name)==0)return false;
    else return true;
  }
  
  bool fill_TH1D(TH1D*& h){
    if(h){
      bool to_delete=false;
      int nbins_h=h->GetXaxis()->GetNbins();
      const char* name_h=h->GetName();
      if(nbins_h!=nbins)to_delete=true;
      if(0!=strcmp(name,name_h))to_delete=true;
      if(to_delete){h->Delete(); h=0;}
    }
    bool created=false;
    if(!h){
      h=new TH1D(name,title,nbins,xmin,xmax);
      created=true;
    }
    for(int i=0; i<nbins+2; ++i)h->SetBinContent(i,cont[i]);
    h->SetEntries(nentries);
    return created;
  }
} DIMHIST;


template <class HIST> void fill_dimHist(dimHist *d, HIST *h){
  int maxBins=4096;
  
  d->nbins=h->GetXaxis()->GetNbins();
  d->xmin=h->GetXaxis()->GetXmin();
  d->xmax=h->GetXaxis()->GetXmax();
  d->nentries=h->GetEntries();
  d->mean=h->GetMean();
  d->rms=h->GetRMS();
  
  memset(d->name,0,sizeof(d->name));
  strncpy(d->name,h->GetName(),250);
  strncpy(d->title,h->GetTitle(),250);
  d->title[250]=d->title[251]=0;
  
  double binWidth=(d->xmax-d->xmin)/d->nbins;
  if(d->nbins>maxBins){
    d->nbins=maxBins;
    d->xmax=d->xmin+binWidth*d->nbins;
  }
  for(int i=0; i<d->nbins+2; ++i)d->cont[i]=h->GetBinContent(i);
  
  //sprintf(d->fmat,"I:1;D:5;C:64;C:256;C:256;D:%d",d->nbins+2);
}

typedef struct dimStat{
  int irun,running,  ievt,nsig,nled,nped;
  double starttime, runtime;
  char fmat[64];
  char conf[64];
  char fnam[256];
  
  dimStat(){
    this->Reset();
  }
  
  void Reset(){
    irun=running=ievt=nsig=nled=nped=0;
    starttime=runtime=0;
    memset(fmat,0,sizeof(fmat));
    memset(conf,0,sizeof(conf));
    memset(fnam,0,sizeof(fnam));
    strncpy(fmat,"I:6;D:2;C:64;C:64;C:256",63);
  };
  
} DIMSTAT;

int readline_strings(char* str, char separ, int nmax, char** s){// reads separated strings
  int nch=strlen(str);
  
  for(int ich=0; ich<nch; ++ich){
    if(str[ich]==separ)str[ich]=0;
    if((int)str[ich]<(int)32)str[ich]=0;
  }
  
  s[0]=&str[0];
  int nstr=1;
  for(int ich=1; ich<nch; ++ich){
    if( str[ich-1]==0 ){
      s[nstr]=&str[ich];
      ++nstr;
      if(nstr>=nmax)break;
    }
  }
  
  return nstr;
}

typedef struct dimSummary{
  int nbins, ndummy;
  double xmin, xmax;
  double nentries,mean,rms;
  char fmat[64];
  char name[256], title[256];
  double nent[50], cont[50], erro[50];
  char binlabels[2048];
  
  dimSummary(){
    this->Reset();
  }
  
  void Reset(){
    nbins=0;
    ndummy=0xAAAA;
    xmin=xmax=nentries=mean=rms=0;
    memset(fmat,0,sizeof(fmat));
    strncpy(fmat,"I:1;D:5;C:64;C:256;C:256;D:50;D:50;D:50;C:2048",63);
    memset(name,0,sizeof(name));
    memset(title,0,sizeof(title));
    memset(nent,0,sizeof(nent));
    memset(cont,0,sizeof(cont));
    memset(erro,0,sizeof(erro));
    memset(binlabels,0,sizeof(binlabels));
  };
  
  bool isUsed(){
    if(strlen(name)==0)return false;
    else return true;
  }
  
  bool fill_TProfile(TProfile*& h){
    if(h){
      bool to_delete=false;
      int nbins_h=h->GetXaxis()->GetNbins();
      const char* name_h=h->GetName();
      if(nbins_h!=nbins)to_delete=true;
      if(0!=strcmp(name,name_h))to_delete=true;
      if(to_delete){h->Delete(); h=0;}
    }
    
    char* s[100];
    //printf("%s: labels: %s\n",__func__,binlabels);
    char str[2048]; memset(str,0,sizeof(str));
    strcpy(str,binlabels);
    int nlab=readline_strings(str, ',', nbins+2, s);
    bool created=false;
    if(!h){
      h=new TProfile(name,title,nbins,xmin,xmax,-1e10,1e10,"S");
      created=true;
    }
    
    for(int i=0; i<nbins+2; ++i){
      h->SetBinContent(i,cont[i]);
      if(i>0 && i<=nbins){
        h->SetBinEntries(i,nent[i]);
        h->SetBinError(i,erro[i]);
        size_t ls=strlen(s[i]);
        if(ls>0) h->GetXaxis()->SetBinLabel(i,s[i]);
      }
    }
    return created;
  }

  bool fill_TH1D(TH1D*& h){
    if(h){
      bool to_delete=false;
      int nbins_h=h->GetXaxis()->GetNbins();
      const char* name_h=h->GetName();
      if(nbins_h!=nbins)to_delete=true;
      if(0!=strcmp(name,name_h))to_delete=true;
      if(to_delete){h->Delete(); h=0;}
    }
    
    char* s[100];
    //printf("%s: labels: %s\n",__func__,binlabels);
    char str[2048]; memset(str,0,sizeof(str));
    strcpy(str,binlabels);
    int nlab=readline_strings(str, ',', nbins+2, s);
    bool created=false;
    if(!h){
      h=new TH1D(name,title,nbins,xmin,xmax);
      created=true;
    }
    
    for(int i=0; i<nbins+2; ++i){
      h->SetBinContent(i,cont[i]);
      if(i>0 && i<=nbins){
        h->SetBinError(i,erro[i]);
        size_t ls=strlen(s[i]);
        if(ls>0) h->GetXaxis()->SetBinLabel(i,s[i]);
      }
    }
    h->SetEntries(nentries);
    return created;
  }

  void fill_dimSummary(TProfile *h){
    int maxBins=50;
    
    nbins=h->GetXaxis()->GetNbins();
    xmin=h->GetXaxis()->GetXmin();
    xmax=h->GetXaxis()->GetXmax();
    nentries=h->GetEntries();
    mean=h->GetMean();
    rms=h->GetRMS();
    
    strncpy(name,h->GetName(),250);
    name[250]=name[251]=0;
    strncpy(title,h->GetTitle(),250);
    title[250]=title[251]=0;
    
    double binWidth=(xmax-xmin)/nbins;
    if(nbins>maxBins){
      nbins=maxBins;
      xmax=xmin+binWidth*nbins;
    }
    memset(binlabels,0,sizeof(binlabels));
    for(int i=0; i<nbins+2; ++i){
      cont[i]=h->GetBinContent(i);
      if(i>0 && i<=nbins){
        nent[i]=h->GetBinEntries(i);
        erro[i]=h->GetBinError(i);
        strcat(binlabels,",");
        strcat(binlabels,h->GetXaxis()->GetBinLabel(i));
      }
    }
    strcat(binlabels,",,,");
    //sprintf(fmat,"I:1;D:5;C:64;C:256;C:256;D:50;D:50;D:50;C:%d",strlen(binlabels));
    //printf("%s: labels: %s\n",__func__,binlabels);
  }
} DIMSUMMARY;

