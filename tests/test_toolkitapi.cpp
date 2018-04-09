/*
 *   test_toolkitAPI.cpp
 *
 *   Created: 03/10/2018
 *   Author: Bryant E. McDonnell
 *           EmNet LLC
 *
 *   Unit testing for SWMM-ToolkitAPI using Boost Test.
 */

// NOTE: Travis installs libboost test version 1.5.4
//#define BOOST_TEST_DYN_LINK

#define BOOST_TEST_MODULE "toolkitAPI"
#include <boost/test/included/unit_test.hpp>

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <math.h>

#include "swmm5.h"
#include "toolkitAPI.h"

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

BOOST_AUTO_TEST_SUITE(test_toolkitapi)

BOOST_AUTO_TEST_CASE(SM_getSimDatetime502) {
    int year, month, day, hours, minutes, seconds;
    
    int error = swmm_getSimulationDateTime(SM_STARTDATE, &year, &month, &day,
                                           &hours, &minutes, &seconds);
    // Testing Error 502 but currently references 105 index. 
    BOOST_CHECK_EQUAL(105, error);
}

BOOST_AUTO_TEST_SUITE_END()

// Adding Fixtures
struct FixtureOpenClose{
    FixtureOpenClose() {
        swmm_open(DATA_PATH_INP, DATA_PATH_RPT, DATA_PATH_OUT);
    }
    ~FixtureOpenClose() {
        swmm_close();
    }
};

struct FixtureBeforeStep{
    FixtureBeforeStep() {
        swmm_open(DATA_PATH_INP, DATA_PATH_RPT, DATA_PATH_OUT);
        swmm_start(0);
    }
    ~FixtureBeforeStep() {
        swmm_close();
    }
};

struct FixtureBeforeClose{
    FixtureBeforeClose() {
        swmm_open(DATA_PATH_INP, DATA_PATH_RPT, DATA_PATH_OUT);
        swmm_start(0);

        int error;
        double elapsedTime = 0.0;
        do
        {
            error = swmm_step(&elapsedTime);
        }while (elapsedTime != 0 && !error);
        BOOST_CHECK_EQUAL(0, error);
        swmm_end();
    }
    ~FixtureBeforeClose() {
        swmm_close();
    }
};

struct FixtureBeforeStart{
    FixtureBeforeStart() {
        swmm_open(DATA_PATH_INP, DATA_PATH_RPT, DATA_PATH_OUT);
    }
    ~FixtureBeforeStart() {
        int error;
        double elapsedTime = 0.0;
        do
        {
            error = swmm_step(&elapsedTime);
        }while (elapsedTime != 0 && !error);
        if (!error) swmm_end();
        if (!error) swmm_report();

        swmm_close();
    }
};

BOOST_AUTO_TEST_SUITE(test_toolkitapi_fixture)

BOOST_FIXTURE_TEST_CASE(SimSetDatetime503, FixtureBeforeStep) {
    char *newStart = "05/20/1986 18:43:01";
    int error = swmm_setSimulationDateTime(SM_STARTDATE, newStart);
    // Testing Error 503 but currently references 106 index. 
    BOOST_REQUIRE(error == 106);
}

BOOST_FIXTURE_TEST_CASE(SimGetDatetime, FixtureOpenClose) {
    int year, month, day, hours, minutes, seconds;
    
    int error = swmm_getSimulationDateTime(SM_STARTDATE, &year, &month, &day,
                               &hours, &minutes, &seconds);
    BOOST_REQUIRE(error == 0);
    BOOST_CHECK_EQUAL(1998, year);
    BOOST_CHECK_EQUAL(1, month);
    BOOST_CHECK_EQUAL(1, day);
    BOOST_CHECK_EQUAL(0, hours);
    BOOST_CHECK_EQUAL(0, minutes);
    BOOST_CHECK_EQUAL(0, seconds);

    error = swmm_getSimulationDateTime(SM_ENDDATE, &year, &month, &day,
                               &hours, &minutes, &seconds);
    BOOST_REQUIRE(error == 0);
    BOOST_CHECK_EQUAL(1998, year);
    BOOST_CHECK_EQUAL(1, month);
    BOOST_CHECK_EQUAL(2, day);
    BOOST_CHECK_EQUAL(12, hours);
    BOOST_CHECK_EQUAL(0, minutes);
    BOOST_CHECK_EQUAL(0, seconds);

    error = swmm_getSimulationDateTime(SM_REPORTDATE, &year, &month, &day,
                               &hours, &minutes, &seconds);
    BOOST_REQUIRE(error == 0);
    BOOST_CHECK_EQUAL(1998, year);
    BOOST_CHECK_EQUAL(1, month);
    BOOST_CHECK_EQUAL(1, day);
    BOOST_CHECK_EQUAL(0, hours);
    BOOST_CHECK_EQUAL(0, minutes);
    BOOST_CHECK_EQUAL(0, seconds);
}

BOOST_FIXTURE_TEST_CASE(SimSetDatetime, FixtureOpenClose) {
    int year, month, day, hours, minutes, seconds;
    
    char *newStart = "05/20/1986 18:43:01";
    int error = swmm_setSimulationDateTime(SM_STARTDATE, newStart);
    BOOST_REQUIRE(error == 0);
    error = swmm_getSimulationDateTime(SM_STARTDATE, &year, &month, &day,
                               &hours, &minutes, &seconds);
    BOOST_REQUIRE(error == 0);
    BOOST_CHECK_EQUAL(1986, year);
    BOOST_CHECK_EQUAL(5, month);
    BOOST_CHECK_EQUAL(20, day);
    BOOST_CHECK_EQUAL(18, hours);
    BOOST_CHECK_EQUAL(43, minutes);
    BOOST_CHECK_EQUAL(1, seconds);

    char *newEnd = "05/21/1986 18:43:01";
    error = swmm_setSimulationDateTime(SM_ENDDATE, newEnd);
    BOOST_REQUIRE(error == 0);
    error = swmm_getSimulationDateTime(SM_ENDDATE, &year, &month, &day,
                               &hours, &minutes, &seconds);
    BOOST_REQUIRE(error == 0);
    BOOST_CHECK_EQUAL(1986, year);
    BOOST_CHECK_EQUAL(5, month);
    BOOST_CHECK_EQUAL(21, day);
    BOOST_CHECK_EQUAL(18, hours);
    BOOST_CHECK_EQUAL(43, minutes);
    BOOST_CHECK_EQUAL(1, seconds);

    error = swmm_setSimulationDateTime(SM_REPORTDATE, newStart);
    BOOST_REQUIRE(error == 0);
    error = swmm_getSimulationDateTime(SM_REPORTDATE, &year, &month, &day,
                               &hours, &minutes, &seconds);
    BOOST_REQUIRE(error == 0);
    BOOST_CHECK_EQUAL(1986, year);
    BOOST_CHECK_EQUAL(5, month);
    BOOST_CHECK_EQUAL(20, day);
    BOOST_CHECK_EQUAL(18, hours);
    BOOST_CHECK_EQUAL(43, minutes);
    BOOST_CHECK_EQUAL(1, seconds);
}


BOOST_AUTO_TEST_SUITE_END()