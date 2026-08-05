/*
 ******************************************************************************
 Project:      OWA SWMM
 Version:      5.2
 Module:       test_live_results.cpp
 Description:  tests that compare live toolkit results to saved output
 Authors:      see AUTHORS
 Copyright:    see AUTHORS
 License:      see LICENSE
 Last Updated: 03/16/2026
 ******************************************************************************
 */

#define BOOST_TEST_MODULE "live_results"
#include <boost/test/included/unit_test.hpp>

#include <vector>

#include "test_predicates.hpp"

extern "C" {
#include "swmm5.h"
#include "toolkit.h"
#include "swmm_output.h"
}

// Add shared data paths here
#define DATA_PATH_INP "test_example1.inp"
#define DATA_PATH_RPT "tmp.rpt"
#define DATA_PATH_OUT "tmp.out"

#define ERR_NONE 0
#define ERR_TKAPI_REPORT_UNAVAILABLE 2014



// Fixture dedicated to exercising live result APIs while persisting output
struct FixtureBeforeStep_SaveResults {
    FixtureBeforeStep_SaveResults() {
        swmm_open(DATA_PATH_INP, DATA_PATH_RPT, DATA_PATH_OUT);
        swmm_start(true);
    }

    ~FixtureBeforeStep_SaveResults() {
        swmm_close();
    }
};

BOOST_AUTO_TEST_SUITE(test_live_results)

// Verifies that live interpolation matches binary output across the full run
BOOST_FIXTURE_TEST_CASE(report_results_match_saved_values, FixtureBeforeStep_SaveResults) {
    int error;
    int subc_ind, nde_ind, lnk_ind;
    double val_node = 0.0, val_link = 0.0, val_subc = 0.0;
    double elapsedTime = 0.0;
    std::vector<double> nodeLive;
    std::vector<double> linkLive;
    std::vector<double> subcLive;
    std::vector<double> reportTimes;

    char subid[] = "1";
    char ndeid[] = "19";
    char lnkid[] = "14";

    error = swmm_getObjectIndex(SM_SUBCATCH, subid, &subc_ind);
    BOOST_REQUIRE(error == ERR_NONE);

    error = swmm_getObjectIndex(SM_NODE, ndeid, &nde_ind);
    BOOST_REQUIRE(error == ERR_NONE);

    error = swmm_getObjectIndex(SM_LINK, lnkid, &lnk_ind);
    BOOST_REQUIRE(error == ERR_NONE);

    // Expect no cached report before the first step
    bool readyFlag = false;
    double lastReady = -1.0;
    double nextReport = 0.0;
    error = swmm_getReportWindow(&readyFlag, &lastReady, &nextReport);
    BOOST_REQUIRE(error == ERR_NONE);
    BOOST_CHECK_EQUAL(readyFlag, 0);

    do
    {
        // Advance simulation until a report snapshot is available
        error = swmm_step(&elapsedTime);
        BOOST_REQUIRE(error == ERR_NONE);

        bool isReady = false;
        double readyDateTime = -1.0;
        double upcomingDateTime = 0.0;

        error = swmm_getReportWindow(&isReady, &readyDateTime, &upcomingDateTime);
        BOOST_REQUIRE(error == ERR_NONE);

        if (isReady) {
            error = swmm_getNodeLiveReport(nde_ind, SM_NODEDEPTH, &val_node);
            BOOST_REQUIRE(error == ERR_NONE);

            error = swmm_getLinkLiveReport(lnk_ind, SM_LINKFLOW, &val_link);
            BOOST_REQUIRE(error == ERR_NONE);

            error = swmm_getSubcatchLiveReport(subc_ind, SM_SUBCRAIN, &val_subc);
            BOOST_REQUIRE(error == ERR_NONE);

            // Store the live values for comparison once the run completes
            reportTimes.push_back(readyDateTime);
            nodeLive.push_back(val_node);
            linkLive.push_back(val_link);
            subcLive.push_back(val_subc);
        }
    } while (elapsedTime != 0.0);

    BOOST_REQUIRE(!nodeLive.empty());
    BOOST_REQUIRE_EQUAL(nodeLive.size(), linkLive.size());
    BOOST_REQUIRE_EQUAL(nodeLive.size(), subcLive.size());
    BOOST_REQUIRE_EQUAL(nodeLive.size(), reportTimes.size());
    const int reportCount = static_cast<int>(nodeLive.size());
    swmm_end();
    swmm_report();   // finalize tmp.out so the output API can read it
    swmm_close();

    SMO_Handle outputHandle = NULL;
    int outputError = SMO_init(&outputHandle);
    BOOST_REQUIRE(outputError == 0);
    outputError = SMO_open(outputHandle, DATA_PATH_OUT);
    BOOST_REQUIRE_MESSAGE(outputError == 0, "SMO_open failed with error code " << outputError);

    float *nodeSeries = NULL;
    float *linkSeries = NULL;
    float *subcSeries = NULL;
    int nodeLen = 0;
    int linkLen = 0;
    int subcLen = 0;
    const int lastPeriod = reportCount - 1;

    outputError = SMO_getNodeSeries(outputHandle, nde_ind, SMO_invert_depth,
                                    0, lastPeriod, &nodeSeries, &nodeLen);
    BOOST_REQUIRE(outputError == 0);
    outputError = SMO_getLinkSeries(outputHandle, lnk_ind, SMO_flow_rate_link,
                                    0, lastPeriod, &linkSeries, &linkLen);
    BOOST_REQUIRE(outputError == 0);
    outputError = SMO_getSubcatchSeries(outputHandle, subc_ind, SMO_rainfall_subcatch,
                                        0, lastPeriod, &subcSeries, &subcLen);
    BOOST_REQUIRE(outputError == 0);

    BOOST_REQUIRE_EQUAL(nodeLen, reportCount);
    BOOST_REQUIRE_EQUAL(linkLen, reportCount);
    BOOST_REQUIRE_EQUAL(subcLen, reportCount);

    std::vector<double> nodeSaved(nodeSeries, nodeSeries + nodeLen);
    std::vector<double> linkSaved(linkSeries, linkSeries + linkLen);
    std::vector<double> subcSaved(subcSeries, subcSeries + subcLen);

    const long cddTolerance = 10;
    BOOST_CHECK(check_cdd_double(nodeLive, nodeSaved, cddTolerance));
    BOOST_CHECK(check_cdd_double(linkLive, linkSaved, cddTolerance));
    BOOST_CHECK(check_cdd_double(subcLive, subcSaved, cddTolerance));

    SMO_freeMemory(nodeSeries);
    SMO_freeMemory(linkSeries);
    SMO_freeMemory(subcSeries);
    SMO_close(outputHandle);
}

BOOST_AUTO_TEST_SUITE_END()
