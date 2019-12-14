import time
import sys
import subprocess
import datetime
import io
import os

prefix = sys.argv[1]
nevts = sys.argv[2] #number of events per turn
trigmode = sys.argv[3] 

count = 0

FNULL = open(os.devnull, 'w')

while True:
     
    #don't take peds when there is a sin sync signal
    if(count % 3 == 0):
        print "Taking a clock sync dataset"
        print "Turning off sync signal"
        args = ["python", "korod-control/power_off.py", "0"]
        process_sync0 = subprocess.Popen(args, stdout=FNULL)
        process_sync0.wait()

        print "Running single shot of sw trigger"
        args = ["python", "single_shot.py", "clk", "1", "0"]
        process1 = subprocess.Popen(args,stdout=FNULL)
        process1.wait()
        print "Done clock syncing"
        time.sleep(5)
   

    print "Turning off sync signal"
    args = ["python", "korod-control/power_off.py", "0"]
    process_sync1 = subprocess.Popen(args, stdout=FNULL)
    process_sync1.wait()

    print "Setting runtime config"
    args = ["./set-config.sh"]
    process2 = subprocess.Popen(args,stdout=FNULL)
    process2.wait()
    print "Done setting config" 
    time.sleep(5)
    
    print "Turning on sync signal"
    args = ["python", "korod-control/power_on.py", "0"]
    process_sync2 = subprocess.Popen(args, stdout=FNULL)
    process_sync2.wait()
   
    print "Waiting for oscillator to warm up"
    time.sleep(20)

    print "Starting logging"
    outfiletag = "data/cosmic/"+prefix+"_"+datetime.datetime.strftime(datetime.datetime.now(), "%y%m%d-%H%M%S")   
    args = ["./bin/logData", outfiletag, str(int(nevts)), str(int(trigmode))]

    outfilename = outfiletag+"_output.txt"
    with io.open(outfilename, 'wb') as writer, io.open(outfilename, 'rb',1) as reader:
        process3 = subprocess.Popen(args, stdout=writer)
        while process3.poll() is None:
            sys.stdout.write(reader.read())
            time.sleep(0.5)
            sys.stdout.write(reader.read())
    
    print "Deleting output file"
    os.remove(outfilename)

    print "Done with: "
    print args
    time.sleep(5)
    count += 1




