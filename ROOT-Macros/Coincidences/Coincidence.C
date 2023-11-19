#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include "TFile.h"
#include "TH1D.h"

#include "sortTree.cpp"
#include "timeDiff.cpp"
#include "timeHistos.cpp"

using namespace std;	     


///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//  Computes a coincidence information for a TTree.                          //
//                                                                           //
//  Input parameters:                                                        //
//    - "file_list" (string) = name of .txt file containing name of          //
//        variables in the TTree to be used, path of the input files, name   //
//        of the the .root files and the TTree to analyse. They must be      //
//        listed in the following order:                                     //
//          <time var> <energy var> <channel var>                            //
//          <path of input files>                                            //
//          <file #1 name>                                                   //
//          <TTree #1 name>                                                  //
//          ...                                                              //
//    - "options" (string) = optional arguments to control the program       //
//        execution. It accepts the following substrings (case sensitive):   //
//         "already sorted" controls whether it is needed to sort the TTree  //
//         by time or not                                                    //
//         "verbose" controls amount of output written to console            //
//         "descending" imposes descing order for the time sorted TTree      //
//         "save" controls whether to save the new time sorted and           //
//         coincidence TTress                                                //
//         "draw" calls timeHistos function to draw the time coincidence     //
//         histograms                                                        //
//    - "path" (string) = optional path needed for the output images of the  //
//       timeHistos function                                                 //
//                                                                           //
//  Output:                                                                  //
//    - void                                                                 //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

void Coincidence(string file_list, string options = "",
                 string path = "") {

  // options
  bool already_sorted = (options.find("already sorted") != string::npos);
  bool verbose        = (options.find("verbose")        != string::npos);
  bool descending     = (options.find("descending")     != string::npos);
  bool save           = (options.find("save")           != string::npos);
  bool draw           = (options.find("draw")           != string::npos);
             
  cout << "SAVE: " << save << endl;
              
  // read input file list
  ifstream fin(file_list);
  if(!fin) {
    cout << "Error while reading input files list" <<endl;
  }
  
  string time_var, energy_var, channel_var, input_path;
  vector<string> input_files, tree_names;
  
  fin >> time_var >> energy_var >> channel_var >> input_path;
  string input_f, tree_name;
  while(fin >> input_f) {
    input_files.push_back(input_path + input_f);
    fin >> tree_name;
    tree_names.push_back(tree_name);
  } 
  
  
  // compute coincidences for all input files
  for(int i = 0; i < input_files.size(); i++) {  
  
    TFile* file = new TFile(input_files[i].c_str(), "UPDATE");
 
    // get time-sorted TTree
    TTree* tsorted;
    if(already_sorted) {
      tsorted = (TTree*) file->Get(tree_names[i].c_str())->Clone(); 
    } else {
      tsorted = sortTree(file, tree_names[i], time_var, descending, save);
    }  
  
    if(verbose) cout << "Obtained time-sorted Tree" << endl;

    // get coincidences information
    TTree* tcoinc = timeDiff(tsorted, time_var, energy_var, channel_var, save);
  
    if(verbose) cout << "Computed coincidences info" << endl;
    
    // draw resulting time coincidences histograms
    if(draw) {
      for(int ch = 0; ch < 2; ch++) {
        timeHistos(file, "coinc_"+tree_names[i], ch, path);
      }
    }
  
    // free memory
    delete tsorted;
    delete tcoinc;
    delete file;
  }
  
  return;
}
  
