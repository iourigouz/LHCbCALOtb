#pragma once

extern int DHandle;                  // for digitizer 1
extern bool g_digitizer_initialized; // for digitizer 1

extern int DHandle2;                  // for digitizer 2
extern bool g_digitizer2_initialized; // for digitizer 2

int digitizer_init(char* config_name);
int digitizer_close();
int digitizer_SWtrg();
int digitizer_read();
int digitizer_reset();
int digitizer_clear();
int digitizer_start();
int digitizer_stop();
int digitizer_adjust_pedestals(double precision);

int digitizer2_init(char* config_name);
int digitizer2_close();
int digitizer2_SWtrg();
int digitizer2_read();
int digitizer2_reset();
int digitizer2_clear();
int digitizer2_start();
int digitizer2_stop();
int digitizer2_adjust_pedestals(double precision);
