/*
 *   test_toolkitAPI.cpp
 *
 *   Created: 01/12/2020
 *   Author: SM Masud Rana
 *           University of Cincinnati, EmNet LLC
 *
 *   Unit testing of the rdii components of SWMM-ToolkitAPI using Boost Test.
 */

// NOTE: Travis installs libboost test version 1.5.4
//#define BOOST_TEST_DYN_LINK

#define BOOST_TEST_MODULE "toolkitAPI_rdii"
#include "test_toolkitapi_rdii.hpp"

#define ERR_NONE 0
#define ERR_API_OUTBOUNDS 501
using namespace std;

// Custom test to check the minimum number of correct decimal digits between
// the test and the ref vectors.
boost::test_tools::predicate_result compare_vectors(std::vector<double>& v1,
	std::vector<double>& v2, double tolerance)
{
	std::vector<double>::iterator test_it;
	std::vector<double>::iterator ref_it;
	int n = v1.size();

	for (int i = 0; i < n; i++) {
		BOOST_REQUIRE_CLOSE(v1[i], v2[i], tolerance);
	}
	return true;
}

BOOST_AUTO_TEST_SUITE(test_toolkitapi_rdii)

// Testing Results Getters (During Simulation)
BOOST_FIXTURE_TEST_CASE(get_link_flow, FixtureGetRTK){
	BOOST_REQUIRE(ERR_NONE == swmm_getRDIIParams(unit_hyd_index, SM_RTK_ALL, rtk));
	BOOST_CHECK_CLOSE(rtk[1], 0.1, 0.0001);
}

// Get RTK values from INP2 and apply it to INP1. Resulting
// flow values from a link should match, after the RTK values
// of the two INP files are the same.
BOOST_FIXTURE_TEST_CASE(set_rtk, FixtureMatchFlow) {
	int error, step_ind;
	double value;
	double elapsedTime = 0.0;
	long newHour, oldHour, theDay, theHour;
	step_ind = 0;
	std::vector<double> flow1;
	std::vector<double> flow2;
	int linx = 128; // Index of link "2690"

	// Get RTK information from INP2 and also store flow values of link "2690".
	BOOST_REQUIRE(ERR_NONE == swmm_open((char *)DATA_PATH_INP2, (char *)DATA_PATH_RPT2, (char *)DATA_PATH_OUT2));
	BOOST_REQUIRE(ERR_NONE == swmm_getRDIIParams(unit_hyd_index, SM_RTK_ALL, rtk));
	BOOST_REQUIRE(ERR_NONE == swmm_start(0)); 
	
	oldHour = -1;
	do {
		error = swmm_step(&elapsedTime);
		newHour = (long)(elapsedTime * 24.0);
		if (newHour > oldHour) {
			theDay = (long)elapsedTime;
			theHour = (long)((elapsedTime - floor(elapsedTime)) * 24.0);
			oldHour = newHour;
			error = swmm_getLinkResult(linx, SM_LINKFLOW, &value);
			flow2.push_back(value);
		}
	} while (elapsedTime != 0 && !error);
	BOOST_REQUIRE(error == ERR_NONE);
	//BOOST_TEST_MESSAGE("Size of flow2: " << flow2.size() << " \n");
	BOOST_REQUIRE(flow2.size() == 96);
	BOOST_REQUIRE(ERR_NONE == swmm_end());
	BOOST_REQUIRE(ERR_NONE == swmm_close());

	// Set RTK values of INP2 to INP1 and calculate flow values of link "2690"
	BOOST_REQUIRE(ERR_NONE == swmm_open((char *)DATA_PATH_INP, (char *)DATA_PATH_RPT, (char *)DATA_PATH_OUT));
	BOOST_REQUIRE(ERR_NONE == swmm_setRDIIParams(unit_hyd_index, SM_RTK_ALL, rtk));
	BOOST_REQUIRE(ERR_NONE == swmm_start(0));
	oldHour = -1;
	do {
		error = swmm_step(&elapsedTime);
		newHour = (long)(elapsedTime * 24.0);
		if (newHour > oldHour) {
			theDay = (long)elapsedTime;
			theHour = (long)((elapsedTime - floor(elapsedTime)) * 24.0);
			oldHour = newHour;
			error = swmm_getLinkResult(linx, SM_LINKFLOW, &value);
			flow1.push_back(value);
		}
	} while (elapsedTime != 0 && !error);
	BOOST_REQUIRE(error == ERR_NONE);
	BOOST_REQUIRE(flow1.size() == 96);
	BOOST_REQUIRE(ERR_NONE == swmm_end());
	BOOST_REQUIRE(ERR_NONE == swmm_close());

	// Compare the flow values from the two INPs
	compare_vectors(flow1, flow2, 0.0001);
}


BOOST_AUTO_TEST_SUITE_END()
