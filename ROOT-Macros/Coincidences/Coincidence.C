#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include "TFile.h"
#include "TH1D.h"

#include "sortTree.cpp"
#include "timeDiff.cpp"

using namespace std;	     


///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//  Computes a coincidence information for a TTree.                          //
//                                                                           //
//  Input parameters:                                                        //
//    - "filename" (string) = name of .root file containing TTree            //
//    - "treename" (string) = name of TTree of interest                      //
//    - "alreadySorted" (bool) = parameter to control whether it is needed   //
//        to sort the TTree by time or not. Defaults to "false"              //
//    - "verbose" (bool) = controls amount of output written to console      //
//                                                                           //
//  Output:                                                                  //
//    - void                                                                 //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

void Coincidence(string filename, string treename,
		             bool alreadySorted = false, bool verbose = false) {
  
  string var = "time_stamp";
  
  TFile* file = new TFile(filename.c_str(), "UPDATE");
 
  // get time-sorted TTree
  TTree* tsorted;
  if (alreadySorted) {
    tsorted = (TTree*) file->Get(treename.c_str())->Clone(); 
  } else {
    tsorted = sortTree(file, treename, var, false, true);
  }  
  
  if (verbose) cout << "Obtained time-sorted Tree" << endl;

  // get coincidences information
  TTree* tcoinc = timeDiff(tsorted);
  
  if (verbose) cout << "Computed coincidences info" << endl;
  
 
  delete tsorted;
  delete tcoinc;
  delete file; 
  
  return;
}
  
