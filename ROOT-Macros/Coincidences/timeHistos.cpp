#include <iostream>
#include <vector>
#include <string>
#include "TTree.h"
#include "TH1F.h"
#include "TLegend.h"

using namespace std;

double meanInRange(TH1F* h, int min, int length);


///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//  Plots the histogram of coincidence times in the specified channel, then  //
//  fits it with a double exponential + constant background and selects a    //
//  rejection region. An image of the plot is saved as .png.                 //
//                                                                           //
//  Input parameters:                                                        //
//    - "file" (TFile*) = pointer to the .root file containing the TTree     //
//        to be analysed                                                     //
//    - "treename" (string) = name of TTree of interest                      //
//    - "ch" (int) = channel number                                          //
//    - "path" (string) = path where to save the image.                      //
//        Defaults to "ProcessedData/Images/"                                //
//                                                                           //
//  Output:                                                                  //
//    - void                                                                 //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

void timeHistos(TFile* file , string treename, int ch,
                string path = "ProcessedData/Images/") {

  TTree* tree = (TTree*) file->Get(treename.c_str())->Clone(); 
  
  // get run info
  int start_pos = treename.find("tree_") + 5;
  int end_pos   = treename.find(".root");
  int name_length = end_pos - start_pos;
  string runID = treename.substr(start_pos, name_length);
  
  
  // HISTOGRAMS //
  
  // make time difference histograms
  string histname = "hist_timeDiff_" + runID + "_" + to_string(ch);
  string histtitle = runID + ": Time Differences ch " + to_string(ch);
  TH1F* h = new TH1F(histname.c_str(), histtitle.c_str(),
                     100, -1e5, 1e5);
  tree->Draw(("time_diff>>" + histname).c_str(),
             ("channel == " + to_string(ch)).c_str());
  h->GetXaxis()->SetTitle("#Delta t [ps]");
  h->GetYaxis()->SetTitle("Counts / 2 ns");
     
  h->Write(histname.c_str(), TObject::kOverwrite);
  
  
  // FIT //
  
  // define fit function:
  // constant background + exponential on  both sides
  int binmax = h->GetMaximumBin();
  double center = h->GetXaxis()->GetBinCenter(binmax);
  string fleft =  "[0] + [1]*exp((x - (" + to_string(center) + ") + [2])/[3])";
  string fright = "[0] + [1]*exp((-1*(x - (" + to_string(center) + ")) +  [2])/[3])";
  string limleft  = "(x < "  + to_string(center) + ")";
  string limright = "(x >= " + to_string(center) + ")";
  string f =    "(" + fleft  + ")*" + limleft + 
             " + (" + fright + ")*" + limright;
  TF1* func = new TF1("doubleExp", f.c_str());
  func->SetParNames("Constant", "a", "x0", "tau");
    
  // set initial parameters
  double bkg = (meanInRange(h, 1, 25) + meanInRange(h, 76, 25)) / 2;
  double tau = 1e3;
  func->SetParameters(bkg, 1, center, tau);

  // execute fit
  TFitResultPtr r = h->Fit(func, "S");  
  vector<double> par = r->Parameters();
  vector<double> err = r->Errors();
  double chi2 = r->Chi2();
  int dof = r->Ndf();

  // compute acceptance region limits:
  // derivative of exponential is equal to e^-6
  double accLimits[2];
  double l = par[3]*log(exp(-6)*par[3]/par[1]);
  accLimits[0] = center - par[2] + l;
  accLimits[1] = center + par[2] - l;  
  
   
  // PLOT //
  
  TCanvas* c1 = new TCanvas("c1","c1",1200,1000);
  TPad* pad1 = new TPad("pad1","",0,0,1,1);
  pad1->Draw();
  pad1->cd();
  pad1->SetLogy();
  pad1->SetGridy();
  
  h->SetLineColor(kBlack);
  h->Draw();
  
  // histograms to highlight rejected regions
  TH1F* h_left = new TH1F("left_reject", "Left Rejection Region",
                          100, -1e5, 1e5);
  tree->Draw("time_diff>>left_reject",
             ("channel == " + to_string(ch) + " && " +
              "time_diff < " + to_string(-16e3)).c_str(), "goff");
  TH1F* h_right = new TH1F("right_reject", "Right Rejection Region",
                           100, -1e5, 1e5);
  tree->Draw("time_diff>>right_reject",
             ("channel == " + to_string(ch) + " && " +
              "time_diff > " + to_string(16e3)).c_str(), "goff");
  h_left->SetLineColor(kBlack);
  h_left->SetFillColor(kRed);
  h_left->SetFillStyle(3003);
  h_right->SetLineColor(kBlack);
  h_right->SetFillColor(kRed);
  h_right->SetFillStyle(3003);
  h_left->Draw("SAME");
  h_right->Draw("SAME");
  
  func->SetLineWidth(1);
  func->Draw("SAME");
  
  TLegend* leg = new TLegend(0.15,0.75,0.35,0.85);
  leg->AddEntry(h_left, "Rejected Region","f");
  leg->AddEntry(func, "Best Fit","l");
  leg->Draw();
 
  c1->SaveAs((path + histname + ".png").c_str());
  
  
  delete c1;
  delete h;
  delete func;
  delete h_left;
  delete h_right;
    
  return; 
}


// get mean of the histogram in the range [min, min+length]
double meanInRange(TH1F* h, int min, int length) {
  double sum = 0;
  for (int i = min; i < (min + length); i++ ) {
    sum += h->GetBinContent(i);
  }
  sum /= length;
  return sum;
}
