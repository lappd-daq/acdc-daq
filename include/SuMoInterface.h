#pragma once

#include <string>
#include <map>
#include <vector>

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
    bool hasTriggered(bool force=false);
    SumoData getData();
    void reset();
    void to_csv(std::vector<SumoData> data, std::string filename);
protected:
    SuMo sumo;
    int last_trigger;
    int board_trigger;
    bool is_prepared = false;
    bool board_mask[numFrontBoards];
};



