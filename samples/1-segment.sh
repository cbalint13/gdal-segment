#!/bin/bash

rm -rf output.???
../bin/gdal-segment -algo SLICO -region 8 kermit000.jpg output.shp

