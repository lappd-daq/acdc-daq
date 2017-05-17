#!/bin/bash

make

echo "\n\nRunning setup for one board test stand hardware trigger\n\n"

#setup LVDS in case it is not automatically 
./bin/setupLVDS

# sleep so that chips can adjust to correct timing frequency
echo "Waiting for frequency alignment..."
sleep 15

./bin/ledEn 0 # Turn off the LED
./bin/setConfig -trig config/trigoff.config
./bin/setConfig -acdc config/ACDC.config
./bin/takePed
./bin/setConfig -trig config/trig-laserstand.config

# Read ACDC and confirm to start test
./bin/readACDC

echo "Does this look okay? Ctrl-c to cancel, any other to continue."
read dummy

# Read the oscope data
./bin/oScope $1 $2 1
