// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>



// TODO:  在此处引用程序需要的其他头文件
#pragma comment(lib, "ippicvmt.lib")
#ifdef _DEBUG
#pragma comment(lib, "IlmImfd.lib")
#pragma comment(lib, "libjasperd.lib")
#pragma comment(lib, "libjpegd.lib")
#pragma comment(lib, "libpngd.lib")
#pragma comment(lib, "libtiffd.lib")
#pragma comment(lib, "libwebpd.lib")
#pragma comment(lib, "zlibd.lib")
#pragma comment(lib, "opencv_calib3d310d.lib")
#pragma comment(lib, "opencv_core310d.lib")
#pragma comment(lib, "opencv_features2d310d.lib")
#pragma comment(lib, "opencv_highgui310d.lib")
#pragma comment(lib, "opencv_imgcodecs310d.lib")
#pragma comment(lib, "opencv_imgproc310d.lib")
#pragma comment(lib, "opencv_ml310d.lib")
#pragma comment(lib, "opencv_objdetect310d.lib")
#pragma comment(lib, "opencv_photo310d.lib")
#pragma comment(lib, "opencv_shape310d.lib")
#pragma comment(lib, "opencv_stitching310d.lib")
#pragma comment(lib, "opencv_superres310d.lib")
#pragma comment(lib, "opencv_video310d.lib")
#pragma comment(lib, "opencv_videoio310d.lib")
#pragma comment(lib, "opencv_videostab310d.lib")
#else
#pragma comment(lib, "IlmImf.lib")
#pragma comment(lib, "libjasper.lib")
#pragma comment(lib, "libjpeg.lib")
#pragma comment(lib, "libpng.lib")
#pragma comment(lib, "libtiff.lib")
#pragma comment(lib, "libwebp.lib")
#pragma comment(lib, "zlib.lib")
#pragma comment(lib, "opencv_calib3d310.lib")
#pragma comment(lib, "opencv_core310.lib")
#pragma comment(lib, "opencv_features2d310.lib")
#pragma comment(lib, "opencv_highgui310.lib")
#pragma comment(lib, "opencv_imgcodecs310.lib")
#pragma comment(lib, "opencv_imgproc310.lib")
#pragma comment(lib, "opencv_ml310.lib")
#pragma comment(lib, "opencv_objdetect310.lib")
#pragma comment(lib, "opencv_photo310.lib")
#pragma comment(lib, "opencv_shape310.lib")
#pragma comment(lib, "opencv_stitching310.lib")
#pragma comment(lib, "opencv_superres310.lib")
#pragma comment(lib, "opencv_video310.lib")
#pragma comment(lib, "opencv_videoio310.lib")
#pragma comment(lib, "opencv_videostab310.lib")
#endif