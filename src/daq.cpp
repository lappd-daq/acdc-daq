#include "SuMoInterface.h"

int main(int argc, char * argv[]) {
    int num_events = 10;
    if (argc == 2) {
        num_events = strtod(argv[1]);
    }

    SuMoInterface s;
    s.configure("../config/rising_pulse.yml");
    s.calibrate();
    s.getStatus();

    return 0;
}