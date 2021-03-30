#pragma once

#include <cctype>

#include "runparam.h"

extern int g_dimsrv_initialized;

extern int g_ievt;
extern int g_nped;
extern int g_nled;
extern int g_nsig;

extern bool g_startrun;
extern bool g_stoprun;
extern bool g_running;

extern bool g_exit;

extern int g_runnumber;

extern char g_config[128];
extern char g_rootfilename[256];

extern int g_print;
extern bool g_write;

extern int g_nTDCclear;

extern double g_t, g_t0, g_tprev;
extern int g_pattern; // 0=PED, 1=LED, 2=SIG
extern int g_ADC[NADCCHAN];

extern int g_tTDCtrig;

extern int g_nTDC[NTDCCHAN];
extern int g_tTDC[NTDCCHAN][NTDCMAXHITS];

extern int g_n742[2*N742CHAN];
extern float* g_evdata742[2*N742CHAN]; // intermediate destination for data pointers
extern float g_a742[2*N742CHAN][N742SAMPL];
extern int g_startCell[2*N742CHAN];
extern int g_trigTag[2*N742CHAN];

void reset_histos();
void delete_histos();

void openROOTfile(const char* filenam, const RUNPARAM* rp);
void closeROOTfile();
void fill_all();

///////////////   DIM stuff  /////

void delete_dimHists();
void delete_dimservices();

void update_dimHists();
void update_services();

void create_dimservices();
void create_dimHists();

bool server_started();
int dimsrv_init();
void dimsrv_update();
void dimsrv_createhistsvc();
void dimsrv_deletehistsvc();
void dimsrv_exit();

const char* dimsrv_getcommand();

void create_dimservstatus();
void delete_dimservstatus();
void update_dimservstatus();

int i2JCH(int i);

int JCH2i(int JCH);

