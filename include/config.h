#pragma once

#include "SuMo.h"

void set_default_values();

int parse_acdc_setup_file(const char *file, bool verbose = false);

int parse_trig_setup_file(const char *file, bool verbose = false);

int write_config_to_hardware(SuMo &, bool WRITETRIG, bool WRITEACDC);