/////////////////////////////////////////////////////
//  file: ScopePipe.h
//
//  Header for ScopePipe c++ class
//
//  Revision History:
//          01/2012 original version 
//               (created 'self-standing' code, 
//                to separate from main program)
//  Author: ejo
////////////////////////////////////////////////////// 
#pragma once

#include <iostream>
#include <fstream>
#include <string>

using std::string;

class ScopePipe {
public:
    ScopePipe();

    ~ScopePipe();

    int init();

    int plot_init();

    int plot_lut_data(int AC_adr);

    int notrigger();

    int plot();

    int multiplot();

    int send_cmd(const string &plot_cmd);

    int finish();

private:
    FILE *gp_cmd;
    string filename;
    string filename_lut;
    string filename_lut_0;
    string filename_lut_1;
    string filename_lut_2;
    string filename_lut_3;

    string plot_cmd;
};
