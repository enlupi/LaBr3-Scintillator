#include <string>
#include <iostream>
#include "TTree.h"

using namespace std;	

Int_t findMinDt(Int_t i, ULong64_t* t, UShort_t* c, Long64_t& dt_min,
               	bool& chk_coinc, Int_t min, Int_t max);


///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//  Takes a time-sorted TTree, computes for each event the time difference   //
//  with the closest event (either in the past or in the future) in the      //
//  other channel and builds a TTree to store coincidence information.       //
//  The new TTree contains the following branches:                           // 
//    - "channel" (unisgned short) = channel the mesurement was taken in     //
//    - "energy_main" (int) = energy measurement              						   //
//    - "count_main" (unsigned short) = number of times the event was part   //
//        of a coincidence                                                   //
//    - "energy_coinc" (unsigned short) = energy measurement of the event    //
//      in coincidence                                                       //
//    - "count_coinc" (int) = number of times the coincident event was part  //
//        of a coincidence                                                   //
//    - "time_diff" (long) = time difference between coincidence events      //
//                                                                           //
//  Input parameters:                                                        //
//    - "tree" (TTree*) = pointer to the (sorted) TTree on which to compute  //
//        coincidences                                                       //
//    - "time_var" (string) = name of branch corresponding to event time.    //
//        Defaults to "time_stamp"
//    - "energy_var" (string) = name of branch corresponding to event        //
//        energy. Defaults to "energy_ch"                                    //
//    - "channel" (string) = name of branch corresponding to measurement     //
//        channel. Deafualts to "channel"                                    //
//    - "save" (bool) = if true, saves coincidence TTree.                    //
//        Defaults to "true"                                                 //    
//                                                                           //
//  Output:                                                                  //
//    - TTree* pointing to coincidences TTree                                //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

TTree* timeDiff(TTree* tree, string time_var = "time_stamp",
		            string energy_var = "energy_ch", string channel_var = "channel",
		            bool save = true) {

  // get time stamps and channel arrays
  Int_t nentries = (Int_t) tree->GetEntries();
  ULong64_t* t = new ULong64_t[nentries];
  UShort_t* e  = new UShort_t[nentries];
  UShort_t* c  = new UShort_t[nentries];
  ULong64_t ts;
  UShort_t  en;
  UShort_t  ch;
  tree->SetBranchAddress(time_var.c_str(),    &ts);
  tree->SetBranchAddress(energy_var.c_str(),  &en);
  tree->SetBranchAddress(channel_var.c_str(), &ch);
  for (Int_t i = 0; i < nentries; i++) {
    tree->GetEntry(i);
    t[i] = ts;
    e[i] = en;
    c[i] = ch;
  }
  
  // create arrays for index of event in coincidence in the other channel,
  // number of coincidences for the single event and time differences
  Int_t* ev_coinc = new Int_t[nentries];
  Int_t* n_coinc  = new Int_t[nentries];
  ULong64_t* dt_coinc = new ULong64_t[nentries];
  for (Int_t i = 0; i < nentries; i++) {
    ev_coinc[i] = -1;
    n_coinc[i]  = 0;
    dt_coinc[i] = 0;
  }  
  
  // For each event look for closest event in the other channel
  for (Int_t i = 0; i < nentries; i++) {
    Long64_t dt_min = 1e18;
    bool chk_coinc = false;
  
    Int_t index = findMinDt(i, t, c, dt_min, chk_coinc, -1, nentries);
    if (chk_coinc) {
      ev_coinc[i] = index;
      if (TMath::Abs(dt_min) < 20000) {
      	n_coinc[index] += 1;
      }
      dt_coinc[i] = dt_min;
    }
  }
 
  // create new TTree to store coincidence information
  UShort_t channel;
  UShort_t energy_main, energy_coinc;
  Int_t  count_main, count_coinc;
  Long64_t dt;
  
  string treename = tree->GetName();
  string coincname = "coinc_" + treename;
  string coinctitle = "Coincidences from " + treename;
  
  TTree* tree_coinc = new TTree(coincname.c_str(), coinctitle.c_str());
  tree_coinc->Branch("channel",      &channel,      "channel/s");
  tree_coinc->Branch("energy_main",  &energy_main,  "energy_main/s");
  tree_coinc->Branch("count_main",   &count_main,   "count_ch0/I");
  tree_coinc->Branch("energy_coinc", &energy_coinc, "energy_main/s");
  tree_coinc->Branch("count_coinc",  &count_coinc,  "count_coinc/I");
  tree_coinc->Branch("time_diff",    &dt,           "time_diff/L");

 
  for (Int_t i = 0; i < nentries; i++) {
    // check if coincidence is found
    bool chk_coinc = (ev_coinc[i] != -1);
    if (chk_coinc) {
      channel = c[i];
      energy_main = e[i];
      count_main = n_coinc[i];
      energy_coinc =  e[ev_coinc[i]];
      count_coinc = n_coinc[ev_coinc[i]];
      dt = dt_coinc[i];
      
      tree_coinc->Fill();
    }
  }
  
  if (save) {
    tree_coinc->Write(coincname.c_str(), TObject::kOverwrite);
  }

 
  delete[] t;
  delete[] e;
  delete[] c;
  delete[] ev_coinc;
  delete[] n_coinc;
  delete[] dt_coinc; 

  return(tree_coinc);
}




// find index of closest event both in past and future
Int_t findMinDt(Int_t i, ULong64_t* t, UShort_t* c, Long64_t& dt_min,
         	bool& chk_coinc, Int_t min, Int_t max) {
  Long64_t dt;
  Int_t index;
  // check prior events
  for (Int_t j = i-1; j > min; j--) {
    if (c[i] != c[j] ) {
      dt = t[i] - t[j];
      if (dt < dt_min) {
      	dt_min = dt;
      	chk_coinc = true;
      	index = j;
      }
      break;
    }  
  }
    
  // check future events 
  for (Int_t k = i+1; k < max; k++) {
    if (c[i] != c[k]) {
      dt = t[i] - t[k];
      if (TMath::Abs(dt) < dt_min) {
        dt_min = dt;
      	chk_coinc = true;
      	index = k;
      }
      break;
    }
  }
  return(index);
}

