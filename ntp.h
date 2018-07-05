#pragma once

#include <cctype>

#include "runparam.h"

extern int g_nclear;

extern double g_t, g_t0;
extern int g_pattern; // 0=PED, 1=LED, 2=SIG
extern int g_ADC[NADCCHAN];

extern int g_tTDCtrig;

extern int g_nTDC[NTDCCHAN];
extern int g_tTDC[NTDCCHAN][NTDCMAXHITS];

extern int g_nDT5742[NDT5742CHAN];
extern float* g_evdata742[NDT5742CHAN]; // intermediate destination for data pointers
extern int g_used742[NDT5742CHAN];          // flag =1 for used channels, 0 otherwise
extern float g_aDT5742[NDT5742CHAN][NDT5742SAMPL];

void reset_histos();
void delete_histos();

void openROOTfile(const char* filenam, const RUNPARAM* rp);
void closeROOTfile();
void fill_all();

extern RUNPARAM g_rp;
