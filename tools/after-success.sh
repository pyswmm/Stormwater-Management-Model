#! /bin/bash

# 
#  after-success.sh - collect artifacts
#
#  Date Created: 2020-06-17
#
#  Author:       Laurent Courty
#
#  Arguments: 
#    1 - relative path regression test file staging location 
#    2 - absolute path to location of software under test
#    3 - build identifier for software under test
#  
#  Note: 
#    Tests and benchmark files are stored in the swmm-example-networks repo.
#    This script retreives them using a stable URL associated with a release on 
#    GitHub and stages the files for nrtest to run. The script assumes that 
#    before-test.sh and gen-config.sh are located together in the same folder. 

cd $SWMM_HOME 
cd $RELEASE_HOME
cmake ../
if [[ "$TRAVIS_OS_NAME" == "linux" ]]; then
  cmake ../ -DBUILD_TESTS=OFF -DBUILD_COVERAGE=OFF ..
fi
if [[ "$TRAVIS_OS_NAME" == "osx" ]]; then
  cmake ../ -DBUILD_TESTS=OFF ..
fi
cmake --build . --config Release
ls -l $SWMM_HOME/$RELEASE_HOME/
cd $SWMM_HOME
SWMM_VERSION=$($SWMM_HOME/$RELEASE_HOME/bin/run-swmm --version 2>&1)
SWMM_VERSION="$(echo -e "${SWMM_VERSION}" | sed -n -e 'H;${x;s/\n//g;p;}')"
echo $SWMM_VERSION
mkdir $GIT_RELEASE_DIR
mkdir $GIT_RELEASE_DIR/$TRAVIS_OS_NAME
mkdir $GIT_RELEASE_DIR/$TRAVIS_OS_NAME/"lib"
mkdir $GIT_RELEASE_DIR/$TRAVIS_OS_NAME/"bin"
mkdir $GIT_RELEASE_DIR/$TRAVIS_OS_NAME/"include"
cp $SWMM_HOME/$RELEASE_HOME/lib/* $GIT_RELEASE_DIR/$TRAVIS_OS_NAME/"lib"
cp $SWMM_HOME/$RELEASE_HOME/bin/run-swmm $GIT_RELEASE_DIR/$TRAVIS_OS_NAME/"bin"
cp $SWMM_HOME/include/* $GIT_RELEASE_DIR/$TRAVIS_OS_NAME/"include"
echo "OWA-SWMM-$TRAVIS_OS_NAME-$SWMM_VERSION.zip"
cd $GIT_RELEASE_DIR/$TRAVIS_OS_NAME
zip -r "../../OWA-SWMM-$TRAVIS_OS_NAME-$SWMM_VERSION.zip" ./lib/ ./bin/ ./include/

if [[ "$TRAVIS_JOB_NAME" == "linux" ]]; then
  bash <(curl -s https://codecov.io/bash)
fi
