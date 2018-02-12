# Tutorial for commands and usage

This page will provide all the script commands' (found in src/functions) descriptions and a quick usage example at the end.

## Commands

**./bin/calEn** enable device  
Toggles the calibration switch for each ACDC card  
Arguments:
* *enable* ("on"/"off", 1/0) - Enable or disable the calibration
* *device* (0-7) - The ACDC board of concern


**./bin/dempData**  
Pulls data from the ACDC cards


**./bin/ledEn** enable  
Toggles the LEDs for all cards  
Arguments:
* *enable* ("on"/"off", 1/0) - Enable or disable the LEDs

**./bin/logData** filename numFrames trigMode  
Logs data taken by the ACDC cards  
Arguments:
* *filename* (path) - The filename for the data to be saved. No extension required
* *numFrames* (int) - The number of events to log
* *trigMode* (0, 1) - The trigger mode for acquisition. 0 is software trigger, 1 is hardware trigger.

**./bin/makeLUT**  
Generate count-to-voltage conversion files for active boards


**./bin/oScope** device numFrames trigMode  
Allows oscilloscope behavior by piping data to gnuplot  
Arguments:
* *device* (0-7) - The ACDC board to connect to
* *numFrames* - The number of event frames to capture
* *trigMode* (0/1) - The trigger mode. 0 is for software trigger and 1 is for hardware trigger

**./bin/readACDC** mode  
Show status for all front-end cards  
Arguments:
* *mode* ("sync"/"-s", "-pull"/"-p") [optional] - read the status in sync mode or pull mode

**./bin/readCC** mode  
Show status for the central cards  
Arguments:
* *mode* ("sync") [optional] - read the status in sync mode

**./bin/Reset** mode  
Reset the front-end cards  
Arguments:
* *mode* ("hard", "time", "usb") [optional] - Reset modes

**./bin/resetDLL** mode  
Reset the sampling lock on front-end cards  
Arguments:
* *mode* ("-sync"/"-s") [optional] - Reset the locks in sync mode

**./bin/setConfig** type file  
Set the configuration for triggering and ACDC values  
Arguments:
* *type* ("-acdc", "-trig") - Choose whether the configuration is for the ACDC settingsg or the trigger setting
* *file* (path) - The path of the configuration file to pass

**./bin/setPed** ped  
Set the front-end card volatge pedestal  
Arguments:
* *ped* (0-4095) - the pedestal value

**./bin/setupLVDS**  
Setup the lvds SERDES interface  

**./bin/takePed**  
Generate pedestal constants for active boards  

## Usage

A typical usage scenario will include setting up the cards, taking pedestal values, and setting trigger settings before logging any data. This might look like the following

```shell
$ ./bin/readACDC
$ ./bin/setConfig -trig config/trigoff.config
$ ./bin/setConfig -acdc config/ACDC.config
$ ./bin/takePed
$ ./bin/setConfig -trig config/trig-laserstand.config
$ ./bin/readACDC
```
There is a shell script provided that does exactly this, called config.sh.
