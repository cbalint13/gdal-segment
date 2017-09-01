#!/bin/bash

rm -rf output-slic.???
../bin/gdal-segment -algo SLIC -region 10 -niter 10 25cm_orto0.jpg -out output-slic.shp

rm -rf output-slico.???
../bin/gdal-segment -algo SLICO -region 10 -niter 10 25cm_orto0.jpg -out output-slico.shp

rm -rf output-mslic.???
../bin/gdal-segment -blur -algo MSLIC -region 15 -niter 10 25cm_orto0.jpg -out output-mslic.shp

rm -rf output-seeds.???
../bin/gdal-segment -algo SEEDS -region 10 -niter 25 25cm_orto0.jpg -out output-seeds.shp

rm -rf output-lsc.???
../bin/gdal-segment -algo LSC -region 10 -niter 20 25cm_orto0.jpg -out output-lsc.shp
