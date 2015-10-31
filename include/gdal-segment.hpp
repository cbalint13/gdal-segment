/*
 *  Copyright (c) 2015  Balint Cristian (cristian.balint@gmail.com)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 */

/* gdal-segment.hpp */
/* main include */

#ifndef SLICSEG_H
#define SLICSEG_H

#include <opencv2/opencv.hpp>



typedef __int32 u_int32_t;

typedef struct LINE {
  unsigned int sX;
  unsigned int sY;
  unsigned int eX;
  unsigned int eY;
} LINE;

// raster operation
void LoadRaster( const std::vector< std::string > InFilenames,
                 std::vector< cv::Mat >& raster );

// raster statistics
void ComputeStats( const cv::Mat klabels,
                   const std::vector< cv::Mat > raster,
                   std::vector< u_int32_t >& labelpixels,
                   std::vector< std::vector <double> >& sumCH,
                   std::vector< std::vector <double> >& avgCH,
                   std::vector< std::vector <double> >& stdCH );

// vector contours
void LabelContours( const cv::Mat klabels, std::vector< std::vector< LINE > >& linelists);

// vactor dump
void SavePolygons( const std::vector< std::string > InFilenames,
                   const char *OutFilename,
                   const cv::Mat klabels,
                   const std::vector< cv::Mat > raster,
                   const std::vector< u_int32_t > labelpixels,
                   const std::vector< std::vector <double> > sumCH,
                   const std::vector< std::vector <double> > avgCH,
                   const std::vector< std::vector <double> > stdCH,
                   std::vector< std::vector< LINE > >& linelists );

#endif
