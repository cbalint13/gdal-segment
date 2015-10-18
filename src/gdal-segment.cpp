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

/*
 "SLIC Superpixels Compared to State-of-the-art Superpixel Methods"
 Radhakrishna Achanta, Appu Shaji, Kevin Smith, Aurelien Lucchi, Pascal Fua,
 and Sabine Susstrunk, IEEE TPAMI, Volume 34, Issue 11, Pages 2274-2282,
 November 2012.

 "SLIC Superpixels" Radhakrishna Achanta, Appu Shaji, Kevin Smith,
 Aurelien Lucchi, Pascal Fua, and Sabine Süsstrunk, EPFL Technical
 Report no. 149300, June 2010.
 */

/* slic-segment.cpp */
/* SLIC superpixels segment */

#include <omp.h>
#include <set>
#include <fstream>
#include <iostream>

#include "gdal.h"
#include "gdal_priv.h"
#include "ogrsf_frmts.h"
#include "cpl_string.h"
#include "cpl_csv.h"

#include "gdal-segment.hpp"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/ximgproc.hpp>

using namespace std;
using namespace cv;
using namespace cv::ximgproc;



int main(int argc, char ** argv)
{
  const char *algo = "SLICO";
  vector< string > InFilenames;
  const char *OutFilename = NULL;

  // general defaults
  int niter = 10;
  int regionsize = 10;
  float regularizer = 10.0f;

  // some counters
  int64 startTime, endTime;
  double frequency = cv::getTickFrequency();

  // register
  GDALAllRegister();
  OGRRegisterAll();

  argc = GDALGeneralCmdLineProcessor( argc, &argv, 0 );

  if( argc < 1 )
    exit( -argc );

  // default help
  bool help = false;
  bool askhelp = false;

  /*
   * load arguments
   */

  for( int i = 1; i < argc; i++ )
  {
    if( argv[i][0] == '-' )
    {
      if( EQUAL( argv[i],"-help" ) ) {
        askhelp = true;
        i++; break;
      }
      if( EQUAL( argv[i],"-algo" ) ) {
        algo = argv[i+1];
        i++; continue;
      }
      if( EQUAL( argv[i],"-region" ) ) {
        regionsize = atoi(argv[i+1]);
        i++; continue;
      }
      if( EQUAL( argv[i],"-ruler" ) ) {
        regularizer = atof(argv[i+1]);
        i++; continue;
      }
      if( EQUAL( argv[i],"-niter" ) ) {
        niter = atoi(argv[i+1]);
        i++; continue;
      }
      if( EQUAL( argv[i],"-out" ) ) {
        OutFilename = argv[i+1];
        i++; continue;
      }
      printf("Invalid %s option.\n\n", argv[i]);
      help = true;
    }
    else if( argv[i][0] != '-' )
    {
      InFilenames.push_back( argv[i] );
      continue;
    }
  }

  if ( !askhelp )
  {
    // check parameters
    if ((! EQUAL( algo, "SLICO")) && ( ! EQUAL( algo, "SLIC"))) {
      printf( "\nERROR: Invalid algorithm: %s\n", algo);
      help = true;
    }
    if ( InFilenames.size() == 0 ) {
      printf( "\nERROR: No input file specified.\n");
      help = true;
    }
    if (! OutFilename ) {
      printf( "\nERROR: No output file specified.\n");
      help = true;
    }
  }

  if ( help || askhelp ) {
    printf( "\nUsage: gdal-segment [-help] src_raster1 src_raster2 .. src_rasterN -out dst_vector\n"
            "    [-algo <SLICO (default), SLIC>] [-niter <1..500>] [-region <pixels>] [-ruler <1.00 ... 40.00>]\n\n"
            "Default niter: 10 iterations\n"
            "Default region: 10 pixels\n"
            "Default ruler: 10.00\n\n");

    GDALDestroyDriverManager();
    exit( 1 );
  }

  printf( "Segment raster using: %s\n", algo );
  printf( "Segmenting parameter: ruler=%.02f region=%i niter=%i\n", regularizer, regionsize, niter );

  /*
   * load raster image
   */

  startTime = cv::getTickCount();
  std::vector< cv::Mat > raster;
  LoadRaster( InFilenames, raster );
  endTime = cv::getTickCount();
  printf( "Time: %.6f sec\n\n", ( endTime - startTime ) / frequency );

  /*
   * init segments
   */

  printf( "Init Superpixels\n");
  Ptr<SuperpixelSLIC> slic;
  startTime = cv::getTickCount();
  if( EQUAL( algo, "SLIC" ) )
    slic = createSuperpixelSLIC( raster, SLIC, regionsize, regularizer );
  else if( EQUAL( algo, "SLICO" ) )
    slic = createSuperpixelSLIC( raster, SLICO, regionsize, regularizer );
  else
  {
    printf( "\nERROR: No such algorithm: [%s].\n", algo );
    exit( 1 );
  }
  endTime = cv::getTickCount();
  printf( "Time: %.6f sec\n\n", ( endTime - startTime ) / frequency );

  printf( "Grow Superpixels: #%i iterations\n", niter );
  printf( "           inits: %i superpixels\n", slic->getNumberOfSuperpixels() );

  /*
   * start compute segments
   */

  startTime = cv::getTickCount();
  slic->iterate( niter );
  endTime = cv::getTickCount();

  // get smooth labels
  printf( "           count: %i superpixels (grows in %.6f sec)\n",
          slic->getNumberOfSuperpixels(), ( endTime - startTime ) / frequency );
  startTime = cv::getTickCount();
  slic->enforceLabelConnectivity();
  endTime = cv::getTickCount();
  printf( "           final: %i superpixels (merge in %.6f sec)\n",
          slic->getNumberOfSuperpixels(), ( endTime - startTime ) / frequency );

  endTime = cv::getTickCount();
  printf( "Time: %.6f sec\n\n", ( endTime - startTime ) / frequency );

  // superpixels property
  size_t m_bands = raster.size();
  size_t m_labels = slic->getNumberOfSuperpixels();

  // storage
  cv::Mat klabels;
  slic->getLabels( klabels );

  // release mem
  slic.release();

  /*
   * get segments contour
   */

  std::vector< std::vector< LINE > > linelists( m_labels );

  startTime = cv::getTickCount();
  LabelContours( klabels, linelists );
  endTime = cv::getTickCount();
  printf( "Time: %.6f sec\n\n", ( endTime - startTime ) / frequency );


  /*
   * statistics
   */

  startTime = cv::getTickCount();
  std::vector< u_int32_t > labelpixels( m_labels, 0 );
  std::vector< std::vector <double> > sumCH( m_bands ); // sum
  std::vector< std::vector <double> > avgCH( m_bands ); // avg
  std::vector< std::vector <double> > stdCH( m_bands ); // std

  for ( size_t b = 0; b < m_bands; b++)
  {
    sumCH[b].resize( m_labels, 0 );
    avgCH[b].resize( m_labels, 0 );
    stdCH[b].resize( m_labels, 0 );
  }

  ComputeStats( klabels, raster, labelpixels, sumCH, avgCH, stdCH );
  endTime = cv::getTickCount();
  printf( "Time: %.6f sec\n\n", ( endTime - startTime ) / frequency );


 /*
  * dump vector
  */

  startTime = cv::getTickCount();
  SavePolygons( InFilenames, OutFilename, klabels, raster,
                labelpixels, sumCH, avgCH, stdCH, linelists );
  endTime = cv::getTickCount();
  printf( "Time: %.6f sec\n\n", ( endTime - startTime ) / frequency );


 /*
  * END
  */

  printf("Finish.\n");

  return 0;

}
