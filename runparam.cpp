#pragma once

#include "runparam.h"


uint32_t VME_ADC1    =0xC30000;    // =0xC30000, ADC1, LECROY 1182
uint32_t VME_ADC2    =0;           // =0xC10000, ADC2, LECROY 1182
uint32_t VME_ADC3    =0;           // =0xC20000, ADC2, LECROY 1182
uint32_t VME_CORBO   =0xF00000;    // =0xF00000, CORBO = CES RCB 8047
uint32_t VME_CRB_CH  =0x000000;    // =0x000000, CORBO main channel
uint32_t VME_CRB_CH2 =0x000000;    // =0x000001, CORBO secondary channel (pulse gen etc)
uint32_t VME_CRB_CH3 =0x000002;    // =0x000002, CORBO channel used for a counter
uint32_t VME_CRB_CH4 =0x000003;    // =0x000003, CORBO channel used for a counter
uint32_t VME_CRB_IRQ =3;        // VME IRQ ised in CORBO
uint32_t VME_CRB_VEC =0x85;      // interrpt vector ised in CORBO
uint32_t VME_V259    =0xC00000;    // =0xC00000, pattern unit
uint32_t VME_V1290   =0xCC0000;    // =0xCC0000 ,CAEN TDC with NIM inputs
uint32_t VME_V260    =0;           // =0x00DD00, CAEN scaler
uint32_t VME_V812    =0;           // =0x880000, CAEN V812 constant fraction discriminator #1
uint32_t VME_V812_2  =0;           // =0x990000, CAEN V812 constant fraction discriminator #2


//     |    inputs         inputs         inputs           inputs         inputs         inputs        | triggers
//     |-----------------------------------------------------------------------------------------------------------
//   i | 0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31|32 33 34 35
//     |-----------------------------------------------------------------------------------------------------------
// JCH | 0  1  2  3  4  5  6  7  9 10 11 12 13 14 15 16 18 19 20 21 22 23 24 25 27 28 29 30 31 32 33 34| 8 17 26 35
//     |-----------------------------------------------------------------------------------------------------------
//   i |36 37 38 39 40 41 42 43 44 45 46 47 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 64 65 66 67|68 69 70 71
//     |-----------------------------------------------------------------------------------------------------------
// JCH |36 37 38 39 40 41 42 43 45 46 47 48 49 50 51 52 54 55 56 57 58 59 60 61 63 64 65 66 67 68 69 70|44 53 62 71

int i2JCH(int i){// i is visible input#, JCH is the internal one
  int JCH=0;
  if(i>=0&&i<TR0DIG0CHAN)JCH=i+i/8;
  else if(i>=TR0DIG0CHAN && i<N742CHAN) JCH=(i-TR0DIG0CHAN)*9+8;
  else if(i>=N742CHAN && i<N742CHAN+TR0DIG0CHAN) JCH=i+(i-N742CHAN)/8;
  else if(i>=N742CHAN+TR0DIG0CHAN && i<2*N742CHAN) JCH=(i-N742CHAN-TR0DIG0CHAN)*9+N742CHAN+8;
  return JCH;
};

int JCH2i(int JCH){// i is visible input#, JCH is the internal one
  int i=0;
  if(JCH>=0 && JCH<N742CHAN){
    if(8==JCH%9)i=TR0DIG0CHAN+JCH/9;
    else i=JCH-JCH/9;
  }
  else if(JCH>=N742CHAN && JCH<2*N742CHAN){
    if(8==JCH%9)i=N742CHAN+TR0DIG0CHAN+(JCH-N742CHAN)/9;
    else i=JCH-(JCH-N742CHAN)/9;
  }
  return i;
};

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
  
  write_ntp=1; // the default is to write a root tree with data into the root file
  write_bin=0; // the default is NOT to write a binary file with data
  
  PEDpatt=1;
  LEDpatt=2;
  SIGpatt=4;
  PEDperiod=LEDperiod=-1; // in seconds!!! negative==do not generate
  
  nLEDs=0;
  for(int ich=0; ich<sizeof(LEDchan)/sizeof(int); ++ich)LEDchan[ich]=-1; //  all illegal
  memset(ULED,0,sizeof(ULED));
  
  vme_adc1    =0xC30000;    // =0xC30000, ADC1, LECROY 1182
  vme_adc2    =0;           // =0xC10000, ADC2, LECROY 1182
  vme_adc3    =0;           // =0xC20000, ADC2, LECROY 1182
  vme_corbo   =0xF00000;    // =0xF00000, CORBO = CES RCB 8047
  vme_crb_ch  =0x000000;    // =0x000000, CORBO main channel
  vme_crb_ch2 =0x000001;    // =0x000001, CORBO secondary channel (pulse gen etc)
  vme_crb_ch3 =0x000002;    // =0x000002, CORBO channel used for a counter
  vme_crb_ch4 =0x000003;    // =0x000003, CORBO channel used for a counter
  vme_crb_irq =3;           // =3,    VME IRQ ised in CORBO
  vme_crb_vec =0x85;        // =0x85, interrpt vector ised in CORBO
  vme_v259    =0xC00000;    // =0xC00000, pattern unit
  vme_v1290   =0xCC0000;    // =0xCC0000, CAEN TDC with NIM inputs
  vme_v260    =0x00DD00;    // =0xC00000, CAEN scaler
  vme_v812    =0;           // =0x880000, CAEN V812 constant fraction discriminator #1
  vme_v812_2  =0;           // =0x990000, CAEN V812 constant fraction discriminator #2
  
  vme_conetnode=0;
  dig_conetnode=1;
  dig2_conetnode=-1; // not connected by default

  nHVchans=0;
  memset(HVIP,0,sizeof(HVIP));
  memset(HVname,0,sizeof(HVname));
  for(int ich=0; ich<sizeof(HVchan)/sizeof(int); ++ich)HVchan[ich]=-1; //  all illegal
  memset(HV,0,sizeof(HV));
  nchans=0;
  memset(chnam,0,sizeof(chnam));
  memset(datatype,0,sizeof(datatype));
  memset(datachan,0,sizeof(datachan));
  memset(polarity,0,sizeof(polarity));
  dig_PED_summ=1; // DIG PED summary plot: 0 -> AMP, otherwise PED
  dig_adjust_offsets=dig2_adjust_offsets=0;
  dig_use_correction=dig2_use_correction=1;
  dig_posttrigger=dig2_posttrigger=5;
  dig_frequency=dig2_frequency=0; // The digitizer freq code: 0->5GHz, 1->2.5GHz, 2->1GHz, 3->0.75GHz
  memset(dig_calib_path,0,sizeof(dig_calib_path));
  memset(dig2_calib_path,0,sizeof(dig2_calib_path));
  
  ADC1_used = ADC2_used = ADC3_used = ADC_used = TDC_used =false;
  digitizer_used=digitizer2_used=false;
  memset(used742,0,sizeof(used742));
  
  evbuflen=0;
  memset(evoffset,0,sizeof(evoffset));
  
  cx1[0]=-7046.; cx1[1]=140.; cx1[2]=7329.;
  cy1[0]=-7119.; cy1[1]= 30.; cy1[2]=7157.;
  cx2[0]=-7005.; cx2[1]=  7.; cx2[2]=7064.;
  cy2[0]=-6929.; cy2[1]=170.; cy2[2]=7199.;
  cx3[0]=-6936.; cx3[1]= 66.; cx3[2]=7032.;
  cy3[0]=-6893.; cy3[1]= 86.; cy3[2]=6995.;
  cx4[0]=-7000.; cx4[1]=  0.; cx4[2]=7000.;
  cy4[0]=-7000.; cy4[1]=  0.; cy4[2]=7000.;
  
  //--- resetting the digitizer calibration coefficients of Vincenzo ------
  memset(dig_p0,0,sizeof(dig_p0)); // coefficients for cell calibration
  memset(dig_p1,0,sizeof(dig_p1)); // coefficients for cell calibration
  memset(dig_p2,0,sizeof(dig_p2)); // coefficients for cell calibration
  memset(dig_pa0,0,sizeof(dig_pa0)); // coefficients for sample calibration
  memset(dig_pa1,0,sizeof(dig_pa1)); // coefficients for sample calibration
  memset(dig_pa2,0,sizeof(dig_pa2)); // coefficients for sample calibration
  memset(dig_timev,0,sizeof(dig_timev));                 // cell time widths
  memset(dig_calibs_inited,0,sizeof(dig_calibs_inited)); // initialized or not
}
  
void runParam::setChanHV(char* nam, int ich, double v){
  int inamexist=-1, ichanexist=-1;
  for(int j=0; j<nHVchans; ++j){
    if(0==strcmp(nam,&HVname[j][0]))inamexist=j;
    if(ich==HVchan[j])ichanexist=j;
  }
  
  if(inamexist<0){ // new chan
    if(nHVchans>=MAXCHANS){
      printf("%s WARNING: too many channels, cannot add %s\n",__func__,nam);
      return;
    }
    if(ichanexist>=0)printf("%s WARNING: assigning existing HVchan %d to new name %s\n",__func__,ich,nam);
    strncpy(&HVname[nHVchans][0],nam,MAXNAMELENGTH-1);
    HVchan[nHVchans]=ich;
    HV[nHVchans]=v;
    nHVchans++;
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
  
  int pol=0;
  int sl=strlen(nam);
  if('P'==nam[sl-2] && 'Z'==nam[sl-1]){
    if(3==ityp)printf(" INFO %s: positive polarity requested for DIG %2.2d %s\n",__func__,ich,nam);
    pol=777;
  }
  if('B'==nam[sl-2] && 'P'==nam[sl-1]){
    if(3==ityp)printf(" INFO %s: bipolar signal requested for DIG %2.2d %s\n",__func__,ich,nam);
    pol=222;
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
    polarity[nchans]=pol;
    nchans++;
  }
  else{// modify existing
    if(datatype[inamexist]>0)printf("%s WARNING: modifying existing: %s\n",__func__,nam);
    if(ichanexist>=0 && ichanexist!=inamexist)
      printf("%s WARNING: changing data chan for %s to busy %s%2.2d\n",__func__,nam,typ,ich);
    datatype[inamexist]=ityp;
    datachan[inamexist]=ich;
    polarity[inamexist]=pol;
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
  
void runParam::writestream(FILE* f){
  if(strlen(HVIP)>2)fprintf(f,"HVIP %s \n",&HVIP[0]);
  fprintf(f,"\n// HVCHAN <name> <HV chan #> <HV value in kV>\n");
  for(int i=0; i<nHVchans; ++i){
    if(HVchan[i]>=0)fprintf(f,"HVCHAN %s %d %6.4f\n",&HVname[i][0],HVchan[i],HV[i]);
  }
  
  fprintf(f,"\n// LEDCHAN <iLED> <LED chan> <U_LED> \n");
  for(int i=0; i<sizeof(LEDchan)/sizeof(int); ++i){
    if(LEDchan[i]>=0)fprintf(f,"LEDCHAN %d %d %6.4f\n",i,LEDchan[i],ULED[i]);
  }
  
  fprintf(f,"\n// CONET nodes for VME, DIG and DIG2. VME should be always 0 (first node)\n");
  fprintf(f,"VME_CONETNODE %d\n",vme_conetnode);
  fprintf(f,"DIG_CONETNODE %d\n",dig_conetnode);
  fprintf(f,"DIG2_CONETNODE %d\n",dig2_conetnode);
  
  fprintf(f,"\n// VME addresses. Must be in HEX \n");
  fprintf(f,"VME_ADC1    0x%8.8X\n",vme_adc1   );
  fprintf(f,"VME_ADC2    0x%8.8X\n",vme_adc2   );
  fprintf(f,"VME_ADC3    0x%8.8X\n",vme_adc3   );
  fprintf(f,"VME_CORBO   0x%8.8X\n",vme_corbo  );
  fprintf(f,"VME_CRB_CH  0x%X\n",vme_crb_ch );
  if(vme_crb_ch2!=vme_crb_ch)fprintf(f,"VME_CRB_CH2 0x%X\n",vme_crb_ch2);
  if(vme_crb_ch3!=vme_crb_ch)fprintf(f,"VME_CRB_CH3 0x%X\n",vme_crb_ch3);
  if(vme_crb_ch4!=vme_crb_ch)fprintf(f,"VME_CRB_CH4 0x%X\n",vme_crb_ch4);
  fprintf(f,"VME_V259    0x%8.8X\n",vme_v259   );
  fprintf(f,"VME_V1290   0x%8.8X\n",vme_v1290  );
  fprintf(f,"VME_V260    0x%8.8X\n",vme_v260   );
  fprintf(f,"VME_V812    0x%8.8X\n",vme_v812   );
  fprintf(f,"VME_V812_2  0x%8.8X\n",vme_v812_2 );
  fprintf(f,"VME_CRB_VEC 0x%X\n",vme_crb_vec);
  fprintf(f,"VME_CRB_IRQ 0x%X\n",vme_crb_irq);
  
  fprintf(f,"\n// PED, LED and SIG patterns\n");
  fprintf(f,"PEDPATT %d\n",PEDpatt);
  fprintf(f,"LEDPATT %d\n",LEDpatt);
  fprintf(f,"SIGPATT %d\n",SIGpatt);
  
  fprintf(f,"\n// LED and PED generators period, in seconds\n");
  fprintf(f,"PEDPERIOD %f\n",PEDperiod);
  fprintf(f,"LEDPERIOD %f\n",LEDperiod);
  
  fprintf(f,"\n// various periods\n");
  fprintf(f,"PRINTPERIOD %f  // for printing stats\n",printperiod);
  fprintf(f,"WRITEPERIOD %f  // not used for the moment\n",writeperiod);
  fprintf(f,"CMDPERIOD %f    // checking for new DIM commands\n",cmdperiod);
  fprintf(f,"UPDPERIOD %f    // updating DIM services\n",updperiod);
  
  fprintf(f,"\n// if !=0: write both ntuple and hists; if ==0, then only hists\n");
  fprintf(f,"WRITE_NTP %d\n",write_ntp);
  
  fprintf(f,"\n// if !=0: write a binary file with data\n");
  fprintf(f,"WRITE_BIN %d\n",write_bin);
  
  fprintf(f,"\n// DATACONN <name> <type> <chan> (NB ADCmodule = ADCchan/8; ADCinput=ADCchan%8)\n");
  const char* typs[4]={"x","ADC","TDC","DIG"};
  for(int i=0; i<nchans; ++i){
    if(datatype[i]>=0)fprintf(f,"DATACONN %s %s %d\n",&chnam[i][0],typs[datatype[i]],datachan[i]);
  }
  
  fprintf(f,"\n// digitizer PED summary histo: MAX or PED (0->MAX, 1->pedestals)\n");
  fprintf(f,"DIG_PED_SUMM %d\n",dig_PED_summ);
  
  fprintf(f,"\n// DIG PED adjust at start of run (<=0 - NO, >0 - required precision). Default 0 (NO)\n");
  fprintf(f,"DIG_ADJUST_OFFSETS %d\n",dig_adjust_offsets);
  fprintf(f,"DIG2_ADJUST_OFFSETS %d\n",dig2_adjust_offsets);
  
  fprintf(f,"\n// to use or not the factory corrections. 0-no, !=0-yes. Default yes\n");
  fprintf(f,"DIG_USE_CORRECTION %d\n",dig_use_correction);
  fprintf(f,"DIG2_USE_CORRECTION %d\n",dig2_use_correction);
  
  fprintf(f,"\n// Digitizer Post-Trigger value, in percents of the window width (0.2 ns*1024). Default 5\n");
  fprintf(f,"DIG_POSTTRIGGER %f\n",dig_posttrigger);
  fprintf(f,"DIG2_POSTTRIGGER %f\n",dig2_posttrigger);
  
  fprintf(f,"\n// digitizer sampling frequency code: 0->5GS/s (default), 1->2.5, 2->1, 3->0.75\n");
  fprintf(f,"DIG_FREQUENCY %d\n",dig_frequency);
  fprintf(f,"DIG2_FREQUENCY %d\n",dig2_frequency);
  
  if(strlen(dig_calib_path)>1 || strlen(dig2_calib_path)>1)
    fprintf(f,"\n// digitizer calibration paths (Vincenzo coefficients)\n");
  if(strlen(dig_calib_path)>1)fprintf(f,"DIG_CALIB_PATH %s\n",dig_calib_path);
  if(strlen(dig2_calib_path)>1)fprintf(f,"DIG2_CALIB_PATH %s\n",dig2_calib_path);
  
  fprintf(f,"\n// DWC calibration parameters\n");
  fprintf(f,"DWCX1PAR %8.1f %8.1f %8.1f\n", cx1[0], cx1[1], cx1[2]);
  fprintf(f,"DWCY1PAR %8.1f %8.1f %8.1f\n", cy1[0], cy1[1], cy1[2]);
  fprintf(f,"DWCX2PAR %8.1f %8.1f %8.1f\n", cx2[0], cx2[1], cx2[2]);
  fprintf(f,"DWCY2PAR %8.1f %8.1f %8.1f\n", cy2[0], cy2[1], cy2[2]);
  fprintf(f,"DWCX3PAR %8.1f %8.1f %8.1f\n", cx3[0], cx3[1], cx3[2]);
  fprintf(f,"DWCY3PAR %8.1f %8.1f %8.1f\n", cy3[0], cy3[1], cy3[2]);
  
  fclose(f);
  f=0;
}
  
void runParam::write(const char* fnam){
  FILE* f=fopen(fnam,"w");
  if(!f)return;
  writestream(f);
}
  
void runParam::readstream(FILE* f){
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
        
      if(0==strcmp(what,"HVCHAN")){
        if(nit>3 && nit2>0 &&nit3>0)setChanHV(cn,(int)limited(v,0,599),limited(z,0,4));
      }
      else if(0==strcmp(what,"HVIP")){
        if(nit>1) strncpy(HVIP,cn,sizeof(HVIP)-2);
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
      else if(0==strcmp(what,"LEDPERIOD")){
        if(nit>1 && nit1>0)LEDperiod=n;
      }
      else if(0==strcmp(what,"PEDPERIOD")){
        if(nit>1 && nit1>0)PEDperiod=n;
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
      else if( (0==strcmp(what,"WRITE_NTP")) || (0==strcmp(what,"WRITE_DATA")) ){
        write_ntp=1;
        if(nit>1 && nit1>0){
          if(n==0)write_ntp=0;
        }
      }
      else if(0==strcmp(what,"WRITE_BIN")){
        write_bin=0;
        if(nit>1 && nit1>0){
          if(n!=0)write_bin=1;
        }
      }
      else if(0==strcmp(what,"LEDCHAN")){
        if(nit>3 && nit1>0 && nit2>0 &&nit3>0)setLED(n,(int)limited(v,0,215),limited(z,0,5));
      }
      else if(0==strcmp(what,"VME_ADC1")){
        if(nit>1 && goodhex(cn,&u))vme_adc1=u;
      }
      else if(0==strcmp(what,"VME_ADC2")){
        if(nit>1 && goodhex(cn,&u))vme_adc2=u;
      }
      else if(0==strcmp(what,"VME_ADC3")){
        if(nit>1 && goodhex(cn,&u))vme_adc3=u;
      }
      else if(0==strcmp(what,"VME_CORBO")){
        if(nit>1 && goodhex(cn,&u))vme_corbo=u;
      }
      else if(0==strcmp(what,"VME_CRB_CH")){
        if(nit>1 && goodhex(cn,&u))vme_crb_ch=u;
      }
      else if(0==strcmp(what,"VME_CRB_CH2")){
        if(nit>1 && goodhex(cn,&u))vme_crb_ch2=u;
      }
      else if(0==strcmp(what,"VME_CRB_CH3")){
        if(nit>1 && goodhex(cn,&u))vme_crb_ch3=u;
      }
      else if(0==strcmp(what,"VME_CRB_CH4")){
        if(nit>1 && goodhex(cn,&u))vme_crb_ch4=u;
      }
      else if(0==strcmp(what,"VME_V259")){
        if(nit>1 && goodhex(cn,&u))vme_v259=u;
      }
      else if(0==strcmp(what,"VME_V1290")){
        if(nit>1 && goodhex(cn,&u))vme_v1290=u;
      }
      else if(0==strcmp(what,"VME_V260")){
        if(nit>1 && goodhex(cn,&u))vme_v260=u;
      }
      else if(0==strcmp(what,"VME_V812")){
        if(nit>1 && goodhex(cn,&u))vme_v812=u;
      }
      else if(0==strcmp(what,"VME_V812_2")){
        if(nit>1 && goodhex(cn,&u))vme_v812_2=u;
      }
      else if(0==strcmp(what,"VME_CRB_VEC")){
        if(nit>1 && goodhex(cn,&u))vme_crb_vec=u&0xFF;
      }
      else if(0==strcmp(what,"VME_CRB_IRQ")){
        if(nit>1 &&nit1>1){
          if(goodhex(cn,&u))vme_crb_irq=u;
          else vme_crb_irq=n;
          vme_crb_irq &= 0x17;
        }
      }
      else if(0==strcmp(what,"VME_CONETNODE")){
        if(nit>1 && nit1>0)vme_conetnode=n;
      }
      else if(0==strcmp(what,"DIG_CONETNODE")){
        if(nit>1 && nit1>0)dig_conetnode=n;
      }
      else if(0==strcmp(what,"DIG2_CONETNODE")){
        if(nit>1 && nit1>0)dig2_conetnode=n;
      }
      else if(0==strcmp(what,"DATACONN")){
        if(nit>3 && nit3>0)setChanDataConn(cn,cv,(int)z);
      }
      else if(0==strcmp(what,"DIG_PED_SUMM")){
        if(nit>1 && nit1>0)dig_PED_summ=n;
      }
      else if(0==strcmp(what,"DIG_ADJUST_OFFSETS")){
        if(nit>1 && nit1>0)dig_adjust_offsets=n;
      }
      else if(0==strcmp(what,"DIG2_ADJUST_OFFSETS")){
        if(nit>1 && nit1>0)dig2_adjust_offsets=n;
      }
      else if(0==strcmp(what,"DIG_USE_CORRECTION")){
        if(nit>1 && nit1>0)dig_use_correction=n;
      }
      else if(0==strcmp(what,"DIG2_USE_CORRECTION")){
        if(nit>1 && nit1>0)dig2_use_correction=n;
      }
      else if(0==strcmp(what,"DIG_POSTTRIGGER")){
        if(nit>1 && nit1>0)dig_posttrigger=n;
      }
      else if(0==strcmp(what,"DIG2_POSTTRIGGER")){
        if(nit>1 && nit1>0)dig2_posttrigger=n;
      }
      else if(0==strcmp(what,"DIG_FREQUENCY")){
        if(nit>1 && nit1>0)dig_frequency=n;
      }
      else if(0==strcmp(what,"DIG2_FREQUENCY")){
        if(nit>1 && nit1>0)dig2_frequency=n;
      }
      else if(0==strcmp(what,"DIG_CALIB_PATH")){
        if(nit>1) strncpy(dig_calib_path,cn,sizeof(dig_calib_path)-2);
      }
      else if(0==strcmp(what,"DIG2_CALIB_PATH")){
        if(nit>1) strncpy(dig2_calib_path,cn,sizeof(dig2_calib_path)-2);
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
  f=0;
  
  // now copy vme variables to globals, just in case
  VME_ADC1=    vme_adc1;
  VME_ADC2=    vme_adc2;
  VME_ADC3=    vme_adc3;
  VME_CORBO=   vme_corbo;
  VME_CRB_CH=  vme_crb_ch;
  VME_CRB_CH2= vme_crb_ch2;
  VME_CRB_CH3= vme_crb_ch3;
  VME_CRB_CH4= vme_crb_ch4;
  VME_V259=    vme_v259;
  VME_V1290=   vme_v1290;
  VME_V260=    vme_v260;
  VME_V812=    vme_v812;
  VME_V812_2=  vme_v812_2;
  VME_CRB_IRQ= vme_crb_irq;
  VME_CRB_VEC= vme_crb_vec;
  
  // check whether the run parameters make sense: 
  // at least one PMT name is present, and its HV >0
  if(nchans>0)init=true;
  //printf("nchans=%d\n",nchans);
  
  // now inventory: what is used
  memset(used742,0,sizeof(used742));
  for(int ich=0; ich<nchans; ++ich){  // 2) ADC
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
      //if( (datachan[ich]>=0 && datachan[ich]<16) || 20==datachan[ich] || 21==datachan[ich])digitizer_used=true;
      int JCH=datachan[ich];
      if(datachan[ich]>=0 && datachan[ich]<TR0DIG0CHAN){ // sig dig 1: 0-31
        JCH=datachan[ich]+datachan[ich]/8;
        digitizer_used=true;
        used742[JCH]=1;
      }
      else if(datachan[ich]>=TR0DIG0CHAN && datachan[ich]<=TR1DIG1CHAN){ // trig dig 1: 32-35
        JCH=(datachan[ich]-TR0DIG0CHAN)*9+8;
        digitizer_used=true;
        used742[JCH]=1;
      }
      else if(datachan[ich]>=N742CHAN && datachan[ich]<N742CHAN+TR0DIG0CHAN){ // sig dig 2: 36-67
        JCH=datachan[ich]+(datachan[ich]-N742CHAN)/8;
        digitizer2_used=true;
        used742[JCH]=1;
      }
      else if(datachan[ich]>=N742CHAN+TR0DIG0CHAN && datachan[ich]<=N742CHAN+TR1DIG1CHAN){ // trig dig 2: 68-71
        JCH=(datachan[ich]-68)*9+44;
        digitizer2_used=true;
        used742[JCH]=1;
      }
      else printf("%s WARNING: %s bad digitizer channel number %d\n",__func__,&chnam[ich][0],datachan[ich]);
    }
  }
  if(ADC1_used || ADC2_used || ADC3_used) ADC_used=true;
  if(digitizer_used && dig_conetnode<0){
    digitizer_used=false;
    for(int j=0; j<N742CHAN; ++j)used742[j]=0;
  }
  if(digitizer2_used && dig2_conetnode<0){
    digitizer2_used=false;
    for(int j=N742CHAN; j<2*N742CHAN; ++j)used742[j]=0;
  }
  
  if(digitizer_used && !dig_use_correction){
    digitizer_calib_init();
  }
  
  if(digitizer2_used && !dig2_use_correction){
    digitizer2_calib_init();
  }
  
  // // version 1: a larger buffer
  // // filling the evoffset array - offsets in the binary buffer, in bytes
  // // The buffer structure will be:
  // // - t                    (double)   (offset=0)
  // // - pattern              (int)      (offset=8)
  // // then nchans times, depending on the connection type
  // //   - if ADC: g_ADC (int)
  // //   - if TDC: g_nTDC (int) + g_tTDC (10*int)
  // //   - if DIG: g_n742 (int) + g_a742 (1024*float) +g_startcell (int)
  
  /*int lastoff=8, nextoff=12;
  for(int ichan=0; ichan<nchans; ++i){
    evoffset[ichan]=nextoff;
    if(1==datatype[ichan]){ // ADC
      nextoff+=4;
    }
    else if(2==datatype[ichan]){ // TDC
      nextoff+=4*(1+NTDCMAXHITS);
    }
    else if(3==datatype[ichan]){ // DIG
      nextoff+=4*(1+1024+1);
    }
  }
  evbuflen=nextoff;*/
  
  // filling the evoffset array - offsets in the binary buffer, in bytes
  // The buffer structure will be:
  // - t                    (double)   (offset=0)
  // - pattern              (int)      (offset=8)
  // then nchans times, depending on the connection type
  //   - if ADC: g_ADC (uint16_t)
  //   - if TDC: g_nTDC (uint16_t) + g_tTDC (10*uint32_t)
  //   - if DIG: g_startcell (uint16_t) + g_a742*10 (1024*uint16_t)
  
  int lastoff=8, nextoff=12;
  for(int ichan=0; ichan<nchans; ++ichan){
    evoffset[ichan]=nextoff;
    if(1==datatype[ichan]){ // ADC
      nextoff+=2;
    }
    else if(2==datatype[ichan]){ // TDC
      nextoff+=2+4*NTDCMAXHITS;
    }
    else if(3==datatype[ichan]){ // DIG
      nextoff+=2*(1+1024);
    }
  }
  evbuflen=nextoff;
  
  if(write_bin){
    printf("%s: BINARY file record size is %d bytes\n",__func__,evbuflen);
  }
}

void runParam::read(const char* fnam){
  FILE* f=fopen(fnam,"r");
  if(!f)return;
  readstream(f);
}

int runParam::read_digitizer_calibs(const char* path, int ifreq, int JCH){
  int ich=JCH2i(JCH)%N742CHAN;
  
  if(strlen(path)<2)return 0;
  
  char ccellnam[256];
  sprintf(ccellnam,"%s/%d/calib_cell_%d.txt",path,ifreq,ich);
  FILE* fcell=fopen(ccellnam,"r");
  if(!fcell)return 0;
  for(int icell=0; icell<1024; ++icell){
    fscanf(fcell,"%lg %lg %lg",&dig_p0[JCH][icell],&dig_p1[JCH][icell],&dig_p2[JCH][icell]);
  }
  fclose(fcell);
  
  char csamplnam[256];
  sprintf(csamplnam,"%s/%d/calib_sample_%d.txt",path,ifreq,ich);
  FILE* fsampl=fopen(csamplnam,"r");
  if(!fsampl)return 0;
  for(int isampl=0; isampl<1024; ++isampl){
    fscanf(fsampl,"%lg %lg %lg",&dig_pa0[JCH][isampl],&dig_pa1[JCH][isampl],&dig_pa2[JCH][isampl]);
  }
  fclose(fsampl);
  
  char ctimenam[256];
  sprintf(ctimenam,"%s/%d/calib_time_%d.txt",path,ifreq,ich);
  FILE* ftime=fopen(ctimenam,"r");
  if(!ftime)return 0;
  for(int itime=0; itime<1024; ++itime){
    fscanf(ftime,"%lg",&dig_timev[JCH][itime]);
  }
  fclose(ftime);
  
  return 1;
}

void runParam::digitizer_calib_init(){
  printf("%s:  ",__func__);
  for(int JCH=0; JCH<N742CHAN; ++JCH){
    dig_calibs_inited[JCH]=read_digitizer_calibs(dig_calib_path, dig_frequency, JCH);
    printf("%d",dig_calibs_inited[JCH]);
  }
  printf("\n");
}

void runParam::digitizer2_calib_init(){
  printf("%s:  ",__func__);
  for(int JCH=N742CHAN; JCH<2*N742CHAN; ++JCH){
    dig_calibs_inited[JCH]=read_digitizer_calibs(dig2_calib_path, dig2_frequency, JCH);
    printf("%d",dig_calibs_inited[JCH]);
  }
  printf("\n");
}

