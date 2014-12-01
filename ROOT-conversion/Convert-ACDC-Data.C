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
const Int_t numBoardsInFile   =  2;
/* data .txt file data delimiter */
#define delimiter                " " 
/************************************/
const Int_t numChannelsPerBoard= 30;
const Int_t numChipsOnBoard =    5;
const Int_t psecSampleCells =    256;

void ConvertACDCdata(char *filein, char *fileout){ 

  TFile *f = new TFile(fileout, "RECREATE");

  int nSample = psecSampleCells;
  int sample[psecSampleCells], unwrap_sample[psecSampleCells], nBoard[numBoardsInFile];
  int nCell = 0, event = 0;
  string linestream, token;
  size_t pos = 0;
  
  TTree *T = new TTree("T", "T");
  T->Branch("event", &event, "event/I");
  T->Branch("sample", sample, "sample[256]/I");
  T->Branch("unwrap_sample", unwrap_sample, "unwrap_sample[256]/I");

  float Data[numBoardsInFile*numChannelsPerBoard][psecSampleCells];
  float metaData[numBoardsInFile][psecSampleCells];
  char data_leaf[200], meta_data_leaf[200];
  sprintf(data_leaf, "Data[%i][256]/F",numBoardsInFile*numChannelsPerBoard); 
  sprintf(meta_data_leaf, "metaData[%i][256]/F",numBoardsInFile); 
  T->Branch("Data", &Data, data_leaf);
  T->Branch("metaData", &metaData, meta_data_leaf);
				  
 
  /* open data .txt file */
  int data_offset = 2;
  ifstream ifs;
  ifs.open(filein, ios::in);
  if(!ifs) cout << "error opening *.txt file" << endl;
  
  /* go through data file */
  while(!ifs.eof()){
    /* read line-by-line */
    getline(ifs, linestream);
    //cout << linestream << endl;
   
    /* parse line */
    int tokenNum = 0;
    while (pos=linestream.find(delimiter)){
      token = linestream.substr(0, pos);
      if(token.length() == 0) break;
      linestream.erase(0,pos+1);
      //cout << line << ":" << nCell << ":" << token<<  endl;

      if(tokenNum == 0) sample[nCell]         = atoi(token.c_str());
      if(tokenNum == 1) unwrap_sample[nCell]  = atoi(token.c_str());
      
      /* 0th board's data */
      if(tokenNum >= 2 && tokenNum < 2+numChannelsPerBoard){
	Data[tokenNum-data_offset][nCell]   = atof(token.c_str()); 

      }
      else if(tokenNum == 2+numChannelsPerBoard){
	metaData[0][nCell]                  = atof(token.c_str());
      }
      
      /* 1st board's data */
      if(tokenNum >= 33 && tokenNum < 33+numChannelsPerBoard){
	Data[tokenNum-data_offset-1][nCell] = atof(token.c_str()); 
      }	
      else if(tokenNum == 33+numChannelsPerBoard){
	metaData[1][nCell]                  = atof(token.c_str());
      }
      
      /* 2nd board's data */
      if(tokenNum >= 64 && tokenNum < 64+numChannelsPerBoard){
	Data[tokenNum-data_offset-2][nCell] = atof(token.c_str()); 
      }	
      else if(tokenNum == 64+numChannelsPerBoard){
	metaData[2][nCell]                  = atof(token.c_str());
      }    
      /* 3rd board's data */
      if(tokenNum >= 95 && tokenNum < 95+numChannelsPerBoard){
	Data[tokenNum-data_offset-2][nCell] = atof(token.c_str()); 
      }	
      else if(tokenNum == 95+numChannelsPerBoard){
	metaData[4][nCell]                  = atof(token.c_str());
      }     
      /* ...etc. */
      tokenNum++;     
    }
    //cout << nCell << ":" << sample[nCell] << ":" << event <<  ":" << tokenNum << endl;
    nCell++;
    //cout << endl;
    if(nCell==nSample){
      T->Fill();
      event ++;
      //if(evtNo > 1) break;
      nCell = 0;
    }
    
  }
  cout << "converted " << event << " events" << endl;
  T->Write();
  f->Close();
  ifs.close();
}				  

