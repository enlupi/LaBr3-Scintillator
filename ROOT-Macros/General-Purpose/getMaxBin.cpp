#include "TH1F.h"


///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//  Get number of bin containing highest value (lower than maxval) in the    //
//  specified range.                                                         //
//                                                                           //
//  Input variables:                                                         //
//    - "h" (TH1F*) = pointer to histogram to be analyzed                    //
//    - "xmin" (double) = starting value of interval of interest             //
//    - "xmax" (double) = ending value of interval of interest               //
//    - "maxval" (double) = upper limit that the value in the wanted bin     //
//        must not exceed                                                    //
//                                                                           //
//  Output:                                                                  //
//    - int containing the number of the wanted bin                          //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

int getMaxBin(TH1F* h, double xmin, double xmax, double maxval = FLT_MAX) {
  int bin1 = h->FindBin(xmin);
  int bin2 = h->FindBin(xmax);
  
  int maxbin;
  double maximum = -FLT_MAX;
  double value;
  for (int bin = bin1; bin <= bin2; bin++) {
    value = h->GetBinContent(bin);
    if (value > maximum && value < maxval) {
      maximum = value;
      maxbin = bin;
    }
  }
  return maxbin;
}

