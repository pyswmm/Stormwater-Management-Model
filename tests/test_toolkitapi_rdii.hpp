/*
 *   fixtures.hpp
 *
 *   Created: 01/12/2020
 *   Author: SM Masud Rana
 *           University of Cincinnati, EmNet LLC
 *
 *   Fixtures for Unit testing of the rdii components of SWMM-ToolkitAPI using Boost Test.
 */
 
#include <boost/test/included/unit_test.hpp>
 
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <math.h>

#include "swmm5.h"
#include "toolkitAPI.h"

// NOTE: Test Input File
#define DATA_PATH_INP "rdii/swmm_rdii.inp"
#define DATA_PATH_RPT "rdii/swmm_rdii.rpt"
#define DATA_PATH_OUT "rdii/swmm_rdii.out"

#define DATA_PATH_INP2 "rdii/swmm_rdii2.inp"
#define DATA_PATH_RPT2 "rdii/swmm_rdii2.rpt"
#define DATA_PATH_OUT2 "rdii/swmm_rdii2.out"

#define DATA_PATH_INP3 "rdii/swmm_rdii3.inp"
#define DATA_PATH_RPT3 "rdii/swmm_rdii3.rpt"
#define DATA_PATH_OUT3 "rdii/swmm_rdii3.out"

using namespace std;

struct FixtureGetRTK {
	FixtureGetRTK() {
		swmm_open((char *)DATA_PATH_INP2, (char *)DATA_PATH_RPT2, (char *)DATA_PATH_OUT2);
	}
	~FixtureGetRTK() {
		swmm_close();
	}
	double rtk[18];
	int unit_hyd_index = 0;
};

struct FixtureMatchFlow {
	FixtureMatchFlow() {
	}
	~FixtureMatchFlow() {
	}
	double rtk[18];
	int unit_hyd_index = 0;
};