#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include "TFile.h"
#include "TH1D.h"
#include "TF1.h"

#include "../Coincidences/countCoincidences.cpp"

using namespace std;	     


///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//  Computes the number of events in coincidence in the photopeak and plots  //
//  them as a function of the beam energy. Fits the graph with a double      //
//  sigmoid and returns the DeltaE.                                          //
//                                                                           //
//  Input parameters:                                                        //
//    - "file_list" (string) = name of .txt file containing variables to     //
//        construct the energy histogram and to fit it, path of the input    //
//        files, name of the the .root files, the TTree to analyse and       //
//        corresponding beam energy. They must be listted in the following   //
//        order:                                                             //
//          <bin number> <xmin> <xmax>                                       //
//          <time diff> <xmin fit> <xmax fit>                                //
//          <path of input files>                                            //
//          <file #1 name>                                                   //
//          <TTree #1 name>                                                  //
//          <beam energy #1>                                                 //
//          ...                                                              //
//    - "options" (string) = optional arguments to control the program       //
//        execution. It accepts the following substrings (case sensitive):   //
//          "save" constrols whether to save or not the energy histogram     //
//          "draw" ???????? to be implememted                                //
//                                                                           //
//  Output:                                                                  //
//    - (double) DeltaE                                                      //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

double findDeltaE(string file_list, string options = "draw") {

  double N_p = 1, eta = 1, W_theta = 1;
  
  bool save = (options.find("save") != string::npos);
  bool draw = (options.find("draw") != string::npos);
  
  // read input file list
  ifstream fin(file_list);
  if(!fin) {
    cout << "Error while reading input files list" <<endl;
  }
  int bin_number;
  double xmin, xmax, time_diff, xmin_fit, xmax_fit;
  string input_path;
  fin >> bin_number >> xmin >> xmax >> time_diff 
      >> xmin_fit >> xmax_fit >> input_path ;
  
  vector<string> input_files, tree_names;
  vector<double> E_p;
  string input_f, tree_name;
  double e_p;
  while(fin >> input_f) {
    input_files.push_back(input_path + input_f);
    fin >> tree_name;
    tree_names.push_back(tree_name);
    fin >> e_p;
    E_p.push_back(e_p);
  } 
  
  
  // compute number of events in coincidence
  vector<double> Y;
  for(int i = 0; i < input_files.size(); i++) {
    TFile* file = new TFile(input_files[i].c_str(), "UPDATE");
    double N = countCoincidences(file, tree_names[i], bin_number, xmin, xmax,
                                 0, time_diff, xmin_fit, xmax_fit, save);
    double y = N/(N_p*eta*W_theta);
    Y.push_back(y);
    
    cout << y << endl;
    
    delete file;
  }
  
  
  // Fit Results //
  
  int n = 7; //Y.size();
  double* Y_arr = &Y[0];
  double* Ep_arr = &E_p[0];
  TGraph* graph = new TGraph(n, Ep_arr, Y_arr);
  
  // define step function
  string step_descr = "[0]*(1/(1 + exp(-[1]*(x - [2]))) + 1/(1 + exp( [3]*(x - [2] - [4])))) + [5]";
  TF1* step = new TF1("step", step_descr.c_str(), Ep_arr[0], Ep_arr[n-1]);
  step->SetParNames("m", "a", "c", "b", "d", "q");
  
  // initialise parameters
  double q = (Y[0] + Y[n-1])/2;
  double m = Y[n/2] - Y[0];
  double a = 10;
  double b = 10;
  double c = E_p[0];
  double d = E_p[n-1] - E_p[0];
  step->SetParameters(m, a, c, b, d, q);
  
  // fit
  TFitResultPtr r_fit = graph->Fit(step, "SR");
  
  return r_fit->Parameter(4);
}
  
