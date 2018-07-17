# acdc-daq
ISU testbench code using Eric Oberla's original acdc-daq teststand code.
# acdc-daq_testing-codes
Automation.cpp:
Measures noise trigger rates as a function of threshold pin voltage for the channels on the board.
Takes Pedestal in DAC counts, Board Number, and savefile name as inputs.
To convert from DAC counts to mV, multiply your value in DAC counts by (1200/4096)
Writes the data to your file in three columns. The first column is channel number, the second column is threshold in DAC counts, and the third column is the number of triggers that occured in 1 second.

Automated_SelfTrig.cpp:
Measures the number of counts in 5 seconds with the calibration input disabled (measuring noise triggers), and the number of counts in 5 seconds with the calibration input enabled. 
Takes Pedestal (in DAC counts), board number, pulse amplitude (mV), and savefile name as inputs.
Writes data to the file in three columns. The first column is threshold voltage in DAC counts, the second is the number of counts in 5 with the calibration input disabled, the third is the number of counts in 5 seconds with the calibration input disabled.

Automed_SelfTrig_Cutoffs.cpp
Works similarly to Automated_SelfTrig.cpp, but instead just writes to file the thresholds corresponding to 50%, 75%, 90%, and 98% efficiencies.
Takes Pedestal in DAC counts, board number, pulse amplitude (mV), pulse frequency (Hz), and savefile name as arguments.
writes data to file in three columns: threshold (in DAC counts), noise trigger rate, cal enabled trigger rate.
There will be 4 rows for each channel. Each row corresponds to the minimum threshold at which self triggering occured with 50%, 75%, 90%, or 98% efficiencies.

