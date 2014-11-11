#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include "SuMo.h"
#include "ScopePipe.h"

using namespace std;

int main(int argc, char* argv[]){

  string cmdline;
  char filename[100];
  int num, num1, num2, num3, num4, num5, thresh;
  char* fname;
  fstream cmd_fin;
  SuMo sumo;
  ScopePipe plot_pipe;
  time_t now;  
      
  time(&now);
  char timestring[100];
  strftime(timestring, 80, " %m-%d-%Y  %H:%M", 
	       localtime(&now));  
  /* Check if commands should come from file */
  if (argc == 2)
    cmd_fin.open(argv[1], ifstream::in);
  cout << "=================================================\n";
  cout << "||         SuMo DAQ control shell              ||\n";
  cout << "=================================================\n";
  cout << timestring << "\n";
  cout << " for list of commands, type 'help' at terminal \n";
  cout << " VERSION 0.1 EJO -- July, 2012 \n";
  cout << " ***\n";
  /* Command line loop */
  while(1){
    /* Switch between prompt and file interface */
    if (argc != 2 || cmd_fin.eof()){
      /* write prompt */
      fprintf(stdout, "$>> ");
      /* Read user input */
      getline(cin, cmdline);
    } else {
      getline(cmd_fin, cmdline);
//      if (cmd_fin.eof()) break;
    }

    ////
    if (cmdline.find("exit") == 0 || cmdline.find("quit") == 0
	|| cmdline.find("q") == 0){
      sumo.set_usb_read_mode(16);
      sumo.set_self_trigger(0, 0, 0, 0);
      sumo.dump_data();
      cout << "- Exiting program.... \n";
      break;
    }
    ////
    else if(cmdline.find("help") == 0){
      cout << "You asked for help:\n";
      cout << "* setup lvds          <-- configures lvds interface\n";
      cout << "* status              <-- prints system status\n";
      cout << "* status AC           <-- w/ analog card info\n"; 
      cout << "* take ped            <-- take AC pedestal data run\n";
      cout << "* set ped N           <-- set pedestal level to N\n";
      cout << "* make lut            <-- generates count-voltage LUT\n";
      cout << "* set threshold N     <-- sets trigger thresh to N\n";
      cout << "* read data N         <-- read raw data (N words)\n";      
      cout << "* exit, quit, q       <-- exits program\n";
    }
    ////    
    else if (cmdline.find("lvds map") == 0){
      cout << "------                            ------\n";
      cout << "* DC *                            * CC *\n";
      cout << "------                            ------\n"; 
      cout << "data SERDES 8bit (1)---->     <----system 40MHz\n";
      cout << "data SERDES 8bit (2)---->     <----system trigger\n";
      cout << "DC lvds indicator------->     <----align LVDS sig.\n";
      cout << "n/a                           <----CC instruction SERDES 32 bit\n";
    }
    ////    
    else if (cmdline.find("setup lvds") == 0){
      sumo.sync_usb(0);
      sumo.align_lvds();
      cout << "aligning LVDS SERDES interface...\n";
      cout << "run 'status' to check\n";
    }
    ////      
    else if (cmdline.find("set ped") == 0){
      sumo.sync_usb(0);
      sscanf(cmdline.c_str(), "set ped %d \n", &num);
      if(num > 4095 || num < 0)
	cout << "invalid pedestal level\n";
      else{
	sumo.set_pedestal_value(num);
	sumo.set_pedestal_value(num);
	cout << "setting level...\n";
	cout << "Pedestal Voltage set to " << num*1200/4096 << " mV\n";
      }
    }
    ////      
    else if (cmdline.find("set dll") == 0){
      sumo.sync_usb(0);
      sscanf(cmdline.c_str(), "set dll %d \n", &num);
      if(num > 4095 || num < 0)
	cout << "invalid level\n";
      else{
	sumo.set_dll_vdd(num);
	sumo.set_dll_vdd(num);
	cout << "setting level...\n";
	cout << "DLL p-voltage set to " << num*1200/4096 << " mV\n";
      }
    }
    ////
    else if (cmdline.find("set trig thresh") == 0){
      sumo.sync_usb(0);
      sscanf(cmdline.c_str(), "set trig thresh %d \n", &thresh);
      if(thresh > 4095 || thresh < 0)
	cout << "invalid threshold level\n";
      else{
	sumo.set_trig_threshold(thresh);
	sumo.set_trig_threshold(thresh);
	cout << "setting level...\n";
	usleep(2000);
	cout << "Threshold Voltage set to " << thresh*1250/4096 << " mV\n";
      }
    }
     ////
    else if (cmdline.find("take ped") == 0){
      sumo.set_usb_read_mode(16);
      sumo.dump_data();
      sumo.generate_ped(true);
    }
    ////
    else if (cmdline.find("make lut") == 0){
      sumo.set_usb_read_mode(16);
      sumo.dump_data();
      sumo.dump_data();
      //sscanf(cmdline.c_str(), "make lut \n");
      //if(num > 3 || num < 0)
      //	cout << "nope\n";
      //else
	sumo.make_count_to_voltage();
    }
    ////
    else if (cmdline.find("status") == 0){
      sumo.set_usb_read_mode(16);
      //      sumo.dump_data();
      //sumo.dump_data();

      if(cmdline.find("status AC") == 0 || cmdline.find("status ac") == 0){
	sumo.set_usb_read_mode(16);
	sumo.dump_data();
	sumo.read_CC(true,true);
      }
      else
	sumo.read_CC(true,false);
    }
    /////
    else if (cmdline.find("set sync 0") == 0){
      sumo.sync_usb(0);
    }
    else if (cmdline.find("set sync 1") == 0){
      sumo.sync_usb(1);
    }
    ////    
    else if (cmdline.find("read data") == 0){
      sscanf(cmdline.c_str(), "read data %d", &num);
      sumo.read_AC(true,0,num);   
      sumo.manage_cc_fifo(1);
    }
    ////    
    else if (cmdline.find("read hdata") == 0){
      sscanf(cmdline.c_str(), "read data %d", &num);
      sumo.read_AC(true,1,num);   
      sumo.manage_cc_fifo(1);
    }
    ////
    else if (cmdline.find("dump data") == 0){
      sumo.dump_data();
    }   
    ////
    else if (cmdline.find("led on") == 0){
      sumo.toggle_LED(true);
    }   
    ////
    else if (cmdline.find("led off") == 0){
      sumo.toggle_LED(false);
    }   
    else if (cmdline.find("cal on") == 0){
      sumo.toggle_CAL(true);
    }
    else if (cmdline.find("cal off") == 0){ 
      sumo.toggle_CAL(false);
    }
    ////
    else if (cmdline.find("log data") == 0){
      sscanf(cmdline.c_str(), "log data %s %d %d", filename, &num, &num2);
      sumo.log_data(filename, num, num2, 10000);
    }
    ////
    else if (cmdline.find("log5 data") == 0){
      int active = sumo.check_active_boards();
      sscanf(cmdline.c_str(), "log5 data %s %d %d", filename, &num, &num2);
      sumo.log_data_hd5(filename, num, num2, 10000, active);
    }
    ////
    else if (cmdline.find("reset dll") == 0){
      sumo.set_usb_read_mode(16);
      usleep(1000);
      sumo.dump_data();
      sumo.dump_data();

      sumo.reset_dll();
      sumo.reset_dll();
      cout << "resetting PSEC4 sampling...\n";
      usleep(4000000);
      cout << "Done.\n";
    }
    ////
    else if (cmdline.find("plot") == 0){
      int again = 1;
      string scope_cmd;
      sumo.load_ped();
      if(cmdline.find("plot lut") == 0){
	sscanf(cmdline.c_str(), "plot lut %d \n", &num);
	if(num > 3 || num < 0)
	  cout << "nope\n";
	else{	 
	  plot_pipe.plot_init();
	  plot_pipe.plot_lut_data(num);
	}
      }

      else if(cmdline.find("plot scope") == 0){
	sscanf(cmdline.c_str(), "plot scope %d", &num);
	plot_pipe.plot_init();
	while(again){
	  fprintf(stdout, "$cope>>");
	  
	  if(sumo.scope_AC(0, false, num) == 1){
	    break;
	  }
	  else{
	    usleep(50000);
	    plot_pipe.plot();
	    //sumo.dump_data();
	    sumo.reset_self_trigger();
	    usleep(20000);
	    getline(cin, scope_cmd);
	  }
      
	  if(scope_cmd.find("n")==0 || scope_cmd.find("q")==0 ){
	    again = 0;
	    plot_pipe.finish();
	  }
	  else
	    again = 1;
	}	  
      }
      else if(cmdline.find("plot sc0pe") == 0){
	sscanf(cmdline.c_str(), "plot sc0pe %d", &num);
	plot_pipe.plot_init();
	while(again){
	  fprintf(stdout, "$c0pe>> ");
	  
	  if(sumo.scope_AC(1, false, num) == 1){
	    break;
	  }
	  else{
	    usleep(50000);
	    plot_pipe.plot();
	    //sumo.dump_data();
	    sumo.reset_self_trigger();
	    sumo.manage_cc_fifo(1);
	    usleep(20000);
	    getline(cin, scope_cmd);
	  }
	  
	  if(scope_cmd.find("n")==0 || scope_cmd.find("q")==0 ){
	    again = 0;
	    plot_pipe.finish();
	  }
	  else
	    again = 1;
	}	  
      }
      else{
	cout << "nothing to plot\n";
      }
    }
    ////	
    else if (cmdline.find("reset trig") == 0){
      sumo.reset_self_trigger();
      cout << "resetting PSEC4 self-trigger...\n";
      usleep(100000);
      cout << "Done.\n";
    }
    ////
    else if (cmdline.find("set selftrig") == 0){
      if (cmdline.find("set selftrig mask") == 0){
	sscanf(cmdline.c_str(), "set selftrig mask%d\n",&num4);
	sumo.set_self_trigger_mask(num4);
	cout << "self trig mask set: " << num4 << endl;
      }
      else{
	sscanf(cmdline.c_str(), "set selftrig %d %d %d %d\n", &num, &num1, &num2, &num3);
	sumo.set_self_trigger((bool)num, (bool)num1, (bool)num2, (bool)num3);
	cout << "config self trigger.." << num << num1 << num2 << num3 << endl;
	if(num == 1)
	  cout << "self trig enabled" << endl;	
	if(num1 == 1)
	  cout << "num1 = 1" << endl;
      }
    }

    ////
    else if (cmdline.find("set htrig on") == 0){
      sumo.set_usb_read_mode(24);
      //sumo.set_usb_read_mode(7);

    }
    else if (cmdline.find("set htrig off") == 0){
      sumo.set_usb_read_mode(16);
      usleep(1000);
      sumo.dump_data();
      sumo.dump_data();
    }
    /////  
    else{
      cout << "invalid entry!! Type 'help' for help\n";
    }
  }
}
