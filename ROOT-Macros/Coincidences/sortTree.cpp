#include <string>
#include "TTree.h"


///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//  Sorts TTree based on desired variable, e.g. ordering events by time.     //
//                                                                           //
//  Input parameters:                                                        //
//    - "file" (TFile*) = pointer to the .root file containing the TTree     //
//        to be sorted                                                       //
//    - "treename" (string) = name of TTree to be sorted                     //
//    - "time_var" (string) = name of variable to base order on.             //
//        Defaults to "time_stamp", assuming time-based ordering             //
//    - "desc_order" (bool) = if true, sortes in descending order and        //
//        viceversa. Defaults to "false" (ascending order)                   //
//    - "save" (bool) = if true, svaes sorted TTree in input TFile           //
//        Defaults to "false"                                                //
//                                                                           //
//  Output:                                                                  //
//    - TTree* pointing to newly sorted TTree                                //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

TTree* sortTree(TFile* file, std::string treename,
		            std::string time_var = "time_stamp", bool desc_order = false,
		            bool save = false) {

  // get TTree
  TTree* tree = (TTree*) file->Get(treename.c_str())->Clone();

  Int_t nentries = (Int_t)tree->GetEntries();
  // get time stamp arrays
  ULong64_t* t = new ULong64_t[nentries];
  ULong64_t ts;
  tree->SetBranchAddress(time_var.c_str(),    &ts);
  for (Int_t i = 0; i < nentries; i++) {
    tree->GetEntry(i);
    t[i] = ts;
  }
  
  Int_t* index = new Int_t[nentries];
  // sort variable array in specified order
  TMath::Sort(nentries, t, index, desc_order);    
  
  // Create a clone of the original tree and fill
  // with time-ordered events
  TTree *tsorted = (TTree*)tree->CloneTree(0);
  for (Int_t i = 0; i < nentries; i++) {
    tree->GetEntry(index[i]);
    tsorted->Fill();
  }
  
  // save tree to file if needed
  if (save) {
    string sortname = "sorted_" + treename;
    tsorted->Write(sortname.c_str(), TObject::kOverwrite);
  }
  
  
  delete[] t;
  delete[] index;
   
  return tsorted;
}

