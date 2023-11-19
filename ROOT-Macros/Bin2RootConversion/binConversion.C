#include <fstream>
#include <string>

#include "TFile.h"
#include "TTree.h"

using namespace std;


///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//  Takes a binary file as input and converts it to a .root file containing  //
//  a TTree of the variables of interests.                                   //
//                                                                           //
//  Input parameters:                                                        //
//    - "inputfile" (string) = input binary file name                        //
//    - "outputfile" (string) = output .root file name                       //
//    - "readoptions" (string) = optional arguments:                         //
//        "calibrated" if the energy is already converted to KeV / MeV       //
//        "both" if the energy is in ADC channels and KeV / MeV              //
//          (default is ADC channels only)                                   //
//        "to calibrate" if the energy is in ADC channels and you also want  //
//          it to be immediately  converted to MeV using previous (linear)   //
//          calibration                                                      //
//        "DPP/PSD" if there is at least one board running DPP‐PSD firmware  //
//        "waves" if wave samples taking is enabled (NOT YET IMPLEMENTED)    //
//                                                                           //
//  Output:                                                                  //       
//    - void                                                                 // 
//                                                                           //
/////////////////////////////////////////////////////////////////////////////// 

void binConversion(string inputfile, string outputfile, string readoptions = "") {

  // calibration constants
  Double_t  q0 = 6.68;
  Double_t  m0 = 0.51785; 
  Double_t  q1 = 1.80;
  Double_t  m1 = 0.53329; 
  
  
  
  UShort_t  header;
  UShort_t  board;
  UShort_t  channel;
  ULong64_t time_stamp;   // ps
  UShort_t  energy_ch;    // ch
  ULong64_t energy;       // keV / MeV
  Double_t  energy_calib; // keV
  UShort_t  en_short;   // ch
  UInt_t    flags;
  // UInt_t    N_waves;
  // TArrayS   samples(N_waves);
  
  // read options
  bool calibrated   = (readoptions.find("calibrated")   != string::npos);
  bool both         = (readoptions.find("both")         != string::npos);
  bool to_calibrate = (readoptions.find("to calibrate") != string::npos);
  bool dpp_psd      = (readoptions.find("DPP-PSD")      != string::npos);
  // bool waves        = (readoptions.find("waves")        != string::npos);

  // read from file
  ifstream file(inputfile, ios::binary);
  if (!file) {
    cout << "Error : input file not found!\n";
  return;
  } 
  
  // create output ROOT file
  TFile* hfile = new TFile(outputfile.c_str(), "UPDATE");
  
  // create TTree
  int start_pos = inputfile.find("DataR_run") + 9;
  int end_pos =   inputfile.find(".BIN");
  int name_length = end_pos - start_pos;
  string runID = inputfile.substr(start_pos, name_length);
  string treename = "tree_" + runID;
  string treetitle = "TTree from run " + runID;
  TTree* tree = new TTree(treename.c_str(), treetitle.c_str());

  tree->Branch("channel",    &channel,    "channel/s"); 
  tree->Branch("time_stamp", &time_stamp, "time_stamp/l");
  tree->Branch("board",      &board,      "board/s");
  
  if (calibrated) {
    tree->Branch("energy",   &energy,     "energy/l");
   }  
  else if (both) {
    tree->Branch("energy_ch",    &energy_ch, "energy_ch/s"); 
    tree->Branch("energy_calib", &energy,    "energy/l"); 
  }
  else if (to_calibrate) {
    tree->Branch("energy_ch",    &energy_ch,    "energy_ch/s"); 
    tree->Branch("energy_calib", &energy_calib, "energy/D"); 
  }
  else {
    tree->Branch("energy_ch",    &energy_ch, "energy_ch/s");
  }
  
  if (dpp_psd) {
    tree->Branch("energy_short", &en_short, "en_short/s");
  }
    
  tree->Branch("flags",          &flags,      "flags/i");

  /*   
  if (waves) {
    tree->Branch("wave_samples", &samples[0], "samples[N_waves]/S");
  } 
  */

  // read file
  file.read(reinterpret_cast<char*>(&header), sizeof(header));
  cout << "Header: ";
  cout << hex << header;
  cout << "\n";
  
  while (file.read(reinterpret_cast<char*>(&board),  sizeof(board))) {
    file.read(reinterpret_cast<char*>(&channel),     sizeof(channel));
    file.read(reinterpret_cast<char*>(&time_stamp),  sizeof(time_stamp));
     
    // energy reading
    if (calibrated) {
      file.read(reinterpret_cast<char*>(&energy),    sizeof(energy));
    }  
    else if (both) {
      file.read(reinterpret_cast<char*>(&energy_ch), sizeof(energy_ch));
      file.read(reinterpret_cast<char*>(&energy),    sizeof(energy));
    }
    else if (to_calibrate) {
      file.read(reinterpret_cast<char*>(&energy_ch), sizeof(energy_ch));
      switch (channel) {
        case 0:
          energy_calib = m0*energy_ch + q0;
          break;
        case 1:
          energy_calib = m1*energy_ch + q1;
          break;
      }
    }
    else {
      file.read(reinterpret_cast<char*>(&energy_ch), sizeof(energy_ch));
    }
 
 
    if (dpp_psd) {
      file.read(reinterpret_cast<char*>(&en_short),  sizeof(en_short));
    } 
    
    file.read(reinterpret_cast<char*>(&flags),       sizeof(flags));

    /*    
    if (waves){
      file.read(reinterpret_cast<char*>(&N_waves),   sizeof(N_waves));
      for (int i = 0; i < N_waves; i++) {
        file.read(reinterpret_cast<char*>(&samples[i]),   sizeof(samples[i]));
      }
    }
    */
    
    // update TTree
    tree->Fill();
  }

  hfile->Write();
  hfile->Close();

  return;
}

