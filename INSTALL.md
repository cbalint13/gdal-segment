

 To compile and install GDAL-Segment GDAL, OpenCV and OpenCV_Contrib libraries are required.

  * GDAL [1] 1.x or 2.x is required, your distro probably already has it.
  * OpenCV [2] minimum 3.1.0 (or trunk) including OpenCV_Contrib [3] part is required.

```
  [1] http://www.gdal.org/
  [2] https://github.com/Itseez/opencv
  [3] https://github.com/Itseez/opencv_contrib
```

(1) Prepareing:

  * You need OpenCV >= 3.1.0 or trunk.
  * If you don't have these versions installed then make sure distro has no *opencv* preinstalled:
```
  mkdir somewhere
  cd somewhere
  git clone https://github.com/Itseez/opencv.git
  cd opencv
  git clone https://github.com/Itseez/opencv_contrib.git
  cd opencv_contrib
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

 * [3] "SEEDS: Superpixels extracted via energy-driven sampling"
 Van den Bergh, Michael and Boix, Xavier and Roig, Gemma and de Capitani,
 Benjamin and Van Gool, Luc, ECCV 2012

 * [4] "Superpixel Segmentation using Linear Spectral Clustering"
 Zhengqin Li, Jiansheng Chen, IEEE Conference on Computer Vision and Pattern
 Recognition (CVPR), Jun. 2015

"

--->>>---
