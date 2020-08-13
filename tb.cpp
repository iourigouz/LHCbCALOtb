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

#include "vme_operations.h"
#include "digitizer_operations.h"

#include "ntp.h"

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

int start_run(const char* task){
  if(g_running){
    printf("%s: already running\n",__func__);
    return 1;
  }
  
  printf("%s: config name is %s\n",__func__,task);
  
  char fnam_param[128];
  sprintf(fnam_param,"%s.param",task);
    
  g_rp.reset();
  g_rp.read(fnam_param);
  if(!g_rp.init){
    printf("invalid config.\n");
    return 2;
  }
  
  if(0==strcmp(g_config,"on") ||
     0==strcmp(g_config,"off")
     ) return 0;
  
  if(0!=vme_init(6)){ // pulser length = 6*25 ns
    printf("cannot open VME, stop.\n");
    return 6;
  }
  
  char digitizer_param[128];
  sprintf(digitizer_param,"X742_%s.param",task);
  if(0!=digitizer_init(digitizer_param)){
    printf("cannot configure digitizer, stop\n");
    return 7;
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
      return 8;
    }
  }
  
  char fnam_param_task[256];
  sprintf(fnam_param_task,"%s/%s_%2.2d.param",task,task,g_runnumber);
  g_rp.write(fnam_param_task);
  
  sprintf(g_rootfilename,"%s/%s_%2.2d.root",task,task,g_runnumber);
  printf("opening root file %s ...",g_rootfilename);
  openROOTfile(g_rootfilename, &g_rp);
  printf(" ... done\n");
  
  dimsrv_createhistsvc();
  
  if(0!=digitizer_adjust_pedestals(g_rp.dig_adjust_offsets)){
    printf("%s: error in digitizer_adjust_pedestals\n",__func__); 
    //    return 12;
  }
  
  g_ievt=0, g_nped=0, g_nled=0, g_nsig=0;
  
  if(0!=vme_start()){ printf("%s: error in vme_start\n",__func__); return 10; }
  if(0!=digitizer_start()){printf("%s: error in digitizer_start\n",__func__); return 11; }
  
  g_running=true;
  return 0;
}

int stop_run(){
  if(0!=vme_stop()){
    printf("%s: error in vme_stop\n",__func__);
    return 1;
  }
  
  if(0!=digitizer_stop()){
    printf("%s: error in digitizer_stop\n",__func__);
    return 2;
  }
  
  if(0!=vme_close()){
    printf("%s: error in vme_close\n",__func__);
    return 3;
  }
  
  if(0!=digitizer_close()){
    printf("%s: error in digitizer_close\n",__func__);
    return 4;
  }
  
  dimsrv_deletehistsvc();
  
  closeROOTfile();
  printf(" Total %10.2f sec, %d events, %d ped, %d led, %d signal\n",
         g_t, g_ievt, g_nped, g_nled, g_nsig);
  
  g_running=false;

  return 0;
}

int exit_server(){
  if(g_running)stop_run();
  
  dimsrv_exit();
  
  int res=0;
  res=vme_close();
  res|=digitizer_close();
  
  return res;
}

int parse_command(const char* dimstr){
  if(!dimstr){
    //printf("%s: empty command\n",__func__);
    return 0;
  }
  
  char cmnd[256]; cmnd[255]='\0';
  int i=0;
  for(i=0; i<255 && (cmnd[i]=toupper(dimstr[i])); ++i){}
  printf("DIM > %s\n",dimstr);

  char w1[256]="",w2[256]="",w3[256]="",w4[256]="",w5[256]="";
  char w6[256]="",w7[256]="",w8[256]="",w9[256]="",w10[256]="";
  int nit=sscanf(cmnd,"%s %s %s %s %s %s %s %s %s %s", 
                 &w1, &w2, &w3, &w4, &w5, &w6, &w7, &w8, &w9, &w10);

  char e1[256]="",e2[256]="",e3[256]="",e4[256]="",e5[256]="";
  char e6[256]="",e7[256]="",e8[256]="",e9[256]="",e10[256]="";
  nit=sscanf(dimstr,"%s %s %s %s %s %s %s %s %s %s", 
             &e1, &e2, &e3, &e4, &e5, &e6, &e7, &e8, &e9, &e10);

  if(0==strcmp(w1,"EXITSRV") ){
    if(!g_running)g_exit=true;
    else printf("%s: cannot exit: running, stop run first\n",__func__);
  }
  else if(0==strcmp(w1,"STARTRUN")){
    if(nit>=2){
      memset(g_config,0,sizeof(g_config));
      strncpy(g_config,e2,sizeof(g_config)-5);
    }
    printf("%s: starting new run with config %s\n",__func__,g_config);
    g_startrun=true;
  }
  else if(0==strcmp(w1,"STOPRUN")){
    g_stoprun=true;
  }
  return 0;
}

int main(int argc, char *argv[]){
  ctrl_init();
  
  printf("Usage: ./tb <task name> \n");
  printf("Usage:      run config file is <task name>.param\n");
  printf("Usage:      digitizer config file is X742_<task name>.param\n");
  
  int resdim=dimsrv_init();
  if (0!=resdim){
    printf("%s: ERROR cannot start DIM, exiting.\n",__func__);
    return 1;
  }
  
  sprintf(g_config,"%s","default");
  if(argc>=2) strncpy(g_config,argv[1],sizeof(g_config)-7);
  printf("%s: config name is %s\n",__func__,g_config);
  int resstart=start_run(g_config);
  if(resstart!=0)return resstart;
  
  if(0==strcmp(g_config,"on") ||
     0==strcmp(g_config,"off")
     ) return 0;
  
  TTimeStamp tst0_tot, tst0_cmd, tst0_upd, tst0_print, tst0_write;
  double dt_tot=0, dt_cmd=0, dt_upd=0, dt_print=0, dt_write=0;
  
  int retwait=0;
  while(true){
    if(g_exit){
      int res=exit_server();
      g_exit=g_startrun=g_stoprun=false;
      return res;
    }
    if(g_stoprun && g_running){
      stop_run();
      g_exit=g_startrun=g_stoprun=false;
    }
    if(g_startrun && !g_running){
      resstart=start_run(g_config);
      if(resstart!=0)return resstart;
      g_exit=g_startrun=g_stoprun=false;
    }
    
    TTimeStamp tst_tot;    dt_tot=tst_tot.AsDouble() - tst0_tot.AsDouble();
    TTimeStamp tst_cmd;    dt_cmd=tst_cmd.AsDouble() - tst0_cmd.AsDouble();
    TTimeStamp tst_upd;    dt_upd=tst_upd.AsDouble() - tst0_upd.AsDouble();
    TTimeStamp tst_print;  dt_print=tst_print.AsDouble() - tst0_print.AsDouble();
    TTimeStamp tst_write;  dt_write=tst_write.AsDouble() - tst0_write.AsDouble();
    
    if(g_running){
      retwait=vme_wait((uint32_t)100); // timeout 100 msec
      if( (retwait & VME_WAIT_OK) ){
        if( VME_WAIT_SIG == (retwait & VME_WAIT_SIG) ){
          g_pattern=g_rp.SIGpatt;
          bool adcerr=false, tdcerr=false, digreaderr=false, digclearerr=false;
          
          if(0!=vme_readADC())      adcerr=true;
          if(0!=vme_readTDC())      tdcerr=true;
          if(0!=digitizer_read())   digreaderr=true;
          if(0!=digitizer_clear())  digclearerr=true;
          
          fill_all();
          g_ievt++; g_nsig++;
          
          if(adcerr)      printf("%s ev%6d: ret=0x%X, WARNING error vme_readADC, dt=%f\n",__func__,g_ievt,retwait,g_t-g_tprev);
          if(tdcerr)      printf("%s ev%6d: ret=0x%X, WARNING error vme_readTDC, dt=%f\n",__func__,g_ievt,retwait,g_t-g_tprev);
          if(digreaderr)  printf("%s ev%6d: ret=0x%X, WARNING error digitizer_read, dt=%f\n",__func__,g_ievt,retwait,g_t-g_tprev);
          if(digclearerr) printf("%s ev%6d: ret=0x%X, WARNING error digitizer_clear, dt=%f\n",__func__,g_ievt,retwait,g_t-g_tprev);
          
          if( (1==g_ievt) || (0==g_ievt%1000) )
            printf("Total of %10.2fs, %9d events, %9d signal, %6d LED, %6d ped\n", g_t, g_ievt, g_nsig, g_nled, g_nped);
        }
        if( VME_WAIT_LED == (retwait & VME_WAIT_LED) ){
          g_pattern=g_rp.LEDpatt;
          bool adcerr=false, tdcerr=false, digreaderr=false, digclearerr=false;
          
          if(0!=vme_pulseLED())     printf("%s ev%6d: ret=0x%X, WARNING error vme_pulseLED\n",__func__,g_ievt,retwait);
          usleep(10);
          
          if(0!=vme_readADC())      adcerr=true;     
          if(0!=vme_readTDC())      tdcerr=true;     
          if(0!=digitizer_read())   digreaderr=true; 
          if(0!=digitizer_clear())  digclearerr=true;
          
          fill_all();
          g_ievt++; g_nled++;
      
          if(adcerr)      printf("%s ev%6d: ret=0x%X, WARNING error vme_readADC, dt=%f\n",__func__,g_ievt,retwait,g_t-g_tprev);
          if(tdcerr)      printf("%s ev%6d: ret=0x%X, WARNING error vme_readTDC, dt=%f\n",__func__,g_ievt,retwait,g_t-g_tprev);
          if(digreaderr)  printf("%s ev%6d: ret=0x%X, WARNING error digitizer_read, dt=%f\n",__func__,g_ievt,retwait,g_t-g_tprev);
          if(digclearerr) printf("%s ev%6d: ret=0x%X, WARNING error digitizer_clear, dt=%f\n",__func__,g_ievt,retwait,g_t-g_tprev);
          
          if( (1==g_ievt) || (0==g_ievt%1000) ) printf("Total of %10.2fs, %9d events, %9d signal, %6d LED, %6d ped\n",
                   g_t, g_ievt, g_nsig, g_nled, g_nped);
        }
        if( VME_WAIT_PED == (retwait & VME_WAIT_PED) ){
          g_pattern=g_rp.PEDpatt;
          bool adcerr=false, tdcerr=false, digreaderr=false, digclearerr=false;
          
          if(0!=vme_pulsePED())     printf("%s ev%6d: ret=0x%X, WARNING error vme_pulsePED\n",__func__,g_ievt,retwait);
          usleep(10);
          
          if(0!=vme_readADC())      adcerr=true;     
          if(0!=vme_readTDC())      tdcerr=true;     
          if(0!=digitizer_read())   digreaderr=true; 
          if(0!=digitizer_clear())  digclearerr=true;
          
          fill_all();
          g_ievt++; g_nped++;
          
          if(adcerr)      printf("%s ev%6d: ret=0x%X, WARNING error vme_readADC, dt=%f\n",__func__,g_ievt,retwait,g_t-g_tprev);
          if(tdcerr)      printf("%s ev%6d: ret=0x%X, WARNING error vme_readTDC, dt=%f\n",__func__,g_ievt,retwait,g_t-g_tprev);
          if(digreaderr)  printf("%s ev%6d: ret=0x%X, WARNING error digitizer_read, dt=%f\n",__func__,g_ievt,retwait,g_t-g_tprev);
          if(digclearerr) printf("%s ev%6d: ret=0x%X, WARNING error digitizer_clear, dt=%f\n",__func__,g_ievt,retwait,g_t-g_tprev);
          
          if( (1==g_ievt) || (0==g_ievt%1000) ) printf("Total of %10.2fs, %9d events, %9d signal, %6d LED, %6d ped\n",
                   g_t, g_ievt, g_nsig, g_nled, g_nped);
        }
        
        if( VME_WAIT_SIG == (retwait & VME_WAIT_SIG) ){
          if(0!=vme_clearCORBO())   printf("%s ev%6d: WARNING error vme_clearCORBO\n",__func__,g_ievt);
        }
        // else // !!! IG
        if( (VME_WAIT_LED == (retwait & VME_WAIT_LED)) || (VME_WAIT_PED == (retwait & VME_WAIT_PED)) ){
          //int irqvec=0, irqlev=0;
          //vme_getirq(irqlev,irqvec);
          //if(0!=irqlev)printf("%s ev%6d irqlev %d, irqvec 0X%X, evkind %d\n",__func__,g_ievt,irqlev,irqvec,retwait%8);
          if(0!=vme_clearCORBO_2())   printf("%s ev%6d: WARNING error vme_clearCORBO_2\n",__func__,g_ievt);
        }
      }
      else if(VME_WAIT_TIMEOUT==retwait){// timeout, probably out of spill - check what useful can I do
      } 
      else if(VME_WAIT_BADVECT==retwait){// bad interrupt vector
        printf("%s ev%6d: wrong interrupt vector\n",__func__,g_ievt);
      } 
      else if(VME_WAIT_BADIRQ==retwait){// error in IRQ check
        printf("%s ev%6d: error in IRQ check\n",__func__,g_ievt);
      } 
      else if(VME_WAIT_BADACK==retwait){// error in IRQ acknowledge
        printf("%s ev%6d: error in IRQ acknowledge\n",__func__,g_ievt);
      } 
      else {
        printf("%s ev%6d: ERROR vme_wait()\n",__func__,g_ievt);
      }
    }
    else usleep(5000);
    
    if(dt_print>g_rp.printperiod){
      tst0_print=tst_print;
    }
    
    if(dt_write>g_rp.writeperiod){
      tst0_write=tst_write;
    }
    
    if(dt_cmd>g_rp.cmdperiod){
      tst0_cmd=tst_cmd;
      parse_command(dimsrv_getcommand());
    }
    
    if(dt_upd>g_rp.updperiod){
      tst0_upd=tst_upd;
      update_dimservstatus();
      if(g_running){
        dimsrv_update();
      }
    }
  }
  
  return exit_server();
}
