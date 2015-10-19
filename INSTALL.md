

 To compile and install GDAL-Segment GDAL, OpenCV and OpenCV_Contrib libraries are required.

  * GDAL [1] minimum version 2.0 is required, your distro probably already has it.
  * OpenCV [2] bigger than 3.0.0 (trunk) including OpenCV_Contrib [3] (trunk) is required.
  * OpenCV_Contrib has to have PR #261 [4] merged (current status is open).

```
  [1] http://www.gdal.org/
  [2] https://github.com/Itseez/opencv
  [3] https://github.com/Itseez/opencv_contrib
  [4] https://github.com/Itseez/opencv_contrib/pull/261
```

(1) Prepareing:

  * In order to have OpenCV (trunk) make sure distro has no *opencv* preinstalled !
```
  mkdir somewhere
  cd somewhere
  git clone https://github.com/Itseez/opencv.git
  cd opencv
  git clone -b slic https://github.com/Itseez/opencv_contrib.git
  cd opencv_contrib
  wget https://github.com/Itseez/opencv_contrib/pull/261.patch
  patch -p1 < 261.patch
  cd ..
  mkdir build
  cd build
  cmake CMAKE_VERBOSE=1 -DOPENCV_EXTRA_MODULES_PATH=../opencv_contrib/modules -DCMAKE_SKIP_RPATH=ON ../
  make
  make install
```
(2) GDAL-Segment:

  * To compile GDAL-Segment itself:
```
  mkdir somewhat
  git clone https://github.com/cbalint13/gdal-segment.git
  cd gdal-segment
  mkdir build
  cd build
  cmake ../
  make
  make install
```

---<<<---

Please cite following papers:

 * [1] "SLIC Superpixels Compared to State-of-the-art Superpixel Methods"
 Radhakrishna Achanta, Appu Shaji, Kevin Smith, Aurelien Lucchi, Pascal Fua,
 and Sabine Susstrunk, IEEE TPAMI, Volume 34, Issue 11, Pages 2274-2282,
 November 2012.

 * [2] "SLIC Superpixels" Radhakrishna Achanta, Appu Shaji, Kevin Smith,
 Aurelien Lucchi, Pascal Fua, and Sabine Süsstrunk, EPFL Technical
 Report no. 149300, June 2010.

--->>>---
