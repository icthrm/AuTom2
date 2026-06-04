//------------------------------------------------------------------------------
// Graphing functions for OpenCV.	Part of "ImageUtils.cpp", a set of handy utility functions for dealing with images in OpenCV.
// by Shervin Emami (http://www.shervinemami.co.cc/) on 20th May, 2010.
// Updated for OpenCV 4 by [Your Name] on [Date].
//------------------------------------------------------------------------------

#define USE_HIGHGUI		// Enable this to display graph windows using OpenCV's HighGUI. (Supports Windows, Linux & Mac, but not iPhone).

#include <stdio.h>
#include <iostream>
#include <string>

// OpenCV
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>

#ifndef UCHAR
	typedef unsigned char UCHAR;
#endif

#include "GraphUtils.h"

using namespace cv;
using namespace std;

//------------------------------------------------------------------------------
// Graphing functions
//------------------------------------------------------------------------------
const Scalar BLACK = Scalar(0,0,0);
const Scalar WHITE = Scalar(255,255,255);
const Scalar GREY = Scalar(150,150,150);

int countGraph = 0;	// Used by 'getGraphColor()'
Scalar customGraphColor;
int usingCustomGraphColor = 0;

// Get a new color to draw graphs. Will use the latest custom color, or change between blue, green, red, dark-blue, dark-green and dark-red until a new image is created.
Scalar getGraphColor(void)
{
	if (usingCustomGraphColor) {
		usingCustomGraphColor = 0;
		return customGraphColor;
	}

	countGraph++;
	switch (countGraph) {
	case 1:	return Scalar(60,60,255);	// light-blue
	case 2:	return Scalar(60,255,60);	// light-green
	case 3:	return Scalar(255,60,40);	// light-red
	case 4:	return Scalar(0,210,210);	// blue-green
	case 5:	return Scalar(180,210,0);	// red-green
	case 6:	return Scalar(210,0,180);	// red-blue
	case 7:	return Scalar(0,0,185);		// dark-blue
	case 8:	return Scalar(0,185,0);		// dark-green
	case 9:	return Scalar(185,0,0);		// dark-red
	default:
		countGraph = 0;	// start rotating through colors again.
		return Scalar(200,200,200);	// grey
	}
}

// Call 'setGraphColor()' to reset the colors that will be used for graphs.
void setGraphColor(int index)
{
	countGraph = index;
	usingCustomGraphColor = 0;	// dont use a custom color.
}

// Specify the exact color that the next graph should be drawn as.
void setCustomGraphColor(int R, int B, int G)
{
	customGraphColor = Scalar(R, G, B);
	usingCustomGraphColor = 1;	// show that it will be used.
}

// Draw the graph of an array of floats into imageDst or a new image, between minV & maxV if given.
// Remember to free the newly created image if imageDst is not given.
Mat drawFloatGraph(const float *arraySrc, int nArrayLength, Mat imageDst, float minV, float maxV, int width, int height, char *graphLabel, bool showScale)
{
    printf("drawFloatGraph\n");
    int w = width;
    int h = height;
    int b = 10;		// border around graph within the image
    int d = 10;
    if (w <= 20)
        w = nArrayLength*d + b*2;	// width of the image
    if (h <= 20)
        h = 220;

    int s = h - b*2;// size of graph height
    float xscale = 1.0;
    if (nArrayLength > 1)
        xscale = (w - b*2) / (float)(nArrayLength-1);	// horizontal scale
    Mat imageGraph;	// output image

    // Get the desired image to draw into.
    if (imageDst.empty()) {
        // Create an RGB image for graphing the data
        imageGraph = Mat(h, w, CV_8UC3, WHITE);
    }
    else {
        // Draw onto the given image.
        imageDst.copyTo(imageGraph);
    }
    if (imageGraph.empty()) {
        cerr << "ERROR in drawFloatGraph(): Couldn't create image of " << w << " x " << h << endl;
        exit(1);
    }
    Scalar colorGraph = getGraphColor();	// use a different color each time.

    // If the user didnt supply min & max values, find them from the data, so we can draw it at full scale.
    if (fabs(minV) < 0.0000001f && fabs(maxV) < 0.0000001f) {
        for (int i=0; i<nArrayLength; i++) {
            float v = (float)arraySrc[i];
            if (v < minV)
                minV = v;
            if (v > maxV)
                maxV = v;
        }
    }
    float diffV = maxV - minV;
    if (diffV == 0)
        diffV = 0.00000001f;	// Stop a divide-by-zero error
    float fscale = (float)s / diffV;

    // Draw the horizontal & vertical axis
    int y0 = cvRound(minV*fscale);
    line(imageGraph, Point(b,h-(b-y0)), Point(w-b, h-(b-y0)), BLACK);
    line(imageGraph, Point(b,h-(b)), Point(b, h-(b+s)), BLACK);

    // Write the scale of the y axis
    if (showScale) {
        Scalar clr = GREY;
        char text[16];
        sprintf(text, "%.1f", maxV);
        putText(imageGraph, text, Point(1, b+4), FONT_HERSHEY_PLAIN, 0.55, clr);
        // Write the scale of the x axis
        sprintf(text, "%d", (nArrayLength-1) );
        putText(imageGraph, text, Point(w-b+4-5*strlen(text), (h/2)+10), FONT_HERSHEY_PLAIN, 0.55, clr);
    }

    // Draw the values
    Point ptPrev = Point(b,h-(b-y0));	// Start the lines at the 1st point.
    for (int i=0; i<nArrayLength; i++) {
        int y = cvRound((arraySrc[i] - minV) * fscale);	// Get the values at a bigger scale
        int x = cvRound(i * xscale);
        Point ptNew = Point(b+x, h-(b+y));
        line(imageGraph, ptPrev, ptNew, colorGraph, 1, LINE_AA);	// Draw a line from the previous point to the new point
        ptPrev = ptNew;
    }

    // Write the graph label, if desired
    if (graphLabel != NULL && strlen(graphLabel) > 0) {
        putText(imageGraph, graphLabel, Point(30, 10), FONT_HERSHEY_PLAIN, 0.55, Scalar(0,0,0));	// black text
    }

    return imageGraph;
}

// Draw the graph of an array of ints into imageDst or a new image, between minV & maxV if given.
// Remember to free the newly created image if imageDst is not given.
Mat drawIntGraph(const int *arraySrc, int nArrayLength, Mat imageDst, int minV, int maxV, int width, int height, char *graphLabel, bool showScale)
{
    int w = width;
    int h = height;
    int b = 10;		// border around graph within the image
    if (w <= 20)
        w = nArrayLength + b*2;	// width of the image
    if (h <= 20)
        h = 220;

    int s = h - b*2;// size of graph height
    float xscale = 1.0;
    if (nArrayLength > 1)
        xscale = (w - b*2) / (float)(nArrayLength-1);	// horizontal scale
    Mat imageGraph;	// output image

    // Get the desired image to draw into.
    if (imageDst.empty()) {
        // Create an RGB image for graphing the data
        imageGraph = Mat(h, w, CV_8UC3, WHITE);
    }
    else {
        // Draw onto the given image.
        imageDst.copyTo(imageGraph);
    }
    if (imageGraph.empty()) {
        cerr << "ERROR in drawIntGraph(): Couldn't create image of " << w << " x " << h << endl;
        exit(1);
    }
    Scalar colorGraph = getGraphColor();	// use a different color each time.

    // If the user didnt supply min & max values, find them from the data, so we can draw it at full scale.
    if (minV == 0 && maxV == 0) {
        for (int i=0; i<nArrayLength; i++) {
            int v = arraySrc[i];
            if (v < minV)
                minV = v;
            if (v > maxV)
                maxV = v;
        }
    }
    int diffV = maxV - minV;
    if (diffV == 0)
        diffV = 1;	// Stop a divide-by-zero error
    float fscale = (float)s / (float)diffV;

    // Draw the horizontal & vertical axis
    int y0 = cvRound(minV*fscale);
    line(imageGraph, Point(b,h-(b-y0)), Point(w-b, h-(b-y0)), BLACK);
    line(imageGraph, Point(b,h-(b)), Point(b, h-(b+s)), BLACK);

    // Write the scale of the y axis
    if (showScale) {
        Scalar clr = GREY;
        char text[16];
        sprintf(text, "%.1f", maxV);
        putText(imageGraph, text, Point(1, b+4), FONT_HERSHEY_PLAIN, 0.55, clr);
        // Write the scale of the x axis
        sprintf(text, "%d", (nArrayLength-1));
        putText(imageGraph, text, Point(w-b+4-5*strlen(text), (h/2)+10), FONT_HERSHEY_PLAIN, 0.55, clr);
    }

    // Draw the values
    Point ptPrev = Point(b,h-(b-y0));	// Start the lines at the 1st point.
    for (int i=0; i<nArrayLength; i++) {
        int y = cvRound((arraySrc[i] - minV) * fscale);	// Get the values at a bigger scale
        int x = cvRound(i * xscale);
        Point ptNew = Point(b+x, h-(b+y));
        line(imageGraph, ptPrev, ptNew, colorGraph, 1, LINE_AA);	// Draw a line from the previous point to the new point
        ptPrev = ptNew;
    }

    // Write the graph label, if desired
    if (graphLabel != NULL && strlen(graphLabel) > 0) {
        putText(imageGraph, graphLabel, Point(30, 10), FONT_HERSHEY_PLAIN, 0.55, Scalar(0,0,0));	// black text
    }

    return imageGraph;
}

// Draw the graph of an array of uchars into imageDst or a new image, between minV & maxV if given..
// Remember to free the newly created image if imageDst is not given.
Mat drawUCharGraph(const uchar *arraySrc, int nArrayLength, Mat imageDst, int minV, int maxV, int width, int height, char *graphLabel, bool showScale)
{
    int w = width;
    int h = height;
    int b = 10;		// border around graph within the image
    if (w <= 20)
        w = nArrayLength + b*2;	// width of the image
    if (h <= 20)
        h = 220;

    int s = h - b*2;// size of graph height
    float xscale = 1.0;
    if (nArrayLength > 1)
        xscale = (w - b*2) / (float)(nArrayLength-1);	// horizontal scale
    Mat imageGraph;	// output image

    // Get the desired image to draw into.
    if (imageDst.empty()) {
        // Create an RGB image for graphing the data
        imageGraph = Mat(h, w, CV_8UC3, WHITE);
    }
    else {
        // Draw onto the given image.
        imageDst.copyTo(imageGraph);
    }
    if (imageGraph.empty()) {
        cerr << "ERROR in drawUCharGraph(): Couldn't create image of " << w << " x " << h << endl;
        exit(1);
    }
    Scalar colorGraph = getGraphColor();	// use a different color each time.

    // If the user didnt supply min & max values, find them from the data, so we can draw it at full scale.
    if (minV == 0 && maxV == 0) {
        for (int i=0; i<nArrayLength; i++) {
            int v = arraySrc[i];
            if (v < minV)
                minV = v;
            if (v > maxV)
                maxV = v;
        }
    }
    int diffV = maxV - minV;
    if (diffV == 0)
        diffV = 1;	// Stop a divide-by-zero error
    float fscale = (float)s / (float)diffV;

    // Draw the horizontal & vertical axis
    int y0 = cvRound(minV*fscale);
    line(imageGraph, Point(b,h-(b-y0)), Point(w-b, h-(b-y0)), BLACK);
    line(imageGraph, Point(b,h-(b)), Point(b, h-(b+s)), BLACK);

    // Write the scale of the y axis
    if (showScale) {
        Scalar clr = GREY;
        char text[16];
        sprintf(text, "%.1f", maxV);
        putText(imageGraph, text, Point(1, b+4), FONT_HERSHEY_PLAIN, 0.55, clr);
        // Write the scale of the x axis
        sprintf(text, "%d", (nArrayLength-1) );
        putText(imageGraph, text, Point(w-b+4-5*strlen(text), (h/2)+10), FONT_HERSHEY_PLAIN, 0.55, clr);
    }

    // Draw the values
    Point ptPrev = Point(b,h-(b-y0));	// Start the lines at the 1st point.
    for (int i=0; i<nArrayLength; i++) {
        int y = cvRound((arraySrc[i] - minV) * fscale);	// Get the values at a bigger scale
        int x = cvRound(i * xscale);
        Point ptNew = Point(b+x, h-(b+y));
        line(imageGraph, ptPrev, ptNew, colorGraph, 1, LINE_AA);	// Draw a line from the previous point to the new point
        ptPrev = ptNew;
    }

    // Write the graph label, if desired
    if (graphLabel != NULL && strlen(graphLabel) > 0) {
        putText(imageGraph, graphLabel, Point(30, 10), FONT_HERSHEY_PLAIN, 0.55, Scalar(0,0,0));	// black text
    }

    return imageGraph;
}

// Display a graph of the given float array.
// If background is provided, it will be drawn into, for combining multiple graphs using drawFloatGraph().
// Set delay_ms to 0 if you want to wait forever until a keypress, or set it to 1 if you want it to delay just 1 millisecond.
void showFloatGraph(const char *name, const float *arraySrc, int nArrayLength, int delay_ms, Mat background)
{
#ifdef USE_HIGHGUI
    // Draw the graph
    Mat imageGraph = drawFloatGraph(arraySrc, nArrayLength, background);

    // Display the graph into a window
    imshow(name, imageGraph);

    waitKey(10);		// Note that waitKey() is required for the OpenCV window to show!
    waitKey(delay_ms);	// Wait longer to make sure the user has seen the graph
#endif
}

// Display a graph of the given int array.
// If background is provided, it will be drawn into, for combining multiple graphs using drawIntGraph().
// Set delay_ms to 0 if you want to wait forever until a keypress, or set it to 1 if you want it to delay just 1 millisecond.
void showIntGraph(const char *name, const int *arraySrc, int nArrayLength, int delay_ms, Mat background)
{
#ifdef USE_HIGHGUI
    // Draw the graph
    Mat imageGraph = drawIntGraph(arraySrc, nArrayLength, background);

    // Display the graph into a window
    imshow(name, imageGraph);

    waitKey(10);		// Note that waitKey() is required for the OpenCV window to show!
    waitKey(delay_ms);	// Wait longer to make sure the user has seen the graph
#endif
}

// Display a graph of the given unsigned char array.
// If background is provided, it will be drawn into, for combining multiple graphs using drawUCharGraph().
// Set delay_ms to 0 if you want to wait forever until a keypress, or set it to 1 if you want it to delay just 1 millisecond.
void showUCharGraph(const char *name, const uchar *arraySrc, int nArrayLength, int delay_ms, Mat background)
{
#ifdef USE_HIGHGUI
    // Draw the graph
    Mat imageGraph = drawUCharGraph(arraySrc, nArrayLength, background);

    // Display the graph into a window
    imshow(name, imageGraph);

    waitKey(10);		// Note that waitKey() is required for the OpenCV window to show!
    waitKey(delay_ms);	// Wait longer to make sure the user has seen the graph
#endif
}

// Simple helper function to easily view an image, with an optional pause.
void showImage(const Mat &img, int delay_ms, char *name)
{
#ifdef USE_HIGHGUI
    if (!name)
        name = "Image";
    imshow(name, img);
    waitKey(delay_ms);
#endif
}
