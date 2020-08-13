#pragma once

extern int DHandle;
extern bool g_digitizer_initialized;

int digitizer_init(char* config_name);

int digitizer_close();

int digitizer_read();

int digitizer_reset();

int digitizer_clear();

int digitizer_start();

int digitizer_stop();

int digitizer_adjust_pedestals(double precision);
