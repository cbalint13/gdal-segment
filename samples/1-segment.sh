#!/bin/bash

rm -rf output.???
../bin/gdal-segment -algo SLIC -region 8 kermit000.jpg -out output.shp

