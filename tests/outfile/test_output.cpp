/*
 ******************************************************************************
 Project:      OWA SWMM
 Version:      5.1.14
 Module:       test_output.cpp
 Description:  tests for output library functions
 Authors:      see AUTHORS
 Copyright:    see AUTHORS
 License:      see LICENSE
 Last Updated: 12/21/2020
 ******************************************************************************
 */

#define BOOST_TEST_MODULE "output"
#include <boost/test/included/unit_test.hpp>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>

#include "swmm_output.h"

// NOTE: Reference data for the unit tests is currently tied to SWMM 5.1.7
#define DATA_PATH "./test_example1.out"

using namespace std;

// Checks for minimum number of correct decimal digits
boost::test_tools::predicate_result check_cdd_float(std::vector<float>& test,
    std::vector<float>& ref, long cdd_tol){

    float tmp, min_cdd = 10.0;

    // TODO: What if the vectors aren't the same length?

    std::vector<float>::iterator test_it;
    std::vector<float>::iterator ref_it;

    for (test_it = test.begin(), ref_it = ref.begin();
        (test_it < test.end()) && (ref_it < ref.end());
        ++test_it, ++ref_it)
    {
        if (*test_it != *ref_it)
        {
            // Compute log absolute error
            tmp = abs(*test_it - *ref_it);
            if (tmp < 1.0e-7f)
                tmp = 1.0e-7f;

            else if (tmp > 2.0f)
                tmp = 1.0f;

            tmp = -log10(tmp);
            if (tmp < 0.0f)
                tmp = 0.0f;

            if (tmp < min_cdd)
                min_cdd = tmp;
        }
    }
    return floor(min_cdd) >= cdd_tol;
}

boost::test_tools::predicate_result check_string(std::string test,
    std::string ref) {

    if (ref.compare(test) == 0)
        return true;
    else
        return false;
}

BOOST_AUTO_TEST_SUITE(test_output_auto)

BOOST_AUTO_TEST_CASE(InitTest) {
    SMO_Handle p_handle = NULL;

    int error = SMO_init(&p_handle);
    BOOST_REQUIRE(error == 0);
    BOOST_CHECK(p_handle != NULL);

    SMO_close(p_handle);

}

BOOST_AUTO_TEST_CASE(CloseTest) {
    SMO_Handle p_handle = NULL;
    SMO_init(&p_handle);

    int error = SMO_close(p_handle);
    BOOST_REQUIRE(error == 0);
}

BOOST_AUTO_TEST_CASE(InitOpenCloseTest) {
    std::string path     = std::string(DATA_PATH);
    SMO_Handle p_handle = NULL;
    SMO_init(&p_handle);

    int error = SMO_open(p_handle, path.c_str());
    BOOST_REQUIRE(error == 0);

    SMO_close(p_handle);
}

BOOST_AUTO_TEST_CASE(OpenNonexistentFileTest) {
    SMO_Handle p_handle = NULL;
    int error = SMO_init(&p_handle);
    BOOST_REQUIRE(error == 0);

    // Try to open a file that does not exist
    std::string bad_path = "./this_file_does_not_exist.out";
    error = SMO_open(p_handle, bad_path.c_str());

    // Should return error code 434 (file open error)
    BOOST_CHECK_EQUAL(error, 434);
}

BOOST_AUTO_TEST_SUITE_END()

struct Fixture {
    Fixture() {
        std::string path = std::string(DATA_PATH);

        error = SMO_init(&p_handle);
        SMO_clearError(p_handle);
        error = SMO_open(p_handle, path.c_str());

        array     = NULL;
        array_dim = 0;
    }
    ~Fixture() {
        SMO_freeMemory((void*)array);
        error = SMO_close(p_handle);
    }

    std::string path;
    int         error;
    SMO_Handle  p_handle;

    float* array;
    int    array_dim;
};

BOOST_AUTO_TEST_SUITE(test_output_fixture)

BOOST_FIXTURE_TEST_CASE(test_getVersion, Fixture) {
    int version;

    error = SMO_getVersion(p_handle, &version);
    BOOST_REQUIRE(error == 0);

    BOOST_CHECK_EQUAL(51000, version);
}

BOOST_FIXTURE_TEST_CASE(test_getProjectSize, Fixture) {
    int* i_array = NULL;

    error = SMO_getProjectSize(p_handle, &i_array, &array_dim);
    BOOST_REQUIRE(error == 0);

    std::vector<int> test;
    test.assign(i_array, i_array + array_dim);

    // subcatchs, nodes, links, pollutants
    const int ref_dim            = 5;
    int       ref_array[ref_dim] = {8, 14, 13, 1, 2};

    std::vector<int> ref;
    ref.assign(ref_array, ref_array + ref_dim);

    BOOST_CHECK_EQUAL_COLLECTIONS(ref.begin(), ref.end(), test.begin(), test.end());

    SMO_freeMemory((void*)i_array);
}

BOOST_FIXTURE_TEST_CASE(test_getUnits, Fixture) {
    int*      i_array            = NULL;

    error = SMO_getUnits(p_handle, &i_array, &array_dim);
    BOOST_REQUIRE(error == 0);

    std::vector<int> test;
    test.assign(i_array, i_array + array_dim);

    // unit system, flow units, pollut units
    const int        ref_dim            = 4;
    const int        ref_array[ref_dim] = {SMO_US, SMO_CFS, SMO_MG, SMO_UG};

    std::vector<int> ref;
    ref.assign(ref_array, ref_array + ref_dim);

    BOOST_CHECK_EQUAL_COLLECTIONS(ref.begin(), ref.end(), test.begin(), test.end());

    SMO_freeMemory((void*)i_array);
}

// BOOST_FIXTURE_TEST_CASE(test_getFlowUnits, Fixture) {
//     int units = -1;
//
//     error = SMO_getFlowUnits(p_handle, &units);
//     BOOST_REQUIRE(error == 0);
//     BOOST_CHECK_EQUAL(0, units);
// }
//
// BOOST_FIXTURE_TEST_CASE(test_getPollutantUnits, Fixture) {
//     int* i_array = NULL;
//
//     error = SMO_getPollutantUnits(p_handle, &i_array, &array_dim);
//     BOOST_REQUIRE(error == 0);
//
//     std::vector<int> test;
//     test.assign(i_array, i_array + array_dim);
//
//     const int ref_dim            = 2;
//     int       ref_array[ref_dim] = {0, 1};
//
//     std::vector<int> ref;
//     ref.assign(ref_array, ref_array + ref_dim);
//
//     BOOST_CHECK_EQUAL_COLLECTIONS(ref.begin(), ref.end(), test.begin(),
//                                   test.end());
//
//     SMO_freeMemory((void**)&i_array);
//     BOOST_CHECK(i_array == NULL);
// }

BOOST_FIXTURE_TEST_CASE(test_getStartDate, Fixture) {
    double date = -1;

    error = SMO_getStartDate(p_handle, &date);
    BOOST_REQUIRE(error == 0);

    BOOST_CHECK_EQUAL(35796., date);
}

BOOST_FIXTURE_TEST_CASE(test_getTimes, Fixture) {
    int time = -1;

    error = SMO_getTimes(p_handle, SMO_reportStep, &time);
    BOOST_REQUIRE(error == 0);

    BOOST_CHECK_EQUAL(3600, time);

    error = SMO_getTimes(p_handle, SMO_numPeriods, &time);
    BOOST_REQUIRE(error == 0);

    BOOST_CHECK_EQUAL(36, time);
}

BOOST_FIXTURE_TEST_CASE(test_getElementName, Fixture) {
    char* c_array = NULL;
    int   index   = 1;

    error = SMO_getElementName(p_handle, SMO_node, index, &c_array, &array_dim);
    BOOST_REQUIRE(error == 0);

    std::string test(c_array);
    std::string ref("10");
    BOOST_CHECK(check_string(test, ref));

    SMO_freeMemory((void*)c_array);
}



BOOST_FIXTURE_TEST_CASE(test_getDateTime, Fixture) {
    // Gather meta
    double startDate = -1.0;
    int reportStepSec = -1, nperiods = -1;

    error = SMO_getStartDate(p_handle, &startDate);
    BOOST_REQUIRE(error == 0);

    error = SMO_getTimes(p_handle, SMO_reportStep, &reportStepSec);
    BOOST_REQUIRE(error == 0);
    BOOST_REQUIRE(reportStepSec > 0);

    error = SMO_getTimes(p_handle, SMO_numPeriods, &nperiods);
    BOOST_REQUIRE(error == 0);
    BOOST_REQUIRE(nperiods > 0);

    const double stepDays = reportStepSec / 86400.0;

    // First
    double dt = -1.0;
    int period = 0;
    error = SMO_getDateTime(p_handle, period, &dt);
    BOOST_REQUIRE(error == 0);
    // Note: even though the period index is zero the 
    // first reporting period is at StartDate + 1 step
    BOOST_CHECK_CLOSE(dt, startDate + (period + 1) * stepDays, 1e-6);

    // Middle
    period = nperiods / 2;
    error = SMO_getDateTime(p_handle, period, &dt);
    BOOST_REQUIRE(error == 0);
    BOOST_CHECK_CLOSE(dt, startDate + (period + 1) * stepDays, 1e-6);

    // Last
    period = nperiods - 1;
    error = SMO_getDateTime(p_handle, period, &dt);
    BOOST_REQUIRE(error == 0);
    BOOST_CHECK_CLOSE(dt, startDate + (period + 1) * stepDays, 1e-6);

    // Out-of-range
    dt = 42.0;
    error = SMO_getDateTime(p_handle, nperiods, &dt);
    BOOST_CHECK_EQUAL(error, 422);
    BOOST_CHECK_EQUAL(dt, -1.0);
}

BOOST_FIXTURE_TEST_CASE(test_getDateSeries, Fixture) {
    // Gather meta
    double startDate = -1.0;
    int reportStepSec = -1, nperiods = -1;

    error = SMO_getStartDate(p_handle, &startDate);
    BOOST_REQUIRE(error == 0);

    error = SMO_getTimes(p_handle, SMO_reportStep, &reportStepSec);
    BOOST_REQUIRE(error == 0);
    BOOST_REQUIRE(reportStepSec > 0);

    error = SMO_getTimes(p_handle, SMO_numPeriods, &nperiods);
    BOOST_REQUIRE(error == 0);
    BOOST_REQUIRE(nperiods > 0);

    const double stepDays = reportStepSec / 86400.0;

    // Full range [0, nperiods-1]
    double* dates = NULL;
    int len = 0;
    error = SMO_getDateSeries(p_handle, 0, nperiods - 1, &dates, &len);
    BOOST_REQUIRE(error == 0);
    BOOST_REQUIRE(dates != NULL);
    BOOST_CHECK_EQUAL(len, nperiods);

    // First equals StartDate
    int period = 0;
    BOOST_CHECK_CLOSE(dates[0], startDate + (period + 1) * stepDays, 1e-6);

    // Monotonic, constant step
    for (int i = 1; i < len; ++i) {
        BOOST_CHECK_CLOSE(dates[i] - dates[i - 1], stepDays, 1e-6);
        // Cross-check SMO_getDateTime
        double dt = -1.0;
        int rc = SMO_getDateTime(p_handle, i, &dt);
        BOOST_REQUIRE(rc == 0);
        BOOST_CHECK_CLOSE(dates[i], dt, 1e-6);
    }

    SMO_freeMemory((void*)dates);

    // Invalid range
    dates = NULL; len = 123;
    error = SMO_getDateSeries(p_handle, 2, 1, &dates, &len);
    BOOST_CHECK_EQUAL(error, 422);
    BOOST_CHECK(dates == NULL);
    BOOST_CHECK_EQUAL(len, 0);
}

BOOST_FIXTURE_TEST_CASE(test_decodeDate, Fixture) {
    // Meta
    double startDate = -1.0;
    int reportStepSec = -1, nperiods = -1;

    error = SMO_getStartDate(p_handle, &startDate);
    BOOST_REQUIRE(error == 0);

    error = SMO_getTimes(p_handle, SMO_reportStep, &reportStepSec);
    BOOST_REQUIRE(error == 0);
    BOOST_REQUIRE(reportStepSec == 3600); // existing fixture expects 1-hour steps

    error = SMO_getTimes(p_handle, SMO_numPeriods, &nperiods);
    BOOST_REQUIRE(error == 0);
    BOOST_REQUIRE(nperiods == 36);

    // Decode StartDate (known: 35796.0 -> 1998-01-01 00:00:00)
    int y = -1, m = -1, d = -1, hh = -1, mm = -1, ss = -1, dow = -1;
    SMO_decodeDate(startDate, &y, &m, &d, &hh, &mm, &ss, &dow);
    BOOST_CHECK_EQUAL(y, 1998);
    BOOST_CHECK_EQUAL(m, 1);
    BOOST_CHECK_EQUAL(d, 1);
    BOOST_CHECK_EQUAL(hh, 0);
    BOOST_CHECK_EQUAL(mm, 0);
    BOOST_CHECK_EQUAL(ss, 0);
    BOOST_CHECK(dow >= 0 && dow <= 6);

    // First reporting period (startDate + 1 hour)
    double dt = -1.0;
    error = SMO_getDateTime(p_handle, 0, &dt);
    BOOST_REQUIRE(error == 0);
    SMO_decodeDate(dt, &y, &m, &d, &hh, &mm, &ss, &dow);
    BOOST_CHECK_EQUAL(y, 1998);
    BOOST_CHECK_EQUAL(m, 1);
    BOOST_CHECK_EQUAL(d, 1);
    BOOST_CHECK_EQUAL(hh, 1);
    BOOST_CHECK_EQUAL(mm, 0);
    BOOST_CHECK_EQUAL(ss, 0);

    // Last reporting period (36th hour -> 1998-01-02 12:00:00)
    error = SMO_getDateTime(p_handle, nperiods - 1, &dt);
    BOOST_REQUIRE(error == 0);
    SMO_decodeDate(dt, &y, &m, &d, &hh, &mm, &ss, &dow);
    BOOST_CHECK_EQUAL(y, 1998);
    BOOST_CHECK_EQUAL(m, 1);
    BOOST_CHECK_EQUAL(d, 2);
    BOOST_CHECK_EQUAL(hh, 12);
    BOOST_CHECK_EQUAL(mm, 0);
    BOOST_CHECK_EQUAL(ss, 0);
}



BOOST_FIXTURE_TEST_CASE(test_getSubcatchSeries, Fixture) {
    error = SMO_getSubcatchSeries(p_handle, 1, SMO_runoff_rate, 0, 10, &array,
                                  &array_dim);
    BOOST_REQUIRE(error == 0);

    const int ref_dim            = 11;
    float     ref_array[ref_dim] = {
        0.0f, 1.2438242f, 2.5639679f, 4.524055f, 2.5115132f, 0.69808137f,
		0.040894926f, 0.011605669f, 0.00509294f, 0.0027438672f, 0.00167188f};

    std::vector<float> ref_vec;
    ref_vec.assign(ref_array, ref_array + ref_dim);

    std::vector<float> test_vec;
    test_vec.assign(array, array + array_dim);

    BOOST_CHECK(check_cdd_float(test_vec, ref_vec, 3));
}

BOOST_FIXTURE_TEST_CASE(test_getSystemSeries, Fixture) {
    error = SMO_getSystemSeries(p_handle, SMO_runoff_flow, 0, 10, &array,
                                  &array_dim);
    BOOST_REQUIRE(error == 0);

    const int ref_dim            = 11;
    float     ref_array[ref_dim] = {
        0.0f, 6.216825f, 13.030855f, 24.252975f, 14.172027f, 4.1949716f,
		0.322329f, 0.056010f, 0.024938f, 0.012474f, 0.00766089f};

    std::vector<float> ref_vec;
    ref_vec.assign(ref_array, ref_array + ref_dim);

    std::vector<float> test_vec;
    test_vec.assign(array, array + array_dim);

    BOOST_CHECK(check_cdd_float(test_vec, ref_vec, 3));
}


BOOST_FIXTURE_TEST_CASE(test_getSubcatchResult, Fixture) {
    error = SMO_getSubcatchResult(p_handle, 1, 1, &array, &array_dim);
    BOOST_REQUIRE(error == 0);

    const int ref_dim            = 10;
    float     ref_array[ref_dim] = {
		0.5f, 0.0f, 0.0f, 0.125f, 1.2438242f,
        0.0f, 0.0f, 0.0f, 33.481991f, 6.6963983f};

    std::vector<float> ref_vec;
    ref_vec.assign(ref_array, ref_array + ref_dim);

    std::vector<float> test_vec;
    test_vec.assign(array, array + array_dim);

    BOOST_CHECK(check_cdd_float(test_vec, ref_vec, 3));
}

BOOST_FIXTURE_TEST_CASE(test_getNodeResult, Fixture) {
    error = SMO_getNodeResult(p_handle, 2, 2, &array, &array_dim);
    BOOST_REQUIRE(error == 0);

    const int ref_dim        = 8;
    float ref_array[ref_dim] = {
		0.296234f, 995.296204f, 0.0f, 1.302650f, 1.302650f, 0.0f,
		15.361463f, 3.072293f};

    std::vector<float> ref_vec;
    ref_vec.assign(ref_array, ref_array + ref_dim);

    std::vector<float> test_vec;
    test_vec.assign(array, array + array_dim);

    BOOST_CHECK(check_cdd_float(test_vec, ref_vec, 3));
}

BOOST_FIXTURE_TEST_CASE(test_getLinkResult, Fixture) {
    error = SMO_getLinkResult(p_handle, 3, 3, &array, &array_dim);
    BOOST_REQUIRE(error == 0);

    const int ref_dim        = 7;
    float ref_array[ref_dim] = {
		4.631762f, 1.0f, 5.8973422f, 314.15927f, 1.0f, 19.070757f,
		3.8141515f};

    std::vector<float> ref_vec;
    ref_vec.assign(ref_array, ref_array + ref_dim);

    std::vector<float> test_vec;
    test_vec.assign(array, array + array_dim);

    BOOST_CHECK(check_cdd_float(test_vec, ref_vec, 3));
}

BOOST_FIXTURE_TEST_CASE(test_getSystemResult, Fixture) {
    error = SMO_getSystemResult(p_handle, 4, 4, &array, &array_dim);
    BOOST_REQUIRE(error == 0);

    const int ref_dim            = 14;
    float     ref_array[ref_dim] = {
        70.0f, 0.1f, 0.0f, 0.19042271f, 14.172027f, 0.0f, 0.0f, 0.0f,
		0.0f, 14.172027f, 0.55517411f, 13.622702f, 2913.0793f, 0.0f};

    std::vector<float> ref_vec;
    ref_vec.assign(ref_array, ref_array + ref_dim);

    std::vector<float> test_vec;
    test_vec.assign(array, array + array_dim);

    BOOST_CHECK(check_cdd_float(test_vec, ref_vec, 3));
}



BOOST_FIXTURE_TEST_CASE(test_decodeDate_components_and_step, Fixture) {
    // Meta
    double startDate = -1.0;
    int reportStepSec = -1, nperiods = -1;

    error = SMO_getStartDate(p_handle, &startDate);
    BOOST_REQUIRE(error == 0);

    error = SMO_getTimes(p_handle, SMO_reportStep, &reportStepSec);
    BOOST_REQUIRE(error == 0);
    BOOST_REQUIRE(reportStepSec > 0);

    error = SMO_getTimes(p_handle, SMO_numPeriods, &nperiods);
    BOOST_REQUIRE(error == 0);
    BOOST_REQUIRE(nperiods > 1);

    const double stepDays = reportStepSec / 86400.0;

    // Decode first two timestamps
    double dt0 = -1.0, dt1 = -1.0;
    error = SMO_getDateTime(p_handle, 0, &dt0);
    BOOST_REQUIRE(error == 0);
    error = SMO_getDateTime(p_handle, 1, &dt1);
    BOOST_REQUIRE(error == 0);

    int y, m, d, hh, mm, ss, dow;

    // First period components are within valid ranges
    SMO_decodeDate(dt0, &y, &m, &d, &hh, &mm, &ss, &dow);
    BOOST_CHECK(y >= 1900 && y <= 2200);
    BOOST_CHECK(m >= 1 && m <= 12);
    BOOST_CHECK(d >= 1 && d <= 31);
    BOOST_CHECK(hh >= 0 && hh <= 23);
    BOOST_CHECK(mm >= 0 && mm <= 59);
    BOOST_CHECK(ss >= 0 && ss <= 59);
    BOOST_CHECK(dow >= 1 && dow <= 7);

    // Step consistency between adjacent periods
    BOOST_CHECK_CLOSE(dt1 - dt0, stepDays, 1e-8);

    // First period should be at or after StartDate
    BOOST_CHECK(dt0 >= startDate - 1e-12);
}

BOOST_FIXTURE_TEST_CASE(test_decodeDate_day_of_week_progression, Fixture) {
    // Meta
    int reportStepSec = -1, nperiods = -1;
    error = SMO_getTimes(p_handle, SMO_reportStep, &reportStepSec);
    BOOST_REQUIRE(error == 0);
    error = SMO_getTimes(p_handle, SMO_numPeriods, &nperiods);
    BOOST_REQUIRE(error == 0);

    // Only check DOW if the step divides a day evenly and we have > 1 full day
    if (86400 % reportStepSec != 0 || nperiods <= (86400 / reportStepSec)) {
        BOOST_TEST_MESSAGE("Skipping DOW progression: report step does not divide a day or not enough periods.");
        return;
    }

    const int periodsPerDay = 86400 / reportStepSec;

    double dt0 = -1.0, dtDayLater = -1.0;
    error = SMO_getDateTime(p_handle, 0, &dt0);
    BOOST_REQUIRE(error == 0);
    error = SMO_getDateTime(p_handle, periodsPerDay, &dtDayLater);
    BOOST_REQUIRE(error == 0);

    int y0, m0, d0, h0, min0, s0, dow0;
    int y1, m1, d1, h1, min1, s1, dow1;

    SMO_decodeDate(dt0, &y0, &m0, &d0, &h0, &min0, &s0, &dow0);
    SMO_decodeDate(dtDayLater, &y1, &m1, &d1, &h1, &min1, &s1, &dow1);

    // Day-of-week should advance by 1 modulo 7 after exactly one day
    BOOST_CHECK_EQUAL(((dow0 + 1) % 7), dow1);

    // Time-of-day should be identical after exactly one day
    BOOST_CHECK_EQUAL(h0, h1);
    BOOST_CHECK_EQUAL(min0, min1);
    BOOST_CHECK_EQUAL(s0, s1);
}



BOOST_AUTO_TEST_SUITE_END()
