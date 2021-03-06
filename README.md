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
2. num_events - The number of triggered events to show. If you want to show a continuous output until manualy stopped 
(ctrl-c) set num_events to a value less than 0 (e.g. -1) 
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