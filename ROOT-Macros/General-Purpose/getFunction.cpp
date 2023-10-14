#include <string>
#include "TH1F.h"

#include "getMaxBin.cpp"

using namespace std;


///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//  Creates a function and sets its initial parameters for a fit.            //
//                                                                           //
//  Input variables:                                                         //
//    - "funcname" (string) = name of the function. Options are:             //
//        "lingaus", gaussian + linear background                            //
//        "compgaus", gaussian + compton shoulder                            //
//        "multi+lingaus", gaussian_1 + gaussian_2 + linear background       //
//    - "h" (TH1F*) = histogram to be fit                                    //
//    - "xmin" (double) = starting value of fit range                        //
//    - "xmax" (double) = ending value of fit range                          //
//                                                                           //
//  Output:                                                                  //
//    - TF1* pointing to wanted function                                     //
//                                                                           //
/////////////////////////////////////////////////////////////////////////////// 

TF1* getFunction(string funcname = "lingaus", TH1F* h = nullptr,
                 double xmin = 50, double xmax = 5500) {

  // define fitting functions: 
  TF1* func = nullptr;
  
  // gaussian + line
  if (!funcname.compare("lingaus")) {
    func = new TF1("lingaus", "gaus(0)+pol1(3)", xmin, xmax);
    func->SetParNames("Constant", "Mean", "Sigma", "q", "m");
    
    // set initial parameters
    double y1 = h->GetBinContent(h->FindBin(xmin));
    double y2 = h->GetBinContent(h->FindBin(xmax));
    double m = (y2 - y1)/(xmax - xmin);
    double q = y2 - m*xmax;
    
    // get bin with maximum content in range
    int maxbin = getMaxBin(h, xmin, xmax);
    double mean = h->GetXaxis()->GetBinCenter(maxbin);
    double c    = h->GetBinContent(maxbin) - (mean*m + q);
    
    func->SetParameters(c, mean, 20, q, m);
  }
  
  // gaussian + compton shoulder
  else if (!funcname.compare("compgaus")) {
    func = new TF1("compgaus", "gaus(0)+[3]*0.5*(erfc((x-[1])/(sqrt(2)*[2])))",
                   xmin, xmax);
    func->SetParNames("Constant", "Mean", "Sigma", "al");
    func->SetParameters(6000, (xmax + xmin)/2, 20, 200);
  }
  
  // 2 gaussian + line
  else if(!funcname.compare("multi_lingaus")) {
    func = new TF1("multi_lingaus", "gaus(0)+gaus(3)+pol1(6)",
                   xmin, xmax);
    func->SetParNames("Constant1", "Mean1", "Sigma1",
                      "Constant2", "Mean2", "Sigma2",
                      "q", "m");
    		      
    // set initial parameters
    double y1 = h->GetBinContent(h->FindBin(xmin));
    double y2 = h->GetBinContent(h->FindBin(xmax));
    double m = (y2 - y1)/(xmax - xmin);
    double q = y2 - m*xmax;
    
    // get bin with maximum content in range
    int maxbin1 = getMaxBin(h, xmin, xmax);
    int maxbin2 = getMaxBin(h, xmin, xmax, h->GetBinContent(maxbin1)); 
    double mean1 = h->GetXaxis()->GetBinCenter(maxbin1);
    double mean2 = h->GetXaxis()->GetBinCenter(maxbin2);
    double c1    = h->GetBinContent(maxbin1) - (mean1*m + q);
    double c2    = h->GetBinContent(maxbin2) - (mean2*m + q);
    
    func->SetParameters(c1, mean1, 20,
                        c2, mean2, 20,
                        q, m);
  }   
  
  return func; 
 }
