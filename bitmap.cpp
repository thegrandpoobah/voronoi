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

Bitmap::Bitmap( std::string &filename ) : 
glWidth(1), glHeight(1), glTex(0) {
	file = PNG::load( filename );

	while ( glWidth < getWidth() || glHeight < getHeight() ) {
		if ( glWidth < getWidth() ) {
			glWidth *= 2;
		}
		if ( glHeight < getHeight() ) {
			glHeight *= 2;
		}
	}
}

Bitmap::~Bitmap() {
	if ( glTex != 0 ) {
		::glDeleteTextures( 1, &glTex );
	}
	PNG::freePng( file );
}

double Bitmap::getIntensity( unsigned int x, unsigned int y ) {
	return (double)getDiscreteIntensity(x,y)/255.0;
}

unsigned char Bitmap::getDiscreteIntensity( unsigned int x, unsigned int y ) {
	unsigned char r, g, b;

	getColour(x, y, r, g, b);

	return 255 - (unsigned char)std::ceil(((double)r * 0.2126 + (double)g * 0.7152 + (double)b * 0.0722));
}

void Bitmap::getColour( unsigned int x, unsigned int y, unsigned char &r, unsigned char &g, unsigned char &b ) {
	unsigned char *dataPtr = file->data + ( (y * file->w + x) * 4 );
	r = *(dataPtr);
	g = *(dataPtr+1);
	b = *(dataPtr+2);
}

unsigned int Bitmap::getWidth() {
	return file->w;
}

unsigned int Bitmap::getHeight() {
	return file->h;
}

unsigned int Bitmap::getGLWidth() {
	return glWidth;
}

unsigned int Bitmap::getGLHeight() {
	return glHeight;
}

GLuint Bitmap::createGLTexture( ) {
	if ( glTex == 0 ) {
		// create an appropriately sized texture (must be powers of 2)
		unsigned int texSizeW = getGLWidth();
		unsigned int texSizeH = getGLHeight();

		unsigned char *target = new unsigned char[texSizeW*texSizeH];
		::memset( target, 0, sizeof( unsigned char ) * texSizeW * texSizeH );
		unsigned char *targetPtr = target;

		for ( unsigned int y = 0; y < getHeight(); ++y ) {
			for ( unsigned int x = 0; x < getWidth(); ++x ) {
				*targetPtr = getDiscreteIntensity( x, y );
				targetPtr++;
			}
			targetPtr += texSizeW - getWidth();
		}

		::glGenTextures( 1, &glTex );
		::glBindTexture( GL_TEXTURE_2D, glTex );
		::glTexImage2D( GL_TEXTURE_2D, 0, GL_LUMINANCE8, texSizeW, texSizeH, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, target );
		::glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
		::glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

		delete[] target;
	}
	return glTex;
}

