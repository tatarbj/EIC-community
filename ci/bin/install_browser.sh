#!/bin/sh
set -e

# ---------------------------------------------------------------------------- #
#
# Instals & runs the environment to do headless browser tests.
#
# ---------------------------------------------------------------------------- #


# No need for browser if the profile is not installed.
if [ $INSTALL_PROFILE -ne 1 ]; then
 exit 0;
fi

# Check first if we realy need this.
if [ "$BEHAT_TAG" -ne "javascript" ]; then
  exit 0
fi


# Create display.
export DISPLAY=:99.0
sh -e /etc/init.d/xvfb start
sleep 3

# Run selenium.
cd $TRAVIS_BUILD_DIR/capacity4more/behat
wget http://selenium-release.storage.googleapis.com/2.40/selenium-server-standalone-2.40.0.jar
java -jar selenium-server-standalone-2.40.0.jar -p 4444 &
sleep 5

# Run phantomJs.
phantomjs --webdriver=8643 > ~/phantomjs.log 2>&1 &

cd $TRAVIS_BUILD_DIR
