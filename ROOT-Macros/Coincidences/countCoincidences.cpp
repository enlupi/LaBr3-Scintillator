#include <string>
#include "TFile.h"
#include "TH1D.h"
#include "TF1.h"

#include "../General-Purpose/getFunction.cpp"

using namespace std;	     


///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//  Computes the numer of events in coincidence.                             //
//                                                                           //
//  Input parameters:                                                        //
//    - "file" (TFile*) = pointer to the .root file containing the TTree     //
//        to be analysed                                                     //
//    - "tree_name" (string) = name of TTree of interest                     //
//    - "bin_number" (int) = number of bins of the energy histogram          //
//    - "xmin" and "xmax" (double) = range of the energy histogram           //
//    - "ch" (int) = channel to analyse                                      //
//    - "time_diff" (double) = time difference where to place the cut to     //
//        consider two events in coincidence                                 //
//    - "xmin_fit" and "xmax_ft" (double) = range of gaussian+linear bkg fit //
//    - "save" (bool) = wheter to save the histogram or not                  //
//                                                                           //
//  Output:                                                                  //
//    - (double) number of events in the photopeak                           //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

double countCoincidences(TFile* file, string tree_name, int bin_number,
                         double xmin, double xmax, int ch, double time_diff = 16000,
                         double xmin_fit = 450, double xmax_fit = 540, bool save = true) {
                         
  double bin_width = (xmax - xmin)/bin_number;

  // create coincience histogram
  TTree* tree = (TTree*) file->Get(tree_name.c_str())->Clone();
  string hist_name = "Coinc_evts" + to_string(ch);
  TH1F* h = new TH1F(hist_name.c_str(), "Coincidence Events",
                     bin_number, xmin, xmax);
  string cut = "channel==" + to_string(ch) + " && abs(time_diff)<" + to_string(time_diff);
  tree->Draw(("energy_main>>"+hist_name).c_str(), cut.c_str(), "goff");
                   
  if(save) {
    h->Write(hist_name.c_str(), TObject::kOverwrite);
  }

  // compute number of events in photopeak
  string funcname = "lingaus";
  TF1* f = getFunction(funcname.c_str(), h, xmin_fit, xmax_fit);
  TFitResultPtr r_fit = h->Fit(f, "SR"); 
  TF1* bkg = new TF1("bkg", "pol1(0)", xmin_fit, xmax_fit);
  bkg->SetParNames("q", "m");
  bkg->SetParameters(r_fit->Parameter(3), r_fit->Parameter(4));
  
  double N = (f->Integral(r_fit->Parameter(1) - 3*r_fit->Parameter(2),
                          r_fit->Parameter(1) + 3*r_fit->Parameter(2)) -
              bkg->Integral(r_fit->Parameter(1) - 3*r_fit->Parameter(2),
                            r_fit->Parameter(1) + 3*r_fit->Parameter(2)))/bin_width;
                            
  return N;   
  }
