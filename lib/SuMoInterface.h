#pragma once

#include <string>

#include "SuMo.h"
#include "Packet.h"

class SuMoInterface {
public:
    void configure();
    void configure(std::string filename, bool verbose=false);
    void getStatus();
    void calibrate();
    void prepare();
    bool hasTriggered();
    void forceTrigger();
    packet_t getData();
    void reset();
protected:
    SuMo sumo;
private:
    int last_trigger;
    bool is_prepared = false;
};

