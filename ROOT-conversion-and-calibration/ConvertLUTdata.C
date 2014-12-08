///////////////////////////////////////////////////////////////////////////////////////
// Convert AC/DC board LUT data into .root format. Compresses data by ~3X
// specify number of boards in file/DAQ system in line # 42
// 
//-----
// to run:
// open root, i.e. type 'root' at command line:
//
// .L ROOT-conversion-and-calibration/Convert-LUT-data.C+
//  ConvertLUTdata()
//
//  to quickly plot
//  re-open root (.q previous session)
//  
//  TFile *f = new TFile("calibrations/LUT.root")
//  ...  to look at channel 5, cell 55 transfer curve:
//  Tlut->Draw("Voltage[5][55]:ADC")
//
///////////////////////////////////////////////////////////////////////////////////////


#include "iostream"
#include "fstream"
#include "TFile.h"
#include "TTree.h"
#include "TChain.h"
#include "TBranch.h"
#include "TF1.h"
#include "TH1F.h"
#include "TProfile.h"
#include "TMath.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

using namespace std;
/**************************************************/
/* number of boards should match number of input cal files */
/* files generated in DAQ software: */
/* src::makePedandLin.cpp::./makeLUT, exe::./bin/makeLUT */
const Int_t numBoardsforLUT =    3;
char lut_file_0[200] = "calibrations/LUT_CELL_0.txt"; 
char lut_file_1[200] = "calibrations/LUT_CELL_2.txt"; 
char lut_file_2[200] = "calibrations/LUT_CELL_3.txt"; 
/**************************************************/

const Int_t numChannelsPerBoard= 30;
const Int_t numChipsOnBoard =    5;
const Int_t psecSampleCells =    256;

char fileout[200]   = "calibrations/LUT.root"; 

void ConvertLUTdata(){ 

  TFile *f = new TFile(fileout, "RECREATE");
  int   ADC;
  int   board;
  //float Voltage[numChannelsPerBoard*numBoardsforLUT][psecSampleCells][4096];
  
  /* put on heap memory so compiling works */
  float*** Voltage;
  Voltage = new float**[numChannelsPerBoard*numBoardsforLUT];
  for(int j=0; j<numBoardsforLUT; j++){
    for(int chan=0; chan<numChannelsPerBoard; chan++){
      Voltage[chan+j*numChannelsPerBoard] = new float*[psecSampleCells];
      for(int samp=0; samp<psecSampleCells; samp++) Voltage[chan+j*numChannelsPerBoard][samp] = new float[4096];
    }
  }
  
  float V[numChannelsPerBoard*numBoardsforLUT][psecSampleCells];

  char Voltage_leaf[200];
  char* filein;
  sprintf(Voltage_leaf, "Voltage[%i][256]/F",numChannelsPerBoard*numBoardsforLUT);
 
  TTree *Tlut = new TTree("Tlut", "Tlut");
  
  TBranch *_ADC = Tlut->Branch("ADC", &ADC, "ADC/I");
  //TBranch *_numBoardsForLUT =Tlut->Branch("numBoardsforLUT", &numBoardsforLUT, "numBoardsforLUT/I");
  TBranch *_Voltage =Tlut->Branch("Voltage", &V, Voltage_leaf);			  
  
  /* open cal data .txt file */
  for(int fileNum = 0; fileNum < numBoardsforLUT; fileNum++){
    ifstream ifs;
    if(fileNum == 0) filein = lut_file_0;
    if(fileNum == 1) filein = lut_file_1;
    if(fileNum == 2) filein = lut_file_2;
    /*...etc..*/

    ifs.open(filein, ios::in);
    if(!ifs) cout << "error opening file" << endl;
    
    /* go through data file */
    board = 0;
    int row = 0, tmp;
    float temp = 0.;
    while(ifs){
      int channel, sample;
      ifs >> tmp;
      //cout << row << ":" << ADC[row] << ":" << numChannelsPerBoard*fileNum << endl;
      for(int i=0; i<numChannelsPerBoard; i++){
	for(int j=0; j<psecSampleCells; j++){
	  ifs >> temp;
	  
	  channel = i;
	  sample =  j;
	  Voltage[channel+numChannelsPerBoard*fileNum][sample][row] = temp;      

	}
      }  
      row++;
      if(row > 4095) break;
      
    }

    ifs.close();
    cout << "finished processing file " << fileNum+1 << " of " << numBoardsforLUT << endl;
  }
  cout << "Now, organize and save to ROOT Tree.." << endl;

  /* this is a messy way to go about this.. */
  for(int i=0; i<4096; i++){
    ADC = i;
    for(int fileNum = 0; fileNum < numBoardsforLUT; fileNum++)
      for(int channel=0; channel<numChannelsPerBoard; channel++)
	for(int sample=0; sample<psecSampleCells; sample++) 
	  V[channel+numChannelsPerBoard*fileNum][sample] = Voltage[channel+numChannelsPerBoard*fileNum][sample][i];

    Tlut->Fill();
  }

  Tlut->Write();
  f->Close();
  cout << "Done" << endl;
  return;
}				  

