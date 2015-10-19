# gdal-segment

 * **GDAL Segment** implements various segmentation algorithms over raster images. It can
be used to segment large aerial, sattelite imagery of various formats supported by GDAL and
various layouts like multispectral or hyperspectral. It uses OpenCV for it's core algorithms,
and uses GDAL as I/O. Implementation here follows several multithread and memory optimizatons,
thus very large scenes are supported well.

```
    Usage: gdal-segment [-help] src_raster1 src_raster2 .. src_rasterN -out dst_vector
           -algo <SLICO (default), SLIC>] [-niter <1..500>]
           [-region <pixels>] [-ruler <1.00 ... 40.00>]

Default niter: 10 iterations
Default region: 10 pixels
Default ruler: 10.00
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
