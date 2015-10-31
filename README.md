# gdal-segment

![Logo](https://github.com/cbalint13/gdal-segment/blob/master/samples/logo/small_logo.png)

 * **GDAL Segment** implements various segmentation algorithms over raster images. It can
be used to segment large aerial, satellite imagery of various formats supported by GDAL and
various layouts like multispectral or hyperspectral. It uses OpenCV for it's core algorithms,
and GDAL for undelying I/O. Implementation here follows several multithread and memory
friendly optimizations, thus very large scenes are supported well.

 * At this moment it implements SLIC, SLICO and SEEDS.

```
Usage: gdal-segment [-help] src_raster1 src_raster2 .. src_rasterN -out dst_vector
    [-b R B (N-th band from R-th raster)] [-algo <SLICO (default), SLIC, SEEDS, LSC>]
    [-niter <1..500>] [-region <pixels>]
    [-blur (apply 3x3 gaussian blur)]

Default niter: 10 iterations
```

 * Please follow INSTALL.md for detailed requirements and install steps.

**Related citations:**

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

