// stab.cpp : Defines the entry point for the console application.
//

#include <string>
#include <iostream>
#include <stdio.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <stdlib.h>

#include <errno.h>
#include <dirent.h>

#include "signal.h"

#include "TROOT.h"
#include "TDatime.h"
#include "TTimeStamp.h"
#include "TH1I.h"
#include "TH1D.h"
#include "TTree.h"

#include "specs_operations.h"
#include "vme_operations.h"
#include "digitizer_operations.h"

#include "ntp.h"

bool g_exit=false;
int g_print=0;
bool g_sleeping=false;

int g_runnumber=0;

int getlastrunnumber(const char* task){
  // this is to find the last run number in the directory "task"
  // the file name format is task_<run_number>.root
  
  //unsigned char isFile =0x8, isFolder =0x4;
  DIR *dp;
  struct dirent *dent;
  int retval=-1;
  
  struct stat st;
  if(0!=stat(task,&st)) return -1;
  
  if( (dp = opendir(task)) == NULL) {
    fprintf(stderr, "%s: opendir error, %s: %s\n", __func__, task, strerror(errno));
    return -1;
  }
  
  errno=0;
  while( (dent = readdir(dp)) ){
    if(DT_REG==dent->d_type){
      const char* taskpos=strstr(dent->d_name,task);
      const char* dotroot=strstr(dent->d_name,".root");
      if(taskpos && dotroot){
        const char* underscore=dotroot;
        for(; '_'!=*underscore && (dent->d_name)!=underscore; underscore--){;};
        if('_'==*underscore){
          int j;
          int nit=sscanf(underscore+1,"%d",&j);
          if(1==nit && j>retval)retval=j;
        }
      }
    }
  }
  return retval;
}

void ctrc_hdl(int s){
  printf("\nCaught signal %d, exiting...\n",s);
  g_exit=true;
}

void ctrd_hdl(int s){
  printf("\nCaught signal %d\n",s);
  //g_exit=true;
  g_print=(g_print+1)%4;
}

void ctrz_hdl(int s){
  //printf("\nCaught signal %d\n",s);
  g_print=(g_print+1)%4;
}

std::string exec(const char* cmd) {
  FILE* pipe = popen(cmd, "r");
  if (!pipe) return "ERROR";
  char buffer[128];
  std::string result = "";
  while(!feof(pipe)) {
    if(fgets(buffer, 128, pipe) != NULL)
      result += buffer;
  }
  pclose(pipe);
  return result;
}

void ctrl_init(){
  //-------------
  struct sigaction sigIntHandler;
  sigIntHandler.sa_handler = ctrc_hdl;
  sigemptyset(&sigIntHandler.sa_mask);
  sigIntHandler.sa_flags = 0;
  sigaction(SIGINT, &sigIntHandler, NULL);
  
  struct sigaction sigQuitHandler;
  sigQuitHandler.sa_handler = ctrd_hdl;
  sigemptyset(&sigQuitHandler.sa_mask);
  sigQuitHandler.sa_flags = 0;
  sigaction(SIGQUIT, &sigQuitHandler, NULL);
  
  struct sigaction sigTstpHandler;
  sigTstpHandler.sa_handler = ctrz_hdl;
  sigemptyset(&sigTstpHandler.sa_mask);
  sigTstpHandler.sa_flags = 0;
  sigaction(SIGTSTP, &sigTstpHandler, NULL);
  //-----------
}


int main(int argc, char *argv[]){
  ctrl_init();
  
  printf("Usage: ./tb <task name> \n");
  printf("Usage:      run config file is <task name>.param\n");
  printf("Usage:      off: switch HV off and exit\n");
  
  if(argc<2){
    printf("task name is missing, stop.\n");
    return 0;
  }
  
  char task[128];
  strncpy(task,argv[1],sizeof(task)-7);
  printf("Task name is %s\n",task);
  
  char fnam_param[128];
  sprintf(fnam_param,"%s.param",argv[1]);
    
  g_rp.reset();
  
  g_rp.read(fnam_param);
  
  if(!g_rp.init){
    printf("invalid run parameters, stop.\n");
    return 0;
  }
  
  if(0!=specs_open(2,-1,2,-1)){
    printf("cannot open SPECS, stop.\n");
    return 1;
  }
  
  if(0!=write_HV()){
    printf(" cannot write HV, stop.\n");
    return 1;
  }
  
  if(0!=write_ULED()){
    printf(" cannot write ULED, stop.\n");
    return 1;
  }
  
  if(0!=vme_init(6)){
    printf("cannot open VME, stop.\n");
    return 1;
  }
  
  char digitizer_param[128];
  sprintf(digitizer_param,"DT5742_%s.param",argv[1]);
  if(0!=digitizer_init(digitizer_param)){
    printf("cannot configure digitizer, stop\n");
    return 1;
  }
  
  printf("preparing environment...\n");
  struct stat st;
  if(0==stat(task,&st)){
    printf("%s already exists\n",task);
    int runlast=getlastrunnumber(task);
    if(runlast>=0)g_runnumber=runlast+1;
    else g_runnumber=0;
  }
  else{
    g_runnumber=0;
    
    if(0!=mkdir(task,0777)){
      printf("cannot create directory %s, stop\n",task);
      return 1;
    }
  }
  
  /*if(0!=chdir(task)){
    printf("cannot cd to %s, stop\n",task);
    return 1;
    }*/
  
  char fnam_param_task[256];
  sprintf(fnam_param_task,"%s/%s_%2.2d.param",task,task,g_runnumber);
  g_rp.write(fnam_param_task);
  
  char rootfilename[128];
  sprintf(rootfilename,"%s/%s_%2.2d.root",task,task,g_runnumber);
  printf("opening root file %s ...",rootfilename);
  openROOTfile(rootfilename, &g_rp);
  printf(" ... done\n");
  
  char* patterns[]={"PED","LED","INT"};
  int iev=0, nped=0, nled=0, nsig=0;
  int res=0;
  
  TTimeStamp tst0_tot;
  TTimeStamp tst0_print;
  TTimeStamp tst0_write;
  double dt_tot=0, dt_write=0, dt_print=0;
  
  if(0!=vme_start()){
    printf("%s: error in vme_start\n",__func__);
    return 1;
  }
  
  if(0!=digitizer_start()){
    printf("%s: error in digitizer_start\n",__func__);
    return 1;
  }
  
  while(true){
    if(g_exit)break;
    TTimeStamp tst_tot;    dt_tot=tst_tot.AsDouble() - tst0_tot.AsDouble();
    TTimeStamp tst_print;  dt_print=tst_print.AsDouble() - tst0_print.AsDouble();
    
    int ret=vme_wait((uint32_t)1000); // timeout 100 msec
    if(0==ret){
      if(0!=vme_read_pattern()) printf("%s ev%6d: WARNING error vme_read_pattern\n",__func__,iev);
      if(0!=vme_readADC())      printf("%s ev%6d: WARNING error vme_readADC\n",__func__,iev);
      if(0!=vme_readTDC())      printf("%s ev%6d: WARNING error vme_readTDC\n",__func__,iev);
      //usleep(1000);
      if(0!=digitizer_read())   printf("%s ev%6d: WARNING error digitizer_read\n",__func__,iev);
      if(0!=digitizer_clear())  printf("%s ev%6d: WARNING error digitizer_clear_data\n",__func__,iev);
      
      if(0!=vme_clearCORBO())   printf("%s ev%6d: WARNING error vme_clearCORBO\n",__func__,iev);
      
      fill_all();
      
      iev++;
      if(g_rp.PEDpatt==g_pattern)nped++;
      else if(g_rp.LEDpatt==g_pattern)nled++;
      else if(g_rp.SIGpatt==g_pattern)nsig++;
      
      if( (1==iev) || (0==iev%1000) )
        printf("Total of %10.2fs, %9d events, %9d signal, %6d LED, %6d ped\n",
               dt_tot, iev, nped, nled, nsig);
      
    }
    else if(ret>100){// timeout, probably out of spill - check what useful can I do
    }
    else {
      printf("%s ev%6d: ERROR vme_wait()\n",__func__,iev);
    }
    
    if(dt_print>g_rp.printperiod){
      tst0_print=tst_print;
    }
  }
  
 StopRun:
  if(0!=vme_stop()){
    printf("%s: error in vme_stop\n",__func__);
    return 1;
  }
  
  if(0!=digitizer_stop()){
    printf("%s: error in vme_stop\n",__func__);
    return 1;
  }
  
  closeROOTfile();
  printf(" Total %10.2f sec, %d events, %d ped, %d led, %d signal\n",
         dt_tot, iev, nped, nled, nsig);
    
 End:  
  res=vme_close();
  res|=specs_close();
  res|=digitizer_close();
  
  return res;
}
