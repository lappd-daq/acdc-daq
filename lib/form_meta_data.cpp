#include <iostream>
#include <cstdlib>
#include <stdio.h>
#include <fstream>
#include <iomanip>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include "SuMo.h"

using namespace std;

int *SuMo::get_AC_info(bool PRINT, int frontEnd, bool PRINTALL, int count, double time,
                       double dateTime, int evts) {

    int aa = frontEnd;
    unsigned short AC_INFO[numChipsOnBoard][infoBuffersize];
    for (int i = 0; i < numChipsOnBoard; i++) {
        for (int j = 0; j < infoBuffersize; j++) {
            AC_INFO[i][j] = adcDat[aa]->AC_INFO[i][j];
        }
    }
    char meta[100][300];
    for (int i = 0; i < psecSampleCells; i++) {
        metaData[i] = 0;
    }
    unsigned short ref_volt_mv = 1200;
    unsigned short num_bits = 4096;

    metaData[0] = count;
    sprintf(meta[0], "%s=%i", "metaData[0] = count", metaData[0]);
    metaData[1] = aa;
    metaData[2] = (int) time;
    metaData[3] = (int) dateTime;
    metaData[4] = evts;


    for (int i = 0; i < numChipsOnBoard; i++) {
        metaData[40 + i] = adcDat[aa]->ro_cnt[i] = AC_INFO[i][0];   //(float) AC_INFO[i][4] * 10 * pow(2,11)/ (pow(10,6));
        metaData[45 + i] = adcDat[aa]->ro_target_cnt[i] = AC_INFO[i][1];  //(float) AC_INFO[i][5] * 10 * pow(2,11)/(pow(10,6));
        metaData[50 + i] = adcDat[aa]->vbias[i] = AC_INFO[i][2];   //(int) AC_INFO[i][6];
        metaData[55 + i] = adcDat[aa]->trigger_threshold[i] = AC_INFO[i][3];  //(float) AC_INFO[i][7] * ref_volt_mv/num_bits;
        metaData[60 + i] = adcDat[aa]->ro_dac_value[i] = AC_INFO[i][4];    //(float) AC_INFO[i][8] * ref_volt_mv/num_bits;
    }

    int ab = metaData[35] = adcDat[aa]->CC_BIN_COUNT = (adcDat[aa]->CC_HEADER_INFO[1] & 0x18) >> 3;
    int bb = metaData[36] = adcDat[aa]->CC_EVENT_COUNT = adcDat[aa]->CC_HEADER_INFO[3] |
            adcDat[aa]->CC_HEADER_INFO[2] << 16;
    int cc = metaData[37] = adcDat[aa]->CC_TIMESTAMP_LO = adcDat[aa]->CC_HEADER_INFO[4];
    int dd = metaData[38] = adcDat[aa]->CC_TIMESTAMP_MID = adcDat[aa]->CC_HEADER_INFO[5];
    int ee = metaData[39] = adcDat[aa]->CC_TIMESTAMP_HI = adcDat[aa]->CC_HEADER_INFO[6];

    int ff = metaData[7] = adcDat[aa]->bin_count_rise = AC_INFO[0][9] & 0x0F;

    int gg = metaData[8] = adcDat[aa]->self_trig_settings_2 = AC_INFO[0][9] >> 4;
    metaData[9] = adcDat[aa]->sys_coincidence_width = adcDat[aa]->self_trig_settings_2 & 0x007;
    metaData[10] = adcDat[aa]->coincidence_num_chips = (adcDat[aa]->self_trig_settings_2 & 0x038) >> 3;
    metaData[11] = adcDat[aa]->coincidence_num_chans = (adcDat[aa]->self_trig_settings_2 & 0xFC0) >> 6;

    int hh = metaData[12] = adcDat[aa]->self_trig_settings = AC_INFO[1][9] & 0x3FF;
    int ii = metaData[13] = adcDat[aa]->trig_en = adcDat[aa]->self_trig_settings & 0x1;
    int jj = metaData[14] = adcDat[aa]->trig_wait_for_sys = adcDat[aa]->self_trig_settings & 0x2;
    int kk = metaData[15] = adcDat[aa]->trig_rate_only = adcDat[aa]->self_trig_settings & 0x4;
    int ll = metaData[16] = adcDat[aa]->trig_sign = adcDat[aa]->self_trig_settings & 0x8;
    metaData[17] = adcDat[aa]->use_sma_trig_input = adcDat[aa]->self_trig_settings & 0x10;
    metaData[18] = adcDat[aa]->use_coincidence_settings = adcDat[aa]->self_trig_settings & 0x20;
    metaData[19] = adcDat[aa]->use_trig_valid_as_reset = adcDat[aa]->self_trig_settings & 0x40;
    metaData[20] = adcDat[aa]->coincidence_window = adcDat[aa]->self_trig_settings >> 7;
    int ad = metaData[112] = adcDat[aa]->last_coincidence_num_chans = AC_INFO[1][9] & 0xF800;

    int ac = metaData[111] = adcDat[aa]->firmware_reset_time = AC_INFO[2][9] | AC_INFO[3][9] << 16;
    int mm = metaData[21] = adcDat[aa]->reg_self_trig = AC_INFO[0][10] | AC_INFO[1][10] << 16;
    int nn = metaData[22] = adcDat[aa]->counts_of_sys_no_local = AC_INFO[2][10] | AC_INFO[3][10] << 16;
    int oo = metaData[23] = adcDat[aa]->sys_trig_count = AC_INFO[4][10];
    int pp = metaData[24] = adcDat[aa]->resets_from_firmw = AC_INFO[0][11] | AC_INFO[1][11] << 16;

    metaData[25] = adcDat[aa]->firmware_version = AC_INFO[2][11];

    int qq = metaData[26] = adcDat[aa]->self_trig_mask = AC_INFO[3][11] | AC_INFO[4][11] << 16;

    int rr_a = metaData[27] = adcDat[aa]->dig_timestamp_lo = AC_INFO[0][12];
    int rr_b = metaData[28] = adcDat[aa]->dig_timestamp_mid = AC_INFO[1][12];
    int rr_c = metaData[29] = adcDat[aa]->dig_timestamp_hi = AC_INFO[2][12] & 0x00F;
    int ss = metaData[30] = adcDat[aa]->dig_event_count = AC_INFO[3][12] |
            AC_INFO[4][12] << 16;  //now another event counter
    int bc = metaData[110] = adcDat[aa]->time_from_valid_to_trig = AC_INFO[2][12] & 0xFF0;
    int tt = metaData[31] = adcDat[aa]->event_count = AC_INFO[3][13] | AC_INFO[4][13] << 16;
    int uu = metaData[32] = adcDat[aa]->timestamp_hi = AC_INFO[2][13];
    int vv = metaData[33] = adcDat[aa]->timestamp_mid = AC_INFO[1][13];
    int ww = metaData[34] = adcDat[aa]->timestamp_lo = AC_INFO[0][13];
    cout << uu << ":" << vv << ":" << ww << endl;

    for (int j = 0; j < AC_CHANNELS; j++) {
        metaData[j + 70] = adcDat[aa]->self_trig_scalar[j];
    }
    if (PRINT) {

        cout << std::fixed;
        cout << std::setprecision(2);
        cout << std::dec << endl;
        cout << "central card header info: " << endl;
        cout << "bin:" << ab
             << " evt.count:" << bb
             << " timestamp: " << ee << ":" << dd << ":" << cc << endl;
        cout << "--------" << endl;
        cout << "event count: " << std::dec << tt;
        cout << " board time: " << uu << ":" << vv << ":" << ww << endl;
        cout << "digtz count: " << std::dec << ss;
        cout << "   ADC time: " << rr_c << ":" << rr_b << ":" << rr_a << endl;

        cout << "self-trig mask:       0x" << hex << qq << endl;
        cout << "self-trig settings:   0x" << hex << hh << endl;
        cout << "self-trig settings_2: 0x" << hex << gg << endl;
        cout << "mis-trig_count          " << dec << nn << endl;
        cout << "firmware-self resets    " << dec << pp << endl;
        cout << "firmware reset time     " << dec << ac << endl;
        cout << "sys trig count on AC/DC " << dec << oo << endl;
        cout << "last time valid->digtz  " << dec << bc << endl;
        cout << "last num-chan trig self " << dec << ad << endl;

        cout << "trig_sign: " << ll
             << ", wait_for_sys: " << jj
             << ", rate_only: " << kk
             << ", EN: " << ii
             << endl;
        cout << "bin count rise edge: " << ff
             << endl;

        for (int i = 0; i < 5; i++) {
            cout << "PSEC:" << i;
            cout << "|ADC clock/trgt:" << adcDat[aa]->ro_cnt[i] * 10 * pow(2, 11) / (pow(10, 6));
            cout << "/" << adcDat[aa]->ro_target_cnt[i] * 10 * pow(2, 11) / (pow(10, 6)) << "MHz";
            cout << ",ro-bias:" << adcDat[aa]->ro_dac_value[i] * ref_volt_mv / num_bits << "mV";
            cout << "|Ped:" << dec << adcDat[aa]->vbias[i] * ref_volt_mv / num_bits << "mV";
            cout << "|Trig:" << adcDat[aa]->trigger_threshold[i] * ref_volt_mv / num_bits << "mV";
            cout << endl;
        }
    }
    return metaData;
}
