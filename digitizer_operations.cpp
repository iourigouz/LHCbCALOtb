#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <inttypes.h>
#include <stdint.h>
#include <unistd.h>

#include "CAENDigitizer.h"

#include <math.h>
#include <string.h>
#include <ctype.h>

#include "wavedump_functions.h"
#include "digitizer_operations.h"

#include "ntp.h"

int       DHandle;

CAEN_DGTZ_BoardInfo_t       BoardInfo;

char* DBuffer=NULL;
uint32_t DBufAllocatedSize=0;
uint32_t DBufferSize=0;
uint32_t DBufferNevts=0;

CAEN_DGTZ_EventInfo_t       EventInfo;
char *EventPtr = NULL;
CAEN_DGTZ_X742_EVENT_t       *Event742=NULL;

WaveDumpConfig_t WDcfg;

int digitizer_init(char* config_name){
  //CAEN_DGTZ_ErrorCode CAENDGTZ_API ret = CAEN_DGTZ_Success;
  int ret = CAEN_DGTZ_Success;
  
  // read config
  memset(&WDcfg, 0, sizeof(WDcfg));
  
  //printf("%s: using /etc/wavedump/WaveDumpConfig_X742.txt\n",__func__);
  //strcpy(ConfigFileName, "/etc/wavedump/WaveDumpConfig_X742.txt");
  char ConfigFileName[256];
  if(config_name)sprintf(ConfigFileName,"%s",config_name);
  else sprintf(ConfigFileName,"DT5742_default.param");
  
  FILE* f_ini = fopen(ConfigFileName, "r");
  if (!f_ini) {
    printf("%s: config file not found\n",__func__);
    goto Close;
  }
  ParseConfigFile(f_ini, &WDcfg);
  fclose(f_ini);
  
  ret = CAEN_DGTZ_OpenDigitizer((CAEN_DGTZ_ConnectionType)WDcfg.LinkType, WDcfg.LinkNum, WDcfg.ConetNode, WDcfg.BaseAddress, &DHandle);
  if(ret!=CAEN_DGTZ_Success){
    printf("%s: error in OpenDigitizer\n",__func__);
    return ret;
  }
  
  ret = CAEN_DGTZ_GetInfo(DHandle, &BoardInfo);
  if (ret!=CAEN_DGTZ_Success) {
    printf("%s: error in GetInfo I\n",__func__);
    goto Close;
  }
  printf("Connected to CAEN Digitizer Model %s\n", BoardInfo.ModelName);
  printf("ROC FPGA Release is %s\n", BoardInfo.ROC_FirmwareRel);
  printf("AMC FPGA Release is %s\n", BoardInfo.AMC_FirmwareRel);

  // Check firmware rivision (DPP firmwares cannot be used with WaveDump)
  int MajorNumber;
  sscanf(BoardInfo.AMC_FirmwareRel, "%d", &MajorNumber);
  if (MajorNumber >= 128) {
    printf("%s: wrong firmware: DPP\n",__func__);
    goto Close;
  }
  
  //Check if model x742 is in use
  if (BoardInfo.FamilyCode != CAEN_DGTZ_XX742_FAMILY_CODE) {
    printf("%s: wrong board type - I\n",__func__);
    goto Close;
  }

  // Get Number of Channels, Number of bits, Number of Groups of the board */
  ret = GetMoreBoardInfo(DHandle, BoardInfo, &WDcfg);
  if (ret!=CAEN_DGTZ_Success) {
    printf("%s: wrong board type - II\n",__func__);
    goto Close;
  }
  
  WDcfg.EnableMask &= (1<<(WDcfg.Nch/8))-1;
  
  ret = ProgramDigitizer(DHandle, WDcfg, BoardInfo);
  if (ret!=CAEN_DGTZ_Success) {
    printf("%s: error in ProgramDigitizer\n",__func__);
    goto Close;
  }
 
  // Read again the board infos, just in case some of them were changed by the programming
  // (like, for example, the TSample and the number of channels if DES mode is changed)
  ret = CAEN_DGTZ_GetInfo(DHandle, &BoardInfo);
  if (ret!=CAEN_DGTZ_Success) {
    printf("%s WARNING: GetInfo returns %d\n",__func__,ret);
    goto Close;
  }
  ret = GetMoreBoardInfo(DHandle,BoardInfo, &WDcfg);
  if (ret!=CAEN_DGTZ_Success) {
    printf("%s: invalid board type II\n",__func__);
    goto Close;
  }

  // Reload Correction Tables if changed
  WDcfg.useCorrections = -1;  // Use AUTO Corrections anyway!!!
  ret = CAEN_DGTZ_LoadDRS4CorrectionData(DHandle, WDcfg.DRS4Frequency);
  if(ret!=CAEN_DGTZ_Success){
    printf("%s WARNING: LoadDRS4CorrectionData returns %d\n",__func__,ret);
    goto Close;
  }
  ret = CAEN_DGTZ_EnableDRS4Correction(DHandle);
  if(ret!=CAEN_DGTZ_Success){
    printf("%s WARNING: EnableDRS4Correction returns %d\n",__func__,ret);
    goto Close;
  }
      
  // allocate event buffer
  ret = CAEN_DGTZ_AllocateEvent(DHandle, (void**)&Event742);
  if(ret!=CAEN_DGTZ_Success){
    printf("%s WARNING: AllocateEvent returns %d\n",__func__,ret);
    goto Close;
  }
 // allocate readout buffer
  ret = CAEN_DGTZ_MallocReadoutBuffer(DHandle, &DBuffer,&DBufAllocatedSize); /* WARNING: This must be done after the digitizer programming */
  if(ret!=CAEN_DGTZ_Success){
    printf("%s WARNING: MallocReadoutBuffer returns %d\n",__func__,ret);
    goto Close;
  }
  
  return ret;

 Close:
  CAEN_DGTZ_CloseDigitizer (DHandle);
  return ret;
}

int digitizer_close(){
  CAEN_DGTZ_ErrorCode CAENDGTZ_API ret = CAEN_DGTZ_Success;
  
  ret=CAEN_DGTZ_FreeEvent(DHandle, (void**)&Event742); 
  if(ret!=CAEN_DGTZ_Success){
    printf("%s: cannot deallocate event buffer\n",__func__);
    return ret;
  }
  ret=CAEN_DGTZ_FreeReadoutBuffer(&DBuffer); 
  if(ret!=CAEN_DGTZ_Success){
    printf("%s: cannot deallocate readout buffer\n",__func__);
    return ret;
  }
  ret=CAEN_DGTZ_CloseDigitizer (DHandle); 
  if(ret!=CAEN_DGTZ_Success){
    printf("%s: cannot close digitizer\n",__func__);
    return ret;
  }
}

int digitizer_reset(){
  CAEN_DGTZ_ErrorCode CAENDGTZ_API ret = CAEN_DGTZ_Reset (DHandle); 
  if(ret!=CAEN_DGTZ_Success){
    printf("%s: error \n",__func__);
  }
  return ret;
}

int digitizer_clear(){
  CAEN_DGTZ_ErrorCode CAENDGTZ_API ret = CAEN_DGTZ_ClearData (DHandle); 
  if(ret!=CAEN_DGTZ_Success){
    printf("%s: error \n",__func__);
  }
  return ret;
}

int digitizer_start(){
  int ret=0;
  ret=CAEN_DGTZ_SWStartAcquisition(DHandle);
  if(ret!=CAEN_DGTZ_Success){
    printf("%s WARNING: SWStartAcquisition returns %d\n",__func__,ret);
  }
  
  return ret;
}

int digitizer_stop(){
  int ret=0;
  ret=CAEN_DGTZ_SWStartAcquisition(DHandle);
  if(ret!=CAEN_DGTZ_Success){
    printf("%s WARNING: SWStartAcquisition returns %d\n",__func__,ret);
  }
  
  return ret;
}

int digitizer_read(){
  CAEN_DGTZ_ErrorCode CAENDGTZ_API ret = CAEN_DGTZ_Success;
  
  ret=CAEN_DGTZ_ReadData(DHandle, CAEN_DGTZ_SLAVE_TERMINATED_READOUT_MBLT, DBuffer, &DBufferSize);
  if(ret!=CAEN_DGTZ_Success){
    printf("%s: error %d in ReadData\n",__func__,ret);
    return ret;
  }
  
  ret=CAEN_DGTZ_GetNumEvents(DHandle, DBuffer, DBufferSize, &DBufferNevts);
  if(ret!=CAEN_DGTZ_Success){
    printf("%s: error %d in GetEventInfo\n",__func__,ret);
    return ret;
  }
  else if (1!=DBufferNevts){
    printf("%s: WARNING %d events read!!\n",__func__,DBufferNevts);
    return DBufferNevts;
  }

  ret=CAEN_DGTZ_GetEventInfo(DHandle, DBuffer, DBufferSize, 0, &EventInfo, &EventPtr);
  if(ret!=CAEN_DGTZ_Success){
    printf("%s: error %d in GetEventInfo\n",__func__,ret);
    return ret;
  }
  
  ret=CAEN_DGTZ_DecodeEvent(DHandle, EventPtr, (void**)&Event742);
  if(ret!=CAEN_DGTZ_Success){
    printf("%s: error in DecodeEvent\n",__func__);
    return ret;
  }
  
  memset(g_nDT5742,0,sizeof(g_nDT5742));
  memset(g_evdata742,0,sizeof(g_evdata742));
  for(int igr=0; igr<MAX_X742_GROUP_SIZE; ++igr){
    if(0!=Event742->GrPresent[igr]){
      for(int ich=0; ich<MAX_X742_CHANNEL_SIZE; ++ich){
        int JCH=igr*MAX_X742_CHANNEL_SIZE+ich;
        g_nDT5742[JCH]=(Event742->DataGroup[igr]).ChSize[ich];
        g_evdata742[JCH]=(Event742->DataGroup[igr]).DataChannel[ich];
        if(g_used742[JCH]){
          for(int i=0; i<g_nDT5742[JCH];++i){
            g_aDT5742[JCH][i]=(g_evdata742[JCH])[i];
          }
        }
      }
    }
  }
}

