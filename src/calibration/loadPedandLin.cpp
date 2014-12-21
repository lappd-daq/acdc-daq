#include "SuMo.h"
#include <iostream>
#include <fstream>

using namespace std;

int SuMo::load_ped(){
  
  for(int targetAC = 0; targetAC < numFrontBoards; targetAC++){  
    char ped_filename[500];
    sprintf(ped_filename, "calibrations/PED_DATA_%d.txt", targetAC);
    
    FILE* fped_in = fopen(ped_filename, "r");

    if (fped_in == NULL){
      //printf("No pedestal file found for board #%d\n", targetAC);
      continue;
    }
    int rows=0;
  
    while (!feof(fped_in)) {
      int i;
      char buf[100000];
      
      fgets(buf, 100000, fped_in);
      
      if (feof(fped_in))
	continue;
      
      if (count(buf, buf + strlen(buf), '\t') < AC_CHANNELS)
	continue;  // not 5-column or more
      
      sscanf(buf, "%d\t", &i);
      
      int tmp;
      sscanf(buf, 
	     "%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t\n",
	      &tmp,&PED_DATA[targetAC][0][i],&PED_DATA[targetAC][1][i],&PED_DATA[targetAC][2][i],
	      &PED_DATA[targetAC][3][i],&PED_DATA[targetAC][4][i],&PED_DATA[targetAC][5][i],
	      &PED_DATA[targetAC][6][i],&PED_DATA[targetAC][7][i],&PED_DATA[targetAC][8][i],
	      &PED_DATA[targetAC][9][i],&PED_DATA[targetAC][10][i],&PED_DATA[targetAC][11][i],
	      &PED_DATA[targetAC][12][i],&PED_DATA[targetAC][13][i],&PED_DATA[targetAC][14][i],
	      &PED_DATA[targetAC][15][i],&PED_DATA[targetAC][16][i],&PED_DATA[targetAC][17][i],
	      &PED_DATA[targetAC][18][i],&PED_DATA[targetAC][19][i],&PED_DATA[targetAC][20][i],
	      &PED_DATA[targetAC][21][i],&PED_DATA[targetAC][22][i],&PED_DATA[targetAC][23][i],
	      &PED_DATA[targetAC][24][i],&PED_DATA[targetAC][25][i],&PED_DATA[targetAC][26][i],
	      &PED_DATA[targetAC][27][i],&PED_DATA[targetAC][28][i],&PED_DATA[targetAC][29][i]);
        
        rows++;
    }
    fclose(fped_in);
  }
    return 0;
}


/*
int SuMo::load_lut(){
    FILE* fin = fopen("calibrations/LUT.txt", "r");
    if (fin == NULL)
        return 1;
    
    int rows=0;
    
    while (!feof(fin)) {
        int i;
        char buf[1000];
        
        fgets(buf, 1000, fin);
        
        if (feof(fin))
            continue;
            
        if (count(buf, buf + strlen(buf), '\t') < AC_CHANNELS)
            continue;  // not 5-column or more
        
        sscanf(buf, "%d\t", &i);
        
        int tmp;
        sscanf(buf,  
"%d\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t\n", 
	    &tmp,&LUT[i][0],&LUT[i][1],&LUT[i][2],
	    &LUT[i][3],&LUT[i][4],&LUT[i][5],
	    &LUT[i][6],&LUT[i][7],&LUT[i][8],
	    &LUT[i][9],&LUT[i][10],&LUT[i][11],
	    &LUT[i][12],&LUT[i][13],&LUT[i][14],
	    &LUT[i][15],&LUT[i][16],&LUT[i][17],
	    &LUT[i][18],&LUT[i][19],&LUT[i][20],
	    &LUT[i][21],&LUT[i][22],&LUT[i][23],
	    &LUT[i][24],&LUT[i][25],&LUT[i][26],
	    &LUT[i][27],&LUT[i][28],&LUT[i][29]);
        
        rows++;
    }
    fclose(fin);
    return 0;
}
*/

