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
#include "wavedump_functions.h"
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
    printf("%s: invalid config.\n",__func__);
    return 2;
  }
  if( g_rp.ADC_used || g_rp.TDC_used ){
    printf("%s: not a digitizer calib config.\n",__func__);
    return 6;
  }
  
  char digitizer_param[128];
  sprintf(digitizer_param,"X742_%s.param",task);
  if(0!=digitizer_init(digitizer_param)){
    printf("%s: cannot configure digitizer, stop\n",__func__);
    return 7;
  }
  
  printf("%s: preparing environment...\n",__func__);
  struct stat st;
  if(0==stat(task,&st)){
    printf("%s: %s already exists\n",__func__,task);
    int runlast=getlastrunnumber(task);
    if(runlast>=0)g_runnumber=runlast+1;
    else g_runnumber=0;
  }
  else{
    g_runnumber=0;
    
    if(0!=mkdir(task,0777)){
      printf("%s: cannot create directory %s, stop\n",__func__,task);
      return 8;
    }
  }
  
  char fnam_param_task[256];
  sprintf(fnam_param_task,"%s/%s_%2.2d.param",task,task,g_runnumber);
  g_rp.write(fnam_param_task);
  
  sprintf(g_rootfilename,"%s/%s_%2.2d.root",task,task,g_runnumber);
  printf("%s: opening root file %s ...",__func__,g_rootfilename);
  openROOTfile(g_rootfilename, &g_rp);
  printf(" ... done\n");
  
  dimsrv_createhistsvc();
  
  if(0!=digitizer_adjust_pedestals(g_rp.dig_adjust_offsets)){
    printf("%s: error in digitizer_adjust_pedestals\n",__func__); 
    //    return 12;
  }
  
  g_ievt=0, g_nped=0, g_nled=0, g_nsig=0;
  
  //  if(0!=digitizer_start()){printf("%s: error in digitizer_start\n",__func__); return 11; }
  
  g_running=true;
  return 0;
}

int stop_run(){
  if(0!=digitizer_stop()){
    printf("%s: error in digitizer_stop\n",__func__);
    return 2;
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
  
  printf("Usage: ./calib <task name> \n");
  printf("Usage:         run config file is <task name>.param\n");
  printf("Usage:         digitizer config file is X742_<task name>.param\n");
  
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
      g_pattern=g_rp.LEDpatt;
      bool digstarterr=false, digtrigerr=false, digreaderr=false, digclearerr=false, digstoperr=false;
      
      if(0!=digitizer_start())  digstarterr=true; 
      if(0!=digitizer_SWtrg())  digtrigerr=true; 
      usleep(100);
      if(0!=digitizer_read())   digreaderr=true; 
      if(0!=digitizer_clear())  digclearerr=true;
      if(0!=digitizer_stop())  digstarterr=true; 
      
      fill_all();
      g_ievt++; g_nled++;
      
      if(digstarterr) printf("%s ev%6d: WARNING error digitizer_start, dt=%f\n",__func__,g_ievt,g_t-g_tprev);
      if(digstoperr)  printf("%s ev%6d: WARNING error digitizer_stop, dt=%f\n",__func__,g_ievt,g_t-g_tprev);
      if(digtrigerr)  printf("%s ev%6d: WARNING error digitizer_SWtrg, dt=%f\n",__func__,g_ievt,g_t-g_tprev);
      if(digreaderr)  printf("%s ev%6d: WARNING error digitizer_read, dt=%f\n",__func__,g_ievt,g_t-g_tprev);
      if(digclearerr) printf("%s ev%6d: WARNING error digitizer_clear, dt=%f\n",__func__,g_ievt,g_t-g_tprev);
      
      if( (1==g_ievt) || (0==g_ievt%1000) ) printf("Total of %10.2fs, %9d events\n", g_t, g_ievt);
      
      // now sleep according to g_rp.LEDperiod
      if(g_rp.LEDperiod>0.0001){
        __useconds_t nusec=g_rp.LEDperiod*1e6;
        usleep(nusec);
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
