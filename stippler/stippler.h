/* The MIT License

Copyright (c) 2011 Sahab Yazdani

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#ifndef STIPPLER_H
#define STIPPLER_H

#ifdef _WIN32
#ifdef STIPPLER_LIBRARY
#define STIPPLER_METHOD __declspec(dllexport)
#else
#define STIPPLER_METHOD __declspec(dllimport)
#endif
#else
#define STIPPLER_METHOD
#endif

#ifdef __cplusplus
extern "C" {
#endif 

typedef void * STIPPLER_HANDLE;

struct StipplingParameters {
	char *inputFile;
	unsigned int points;
	bool noOverlap;
	unsigned int subpixels;
};

struct StipplePoint {
	float x;
	float y;
	float radius;
	unsigned char r;
	unsigned char g;
	unsigned char b;
};

STIPPLER_METHOD void stippler_lib_init();
STIPPLER_METHOD void stippler_lib_destroy();

STIPPLER_METHOD STIPPLER_HANDLE create_stippler( StipplingParameters *parameters );
STIPPLER_METHOD void destroy_stippler( STIPPLER_HANDLE handle );

STIPPLER_METHOD void stippler_distribute( STIPPLER_HANDLE handle );
STIPPLER_METHOD float stippler_getAverageDisplacement( STIPPLER_HANDLE handle );
STIPPLER_METHOD void stippler_getStipples( STIPPLER_HANDLE handle, StipplePoint *dst );

STIPPLER_METHOD const char *stippler_getLastError();

#ifdef __cplusplus
}
#endif

#endif // STIPPLER_H