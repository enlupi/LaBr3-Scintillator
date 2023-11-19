#include <string>
#include <vector>
#include <fstream>

#include "binConversion.C"

using namespace std;	     


///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//  Converts all files included in a list from BIN to .root files.           //
//                                                                           //
//  Input parameters:                                                        //
//    - "file_list" (string) = name of .txt file containing path of the      //
//        files, name of the BIN files and names of the .root files. They    //
//        must be listed in the following order:                             //
//          <path of input files>                                            //
//          <path of output files>                                           //
//          <BIN file #1 name>                                               //
//          <.root file #1 name>                                             //
//          ...                                                              //
//    - "options" (string) = optional arguments to pass to "binConversion"   //
//        function. Please refer to its file for the specific options        //
//                                                                           //
//  Output:                                                                  //
//    - void                                                                 //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

void convertFiles(string file_list, string options = "to calibrate") {
  
  // read input file list
  ifstream fin(file_list);
  if(!fin) {
    cout << "Error while reading input files list" <<endl;
  }
  
  string input_path, output_path;
  vector<string> input_files, output_files;
  
  fin >> input_path;
  fin >> output_path;
  
  string input_f, output_f;
  while(fin >> input_f) {
    input_files.push_back(input_path + input_f);
    fin >> output_f;   
    output_files.push_back(output_path + output_f);
  } 
  
  // convert bin file to .root
  for(int i = 0; i < input_files.size(); i++) {
    binConversion(input_files[i], output_files[i], options);
  }

  return;
  }    
  
