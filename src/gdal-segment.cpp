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

 "SEEDS: Superpixels Extracted via Energy-Driven Sampling",
 Van den Bergh M., Boix X., Roig G., de Capitani B. and Van Gool L.,
 In European Conference on Computer Vision (Vol. 7, pp. 13-26)., 2012

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
  int niter = 0;
  bool blur = false;
  int regionsize = 0;

  // some counters
  int64 startTime, endTime;
  int64 startSecond, endSecond;
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
        break;
      }
      if( EQUAL( argv[i],"-algo" ) ) {
        algo = argv[i+1];
        i++; continue;
      }
      if( EQUAL( argv[i],"-region" ) ) {
        regionsize = atoi(argv[i+1]);
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
      if( EQUAL( argv[i],"-blur" ) ) {
        blur = true;
        continue;
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
    if ( EQUAL( algo, "SLIC"  )
      || EQUAL( algo, "SLICO" ) )
    {
        if (!niter) niter = 10;
        if (!regionsize) regionsize = 10;
    }
    else if ( EQUAL( algo, "SEEDS" ) )
    {
        if (!niter) niter = 20;
        if (!regionsize) regionsize = 10;
    }
    else if ( EQUAL( algo, "LSC" ) )
    {
        if (!niter) niter = 20;
        if (!regionsize) regionsize = 10;
    }
    else
    {
      printf( "\nERROR: Invalid algorithm: %s\n", algo);
      help = true;
    }
    if ( InFilenames.size() == 0 )
    {
      printf( "\nERROR: No input file specified.\n");
      help = true;
    }
    if (! OutFilename )
    {
      printf( "\nERROR: No output file specified.\n");
      help = true;
    }
  }

  if ( help || askhelp ) {
    printf( "\nUsage: gdal-segment [-help] src_raster1 src_raster2 .. src_rasterN -out dst_vector\n"
            "    [-b R B (N-th band from R-th raster)] [-algo <SLICO (default), SLIC, SEEDS, LSC>]\n"
            "    [-niter <1..500>] [-region <pixels>]\n"
            "    [-blur (apply 3x3 gaussian blur)]\n\n"
            "Default niter: 10 iterations\n\n");

    GDALDestroyDriverManager();
    exit( 1 );
  }

  printf( "Segments raster using: %s\n", algo );
  printf( "Process use parameter: region=%i niter=%i\n", regionsize, niter );

  /*
   * load raster image
   */

  startTime = cv::getTickCount();
  std::vector< cv::Mat > raster;
  LoadRaster( InFilenames, raster );
  endTime = cv::getTickCount();
  printf( "Time: %.6f sec\n\n", ( endTime - startTime ) / frequency );

  if ( blur )
  {
    printf( "Apply Gaussian Blur (3x3 kernel)\n");
    startTime = cv::getTickCount();
    for( size_t b = 0; b < raster.size(); b++ )
    {
      GaussianBlur( raster[b], raster[b], Size( 3, 3 ), 0.0f, 0.0f, BORDER_DEFAULT );
      GDALTermProgress( (float)b / (float)raster.size(), NULL, NULL );
    }
    GDALTermProgress( 1.0f, NULL, NULL );
    endTime = cv::getTickCount();
    printf( "Time: %.6f sec\n\n", ( endTime - startTime ) / frequency );
  }

  /*
   * init segments
   */

  printf( "Init Superpixels\n");
  Ptr<SuperpixelSLIC> slic;
  Ptr<SuperpixelSEEDS> seed;
  Ptr<SuperpixelLSC> lsc;

  startTime = cv::getTickCount();
  if ( EQUAL ( algo, "SLIC" ) )
    slic = createSuperpixelSLIC( raster, SLIC, regionsize, 10.0f );
  else if ( EQUAL( algo, "SLICO" ) )
    slic = createSuperpixelSLIC( raster, SLICO, regionsize, 10.0f );
  else if ( EQUAL( algo, "LSC" ) )
    lsc = createSuperpixelLSC( raster, regionsize, 0.075f );
  else if ( EQUAL( algo, "SEEDS" ) )
  {
    // only few datatype is supported
    if ( ( raster[0].depth() != CV_8U )
       &&( raster.size() != 3 ) )
    {
      printf( "\nERROR: Input datatype is not supported by SEED. Use RGB or Gray with Byte types.\n" );
      exit( 0 );
    }

    int clusters = int(((float)raster[0].cols / (float)regionsize)
                     * ((float)raster[0].rows / (float)regionsize));
    seed = createSuperpixelSEEDS( raster[0].cols, raster[0].rows, raster.size(), clusters, 1, 2, 5, true );
  }
  else
  {
    printf( "\nERROR: No such algorithm: [%s].\n", algo );
    exit( 1 );
  }
  endTime = cv::getTickCount();
  printf( "Time: %.6f sec\n\n", ( endTime - startTime ) / frequency );

  size_t m_labels = 0;
  if ( EQUAL( algo, "SLIC" )
    || EQUAL( algo, "SLICO" ) )
    m_labels = slic->getNumberOfSuperpixels();
  else if ( EQUAL( algo, "SEEDS" ) )
    m_labels = seed->getNumberOfSuperpixels();
  else if ( EQUAL( algo, "LSC" ) )
    m_labels = lsc->getNumberOfSuperpixels();

  startTime = cv::getTickCount();
  printf( "Grow Superpixels: #%i iterations\n", niter );
  printf( "           inits: %lu superpixels\n", m_labels );

  /*
   * start compute segments
   */

  startSecond = cv::getTickCount();
  if ( EQUAL( algo, "SLIC" )
    || EQUAL( algo, "SLICO" ) )
    slic->iterate( niter );
  else if ( EQUAL( algo, "LSC" ) )
    lsc->iterate( niter );
  else if ( EQUAL( algo, "SEEDS" ) )
  {
    cv::Mat whole;
    cv::merge(raster,whole);
    seed->iterate( whole, niter );
  }
  endSecond = cv::getTickCount();


  if( EQUAL( algo, "SLIC" )
   || EQUAL( algo, "SLICO" ) )
    m_labels = slic->getNumberOfSuperpixels();
  else if ( EQUAL( algo, "SEEDS" ) )
    m_labels = seed->getNumberOfSuperpixels();
  else if ( EQUAL( algo, "LSC" ) )
    m_labels = lsc->getNumberOfSuperpixels();

  printf( "           count: %lu superpixels (growed in %.6f sec)\n",
          m_labels, ( endSecond - startSecond ) / frequency );

  // get smooth labels
  startSecond = cv::getTickCount();
  if ( EQUAL( algo, "SLIC" )
    || EQUAL( algo, "SLICO" ) )
  {
    slic->enforceLabelConnectivity();
  }
  else if ( EQUAL( algo, "LSC" ) )
  {
    lsc->enforceLabelConnectivity();
  }
  endSecond = cv::getTickCount();

  if ( EQUAL( algo, "SLIC" )
    || EQUAL( algo, "SLICO" ) )
    m_labels = slic->getNumberOfSuperpixels();
  else if ( EQUAL( algo, "SEEDS" ) )
    m_labels = seed->getNumberOfSuperpixels();
  else if ( EQUAL( algo, "LSC" ) )
    m_labels = lsc->getNumberOfSuperpixels();

  if ( ! EQUAL( algo, "SEEDS" ) )
  {
    printf( "           final: %lu superpixels (merged in %.6f sec)\n",
            m_labels, ( endSecond - startSecond ) / frequency );
    endTime = cv::getTickCount();
    printf( "Time: %.6f sec\n\n", ( endTime - startTime ) / frequency );
  }

  // superpixels property
  size_t m_bands = raster.size();

  // storage
  cv::Mat klabels;
  if( EQUAL( algo, "SLIC" )
   || EQUAL( algo, "SLICO" ) )
    slic->getLabels( klabels );
  else if( EQUAL( algo, "SEEDS" ) )
    seed->getLabels( klabels );
  else if( EQUAL( algo, "LSC" ) )
    lsc->getLabels( klabels );

  // release mem
  slic.release();
  seed.release();
  lsc.release();

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
