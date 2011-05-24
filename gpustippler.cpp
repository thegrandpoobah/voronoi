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

#include "GLee/GLee.h"
#include "gpustippler.h"

#ifdef WIN32
#undef max
#undef min
#endif

#include <cmath>
#include <fstream>
#include <limits>

#include <SDL.h>

#include <GL/glu.h>

#include <boost/random.hpp>

#include "glslprogram.h"

GPUStippler::GPUStippler( std::string &image_path, const unsigned int points )
: AbstractStippler(image_path, points) {
	using std::exception;

	if ( !GL_ARB_texture_rectangle ) {
		throw exception();
	}

	createCellBuffer();
	createCalculationBuffer();
	initializeGLSL();
}

GPUStippler::~GPUStippler() {
	delete dataMappingShader;
	delete summingShader;
}

// creates a frame buffer for drawing the volonoi cell 
// this is essentially the same as preparing our computation data
void GPUStippler::createCellBuffer() {
	// allocate the unseen fbo
	::glGenFramebuffersEXT( 1, &cellFBO );

	::glGenTextures( 1, &cellTexture );
	::glBindTexture( GL_TEXTURE_2D, cellTexture );
	::glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	::glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	::glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
	::glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
	::glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	::glTexImage2D( GL_TEXTURE_2D, 0, 4, tileWidth, tileHeight, 0, GL_RGBA, GL_FLOAT, NULL );

	::glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, cellFBO );
	::glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, cellTexture, 0 );
	::glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, 0 );

	::glBindTexture( GL_TEXTURE_2D, 0 );
}

void GPUStippler::createCalculationBuffer() {
	// allocate the unseen fbo
	::glGenFramebuffersEXT( 1, &calculationFBO );

	::glGenTextures( 2, calculationTextures );

	::glBindTexture( GL_TEXTURE_RECTANGLE_ARB, calculationTextures[0] );
	::glTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	::glTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	::glTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP );
	::glTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP );
	::glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	::glTexImage2D( GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGBA32F_ARB, tileWidth, tileHeight, 0, GL_RGBA, GL_FLOAT, NULL );

	::glBindTexture( GL_TEXTURE_RECTANGLE_ARB, calculationTextures[1] );
	::glTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	::glTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	::glTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP );
	::glTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP );
	::glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	::glTexImage2D( GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGBA32F_ARB, tileWidth, tileHeight, 0, GL_RGBA, GL_FLOAT, NULL );

	::glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, calculationFBO );
	::glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, calculationTextures[0], 0 );
	::glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT1_EXT, GL_TEXTURE_2D, calculationTextures[1], 0 );
	::glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, 0 );

	::glBindTexture( GL_TEXTURE_RECTANGLE_ARB, 0 );
}

// adapted from http://bytes.com/topic/c/answers/63797-reading-file-into-string#post242475
std::string GPUStippler::readFile( const char *filename ) {
	using std::string;
	using std::ifstream;
	using std::getline;
	using std::exception;

	ifstream in( filename );

	if (!in) {
		throw exception();
	}

	string line, buf;
	while(getline(in,line)) {
		buf += line + "\n";
	}

	return buf;
}

void GPUStippler::initializeGLSL() {
	dataMappingShader = new GLSLProgram( readFile( "shaders/dataVertex.glsl" ),
		readFile( "shaders/dataFragment.glsl" ) );

	summingShader = new GLSLProgram( readFile( "shaders/summingVertex.glsl" ),
		readFile( "shaders/summingFragment.glsl" ) );
}

void GPUStippler::renderCell( EdgeMap::iterator &cell, const AbstractStippler::extents &extent ) {
	// make it so that OpenGL writes to the cell texture (data source)
	::glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, cellFBO );
	::glDrawBuffer( GL_COLOR_ATTACHMENT0_EXT );

	AbstractStippler::renderCell( cell, extent );

	// switch back to the normal framebuffer
	::glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, 0 );
	::glDrawBuffer( GL_BACK );
}

void GPUStippler::transferDataToReadBuffer( const AbstractStippler::extents &extent ) {
	// the "write" buffer
	// use it as a temporary store for the actual data
	::glDrawBuffer( GL_COLOR_ATTACHMENT1_EXT );

	// copy the cell image into the data read buffer
	::glMatrixMode( GL_PROJECTION );
	::glLoadIdentity();
	::gluOrtho2D( 0.0, 1.0, 0.0, 1.0 );

	::glMatrixMode( GL_MODELVIEW );
	::glLoadIdentity();

	::glMatrixMode( GL_TEXTURE );
	::glLoadIdentity();

	::glViewport( 0, 0, tileWidth, tileHeight );

	::glEnable( GL_TEXTURE_2D );
	::glBindTexture( GL_TEXTURE_2D, cellTexture );
	::glPolygonMode( GL_FRONT, GL_FILL );

	float sizeX = ::abs( extent.maxX - extent.minX );
	float sizeY = ::abs( extent.maxY - extent.minY );
	float stepX = sizeX / (float)tileWidth;
	float stepY = sizeY / (float)tileHeight;

	dataMappingShader->startProgram();
	::glUniform1i( dataMappingShader->getParameter( std::string( "texelUnit" ) ),
		0 );
	::glUniform1f( dataMappingShader->getParameter( std::string( "subarea" ) ), 
		stepX * stepY );
	::glUniform4f( dataMappingShader->getParameter( std::string( "extents" ) ),
		extent.minX, sizeX, extent.maxY, sizeY );

	::glBegin( GL_QUADS );
	::glVertex2i( 0, 0 );
	::glVertex2i( 0, 1 );
	::glVertex2i( 1, 1 );
	::glVertex2i( 1, 0 );
	::glEnd();

	::glBindTexture( GL_TEXTURE_2D, 0 );
	::glDisable( GL_TEXTURE_2D );

	dataMappingShader->endProgram();
}

std::pair< Point<float>, float > GPUStippler::calculateCellCentroid( const AbstractStippler::extents &extent ) {
	const GLenum attachmentPoints[2] = { 
		GL_COLOR_ATTACHMENT0_EXT,
		GL_COLOR_ATTACHMENT1_EXT };
	unsigned int writeTexture = 0, readTexture = 1;

	::glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, calculationFBO );

	transferDataToReadBuffer( extent );

	int numPasses = (int)(::log((double)tileWidth)/::log(2.0));

	summingShader->startProgram();
	::glUniform1i( summingShader->getParameter( std::string( "texelUnit" ) ), 0 );

	::glMatrixMode( GL_PROJECTION );
	::glLoadIdentity();
	::gluOrtho2D( 0.0, (double)(tileWidth), 0.0, (double)(tileHeight) );

	::glMatrixMode( GL_MODELVIEW );
	::glLoadIdentity();

	::glViewport( 0, 0, tileWidth, tileHeight );

	::glPolygonMode( GL_FRONT, GL_FILL );

	::glEnable( GL_TEXTURE_RECTANGLE_ARB );

	unsigned tileSize = tileWidth;
	for ( int i = 0; i < 7; ++i ) {
		tileSize /= 2;

		::glDrawBuffer( attachmentPoints[writeTexture] );

		::glBindTexture( GL_TEXTURE_RECTANGLE_ARB, calculationTextures[readTexture] );

		::glBegin( GL_QUADS );
		::glVertex2i( 0, 0 );
		::glVertex2i( 0, tileSize );
		::glVertex2i( tileSize, tileSize );
		::glVertex2i( tileSize, 0 );
		::glEnd();

		// do the ping pong step
		unsigned int tmp = writeTexture;
		writeTexture = readTexture;
		readTexture = tmp;
	}
	::glDisable( GL_TEXTURE_RECTANGLE_ARB );
	::glBindTexture( GL_TEXTURE_RECTANGLE_ARB, 0 );

	summingShader->endProgram();

	// now that the data has been calculated, we can actually use it
	float data[4]; // (running total, x direction accumulator, y direction accumulator, centroid area accumulator)
	::glReadBuffer( attachmentPoints[readTexture] );
	::glReadPixels( 0, 0, 1, 1, GL_RGBA, GL_FLOAT, (void *)data );

	::glDrawBuffer( GL_BACK );
	::glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, 0 );

	Point<float> centroid;

	centroid.x = data[1] / data[0]; 
	centroid.y = data[2] / data[0];

	return std::make_pair( centroid, data[3] );
}
