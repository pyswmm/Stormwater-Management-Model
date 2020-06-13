/*
 *   test_coupling.cpp
 *
 *   Created: 09/06/2020
 *   Author: Velitchko G. Tzatchkov
 *           Laurent Courty
 *           IMTA, Mexico
 *
 *   Unit testing for SWMM coupling using Boost Test.
 */

#define BOOST_TEST_MODULE "coupling"
#include <boost/test/included/unit_test.hpp>
#include <boost/test/data/test_case.hpp>
#include "headers.h"
#include "coupling.h"

// The following two declarations are required by coupling.c to work
TNode* Node = new TNode[2];
int Nobjects[MAX_OBJ_TYPES];

int couplingType;

// Test number             0     1     2     3     4     5     6     7     8     9     10
double crestElev[11] =    {2.0,  2.0,  2.0,  2.0,  2.0,  3.0,  2.0,  2.0,  2.0,  2.0, 2.0};
double nodeHead[11] =     {0.0,  2.0,  3.0,  4.0,  1.0,  2.0,  1.0,  3.0,  3.0,  3.0, 1.0};
double overlandHead[11] = {2.0,  2.0,  2.0,  3.0,  1.0,  1.0,  3.0,  4.0,  3.5,  5.0, 2.0};
double overflowArea[11] = {25.0, 25.0, 25.0, 25.0, 25.0, 25.0, 10.0, 10.0, 10.0, 10.0, 10.0};
double weirWidth[11] =    {1.0,  1.0,  1.0,  1.0,  1.0,  1.0,  5.0,  5.0,  5.0,  5.0, 5.0};
int ExpectedType[11] = {NO_COUPLING_FLOW, NO_COUPLING_FLOW, ORIFICE_COUPLING, ORIFICE_COUPLING,
                        NO_COUPLING_FLOW, NO_COUPLING_FLOW, FREE_WEIR_COUPLING, ORIFICE_COUPLING,
                        SUBMERGED_WEIR_COUPLING, ORIFICE_COUPLING, NO_COUPLING_FLOW};
        
BOOST_AUTO_TEST_SUITE(test_coupling)

    BOOST_DATA_TEST_CASE(ParameterizedCouplingTest, boost::unit_test::data::xrange(0, 11), var)
    {

        // Unit conversion - from m to ft
        double crestElev_var = crestElev[var] / .3048;       
        double nodeHead_var = nodeHead[var] / .3048;       
        double overlandHead_var = overlandHead[var] / .3048;
        double weirWidth_var = weirWidth[var] / .3048;
        double overflowArea_var = overflowArea[var] / (.3048 * .3048);

        couplingType = opening_findCouplingType(crestElev[var], nodeHead[var], overlandHead[var],
                                                overflowArea[var], weirWidth[var]);
        BOOST_CHECK_EQUAL(couplingType, ExpectedType[var]);

    }

BOOST_AUTO_TEST_SUITE_END()
