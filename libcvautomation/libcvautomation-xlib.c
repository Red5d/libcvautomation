/*
 * =====================================================================================
 *
 *       Filename:  libcvautomation-xlib.c
 *
 *    Description:  
 *
 *        Created:  06/21/2012 08:34:21 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Bradlee Speice, bspeice@uncc.edu
 *   Organization:  MOSAIC at University of North Carolina at Charlotte
 *
 * =====================================================================================
 */

#include <libcvautomation/libcvautomation-xlib.h>


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  cvaOpenDisplay
 *  Description:  Custom wrapper for XOpenDisplay function
 * =====================================================================================
 */
Display* cvaOpenDisplay ( char *displayName )
{
	return XOpenDisplay( displayName);
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  cvaCloseDisplay
 *  Description:  Custom wrapper for XOpenDisplay function
 * =====================================================================================
 */
void cvaCloseDisplay ( Display *displayLocation )
{
	XCloseDisplay( displayLocation );
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  matchSubImage_X11
 *  Description:  Match a sub image using the X11 root window as root
 * =====================================================================================
 */
cvaPoint matchSubImage_X11( Display *displayLocation, IplImage *subImage, int searchMethod, int tolerance )
{
	/* First things first, grab the root X window and convert it to
	 * the IplImage format.
	 * Much of this code was ripped lovingly from:
	 * 	http://opencv.willowgarage.com/wiki/ximage2opencvimage */
	IplImage *X_IPL;
	CvSize imageSize;
	cvaPoint resultPoint;

	XImage *rootImage;
	XColor color;
	Screen *screen;
	unsigned long rmask, gmask, bmask;
	unsigned long rshift, rbits,
					gshift, gbits,
					bshift, bbits;
	unsigned char colorChannel[3];

	int startX = 0, startY = 0;
	unsigned int width, height;

	/* Setting up the X screengrab is the first order of business */
	screen = DefaultScreenOfDisplay(displayLocation);
	
	width = screen->width;
	height = screen->height;

	rootImage = XGetImage( displayLocation, DefaultRootWindow(displayLocation),
							startX, startY, width, height,
							AllPlanes, ZPixmap );

	/* Make sure that we're good to go before going any farther */
	if ( rootImage == NULL || displayLocation == NULL || screen == NULL )
	{
		fprintf( stderr, "Couldn't grab the root window!" );
		resultPoint.x = -1;
		resultPoint.y = -1;
		return resultPoint;
	}

	/* Set up the OpenCV Image */
	imageSize.width = rootImage->width;
	imageSize.height = rootImage->height;
	X_IPL = cvCreateImage( imageSize, IPL_DEPTH_8U, 3 ); /* 3 channels - RGB */

	/* This if block converts the X root window to an IPL image. See you on the other side! */
	unsigned int x, y; /* To be used later */

	if ( screen->depths->depth == 24 )
	{
		/* Actually convert the XImage to Ipl */
		rmask = screen->root_visual->red_mask;
		gmask = screen->root_visual->green_mask;
		bmask = screen->root_visual->blue_mask;

		/* I honestly have no clue how most of the below code works */
		/* TODO: Figure out how this code works and document it */
		rshift = 0; rbits = 0;
		while ( !(rmask & 1) ){
			rshift++;
			rmask >>= 1; /* Right bitshift by 1 */
		}
		while (rmask & 1) {
			rbits++;
			rmask >>= 1; /* Right bitshift by 1 */
		}
		if (rbits > 8) {
			rshift += rbits - 8;
			rbits = 8;
		}

		gshift = 0; gbits = 0;
		while ( !(gmask & 1) ){
			gshift++;
			gmask >>= 1; /* Right bitshift by 1 */
		}
		while (gmask & 1) {
			gbits++;
			gmask >>= 1; /* Right bitshift by 1 */
		}
		if (gbits > 8) {
			gshift += gbits - 8;
			gbits = 8;
		}

		bshift = 0; bbits = 0;
		while ( !(bmask & 1) ){
			bshift++;
			bmask >>= 1; /* Right bitshift by 1 */
		}
		while (bmask & 1) {
			bbits++;
			bmask >>= 1; /* Right bitshift by 1 */
		}
		if (bbits > 8) {
			bshift += bbits - 8;
			bbits = 8;
		}

		for ( x = 0; x < rootImage->width; x++) {
			for ( y = 0; y < rootImage->height; y++) {
				color.pixel = XGetPixel ( rootImage, x, y );
				colorChannel[0] = ((color.pixel >> bshift) & ((1 << bbits) - 1)) << (8 - bbits);
				colorChannel[1] = ((color.pixel >> gshift) & ((1 << gbits) - 1)) << (8 - gbits);
				colorChannel[2] = ((color.pixel >> rshift) & ((1 << rbits) - 1)) << (8 - rbits);
				CV_IMAGE_ELEM(X_IPL, uchar, y, x * X_IPL->nChannels) = colorChannel[0];
				CV_IMAGE_ELEM(X_IPL, uchar, y, x * X_IPL->nChannels + 1) = colorChannel[1];
				CV_IMAGE_ELEM(X_IPL, uchar, y, x * X_IPL->nChannels + 2) = colorChannel[2];
			}
		}
	}
	else
	{
		/* Slow alternative for non-24-bit-depth images */
		/* I don't know how this works either. */
		/* TODO: Figure out how this code works and document it */
		Colormap colormap;
		colormap = DefaultColormap( displayLocation, DefaultScreen( displayLocation ));
		for ( x = 0; x < rootImage->width; x++ ) {
			for ( y = 0; y < rootImage->height; y++ ) {
				color.pixel = XGetPixel ( rootImage, x, y );
				XQueryColor( displayLocation, colormap, &color );
				CV_IMAGE_ELEM(X_IPL, uchar, y, x * X_IPL->nChannels) = color.blue;
				CV_IMAGE_ELEM(X_IPL, uchar, y, x * X_IPL->nChannels + 1) = color.green;
				CV_IMAGE_ELEM(X_IPL, uchar, y, x * X_IPL->nChannels + 2) = color.red;
			}
		}
	}

	/* Now that we've got the image we want as X_IPL, time to actually do the comparison.
	 * However, we don't want to do any more work than we have to - send our images off
	 * to matchSubImage in libopencvautomation-opencv. */

	resultPoint = matchSubImage ( X_IPL, subImage, searchMethod, tolerance );

	/* Clean up the CV image we created, as well as all X resources */
	XDestroyImage( rootImage );
	cvReleaseImage( &X_IPL );

	/* Return and be done */
	return resultPoint;

}		/* -----  end of function matchSubImage_X11  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  matchSubImage_X11_location
 *  Description:  Match a sub image using the X11 root window as root, from filename
 * =====================================================================================
 */
cvaPoint matchSubImage_X11_location( Display *displayLocation, const char *subImage_location, int searchMethod, int tolerance )
{
	/* This is basically a wrapper for matchSubImage_X11( char *display, IplImage )
	 * All we do is load the sub-image from the given filename, and then
	 * pass off the result to the above-named function */

	IplImage *subImage;
	subImage = cvLoadImage( subImage_location, CV_LOAD_IMAGE_COLOR );

	cvaPoint resultPoint;
	resultPoint.x = -1;
	resultPoint.y = -1;

	/* Make sure we have a good image */
	if ( subImage == 0 )
	{
		/* Return error */
		return resultPoint;
	}

	resultPoint = matchSubImage_X11( displayLocation, subImage, searchMethod, tolerance );

	/* Free up the memory we created */
	cvReleaseImage( &subImage );

	/* Our resultPoint will already be bad if there's no match,
	 * we don't need to worry about setting it. */
	return resultPoint;

}		/* -----  end of function matchSubImage_X11_location  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  matchSubImage_X11_center
 *  Description:  Match a sub image using the X11 root window as root
 *  				The code is mostly the same as matchSubImage_X11, but use
 *  				matchSubImage_center to get the center of the result, rather than
 *  				the top-left corner.
 * =====================================================================================
 */
cvaPoint matchSubImage_X11_center( Display *displayLocation, IplImage *subImage, int searchMethod, int tolerance )
{
	/* First things first, grab the root X window and convert it to
	 * the IplImage format.
	 * Much of this code was ripped lovingly from:
	 * 	http://opencv.willowgarage.com/wiki/ximage2opencvimage */
	IplImage *X_IPL;
	CvSize imageSize;
	cvaPoint resultPoint;

	XImage *rootImage;
	XColor color;
	Screen *screen;
	unsigned long rmask, gmask, bmask;
	unsigned long rshift, rbits,
					gshift, gbits,
					bshift, bbits;
	unsigned char colorChannel[3];

	int startX = 0, startY = 0;
	unsigned int width, height;

	/* Setting up the X screengrab is the first order of business */
	screen = DefaultScreenOfDisplay(displayLocation);
	
	width = screen->width;
	height = screen->height;

	rootImage = XGetImage( displayLocation, DefaultRootWindow(displayLocation),
							startX, startY, width, height,
							AllPlanes, ZPixmap );

	/* Make sure that we're good to go before going any farther */
	if ( rootImage == NULL || displayLocation == NULL || screen == NULL )
	{
		fprintf( stderr, "Couldn't grab the root window!" );
		resultPoint.x = -1;
		resultPoint.y = -1;
		return resultPoint;
	}

	/* Set up the OpenCV Image */
	imageSize.width = rootImage->width;
	imageSize.height = rootImage->height;
	X_IPL = cvCreateImage( imageSize, IPL_DEPTH_8U, 3 ); /* 3 channels - RGB */

	/* This if block converts the X root window to an IPL image. See you on the other side! */
	unsigned int x, y; /* To be used later */

	if ( screen->depths->depth == 24 )
	{
		/* Actually convert the XImage to Ipl */
		rmask = screen->root_visual->red_mask;
		gmask = screen->root_visual->green_mask;
		bmask = screen->root_visual->blue_mask;

		/* I honestly have no clue how most of the below code works */
		/* TODO: Figure out how this code works and document it */
		rshift = 0; rbits = 0;
		while ( !(rmask & 1) ){
			rshift++;
			rmask >>= 1; /* Right bitshift by 1 */
		}
		while (rmask & 1) {
			rbits++;
			rmask >>= 1; /* Right bitshift by 1 */
		}
		if (rbits > 8) {
			rshift += rbits - 8;
			rbits = 8;
		}

		gshift = 0; gbits = 0;
		while ( !(gmask & 1) ){
			gshift++;
			gmask >>= 1; /* Right bitshift by 1 */
		}
		while (gmask & 1) {
			gbits++;
			gmask >>= 1; /* Right bitshift by 1 */
		}
		if (gbits > 8) {
			gshift += gbits - 8;
			gbits = 8;
		}

		bshift = 0; bbits = 0;
		while ( !(bmask & 1) ){
			bshift++;
			bmask >>= 1; /* Right bitshift by 1 */
		}
		while (bmask & 1) {
			bbits++;
			bmask >>= 1; /* Right bitshift by 1 */
		}
		if (bbits > 8) {
			bshift += bbits - 8;
			bbits = 8;
		}

		for ( x = 0; x < rootImage->width; x++) {
			for ( y = 0; y < rootImage->height; y++) {
				color.pixel = XGetPixel ( rootImage, x, y );
				colorChannel[0] = ((color.pixel >> bshift) & ((1 << bbits) - 1)) << (8 - bbits);
				colorChannel[1] = ((color.pixel >> gshift) & ((1 << gbits) - 1)) << (8 - gbits);
				colorChannel[2] = ((color.pixel >> rshift) & ((1 << rbits) - 1)) << (8 - rbits);
				CV_IMAGE_ELEM(X_IPL, uchar, y, x * X_IPL->nChannels) = colorChannel[0];
				CV_IMAGE_ELEM(X_IPL, uchar, y, x * X_IPL->nChannels + 1) = colorChannel[1];
				CV_IMAGE_ELEM(X_IPL, uchar, y, x * X_IPL->nChannels + 2) = colorChannel[2];
			}
		}
	}
	else
	{
		/* Slow alternative for non-24-bit-depth images */
		/* I don't know how this works either. */
		/* TODO: Figure out how this code works and document it */
		Colormap colormap;
		colormap = DefaultColormap( displayLocation, DefaultScreen( displayLocation ));
		for ( x = 0; x < rootImage->width; x++ ) {
			for ( y = 0; y < rootImage->height; y++ ) {
				color.pixel = XGetPixel ( rootImage, x, y );
				XQueryColor( displayLocation, colormap, &color );
				CV_IMAGE_ELEM(X_IPL, uchar, y, x * X_IPL->nChannels) = color.blue;
				CV_IMAGE_ELEM(X_IPL, uchar, y, x * X_IPL->nChannels + 1) = color.green;
				CV_IMAGE_ELEM(X_IPL, uchar, y, x * X_IPL->nChannels + 2) = color.red;
			}
		}
	}

	/* Now that we've got the image we want as X_IPL, time to actually do the comparison.
	 * However, we don't want to do any more work than we have to - send our images off
	 * to matchSubImage in libopencvautomation-opencv. */

	resultPoint = matchSubImage_center( X_IPL, subImage, searchMethod, tolerance );

	/* Clean up the CV image we created, as well as all X resources */
	XDestroyImage( rootImage );
	cvReleaseImage( &X_IPL );

	/* Return and be done */
	return resultPoint;

}		/* -----  end of function matchSubImage_X11_center  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  matchSubImage_X11_location_center
 *  Description:  Match a sub image using the X11 root window as root, from filename
 * =====================================================================================
 */
cvaPoint matchSubImage_X11_location_center( Display *displayLocation, const char *subImage_location, int searchMethod, int tolerance )
{
	/* This is basically a wrapper for matchSubImage_X11( char *display, IplImage )
	 * All we do is load the sub-image from the given filename, and then
	 * pass off the result to the above-named function */

	IplImage *subImage;
	subImage = cvLoadImage( subImage_location, CV_LOAD_IMAGE_COLOR );

	cvaPoint resultPoint;
	resultPoint.x = -1;
	resultPoint.y = -1;

	/* Make sure we have a good image */
	if ( subImage == 0 )
	{
		/* Return error */
		return resultPoint;
	}

	resultPoint = matchSubImage_X11_center( displayLocation, subImage, searchMethod, tolerance );

	/* Free up the memory we created */
	cvReleaseImage( &subImage );

	/* Our resultPoint will already be bad if there's no match,
	 * we don't need to worry about setting it. */
	return resultPoint;

}		/* -----  end of function matchSubImage_X11_location_center  ----- */
