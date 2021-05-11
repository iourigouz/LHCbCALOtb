#include <math.h>
#include "CAENHVWrapper.h"
#include "runparam.h"

int HVhandle=-1;

int SY5527_login(const char* ipaddress){
  if(HVhandle>=0){
    printf("%s(%s): already logged in\n",__func__,ipaddress);
    return CAENHV_OK;
  }
  // 
  char str[256];
  memset(str,0,sizeof(str));
  strncpy(str,ipaddress,200);
  int ret = CAENHV_InitSystem(SY5527, LINKTYPE_TCPIP, str, "admin", "admin", &HVhandle);
  //int ret = CAENHV_InitSystem(3, 0, ipaddress, "admin", "admin", &HVhandle);
  if(CAENHV_OK!=ret){
    printf("%s(%s): WARNING! CAENHV_InitSystem returns %d, %s\n",__func__,ipaddress,
           ret,CAENHV_GetError(HVhandle));
  }
  return ret;
}

int SY5527_logout(){
  if(HVhandle<0){
    printf("%s: already logged out\n",__func__);
    return CAENHV_OK;
  }
  // 
  int ret = CAENHV_DeinitSystem(HVhandle);
  if(CAENHV_OK!=ret){
    printf("%s(): WARNING! CAENHV_DeinitSystem returns %d, %s\n",__func__,ret,CAENHV_GetError(HVhandle));
  }
  HVhandle=-1;
  return ret;
}

int SY5527_setChName(int chan, const char* ChName){// chan = 100*slot + out
  if(HVhandle<0){
    printf("%s: HV handle <0 - not connected?\n",__func__);
    return -1;
  }
  unsigned short slot=chan/100;
  unsigned short out=chan%100;
  int ret = CAENHV_SetChName(HVhandle, slot, 1, &out, ChName);
  if(CAENHV_OK!=ret){
    printf("%s(%d,%s): WARNING! CAENHV_SetChName returns %d, %s\n",__func__,chan,ChName,
           ret,CAENHV_GetError(HVhandle));
  }
  return ret;
}

int SY5527_setHV(int chan, double HV_kV){
  float volt=HV_kV*1000;
  unsigned short slot=chan/100;
  unsigned short out=chan%100;
  int ret=CAENHV_SetChParam(HVhandle, slot, "V0Set", 1, &out, &volt);
  if(CAENHV_OK!=ret){
    printf("%s(%d,%.3f): WARNING! CAENHV_SetChParam returns %d, %s\n",__func__,chan,HV_kV,
           ret,CAENHV_GetError(HVhandle));
  }
  return ret;
}

int SY5527_HVOn(int chan){
  unsigned short slot=chan/100;
  unsigned short out=chan%100;
  unsigned long On=1;
  int ret=CAENHV_SetChParam(HVhandle, slot, "Pw", 1, &out, &On);
  if(CAENHV_OK!=ret){
    printf("%s(%d): WARNING! CAENHV_SetChParam returns %d, %s\n",__func__,chan,
           ret,CAENHV_GetError(HVhandle));
  }
  return ret;
}

int SY5527_HVOff(int chan){
  unsigned short slot=chan/100;
  unsigned short out=chan%100;
  unsigned long On=1;
  int ret=CAENHV_SetChParam(HVhandle, slot, "Pw", 1, &out, &On);
  if(CAENHV_OK!=ret){
    printf("%s(%d): WARNING! CAENHV_SetChParam returns %d, %s\n",__func__,chan,
           ret,CAENHV_GetError(HVhandle));
  }
  return ret;
}

int SY5527_getChStatus(int chan, unsigned long &Status){// 0-Off, 1-On, 5-RampDown, 3-RampUp
  // Bit 0 Channel is on 
  // Bit 1 Channel is ramping up 
  // Bit 2 Channel is ramping down 
  // Bit 3 Channel is in overcurrent 
  // Bit 4 Channel is in overvoltage 
  // Bit 5 Channel is in undervoltage 
  // Bit 6 Channel is in external trip 
  // Bit 7 Channel is in max V 
  // Bit 8 Channel is in external disable 
  // Bit 9 Channel is in internal trip 
  // Bit 10 Channel is in calibration error 
  // Bit 11 Channel is unplugged 
  
  unsigned short slot=chan/100;
  unsigned short out=chan%100;
  int ret = CAENHV_GetChParam(HVhandle, slot, "Status", 1, &out, &Status);
  if(CAENHV_OK!=ret){
    printf("%s(%d): WARNING! CAENHV_GetChParam(..,""Status"",..) returns %d, %s\n",__func__,chan,
           ret,CAENHV_GetError(HVhandle));
  }
  return ret;
}

int SY5527_getChVMon(int chan, float &VMon){
  unsigned short slot=chan/100;
  unsigned short out=chan%100;
  int ret = CAENHV_GetChParam(HVhandle, slot, "VMon", 1, &out, &VMon);
  if(CAENHV_OK!=ret){
    printf("%s(%d): WARNING! CAENHV_GetChParam(..,""VMon"",..) returns %d, %s\n",__func__,chan,
           ret,CAENHV_GetError(HVhandle));
  }
  return ret;
}

//===================================================================================

int SY5527_connect(){
  int ret=SY5527_login(g_rp.HVIP);
  return ret;
}

int SY5527_disconnect(){
  int ret=SY5527_logout();
  return ret;
}

bool SY5527_checkAllHV(){
  if(strlen(g_rp.HVIP)<7){
    printf("%s: IP address of HV system not set, proceed without HV control\n");
    HVhandle=-1;
    return true;
  }
  
  int ret;
  bool result=true;
  
  ret=SY5527_login(g_rp.HVIP);
  if(0!=ret){
    printf("%s(): HV login unsuccessful, will proceed without HV control\n",__func__);
    HVhandle=-1;
    memset(g_rp.HVIP,0,sizeof(g_rp.HVIP));
    return true;
  }
  // first set the names
  for (int i=0; i<g_rp.nHVchans; ++i){
    SY5527_setChName(g_rp.HVchan[i],&g_rp.HVname[i][0]);
  }
  usleep(500000);
  
  float VMon=0;
  for (int i=0; i<g_rp.nHVchans; ++i){
    SY5527_getChVMon(g_rp.HVchan[i],VMon);
    if( fabs(g_rp.HV[i]*1000-VMon)>1 )result=false;
  }
  
  if(result){
    ret=SY5527_logout();
    if(0!=ret){
      printf("%s(): HV logout error, will proceed without HV control\n",__func__);
      HVhandle=-1;
      memset(g_rp.HVIP,0,sizeof(g_rp.HVIP));
    }
  }
  
  return result;
}

void SY5527_setAllHVOn(){
  if(strlen(g_rp.HVIP)<7){
    printf("%s: IP address of HV system not set, proceed without HV control\n");
    HVhandle=-1;
    return;
  }
  
  int ret=0;
  
  ret=SY5527_login(g_rp.HVIP);
  if(0!=ret){
    printf("%s(): HV login unsuccessful, will proceed without HV control\n",__func__);
    HVhandle=-1;
    memset(g_rp.HVIP,0,sizeof(g_rp.HVIP));
    return;
  }
  
  // first set the names
  for (int i=0; i<g_rp.nHVchans; ++i){
    SY5527_setChName(g_rp.HVchan[i],&g_rp.HVname[i][0]);
  }
  usleep(500000);
  //
  for (int i=0; i<g_rp.nHVchans; ++i){
    SY5527_setHV(g_rp.HVchan[i],g_rp.HV[i]);
  }
  //
  usleep(500000);
  for (int i=0; i<g_rp.nHVchans; ++i){
    SY5527_HVOn(g_rp.HVchan[i]);
  }
  
  // No need to logout: we will then call SY5527_waitAllHvOn !!!
  /*ret=SY5527_logout();
  if(0!=ret){
    printf("%s(): HV logout error, will proceed without HV control\n",__func__);
    HVhandle=-1;
    memset(g_rp.HVIP,0,sizeof(g_rp.HVIP));
    }*/
}

void SY5527_waitAllHVOn(){
  if(strlen(g_rp.HVIP)<7){
    printf("%s: IP address of HV system not set, proceed without HV control\n");
    HVhandle=-1;
    return;
  }
  
  int ret=0;
  
  ret=SY5527_login(g_rp.HVIP);
  if(0!=ret){
    printf("%s(): HV login unsuccessful, will proceed without HV control\n",__func__);
    HVhandle=-1;
    memset(g_rp.HVIP,0,sizeof(g_rp.HVIP));
    return;
  }
  
  unsigned long Status=0;
  int HV_MAXTRY=1000;
  useconds_t HV_WAIT1=500000;
  for (int i=0; i<g_rp.nHVchans; ++i){
    int j;
    for(j=0; j<HV_MAXTRY; ++j){
      SY5527_getChStatus(g_rp.HVchan[i],Status);
      if(1==Status)break;
      else usleep(HV_WAIT1);
    }
    if(j>=HV_MAXTRY){
      printf("%s: WARNING!! HV channel %s (%d) HV not reached after %f seconds\n",__func__,
             &g_rp.HVname[i][0],g_rp.HVchan[i],(double)HV_WAIT1*HV_MAXTRY/1.0e6);
    }
  }
  
  ret=SY5527_logout();
  if(0!=ret){
    printf("%s(): HV logout error, will proceed without HV control\n",__func__);
    HVhandle=-1;
    memset(g_rp.HVIP,0,sizeof(g_rp.HVIP));
  }
}

void SY5527_setAllHVOff(){
  if(strlen(g_rp.HVIP)<7){
    printf("%s: WARNING cannot turn HV off!\n");
    HVhandle=-1;
    return;
  }
  
  int ret;
  
  ret=SY5527_login(g_rp.HVIP);
  if(0!=ret){
    printf("%s(): HV login unsuccessful, will proceed without HV control\n",__func__);
    printf("%s: WARNING cannot turn HV off!\n");
    HVhandle=-1;
    memset(g_rp.HVIP,0,sizeof(g_rp.HVIP));
    return;
  }
  
  for (int i=0; i<g_rp.nHVchans; ++i){
    SY5527_HVOff(g_rp.HVchan[i]);
  }
  
  ret=SY5527_logout();
  if(0!=ret){
    printf("%s(): HV logout error, will proceed without HV control\n",__func__);
    printf("%s: WARNING cannot turn HV off!\n");
    HVhandle=-1;
    memset(g_rp.HVIP,0,sizeof(g_rp.HVIP));
  }
}

