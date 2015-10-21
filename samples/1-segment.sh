#!/bin/bash

rm -rf output-slic.???
../bin/gdal-segment -algo SLIC -region 8 -niter 10 kermit000.jpg -out output-slic.shp

rm -rf output-slico.???
../bin/gdal-segment -algo SLICO -region 8 -niter 10 kermit000.jpg -out output-slico.shp

rm -rf output-seeds.???
../bin/gdal-segment -algo SEEDS -region 8 -niter 25 kermit000.jpg -out output-seeds.shp

