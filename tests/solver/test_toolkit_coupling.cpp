/*
 ******************************************************************************
 Project:      OWA SWMM
 Version:      5.2.4
 Module:       test_toolkit_coupling.cpp
 Description:  tests for SWMM toolkit coupling API functions
 Authors:      see AUTHORS
 Copyright:    see AUTHORS
 License:      see LICENSE
 Last Updated: 2025-03-15
 ******************************************************************************
 */

// #define BOOST_TEST_MODULE "toolkit_coupling"
#include "test_toolkit_coupling.hpp"

#define ERR_NONE 0
#define ERR_TKAPI_OUTBOUNDS 2000
#define ERR_TKAPI_INPUTNOTOPEN 2001
#define ERR_TKAPI_SIM_NRUNNING 2002
#define ERR_TKAPI_WRONG_TYPE 2003
#define ERR_TKAPI_OBJECT_INDEX 2004
#define ERR_TKAPI_SIM_RUNNING 2013

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
BOOST_AUTO_TEST_SUITE(test_toolkit_coupling)

// Test Model Not Open
BOOST_AUTO_TEST_CASE(model_not_open) {
    int error, index, num, type;
    double val;
    double input_val = 0;
    char id[] = "test";
    int arr;
    int iscoupled;

    //Project
    error = swmm_getObjectIndex(SM_NODE, id, &index);
    BOOST_CHECK_EQUAL(error, ERR_TKAPI_INPUTNOTOPEN);

    //Coupling
    error = swmm_setNodeOpening(0, 0, 0, 0.0, 0.0, 0.0, 0.0, 0.0);
    BOOST_CHECK_EQUAL(error, ERR_TKAPI_INPUTNOTOPEN);

    error = swmm_deleteNodeOpening(0, 0);
    BOOST_CHECK_EQUAL(error, ERR_TKAPI_INPUTNOTOPEN);

    error = swmm_getNodeOpeningParam(0, 0, 0, &val);
    BOOST_CHECK_EQUAL(error, ERR_TKAPI_INPUTNOTOPEN);

    error = swmm_getNodeOpeningFlow(0, 0, &val);
    BOOST_CHECK_EQUAL(error, ERR_TKAPI_INPUTNOTOPEN);

    error = swmm_getNodeOpeningType(0, 0, &type);
    BOOST_CHECK_EQUAL(error, ERR_TKAPI_INPUTNOTOPEN); 

    error = swmm_getOpeningCouplingType(0, 0, &type);
    BOOST_CHECK_EQUAL(error, ERR_TKAPI_INPUTNOTOPEN); 

    error = swmm_getOpeningsNum(0, &num);
    BOOST_CHECK_EQUAL(error, ERR_TKAPI_INPUTNOTOPEN); 

    error = swmm_getOpeningsIndices(0, 0, &arr);
    BOOST_CHECK_EQUAL(error, ERR_TKAPI_INPUTNOTOPEN); 

    error = swmm_getNodeIsCoupled(0, &iscoupled);
    BOOST_CHECK_EQUAL(error, ERR_TKAPI_INPUTNOTOPEN); 

    error = swmm_closeOpening(0, 0);
    BOOST_CHECK_EQUAL(error, ERR_TKAPI_INPUTNOTOPEN); 

    error = swmm_openOpening(0, 0);
    BOOST_CHECK_EQUAL(error, ERR_TKAPI_INPUTNOTOPEN); 

    error = swmm_coupling_findNodeInflow(0.0, 0.0, 0.0, 0.0, 0.0, NULL, 0.0, NULL);
    BOOST_CHECK_EQUAL(error, ERR_TKAPI_INPUTNOTOPEN); 

    error = swmm_deleteNodeOpenings(0);
    BOOST_CHECK_EQUAL(error, ERR_TKAPI_INPUTNOTOPEN); 

}

BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_SUITE(test_toolkit_coupling_fixture)

// Testing for Simulation Started Error
BOOST_FIXTURE_TEST_CASE(sim_started_check, FixtureBeforeStart) {
    int error;
    error = swmm_setNodeOpening(0, 1, 1, 0.0, 0.0, 0.0, 0.0, 0.0);
    BOOST_CHECK_EQUAL(error, ERR_TKAPI_SIM_NRUNNING);
}


// Testing for invalid object index
BOOST_FIXTURE_TEST_CASE(object_bounds_check, FixtureBeforeEnd) {
    int error, num, type;
    double val;
    double input_val = 0;
    int arr;
    int iscoupled;

    error = swmm_setNodeOpening(100, 0, 0, 0.0, 0.0, 0.0, 0.0, 0.0);
    BOOST_CHECK_EQUAL(error, ERR_TKAPI_OBJECT_INDEX);

    error = swmm_getNodeOpeningParam(100, 0, 0, &val);
    BOOST_CHECK_EQUAL(error, ERR_TKAPI_OBJECT_INDEX);

    error = swmm_getNodeOpeningFlow(100, 0, &val);
    BOOST_CHECK_EQUAL(error, ERR_TKAPI_OBJECT_INDEX);

    error = swmm_getNodeOpeningType(100, 0, &type);
    BOOST_CHECK_EQUAL(error, ERR_TKAPI_OBJECT_INDEX);

    error = swmm_getOpeningCouplingType(100, 0, &type);
    BOOST_CHECK_EQUAL(error, ERR_TKAPI_OBJECT_INDEX);

    error = swmm_getOpeningsNum(100, &num);
    BOOST_CHECK_EQUAL(error, ERR_TKAPI_OBJECT_INDEX);

    error = swmm_getOpeningsIndices(100, 0, &arr);
    BOOST_CHECK_EQUAL(error, ERR_TKAPI_OBJECT_INDEX);

    error = swmm_getNodeIsCoupled(100, &iscoupled);
    BOOST_CHECK_EQUAL(error, ERR_TKAPI_OBJECT_INDEX);

    error = swmm_closeOpening(100, 0);
    BOOST_CHECK_EQUAL(error, ERR_TKAPI_OBJECT_INDEX);

    error = swmm_openOpening(100, 0);
    BOOST_CHECK_EQUAL(error, ERR_TKAPI_OBJECT_INDEX);

    error = swmm_deleteNodeOpenings(100);
    BOOST_CHECK_EQUAL(error, ERR_TKAPI_OBJECT_INDEX);

}

// Testing for invalid parameter key
BOOST_FIXTURE_TEST_CASE(key_bounds_check, FixtureOpenClose) {
    int error, type;
    double val;
    char* error_msg=new char[256];
    int arr;

    error = swmm_getNodeOpeningParam(0, 100, 0, &val);
    BOOST_CHECK_EQUAL(error, ERR_TKAPI_OBJECT_INDEX);

    error = swmm_getNodeOpeningType(0, 100, &type);
    BOOST_CHECK_EQUAL(error, ERR_TKAPI_OBJECT_INDEX);

    error = swmm_getOpeningCouplingType(0, 100, &type);
    BOOST_CHECK_EQUAL(error, ERR_TKAPI_OBJECT_INDEX);

    error = swmm_getOpeningsIndices(0, 100, &arr);
    BOOST_CHECK_EQUAL(error, ERR_TKAPI_OBJECT_INDEX);
}

// Testing for node get/set
BOOST_FIXTURE_TEST_CASE(getset_node, FixtureOpenClose) {

    int error;
    int node_ind = 1;
    double val;

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
