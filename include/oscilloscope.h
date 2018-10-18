#pragma once

#include "Definitions.h"
#include "SuMo.h"
#include "ScopePipe.h"

int setup_scope(ScopePipe &, int boardID);

int setup_scope(SuMo &, int boardID);

int prime_scope(SuMo &, int trig_mode, int device, int boardID, int scopeRefresh);

int log_from_scope(SuMo &, int boardID, int pdat[AC_CHANNELS][psecSampleCells], bool *scopeBoard);

int plot_scope(ScopePipe &, int pdat[AC_CHANNELS][psecSampleCells], int *range);

int oscilloscope(SuMo &, int trig_mode, int numFrames, int boardID, int range[2]);

