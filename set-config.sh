#!/bin/bash
./bin/ledEn 0
./bin/setConfig config/falling_pulse_self_trigger.yml -v
sleep 2
echo taking ped
./bin/takePed
./bin/readACDC
