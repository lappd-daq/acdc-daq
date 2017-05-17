#############################################################################
# Makefile for PSEC4 acdc
# 2014
# ejo
#############################################################################
#Generic and Site Specific Flags
CC=g++ -g -fPIC #-DH5_USE_16_API
#LIBS    
LDFLAGS= $(LIBS)  -lusb -lz -lm #-lhdf5
CXXFLAGS=-Wall -O2

INC= -I./include/ 
#-I/usr/src/linux-headers-2.6.38-12/include/ \
#-I/usr/src/linux-headers-2.6.38-12/include/linux

EXE=	bin/logData \
	bin/takePed  bin/setPed   bin/ledEn bin/setupLVDS \
	bin/readCC   bin/readACDC bin/Reset \
	bin/calEn    bin/resetDll bin/setConfig \
	bin/dumpData bin/oScope   bin/usbResetDevice \
	bin/makeLUT
#############################################################################
OBJS= 	obj/stdUSBl.o obj/stdUSBl_Slave.o\
		obj/SuMo.o \
		obj/ScopePipe.o \
		obj/oscilloscope.o
#	obj/log_data_hd5.o
#############################################################################
default:
	if [ ! -e bin	]; then mkdir	bin; fi
	if [ ! -e obj	]; then mkdir   obj; fi
	if [ ! -e calibrations	]; then mkdir   calibrations; fi
	$(MAKE) all

all : $(EXE)

obj/%.o : src/%.cpp
	$(CC) $(INC) -c $< -o $@
obj/%.o : src/functions/%.cpp
	$(CC) $(INC) -c $< -o $@
obj/%.o : src/calibrations/%.cpp
	$(CC) $(INC) -c $< -o $@
obj/%.o : src/usb/%.cpp
	$(CC) $(INC) -c $< -o $@
#############################################################################
#SuMo_driver  	: obj/SuMo_driver.o $(OBJS); $(CC) $^ $(LDFLAGS) -o $@
bin/logData 	: obj/logData.o		$(OBJS); $(CC) $^ $(LDFLAGS) -o $@
#bin/logH5Data 	: obj/logH5Data.o   	$(OBJS); $(CC) $^ $(LDFLAGS) -o $@
bin/takePed 	: obj/takePed.o     	$(OBJS); $(CC) $^ $(LDFLAGS) -o $@
bin/setPed 	: obj/setPed.o      	$(OBJS); $(CC) $^ $(LDFLAGS) -o $@
bin/readCC   	: obj/readCC.o      	$(OBJS); $(CC) $^ $(LDFLAGS) -o $@
bin/readACDC 	: obj/readACDC.o    	$(OBJS); $(CC) $^ $(LDFLAGS) -o $@
bin/ledEn	: obj/ledEn.o       	$(OBJS); $(CC) $^ $(LDFLAGS) -o $@
bin/calEn	: obj/calEn.o    	$(OBJS); $(CC) $^ $(LDFLAGS) -o $@
bin/setupLVDS	: obj/setupLVDS.o  	$(OBJS); $(CC) $^ $(LDFLAGS) -o $@
bin/Reset	: obj/Reset.o  		$(OBJS); $(CC) $^ $(LDFLAGS) -o $@
bin/resetDll	: obj/resetDll.o  	$(OBJS); $(CC) $^ $(LDFLAGS) -o $@
bin/setConfig	: obj/setConfig.o  	$(OBJS); $(CC) $^ $(LDFLAGS) -o $@
bin/dumpData	: obj/dumpData.o  	$(OBJS); $(CC) $^ $(LDFLAGS) -o $@
bin/oScope	: obj/oScope.o  	$(OBJS); $(CC) $^ $(LDFLAGS) -o $@
bin/makeLUT	: obj/makeLUT.o  	$(OBJS); $(CC) $^ $(LDFLAGS) -o $@
bin/usbResetDevice: 
	g++ -o bin/usbResetDevice src/usb/usbResetDevice.C
#############################################################################
clean:
	@ rm -f $(OBJS) *~ *.o src/*~ include/*~ src/functions/*~ -rf obj/

cleanall:
	@ rm -f $(OBJS) *~ *.o src/*~ include/*~ src/functions/*~ -rf bin/ obj/

.PHONY: clean
#############################################################################
