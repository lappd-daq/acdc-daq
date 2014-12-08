///////////////////////////////////////////////////////////////////////////////////////
// Convert AC/DC board data into .root format. Compresses data by ~3X
// specify number of boards in file/DAQ system in line # 42 
// 
//-----
// to run:
// open root, i.e. type 'root' at command line:
//
// .L ROOT-conversion-and-calibration/ConvertACDCdata.C+
//  ...  takes 2 (required) filein, filout, and option to correct 
//  ...  to correct for cell-by-cell voltage gain calibration
//  ConvertACDCdata("test.txt", "test.root")
//
//  to quickly plot
//  re-open root (.q previous session)
//  
//  TFile *f = new TFile("test.root")
//  ...  to look at waveform in channel 4 at 5th event, for example:
//  T->Draw("Data[4]:sample", "event==5")
//
///////////////////////////////////////////////////////////////////////////////////////

//#ifndef __CINT__
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
//#endif
using namespace std;

/* modify for # of front end Boards */
const Int_t numBoardsInFile   =  3;
/************************************/
const Int_t numChannelsPerBoard= 30;
const Int_t numChipsOnBoard =    5;
const Int_t psecSampleCells =    256;

void ConvertACDCdata(char *filein, char *fileout, bool applyGainCorrection=true){ 
     
  float*** VoltageLUT;
  VoltageLUT = new float**[numChannelsPerBoard*numBoardsInFile];
  for(int j=0; j<numBoardsInFile; j++){
    for(int chan=0; chan<numChannelsPerBoard; chan++){
      VoltageLUT[chan+j*numChannelsPerBoard] = new float*[psecSampleCells];
      for(int samp=0; samp<psecSampleCells; samp++) VoltageLUT[chan+j*numChannelsPerBoard][samp] = new float[4096];
    }
  }
  if( applyGainCorrection){
    TFile *Flut = new TFile("calibrations/LUT.root");
    TTree *T_lut = (TTree*)Flut->Get("Tlut");
    //T_lut->Print();
    int entry;
    
    float Voltage[numChannelsPerBoard*numBoardsInFile][psecSampleCells]; 
    //int numBoardsinLUTFile = 0;
    //T_lut->SetBranchAddress("numBoardsforLUT", numBoardsinLUTFile);
    TBranch* Entrylut  = T_lut->GetBranch("ADC"); 
    TBranch* Vlut  =     T_lut->GetBranch("Voltage"); 
    //TBranch* ADlut  = T_lut->GetBranch("ADC"); 
    
    Entrylut->SetAddress( &entry);
    Vlut->SetAddress( &Voltage);
    //ADlut->SetAddress( &ADC);
    
    for(int i=0; i< T_lut->GetEntries(); i++){
      T_lut->GetEntry(i);
      for(int j=0; j<numBoardsInFile; j++){
	for(int chan=0; chan<numChannelsPerBoard; chan++){
	  for(int samp=0; samp<psecSampleCells; samp++){
	    VoltageLUT[chan+j*numChannelsPerBoard][samp][i]=
	      Voltage[chan+j*numChannelsPerBoard][samp];
	  }
	}
      }
    }

    T_lut->Delete();
    Flut->Delete();
  }
  
  TFile *f = new TFile(fileout, "RECREATE");

  int nSample = psecSampleCells;
  int sample[psecSampleCells], unwrap_sample[psecSampleCells];
  int nCell = 0, event = 0;
  float Data[numBoardsInFile*numChannelsPerBoard][psecSampleCells];
  int Pedestals[numBoardsInFile*numChannelsPerBoard][psecSampleCells];
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
  
  int data_tmp=0, dummy=0;
  float wfm_temp=0., ped_temp=0.;
  /* go through data file */
  while(ifs){
    /* read pedestal values (first 'dummy event') */
    if(event==0){
      ifs >> sample[nCell] >> dummy;
      for(int j=0; j<numBoardsInFile; j++){
	for(int i=0; i<numChannelsPerBoard+1; i++){
	  if(i==numChannelsPerBoard) ifs >> dummy;
	  else ifs >> Pedestals[i+j*numChannelsPerBoard][nCell];
	} 
      }			       
    }
    else{
    /* read real data line-by-line */
      ifs >> sample[nCell] >> unwrap_sample[nCell];
    
      for(int j=0; j<numBoardsInFile; j++){
	for(int i=0; i<numChannelsPerBoard+1; i++){
	  if(i==numChannelsPerBoard) ifs >> metaData[j][nCell];
	  else{
	    
	    ifs >> data_tmp;

	    /* data saved in Volts... */
	    if( applyGainCorrection){
	      /* look up waveform voltage */
	      //cout << VoltageLUT[i+j*numChannelsPerBoard][nCell][2];
	      wfm_temp = VoltageLUT[i+j*numChannelsPerBoard][nCell][data_tmp];
	      /* look up pedestal voltage */
	      ped_temp = VoltageLUT[i+j*numChannelsPerBoard][nCell][Pedestals[i+j*numChannelsPerBoard][nCell]];
	      /* subtract the two */
	      Data[i+j*numChannelsPerBoard][nCell]= wfm_temp-ped_temp;
	    }
	    /* OR, data saved in ADC counts */
	    else Data[i+j*numChannelsPerBoard][nCell]=data_tmp-Pedestals[i+j*numChannelsPerBoard][nCell];
	    
	  }
	}
      }
    }
    //cout << nCell << ":" << sample[nCell] << ":" << event << endl;
    nCell++;
  
    if(nCell==nSample){
      T->Fill();
      event++;
      nCell = 0;
    }
  }

  cout << "converted " << event-1 << " events" << endl;
  T->Write();
  f->Close();
  ifs.close();
  return;
}				  

