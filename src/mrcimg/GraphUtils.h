//------------------------------------------------------------------------------
// Graphing functions for OpenCV. Part of "ImageUtils.cpp", a set of handy utility functions for dealing with images in OpenCV.
// by Shervin Emami (http://www.shervinemami.co.cc/) on 20th May, 2010.
// Updated for OpenCV 4 by [Your Name] on [Date].
//------------------------------------------------------------------------------

#ifndef GRAPH_UTILS_H
#define GRAPH_UTILS_H

#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>

// Allow 'bool' variables in both C and C++ code.
#ifndef __cplusplus
    #include <stdbool.h>
#endif

//------------------------------------------------------------------------------
// Graphing functions
//------------------------------------------------------------------------------

namespace cv {

// Draw the graph of an array of floats into imageDst or a new image, between minV & maxV if given.
Mat drawFloatGraph(const float *arraySrc, int nArrayLength, Mat imageDst = Mat(),
                  float minV = 0.0f, float maxV = 0.0f,
                  int width = 0, int height = 0,
                  const char *graphLabel = nullptr, bool showScale = true);

// Draw the graph of an array of ints into imageDst or a new image, between minV & maxV if given.
Mat drawIntGraph(const int *arraySrc, int nArrayLength, Mat imageDst = Mat(),
                int minV = 0, int maxV = 0,
                int width = 0, int height = 0,
                const char *graphLabel = nullptr, bool showScale = true);

// Draw the graph of an array of uchars into imageDst or a new image, between minV & maxV if given.
Mat drawUCharGraph(const uchar *arraySrc, int nArrayLength, Mat imageDst = Mat(),
                  int minV = 0, int maxV = 0,
                  int width = 0, int height = 0,
                  const char *graphLabel = nullptr, bool showScale = true);

// Display a graph of the given float array.
// If background is provided, it will be drawn into, for combining multiple graphs using drawFloatGraph().
// Set delay_ms to 0 if you want to wait forever until a keypress, or set it to 1 if you want it to delay just 1 millisecond.
void showFloatGraph(const char *name, const float *arraySrc, int nArrayLength,
                   int delay_ms = 500, Mat background = Mat());

// Display a graph of the given int array.
// If background is provided, it will be drawn into, for combining multiple graphs using drawIntGraph().
// Set delay_ms to 0 if you want to wait forever until a keypress, or set it to 1 if you want it to delay just 1 millisecond.
void showIntGraph(const char *name, const int *arraySrc, int nArrayLength,
                 int delay_ms = 500, Mat background = Mat());

// Display a graph of the given unsigned char array.
// If background is provided, it will be drawn into, for combining multiple graphs using drawUCharGraph().
// Set delay_ms to 0 if you want to wait forever until a keypress, or set it to 1 if you want it to delay just 1 millisecond.
void showUCharGraph(const char *name, const uchar *arraySrc, int nArrayLength,
                   int delay_ms = 500, Mat background = Mat());

// Simple helper function to easily view an image, with an optional pause.
void showImage(const Mat &img, int delay_ms = 0, const char *name = nullptr);

// Call 'setGraphColor(0)' to reset the colors that will be used for graphs.
void setGraphColor(int index = 0);

// Specify the exact color that the next graph should be drawn as.
void setCustomGraphColor(int R, int B, int G);

} // namespace cv

#endif // GRAPH_UTILS_H
