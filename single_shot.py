import time
import sys
import subprocess
import datetime
import io
import os

prefix = sys.argv[1]
nevts = sys.argv[2]
trigmode = sys.argv[3]
FNULL = open(os.devnull, 'w')

#signal that this is a clk syncing event, i.e. software trigger
if(prefix == 'clk'):
    args = ["./clk-set-config.sh"]
else:
    args = ["./set-config.sh"]

print "Setting config"
popen = subprocess.Popen(args,stdout=FNULL)
popen.wait()
print "Done setting config" 
#output = popen.stdout.read()
time.sleep(5)


print "Starting logging"
outfiletag = "data/cosmic/"+prefix+"_"+datetime.datetime.strftime(datetime.datetime.now(), "%y%m%d-%H%M%S")   
args = ["./bin/logData", outfiletag, str(int(nevts)), str(int(trigmode))]

outfilename = outfiletag+"_output.txt"
with io.open(outfilename, 'wb') as writer, io.open(outfilename, 'rb',1) as reader:
    process = subprocess.Popen(args,stdout=writer)
    while process.poll() is None:
        sys.stdout.write(reader.read())
        time.sleep(0.5)
    sys.stdout.write(reader.read())

print "Done with: "
print args
time.sleep(5)




