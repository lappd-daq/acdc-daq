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
  int board, count ,ADC[4096], channel, sample;
  float Voltage[numChannelsPerBoard*numBoardsforLUT][psecSampleCells][4096];
  float V[numChannelsPerBoard*numBoardsforLUT][psecSampleCells];

  char Voltage_leaf[200];
  char* filein;
  sprintf(Voltage_leaf, "Voltage[%i][256]/F",numChannelsPerBoard*numBoardsforLUT);
 
  TTree *Tlut = new TTree("Tlut", "Tlut");
  //Tlut->Branch("board", &board, "board/I");
  Tlut->Branch("count", &count, "count/I");
  Tlut->Branch("ADC", &ADC, "ADC[4096]/I");
  Tlut->Branch("numBoardsforLUT", &numBoardsforLUT, "numBoardsforLUT/I");
  //Tlut->Branch("channel", &channel, "channel/I");
  //Tlut->Branch("sample", &sample, "sample/I");
  Tlut->Branch("Voltage", &V, Voltage_leaf);			  
  
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
    count = i;
    ADC[i] = i;
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

