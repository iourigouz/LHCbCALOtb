
#include <CAENDigitizer.h>
#include "wavedump_functions.h"

int dc_file[MAX_CH];
float dc_8file[8];
int thr_file[MAX_CH] = { 0 };
/*! \fn      void SetDefaultConfiguration(WaveDumpConfig_t *WDcfg) 
 *   \brief   Fill the Configuration Structure with Default Values
 *            
 *   \param   WDcfg:   Pointer to the WaveDumpConfig data structure
 */
void SetDefaultConfiguration(WaveDumpConfig_t *WDcfg) {
  int i, j;
  
  WDcfg->LinkType = CAEN_DGTZ_OpticalLink;
  WDcfg->LinkNum = 0;           // only one V1718 in the computer, #0
  WDcfg->ConetNode = 1;         // #1 (or maybe #2 if two digitizers) (#0 is the VME crate)
  WDcfg->BaseAddress = 0;       // irrelevant in case of optical connections
  
  WDcfg->MaxGroupNumber = 4;   // for V1742!
  WDcfg->Nch = 36;             // for V1742!
  
  WDcfg->Nbit = 12; 
  WDcfg->DRS4Frequency = CAEN_DGTZ_DRS4_5GHz; // 5 GHz
  WDcfg->Ts = 0.2;     // 0.2 ns (corresponds to 5 GHz)
  
  WDcfg->RecordLength = 1024;
  WDcfg->PostTrigger = 5;
  WDcfg->NumEvents = 1;
  WDcfg->EnableMask = 0xF;
  
  WDcfg->GWn = 0;
  for(i=0; i<MAX_GW; ++i) WDcfg->GWaddr[i]=WDcfg->GWdata[i]=WDcfg->GWmask[i]=0;
  
  WDcfg->ExtTriggerMode = CAEN_DGTZ_TRGMODE_ACQ_ONLY;
  WDcfg->InterruptNumEvents = 0;
  WDcfg->TestPattern = 0;
  WDcfg->DecimationFactor = 1;
  WDcfg->FastTriggerMode = CAEN_DGTZ_TRGMODE_ACQ_ONLY; 
  WDcfg->FastTriggerEnabled = CAEN_DGTZ_ENABLE; 
  WDcfg->FPIOtype = CAEN_DGTZ_IOLevel_NIM;
  
  for(i=0; i<MAX_SET; i++) {
    WDcfg->PulsePolarity[i] = CAEN_DGTZ_PulsePolarityNegative;
    WDcfg->Version_used[i] = 0;
    WDcfg->desiredPED[i]=3500;  // for negative polarity!!!
    WDcfg->DCoffset[i] = 26000; // for negative polarity!!!
    WDcfg->Threshold[i] = 0;
    WDcfg->ChannelTriggerMode[i] = CAEN_DGTZ_TRGMODE_DISABLED;
    WDcfg->GroupTrgEnableMask[i] = 0;
    for(j=0; j<MAX_SET; j++) WDcfg->DCoffsetGrpCh[i][j] = -1; // if not -1, overrides DCoffset !!!???
    WDcfg->FTThreshold[i] = 20934;
    WDcfg->FTDCoffset[i] = 32768;
  }
  WDcfg->useCorrections = -1;  // only AUTO correction is implemented, these parameters are meaningless
  WDcfg->UseManualTables = -1; // manual corrections will be implemented later
  for(i=0; i<MAX_X742_GROUP_SIZE; i++) sprintf(WDcfg->TablesFilenames[i], "Tables_gr%d", i);
  
  WDcfg->DRS4Frequency = CAEN_DGTZ_DRS4_5GHz;
  WDcfg->StartupCalibration = 1;
}

/*! \fn      int ParseConfigFile(FILE *f_ini, WaveDumpConfig_t *WDcfg) 
 *   \brief   Read the configuration file and set the WaveDump paremeters
 *            
 *   \param   f_ini        Pointer to the config file
 *   \param   WDcfg:   Pointer to the WaveDumpConfig data structure
 *   \return  0 = Success; negative numbers are error codes; positive numbers
 *               decodes the changes which need to perform internal parameters
 *               recalculations.
 */
int ParseConfigFile(FILE *f_ini, WaveDumpConfig_t *WDcfg) {
  if(!f_ini)return -1;
  
  char str[1000], str1[1000], *pread;
  int i, ch=-1, val, Off=0, tr = -1;
  int ret = 0;
  
  // Save previous values (for compare)
  int PrevUseCorrections = WDcfg->useCorrections;
  int PrevUseManualTables = WDcfg->UseManualTables;
  size_t TabBuf[sizeof(WDcfg->TablesFilenames)];
  // Copy the filenames to watch for changes
  memcpy(TabBuf, WDcfg->TablesFilenames, sizeof(WDcfg->TablesFilenames));      
  
  /* Default settings */
  SetDefaultConfiguration(WDcfg);
  
  /* read config file and assign parameters */
  while(!feof(f_ini)) {
    int read;
    char *res;
    
    // read a word from the file
    read = fscanf(f_ini, "%s", str);
    if( !read || (read == EOF) || !strlen(str))   continue;
    // skip comments
    if(str[0] == '#') {
      res = fgets(str, 1000, f_ini);
      continue;
    }
    
    if (strcmp(str, "@ON")==0) {
      Off = 0;
      continue;
    }
    if (strcmp(str, "@OFF")==0)
      Off = 1;
    if (Off)      continue;
    
    // Section (COMMON or individual channel)
    if (str[0] == '[') {
      if (strstr(str, "COMMON")) {
        ch = -1;
        continue; 
      }
      if (strstr(str, "TR")) {
        sscanf(str+1, "TR%d", &val);
        if (val < 0 || val >= MAX_SET) {
          printf("%s: Invalid channel number\n", str);
        } else {
          tr = val;
        }
      } else {
        sscanf(str+1, "%d", &val);
        if (val < 0 || val >= MAX_SET) {
          printf("%s: Invalid channel number\n", str);
        } else {
          tr=-1;
          ch = val;
        }
      }
      continue;
    }
    
    // OPEN: read the details of physical path to the digitizer
    if (strstr(str, "OPEN")!=NULL) {
      read = fscanf(f_ini, "%s", str1);
      if (strcmp(str1, "USB")==0)
        WDcfg->LinkType = CAEN_DGTZ_USB;
      else if (strcmp(str1, "PCI")==0)
        WDcfg->LinkType = CAEN_DGTZ_OpticalLink;
      else {
        printf("%s %s: Invalid connection type\n", str, str1);
        return -1; 
      }
      read = fscanf(f_ini, "%d", &WDcfg->LinkNum);
      if (WDcfg->LinkType == CAEN_DGTZ_USB)
        WDcfg->ConetNode = 0;
      else
        read = fscanf(f_ini, "%d", &WDcfg->ConetNode);
      read = fscanf(f_ini, "%x", &WDcfg->BaseAddress);
      continue;
    }

    // Generic VME Write (address offset + data + mask, each exadecimal)
    if ((strstr(str, "WRITE_REGISTER")!=NULL) && (WDcfg->GWn < MAX_GW)) {
      read = fscanf(f_ini, "%x", (int *)&WDcfg->GWaddr[WDcfg->GWn]);
      read = fscanf(f_ini, "%x", (int *)&WDcfg->GWdata[WDcfg->GWn]);
      read = fscanf(f_ini, "%x", (int *)&WDcfg->GWmask[WDcfg->GWn]);
      WDcfg->GWn++;
      continue;
    }

    // Acquisition Record Length (number of samples)
    if (strstr(str, "RECORD_LENGTH")!=NULL) {
      read = fscanf(f_ini, "%d", &WDcfg->RecordLength);
      continue;
    }

    // Acquisition Frequency (X742 only)
    if (strstr(str, "DRS4_FREQUENCY")!=NULL) {
      int PrevDRS4Freq = WDcfg->DRS4Frequency;
      int freq;
      read = fscanf(f_ini, "%d", &freq);
      WDcfg->DRS4Frequency = (CAEN_DGTZ_DRS4Frequency_t)freq;
      if(PrevDRS4Freq != WDcfg->DRS4Frequency)
        ret |= (0x1 << CFGRELOAD_CORRTABLES_BIT);
      continue;
    }

    // Correction Level (mask)
    if (strstr(str, "CORRECTION_LEVEL")!=NULL) {
      int changed = 0;
            
      read = fscanf(f_ini, "%s", str1);
      if( strcmp(str1, "AUTO") == 0 )
        WDcfg->useCorrections = -1;
      else {
        int gr = 0;
        char Buf[1000];
        char *ptr = Buf;

        WDcfg->useCorrections = atoi(str1);
        pread = fgets(Buf, 1000, f_ini); // Get the remaining line
        WDcfg->UseManualTables = -1;
        if(sscanf(ptr, "%s", str1) == 0) {
          printf("Invalid syntax for parameter %s\n", str);
          continue;
        }
        if(strcmp(str1, "AUTO") != 0) { // The user wants to use custom correction tables
          WDcfg->UseManualTables = atoi(ptr); // Save the group mask
          ptr = strstr(ptr, str1);
          ptr += strlen(str1);
          while(sscanf(ptr, "%s", str1) == 1 && gr < MAX_X742_GROUP_SIZE) {
            while( ((WDcfg->UseManualTables) & (0x1 << gr)) == 0 && gr < MAX_X742_GROUP_SIZE)
              gr++;
            if(gr >= MAX_X742_GROUP_SIZE) {
              printf("Error parsing values for parameter %s\n", str);
              continue;
            }
            ptr = strstr(ptr, str1);
            ptr += strlen(str1);
            strcpy(WDcfg->TablesFilenames[gr], str1);
            gr++;
          }
        }
      }

      // Check for changes
      if (PrevUseCorrections != WDcfg->useCorrections)
        changed = 1;
      else if (PrevUseManualTables != WDcfg->UseManualTables)
        changed = 1;
      else if (memcmp(TabBuf, WDcfg->TablesFilenames, sizeof(WDcfg->TablesFilenames)))
        changed = 1;
      if (changed == 1)
        ret |= (0x1 << CFGRELOAD_CORRTABLES_BIT);
      continue;
    }

    // Test Pattern
    if (strstr(str, "TEST_PATTERN")!=NULL) {
      read = fscanf(f_ini, "%s", str1);
      if (strcmp(str1, "YES")==0)
        WDcfg->TestPattern = 1;
      else if (strcmp(str1, "NO")!=0)
        printf("%s: invalid option\n", str);
      continue;
    }

    // Acquisition Record Length (number of samples)
    if (strstr(str, "DECIMATION_FACTOR")!=NULL) {
      read = fscanf(f_ini, "%d", (int*)&WDcfg->DecimationFactor);
      continue;
    }

    // External Trigger (DISABLED, ACQUISITION_ONLY, ACQUISITION_AND_TRGOUT)
    if (strstr(str, "EXTERNAL_TRIGGER")!=NULL) {
      read = fscanf(f_ini, "%s", str1);
      if (strcmp(str1, "DISABLED")==0)
        WDcfg->ExtTriggerMode = CAEN_DGTZ_TRGMODE_DISABLED;
      else if (strcmp(str1, "ACQUISITION_ONLY")==0)
        WDcfg->ExtTriggerMode = CAEN_DGTZ_TRGMODE_ACQ_ONLY;
      else if (strcmp(str1, "ACQUISITION_AND_TRGOUT")==0)
        WDcfg->ExtTriggerMode = CAEN_DGTZ_TRGMODE_ACQ_AND_EXTOUT;
      else
        printf("%s: Invalid Parameter\n", str);
      continue;
    }
    
    // Max. number of events for a block transfer (0 to 1023)
    if (strstr(str, "MAX_NUM_EVENTS_BLT")!=NULL) {
      read = fscanf(f_ini, "%d", &WDcfg->NumEvents);
      continue;
    }
    
    // Post Trigger (percent of the acquisition window)
    if (strstr(str, "POST_TRIGGER")!=NULL) {
      read = fscanf(f_ini, "%d", &WDcfg->PostTrigger);
      continue;
    }
    
    // Interrupt settings (request interrupt when there are at least N events to read; 0=disable interrupts (polling mode))
    if (strstr(str, "USE_INTERRUPT")!=NULL) {
      read = fscanf(f_ini, "%d", &WDcfg->InterruptNumEvents);
      continue;
    }
		
    if (!strcmp(str, "FAST_TRIGGER")) {
      read = fscanf(f_ini, "%s", str1);
      if (strcmp(str1, "DISABLED")==0)
        WDcfg->FastTriggerMode = CAEN_DGTZ_TRGMODE_DISABLED;
      else if (strcmp(str1, "ACQUISITION_ONLY")==0)
        WDcfg->FastTriggerMode = CAEN_DGTZ_TRGMODE_ACQ_ONLY;
      else
        printf("%s: Invalid Parameter\n", str);
      continue;
    }
		
    if (strstr(str, "ENABLED_FAST_TRIGGER_DIGITIZING")!=NULL) {
      read = fscanf(f_ini, "%s", str1);
      if (strcmp(str1, "YES")==0)       WDcfg->FastTriggerEnabled= CAEN_DGTZ_ENABLE;
      else if (strcmp(str1, "NO")==0)   WDcfg->FastTriggerEnabled= CAEN_DGTZ_DISABLE;
      else                              printf("%s: invalid option\n", str);
      continue;
    }
    ///Input polarity	
    if (strstr(str, "PULSE_POLARITY")!=NULL) {
      CAEN_DGTZ_PulsePolarity_t pp;
      read = fscanf(f_ini, "%s", str1);
      if (strcmp(str1, "POSITIVE") == 0)
        pp = CAEN_DGTZ_PulsePolarityPositive;
      else if (strcmp(str1, "NEGATIVE") == 0) 
        pp = CAEN_DGTZ_PulsePolarityNegative;
      else
        printf("%s: Invalid Parameter\n", str);
      
      if (ch == -1)
        for (i = 0; i<MAX_SET; i++)
          WDcfg->PulsePolarity[i] = pp;
      else			
        WDcfg->PulsePolarity[ch] = pp;
			
      continue;
    }
    
    //DC offset (percent of the dynamic range, -50 to 50)
    if (!strcmp(str, "DC_OFFSET")) {
      int dc;
      read = fscanf(f_ini, "%d", &dc);
      if (tr != -1) {
        WDcfg->FTDCoffset[tr * 2] = (uint32_t)dc;
        WDcfg->FTDCoffset[tr * 2 + 1] = (uint32_t)dc;
        continue;
      }
      
      val = (int)((float)(dc + 50) * 65535 / 100);
      if (ch == -1)
        for (i = 0; i < MAX_SET; i++){
          WDcfg->DCoffset[i] = val;
          WDcfg->Version_used[i] = 0;
        }
      else{
        WDcfg->DCoffset[ch] = val;
        WDcfg->Version_used[ch] = 0;
      }
      
      continue;
    }
    
    
    if (!strcmp(str, "BASELINE_SHIFT")) {
      int dc;
      read = fscanf(f_ini, "%d", &dc);
      if (tr != -1) {
        // 				WDcfg->FTDCoffset[tr] = dc;
        WDcfg->FTDCoffset[tr * 2] = (uint32_t)dc;
        WDcfg->FTDCoffset[tr * 2 + 1] = (uint32_t)dc;
        continue;
      }
      
      if (ch == -1){
        for (i = 0; i < MAX_SET; i++){
          WDcfg->Version_used[i] = 1;
          dc_file[i] = dc;
          if (WDcfg->PulsePolarity[i] == CAEN_DGTZ_PulsePolarityPositive)					
            WDcfg->DCoffset[i] = (uint32_t)((float)(fabs(dc - 100))*(655.35));
          else if (WDcfg->PulsePolarity[i] == CAEN_DGTZ_PulsePolarityNegative)					
            WDcfg->DCoffset[i] = (uint32_t)((float)(dc)*(655.35));					
        }
      }
      else {
        WDcfg->Version_used[ch] = 1;
        dc_file[ch] = dc;
        if (WDcfg->PulsePolarity[ch] == CAEN_DGTZ_PulsePolarityPositive){
          WDcfg->DCoffset[ch] = (uint32_t)((float)(fabs(dc - 100))*(655.35));
          //printf("ch %d positive, offset %d\n",ch, WDcfg->DCoffset[ch]);
        }
        else if (WDcfg->PulsePolarity[ch] == CAEN_DGTZ_PulsePolarityNegative){
          WDcfg->DCoffset[ch] = (uint32_t)((float)(dc)*(655.35));
          //printf("ch %d negative, offset %d\n",ch, WDcfg->DCoffset[ch]);
        }
      }
      continue;
    }
    
    if (strstr(str, "GRP_CH_DC_OFFSET")!=NULL){ ///xx742
      float dc[8];
      read = fscanf(f_ini, "%f,%f,%f,%f,%f,%f,%f,%f", &dc[0], &dc[1], &dc[2], &dc[3], &dc[4], &dc[5], &dc[6], &dc[7]);
      int j = 0;
      for( j=0;j<8;j++) dc_8file[j] = dc[j];
      for(i=0; i<8; i++){ //MAX_SET
        val = (int)((dc[i]+50) * 65535 / 100); ///DC offset (percent of the dynamic range, -50 to 50)
        WDcfg->DCoffsetGrpCh[ch][i]=val;
      }
      continue;
    }
    
    // Threshold
    if (strstr(str, "TRIGGER_THRESHOLD")!=NULL) {
      read = fscanf(f_ini, "%d", &val);
      if (tr != -1) {
        //				WDcfg->FTThreshold[tr] = val;
        WDcfg->FTThreshold[tr*2] = val;
        WDcfg->FTThreshold[tr*2+1] = val;
        continue;
      }
      if (ch == -1)
        for (i = 0; i < MAX_SET; i++){
          WDcfg->Threshold[i] = val;
          thr_file[i] = val;
        }
      else{
        WDcfg->Threshold[ch] = val;
        thr_file[ch] = val;
      }
      continue;
    }
    
    // Group Trigger Enable Mask (hex 8 bit)
    if (strstr(str, "GROUP_TRG_ENABLE_MASK")!=NULL) {
      read = fscanf(f_ini, "%x", &val);
      if (ch == -1)
        for(i=0; i<MAX_SET; i++)
          WDcfg->GroupTrgEnableMask[i] = val & 0xFF;
      else
        WDcfg->GroupTrgEnableMask[ch] = val & 0xFF;
      continue;
    }

    // Channel Auto trigger (DISABLED, ACQUISITION_ONLY, ACQUISITION_AND_TRGOUT)
    if (strstr(str, "CHANNEL_TRIGGER")!=NULL) {
      CAEN_DGTZ_TriggerMode_t tm;
      read = fscanf(f_ini, "%s", str1);
      if (strcmp(str1, "DISABLED") == 0)
        tm = CAEN_DGTZ_TRGMODE_DISABLED;
      else if (strcmp(str1, "ACQUISITION_ONLY") == 0)
        tm = CAEN_DGTZ_TRGMODE_ACQ_ONLY;
      else if (strcmp(str1, "ACQUISITION_AND_TRGOUT") == 0)
        tm = CAEN_DGTZ_TRGMODE_ACQ_AND_EXTOUT;
      else if (strcmp(str1, "TRGOUT_ONLY") == 0)
        tm = CAEN_DGTZ_TRGMODE_EXTOUT_ONLY;
      else {
        printf("%s: Invalid Parameter\n", str);
        continue;
      }
      if (ch == -1)
        for(i=0; i<MAX_SET; i++)
          WDcfg->ChannelTriggerMode[i] = tm;
      else
        WDcfg->ChannelTriggerMode[ch] = tm; 
			
      continue;
    }

    // Front Panel LEMO I/O level (NIM, TTL)
    if (strstr(str, "FPIO_LEVEL")!=NULL) {
      read = fscanf(f_ini, "%s", str1);
      if      (strcmp(str1, "TTL")==0) WDcfg->FPIOtype = CAEN_DGTZ_IOLevel_TTL;
      else if (strcmp(str1, "NIM")==0) WDcfg->FPIOtype = CAEN_DGTZ_IOLevel_NIM;
      else                             printf("%s: invalid option\n", str);
      continue;
    }

    // Channel Enable (or Group enable for the V1740) (YES/NO)
    if (strstr(str, "ENABLE_INPUT")!=NULL) {
      read = fscanf(f_ini, "%s", str1);
      if (strcmp(str1, "YES")==0) {
        if (ch == -1)
          WDcfg->EnableMask = 0xFF;
        else
          {
            WDcfg->EnableMask |= (1 << ch);
          }
        continue;
      } else if (strcmp(str1, "NO")==0) {
        if (ch == -1)
          WDcfg->EnableMask = 0x00;
        else
          WDcfg->EnableMask &= ~(1 << ch);
        continue;
      } else {
        printf("%s: invalid option\n", str);
      }
      continue;
    }

    // Skip startup calibration
    if (strstr(str, "SKIP_STARTUP_CALIBRATION") != NULL) {
      read = fscanf(f_ini, "%s", str1);
      if (strcmp(str1, "YES") == 0)
        WDcfg->StartupCalibration = 0;
      else
        WDcfg->StartupCalibration = 1;
      continue;
    }

    printf("%s: invalid setting\n", str);
  }
  return ret;
}

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

static CAEN_DGTZ_IRQMode_t INTERRUPT_MODE = CAEN_DGTZ_IRQ_MODE_ROAK;

/*! \fn      int GetMoreBoardNumChannels(CAEN_DGTZ_BoardInfo_t BoardInfo,  WaveDumpConfig_t *WDcfg)
 *   \brief   calculate num of channels, num of bit and sampl period according to the board type
 *
 *   \param   BoardInfo   Board Type
 *   \param   WDcfg       pointer to the config. struct
 *   \return  0 = Success; -1 = unknown board type
 */
int GetMoreBoardInfo(int handle, CAEN_DGTZ_BoardInfo_t BoardInfo, WaveDumpConfig_t *WDcfg)
{
  int ret;
  
  CAEN_DGTZ_DRS4Frequency_t freq;
  WDcfg->Nbit = 12; 
  ret = CAEN_DGTZ_GetDRS4SamplingFrequency(handle, &freq);
  if (ret != CAEN_DGTZ_Success){
    printf("%s: error in GetDRS4SamplingFrequency --> communication error\n",__func__);
    return ret;
  }
  if(CAEN_DGTZ_DRS4_1GHz==freq) WDcfg->Ts = 1.0;
  else if (CAEN_DGTZ_DRS4_2_5GHz==freq) WDcfg->Ts = (float)0.4;
  else if (CAEN_DGTZ_DRS4_5GHz==freq) WDcfg->Ts = (float)0.2;
  else if (CAEN_DGTZ_DRS4_750MHz==freq) WDcfg->Ts = (float)(1.0/750.0)*1000.0;
  
  switch(BoardInfo.FormFactor) {
  case CAEN_DGTZ_VME64_FORM_FACTOR:
  case CAEN_DGTZ_VME64X_FORM_FACTOR:
    WDcfg->MaxGroupNumber = 4;
    WDcfg->Nch = 36;
    break;
  case CAEN_DGTZ_DESKTOP_FORM_FACTOR:
  case CAEN_DGTZ_NIM_FORM_FACTOR:
  default:
    WDcfg->MaxGroupNumber = 2;
    WDcfg->Nch = 18;
    break;
  }
  printf("%s DIGITIZER: MaxGroupNumber=%d, Nch=%d\n",__func__,WDcfg->MaxGroupNumber,WDcfg->Nch);
  
  return 0;
}

int WriteRegisterBitmask(int32_t handle, uint32_t address, uint32_t data, uint32_t mask) {
  int32_t ret = CAEN_DGTZ_Success;
  uint32_t d32 = 0xFFFFFFFF;

  ret = CAEN_DGTZ_ReadRegister(handle, address, &d32);
  if(ret != CAEN_DGTZ_Success){
    printf("%s: error in ReadRegister\n",__func__);
    return ret;
  }

  data &= mask;
  d32 &= ~mask;
  d32 |= data;
  ret = CAEN_DGTZ_WriteRegister(handle, address, d32);
  if(ret != CAEN_DGTZ_Success){
    printf("%s: error in WriteRegister\n",__func__);
    return ret;
  }
  
  return CAEN_DGTZ_Success;
}

int ProgramDigitizer(int handle, WaveDumpConfig_t WDcfg, CAEN_DGTZ_BoardInfo_t BoardInfo){
  // setting for X742 boards only !!!
  int i, j, ret = 0;

  ret |= CAEN_DGTZ_Reset(handle); // reset the digitizer
  if (ret != 0) {
    printf("Error: Unable to reset digitizer.\nPlease reset digitizer manually then restart the program\n");
    return -1;
  }

  // Set the waveform test bit for debugging
  if (WDcfg.TestPattern) ret |= CAEN_DGTZ_WriteRegister(handle, CAEN_DGTZ_BROAD_CH_CONFIGBIT_SET_ADD, 1<<3);
  // enable software trigger for acquisition only
  ret |= CAEN_DGTZ_SetSWTriggerMode(handle, CAEN_DGTZ_TRGMODE_ACQ_ONLY);
  
  ret |= CAEN_DGTZ_SetFastTriggerDigitizing(handle,WDcfg.FastTriggerEnabled);
  ret |= CAEN_DGTZ_SetFastTriggerMode(handle,WDcfg.FastTriggerMode);
  
  ret |= CAEN_DGTZ_SetRecordLength(handle, WDcfg.RecordLength);
  ret |= CAEN_DGTZ_GetRecordLength(handle, &WDcfg.RecordLength);

  ret |= CAEN_DGTZ_SetPostTriggerSize(handle, WDcfg.PostTrigger);
  
  ret |= CAEN_DGTZ_SetIOLevel(handle, WDcfg.FPIOtype);
  if( WDcfg.InterruptNumEvents > 0) {
    // Interrupt handling
    if( ret |= CAEN_DGTZ_SetInterruptConfig( handle, CAEN_DGTZ_ENABLE,
                                             VME_INTERRUPT_LEVEL, VME_INTERRUPT_STATUS_ID,
                                             (uint16_t)WDcfg.InterruptNumEvents, INTERRUPT_MODE)!= CAEN_DGTZ_Success) {
      printf( "\nError configuring interrupts. Interrupts disabled\n\n");
      WDcfg.InterruptNumEvents = 0;
    }
  }
  
  ret |= CAEN_DGTZ_SetMaxNumEventsBLT(handle, WDcfg.NumEvents);
  ret |= CAEN_DGTZ_SetAcquisitionMode(handle, CAEN_DGTZ_SW_CONTROLLED);
  ret |= CAEN_DGTZ_SetExtTriggerInputMode(handle, WDcfg.ExtTriggerMode);
  
  ret |= CAEN_DGTZ_SetGroupEnableMask(handle, WDcfg.EnableMask);
  for(i=0; i<(WDcfg.Nch/8); i++) {
    if (WDcfg.EnableMask & (1<<i)) {
      for(j=0; j<8; j++) {
        if (WDcfg.DCoffsetGrpCh[i][j] != -1){
          ret |= CAEN_DGTZ_SetChannelDCOffset(handle,(i*8)+j, WDcfg.DCoffsetGrpCh[i][j]);
        }
        else{
          //ret |= CAEN_DGTZ_SetChannelDCOffset(handle,(i*8)+j, WDcfg.DCoffset[i]);
          ret |= CAEN_DGTZ_SetChannelDCOffset(handle,(i*8)+j, WDcfg.DCoffset[(i*8)+j]);
        }
      }
      ret |= CAEN_DGTZ_SetTriggerPolarity(handle, i, (CAEN_DGTZ_TriggerPolarity_t)WDcfg.PulsePolarity[i]); 
    }
  }
  for(i=0; i<(WDcfg.Nch/8); i++) {
    ret |= CAEN_DGTZ_SetDRS4SamplingFrequency(handle, WDcfg.DRS4Frequency);
    ret |= CAEN_DGTZ_SetGroupFastTriggerDCOffset(handle,i,WDcfg.FTDCoffset[i]);
    ret |= CAEN_DGTZ_SetGroupFastTriggerThreshold(handle,i,WDcfg.FTThreshold[i]);
  }
  
  /* execute generic write commands */
  for(i=0; i<WDcfg.GWn; i++)
    ret |= WriteRegisterBitmask(handle, WDcfg.GWaddr[i], WDcfg.GWdata[i], WDcfg.GWmask[i]);
  
  if (ret)
    printf("Warning: errors found during the programming of the digitizer.\nSome settings may not be executed\n");
  
  return 0;
}
