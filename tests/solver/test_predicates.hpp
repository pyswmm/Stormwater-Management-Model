/*
 ******************************************************************************
 Project:      OWA SWMM
 Version:      5.1.13
 Module:       test_predicates.hpp
 Description:  shared Boost predicate helpers for solver tests
 Authors:      see AUTHORS
 Copyright:    see AUTHORS
 License:      see LICENSE
 Last Updated: 03/17/2026
 ******************************************************************************
*/

#ifndef TEST_PREDICATES_HPP
#define TEST_PREDICATES_HPP

#include <vector>
#include <string>
#include <cmath>

#include <boost/test/tools/interface.hpp>

inline boost::test_tools::predicate_result check_cdd_double(
    const std::vector<double>& test,
    const std::vector<double>& ref,
    long cdd_tol)
{
    double min_cdd = 10.0;

    auto test_it = test.begin();
    auto ref_it = ref.begin();

    for (; (test_it < test.end()) && (ref_it < ref.end()); ++test_it, ++ref_it)
    {
        if (*test_it != *ref_it) {
            double tmp = std::abs(*test_it - *ref_it);
            if (tmp < 1.0e-7)
                tmp = 1.0e-7;
            else if (tmp > 2.0)
                tmp = 1.0;

            tmp = -std::log10(tmp);
            if (tmp < 0.0)
                tmp = 0.0;

            if (tmp < min_cdd)
                min_cdd = tmp;
        }
    }

    return std::floor(min_cdd) >= cdd_tol;
}

inline boost::test_tools::predicate_result check_string(
    const std::string& test,
    const std::string& ref)
{
    return ref.compare(test) == 0;
}

#endif // TEST_PREDICATES_HPP
