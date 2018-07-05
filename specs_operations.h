#pragma once
#include <stdint.h>

int specs_open(int iportDAC, int iportINT, int islaveDAC, int islaveINT);
int specs_init_INT(int iportINT, int islaveINT);
int specs_init_DAC(int iportDAC, int islaveDAC);

int specs_setINTRange(int iconn, int integ, int rng);
int specs_setINTDAC(int idac);
int specs_readINT(int iconn, int rng, int nchan, int* currs);

int specs_setDACchan(int ichan, int iDAC);
int specs_writeDAC(int* DAC);

int write_HV();
int write_ULED();

int specs_close();
