#ifndef __CINT__
#include "iostream"
#include "fstream"
#include "TFile.h"
#include "TTree.h"
#include "TChain.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#endif
using namespace std;

/* modify for # of front end Boards */
const Int_t numBoardsInFile   =  3;
/************************************/
const Int_t numChannelsPerBoard= 30;
const Int_t numChipsOnBoard =    5;
const Int_t psecSampleCells =    256;

void ConvertACDCdata(char *filein, char *fileout, bool applyGainCorrection=true){ 
  if(applyGainCorrection){
 
    TFile *Flut = new TFile("calibrations/LUT.root", "READ");
    TTree *T_lut = (TTree*)Flut->Get("Tlut");
    //T_lut->Print();
    float Voltage[numChannelsPerBoard*numBoardsInFile][psecSampleCells];
    int ADC[4096];
    int numBoardsinLUTFile = 0;
    
    T_lut->SetBranchAddress("numBoardsforLUT", numBoardsinLUTFile);
    T_lut->SetBranchAddress("Voltage", Voltage);
    T_lut->SetBranchAddress("ADC", ADC);
  }
  TFile *f = new TFile(fileout, "RECREATE");

  int nSample = psecSampleCells;
  int sample[psecSampleCells], unwrap_sample[psecSampleCells];
  int nCell = 0, event = 0;
  float Data[numBoardsInFile*numChannelsPerBoard][psecSampleCells];
  float metaData[numBoardsInFile][psecSampleCells];
  char data_leaf[200], meta_data_leaf[200];
  sprintf(data_leaf, "Data[%i][256]/F",numBoardsInFile*numChannelsPerBoard); 
  sprintf(meta_data_leaf, "metaData[%i][256]/F",numBoardsInFile);  
  
  TTree *T = new TTree("T", "T");
  T->Branch("event", &event, "event/I");
  T->Branch("sample", sample, "sample[256]/I");
  T->Branch("unwrap_sample", unwrap_sample, "unwrap_sample[256]/I");
  T->Branch("Data", &Data, data_leaf);
  T->Branch("metaData", &metaData, meta_data_leaf);

  /* open data .txt file */
  ifstream ifs;
  ifs.open(filein, ios::in);
  if(!ifs) cout << "error opening *.txt file" << endl;
  
  int data_tmp;
  /* go through data file */
  while(ifs){
    /* read line-by-line */
    ifs >> sample[nCell] >> unwrap_sample[nCell];
    
    for(int j=0; j<numBoardsInFile; j++){
      for(int i=0; i<numChannelsPerBoard+1; i++){
	if(i==numChannelsPerBoard) ifs >> metaData[j][nCell];
	else{

	  ifs >> Data[i+j*numChannelsPerBoard][nCell];
	  if( applyGainCorrection){
	    data_tmp = Data[i+j*numChannelsPerBoard][nCell];
	    T_lut->GetEntry( data_tmp);
	    Data[i+j*numChannelsPerBoard][nCell]= 
	      Voltage[i+j*numChannelsPerBoard][nCell];
	  }
	}
      }
    }
    //    cout << nCell << ":" << sample[nCell] << ":" << event << endl;
    nCell++;

    if(nCell==nSample){
      T->Fill();
      event ++;
      nCell = 0;
    }
    
  }
  cout << "converted " << event << " events" << endl;
  T->Write();
  f->Close();
  ifs.close();
}				  

