/*
 *   test_toolkitAPI_coupling.cpp
 *
 *   Created: 11/06/2020
 *   Author: Velitchko G. Tzatchkov
 *           Laurent Courty
 *           IMTA, Mexico
 *
 *   Unit testing for SWMM-ToolkitAPI with coupling functions using Boost Test.
 *   
 *   Based on:
 *   "test_toolkitAPI.cpp
 *   Created: 03/10/2018
 *   Author: Bryant E. McDonnell
 *           EmNet LLC
 *   Unit testing for SWMM-ToolkitAPI using Boost Test."


* TABLE OF CONTENTS AND STRUCTURE

  ->  Numerical Diff Functions
  ->  Non-Fixuture Unit Tests
  ->  Unit tests using the fixtures
      - Error Test Checks
      - Parameter Get / Set Tests
         1. Subcatchments
         2. Nodes
         3. Links
      - Result Get Tests
 */

// NOTE: Travis installs libboost test version 1.5.4
//#define BOOST_TEST_DYN_LINK

#define BOOST_TEST_MODULE "toolkitAPI_coupling"
#include "test_toolkitapi_coupling.hpp"

#define ERR_NONE 0
#define ERR_API_OUTBOUNDS 501
#define ERR_API_INPUTNOTOPEN 502
#define ERR_API_SIM_NRUNNING 503
#define ERR_API_WRONG_TYPE 504
#define ERR_API_OBJECT_INDEX 505

using namespace std;

// Custom test to check the minimum number of correct decimal digits between
// the test and the ref vectors.
boost::test_tools::predicate_result check_cdd(std::vector<double>& test,
    std::vector<double>& ref, long cdd_tol)
{
    double tmp, min_cdd = 10.0;

    // TODO: What if the vectors aren't the same length?

    std::vector<double>::iterator test_it;
    std::vector<double>::iterator ref_it;

    for (test_it = test.begin(); test_it < test.end(); ++test_it) {
        for (ref_it = ref.begin(); ref_it < ref.end(); ++ref_it) {

             if (*test_it != *ref_it) {
                // Compute log absolute error
                tmp = abs(*test_it - *ref_it);
                if (tmp < 1.0e-7)
                    tmp = 1.0e-7;

                else if (tmp > 2.0)
                    tmp = 1.0;

                tmp = - log10(tmp);
                if (tmp < 0.0)
                    tmp = 0.0;

                if (tmp < min_cdd)
                    min_cdd = tmp;
            }
        }
    }

    return floor(min_cdd) <= cdd_tol;
}

// Non-Fixuture Unit Tests
BOOST_AUTO_TEST_SUITE(test_toolkitapi_coupling)

// Test Model Not Open
BOOST_AUTO_TEST_CASE(model_not_open) {
    int error, index, num, type;
    double val;
    double input_val = 0;
    double *result_array;
    char id[] = "test";
    int arr;
    int iscoupled;

    //Project
    error = swmm_getObjectIndex(SM_NODE, id, &index);
    BOOST_CHECK_EQUAL(error, ERR_API_INPUTNOTOPEN);

    //Coupling
    error = swmm_setNodeOpening(0, 0, 0, 0.0, 0.0, 0.0, 0.0, 0.0);
    BOOST_CHECK_EQUAL(error, ERR_API_INPUTNOTOPEN);

    error = swmm_deleteNodeOpening(0, 0);
    BOOST_CHECK_EQUAL(error, ERR_API_INPUTNOTOPEN);

    error = swmm_getNodeOpeningParam(0, 0, 0, &val);
    BOOST_CHECK_EQUAL(error, ERR_API_INPUTNOTOPEN);

    error = swmm_getNodeOpeningFlow(0, 0, &val);
    BOOST_CHECK_EQUAL(error, ERR_API_INPUTNOTOPEN);

    error = swmm_getNodeOpeningType(0, 0, &type);
    BOOST_CHECK_EQUAL(error, ERR_API_INPUTNOTOPEN); 

    error = swmm_getOpeningCouplingType(0, 0, &type);
    BOOST_CHECK_EQUAL(error, ERR_API_INPUTNOTOPEN); 

    error = swmm_getOpeningsNum(0, &num);
    BOOST_CHECK_EQUAL(error, ERR_API_INPUTNOTOPEN); 

    error = swmm_getOpeningsIndices(0, 0, &arr);
    BOOST_CHECK_EQUAL(error, ERR_API_INPUTNOTOPEN); 

    error = swmm_getNodeIsCoupled(0, &iscoupled);
    BOOST_CHECK_EQUAL(error, ERR_API_INPUTNOTOPEN); 

    error = swmm_closeOpening(0, 0);
    BOOST_CHECK_EQUAL(error, ERR_API_INPUTNOTOPEN); 

    error = swmm_openOpening(0, 0);
    BOOST_CHECK_EQUAL(error, ERR_API_INPUTNOTOPEN); 
}

BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_SUITE(test_toolkitapi_coupling_fixture)

// Testing for Simulation Started Error
BOOST_FIXTURE_TEST_CASE(sim_started_check, FixtureBeforeStep) {
    int error;
    error = swmm_setNodeOpening(0, 1, 1, 0.0, 0.0, 0.0, 0.0, 0.0);
    BOOST_CHECK_EQUAL(error, ERR_API_SIM_NRUNNING);
}


// Testing for invalid object index
BOOST_FIXTURE_TEST_CASE(object_bounds_check, FixtureOpenClose) {
    int error, index, num, type;
    double val;
    double input_val = 0;
    double *result_array;
    int arr;
    int iscoupled;

    error = swmm_setNodeOpening(100, 0, 0, 0.0, 0.0, 0.0, 0.0, 0.0);
    BOOST_CHECK_EQUAL(error, ERR_API_OBJECT_INDEX);

    error = swmm_getNodeOpeningParam(100, 0, 0, &val);
    BOOST_CHECK_EQUAL(error, ERR_API_OBJECT_INDEX);

    error = swmm_getNodeOpeningFlow(100, 0, &val);
    BOOST_CHECK_EQUAL(error, ERR_API_OBJECT_INDEX);

    error = swmm_getNodeOpeningType(100, 0, &type);
    BOOST_CHECK_EQUAL(error, ERR_API_OBJECT_INDEX);

    error = swmm_getOpeningCouplingType(100, 0, &type);
    BOOST_CHECK_EQUAL(error, ERR_API_OBJECT_INDEX);

    error = swmm_getOpeningsNum(100, &num);
    BOOST_CHECK_EQUAL(error, ERR_API_OBJECT_INDEX);

    error = swmm_getOpeningsIndices(100, 0, &arr);
    BOOST_CHECK_EQUAL(error, ERR_API_OBJECT_INDEX);

    error = swmm_getNodeIsCoupled(100, &iscoupled);
    BOOST_CHECK_EQUAL(error, ERR_API_OBJECT_INDEX);

    error = swmm_closeOpening(100, 0);
    BOOST_CHECK_EQUAL(error, ERR_API_OBJECT_INDEX);

    error = swmm_openOpening(100, 0);
    BOOST_CHECK_EQUAL(error, ERR_API_OBJECT_INDEX);
}


// Testing for invalid parameter key
BOOST_FIXTURE_TEST_CASE(key_bounds_check, FixtureOpenClose) {
    int error, index, num, type;
    double val;
    char* error_msg=new char[256];
    int arr;
    int iscoupled;

    //Error codes
    swmm_getAPIError(341, error_msg);
    BOOST_CHECK_EQUAL(error_msg, "\n  ERROR 341: cannot open scratch RDII interface file.");
    delete[] error_msg;

    error = swmm_getNodeOpeningParam(0, 100, 0, &val);
    BOOST_CHECK_EQUAL(error, ERR_API_OBJECT_INDEX);

    error = swmm_getNodeOpeningType(0, 100, &type);
    BOOST_CHECK_EQUAL(error, ERR_API_OBJECT_INDEX);

    error = swmm_getOpeningCouplingType(0, 100, &type);
    BOOST_CHECK_EQUAL(error, ERR_API_OBJECT_INDEX);

    error = swmm_getOpeningsIndices(0, 100, &arr);
    BOOST_CHECK_EQUAL(error, ERR_API_OBJECT_INDEX);
}

// Testing for node get/set
BOOST_FIXTURE_TEST_CASE(getset_node, FixtureOpenClose) {

    int error, node_ind;
    double val;
    char id[] = "O7";

    // Get/Set Node SM_SURFAREA (Coupling)
    error = swmm_getNodeParam(node_ind, SM_SURFAREA, &val); //Coupling
    BOOST_REQUIRE(error == ERR_NONE);  //Coupling
    BOOST_CHECK_SMALL(val - 0, 0.0001);  //Coupling

    error = swmm_setNodeParam(node_ind, SM_SURFAREA, 1);  //Coupling
    BOOST_REQUIRE(error == ERR_NONE);  //Coupling

    error = swmm_getNodeParam(node_ind, SM_SURFAREA, &val);  //Coupling
    BOOST_REQUIRE(error == ERR_NONE);  //Coupling

    // Get/Set Node SM_COUPAREA (Coupling)
    error = swmm_getNodeParam(node_ind, SM_COUPAREA, &val); //Coupling
    BOOST_REQUIRE(error == ERR_NONE);  //Coupling
    BOOST_CHECK_SMALL(val - 0, 0.0001);  //Coupling

    error = swmm_setNodeParam(node_ind, SM_COUPAREA, 1);  //Coupling
    BOOST_REQUIRE(error == ERR_NONE);  //Coupling

    error = swmm_getNodeParam(node_ind, SM_COUPAREA, &val);  //Coupling
    BOOST_REQUIRE(error == ERR_NONE);  //Coupling
    BOOST_CHECK_SMALL(val - 1, 0.0001);  //Coupling

    // Get/Set Node SM_OVERLANDDEPTH (Coupling)
    error = swmm_getNodeParam(node_ind, SM_OVERLANDDEPTH, &val); //Coupling
    BOOST_REQUIRE(error == ERR_NONE);  //Coupling
    BOOST_CHECK_SMALL(val - 0, 0.0001);  //Coupling

    error = swmm_setNodeParam(node_ind, SM_SURFAREA, 1);  //Coupling
    BOOST_REQUIRE(error == ERR_NONE);  //Coupling

    error = swmm_getNodeParam(node_ind, SM_SURFAREA, &val);  //Coupling
    BOOST_REQUIRE(error == ERR_NONE);  //Coupling
}

// Testing Results Getters (During Simulation)
BOOST_FIXTURE_TEST_CASE(get_result_during_sim, FixtureBeforeStep){
    int error, step_ind;
    int subc_ind, nde_ind, lnk_ind;
    double val;
    double elapsedTime = 0.0;

    char subid[] = "1";
    char ndeid[] = "J1";
    char lnkid[] = "C6";

    error = swmm_getObjectIndex(SM_SUBCATCH, subid, &subc_ind);
    BOOST_REQUIRE(error == ERR_NONE);

    error = swmm_getObjectIndex(SM_NODE, ndeid, &nde_ind);
    BOOST_REQUIRE(error == ERR_NONE);

    error = swmm_getObjectIndex(SM_LINK, lnkid, &lnk_ind);
    BOOST_REQUIRE(error == ERR_NONE);

    step_ind = 0;

    do
    {
        error = swmm_step(&elapsedTime);
        BOOST_REQUIRE(error == ERR_NONE);

        error = swmm_getNodeResult(nde_ind, SM_TOTALINFLOW, &val);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getNodeResult(nde_ind, SM_LOSSES, &val);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getNodeResult(nde_ind, SM_NODEFLOOD, &val);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getNodeResult(nde_ind, SM_LATINFLOW, &val);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getNodeResult(nde_ind, SM_COUPINFLOW, &val);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getNodeResult(nde_ind, SM_NODEVOL, &val);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getNodeResult(nde_ind, SM_NODEFLOOD, &val);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getNodeResult(nde_ind, SM_NODEDEPTH, &val);
        BOOST_REQUIRE(error == ERR_NONE);
        error = swmm_getNodeResult(nde_ind, SM_NODEHEAD, &val);
        BOOST_REQUIRE(error == ERR_NONE);
        step_ind+=1;
    }while (elapsedTime != 0 && !error);
    BOOST_REQUIRE(error == ERR_NONE);
    swmm_end();
}

BOOST_AUTO_TEST_SUITE_END()
