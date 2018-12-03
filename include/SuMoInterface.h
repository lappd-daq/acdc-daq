#pragma once

#include <string>
#include <map>

#include "SuMo.h"
#include "Packet.h"

class SumoData {
public:
    int metaData[numFrontBoards][psecSampleCells];
    int data[numFrontBoards][AC_CHANNELS][psecSampleCells];

};

class SuMoInterface {
public:
    void configure();
    void configure(std::string filename, bool verbose=false);
    void getStatus();
    void calibrate();
    void prepare();
    bool hasTriggered();
    void forceTrigger();
    SumoData getData();
    void reset();
protected:
    SuMo sumo;
private:
    int last_trigger;
    bool is_prepared = false;
    bool board_mask[numFrontBoards];
};



