#include "SuMoInterface.h"
#include "Timer.h"
#include <iostream>
#include <vector>
using namespace std;

int main(int argc, char * argv[]) {
    int max_events = 10;
    if (argc == 2) {
        max_events = stoi(argv[1]);
    }

    SuMoInterface s;
    s.configure("../config/rising_pulse.yml");
//    s.configure();
    s.calibrate();
    s.getStatus();
    vector<SumoData> d;
    Timer timer = Timer();
    timer.start();
    time(0);
    float t = timer.stop();
    while (d.size() < max_events) {
        t = timer.stop();
        cout << "Attempting read for event " << d.size() << " " << t << "s\r" << flush;
        if (s.hasTriggered()) {
            try {
                d.push_back(s.getData());
            } catch (runtime_error) {
                cout << "Invalid readout. Data will be dumped." << endl;
            }
        }
    }
    s.data_to_csv(d, "../test.csv");
    s.meta_to_csv(d, "../test_meta.csv");
    return 0;
}

