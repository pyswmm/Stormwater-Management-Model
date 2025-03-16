/*
 *   test_coupling.cpp
 *
 *   Created: 09/06/2020
 *   Authors: Velitchko G. Tzatchkov
 *            Laurent Courty
 *            IMTA, Mexico
 *
 *   Unit testing for SWMM coupling functions using Boost Test.
 */

#define BOOST_TEST_MODULE "coupling"
#include <boost/test/unit_test.hpp>
// #include <boost/test/data/test_case.hpp>
#include "test_toolkit_coupling.hpp"

#define GRAVITY 32.2           // accel. of gravity in US units
#define MAX_OBJ_TYPES 16
#define ERR_NONE 0
#define ERR_TKAPI_OBJECT_INDEX 2004
// #define ERR_MEMORY 101

TNode* Node = new TNode;
int couplingType;
double CouplingInflow;
double orificeCoeff = 0.167;
double freeWeirCoeff = 0.54;
double subWeirCoeff = 0.056;
double tStep = 2.0;
double Node_couplingArea= 1.0 / (.3048 * .3048);

// Test case               0     1     2     3     4     5     6     7     8     9    10
double crestElev[11] =    {2.0,  2.0,  2.0,  2.0,  2.0,  3.0,  2.0,  2.0,  2.0,  2.0, 2.0};
double nodeHead[11] =     {0.0,  2.0,  3.0,  4.0,  1.0,  2.0,  1.0,  3.0,  3.0,  3.0, 1.0};
double overlandHead[11] = {2.0,  2.0,  2.0,  3.0,  1.0,  1.0,  3.0,  4.0,  3.5,  5.0, 2.0};
double overflowArea[11] = {25.0, 25.0, 25.0, 25.0, 25.0, 25.0, 10.0, 10.0, 10.0, 10.0, 10.0};
double weirWidth[11] =    {1.0,  1.0,  1.0,  1.0,  1.0,  1.0,  5.0,  5.0,  5.0,  5.0, 5.0};
int ExpectedType[11] = {NO_COUPLING_FLOW, NO_COUPLING_FLOW, ORIFICE_COUPLING, ORIFICE_COUPLING,
                        NO_COUPLING_FLOW, NO_COUPLING_FLOW, FREE_WEIR_COUPLING, ORIFICE_COUPLING,
                        SUBMERGED_WEIR_COUPLING, ORIFICE_COUPLING, NO_COUPLING_FLOW};
double ExpectedQ[11] =    {0.0,  0.0,  -18.49294,  
                                            -18.49294,
                                                    0.0,  0.0,  
                                                                7.973,  
                                                                     7.39717,  
                                                                           1.31547,  
                                                                                  10.4611, 
                                                                                       0.0};
        
BOOST_AUTO_TEST_SUITE(test_coupling)

    BOOST_DATA_TEST_CASE(ParameterizedCouplingInflowTest, boost::unit_test::data::xrange(0, 11), var)
    {

        // Unit conversion - from m to ft
        crestElev[var] = crestElev[var] / .3048;       
        nodeHead[var] = nodeHead[var] / .3048;       
        overlandHead[var] = overlandHead[var] / .3048;
        weirWidth[var] = weirWidth[var] / .3048;
        overflowArea[var] = overflowArea[var] / (.3048 * .3048);

        couplingType = swmm_couplingType(crestElev[var], nodeHead[var], overlandHead[var], overflowArea[var], weirWidth[var]);
        BOOST_CHECK_EQUAL(couplingType, ExpectedType[var]);

        CouplingInflow = swmm_findCouplingInflow(couplingType, crestElev[var], nodeHead[var], overlandHead[var],
                                                orificeCoeff, freeWeirCoeff, subWeirCoeff, overflowArea[var], weirWidth[var]);
        CouplingInflow = CouplingInflow * (.3048 * .3048 * .3048);
        BOOST_CHECK_SMALL(abs(CouplingInflow) - abs(ExpectedQ[var]), 0.005);
        BOOST_CHECK_SMALL(CouplingInflow - ExpectedQ[var], 0.005);

    }

    BOOST_FIXTURE_TEST_CASE(NodeOpeningTests, FixtureOpenClose) 
    {
        int error, node_ind, no_of_deleted_openings;
        int no_of_openings;
        double area, width;
        double NodeInflow = 0.0;
        char node_id[] = "J1";
        error = swmm_getObjectIndex(SM_NODE, node_id, &node_ind);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_REQUIRE(node_ind == 1);
        Node[node_ind].coverOpening = nullptr;

        // No openings added
        error = swmm_getOpeningsNum(node_ind, &no_of_openings);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_EQUAL(no_of_openings, 0);
    	error =  swmm_coupling_findNodeInflow(node_ind, tStep, Node[node_ind].invertElev, Node[node_ind].fullDepth, 
                                              Node[node_ind].newDepth, Node[node_ind].overlandDepth, 
						                      Node[node_ind].couplingArea, &NodeInflow);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_EQUAL(NodeInflow, 0.0);
        
        // Add a first opening to Node[node_ind]
        area = 25.0;
        width = 1.0;
        error = swmm_setNodeOpening(node_ind, 0, 0, area, width, orificeCoeff, freeWeirCoeff, subWeirCoeff);
        BOOST_CHECK_EQUAL(error, ERR_NONE);
        // One opening added
        error = swmm_getOpeningsNum(node_ind, &no_of_openings);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_EQUAL(no_of_openings, 1);

        // Test case 2 with one opening
        Node[node_ind].fullDepth = 2.0 / .3048;
        Node[node_ind].newDepth = 3.0  / .3048;
        Node[node_ind].overlandDepth = 0.0 / .3048;
        Node[node_ind].couplingArea = 1.0 / (.3048 * .3048);

        couplingType = swmm_couplingType(Node[node_ind].invertElev + Node[node_ind].fullDepth, Node[node_ind].invertElev + Node[node_ind].newDepth, 
                                                Node[node_ind].invertElev + Node[node_ind].fullDepth + Node[node_ind].overlandDepth, 
                                                Node[node_ind].couplingArea, width);
        BOOST_CHECK_EQUAL(couplingType, ORIFICE_COUPLING);
    	error =  swmm_coupling_findNodeInflow(node_ind, tStep, Node[node_ind].invertElev, Node[node_ind].fullDepth, 
                                              Node[node_ind].newDepth, Node[node_ind].overlandDepth, 
						                      Node[node_ind].couplingArea, &NodeInflow);
        // Type of coupling is ORIFICE_COUPLING
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_SMALL(NodeInflow * (.3048 * .3048 * .3048)- ExpectedQ[2], 0.005);
        // Test case 4 with one opening
        Node[node_ind].newDepth = 1.0  / .3048;
        Node[node_ind].overlandDepth = -1.0 / .3048;
    	error =  swmm_coupling_findNodeInflow(node_ind, tStep, Node[node_ind].invertElev, Node[node_ind].fullDepth, 
                                              Node[node_ind].newDepth, Node[node_ind].overlandDepth, 
						                      Node[node_ind].couplingArea, &NodeInflow);
        BOOST_REQUIRE(error == ERR_NONE);
        // Type of coupling is NO_COUPLING_FLOW
        BOOST_CHECK_EQUAL(NodeInflow, 0.0);
        // Test case 6 with one opening
        Node[node_ind].newDepth = 1.0  / .3048;
        Node[node_ind].overlandDepth = 1.0 / .3048;
        // Change area and width to correspond to test case 6
        area = 10.0;
        width = 5.0;
        error = swmm_setNodeOpening(node_ind, 0, 0, area, width, orificeCoeff, freeWeirCoeff, subWeirCoeff);
        BOOST_CHECK_EQUAL(error, ERR_NONE);
    	error =  swmm_coupling_findNodeInflow(node_ind, tStep, Node[node_ind].invertElev, Node[node_ind].fullDepth, 
                                              Node[node_ind].newDepth, Node[node_ind].overlandDepth, 
						                      Node[node_ind].couplingArea, &NodeInflow);
        BOOST_REQUIRE(error == ERR_NONE);
        // Type of coupling is FREE_WEIR_COUPLING
        BOOST_CHECK_SMALL(NodeInflow * (.3048 * .3048 * .3048)- ExpectedQ[6], 0.005);
        // Test case 8 with one opening
        Node[node_ind].newDepth = 3.0  / .3048;
        Node[node_ind].overlandDepth = 1.5 / .3048;
    	error =  swmm_coupling_findNodeInflow(node_ind, tStep, Node[node_ind].invertElev, Node[node_ind].fullDepth, 
                                              Node[node_ind].newDepth, Node[node_ind].overlandDepth, 
						                      Node[node_ind].couplingArea, &NodeInflow);
        BOOST_REQUIRE(error == ERR_NONE);
        // Type of coupling is SUBMERGED_WEIR_COUPLING
        BOOST_CHECK_SMALL(NodeInflow * (.3048 * .3048 * .3048)- ExpectedQ[8], 0.005);

        // Reset area and width of the first opening
        area = 25.0;
        width = 1.0;
        error = swmm_setNodeOpening(node_ind, 0, 0, area, width, orificeCoeff, freeWeirCoeff, subWeirCoeff);
        BOOST_CHECK_EQUAL(error, ERR_NONE);
        // Add a second opening, with different area and width
        area = 10.0;
        width = 5.0;
        error = swmm_setNodeOpening(node_ind, 1, 0, area, width, orificeCoeff, freeWeirCoeff, subWeirCoeff);
        BOOST_CHECK_EQUAL(error, ERR_NONE);
        // Two openings added
        error = swmm_getOpeningsNum(node_ind, &no_of_openings);
        BOOST_REQUIRE(error == ERR_NONE);
        BOOST_CHECK_EQUAL(no_of_openings, 2);

        // Test case 2 with two openings
        Node[node_ind].newDepth = 3.0  / .3048;
        Node[node_ind].overlandDepth = 0.0 / .3048;
    	error =  swmm_coupling_findNodeInflow(node_ind, tStep, Node[node_ind].invertElev, Node[node_ind].fullDepth, 
                                              Node[node_ind].newDepth, Node[node_ind].overlandDepth, 
						                      Node[node_ind].couplingArea, &NodeInflow);
        BOOST_REQUIRE(error == ERR_NONE);
        // Type of coupling is ORIFICE_COUPLING
        BOOST_CHECK_SMALL(NodeInflow * (.3048 * .3048 * .3048)- (ExpectedQ[2]-7.39718), 0.01);
        // Test case 4 with two openings
        Node[node_ind].newDepth = 1.0  / .3048;
        Node[node_ind].overlandDepth = -1.0 / .3048;
    	error =  swmm_coupling_findNodeInflow(node_ind, tStep, Node[node_ind].invertElev, Node[node_ind].fullDepth, 
                                              Node[node_ind].newDepth, Node[node_ind].overlandDepth, 
						                      Node[node_ind].couplingArea, &NodeInflow);
        BOOST_REQUIRE(error == ERR_NONE);
        // Type of coupling is NO_COUPLING_FLOW
        BOOST_CHECK_EQUAL(NodeInflow, 0.0);
        // Test case 6 with two openings
        Node[node_ind].newDepth = 1.0  / .3048;
        Node[node_ind].overlandDepth = 1.0 / .3048;
        // Reset area and width of the first opening
        area = 10.0;
        width = 5.0;
        error = swmm_setNodeOpening(node_ind, 0, 0, area, width, orificeCoeff, freeWeirCoeff, subWeirCoeff);
        BOOST_CHECK_EQUAL(error, ERR_NONE);
        // Change area and width of the second opening, to be different test from case 6
        area = 25.0;
        width = 1.0;
        error = swmm_setNodeOpening(node_ind, 1, 0, area, width, orificeCoeff, freeWeirCoeff, subWeirCoeff);
        BOOST_CHECK_EQUAL(error, ERR_NONE);
    	error =  swmm_coupling_findNodeInflow(node_ind, tStep, Node[node_ind].invertElev, Node[node_ind].fullDepth, 
                                              Node[node_ind].newDepth, Node[node_ind].overlandDepth, 
						                      Node[node_ind].couplingArea, &NodeInflow);
        BOOST_REQUIRE(error == ERR_NONE);
        // Type of coupling is FREE_WEIR_COUPLING
        BOOST_CHECK_SMALL(NodeInflow * (.3048 * .3048 * .3048)- (ExpectedQ[6]+1.5946), 0.005);
        // Test case 8 with two openings
        Node[node_ind].newDepth = 3.0  / .3048;
        Node[node_ind].overlandDepth = 1.5 / .3048;
    	error =  swmm_coupling_findNodeInflow(node_ind, tStep, Node[node_ind].invertElev, Node[node_ind].fullDepth, 
                                              Node[node_ind].newDepth, Node[node_ind].overlandDepth, 
						                      Node[node_ind].couplingArea, &NodeInflow);
        BOOST_REQUIRE(error == ERR_NONE);
        // Type of coupling is SUBMERGED_WEIR_COUPLING
        BOOST_CHECK_SMALL(NodeInflow * (.3048 * .3048 * .3048)- (ExpectedQ[8]+.26310), 0.005);

        //Close the second opening
        error = swmm_closeOpening(node_ind, 1);
        BOOST_CHECK_EQUAL(error, ERR_NONE);
        // Test case 2 with opening 1 open and opening 2 closed
        // Reset area and width
        area = 25.0;
        width = 1.0;
        error = swmm_setNodeOpening(node_ind, 0, 0, area, width, orificeCoeff, freeWeirCoeff, subWeirCoeff);
        BOOST_CHECK_EQUAL(error, ERR_NONE);
        // There are two openings
        error = swmm_getOpeningsNum(node_ind, &no_of_openings);
        BOOST_CHECK_EQUAL(no_of_openings, 2);
        Node[node_ind].newDepth = 3.0  / .3048;
        Node[node_ind].overlandDepth = 0.0 / .3048;
    	error =  swmm_coupling_findNodeInflow(node_ind, tStep, Node[node_ind].invertElev, Node[node_ind].fullDepth, 
                                              Node[node_ind].newDepth, Node[node_ind].overlandDepth, 
						                      Node[node_ind].couplingArea, &NodeInflow);
        BOOST_REQUIRE(error == ERR_NONE);
        // Type of coupling is ORIFICE_COUPLING
        BOOST_CHECK_SMALL(NodeInflow * (.3048 * .3048 * .3048)- ExpectedQ[2], 0.005);

        // Reopen the second opening
        error = swmm_openOpening(node_ind, 1);
        BOOST_CHECK_EQUAL(error, ERR_NONE);
        // Test case 2 with both openings 1 and opening 2 open
        // Reset area and width of the second opening
        area = 10.0;
        width = 5.0;
        error = swmm_setNodeOpening(node_ind, 1, 0, area, width, orificeCoeff, freeWeirCoeff, subWeirCoeff);
        BOOST_CHECK_EQUAL(error, ERR_NONE);
    	error =  swmm_coupling_findNodeInflow(node_ind, tStep, Node[node_ind].invertElev, Node[node_ind].fullDepth, 
                                              Node[node_ind].newDepth, Node[node_ind].overlandDepth, 
						                      Node[node_ind].couplingArea, &NodeInflow);
        BOOST_REQUIRE(error == ERR_NONE);
        // Type of coupling is ORIFICE_COUPLING
        BOOST_CHECK_SMALL(NodeInflow * (.3048 * .3048 * .3048)- (ExpectedQ[2]-7.39718), 0.01);

        // Add a third opening to Node[node_ind]
        error = swmm_setNodeOpening(node_ind, 2, 0, area, width, orificeCoeff, freeWeirCoeff, subWeirCoeff);
        BOOST_CHECK_EQUAL(error, ERR_NONE);
        // Three openings added
        error = swmm_getOpeningsNum(node_ind, &no_of_openings);
        BOOST_CHECK_EQUAL(no_of_openings, 3);
        // Delete the second opening
        error = swmm_deleteNodeOpening(node_ind, 1);
        BOOST_CHECK_EQUAL(error, ERR_NONE);
        error = swmm_getOpeningsNum(node_ind, &no_of_openings);
        BOOST_CHECK_EQUAL(error, ERR_NONE);
        // Two remaininig openings
        error = swmm_getOpeningsNum(node_ind, &no_of_openings);
        BOOST_CHECK_EQUAL(no_of_openings, 2);
        // Delete first opening
        error = swmm_deleteNodeOpening(node_ind, 0);
        BOOST_CHECK_EQUAL(error, ERR_NONE);
        // One remaininig opening
        error = swmm_getOpeningsNum(node_ind, &no_of_openings);
        BOOST_CHECK_EQUAL(no_of_openings, 1);
        // Delete the remaininig opening
        error = swmm_deleteNodeOpening(node_ind, 0);
        // No remaininig opening
        error = swmm_getOpeningsNum(node_ind, &no_of_openings);
        BOOST_CHECK_EQUAL(no_of_openings, 0);
        // Try to delete with no remaininig opening
        no_of_deleted_openings = swmm_deleteNodeOpening(node_ind, 0);
        BOOST_CHECK_EQUAL(error, ERR_NONE);
        BOOST_CHECK_EQUAL(no_of_deleted_openings, 0);

        // Add a first opening to Node[node_ind]
        area = 25.0;
        width = 1.0;
        error = swmm_setNodeOpening(node_ind, 0, 0, area, width, orificeCoeff, freeWeirCoeff, subWeirCoeff);
        BOOST_CHECK_EQUAL(error, ERR_NONE);
        // One opening added
        error = swmm_getOpeningsNum(node_ind, &no_of_openings);
        BOOST_CHECK_EQUAL(no_of_openings, 1);
        // Add a second opening, with different area and width
        area = 10.0;
        width = 5.0;
        error = swmm_setNodeOpening(node_ind, 1, 0, area, width, orificeCoeff, freeWeirCoeff, subWeirCoeff);
        BOOST_CHECK_EQUAL(error, ERR_NONE);
        // Two openings added
        error = swmm_getOpeningsNum(node_ind, &no_of_openings);
        BOOST_CHECK_EQUAL(no_of_openings, 2);
        // Delete openings
        error = swmm_deleteNodeOpenings(node_ind);
        BOOST_CHECK_EQUAL(error, ERR_NONE);
        // Check if there are remaining openings
        error = swmm_getOpeningsNum(node_ind, &no_of_openings);
        BOOST_CHECK_EQUAL(no_of_openings, 0);

    }

BOOST_AUTO_TEST_SUITE_END()
