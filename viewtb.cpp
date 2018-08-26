// hcalcurr_view.cpp : Defines the entry point for the console application.
//

#include "TROOT.h"
//#include "TRint.h"
#include "TApplication.h"
#include "TDatime.h"
#include "TH2F.h"
#include "MyMainFrame.h"

int main(int argc, char *argv[]){
  int argC=1;
  TApplication theApp("App", &argC, argv);
  //TRint theApp("App", &argC, argv);

  MyMainFrame* fram=new MyMainFrame(gClient->GetRoot(), 200, 200);

  fram->Init();

  gROOT->Add(fram);
  /*  
  theApp.SetPrompt("BeamTest [%d] ");
  theApp.ProcessLine("TObject* obj; MyMainFrame* mmf; TIter nex(gROOT->GetList());"
                 "while ((obj=nex())){ if(obj->IsA()==MyMainFrame::Class())mmf=(MyMainFrame*)obj;}");
  */
  theApp.Run();

  return 0;
}
