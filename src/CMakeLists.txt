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
#/* GDAL Segment */

ADD_EXECUTABLE(gdal-segment
               io/raster.cpp
               io/vector.cpp
               gdal-segment.cpp)

TARGET_LINK_LIBRARIES(gdal-segment ${GDAL_LIBRARY} ${OpenCV_LIBS})


