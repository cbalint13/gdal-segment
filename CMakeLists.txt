#/*
# *  Copyright (c) 2015  Balint Cristian (cristian.balint@gmail.com)
# *
# *  This program is free software; you can redistribute it and/or modify
# *  it under the terms of the GNU General Public License as published by
# *  the Free Software Foundation; either version 2 of the License, or
# *  (at your option) any later version.
# *
# *  This program is distributed in the hope that it will be useful,
# *  but WITHOUT ANY WARRANTY; without even the implied warranty of
# *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# *  GNU General Public License for more details.
# *
# */

#/* CMakeLists.txt */
#/* Project builder */

PROJECT(gdal-segment)

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

IF(COMMAND CMAKE_POLICY)
     CMAKE_POLICY(SET CMP0016 NEW)
ENDIF(COMMAND CMAKE_POLICY)

SET(CMAKE_SKIP_BUILD_RPATH TRUE)
SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE)

SET(CMAKE_MODULE_PATH "cmake/;${CMAKE_MODULE_PATH}")

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

# Set a default build type if none was specified
IF(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  MESSAGE(STATUS "Setting build type to 'Release' as none was specified.")
  SET(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  SET_PROPERTY(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release"
                     "MinSizeRel" "RelWithDebInfo")
ENDIF()

SET(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -O3")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -O3")

#####
# find OpenMP library
FIND_PACKAGE(OpenMP)
IF(OPENMP_FOUND)
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
  SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
  MESSAGE(STATUS "OpenMP found.")
ENDIF()

#####
# find OpenCV
FIND_PACKAGE(OpenCV REQUIRED core ximgproc)
IF(OpenCV_FOUND)
  INCLUDE_DIRECTORIES(${OpenCV_INCLUDE_DIRS})
  MESSAGE(STATUS "OpenCV version: " ${OpenCV_VERSION})
  STRING(COMPARE LESS "${OpenCV_VERSION}" 3.0.0 VersionIncompatible)
  IF(VersionIncompatible)
    MESSAGE(FATAL_ERROR "OpenCV minimum 3.0.0 required.")
  ENDIF()
ENDIF()

#####
# find GDAL library
FIND_PACKAGE(GDAL COMPONENTS REQUIRED)
IF(GDAL_FOUND)
  INCLUDE_DIRECTORIES(${GDAL_INCLUDE_DIRS})
  MESSAGE(STATUS "GDAL found.")
ENDIF()

# local directory
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/include/)

ADD_SUBDIRECTORY(src/)
