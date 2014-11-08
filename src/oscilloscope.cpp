#include "SuMo.h"
#include "ScopePipe.h"

int SuMo::scope_AC( int trig_mode, bool output_mode, int AC_adr){
  bool convert_to_voltage = false;
  int check_event, count = 0, psec_cnt = 0;
  bool scope_active = true;
  float pdat[AC_CHANNELS][256];
  int asic_baseline[5][256];
  int targetAC = 0;

  float sample;

  ScopePipe mypipe;

  //int* bl = new int[5][256];
  //if(load_lut() != 0) convert_to_voltage = false;
  //else
  //  convert_to_voltage = output_mode;
  
  if(DC_ACTIVE[AC_adr] == false){
    printf("no AC detected at specified address. cannot perform oscilloscope function!\n");
    return 1;
  }
  
  //load_ped();
  //mypipe.plot_init();
  
  //while(run){
  // count++;
  psec_cnt = 0;
  manage_cc_fifo(1);
  if(trig_mode) set_usb_read_mode(7);
  usleep(100000);
    //if(count == 100) break;
  //if(!trig_mode) software_trigger(1 << AC_adr);
  if(!trig_mode){ 
    set_usb_read_mode(AC_adr);
    software_trigger((unsigned int)15);
  }
  usleep(100);
  
  read_AC(true, 1, AC_adr); 
  //read_ACS(true);
  check_event = 0;
  get_AC_info(false);
  //if(trig_mode) manage_cc_fifo(1);

    for(int i = 0; i < AC_CHANNELS; i++){
      if(i>0 && i % 6 == 0) psec_cnt ++;

      for(int j = 0; j < 256; j++){
	if(convert_to_voltage){
	  //sample =  LUT[(int)AC_RAW_DATA[psec_cnt][i%6*256+j]][i]*1000;
	  //sample -= LUT[(int)PED_DATA[i][j]][i]*1000;
	}
	else{
	  sample = (float) AC_RAW_DATA[psec_cnt][i%6*256+j];
	  sample -= (float) PED_DATA[AC_adr][i][j];
	}
	pdat[i][j] = sample;
      }
    }
    
    int baseline[256];

    for(int i = 0; i < 5; i ++){
    unwrap_baseline(baseline, 2);   // firmware wraparound marked on ASIC #2
      for (int j = 0; j < 256; j++){
	asic_baseline[i][j] = baseline[j];
	//if(j == 0)
	  //printf("%i, ", asic_baseline[i][j]);
      
      }
    }
    

    FILE* foutdata = fopen("scope.dat", "w");
    if(foutdata == NULL) return 1;
    
    for(int i=0; i < 256; i++)
      fprintf(foutdata, 
	      "%4.1d\t%4.1f\t%4.1f\t%4.1f\t%4.1f\t%4.1f\t%4.1f\t%4.1f\t%4.1f\t%4.1f\t%4.1f\t%4.1f\t%4.1f\t%4.1f\t%4.1f\t%4.1f\t%4.1f\t%4.1f\t%4.1f\t%4.1f\t%4.1f\t%4.1f\t%4.1f\t%4.1f\t%4.1f\t%4.1f\t%4.1f\t%4.1f\t%4.1f\t%4.1f\t%4.1f\t\n",    
	      i,pdat[0][asic_baseline[0][i]],pdat[1][asic_baseline[0][i]],pdat[2][asic_baseline[0][i]],
	      pdat[3][asic_baseline[0][i]],pdat[4][asic_baseline[0][i]],pdat[5][asic_baseline[0][i]],
	      pdat[6][asic_baseline[1][i]],pdat[7][asic_baseline[1][i]],pdat[8][asic_baseline[1][i]],
	      pdat[9][asic_baseline[1][i]],pdat[10][asic_baseline[1][i]],pdat[11][asic_baseline[1][i]],
	      pdat[12][asic_baseline[2][i]],pdat[13][asic_baseline[2][i]],pdat[14][asic_baseline[2][i]],
	      pdat[15][asic_baseline[2][i]],pdat[16][asic_baseline[2][i]],pdat[17][asic_baseline[2][i]],
	      pdat[18][asic_baseline[3][i]],pdat[19][asic_baseline[3][i]],pdat[20][asic_baseline[3][i]],
	      pdat[21][asic_baseline[3][i]],pdat[22][asic_baseline[3][i]],pdat[23][asic_baseline[3][i]],
	      pdat[24][asic_baseline[4][i]],pdat[25][asic_baseline[4][i]],pdat[26][asic_baseline[4][i]],
	      pdat[27][asic_baseline[4][i]],pdat[28][asic_baseline[4][i]],pdat[29][asic_baseline[4][i]]);
    
    fclose(foutdata);
    usleep(30000);
    
    //set_usb_read_mode(111);
    //mypipe.plot();
    
    
    for(int targetAC = 0; targetAC < 4; targetAC++){
      if(DC_ACTIVE[targetAC] == true){
	//printf("plugged boards: %d\n", targetAC);
	if(targetAC != AC_adr){
	  read_AC(true, 1, targetAC);
	  //printf("dumped data: %d\n", targetAC);
	}
      }
      
    }
    
    usleep(30000); 
    //manage_cc_fifo(1);
    //if(trig_mode) set_usb_read_mode(7);
    manage_cc_fifo(1);
    //}
  //mypipe.finish();
  return 0;
  
}
