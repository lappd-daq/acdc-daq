#############################################################################
# Makefile for PSEC4 acdc
# 2014
# ejo
#############################################################################
#Generic and Site Specific Flags
LDFLAGS := $(LIBS)  -lusb -lz -lm #-lhdf5
CXXFLAGS := -O2 -g -fPIC
CC := g++  $(CXXFLAGS)#-DH5_USE_16_API

INC= -I./include/
#-I/usr/src/linux-headers-2.6.38-12/include/ \
#-I/usr/src/linux-headers-2.6.38-12/include/linux

EXE=	bin/logData \
	bin/takePed  bin/setPed   bin/ledEn bin/setupLVDS \
	bin/readCC   bin/readACDC bin/Reset \
	bin/calEn    bin/resetDll bin/setConfig \
	bin/dumpData bin/oScope   bin/usbResetDevice \
	bin/makeLUT \
	tests/test_logData
#############################################################################
OBJS= 	obj/stdUSBl.o obj/stdUSBl_Slave.o\
		obj/SuMo.o \
		obj/ScopePipe.o \
		obj/oscilloscope.o \
#	obj/log_data_hd5.o
#############################################################################
default:
	if [ ! -e bin	]; then mkdir -p bin; fi
	if [ ! -e obj	]; then mkdir -p obj; fi
	if [ ! -e tests ]; then mkdir -p tests; fi
	if [ ! -e tests/data ]; then mkdir -p tests/testdata; fi
	if [ ! -e calibrations	]; then mkdir -p calibrations; fi
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
obj/%.o : src/tests/%.cpp
	$(CC) $(INC) -c $< -o $@

#############################################################################
bin/% : obj/%.o $(OBJS)
	$(CC) $(INC) $^ $(LDFLAGS) -o $@
bin/usbResetDevice:
	g++ -o bin/usbResetDevice src/usb/usbResetDevice.C
#############################################################################
tests/% : obj/%.o $(OBJS)
	$(CC) $(INC) $^ $(LDFLAGS) -o $@

#############################################################################
clean:
	@ rm -rf $(OBJS) *~ *.o src/*~ include/*~ src/functions/*~ -rf obj/ tests/

cleanall:
	@ rm -rf $(OBJS) *~ *.o src/*~ include/*~ src/functions/*~ -rf bin/ obj/ tests/

.PHONY: clean
#############################################################################
