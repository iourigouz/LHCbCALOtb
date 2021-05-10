int SY5527_login(const char* ipaddress);
int SY5527_logout();
int SY5527_setChName(int chan, const char* ChName); // NB chan=100*islot+iout
int SY5527_setHV(int chan, double HV_kV); // NB HV in kilovolts
int SY5527_HVOn(int chan);
int SY5527_HVOff(int chan);
int SY5527_getChStatus(int chan, unsigned long &Status);// 0-Off, 1-On, 5-RampDown, 3-RampUp
int SY5527_getChVMon(int chan, float &VMon);
//=======================================
int SY5527_connect();
int SY5527_disconnect();
bool SY5527_checkAllHV();
void SY5527_setAllHVOn();
void SY5527_waitAllHVOn();
void SY5527_setAllHVOff();
