#include "SpecsUser.h"
//#include "PlxTypes.h"

#include <cstdlib>
#include <curses.h>
#include <unistd.h>

#include "runparam.h"
#include "specs_operations.h"

DEVICE_INVENT deviceList[ MAX_CARD ] ;
U8 g_nSpecs=0;

SPECSMASTER MasterPorts[12];
bool masterPortIsOpen[12] = 
  {false, false, false, false, 
   false, false, false, false, 
   false, false, false, false};
int g_portDAC=0, g_portINT=0;
int g_slaveDAC=0, g_slaveINT=0;

SPECSSLAVE DACslave; // only one DAC bd
SPECSSLAVE INTslave; // only one INT bd

unsigned char g_DAC_addCNT   = 0xF8;
unsigned char g_DAC_addDAT   = 0xF9;
unsigned char g_DAC_fst_last = 0xFA;
unsigned char g_DAC_rump_hv  = 0xFB;
unsigned char g_DAC_rump_tim = 0xFC;
unsigned char g_DAC_c_pwr1   = 0xFD;
unsigned char g_DAC_addADC   = 0xFE;
unsigned char g_DAC_addCSR   = 0xFF;

bool g_INT_installed=true;

unsigned char g_INT_addCNT   = 0x00;
unsigned char g_INT_addDAT   = 0x01;
unsigned char g_INT_addDAC   = 0x02;
unsigned char g_INT_addCSR   = 0x03;
unsigned char g_INT_addTIM   = 0x04;
unsigned char g_INT_addBRD   = 0x05;

bool g_INT_calib_ON=false;
bool g_INT_block_mode=true;

int specs_open(int iportDAC, int iportINT, int islaveDAC, int islaveINT)
{
  if(!g_rp.SPECS_used)return 0;
  
  int res;

  SpecsError theSpError; 
  
  if(0==g_nSpecs){
    U8 nSpecs = specs_master_card_select( deviceList ) ;
    if ( 0 == nSpecs ) {printf( "%s: no SPECS master Card found !!!\n", __FUNCTION__); return -1; }
  }
  
  res=specs_init_DAC(iportDAC, islaveDAC);
  if(res!=0)return res;
  res=specs_init_INT(iportINT, islaveINT);
  if(res!=0) g_INT_installed=false;
  
  return 0;
}

int specs_readINT(int iconn, int rng, int nchan, int* currs){
  if(!g_rp.SPECS_used)return 0;
  
  if(!g_INT_installed)return 0;
  if(iconn<0 || iconn>12)return 0;
  
  SpecsError theSpError;
  unsigned short w_dat=0, r_dat=0;
  
  w_dat=0x4000 | iconn<<6 ;  // for CALIB OFF mode
  if(g_INT_calib_ON) w_dat=0x4001 | iconn<<6 | rng<<1 ;
  theSpError=specs_parallel_write(&INTslave, g_INT_addCSR, 1, &w_dat); // start measurement
  if(0!=theSpError){printf("%s: cannot start measurement\n", __func__); return -1;}
  
  int j;
  for(j=0; j<10000; ++j){
    theSpError=specs_parallel_read(&INTslave, g_INT_addCSR, 1, &r_dat);  // read  CSR 
    if(0!=theSpError){printf("%s: cannot read CSR\n", __func__); return -1;}
    if ((r_dat & 0x2000) == 0)break; // end of digitization, go readout
  }
  if(j>9990){printf("%s: stuck at digitization\n", __func__); return -1;}
  
  if(!g_INT_block_mode){
    for ( int i=0; i<nchan; ++i) {	// readout results
      w_dat=i;
      theSpError = specs_parallel_write( &INTslave, g_INT_addCNT, 1, &w_dat);  // write address to CNT 
      if(0!=theSpError){printf("%s: cannot write CNT\n", __func__); return -1;}
      theSpError = specs_parallel_read( &INTslave, g_INT_addDAT, 1, &r_dat) ;  // read  MEM 
      if(0!=theSpError){printf("%s: cannot read DAT\n",__func__); return -1;}
      currs[i]=r_dat;
    }
  }
  else{
    w_dat=0x400 ;  // for CALIB OFF mode
    if(g_INT_calib_ON) w_dat=0x401 | iconn<<6 | rng<<1 ;
    theSpError=specs_parallel_write(&INTslave, g_INT_addCSR, 1, &w_dat); // block mode on
    if(0!=theSpError){printf("%s: cannot set block mode\n",__func__); return -1;}
    
    unsigned short ddd[100];
    theSpError = specs_parallel_read( &INTslave, g_INT_addDAT, (U8)nchan, ddd) ;  // read  MEM 
    if(0!=theSpError){printf("%s: cannot read in block mode\n",__func__); return -1;}
    
    w_dat=0x0 ;  // for CALIB OFF mode
    if(g_INT_calib_ON) w_dat=0x1 | iconn<<6 | rng<<1 ;
    theSpError=specs_parallel_write(&INTslave, g_INT_addCSR, 1, &w_dat); // block mode off
    if(0!=theSpError){printf("%s: cannot unset block mode\n",__func__); return -1;}
    
    for(int j=0; j<nchan; ++j)currs[j]=ddd[j];
  }
  for(int j=0; j<nchan; ++j)currs[j]&=0xFFF;
  
  return 0;
}

int specs_setINTRange(int iconn, int integ, int rng){
  if(!g_rp.SPECS_used)return 0;
  
  if(!g_INT_installed)return 0;
  
  SpecsError theSpError;
  unsigned short w_dat=0, r_dat=0;
  
  w_dat=(iconn << 6) + (integ << 3);
  theSpError = specs_parallel_write(&INTslave, g_INT_addCNT, 1, &w_dat) ;   
  if(0!=theSpError){printf("%s: cannot write (1) to INT CNT\n",__func__); return -1;}
  
  w_dat=0x8 + (rng << 1);
  theSpError = specs_parallel_write(&INTslave, g_INT_addCSR, 1, &w_dat) ;  
  if(0!=theSpError){printf("%s: cannot write (1) to INT CSR\n",__func__); return -1;}
  
  w_dat=(iconn << 6) + (integ << 3);
  theSpError = specs_parallel_write(&INTslave, g_INT_addCNT, 1, &w_dat) ;   
  if(0!=theSpError){printf("%s: cannot write (2) to INT CNT\n",__func__); return -1;}
  
  w_dat=0x9 + (rng << 1);
  theSpError = specs_parallel_write(&INTslave, g_INT_addCSR, 1, &w_dat) ;  
  if(0!=theSpError){printf("%s: cannot write (2) to INT CSR\n",__func__); return -1;}
  
  w_dat=0x8 + (rng << 1);
  theSpError = specs_parallel_write(&INTslave, g_INT_addCSR, 1, &w_dat) ;  
  if(0!=theSpError){printf("%s: cannot write (3) to INT CSR\n",__func__); return -1;}
  
  return 0;
}

int specs_setINTDAC(int idac){
  if(!g_rp.SPECS_used)return 0;
  
  if(!g_INT_installed)return 0;
  
  SpecsError theSpError;
  unsigned short w_dat=0, r_dat=0;
  
  w_dat=idac & 0x0FFF;
  theSpError = specs_parallel_write(&INTslave, g_INT_addDAC, 1, &w_dat);
  if(0!=theSpError){printf("%s: cannot write INT DAC\n",__func__); return -1;}
  theSpError = specs_parallel_read(&INTslave, g_INT_addDAC, 1, &r_dat);
  if(0!=theSpError){printf("%s: cannot read back INT DAC\n",__func__); return -1;}
  if(r_dat!=w_dat){printf("%s: DAC read != write\n",__func__);return -1;}
  
  w_dat=0x1000;
  if(g_INT_calib_ON) w_dat=0x1001;
  theSpError = specs_parallel_write(&INTslave, g_INT_addCSR, 1, &w_dat) ;  
  if(theSpError){printf("%s: cannot set INT DAC\n",__func__); return -1;}
  
  return 0;
}

int specs_init_INT(int iportINT, int islaveINT){
  if(!g_rp.SPECS_used)return 0;
  
  if(!g_INT_installed)return 0;
  
  SpecsError theSpError;
  
  if(0==g_nSpecs){
    U8 nSpecs = specs_master_card_select( deviceList ) ;
    if ( 0 == nSpecs ) {printf( "%s: no SPECS master Card found !!!\n", __FUNCTION__); return -1; }
  }
  
  if(iportINT<=0 || islaveINT<=0) {
    g_INT_installed=false;
    return 0;
  }
  if(!masterPortIsOpen[iportINT]){
    // opening master
    theSpError = specs_master_open( deviceList[0] , iportINT , &MasterPorts[iportINT] );
    if(0!=theSpError){printf("%s: cannot open INT master port (%d)\n",__func__,iportINT); return -1;}
    
    theSpError=specs_master_reset(&MasterPorts[iportINT]);
    if(0!=theSpError){printf("%s: cannot reset INT master port (%d)\n",__func__,iportINT);return -1;}
    
    U8 speed = 0; //  0,1,2,3,4  -> 1024,512,256 kHz 
    SpecsmasterCtrlWrite( &MasterPorts[iportINT] , ClockDivisionBits & speed ) ;
  }
  
  // opening slave
  theSpError=specs_slave_open( &MasterPorts[iportINT], islaveINT, &INTslave ) ;
  if(0!=theSpError){printf("%s: cannot open INT slave (%d)\n",__func__,islaveINT);return -1;}
  
  //   Initialize INT board
  theSpError = specs_slave_external_shortreset(&INTslave) ;
  if(0!=theSpError){printf("%s: cannot shortreset (1) INT SPECS slave (%d)\n",__func__,islaveINT);return -1;}
  
  unsigned char csr=g_INT_addCSR;
  
  U16 dat=0x8000;  // write 0x8000 to CSR -- board RESET
  theSpError = specs_parallel_write( &INTslave, csr, 1, &dat ) ;
  if( 0 != theSpError ){printf("%s: cannot reset INT control mezzanine\n",__func__);return -1;}
  
  // check board: write/read CSR
  unsigned short rData=0, wData = 0x0F;
  theSpError = specs_parallel_write (&INTslave, csr, 1, &wData);  
  if( 0 != theSpError ){printf("%s: cannot write INT CSR\n",__func__);return -1;}
  theSpError = specs_parallel_read  (&INTslave, csr, 1, &rData);  
  if( 0 != theSpError ){printf("%s: cannot read INT CSR\n",__func__);return -1;}
  if ( (wData & 0xDFFF) != (rData & 0xDFFF) ){printf("%s: CSR read != write: write 0x%04X, read 0x%04X\n",__func__,wData, rData);return -1;}
  if ( 0 != (rData & 0x2000) ){printf("%s WARNING: CSR bit 13 (busy digitising) is UP; continuing but fingers crossed\n",__func__);};
  
  g_portINT=iportINT;
  masterPortIsOpen[iportINT]=true;
  
  int res=specs_setINTDAC(0);
  if(res!=0){printf("%s: cannot execute setINTDAC(0)\n",__func__);return -1;}
  
  res=specs_setINTRange(0,0,1);
  if(res!=0){printf("%s: cannot execute setINTRange(0,0,1)\n",__func__);return -1;}
  res=specs_setINTRange(0,1,1);
  if(res!=0){printf("%s: cannot execute setINTRange(0,1,1)\n",__func__);return -1;}
  
  return 0;
}

int specs_init_DAC(int iportDAC, int islaveDAC){
  if(!g_rp.SPECS_used)return 0;
  
  SpecsError theSpError;
  
  if(0==g_nSpecs){
    U8 nSpecs = specs_master_card_select( deviceList ) ;
    if ( 0 == nSpecs ) {printf( "%s: no SPECS master Card found !!!\n", __FUNCTION__); return -1; }
  }
  
  if(iportDAC<=0 || islaveDAC<=0) return 0;
  
  if(!masterPortIsOpen[iportDAC]){
    // opening master
    theSpError = specs_master_open( deviceList[0] , iportDAC , &MasterPorts[iportDAC] );
    if(0!=theSpError){printf("%s: cannot open DAC master port (%d)\n",__func__,iportDAC); return -1;}
    
    theSpError=specs_master_reset(&MasterPorts[iportDAC]);
    if(0!=theSpError){printf("%s: cannot reset DAC master port (%d)\n",__func__,iportDAC);return -1;}
    
    U8 speed = 0; //  0,1,2,3,4  -> 1024,512,256 kHz 
    SpecsmasterCtrlWrite( &MasterPorts[iportDAC] , ClockDivisionBits & speed ) ;
  }
  
  // opening slave
  theSpError=specs_slave_open( &MasterPorts[iportDAC], islaveDAC, &DACslave ) ;
  if(0!=theSpError){printf("%s: cannot open DAC slave (%d)\n",__func__,islaveDAC);return -1;}
  
  //   Initialize DAC board
  theSpError = specs_slave_external_shortreset(&DACslave) ;
  if(0!=theSpError){printf("%s: cannot shortreset (1) DAC SPECS slave (%d)\n",__func__,islaveDAC);return -1;}
  
  unsigned char csr=g_DAC_addCSR;
  U16 dat=0x8000, datr=0;  // write dat=0x8000 to CSR -- board RESET
  theSpError = specs_parallel_write( &DACslave, csr, 1, &dat );
  if( 0 != theSpError ){printf("%s: cannot reset DAC control mezzanine\n",__func__);return -1;}
  
  // check board: read CSR
  theSpError = specs_parallel_read(&DACslave, csr, 1, &dat);
  if(0!=theSpError){printf("%s: cannot read DAC CSR\n",__func__);return -1;}
  
  // DCU init
  theSpError = specs_slave_external_shortreset(&DACslave) ;
  if(0!=theSpError){printf("%s: cannot shortreset (2) DAC slave (%d)\n",__func__,islaveDAC);return -1;}
  theSpError = specs_dcu_reset(&DACslave);
  if(0!=theSpError){printf("%s: cannot reset DCU of the DAC board\n",__func__);return -1;}
  theSpError = specs_dcu_initialize(&DACslave);
  if(0!=theSpError){printf("%s: cannot initialize DCU of the DAC board\n",__func__);return -1;}
  
  U8 DCUmode=0;
  theSpError = specs_dcu_read_mode(&DACslave, &DCUmode);
  if(0!=theSpError){printf("%s: cannot read (1) DCU mode\n",__func__,islaveDAC);return -1;}
  theSpError = specs_dcu_set_LIR(&DACslave);
  if (0!=theSpError){printf("%s: cannot set LIR DCU mode\n",__func__);return -1;}
  theSpError = specs_dcu_read_mode(&DACslave, &DCUmode ) ;
  if(0!=theSpError){printf("%s: cannot read (2) DCU mode\n",__func__);return -1;}
  if(1!=DCUmode){printf("%s: cannot read back LIR DCU mode\n",__func__);return -1;}
  
  // switch on all the 100V relays
  unsigned char pwr=g_DAC_c_pwr1;
  dat=3 << 8; // autoprotect reaction time
  unsigned char rumptim=g_DAC_rump_tim;
  theSpError = specs_parallel_write(&DACslave, rumptim, 1, &dat ) ;
  if(0!=theSpError){printf("%s: cannot write DAC ramp time\n",__func__);return -1;}
  theSpError = specs_parallel_read( &DACslave, rumptim, 1, &datr) ;
  if(0!=theSpError){printf("%s: cannot read DAC ramp time\n",__func__);return -1;}
  if(dat!=datr){printf("%s: cannot read back DAC ramp time\n",__func__);return -1;}
  
  dat=2;      // set autoprotect on
  theSpError = specs_parallel_write( &DACslave, csr, 1, &dat ) ;
  if(0!=theSpError){printf("%s: cannot write CSR\n",__func__);return -1;}
  theSpError = specs_parallel_read( &DACslave, pwr, 1, &datr); 
  if(0!=theSpError){printf("%s: cannot read PWR\n",__func__);return -1;}
  
  g_portDAC=iportDAC;
  masterPortIsOpen[iportDAC]=true;
  
  return 0;
}


int specs_close(){
  if(!g_rp.SPECS_used)return 0;
  
  int ret=0;
  SpecsError theSpError; 
  
  if(masterPortIsOpen[g_portDAC]){
    theSpError = specs_master_close(&MasterPorts[g_portDAC]);
    if(0!=theSpError){printf("%s: cannot close DAC port %d\n",__func__,g_portDAC);ret=-1;}
    masterPortIsOpen[g_portDAC]=false;
  }
  
  if(masterPortIsOpen[g_portINT]){
    theSpError = specs_master_close(&MasterPorts[g_portINT]);
    if(0!=theSpError){printf("%s: cannot close INT port %d\n",__func__,g_portINT);ret=-1;}
    masterPortIsOpen[g_portINT]=false;
  }
  
  return ret;
}

int specs_writeDAC(int* DAC){
  if(!g_rp.SPECS_used)return 0;
  
  for(int i=0; i<12; ++i){
    int res=specs_setDACchan(i,DAC[i]);
    if(0!=res){printf("%s: cannot write %d into chan %d\n",__func__,DAC[i],i);return -1;}
  }
  return 0;
}

int specs_setDACchan(int ichan, int iDAC){
  if(!g_rp.SPECS_used)return 0;
  
  if(ichan<0 || ichan>215)return 0;

  unsigned char ch=ichan;
  unsigned short cnt=iDAC&0xFFF;
  SpecsError theSpError;
  
  // write data to Channel 
  theSpError=specs_parallel_write(&DACslave, ch, 1, &cnt);
  if(0!=theSpError){printf("%s: cannot write DAC value %d into chan %d\n",__func__,iDAC,ichan);return -1;}
  // check address transmission
  unsigned short ch_o;
  theSpError=specs_parallel_read(&DACslave, g_DAC_addCNT, 1, &ch_o);
  if(0!=theSpError){printf("%s: cannot read back chan#\n",__func__);return -1;}
  if(ch_o!=ch){printf("%s: wrong chan# read back\n",__func__);return -1;}
  // check data transmission
  unsigned short cnt_o;
  theSpError=specs_parallel_read(&DACslave, g_DAC_addDAT, 1, &cnt_o);
  if(0!=theSpError){printf("%s: cannot read back DAC value#\n",__func__);return -1;}
  if(cnt_o!=cnt){printf("%s: wrong DAC value read back\n",__func__);return -1;}
  // write 0x100 to CSR - execute set HV; set to 0x102 to set autoprotect on
  unsigned short cmd=0x102;
  theSpError=specs_parallel_write(&DACslave, g_DAC_addCSR, 1, &cmd);
  if(0!=theSpError){printf("%s: cannot send command to execute write DAC#\n",__func__);return -1;}
  
  return 0;
}

int write_HV(){
  if(!g_rp.SPECS_used)return 0;
  
  int res=0;
  for(int i=0; i<MAXCHANS; ++i){
    if(g_rp.HVchan[i]>=0){
      double uDAC=g_rp.HV[i]/0.428;
      int iDAC=uDAC * 4095/5;
      res|=specs_setDACchan(g_rp.HVchan[i],iDAC);
    }
  }
  return res;
}

int write_ULED(){
  if(!g_rp.SPECS_used)return 0;
  
  int res=0;
  for(int i=0; i<MAXLEDS; ++i){
    int iLED=g_rp.ULED[i] * 4095/5;
    res|=specs_setDACchan(g_rp.LEDchan[i],iLED);
  }
  return res;
}
