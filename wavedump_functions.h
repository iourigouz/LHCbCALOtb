#ifndef _WAVEDUMP_FUNCTIONS__H
#define _WAVEDUMP_FUNCTIONS__H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
//#include <fstream.h>
#include <CAENDigitizer.h>
#include <CAENDigitizerType.h>

#include <unistd.h>
#include <stdint.h>   /* C99 compliant compilers: uint64_t */
#include <ctype.h>    /* toupper() */
#include <sys/time.h>

#define         _PACKED_                __attribute__ ((packed, aligned(1)))
#define         _INLINE_                __inline__ 

#define DEFAULT_CONFIG_FILE  "/etc/wavedump/WaveDumpConfig_X742.txt"

#define MAX_CH  64          /* max. number of channels */
#define MAX_SET 32           /* max. number of independent settings */

#define MAX_GW  1000        /* max. number of generic write commads */

#define VME_INTERRUPT_LEVEL      1
#define VME_INTERRUPT_STATUS_ID  0xAAAA
#define INTERRUPT_TIMEOUT        200  // ms

#define CFGRELOAD_CORRTABLES_BIT (0)

typedef struct {
  int LinkType;                                                //+
  int LinkNum;                                                 //+
  int ConetNode;                                               //+
  uint32_t BaseAddress;                                        //+
  int Nch;                                                     //+
  int Nbit;                                                    //+
  float Ts;                                                    //+
  int NumEvents;                                               //+
  uint32_t RecordLength;                                       //+
  int PostTrigger;                                             //+
  int InterruptNumEvents;                                      //+
  int TestPattern;                                             //+
  CAEN_DGTZ_IOLevel_t FPIOtype;                                //+
  CAEN_DGTZ_TriggerMode_t ExtTriggerMode;                      //+
  uint16_t EnableMask;                                         //+
  CAEN_DGTZ_TriggerMode_t ChannelTriggerMode[MAX_SET];         //+
  CAEN_DGTZ_PulsePolarity_t PulsePolarity[MAX_SET];            //+
  uint32_t DCoffset[MAX_SET];                                  //+
  int32_t  DCoffsetGrpCh[MAX_SET][MAX_SET];                    //??????????????????????
  uint32_t Threshold[MAX_SET];                                 //+
  int Version_used[MAX_SET];                                   //+
  uint8_t GroupTrgEnableMask[MAX_SET];                         //+
  uint32_t MaxGroupNumber;                                     //+ 
        
  uint32_t FTDCoffset[MAX_SET];                                //+
  uint32_t FTThreshold[MAX_SET];                               //+
  CAEN_DGTZ_TriggerMode_t FastTriggerMode;                     //+
  CAEN_DGTZ_EnaDis_t      FastTriggerEnabled;                  //+
  int GWn;                                                     //+
  uint32_t GWaddr[MAX_GW];                                     //+
  uint32_t GWdata[MAX_GW];                                     //+
  uint32_t GWmask[MAX_GW];                                     //+
  uint16_t DecimationFactor;                                   //+
  int useCorrections;                                          //- only AUTO is implemented 
  int UseManualTables;                                         //- manual will come later
  char TablesFilenames[MAX_X742_GROUP_SIZE][1000];             //- (maybe)
  CAEN_DGTZ_DRS4Frequency_t DRS4Frequency;                     //+
  int StartupCalibration;                                      //- not relevant for X742
} WaveDumpConfig_t;

/* ###########################################################################
*  Functions
*  ########################################################################### */

int GetMoreBoardInfo(int handle, CAEN_DGTZ_BoardInfo_t BoardInfo, WaveDumpConfig_t *WDcfg);

void SetDefaultConfiguration(WaveDumpConfig_t *WDcfg);
int ParseConfigFile(FILE *f_ini, WaveDumpConfig_t *WDcfg);

int WriteRegisterBitmask(int32_t handle, uint32_t address, uint32_t data, uint32_t mask);

int ProgramDigitizer(int handle, WaveDumpConfig_t WDcfg, CAEN_DGTZ_BoardInfo_t BoardInfo);

#endif // _WAVEDUMP_FUNCTIONS__H
