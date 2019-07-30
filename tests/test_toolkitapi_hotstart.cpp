/*
 *   test_toolkitAPI_hotstart.cpp
 *
 *   Created: 07/29/2019
 *   Author: Bryant E. McDonnell
 *           EmNet, a Xylem Brand
 *
 *   Unit testing mechanics for the hotstart API using Boost Test.
 */

// NOTE: Travis installs libboost test version 1.5.4
//#define BOOST_TEST_DYN_LINK

#define BOOST_TEST_MODULE "test_save_hotstart"
#include "test_toolkitapi.hpp"
#include <fstream>
#include <iterator>

#define BENCHMARK_HOTSTART "./hotstart/benchmark_hotstart.hsf"
#define HOTSTART_SWMM_SAVE_NORMAL "./hotstart/TestHSF.hsf"
#define HOTSTART_API_SAVE "./hotstart/swmm_api_test.hsf"
#define HOTSTART_SWMM_SAVE_19980101_014000 "./hotstart/TestHSF_19980101_014000.hsf"
#define HOTSTART_API_SAVE_19980101_014000 "./hotstart/swmm_api_test_19980101_014000.hsf"

#define ERR_NONE 0


BOOST_AUTO_TEST_SUITE(test_save_hotstart)

// Testing Run Simulation and Generate Hot Start File using Model (as normal)
// and save a hot start file using the API swmm_saveHotstart()
BOOST_FIXTURE_TEST_CASE(save_hotstart_file, FixtureBeforeEnd){
    int error, step_ind;
    int y, m, d, h, mn, s;
    double value;
    double elapsedTime = 0.0;

    // Cleanup files if they exist
    if (FILE *fl_check = fopen(HOTSTART_API_SAVE, "r"))
    {
      fclose(fl_check);
      remove(HOTSTART_API_SAVE);
    }
    if (FILE *fl_check = fopen(HOTSTART_SWMM_SAVE_NORMAL, "r"))
    {
      fclose(fl_check);
      remove(HOTSTART_SWMM_SAVE_NORMAL);
    }
    if (FILE *fl_check = fopen(HOTSTART_SWMM_SAVE_19980101_014000, "r"))
    {
      fclose(fl_check);
      remove(HOTSTART_SWMM_SAVE_19980101_014000);
    }
    if (FILE *fl_check = fopen(HOTSTART_API_SAVE_19980101_014000, "r"))
    {
      fclose(fl_check);
      remove(HOTSTART_API_SAVE_19980101_014000);
    }

    // Start simulation
    swmm_open((char *)DATA_PATH_INP, (char *)DATA_PATH_RPT, (char *)DATA_PATH_OUT);
    swmm_start(0);
    step_ind = 0;
    do
    {
        error = swmm_step(&elapsedTime);
        step_ind += 1;
        if (step_ind == 100)
        {
          swmm_getCurrentDateTime(&y, &m, &d, &h, &mn, &s);
          //char *output_time = (char*)malloc(30*sizeof(char));
          //sprintf(output_time, "%d/%d/%d %d:%d:%d",y,m,d,h,mn,s);
          //BOOST_TEST(1 == 0, output_time);
          swmm_saveHotstart((char *) HOTSTART_API_SAVE_19980101_014000);
        }

    }while (elapsedTime != 0 && !error);
    BOOST_CHECK_EQUAL(0, error);
    // Save New Hotstart File at the End of the Simulation
    error = swmm_saveHotstart((char *) HOTSTART_API_SAVE);
    BOOST_CHECK_EQUAL(0, error);

    swmm_end();
    swmm_close();


    // Start simulation 2
    swmm_open((char *)"hotstart/swmm_api_test_19980101_014000.inp",
              (char *)"hotstart/swmm_api_test_19980101_014000.rpt",
              (char *)"hotstart/swmm_api_test_19980101_014000.out");
    swmm_start(0);
    step_ind = 0;
    elapsedTime = 0.0;
    do
    {
        error = swmm_step(&elapsedTime);
        step_ind += 1;
    }while (step_ind != 100 && !error);
    BOOST_CHECK_EQUAL(0, error);
    //swmm_getCurrentDateTime(&y, &m, &d, &h, &mn, &s);
    //char *output_time = (char*)malloc(30*sizeof(char));
    //sprintf(output_time, "%d/%d/%d %d:%d:%d",y,m,d,h,mn,s);
    //BOOST_TEST(1 == 0, output_time);
    swmm_end();
    swmm_close();

    // Diff the three hot start files that come from the model.
    std::ifstream ifsbench1(BENCHMARK_HOTSTART);
    std::ifstream ifsbench2(BENCHMARK_HOTSTART);
    std::ifstream ifsbench_1998(HOTSTART_SWMM_SAVE_19980101_014000);
    std::ifstream ifs1(HOTSTART_SWMM_SAVE_NORMAL);
    std::ifstream ifs2(HOTSTART_API_SAVE);
    std::ifstream ifs1_1998(HOTSTART_API_SAVE_19980101_014000);

    std::istream_iterator<char> bbench(ifsbench1), ebench;
    std::istream_iterator<char> bbench2(ifsbench2), ebench2;
    std::istream_iterator<char> bench_1998(ifsbench_1998), ebench_1998;
    std::istream_iterator<char> b1(ifs1), e1;
    std::istream_iterator<char> b2(ifs2), e2;
    std::istream_iterator<char> b_1998(ifs1_1998), e_1998;

    // iterate over hotstart files and check all of them
    BOOST_CHECK_EQUAL_COLLECTIONS(bbench, ebench, b1, e1);
    BOOST_CHECK_EQUAL_COLLECTIONS(bbench2, ebench2, b2, e2);
    BOOST_CHECK_EQUAL_COLLECTIONS(b1, e1, b2, e2);
    BOOST_CHECK_EQUAL_COLLECTIONS(bench_1998, ebench_1998, b_1998, e_1998);


    // Testing USE the new generated hotstart file
    // Start simulation 2
    swmm_open((char *)"hotstart/swmm_api_test_use_hotstart.inp",
              (char *)"hotstart/swmm_api_test_use_hotstart.rpt",
              (char *)"hotstart/swmm_api_test_use_hotstart.out");
    swmm_start(0);
    step_ind = 0;
    elapsedTime = 0.0;
    do
    {
        if(step_ind == 0)
        {
          error = swmm_getNodeResult(0, SM_FULLDEPTH, &value);
          BOOST_CHECK_EQUAL(0, error);
          BOOST_CHECK_SMALL(value - 0.0012, 0.0002);
        }
        error = swmm_step(&elapsedTime);
        step_ind += 1;
    }while (step_ind != 1 && !error);
    BOOST_CHECK_EQUAL(0, error);
    //swmm_getCurrentDateTime(&y, &m, &d, &h, &mn, &s);
    //char *output_time = (char*)malloc(30*sizeof(char));
    //sprintf(output_time, "%d/%d/%d %d:%d:%d",y,m,d,h,mn,s);
    //BOOST_TEST(1 == 0, output_time);
    swmm_end();
    swmm_close();
}

BOOST_AUTO_TEST_SUITE_END()
