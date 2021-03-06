//
// Author: Ilka Antcheva   1/12/2006

// This macro gives an example of how to create a status bar 
// related to an embedded canvas that shows the info of the selected object 
// exactly as the status bar of any canvas window
// To run it do either:
// .x statusBar.C
// .x statusBar.C++

#include <math.h>
#include <iostream>
#include <map>

#include "TROOT.h"
#include "TSystem.h"
#include "TApplication.h"
#include "TGClient.h"
#include "TClass.h"
#include "TString.h"

#include "getVal.h"

ClassImp(getVal)

#define TOTWID 500
#define TOTHGT 500

getVal::getVal(const TGWindow *main, char* question, char* val, char* comment)
{
  Question=question;
  Val = val;
  strcpy(bufVal,val);
  //  Val[0]='\0';
  Comment=comment;
  
  fMain = new TGTransientFrame(gClient->GetRoot(), main, TOTWID, TOTHGT);
  fMain->Connect("CloseWindow()", "getVal", this, "CloseWindow()");
  fMain->DontCallClose(); // to avoid double deletions.
  
  // use hierarchical cleaning
  fMain->SetCleanup(kDeepCleanup);
  
  // Create a horizontal frame for username
  TGHorizontalFrame *hframVal = new TGHorizontalFrame(fMain, TOTWID, 40);
  
  TGLabel *tVal;
  if(question)tVal=new TGLabel(hframVal,question);
  else tVal=new TGLabel(hframVal,"INPUT:");
  
  hframVal->AddFrame(tVal, new TGLayoutHints(kLHintsLeft, 5, 1, 3, 4));
  fVal=new TGTextEntry(hframVal,bufVal);
  hframVal->AddFrame(fVal, new TGLayoutHints(kLHintsLeft, 5, 1, 3, 4));
  fMain->AddFrame(hframVal,new TGLayoutHints(kLHintsLeft, 5, 1, 3, 4));
  fVal->Resize(150,fVal->GetDefaultHeight());

  TGHorizontalFrame *hframCom = new TGHorizontalFrame(fMain, TOTWID, 40);
  TGLabel *tCom=new TGLabel(hframCom,"Comments:");
  hframCom->AddFrame(tCom, new TGLayoutHints(kLHintsLeft, 5, 1, 3, 4));
  fMain->AddFrame(hframCom,new TGLayoutHints(kLHintsLeft, 5, 1, 3, 4));

  fEdit = new TGTextEdit(fMain, TOTWID, 300, kSunkenFrame | kDoubleBorder);
  fMain->AddFrame(fEdit, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY, 3, 3, 3, 3));
  fEdit->Connect("Closed()", "getVal", this, "DoCANCEL()");

  // Create a horizontal frame for OK and CANCEL buttons
  TGHorizontalFrame *hframButt = new TGHorizontalFrame(fMain, TOTWID, 300);
  fOK = new TGTextButton(hframButt, "  &OK  ");
  //fOK->SetToolTipText("YES, PLEASE save to DB",200);
  fOK->Connect("Released()", "getVal", this, "DoOK()");
  hframButt->AddFrame(fOK, new TGLayoutHints(kLHintsLeft, 5, 1, 3, 4));
  fCANCEL = new TGTextButton(hframButt, "  &CANCEL  ");
  //fCANCEL->SetToolTipText("NO, DO NOT save to DB",200);
  fCANCEL->Connect("Released()", "getVal", this, "DoCANCEL()");
  hframButt->AddFrame(fCANCEL, new TGLayoutHints(kLHintsLeft, 5, 1, 3, 4));
  fMain->AddFrame(hframButt,new TGLayoutHints(kLHintsLeft, 5, 1, 3, 4));

  SetTitle();

  fMain->MapSubwindows();

  fMain->Resize();

  // editor covers right half of parent window
  fMain->CenterOnParent(kTRUE, TGTransientFrame::kRight);
}

getVal::~getVal()
{
  // Delete editor dialog.

  fMain->DeleteWindow();  // deletes fMain
}

void getVal::Popup()
{
  fMain->MapWindow();
  gClient->WaitFor(fMain);
}

void getVal::SetTitle()
{
  fMain->SetWindowName("getVal");
  fMain->SetIconName("getVal");
}

void getVal::CloseWindow()
{
  // Called when closed via window manager action.

  delete this;
}

void getVal::DoOK()
{
  // Handle ok button.
  strcpy(Val,fVal->GetText());
  strcpy(Question,"OK");
  
  TString tstrcomm=(fEdit->GetText()->AsString());
  const char* txtcomm=(const char*)tstrcomm;
  strncpy(Comment, txtcomm, 10000); Comment[10000]=0;
  
  CloseWindow();
}

void getVal::DoCANCEL()
{
  // Handle ok button.

  //Unam[0]='\0';
  strcpy(Question,"CANCEL");
  
  CloseWindow();
}

