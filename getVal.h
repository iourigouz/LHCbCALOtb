#pragma once

#include "TGButton.h"
#include "TGFrame.h"
#include "TGTextEntry.h"
#include "TGLabel.h"

class getVal {

private:
  TGTransientFrame *fMain;   // main frame of this widget
  TGTextEntry      *fVal;   // val entry 
  TGTextButton     *fOK;     // OK button
  TGTextButton     *fCANCEL; // CANCEL button

  char *Val;   // address of the Val
  char *Question; //address of the question, to put OK or CANCEL

  char bufVal[512];

public:
  getVal(const TGWindow *main, char* question, char* val);
   virtual ~getVal();

   void   SetTitle();
   void   Popup();

   // slots
   void   CloseWindow();
   void   DoOK();
   void   DoCANCEL();

   ClassDef(getVal, 0)
};

