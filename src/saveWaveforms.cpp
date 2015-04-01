




int setup_scope(ScopePipe& myPipe){
  
  if(myPipe.init()) return 1;
  //myPipe.send_cmd("set zrange [-1000:1000]");
  myPipe.send_cmd("set term wxt size 610, 480");
  myPipe.send_cmd("set grid x z back");
  myPipe.send_cmd("set xyplane 0");
  myPipe.send_cmd("set view 57, 105, 1, 1.5");
  myPipe.send_cmd("set yrange [0:256]");
  
  myPipe.send_cmd("set xlabel \'channel\' ");
  myPipe.send_cmd("set ylabel \'sample no.\' ");
  //  myPipe.send_cmd("set zlabel \'counts\' ");

  return 0;
}

int prime_scope(SuMo& Sumo, int trig_mode, int device, int boardID, int scopeRefresh){
  if(device==1) Sumo.manage_cc_fifo_slaveDevice(1);
  Sumo.manage_cc_fifo(1);

  if(trig_mode == 1){
      if(device==1) Sumo.reset_self_trigger(15, 1);
      Sumo.reset_self_trigger(15, 0);
      Sumo.prep_sync();
      if(Sumo.mode==USB2x) Sumo.system_slave_card_trig_valid(true);
      Sumo.system_card_trig_valid(true);
      Sumo.make_sync();
   
      Sumo.sys_wait(scopeRefresh)

      Sumo.system_card_trig_valid(false);
      if(Sumo.mode==USB2x) Sumo.system_slave_card_trig_valid(false);

    }
    // otherwise, send trigger over software 
    else if(trig_mode == 0){ 
      if(device==1) Sumo.software_trigger_slaveDevice(1 << boardId-Sumo.boardsPerCC);
      else          Sumo.software_trigger(1 << AC_adr);

      usleep(scopeRefresh);
    }
    
    //int evts = read_CC(false, false, 0);
    //if(mode==USB2x) evts += read_CC(false, false, 1);
    int evts = read_CC(false, false, 100);
    cout << evts << ":";
    cout << EVENT_FLAG[2] << EVENT_FLAG[3] << EVENT_FLAG[4] << EVENT_FLAG[5] 
	 << EVENT_FLAG[6] << EVENT_FLAG[7] << " \r";
    cout.flush();

    if( EVENT_FLAG[AC_adr] == 0 ) continue;
}

int* log_from_scope(SuMo& Sumo,  int boardID){
  
  Sumo.read_AC(1, boardID, false);
  Sumo.set_usb_read_mode(0);
  if(Sumo.mode == USB2x) Sumo.usb_read_mode_slaveDevice(0);
  Sumo.get_AC_info(false, boardID);
      
  for(int i = 0; i < Sumo.AC_CHANNELS; i++){
      if(i>0 && i % 6 == 0) psec_cnt ++;
      
        for(int j = 0; j < psecSampleCells; j++){	
	sample  = Sumo.adcDat[AC_adr]->Sumo.AC_RAW_DATA[psec_cnt][i%6*256+j];
	sample -= Sumo.PED_DATA[AC_adr][i][j];
	
	pdat[i][j] = sample;
	if(fabs(pdat[i][j]) >= SCOPE_AUTOSCALE) max_pdat = fabs(pdat[i][j]);
      }
    }
  
