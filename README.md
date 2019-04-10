# acdc-daq
ISU testbench code using Eric Oberla's original acdc-daq teststand code.

## Prerequisites
This code is built using `cmake` and requires `libusb` headeras (which usually come prepackaged with your os-- you may want to double-check).

To install on a debian-based machine

```bash
$ sudo apt install libusb-dev cmake
```

## Usage

To make and use the functions, issue the following commands
```bash
$ cmake . -Bbuild
$ cmake --build build -- -j4
```

Where `-j4` specifies the number of cores available to multi-thread the process. This will create a directory `build/` 
with the proper build files and then use those to compile and link the libraries and executables.

After this any command can be issued, for example

```bash
$ ./bin/readACDC
```

If you ever need to remake without having changed to CMakeLists file, issue
```bash
$ cmake --build build -- -j4
```
again.

## Commands

There are many executables available after building the project. Below is a list of them

### readCC
This will probe the central card and return the addresses of the connected ADC boards. Useful for debugging connection
```bash
$ ./bin/readCC
``` 

### readACDC
This will read out the settings of any connected ADC boards. This is useful for checking the status of each ADC board
```bash
$ ./bin/readACDC
```

### setConfig
This configures the PSEC electronics given a yaml based configuration file. 
See the `config/` folder examples for more info.

```bash
$ ./bin/setConfig [<filename>] [-v] 
```

**Parameters**

If no parameters set, will set the default (trig off) settings 
1. (optional) filename - the filename of the config file
2. (optional) verbose flag - if provided, will show verbose output

### takePed
This will find the pedestal values of the connected ADC, save them to a text file, and let them be used by further commands
```bash
$ ./bin/takePed
```

### oScope
This will use GNUplot to give a visual representation of the incoming traces. This will mimic the output of an oscilloscope
```bash
$ ./bin/oScope <board> <num_events> <trigger_mode> [<channel>]
```
**Parameters**
1. board - The ADC board to show output for
2. num_events - The number of triggered events to show
3. trigger_mode - 0 for software triggering, 1 for hardware triggering
4. (optional) channel - If set, will plot the given channel and the 5 after it. For example, 
if channel is 10, channels 10-14 will be plotted. If the channel provided is
more than 25, it will be set to 25

### logData
This will log data from the PSEC electronics. For more information, see the data heading
```bash
$ ./bin/logData <file> <num_events> <trigger_mode>
```
**Parameters**
1. file - The prefix of files. Three files will be created `<file>.acdc`, `<file>.ped`, and `<file>.meta`
2. num_events - The number of events to capture
3. trigger_mode - 0 for software trigger, 1 for hardware trigger

### Automation:
Measures noise trigger rates as a function of threshold pin voltage for the channels on the board.
To convert from DAC counts to mV, multiply your value in DAC counts by (1200/4096)
Writes the data to your file in three columns. The first column is channel number, the second column is threshold in DAC counts, and the third column is the number of triggers that occured in 1 second.

```bash
$ ./bin/Automation <pedestal> <board> <filename>
```
**Parameters**
1. pedestal - The pedestal in ADC counts
2. board - The connected ADC board
3. filename - The filename to save output to

### Automated_SelfTrig:
Measures the number of counts in 5 seconds with the calibration input disabled (measuring noise triggers), and the number of counts in 5 seconds with the calibration input enabled. 
Writes data to the file in three columns. The first column is threshold voltage in DAC counts, the second is the number of counts in 5 with the calibration input disabled, the third is the number of counts in 5 seconds with the calibration input disabled.

```bash
$ ./bin/Automated_SelfTrig <pedestal> <board> <amp> <filename>
```
**Parameters**
1. pedestal - The ADC pedestal in ADC counts
2. board - The connected ADC board
3. amp - The pulse amplitude in mV
4. filename - The filename to save output to

### Automated_SelfTrig_Cutoffs
Works similarly to Automated_SelfTrig.cpp, but instead just writes to file the thresholds corresponding to 50%, 75%, 90%, and 98% efficiencies.
Takes Pedestal in DAC counts, board number, pulse amplitude (mV), pulse frequency (Hz), and savefile name as arguments.
writes data to file in three columns: threshold (in DAC counts), noise trigger rate, cal enabled trigger rate.
There will be 4 rows for each channel. Each row corresponds to the minimum threshold at which self triggering occured with 50%, 75%, 90%, or 98% efficiencies.
```bash
$ ./bin/Automated_SelfTrig_Cutoffs <pedestal> <board> <amp> <freq> <filename>
```

**Parameters**
1. pedestal - The ADC pedestal in ADC counts
2. board - The connected ADC board
3. amp - The pulse amplitude in mV
4. freq - The pulse frequency in Hz
5. filename - The filename to save output to

### calEn

### dumpData

### ledEn

### makeLUT

### Reset

### resetDLL

### setPed

### setupLVDS

### toggle_led

### Metadata Descriptions
1. count 			 
  -Example value: 4
  -Description: event number on a board to board basis, starting with 0 at each new call of logData. 
2. aa 			 0
3. time 			 203
4. datetime 			 203
5. events 			 1
6. bin_count_rise 			 
  -Example value: 8
  -Range: 0-9 (occasionally bugged at 12?)
  -Description: Firmware comment possibly links this to the finest scale of timing for when the trigger was made on the ACDC
7. self_trig_settings_2 			 0
8. sys_coincidence_width 			 0
9. coincidence_num_chips 			 0
10. coincidence_num_chans 			 0
11. self_trig_settings 			 1
12. trig_en 			 1
13. trig_wait_for_sys 			 0
14. trig_rate_only 			 0
15. trig_sign 			 0
16. use_sma_trig_input 			 0
17. use_coincidence_settings 			 0
18. use_trig_valid_as_reset 			 0
19. coinc_window 			 0
20. reg_self_trig 			 
  -Example: 128
  -Range: 0 - 1073741823 (2^30)
  -Description: This is a 30 bit word in decimal form that represents the channels on the board that triggered in a self trigger. Is 0 when not in self trigger mode. This value is not ANDED with the self trigger mask. So though the mask may be 0x1, the value here includes discriminators that fired regardless of the mask and can return 1111010101110101... 
21. counts_of_sys_no_local 			 0
22. sys_trig_count 			 0
23. resets_from_firmw 			 0
24. firmware_version 			 22
25. self_trig_mask 			 1073479679
26. dig_timestamp_lo 
  -Example: 23201
  -Range: 0 - 65535 
  -Description: This is a decimal casting of a 15 bit word that keeps time. When the "lo" reaches 65535, at the next tick it goes to 0 and increments the "mid" word. This particular variable counts clock cycles on the ACDC. The lo is a direct clock counter
27. dig_timestamp_mid 			 
  -Example: 23201
  -Range: 0 - 65535 
  -Description: This is a decimal casting of a 15 bit word that keeps time. When the "mid" reaches 65535, at the next tick it goes to 0 and increments the "hi" word.
28. dig_timestamp_hi 			 
  -Example: 23201
  -Range: 0 - 65535 
  -Description: This is a decimal casting of a 15 bit word that keeps time. I don't know what happens when one reaches 65535. I assume the clock resets all three words to 0. 
29. dig_event_count 			 107
30. event_count 			 4

31. timestamp_hi 			 
  -Example: 23201
  -Range: 0 - 65535 
  -Description: This set of timestamps is in sync with "dig_timestamp" except that the "hi" variable can be offset by a constant number. Works like the above two time-like metadata variables. The "lo" field counts clock ticks of the ACDC
32. timestamp_mid 			 
33. timestamp_lo 	

34. CC_BIN_COUNT 			 1
35. CC_EVENT_COUNT 			 32659
36. CC_TIMESTAMP_LO 			 
  -Example: 23201
  -Range: 0 - 65535 
  -Description: Relative time of the central card clock. Works like the above two time-like metadata variables.
37. CC_TIMESTAMP_MID 			 
38. CC_TIMESTAMP_HI 			 

39. ro_cnt_chip_[0-4] 			 
40. ro_target_cnt_chip_[0-4] 			 51712
41. vbias_chip_[0-4] 			 2000
42. trigger_threshold_chip_[0-4] 			 50
43. ro_dac_value_chip_[0-4] 			 1742
44. self_trig_scalar_ch_[1-30] 			 0
45. time_from_valid_to_trig 			 
46. firmware_reset_time 			 
47. last_coincidence_num_chans 			 