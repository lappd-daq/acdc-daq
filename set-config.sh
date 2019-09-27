#!/bin/bash
./bin/ledEn 0
./bin/setConfig config/cosmic_run5.yml -v
#./bin/setConfig config/test.yml -v
sleep 2
echo taking ped
./bin/takePed
./bin/readACDC
