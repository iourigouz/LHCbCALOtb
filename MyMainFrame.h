#pragma once

#define NADCCHAN 24
#define NTDCCHAN 32
#define NTDCMAXHITS 10
#define NDT5742CHAN 32
#define NDT5742SAMPL 2048

#define MAXCHANS 100
#define MAXNAMELENGTH 32

#define MAXLEDS 2

#include "TGButton.h"
#include "Buttons.h"
#include "TGNumberEntry.h"
#include "TGComboBox.h"
#include "TGLabel.h"
#include "TTimer.h"
#include "TGFrame.h"
#include "TFrame.h"
#include "TRootEmbeddedCanvas.h"
#include "TGStatusBar.h"
#include "TCanvas.h"
#include "TH1D.h"
#include "TProfile.h"

class MyMainFrame : public TGMainFrame {

 private:
  TRootEmbeddedCanvas  *fEcan;
  TGLabel *lbRunnumber;
  TGTextButton *tbEnable, *tbStartRun, *tbAddComment;
  TGLabel *lbNrun, *lbTbeg, *lbTdur, *lbRunStatus, *lbNLED, *lbNPED, *lbNSIG, *lbNTOT, *lbConf;
  TGStatusBar          *fStatusBar;
  TGComboBox *cbPatt, *cbType, *cbHist;
  TGTextButton *tb2Ref;
  TGCheckButton *tbAutoDraw, *tbSubRef;
  TGLabel *lbFile;
  
  TTimer            *fTimer;           // update timer
  TH1D* h0;

 public:
  MyMainFrame(const TGWindow *p, UInt_t w, UInt_t h);
  virtual ~MyMainFrame();
  void Init();
  void EventInfo(Int_t event, Int_t px, Int_t py, TObject *selected);
  Bool_t HandleTimer(TTimer* timer);
  
  void DoEnableStartRun();
  void DoStartRun();
  void DoAddComment();
  
  void fillHistCombo(const char* cpatt, const char* ctype);
  void DoSetPattType(Int_t wId, Int_t id);
  void DoSetHist(Int_t wId, Int_t id);
  
  void DoDrawHist();
  void drawHist();
  
  void Do2Ref();
  
  void fillRunStatus();
  
  void              CloseWindow();
  
  void DoAutoDraw(bool on);
  void DoSubRef(bool on);
  
  ClassDef(MyMainFrame, 0)
};

int parse_service_name(const char* servicename, char* type, char* patt, char* cell, int& chan, char* suppl);
void* address_dimhist(const char* type, const char* patt, const char* cell, int chan, const char* suppl);
void* address_ref_dimhist_ref(const char* type, const char* patt, const char* cell, int chan, const char* suppl);
void* address_dimsummary(const char* type, const char* patt, const char* cell, int chan, const char* suppl);
void* address_ref_dimsummary(const char* type, const char* patt, const char* cell, int chan, const char* suppl);
void reset_dimhists();
int list_dimhists();
