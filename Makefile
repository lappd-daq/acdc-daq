#############################################################################
# Makefile for PSEC4 acdc
# 2013
# ejo
#############################################################################
#Generic and Site Specific Flags
CC=g++ -g -fPIC #-DH5_USE_16_API
#LIBS    
LDFLAGS= $(LIBS)  -lusb -lz -lm -lhdf5
CXXFLAGS=-Wall -O2

INC= -I./include/ 
#-I/usr/src/linux-headers-2.6.38-12/include/ \
#-I/usr/src/linux-headers-2.6.38-12/include/linux

EXE=	SuMo_driver \
	bin/LogData
#############################################################################
OBJS= 	obj/stdUSBl.o \
	obj/SuMo.o \
	obj/ScopePipe.o \
	obj/usb_commands.o \
	obj/log_data_hd5.o
#############################################################################
default:
	if [ ! -e bin	]; then mkdir	bin; fi
	if [ ! -e obj	]; then mkdir   obj; fi
	if [ ! -e calibrations	]; then mkdir   calibrations; fi
	$(MAKE) all

all : $(EXE)

obj/%.o : src/%.cpp
	$(CC) $(INC) -c $< -o $@

#############################################################################
SuMo_driver  : obj/SuMo_driver.o $(OBJS); $(CC) $^ $(LDFLAGS) -o $@
bin/LogData : obj/LogData.o $(OBJS); $(CC) $^ $(LDFLAGS) -o $@
#############################################################################
clean:
	@ rm -f $(OBJS) *~ *.o src/*~ -rf obj/

cleanall:
	@ rm -f $(OBJS) *~ *.o src/*~ -rf bin/ obj/

.PHONY: clean
#############################################################################
