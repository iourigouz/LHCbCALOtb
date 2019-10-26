#pragma once

#include "TGButton.h"
#include "TGFrame.h"
#include "TGTextEntry.h"
#include "TGTextEdit.h"
#include "TGLabel.h"

class getComment {

private:
  TGTransientFrame *fMain;   // main frame of this widget
  TGTextEntry      *fVal;   // val entry 
  TGTextButton     *fOK;     // OK button
  TGTextEdit       *fEdit;   // text edit widget
  TGTextButton     *fCANCEL; // CANCEL button

  char *Answer; //address of the answer, to put OK or CANCEL
  char* Comment; // address of the comment, the string separator is assumed \n

public:
  getComment(const TGWindow *main, char* answer, char* comment);
   virtual ~getComment();

   void   SetTitle();
   void   Popup();

   // slots
   void   CloseWindow();
   void   DoOK();
   void   DoCANCEL();

   ClassDef(getComment, 0)
};

