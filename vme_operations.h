#pragma once

#include <stdint.h>

extern bool g_vme_initialized;

extern uint32_t VME_WAIT_OK; //=0x80;
extern uint32_t VME_WAIT_SIG; //=0x84;
extern uint32_t VME_WAIT_LED; //=0x82;
extern uint32_t VME_WAIT_PED; //=0x81;
extern uint32_t VME_WAIT_TIMEOUT; //=0x6F;
extern uint32_t VME_WAIT_BADVECT; //=0x70;
extern uint32_t VME_WAIT_BADIRQ; //=0x71;
extern uint32_t VME_WAIT_BADACK; //=0x72;

int vme_init(int pulse_len=6); // + configure pulsers for LED (Pulser A) and PED (Pulser B)

int vme_pulse(int pattern); // 0 if PED, 1 if LED, no pulse otherwise
int vme_pulseLED();
int vme_pulsePED();

int vme_wait0(uint32_t timeout);
int vme_wait(uint32_t timeout);
int vme_getirq(int& level, int& vector);
int vme_clearCORBO(); // clear BUSY VME_CRB_CH
int vme_setCORBO();   // set BUSY VME_CRB_CH
int vme_clearCORBO_2(); // clear BUSY VME_CRB_CH2
int vme_setCORBO_2();   // set BUSY VME_CRB_CH2

int vme_read_pattern();
int vme_readADC();
int vme_readTDC();

int vme_start();
int vme_stop();

int vme_close();

//---------------
int V260_init(const uint32_t base);
int V260_clear_scalers(const uint32_t base);
int V260_set_inhibit(const uint32_t base);
int V260_clear_inhibit(const uint32_t base);
int V260_increment(const uint32_t base);
int V260_disable_interrupt(const uint32_t base);
int V260_enable_interrupt(const uint32_t base);
int V260_set_interrupt(const uint32_t base, uint32_t vector);
int V260_read(const uint32_t base, uint32_t chan, uint32_t &count);// read one counter, 24 bit

//------------
int V812_init(const uint32_t base, int* CFDthr);

int V812_write_threshold(const uint32_t base, uint32_t chan, uint32_t val);
int V812_write_width1(const uint32_t base, uint32_t val); // ch 0-7
int V812_write_width2(const uint32_t base, uint32_t val); // ch 8-15
int V812_write_deadtime1(const uint32_t base, uint32_t val); // ch 0-7
int V812_write_deadtime2(const uint32_t base, uint32_t val); // ch 8-15
int V812_write_maj_thr(const uint32_t base, uint32_t val); // ch 0-7
int V812_write_inh_patt(const uint32_t base, uint32_t val); // ch 8-15
int V812_send_test_pulse(const uint32_t base);

//-----------
int write_reg2(const uint32_t base, const uint32_t addr, uint32_t& data);
int  read_reg2(const uint32_t base, const uint32_t addr, uint32_t& data);
int write_reg4(const uint32_t base, const uint32_t addr, uint32_t& data);
int  read_reg4(const uint32_t base, const uint32_t addr, uint32_t& data);

int V1290_read_ctrlreg(const uint32_t base, uint16_t &ctrlreg);
int V1290_write_ctrlreg(const uint32_t base, uint16_t &ctrlreg);
int V1290_read_statreg(const uint32_t base, uint16_t &statreg);
int V1290_write_dummy32reg(const uint32_t base, uint32_t& dummy32);
int V1290_read_dummy32reg(const uint32_t base, uint32_t& dummy32);

int V1290_write_micro(const uint32_t base, uint16_t &datum); 
int V1290_read_micro(const uint32_t base, uint16_t &datum); 
int V1290_opcode(const uint32_t base, int opcode, int nR, uint16_t *R, int nW, uint16_t *W); 

int V1290_clear(const uint32_t base);
int V1290_count_reset(const uint32_t base);

int V1290_read_buffer(const uint32_t base, int maxdata, int &ndata, uint32_t *data);
void V1290_print_buffer(int ndata, uint32_t *data);
void V1290_unpack_buffer(int ndata, uint32_t *data);

int V1290_init(const uint32_t base);
int V1290_test(const uint32_t base);

//------------

int V259_read(const uint32_t base, uint16_t& data);
int V259_clear(const uint32_t base);

//------------
int CORBO_init(int base);
int CORBO_set_IRQ(const uint32_t base, int ch, int type, uint16_t  cr, uint16_t  vr);
int CORBO_get_IRQ(const uint32_t base, int ch, int type, uint16_t &cr, uint16_t &vr);
int CORBO_setbusy(const uint32_t base, int ch);
int CORBO_test(const uint32_t base, int ch);
int CORBO_clear(const uint32_t base, int ch);
int CORBO_read_CSR(const uint32_t base, int ch, uint16_t& data);
int CORBO_write_CSR(const uint32_t base, const int ch, const uint16_t data);

//---------

int ADC_write_CSR(const uint32_t base, uint16_t& data);
int ADC_clear(const uint32_t base);
int ADC_read_CSR(const uint32_t base, uint16_t& data);
int ADC_read_data(const uint32_t base, int& evcnt, uint16_t* data);
int ADC_read_data1(const uint32_t base, int& evcnt, uint16_t* data);

