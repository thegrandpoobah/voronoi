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

#include "cpustippler.h"

#ifdef WIN32
#undef min
#undef max
#endif // WIN32

#include <SDL.h>

#include <GL/glu.h>

#include "utility.h"

CPUStippler::CPUStippler( std::string &image_path, const unsigned int points )
: AbstractStippler( image_path, points ) {
	framebuffer = new unsigned char[tileWidth*tileHeight*4];
}

CPUStippler::~CPUStippler() {
	delete[] framebuffer;
}

#ifdef OUTPUT_TILE
#include "lodepng.h"
#endif // OUTPUT_TILE

void CPUStippler::renderCell( EdgeMap::iterator &cell, const AbstractStippler::extents &extent ) {
	AbstractStippler::renderCell( cell, extent );

	::glReadBuffer( GL_BACK );
	::glReadPixels( 0, 0, tileWidth, tileHeight, GL_RGBA, GL_UNSIGNED_BYTE, framebuffer );

#ifdef OUTPUT_TILE
	std::vector<unsigned char> out;
	LodePNG::encode( out, framebuffer, tileWidth, tileHeight );
	LodePNG::saveFile( out, std::string( "tile.png" ) );
#endif // OUTPUT_TILE
}

std::pair< Point<float>, float > CPUStippler::calculateCellCentroid( const AbstractStippler::extents &extent ) {
	unsigned char *fbPtr = framebuffer;

	float areaDensity = 0.0;
	float xSum = 0.0;
	float ySum = 0.0;

	float xStep = ( extent.maxX - extent.minX ) / (float)tileWidth;
	float yStep = ( extent.maxY - extent.minY ) / (float)tileHeight;
	float xCurrent;
	float yCurrent;

	float area = 0.0f;
	float subarea = xStep * yStep;

	yCurrent = extent.minY;
	for ( unsigned int y = 0; y < tileHeight; ++y, yCurrent+=yStep ) {
		xCurrent = extent.minX;
		for ( unsigned int x = 0; x < tileWidth; ++x, xCurrent += xStep ) {
			float density = (float)(*fbPtr)/255.0f;
			fbPtr+=4;

			if ( density < getIntensityThreshold() ) { 
				continue;
			}

			area += subarea * density;
			areaDensity += density;
			xSum += density * xCurrent;
			ySum += density * ( extent.maxY - (yCurrent - extent.minY) );
		}
	}

	Point<float> pt;
	pt.x = xSum / areaDensity;
	pt.y = ySum / areaDensity;

	return std::make_pair( pt, area );
}
