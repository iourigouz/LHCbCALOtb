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

#include "getComment.h"

ClassImp(getComment)

#define TOTWID 500
#define TOTHGT 500

getComment::getComment(const TGWindow *main, char* answer, char* comment)
{
  Answer=answer;
  Comment=comment;
  
  fMain = new TGTransientFrame(gClient->GetRoot(), main, TOTWID, TOTHGT);
  fMain->Connect("CloseWindow()", "getComment", this, "CloseWindow()");
  fMain->DontCallClose(); // to avoid double deletions.
  
  // use hierarchical cleaning
  fMain->SetCleanup(kDeepCleanup);
  
  TGHorizontalFrame *hframCom = new TGHorizontalFrame(fMain, TOTWID, 40);
  TGLabel *tCom=new TGLabel(hframCom,"Enter a comment:");
  hframCom->AddFrame(tCom, new TGLayoutHints(kLHintsLeft, 5, 1, 3, 4));
  fMain->AddFrame(hframCom,new TGLayoutHints(kLHintsLeft, 5, 1, 3, 4));

  fEdit = new TGTextEdit(fMain, TOTWID, 300, kSunkenFrame | kDoubleBorder);
  fMain->AddFrame(fEdit, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY, 3, 3, 3, 3));
  fEdit->Connect("Closed()", "getComment", this, "DoCANCEL()");

  // Create a horizontal frame for OK and CANCEL buttons
  TGHorizontalFrame *hframButt = new TGHorizontalFrame(fMain, TOTWID, 50);
  fOK = new TGTextButton(hframButt, "  &OK  ");
  //fOK->SetToolTipText("YES, PLEASE add the comment to the logbook",200);
  fOK->Connect("Released()", "getComment", this, "DoOK()");
  hframButt->AddFrame(fOK, new TGLayoutHints(kLHintsLeft, 5, 1, 3, 4));
  fCANCEL = new TGTextButton(hframButt, "  &CANCEL  ");
  //fCANCEL->SetToolTipText("NO, DO NOT add the comment to the logbook",200);
  fCANCEL->Connect("Released()", "getComment", this, "DoCANCEL()");
  hframButt->AddFrame(fCANCEL, new TGLayoutHints(kLHintsLeft, 5, 1, 3, 4));
  fMain->AddFrame(hframButt,new TGLayoutHints(kLHintsLeft, 5, 1, 3, 4));

  SetTitle();

  fMain->MapSubwindows();

  fMain->Resize();

  // editor covers right half of parent window
  fMain->CenterOnParent(kTRUE, TGTransientFrame::kRight);
}

getComment::~getComment()
{
  // Delete editor dialog.

  fMain->DeleteWindow();  // deletes fMain
}

void getComment::Popup()
{
  fMain->MapWindow();
  gClient->WaitFor(fMain);
}

void getComment::SetTitle()
{
  fMain->SetWindowName("getComment");
  fMain->SetIconName("getComment");
}

void getComment::CloseWindow()
{
  // Called when closed via window manager action.

  delete this;
}

void getComment::DoOK()
{
  // Handle ok button.
  strcpy(Answer,"OK");
  
  TString tstrcomm=(fEdit->GetText()->AsString());
  const char* txtcomm=(const char*)tstrcomm;
  strncpy(Comment, txtcomm, 10000); Comment[10000]=0;
  
  CloseWindow();
}

void getComment::DoCANCEL()
{
  // Handle ok button.

  strcpy(Answer,"CANCEL");
  
  CloseWindow();
}

