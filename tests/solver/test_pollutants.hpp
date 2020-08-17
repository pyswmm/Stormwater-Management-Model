//
//  test_pollutant.hpp
//
//  Created: Aug 14, 2020
//
//  Author: Abhiram Mullapudi
//        
//
#ifndef TEST_POLLUTANT_HPP
#define TEST_POLLUTANT_HPP

#include "swmm5.h"
#include "toolkitAPI.h"

#define ERR_NONE 0

// Add shared data paths here
#define DATA_PATH_INP "test_example1.inp"
#define DATA_PATH_RPT "tmp.rpt"
#define DATA_PATH_OUT "tmp.out"

#define DATA_PATH_INP_POLLUT "pollutants/tank_constantinflow_constanteffluent.inp"
#define DATA_PATH_RPT_POLLUT "pollutants/tank_constantinflow_constanteffluent.rpt"
#define DATA_PATH_OUT_POLLUT "pollutants/tank_constantinflow_constanteffluent.out"

struct FixtureBeforeStep{
    FixtureBeforeStep() {
        swmm_open(DATA_PATH_INP, DATA_PATH_RPT, DATA_PATH_OUT);
        swmm_start(0);
    }
    ~FixtureBeforeStep() {
        swmm_close();
    }
};

struct FixtureBeforeStep_Pollut{
    FixtureBeforeStep_Pollut() {
        swmm_open(DATA_PATH_INP_POLLUT, DATA_PATH_RPT_POLLUT, DATA_PATH_OUT_POLLUT);
        swmm_start(0);
    }
    ~FixtureBeforeStep_Pollut() {
        swmm_close();
    }
};


#endif
