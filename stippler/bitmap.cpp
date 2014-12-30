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

#include <cstdlib>
#include <cmath>

#include "bitmap.h"

Bitmap::Bitmap( std::string filename ) {
	using std::ceil;

	file = PNG::load( filename );

	intensityMap = new unsigned char[file->w * file->h];
	unsigned char *imPtr = intensityMap, *cPtr = file->data;

	for (unsigned int y = 0; y < file->h; y++) {
		for (unsigned int x = 0; x < file->w; x++, imPtr++, cPtr+=4) {
			*imPtr = 255 - (unsigned char)ceil(((float)(*(cPtr)) * 0.2126 + (float)(*(cPtr+1)) * 0.7152 + (float)(*(cPtr+2)) * 0.0722));
		}
	}
}

Bitmap::~Bitmap() {
	PNG::freePng( file );
	delete[] intensityMap;
}

float Bitmap::getIntensity( float x, float y ) {
	using std::floor;

	// from wikipedia 
	unsigned char *iMPtr = intensityMap + (unsigned int)floor(y) * file->w + (unsigned int)floor(x);
	float fX = x - floor(x), fY = y - floor(y);
	
	return 
		(float)(*(iMPtr)) * (1 - fX) * (1 - fY) + 
		(float)(*(iMPtr + 1)) * fX * (1 - fY) +
		(float)(*(iMPtr + file->w)) * (1 - fX) * fY +
		(float)(*(iMPtr + file->w + 1)) * fX * fY;
}

void Bitmap::getColour( float x, float y, unsigned char &r, unsigned char &g, unsigned char &b ) {
	using std::floor;

	float fX = x - floor(x), fY = y - floor(y);
	float f00 = (1 - fX) * (1 - fY),
		f10 = fX * (1 - fY),
		f01 = (1 - fX) * fY,
		f11 = fX * fY;

	unsigned char *dataPtr = file->data + (((unsigned int)floor(y) * file->w + (unsigned int)floor(x)) * 4);
	r = (unsigned char)floor((float)(*(dataPtr)) * f00 + 
		(float)(*(dataPtr + 4)) * f10 +
		(float)(*(dataPtr + file->w * 4)) * f01 +
		(float)(*(dataPtr + file->w * 4 + 4)) * f11);

	dataPtr++;
	g = (unsigned char)floor((float)(*(dataPtr)) * f00 + 
		(float)(*(dataPtr + 4)) * f10 +
		(float)(*(dataPtr + file->w * 4)) * f01 +
		(float)(*(dataPtr + file->w * 4 + 4)) * f11);

	dataPtr++;
	b = (unsigned char)floor((float)(*(dataPtr)) * f00 + 
		(float)(*(dataPtr + 4)) * f10 +
		(float)(*(dataPtr + file->w * 4)) * f01 +
		(float)(*(dataPtr + file->w * 4 + 4)) * f11);
}

unsigned int Bitmap::getWidth() {
	return file->w;
}

unsigned int Bitmap::getHeight() {
	return file->h;
}

