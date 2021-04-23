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

#include "ntp.h"

int getlastrunnumber1(const char* task){
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

const char* lastoccurrence(const char* str, const char* substr){
  const char *ret, *pnt;
  ret=pnt=strstr(str,substr);
  while(pnt){
    ret=pnt;
    pnt=strstr(pnt+1,substr);
  }
  return ret;
}

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
  
  char task_[256];
  sprintf(task_,"%s_",task);
  
  errno=0;
  while( (dent = readdir(dp)) ){
    if(DT_REG==dent->d_type){
      const char* task_pos=lastoccurrence(dent->d_name,task_);
      const char* dotroot=lastoccurrence(dent->d_name,".root");
      if(task_pos && dotroot){
        if(0==strcmp(dotroot,".root")){
          int j;
          int nit=sscanf(task_pos+strlen(task_),"%d",&j);
          if(1==nit && j>retval)retval=j;
        }
      }
    }
  }
  return retval;
}

int getlastversionnumber(const char* task, int irun){
  // this is to find the last version number of <irun>  in the directory "task"
  // the file name format is task/task_<irun>.v<version>.root
  
  DIR *dp;
  struct dirent *dent;
  int retval=-1;
  
  struct stat st;
  if(0!=stat(task,&st)) return -1;
  
  if( (dp = opendir(task)) == NULL) {
    fprintf(stderr, "%s: opendir error, %s: %s\n", __func__, task, strerror(errno));
    return -1;
  }
  
  char task_run[256];
  sprintf(task_run,"%s_%2.2d",task,irun);
  
  errno=0;
  while( (dent = readdir(dp)) ){
    if(DT_REG==dent->d_type){
      const char* task_pos=lastoccurrence(dent->d_name,task_run);
      const char* dotroot=lastoccurrence(dent->d_name,".root");
      const char* dotv=lastoccurrence(dent->d_name,".v");
      if(task_pos && dotroot && dotv){
        if(0==strcmp(dotroot,".root")){
          int j;
          int nit=sscanf(dotv+strlen(".v"),"%d",&j);
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
  g_print=(g_print+1)%3;
}

void ctrz_hdl(int s){
  printf("\nCaught signal %d\n",s);
  g_print=(g_print+1)%3;
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
  
  printf("Usage: ./bin2cpp <config name> <run number> \n");
  
  if(argc<3){
    printf("too few parameters, stop.\n");
    return 0;
  }
  
  strncpy(g_config,argv[1],sizeof(g_config)-7);
  int nit=sscanf(argv[2],"%d",&g_runnumber);
  if(nit<1){
    printf("bad run number, stop.\n");
    return 0;
  }
  
  struct stat st;
  
  sprintf(g_binfilename,"%s/%s_%2.2d.bin",g_config,g_config,g_runnumber);
  if(0!=stat(g_binfilename,&st)){
    printf("bin file does not exist, stop.\n");
    return 0;
  }
  
  char fnam_param[256];
  sprintf(fnam_param,"%s/%s_%2.2d.param",g_config,g_config,g_runnumber);
  
  printf("%s: opening bin file %s for read ...",__func__,g_binfilename);
  int res=openBINfile_r(g_binfilename);
  if(0!=res){
    printf(" ... ERROR, stop.\n");
    return 1;
  }
  else printf(" ... done\n");
  //
  if(!g_rp.init){ // the header with run params is invalid
    printf("%s: invalid param record in %s, will now try a param file\n",__func__,g_binfilename);
    if(0!=stat(fnam_param,&st)){
      printf("param file does not exist, stop.\n");
      return 0;
    }
    g_rp.reset();
    g_rp.read(fnam_param);
    if(!g_rp.init){
      printf("%s: invalid config.\n",__func__);
      return 2;
    }
    // now rewind the bin file to the beginning
    rewind_binfile();
  }
  //
  if(0!=stat(fnam_param,&st)){
    printf("%s: %s not found, creating\n",__func__,fnam_param);
    g_rp.write(fnam_param);
  }
  //
  sprintf(g_rootfilename,"%s/%s_%2.2d.root",g_config,g_config,g_runnumber);
  if(0==stat(g_rootfilename,&st)){
    int vers=getlastversionnumber(g_config,g_runnumber);
    char rootfilename_new[256];
    sprintf(rootfilename_new,"%s/%s_%2.2d.v%2.2d.root",g_config,g_config,g_runnumber,vers+1);
    printf("copying the existing root file into version %2.2d ... ",vers+1);
    char cmd[256];
    sprintf(cmd,"cp %s %s\n",g_rootfilename, rootfilename_new);
    exec(cmd);
    printf(" ... done\n");
    //char fnam_param_new[256];
    //sprintf(fnam_param_new,"%s/%s_%2.2d.v%2.2d.param",g_config,g_config,g_runnumber,vers+1);
    //printf("%s: creating param file version: %s\n",__func__,fnam_param_new);
    //g_rp.write(fnam_param_new);
  }

  g_rp.write_ntp=1; // will write an ntuple
  g_rp.write_bin=0; // and not a bin file!
  //
  printf("%s: creating %s ... ",__func__,g_rootfilename);
  openROOTfile(g_rootfilename, &g_rp);
  
  while(1==readBINfile()){
    unpack_evbuf(g_evbuf);
    fill_all();
    
    g_ievt++;
    if(g_pattern==g_rp.SIGpatt)g_nsig++;
    else if(g_pattern==g_rp.LEDpatt)g_nled++;
    else if(g_pattern==g_rp.PEDpatt)g_nped++;
    
    if( (1==g_ievt) || (0==g_ievt%1000) )
      printf("Total of %10.2fs, %9d events, %9d signal, %6d LED, %6d ped\n", g_t, g_ievt, g_nsig, g_nled, g_nped);
  }
  closeBINfile();
  closeROOTfile();
  
  printf("%s: conversion finished,  %10.2fs, %9d events, %9d signal, %6d LED, %6d ped\n",
         __func__, g_t, g_ievt, g_nsig, g_nled, g_nped);
}
