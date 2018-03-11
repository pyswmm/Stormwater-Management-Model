/*
 *   test_output.cpp
 *
 *   Created: 11/2/2017
 *   Author: Michael E. Tryby
 *           US EPA - ORD/NRMRL
 *
 *   Unit testing for SWMM outputapi using Boost Test.
 */

// NOTE: Travis installs libboost test version 1.5.4
//#define BOOST_TEST_DYN_LINK

#define BOOST_TEST_MODULE "swmm5"
#include <boost/test/included/unit_test.hpp>

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <math.h>

#include "swmm5.h"


// NOTE: Test Input File
#define DATA_PATH_INP "./swmm_api_test.inp"
#define DATA_PATH_RPT "./swmm_api_test.rpt"
#define DATA_PATH_OUT "./swmm_api_test.out"

using namespace std;

// Custom test to check the minimum number of correct decimal digits between 
// the test and the ref vectors.  
bool tolCheck(float a, float b, float tol)
{
    return abs(a - b) <= tol;
}

BOOST_AUTO_TEST_SUITE (test_swmm_auto)

BOOST_AUTO_TEST_CASE(RunTest) {
    int error = swmm_run(DATA_PATH_INP, DATA_PATH_RPT, DATA_PATH_OUT);
    BOOST_REQUIRE(error == 0);
    swmm_close();
}

BOOST_AUTO_TEST_CASE(OpenTest) {
    int error = swmm_open(DATA_PATH_INP, DATA_PATH_RPT, DATA_PATH_OUT);
    BOOST_REQUIRE(error == 0);
    swmm_close();
}

BOOST_AUTO_TEST_CASE(FailOpenTest) {
    int error = swmm_open("", "", "");
    BOOST_REQUIRE(error != 0);
    swmm_close();
}

BOOST_AUTO_TEST_CASE(ManualRun) {
    int error = swmm_open(DATA_PATH_INP, DATA_PATH_RPT, DATA_PATH_OUT);
    error = swmm_start(0);
    double elapsedTime;
    do
    {
        error = swmm_step(&elapsedTime);
        BOOST_REQUIRE(error == 0);
    }while (elapsedTime != 0);
    error = swmm_end();
    BOOST_REQUIRE(error == 0);
    error = swmm_report();
    BOOST_REQUIRE(error == 0);
    float runoffErr, flowErr, qualErr;
    error = swmm_getMassBalErr(&runoffErr, &flowErr, &qualErr);
    BOOST_REQUIRE(error == 0);
    BOOST_CHECK(tolCheck(runoffErr, -0.272, 0.001));
    BOOST_CHECK(tolCheck(flowErr, 0.101, 0.001));
    BOOST_CHECK(tolCheck(qualErr, -0.154, 0.001));
    error = swmm_close();
    BOOST_REQUIRE(error == 0);
}

BOOST_AUTO_TEST_CASE(ManualRunDetailReport) {
    int error = swmm_open(DATA_PATH_INP, DATA_PATH_RPT, DATA_PATH_OUT);
    error = swmm_start(1);
    double elapsedTime;
    do
    {
        error = swmm_step(&elapsedTime);
        BOOST_REQUIRE(error == 0);
    }while (elapsedTime != 0);
    error = swmm_end();
    BOOST_REQUIRE(error == 0);
    error = swmm_report();
    BOOST_REQUIRE(error == 0);
    error = swmm_close();
    BOOST_REQUIRE(error == 0);
}

BOOST_AUTO_TEST_SUITE_END()
