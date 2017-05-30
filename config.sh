#!/bin/bash

#Write config files
#./bin/ledEn 0 # Turn off the LED
./bin/setConfig -trig config/trigoff.config
./bin/setConfig -acdc config/ACDC.config
sleep 1
./bin/takePed
sleep 1
./bin/setConfig -trig config/trig-laserstand.config

# Read ACDC
./bin/readACDC


