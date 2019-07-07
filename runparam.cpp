#pragma once

#include "runparam.h"


uint32_t VME_ADC1    =0xC30000;    // =0xC30000, ADC1, LECROY 1182
uint32_t VME_ADC2    =0;           // =0xC10000, ADC2, LECROY 1182
uint32_t VME_ADC3    =0;           // =0xC20000, ADC2, LECROY 1182
uint32_t VME_CORBO   =0xF00000;    // =0xF00000, CORBO = CES RCB 8047
uint32_t VME_CRB_CH  =0x000000;    // =0x000000, CORBO main channel
uint32_t VME_CRB_CH2 =0x000000;    // =0x000001, CORBO secondary channel (pulse gen etc)
uint32_t VME_V259    =0xC00000;    // =0xC00000, pattern unit
uint32_t VME_V1290   =0xCC0000;    // =0xCC0000 ,CAEN TDC with NIM inputs
uint32_t VME_V260    =0;           // =0x00DD00, CAEN scaler
uint32_t VME_V812    =0;           // =0x880000, CAEN V812 constant fraction discriminator #1
uint32_t VME_V812_2  =0;           // =0x990000, CAEN V812 constant fraction discriminator #2
uint32_t VME_CRB_IRQ =3;        // VME IRQ ised in CORBO
uint32_t VME_CRB_VEC =0x85;      // interrpt vector ised in CORBO


bool goodhex(char* s, uint32_t *u){
  char hsyms[]="0123456789ABCDEF";
  if(s[0]=='0' && s[1]=='x'){
    for(int i=2; i<strlen(s); ++i){
      if(!strchr(hsyms,s[i]))return false;
    }
    int nh=sscanf(s,"%x",u);
    if(1==nh)return true;
    else return false;
  }
  return false;
}

double limited(double z, double zmin, double zmax){
  if(z<zmin)return zmin;
  else if(z>zmax)return zmax;
  else return z;
}
  
runParam::runParam(){
  this->reset();
}
  
int runParam::findch(const char* nam){
  // remove trailing blanks
  char str[256];
  strncpy(str,nam,254);
  for(int i=strlen(str)-1; i>=0; --i){
    if(str[i]==' ')str[i]=0;
    else break;
  }
  
  for(int i=0; i<nchans; ++i){
    if(0==strcmp(str,&chnam[i][0]))return i;
  }
  return -1;
}

int runParam::findch(const char* typ, int ch){
  const char* typs[4]={"x","ADC","TDC","DIG"};
  int ityp=0;
  for(int j=0; j<4; ++j)    if(0==strcmp(typ,typs[j]))ityp=j;
  
  for(int i=0; i<nchans; ++i){
    if(ityp==datatype[i] && ch==datachan[i])return i;
  }
  return -1;
}

void runParam::reset(){
  init=false;
  
  printperiod=writeperiod=cmdperiod=updperiod=1; // in seconds!!!
  
  HVmaster=HVslave=0; // both illegal, HVmaster should be 1...4, HVslave 1...31
  
  PEDpatt=1;
  LEDpatt=2;
  SIGpatt=4;
  
  for(int i=0; i<MAXCHANS; ++i)HVchan[i]=-1;
  
  nLEDs=0;
  for(int ich=0; ich<sizeof(LEDchan)/sizeof(int); ++ich)LEDchan[ich]=-1; //  all illegal
  memset(ULED,0,sizeof(ULED));
  
  nchans=0;
  memset(chnam,0,sizeof(chnam));
  for(int ich=0; ich<sizeof(HVchan)/sizeof(int); ++ich)HVchan[ich]=-1; //  all illegal
  memset(datatype,0,sizeof(datatype));
  memset(datachan,0,sizeof(datachan));
  
  SPECS_used=false;
  ADC1_used = ADC2_used = ADC3_used = ADC_used = TDC_used =false;
  VME_ADC1 = VME_ADC2 = VME_ADC3 = 0;
  digitizer_used=false;
  
  cx1[0]=-7046.; cx1[1]=140.; cx1[2]=7329.;
  cy1[0]=-7119.; cy1[1]= 30.; cy1[2]=7157.;
  cx2[0]=-7005.; cx2[1]=  7.; cx2[2]=7064.;
  cy2[0]=-6929.; cy2[1]=170.; cy2[2]=7199.;
  cx3[0]=-6936.; cx3[1]= 66.; cx3[2]=7032.;
  cy3[0]=-6893.; cy3[1]= 86.; cy3[2]=6995.;
  cx4[0]=-7000.; cx4[1]=  0.; cx4[2]=7000.;
  cy4[0]=-7000.; cy4[1]=  0.; cy4[2]=7000.;
}
  
void runParam::setChanHV(char* nam, int ich, double v){
  int inamexist=-1, ichanexist=-1;
  for(int j=0; j<nchans; ++j){
    if(0==strcmp(nam,&chnam[j][0]))inamexist=j;
    if(ich==HVchan[j])ichanexist=j;
  }
  
  if(inamexist<0){ // new chan
    if(nchans>=MAXCHANS){
      printf("%s WARNING: too many channels, cannot add %s\n",__func__,nam);
      return;
    }
    if(ichanexist>=0)printf("%s WARNING: assigning existing HVchan %d to new name %s\n",__func__,ich,nam);
    strncpy(&chnam[nchans][0],nam,MAXNAMELENGTH-1);
    HVchan[nchans]=ich;
    HV[nchans]=v;
    nchans++;
  }
  else{// modify existing
    if(HVchan[inamexist]>=0)printf("%s WARNING: modifying existing: %s\n",__func__,nam);
    if(ichanexist>=0 && ichanexist!=inamexist)
      printf("%s WARNING: changing HVchan for %s to busy %d \n",__func__, nam,ich);
    HVchan[inamexist]=ich;
    HV[inamexist]=v;
  }
}

void runParam::setChanDataConn(char* nam, char* typ, int ich){
  const char* typs[4]={"x","ADC","TDC","DIG"};
  int ityp=0;
  for(int j=1; j<4; ++j)    if(0==strcmp(typ,typs[j]))ityp=j;
  if(ityp<=0){
    printf("%s WARNING wrong datatype: %s for %s\n",__func__,typ,nam);
    return;
  }

  int inamexist=-1, ichanexist=-1;
  for(int j=0; j<nchans; ++j){
    if(0==strcmp(nam,&chnam[j][0])) inamexist=j;
    if(ityp==datatype[j] && ich==datachan[j])ichanexist=j;
  }
  
  if(inamexist<0){ // new chan
    if(nchans>=MAXCHANS){
      printf("%s WARNING: too many channels, cannot add %s\n",__func__,nam);
      return;
    }
    if(ichanexist>=0)printf("%s WARNING: assigning busy data chan %s%2.2d to new name %s\n",__func__,typ,ich,nam);
    strncpy(&chnam[nchans][0],nam,MAXNAMELENGTH-1);
    datatype[nchans]=ityp;
    datachan[nchans]=ich;
    nchans++;
  }
  else{// modify existing
    if(datatype[inamexist]>0)printf("%s WARNING: modifying existing: %s\n",__func__,nam);
    if(ichanexist>=0 && ichanexist!=inamexist)
      printf("%s WARNING: changing data chan for %s to busy %s%2.2d\n",__func__,nam,typ,ich);
    datatype[inamexist]=ityp;
    datachan[inamexist]=ich;
  }
}
  
void runParam::setLED(int iLED, int ich, double v){
  if(iLED<0 || iLED>=MAXLEDS){
    printf("%s WARNING: wrong LED number %d\n",__func__,iLED);
    return;
  }
  
  if(LEDchan[iLED]>=0)printf("%s WARNING: modifying settings of LED %d\n",__func__,iLED);
  LEDchan[iLED]=ich;
  ULED[iLED]=v;
}
  
void runParam::write(const char* fnam){
  FILE* f=fopen(fnam,"w");
  if(!f)return;
  
  fprintf(f,"//SPECS part\n");
  fprintf(f,"HVMASTER %d\n",HVmaster);
  fprintf(f,"HVSLAVE %d\n\n",HVslave);
  
  fprintf(f,"\n// HVCHAN <name> <HV chan #> <HV value in kV>\n");
  for(int i=0; i<nchans; ++i){
    if(HVchan[i]>=0)fprintf(f,"HVCHAN %s %d %6.4f\n",&chnam[i][0],HVchan[i],HV[i]);
  }
  
  fprintf(f,"\n//Pattern Unit (V259) connections\n");
  fprintf(f,"PEDPATT %d\n",PEDpatt);
  fprintf(f,"LEDPATT %d\n",LEDpatt);
  fprintf(f,"SIGPATT %d\n",SIGpatt);
  fprintf(f,"PRINTPERIOD %f\n",printperiod);
  fprintf(f,"WRITEPERIOD %f\n",writeperiod);
  fprintf(f,"CMDPERIOD %f\n",cmdperiod);
  fprintf(f,"UPDPERIOD %f\n",updperiod);
  
  fprintf(f,"\n// LEDCHAN <iLED> <LED chan> <U_LED> \n");
  for(int i=0; i<sizeof(LEDchan)/sizeof(int); ++i){
    if(LEDchan[i]>=0)fprintf(f,"LEDCHAN %d %d %6.4f\n",i,LEDchan[i],ULED[i]);
  }
  
  if(VME_ADC1   >0)fprintf(f,"VME_ADC1    0x%8.8X\n",VME_ADC1   );
  if(VME_ADC2   >0)fprintf(f,"VME_ADC2    0x%8.8X\n",VME_ADC2   );
  if(VME_ADC3   >0)fprintf(f,"VME_ADC3    0x%8.8X\n",VME_ADC3   );
  if(VME_CORBO  >0)fprintf(f,"VME_CORBO   0x%8.8X\n",VME_CORBO  );
  fprintf(f,"VME_CRB_CH  0x%8.8X\n",VME_CRB_CH );
  if(VME_CRB_CH2!=VME_CRB_CH)fprintf(f,"VME_CRB_CH2 0x%8.8X\n",VME_CRB_CH2);
  if(VME_V259   >0)fprintf(f,"VME_V259    0x%8.8X\n",VME_V259   );
  if(VME_V1290  >0)fprintf(f,"VME_V1290   0x%8.8X\n",VME_V1290  );
  if(VME_V260   >0)fprintf(f,"VME_V260    0x%8.8X\n",VME_V260   );
  if(VME_V812   >0)fprintf(f,"VME_V812    0x%8.8X\n",VME_V812   );
  if(VME_V812_2 >0)fprintf(f,"VME_V812_2  0x%8.8X\n",VME_V812_2 );
  fprintf(f,"VME_CRB_VEC 0x%X\n",VME_CRB_VEC);
  fprintf(f,"VME_CRB_IRQ 0x%X\n",VME_CRB_IRQ);

  fprintf(f,"\n// DATACONN <name> <type> <chan> (NB ADCmodule = ADCchan/8; ADCinput=ADCchan%8)\n");
  const char* typs[4]={"x","ADC","TDC","DIG"};
  for(int i=0; i<nchans; ++i){
    if(datatype[i]>=0)fprintf(f,"DATACONN %s %s %d\n",&chnam[i][0],typs[datatype[i]],datachan[i]);
  }
  
  fclose(f);
}
  
void runParam::read(const char* fnam){
  FILE* f=fopen(fnam,"r");
  if(!f)return;
    
  char str[256], STR[256];
  while(str==fgets(str,255,f)){
    //for(int i=0; i<sizeof(STR)-1 && (STR[i]=toupper(str[i])); ++i){}
    memset(STR,0,sizeof(STR));
    for(int i=0; i<sizeof(STR)-1; ++i){
      if('/'==str[i] && '/'==str[i+1]) break;
      else if(0==str[i])break;
      else STR[i]=str[i];
    }
    char what[128]="", cn[128]="", cv[128]="", cz[128]="";
    int nit=sscanf(STR,"%s %s %s %s",&what[0],&cn[0],&cv[0],&cz[0]);
    if(nit>1){
      double n=0, v=0, z=0;
      uint32_t u;
      int nit1=sscanf(cn,"%lf",&n);
      int nit2=0; if(nit>2) nit2=sscanf(cv,"%lf",&v);
      int nit3=0; if(nit>3) nit3=sscanf(cz,"%lf",&z);
        
      if(0==strcmp(what,"HVMASTER")){
        if(1==nit1) HVmaster=(int)limited(n,1,4);
      }
      else if(0==strcmp(what,"HVSLAVE")){
        if(1==nit1) HVslave=(int)limited(n,1,31);
      }
      else if(0==strcmp(what,"HVCHAN")){
        if(nit>3 && nit2>0 &&nit3>0)setChanHV(cn,(int)limited(v,0,199),limited(z,0,2));
      }
      else if(0==strcmp(what,"PEDPATT")){
        if(nit>1 && nit1>0)PEDpatt=n;
      }
      else if(0==strcmp(what,"LEDPATT")){
        if(nit>1 && nit1>0)LEDpatt=n;
      }
      else if(0==strcmp(what,"SIGPATT")){
        if(nit>1 && nit1>0)SIGpatt=n;
      }
      else if(0==strcmp(what,"LEDCHAN")){
        if(nit>3 && nit1>0 && nit2>0 &&nit3>0)setLED(n,(int)limited(v,0,215),limited(z,0,5));
      }
      else if(0==strcmp(what,"DATACONN")){
        if(nit>3 >> nit3>0)setChanDataConn(cn,cv,(int)z);
      }
      else if(0==strcmp(what,"VME_ADC1")){
        if(nit>1 && goodhex(cn,&u))VME_ADC1=u;
      }
      else if(0==strcmp(what,"VME_ADC2")){
        if(nit>1 && goodhex(cn,&u))VME_ADC2=u;
      }
      else if(0==strcmp(what,"VME_ADC3")){
        if(nit>1 && goodhex(cn,&u))VME_ADC3=u;
      }
      else if(0==strcmp(what,"VME_CORBO")){
        if(nit>1 && goodhex(cn,&u))VME_CORBO=u;
      }
      else if(0==strcmp(what,"VME_CRB_CH")){
        if(nit>1 && goodhex(cn,&u))VME_CRB_CH=u;
      }
      else if(0==strcmp(what,"VME_CRB_CH2")){
        if(nit>1 && goodhex(cn,&u))VME_CRB_CH2=u;
      }
      else if(0==strcmp(what,"VME_V259")){
        if(nit>1 && goodhex(cn,&u))VME_V259=u;
      }
      else if(0==strcmp(what,"VME_V1290")){
        if(nit>1 && goodhex(cn,&u))VME_V1290=u;
      }
      else if(0==strcmp(what,"VME_V260")){
        if(nit>1 && goodhex(cn,&u))VME_V260=u;
      }
      else if(0==strcmp(what,"VME_V812")){
        if(nit>1 && goodhex(cn,&u))VME_V812=u;
      }
      else if(0==strcmp(what,"VME_V812_2")){
        if(nit>1 && goodhex(cn,&u))VME_V812_2=u;
      }
      else if(0==strcmp(what,"VME_CRB_VEC")){
        if(nit>1 && goodhex(cn,&u))VME_CRB_VEC=u&0xFF;
      }
      else if(0==strcmp(what,"VME_CRB_IRQ")){
        if(nit>1 &&nit1>1){
          if(goodhex(cn,&u))VME_CRB_IRQ=u;
          else VME_CRB_IRQ=n;
          VME_CRB_IRQ &= 0x17;
        }
      }
      else if(0==strcmp(what,"PRINTPERIOD")){
        if(nit>1 && nit1>0)printperiod=n;
      }
      else if(0==strcmp(what,"WRITEPERIOD")){
        if(nit>1 && nit1>0)writeperiod=n;
      }
      else if(0==strcmp(what,"CMDPERIOD")){
        if(nit>1 && nit1>0)cmdperiod=n;
      }
      else if(0==strcmp(what,"UPDPERIOD")){
        if(nit>1 && nit1>0)updperiod=n;
      }
      else if(0==strcmp(what,"DWC1XPAR")){
        if(nit>1 && nit1>0 && nit2>0 && nit3>0) {cx1[0]=n; cx1[1]=v; cx1[2]=z;}
      }
      else if(0==strcmp(what,"DWC2XPAR")){
        if(nit>1 && nit1>0 && nit2>0 && nit3>0) {cx2[0]=n; cx2[1]=v; cx2[2]=z;}
      }
      else if(0==strcmp(what,"DWC3XPAR")){
        if(nit>1 && nit1>0 && nit2>0 && nit3>0) {cx3[0]=n; cx3[1]=v; cx3[2]=z;}
      }
      else if(0==strcmp(what,"DWC4XPAR")){
        if(nit>1 && nit1>0 && nit2>0 && nit3>0) {cx4[0]=n; cx4[1]=v; cx4[2]=z;}
      }
      else if(0==strcmp(what,"DWC1YPAR")){
        if(nit>1 && nit1>0 && nit2>0 && nit3>0) {cy1[0]=n; cy1[1]=v; cy1[2]=z;}
      }
      else if(0==strcmp(what,"DWC2YPAR")){
        if(nit>1 && nit1>0 && nit2>0 && nit3>0) {cy2[0]=n; cy2[1]=v; cy2[2]=z;}
      }
      else if(0==strcmp(what,"DWC3YPAR")){
        if(nit>1 && nit1>0 && nit2>0 && nit3>0) {cy3[0]=n; cy3[1]=v; cy3[2]=z;}
      }
      else if(0==strcmp(what,"DWC4YPAR")){
        if(nit>1 && nit1>0 && nit2>0 && nit3>0) {cy4[0]=n; cy4[1]=v; cy4[2]=z;}
      }
    }
  }
  fclose(f);
  
  // now copy vme variables
  vme_adc1=    VME_ADC1;    
  vme_adc2=    VME_ADC2;    
  vme_adc3=    VME_ADC3;    
  vme_corbo=   VME_CORBO;   
  vme_crb_ch=  VME_CRB_CH;  
  vme_crb_ch2= VME_CRB_CH2; 
  vme_v259=    VME_V259;    
  vme_v1290=   VME_V1290;   
  vme_v260=    VME_V260;    
  vme_v812=    VME_V812;    
  vme_v812_2=  VME_V812_2;  
  vme_crb_irq= VME_CRB_IRQ; 
  vme_crb_vec= VME_CRB_VEC; 
  
  // check whether the run parameters make sense: 
  // at least one PMT name is present, and its HV >0
  if(0!=nchans)init=true;
  //printf("nchans=%d\n",nchans);
  
  // now inventory: what is used
  for(int i=0; i<sizeof(LEDchan)/sizeof(int); ++i){  // 1) SPECS
    if(LEDchan[i]>=0)SPECS_used=true;
  }
  for(int ich=0; ich<nchans; ++ich){  // 2) ADC
    if(HVchan[ich]>=0)SPECS_used=true;
    if(1==datatype[ich]){// 2) ADC
      if(datachan[ich]>=0 && datachan[ich]<8)ADC1_used=true;
      else if(datachan[ich]>=8 && datachan[ich]<16)ADC2_used=true;
      else if(datachan[ich]>=16 && datachan[ich]<24)ADC3_used=true;
      else printf("%s WARNING: %s bad ADC channel number %d\n",__func__,&chnam[ich][0],datachan[ich]);
        
    }
    else if (2==datatype[ich]){// 3) TDC
      if(datachan[ich]>=0 && datachan[ich]<32)TDC_used=true;
      else printf("%s WARNING: %s bad TDC channel number %d\n",__func__,&chnam[ich][0],datachan[ich]);
    }
    else if(3==datatype[ich]){// 4) digitizer
      if( (datachan[ich]>=0 && datachan[ich]<16) || 20==datachan[ich] || 21==datachan[ich])digitizer_used=true;
      else printf("%s WARNING: %s bad digitizer channel number %d\n",__func__,&chnam[ich][0],datachan[ich]);
    }
  }
  if(ADC1_used || ADC2_used || ADC3_used) ADC_used=true;
}
