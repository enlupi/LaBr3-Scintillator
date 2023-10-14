#include <string>
#include "TH1D.h"


///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//  Finds the extremes of the shortest interval so that the integral of the  //
//  histogram in that range exceeds the desired value. The interval is       //
//  symmetric either around the mean or zero.                                //
//                                                                           //
//  Input variables:                                                         //
//    - "h" (T1HD*) = pointer to histogram to be analyzed                    //
//    - "perc" (double) = value of the integral in the wanted interval,      //
//       expressed as fraction w.r.t. total integral                         //
//    - "start" (string) = point around which the interval will be           //
//        symmetric. Can either be "mean" (histogram mean) or "zero"         //
//                                                                           //
//  Output:                                                                  //
//    - double[2] containing the two extremities of the interval             // 
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

double[2] findInterval(TH1D* h, double perc, std::string start) {

  int n = h->GetNbinsX();
  // select starting bin
  int bin_start;
  if (tolower(start) == "mean") {
    bin_start = h->FindBin(h->GetMean());
  }
  else if (tolower(start) == "zero") {
    bin_start = h->FindBin(0);
  }
 
  // loop until integral exceeds desired value
  double cut = new double[2];
  double N = h->Integral(1, n);
  double sum = h->GetBinContent(bin_start);
  int i = 1;
  while ((bin_start-i) > 0 && (bin_start+i) < n+1) {
    sum += h->GetBinContent(bin_start-i) +
           h->GetBinContent(bin_start+i);
           
    if (sum/N > perc) {
      cut[0] = h->GetBinCenter(bin_start-i);
      cut[1] = h->GetBinCenter(bin_start+i);
      break;
    }
  }
  
  return(cut);
}

