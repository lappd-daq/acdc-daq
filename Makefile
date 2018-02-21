#############################################################################
# Makefile for PSEC4 acdc
# 2014
# ejo
#############################################################################
#Generic and Site Specific Flags
CC=g++ -g -fPIC #-DH5_USE_16_API
#LIBS
LDFLAGS= $(LIBS)  -lusb -lz -lm #-lhdf5
CXXFLAGS=-Wall -Werror -O2

INC= -I./include/
#-I/usr/src/linux-headers-2.6.38-12/include/ \
#-I/usr/src/linux-headers-2.6.38-12/include/linux

EXE=	bin/logData \
	bin/takePed  bin/setPed   bin/ledEn bin/setupLVDS \
	bin/readCC   bin/readACDC bin/Reset \
	bin/calEn    bin/resetDll bin/setConfig \
	bin/dumpData bin/oScope   bin/usbResetDevice \
	bin/makeLUT \

TESTS= tests/test_logData
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

all : $(EXE) #tests

tests : $(TESTS)

obj/%.o : src/%.cpp
	$(CC) $(INC) -c $< -o $@
obj/%.o : src/functions/%.cpp
	$(CC) $(INC) -c $< -o $@
obj/%.o : src/calibrations/%.cpp
	$(CC) $(INC) -c $< -o $@
obj/%.o : src/usb/%.cpp
	$(CC) $(INC) -c $< -o $@
#############################################################################
bin/% : obj/%.o $(OBJS)
	$(CC) $^ $(LDFLAGS) -o $@

#############################################################################
tests/test_% : src/tests/test_%.cpp
	$(CC) $^ $(LDFLAGS) -o $@

#############################################################################
clean:
	@ rm -f $(OBJS) *~ *.o src/*~ include/*~ src/functions/*~ -rf obj/ tests/

cleanall:
	@ rm -f $(OBJS) *~ *.o src/*~ include/*~ src/functions/*~ -rf bin/ obj/ tests/

.PHONY: clean
#############################################################################
