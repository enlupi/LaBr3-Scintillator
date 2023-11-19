#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"
#include <string>
#include <vector>

using namespace std;

void plotHisto(string filename, string treename, string option = "",
	       int bin_number = 2725, double xmin = 50, double xmax = 5500) {

  vector<string> hist_names{"energy_ch0", "energy_ch1"};
  if (!option.compare("calibration")) {
    hist_names.push_back("energy_calib_ch0");
    hist_names.push_back("energy_calib_ch1");
    }
    
  // set calibration constants
  double m0 = 0.51785;
  double q0 = 6.68;
  double m1 = 0.53328;
  double q1 = 1.81;
  
  // get TTree object 
  TFile* file = new TFile(filename.c_str(), "UPDATE");
  TTree* tree = (TTree*) file->Get(treename.c_str())->Clone();
    
  // create histograms for the two channels
  int start_pos = filename.find("run") + 3;
  int end_pos =   filename.find(".root");
  int name_length = end_pos - start_pos;
  string hist_title = filename.substr(start_pos, name_length) +
  		      ": ADC Energy ch ";
  TH1F* h_ch0 = new TH1F(hist_names[0].c_str(), (hist_title + "0").c_str(),
  			 bin_number, xmin, xmax);
  TH1F* h_ch1 = new TH1F(hist_names[1].c_str(), (hist_title + "1").c_str(), 
  			 bin_number, xmin, xmax);
  tree->Draw("energy_ch>>energy_ch0", "channel==0", "goff");
  tree->Draw("energy_ch>>energy_ch1", "channel==1", "goff");
  
  // overwrite previous iterations
  h_ch0->Write(hist_names[0].c_str(), TObject::kOverwrite);
  h_ch1->Write(hist_names[1].c_str(), TObject::kOverwrite);
  
  // create calibrated histograms if required
  if (!option.compare("calibration")) {
    double xmin_cal = 0.5*xmin;
    double xmax_cal = 0.5*xmax; 
    string hist_title_cal = filename.substr(start_pos, name_length) +
    			    ": Energy ch ";
    TH1F* h_cal_ch0 = new TH1F(hist_names[2].c_str(), (hist_title_cal + "0").c_str(),
  	       	  	       bin_number, xmin_cal, xmax_cal);
    TH1F* h_cal_ch1 = new TH1F(hist_names[3].c_str(), (hist_title_cal + "1").c_str(), 
  			       bin_number, xmin_cal, xmax_cal);
    
    string cal0_name = to_string(m0) + "*energy_ch+" +
    		       to_string(q0) + ">>" + hist_names[2].c_str();
    string cal1_name = to_string(m1) + "*energy_ch+" +
    		       to_string(q1) + ">>" + hist_names[3].c_str();
    tree->Draw(cal0_name.c_str(), "channel==0", "goff");
    tree->Draw(cal1_name.c_str(), "channel==1", "goff");
  
    h_cal_ch0->Write(hist_names[2].c_str(), TObject::kOverwrite);
    h_cal_ch1->Write(hist_names[3].c_str(), TObject::kOverwrite);
    }
 
  file->Close();
  delete file;
  
  return; 
}  
