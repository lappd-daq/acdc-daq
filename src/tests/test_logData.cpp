#include <cstdlib>
#include <ctime>
#include <iostream>
#include <vector>

#include "SuMo.h"

using namespace std;

int main(int argc, char *argv[]) {
    SuMo sumo;
    int num_events = 100;
    int boards[] = {2, 4};
    vector<packet_t**> data;
    srand((unsigned)time(0));

    /* Create fake dataset */
    for (int e = 0; e < num_events; e++) {
        for (int b = 0; b < 2; b++) {
            int board = boards[b];
            for (int j = 0; j < psecSampleCells; j++){
                sumo.adcDat[board]->Meta[j] = rand() % 600;
            }
            for(int i = 0; i < AC_CHANNELS; i++){
                for(int j = 0; j < psecSampleCells; j++){
                    int sample = rand() % 3000;
                    sumo.adcDat[board]->Data[i][j] = sample;
                }
            }
        }
        data.push_back(sumo.adcDat);
    }

    sumo.log_data("tests/testdata/test", data, 0);

    return 0;
}
