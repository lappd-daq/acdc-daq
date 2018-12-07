#pragma once

#include <string>
#include <map>
#include <vector>

#include "SuMo.h"
#include "Packet.h"

class SumoData {
public:
    std::map<int, std::map<std::string, unsigned int> > meta;
    std::map<int, std::map<int, std::vector<int> > > data;
};

std::map<std::string, unsigned int> map_metadata(int meta_array[]);

class SuMoInterface {
public:
    void configure();

    void configure(std::string filename, bool verbose = false);

    void getStatus();

    void calibrate();

    bool hasTriggered(bool force = false);

    SumoData getData();

    void reset();

    void data_to_csv(std::vector<SumoData> data, std::string filename);

    void meta_to_csv(std::vector<SumoData> data, std::string filename);

protected:
    SuMo sumo;
};



