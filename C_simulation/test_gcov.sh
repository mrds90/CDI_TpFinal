#!/bin/sh

ceedling test:all
ceedling gcov:all utils:gcov 
xdg-open build/artifacts/gcov/GcovCoverageResults.html