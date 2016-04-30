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

/* vector.cpp */
/* Vector I/O */

#include <omp.h>

#include "gdal.h"
#include "gdal_priv.h"
#include "ogrsf_frmts.h"
#include "cpl_string.h"
#include "cpl_csv.h"

#include <opencv2/opencv.hpp>

#include "gdal-segment.hpp"

using namespace std;
using namespace cv;


void LabelContours( const cv::Mat klabels, std::vector< std::vector< LINE > >& linelists )
{
  // iterate through pixels and check edges
  printf ("Parse edges in segmented image\n");
  for (int y = 0; y < klabels.rows; y++)
  {
      const int yklabels = y*klabels.cols;
      for (int x = 0; x < klabels.cols; x++)
      {
          const int i = yklabels + x;

          LINE line;
          // check right pixel
          if ( x == klabels.cols - 1 )
          {
            line.sX = x + 1; line.sY = y;
            line.eX = x + 1; line.eY = y + 1;
            {
              linelists[klabels.at<u_int32_t>(i)].push_back(line);
            }

          } else if ( klabels.at<u_int32_t>(i)
                   != klabels.at<u_int32_t>(i + 1) )
          {
            line.sX = x + 1; line.sY = y;
            line.eX = x + 1; line.eY = y + 1;
            {
              linelists[klabels.at<u_int32_t>(i)].push_back(line);
            }
          }
          // check left pixel
          if ( x == 0 )
          {
            line.sX = x; line.sY = y;
            line.eX = x, line.eY = y + 1;
            {
              linelists[klabels.at<u_int32_t>(i)].push_back(line);
            }
          } else if ( klabels.at<u_int32_t>(i)
                   != klabels.at<u_int32_t>(i - 1) )
          {
            line.sX = x; line.sY = y;
            line.eX = x, line.eY = y + 1;
            {
              linelists[klabels.at<u_int32_t>(i)].push_back(line);
            }
          }
          // check top pixel
          if ( y == 0 )
          {
            line.sX = x;     line.sY = y;
            line.eX = x + 1; line.eY = y;
            {
              linelists[klabels.at<u_int32_t>(i)].push_back(line);
            }
          } else if ( klabels.at<u_int32_t>(i)
                   != klabels.at<u_int32_t>((y-1)*klabels.cols + x) )
          {
            line.sX = x; line.sY = y;
            line.eX = x + 1; line.eY = y;
            {
              linelists[klabels.at<u_int32_t>(i)].push_back(line);
            }
          }
          // check bottom pixel
          if ( y == klabels.rows - 1 )
          {
            line.sX = x;     line.sY = y + 1;
            line.eX = x + 1; line.eY = y + 1;
            {
              linelists[klabels.at<u_int32_t>(i)].push_back(line);
            }

          } else if ( klabels.at<u_int32_t>(i)
                   != klabels.at<u_int32_t>((y+1)*klabels.cols + x) )
          {
            line.sX = x;     line.sY = y + 1;
            line.eX = x + 1; line.eY = y + 1;
            {
              linelists[klabels.at<u_int32_t>(i)].push_back(line);
            }
          }
       }
       GDALTermProgress( (float)(y) / (float)(klabels.rows), NULL, NULL );
    }
    GDALTermProgress( 1.0f, NULL, NULL );
}


void SavePolygons( const std::vector< std::string > InFilenames,
                   const char *OutFilename,
                   const cv::Mat klabels,
                   const std::vector< cv::Mat > raster,
                   const std::vector< u_int32_t > labelpixels,
                   const std::vector< std::vector <double> > sumCH,
                   const std::vector< std::vector <double> > avgCH,
                   const std::vector< std::vector <double> > stdCH,
                   std::vector< std::vector< LINE > >& linelists )
{

  CPLLocaleC oLocaleCForcer();
  CPLErrorReset();

  const char *pszDriverName = "ESRI Shapefile";

#if GDALVER >= 2
  GDALDriver *liDriver;
  liDriver = GetGDALDriverManager()->GetDriverByName( pszDriverName );
#else
  OGRSFDriver *liDriver;
  liDriver = OGRSFDriverRegistrar::GetRegistrar()
           ->GetDriverByName( pszDriverName );
#endif

  if( liDriver == NULL )
  {
      printf( "\nERROR: %s driver not available.\n", pszDriverName );
      exit( 1 );
  }

  const size_t m_bands = raster.size();
  const size_t m_labels = labelpixels.size();

#if GDALVER >= 2
  GDALDataset *liDS;
  liDS = liDriver->Create( OutFilename, 0, 0, 0, GDT_Unknown, NULL );
#else
  OGRDataSource *liDS;
  liDS = liDriver->CreateDataSource( OutFilename, NULL );
#endif

  if( liDS == NULL )
  {
      printf( "\nERROR: Creation of output file failed.\n" );
      exit( 1 );
  }

  // dataset
  GDALDataset* piDataset;
  piDataset = (GDALDataset*) GDALOpen(InFilenames[0].c_str(), GA_ReadOnly);

  // spatialref
  OGRSpatialReference oSRS;
  oSRS.SetProjCS( piDataset->GetProjectionRef() );

  OGRLayer *liLayer;
  liLayer = liDS->CreateLayer( "segments", &oSRS, wkbPolygon, NULL );

  if( liLayer == NULL )
  {
      printf( "\nERROR: Layer creation failed.\n" );
      exit( 1 );
  }
  // spatial transform
  double adfGeoTransform[6];
  double oX = 0.0f; double oY = 0.0f;
  double mX = 1.0f; double mY = -1.0f;
  if( piDataset->GetGeoTransform( adfGeoTransform ) == CE_None ) {
      oX = adfGeoTransform[0]; oY = adfGeoTransform[3];
      mX = adfGeoTransform[1]; mY = adfGeoTransform[5];
  }
  GDALClose( (GDALDatasetH) piDataset );

  OGRFieldDefn *clsIdField = new OGRFieldDefn( "CLASS", OFTInteger );
  liLayer->CreateField( clsIdField );

  OGRFieldDefn *pixArField = new OGRFieldDefn( "AREA", OFTInteger );
  liLayer->CreateField( pixArField );

  for ( size_t b = 0; b < m_bands; b++ )
  {
     stringstream value; value << b+1;
     std::string FieldName = value.str() + "_AVERAGE";
     OGRFieldDefn *lavrgField = new OGRFieldDefn( FieldName.c_str(), OFTReal );
     liLayer->CreateField( lavrgField );
  }

  for ( size_t b = 0; b < m_bands; b++ )
  {
     stringstream value; value << b+1;
     std::string FieldName = value.str() + "_STDDEV";
     OGRFieldDefn *lavrgField = new OGRFieldDefn( FieldName.c_str(), OFTReal );
     liLayer->CreateField( lavrgField );
  }

  int multiring = 0;
  printf ("Write File: %s (polygon)\n", OutFilename);
  for (size_t k = 0; k < m_labels; k++)
  {

      if ( multiring == 1 )
      {
        k = k - 1;
        multiring = 0;
      }

      if ( linelists[k].size() == 0 )
        continue;

      // insert field data
      OGRFeature *liFeature;
      liFeature = OGRFeature::CreateFeature( liLayer->GetLayerDefn() );
      liFeature->SetField( "CLASS", (int) k );
      liFeature->SetField( "AREA", (int) labelpixels.at(k) );

      for ( size_t b = 0; b < m_bands; b++ )
      {
        stringstream value; value << b+1;
        std::string FieldName = value.str() + "_AVERAGE";
        liFeature->SetField( FieldName.c_str(), (double) avgCH[b].at(k) );
      }
      for ( size_t b = 0; b < m_bands; b++ )
      {
        stringstream value; value << b+1;
        std::string FieldName = value.str() + "_STDDEV";
        liFeature->SetField( FieldName.c_str(), stdCH[b].at(k) );
      }

      // initiate polygon start
      OGRLinearRing linestring;
      linestring.setCoordinateDimension(2);
      linestring.addPoint( oX + (double) linelists[k][0].sX * mX, oY + mY * (double) linelists[k][0].sY );
      linestring.addPoint( oX + (double) linelists[k][0].eX * mX, oY + mY * (double) linelists[k][0].eY );
      linelists[k].erase( linelists[k].begin() );

      // construct polygon from lines
      while ( linelists[k].size() > 0 )
      {
        if ( multiring == 1 ) break;

        vector<LINE>::iterator it = linelists[k].begin();
        for (; it != linelists[k].end(); ++it)
        {
          double ltX = linestring.getX(linestring.getNumPoints()-1);
          double ltY = linestring.getY(linestring.getNumPoints()-1);
          double csX = oX + (double) it->sX * mX;
          double csY = oY + mY * (double) it->sY;
          double ceX = oX + (double) it->eX * mX;
          double ceY = oY + mY * (double) it->eY;

          if ( ( csX == ltX  ) && ( csY == ltY ) ) {
              linestring.addPoint(ceX, ceY);
              linelists[k].erase(it);
              break;
          }
          if ( ( ceX == ltX  ) && ( ceY == ltY ) ) {
              linestring.addPoint(csX, csY);
              linelists[k].erase(it);
              break;
          }
          if (it == linelists[k].end()-1) {
              multiring = 1;
              break;
          }
        }
      }

      OGRPolygon polygon;
      linestring.closeRings();

      // simplify poligons
      // remove colinear vertices
      OGRLinearRing linesimple;
      float pointPrevX = 0, pointPrevY = 0;
      for (int i = 0; i < linestring.getNumPoints(); i++)
      {
        OGRPoint point;
        linestring.getPoint(i, &point);

        // start
        if ( i == 0)
        {
          linesimple.addPoint( &point );
          pointPrevX = point.getX();
          pointPrevY = point.getY();
          continue;
        }
        // end vertex
        if ( i == linestring.getNumPoints() - 1 )
        {
          linesimple.addPoint( &point );
          continue;
        }

        OGRPoint pointNext;
        linestring.getPoint(i+1, &pointNext);
        //     | x1 y1 1 |
        // det | x2 y2 1 | = 0 => p1,p2,p3 are colinear
        //     | x3 y3 1 |
        // x1*(y2-y3) + x2*(y3-y1) + x3*(y1-y2) == 0
        // only if not colinear with previous and next
        if ( pointPrevX*(point.getY()-pointNext.getY()) +
             point.getX()*(pointNext.getY()-pointPrevY) +
             pointNext.getX()*(pointPrevY-point.getY()) != 0 )
        {
          linesimple.addPoint( &point );
          pointPrevX = point.getX();
          pointPrevY = point.getY();
        }
      }

      // as polygon geometry
      polygon.addRing( &linesimple );
      liFeature->SetGeometry( &polygon );

      if( liLayer->CreateFeature( liFeature ) != OGRERR_NONE )
      {
         printf( "\nERROR: Failed to create feature in shapefile.\n" );
         exit( 1 );
      }
      OGRFeature::DestroyFeature( liFeature );
      GDALTermProgress( (float)(k+1) / (float)(m_labels), NULL, NULL );
  }
  GDALTermProgress( 1.0f, NULL, NULL );

#if GDALVER >= 2
  GDALClose( liDS );
#else
  OGRDataSource::DestroyDataSource( liDS );
#endif


}
