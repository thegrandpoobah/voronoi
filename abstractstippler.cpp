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

#include "abstractstippler.h"

#include <GL/glu.h>

#include <fstream>
#include <sstream>
#include <limits>

#include <boost/random.hpp>

#include "VoronoiDiagramGenerator.h"

#ifdef WIN32
#undef min
#undef max
#endif

CPUStippler::CPUStippler( std::string &image_path, const unsigned int points )
: IStippler(),
points(points),
tileWidth(128), tileHeight(128),
displacement(std::numeric_limits<float>::max()),
vertsX(new float[points]), vertsY(new float[points]), radii(new float[points]),
image(image_path), _useColour(false),
dl_circle(0) {
	framebuffer = new unsigned char[tileWidth*tileHeight*4];

	createInitialDistribution();
	createCircleDisplayList();
}

CPUStippler::~CPUStippler() {
	::glDeleteLists( dl_circle, 1 );

	delete[] framebuffer;
	delete[] radii;
	delete[] vertsX;
	delete[] vertsY;
}

void CPUStippler::useColour() {
	_useColour = true;
}

void CPUStippler::distribute() {
	createVoronoiDiagram();
	redistributeStipples();
}

void CPUStippler::paint() {
	// remember what the viewport used to look like
	GLint vp[4];
	::glGetIntegerv( GL_VIEWPORT, vp );
	GLint vpWidth = vp[2], vpHeight = vp[3];
	float aspect = (float)vpWidth/(float)vpHeight;
	::glDisable( GL_DEPTH_TEST );

	// setup opengl state
	::glMatrixMode( GL_PROJECTION );
	::glLoadIdentity();

	// align everything so that it is centered
	float orthoWidth;
	float orthoHeight;
	if ( aspect < 1.0f ) {
		orthoWidth = (float)image.getWidth();
		orthoHeight = (float)image.getWidth()*aspect;
	} else {
		orthoWidth = (float)image.getHeight()*1.0f/aspect;
		orthoHeight = (float)image.getHeight();
	}
	float left = orthoWidth - (float)image.getWidth(); // >= 0
	float top = orthoHeight - (float)image.getHeight(); // >= 0
	::gluOrtho2D( -left/2.0f, orthoWidth - left/2.0f, orthoHeight - top/2.0f, -top/2.0f );

	::glMatrixMode( GL_MODELVIEW );
	::glLoadIdentity();

	::glMatrixMode( GL_TEXTURE );
	::glLoadIdentity();
	::glScaled( 1.0/(double)image.getGLWidth(), 1.0/(double)image.getGLHeight(), 1.0 );

	::glClearColor( 1.0, 1.0, 1.0, 0.0 );
	::glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	::glMatrixMode( GL_MODELVIEW );
	::glColor3d( 0.0, 0.0, 0.0 );

	unsigned char r = 0, g = 0, b = 0;
	for ( unsigned int i = 0; i < points; ++i ) {
		if (_useColour) {
			image.getColour((unsigned int)std::ceil(vertsX[i]), (unsigned int)std::ceil(vertsY[i]), r, g, b);
		}

		::glPushMatrix();
		::glTranslatef( vertsX[i], vertsY[i], 0.0f );
		::glScalef( radii[i], radii[i], 1.0f );
		::glColor3ub( r, g, b );
		::glCallList( dl_circle );
		::glPopMatrix();
	}

#ifdef RENDER_PREVIOUS_ITERATION
	::glColor3d( 0.0, 0.0, 1.0 );
	for ( EdgeMap::iterator key_iter = edges.begin(); key_iter != edges.end(); ++key_iter ) {
		::glPushMatrix();
		::glTranslatef( key_iter->first.x, key_iter->first.y, 0.0f );
		::glScalef( 2.0f, 2.0f, 1.0f );
		::glCallList( dl_circle );
		::glPopMatrix();
	}
#endif // RENDER_PREVIOUS_ITERATION

#ifdef RENDER_VORONOI_EDGES
	::glBegin( GL_LINES );
	::glColor3d( 0.0, 1.0, 0.0 );
	for ( EdgeMap::iterator key_iter = edges.begin(); key_iter != edges.end(); ++key_iter ) {
		for ( EdgeList::iterator value_iter = key_iter->second.begin(); value_iter != key_iter->second.end(); ++value_iter ) {
		::glVertex2f( value_iter->begin.x, value_iter->begin.y );
			::glVertex2f( value_iter->end.x, value_iter->end.y );
		}
	}
	::glEnd();
#endif // RENDER_VORONOI_EDGES

	::glEnable( GL_DEPTH_TEST );
}

float CPUStippler::getAverageDisplacement() {
	return displacement;
}

void CPUStippler::createInitialDistribution() {
	// find initial distribution
	boost::mt19937 rng;
	boost::uniform_01<boost::mt19937, float> generator( rng );

	for ( unsigned int i = 0; i < points; ) {
		float xC, yC;

		xC = generator() * (float)image.getWidth();
		yC = generator() * (float)image.getHeight();

		// do a nearest neighbour search on the vertices
		if ( generator() <= image.getIntensity( (unsigned int)xC, (unsigned int)yC ) ) {
			if ( image.getDiscreteIntensity( (unsigned int)xC, (unsigned int)yC ) > 25 ) {
				vertsX[i] = xC;
				vertsY[i] = yC;
				radii[i] = 0.0f;

				i++;
			}
		}
	}
}

void CPUStippler::render( std::string &output_path ) {
	using namespace std;

	unsigned char r = 0, g = 0, b = 0;

	ofstream outputStream( output_path.c_str() );

	if ( !outputStream.is_open() ) {
		stringstream s;
		s<< "Unable to open output file " << output_path;
		throw exception(s.str().c_str());
	}

	outputStream << "<?xml version=\"1.0\" ?>" << endl;
	outputStream << "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\" \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">" << endl;
	outputStream << "<svg width=\"" << image.getWidth() << "\" height=\"" << image.getHeight() << "\" version=\"1.1\" xmlns=\"http://www.w3.org/2000/svg\">" << endl;
	for ( unsigned int i = 0; i < points; ++i ) {

		if ( _useColour ) {
			image.getColour((unsigned int)std::ceil(vertsX[i]), (unsigned int)std::ceil(vertsY[i]), r, g, b);
		}

		outputStream << "<circle cx=\"" << vertsX[i] << "\" cy=\"" << vertsY[i] << "\" r=\"" << radii[i] << "\" fill=\"rgb(" << (int)r << "," << (int)g << "," << (int)b << ")\" />" << endl;
	}
//stroke=\"black\" stroke-width=\"1\" 
	outputStream << "</svg>" << endl;

	outputStream.close();
}

void CPUStippler::createCircleDisplayList() {
	static const GLint SEGMENTS = 32;

	if ( dl_circle == 0 ) {
		dl_circle = ::glGenLists( 1 );

		::glNewList( dl_circle, GL_COMPILE );

		GLUquadric *quadric = ::gluNewQuadric();
		::gluDisk( quadric, 0.0, 1.0, SEGMENTS, 1 );
		::gluDeleteQuadric( quadric );

		::glEndList();
	}
}

void CPUStippler::createVoronoiDiagram() {
	VoronoiDiagramGenerator generator;

	generator.generateVoronoi( vertsX, vertsY, points, 
		0.0f, (float)image.getWidth(), 0.0f, (float)image.getHeight(), 0.0f/*::sqrt(8.0f) + 0.1f*/ );

	edges.clear();

	Point< float > p1, p2;
	Edge< float > edge;

	generator.resetIterator();
	while ( generator.getNext( 
		edge.begin.x, edge.begin.y, edge.end.x, edge.end.y,
		p1.x, p1.y, p2.x, p2.y ) ) {

		if ( edge.begin == edge.end ) {
			continue;
		}

		if ( edges.find( p1 ) == edges.end() ) {
			edges[p1] = EdgeList();
		}
		if ( edges.find( p2 ) == edges.end() ) {
			edges[p2] = EdgeList();
		}
		edges[p1].push_back( edge );
		edges[p2].push_back( edge );
	}
}

void CPUStippler::redistributeStipples() {
	// remember what the viewport used to look like
	GLint params[4];
	::glGetIntegerv( GL_VIEWPORT, params );

	::glViewport( 0, 0, tileWidth, tileHeight );

	unsigned int j = 0;
	displacement = 0.0f;
	for ( EdgeMap::iterator key_iter = edges.begin(); key_iter != edges.end(); ++key_iter ) {
		extents extent = getCellExtents( key_iter );

		renderCell( key_iter, extent );
		std::pair< Point<float>, float > centroid = calculateCellCentroid( extent ); // must be implemented by derived classes

		float rad = ::sqrt( centroid.second / PI );
		if ( !_isnan( rad ) ) {
			radii[j] = rad;
		} else {
			radii[j] = 0.0f;
		}
		if ( !_isnan( centroid.first.x ) && !_isnan( centroid.first.y ) ) {
			displacement += ::sqrt( ::pow( key_iter->first.x - centroid.first.x, 2.0f ) + ::pow( key_iter->first.y - centroid.first.y, 2.0f ) );
			vertsX[j] = centroid.first.x;
			vertsY[j] = centroid.first.y;
		}

		j++;
	}

	displacement /= j; // average out the displacement

	// undo the changes we made to the viewport
	::glViewport( params[0], params[1], params[2], params[3] );
}

#ifdef OUTPUT_TILE
#include "lodepng.h"
#endif // OUTPUT_TILE

void CPUStippler::renderCell( EdgeMap::iterator &cell, const CPUStippler::extents &extent ) {
	// setup opengl state
	::glMatrixMode( GL_PROJECTION );
	::glLoadIdentity();
	::gluOrtho2D( extent.minX, extent.maxX, extent.maxY, extent.minY );

	::glMatrixMode( GL_MODELVIEW );
	::glLoadIdentity();

	::glClearColor(	0.0, 0.0, 0.0, 0.0 );
	::glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	::glDisable( GL_DEPTH_TEST );

	::glMatrixMode( GL_TEXTURE );
	::glLoadIdentity();
	::glScaled( 1.0/(double)image.getGLWidth(), 1.0/(double)image.getGLHeight(), 1.0 );

	// cover the entire colour buffer with the texture
	::glEnable( GL_TEXTURE_2D );
	::glBindTexture( GL_TEXTURE_2D, image.createGLTexture() );
	::glColor3d( 1.0, 1.0, 1.0 );

	::glBegin( GL_QUADS );
	::glTexCoord2f( extent.minX, extent.minY ); ::glVertex2f( extent.minX, extent.minY );
	::glTexCoord2f( extent.minX, extent.maxY ); ::glVertex2f( extent.minX, extent.maxY );
	::glTexCoord2f( extent.maxX, extent.maxY ); ::glVertex2f( extent.maxX, extent.maxY );
	::glTexCoord2f( extent.maxX, extent.minY ); ::glVertex2f( extent.maxX, extent.minY );
	::glEnd();
	::glDisable( GL_TEXTURE_2D );

	// now clip out the regions that don't belong to this centroid
	::glEnable( GL_CLIP_PLANE0 );
	for ( EdgeList::iterator value_iter = cell->second.begin(); value_iter != cell->second.end(); ++value_iter ) {
		// create equation of a plane
		double x1, y1, z1, x2, y2, z2, x3, y3, z3;

		x1 = (double)value_iter->begin.x;
		y1 = (double)value_iter->begin.y;
		z1 = 0.0;

		x2 = (double)value_iter->end.x;
		y2 = (double)value_iter->end.y;
		z2 = 0.0;

		x3 = (double)value_iter->end.x;
		y3 = (double)value_iter->end.y;
		z3 = 1.0;

		double plane[4];

		plane[0] = y1 * (z2 - z3) + y2 * (z3 - z1) + y3 * (z1 - z2) ;
		plane[1] = z1 * (x2 - x3) + z2 * (x3 - x1) + z3 * (x1 - x2) ;
		plane[2] = x1 * (y2 - y3) + x2 * (y3 - y1) + x3 * (y1 - y2) ;
		plane[3] = -( x1 * (y2 * z3 - y3 * z2) + x2 * (y3 * z1 - y1 * z3) + x3 * (y1 * z2 - y2 * z1) );

		// make sure the pixel falls on the right side of the clipping plane
		if ( cell->first.x * plane[0] + cell->first.y * plane[1] + plane[3] > 0 ) {
			plane[0] *= -1;
			plane[1] *= -1;
			plane[2] *= -1;
			plane[3] *= -1;
		}

		// enable the clip plane
		::glClipPlane( GL_CLIP_PLANE0, plane );

		// write out 0 to entire surface, note that clipping will kill most of it
		::glColor4d( 0.0, 0.0, 1.0, 1.0 );
		::glBegin( GL_QUADS );
		::glVertex2f( extent.minX, extent.minY );
		::glVertex2f( extent.minX, extent.maxY );
		::glVertex2f( extent.maxX, extent.maxY );
		::glVertex2f( extent.maxX, extent.minY );
		::glEnd();
	}
	::glDisable( GL_CLIP_PLANE0 );

#ifdef _DEBUG
	::glColor4d( 0.0, 1.0, 1.0, 1.0 );
	::glBegin( GL_LINES );
	for ( EdgeList::iterator value_iter = cell->second.begin(); value_iter != cell->second.end(); ++value_iter ) {
		::glVertex2f( value_iter->begin.x, value_iter->begin.y );
		::glVertex2f( value_iter->end.x, value_iter->end.y );
	}
	::glEnd();
#endif // _DEBUG

	::glEnable( GL_DEPTH_TEST );

	::glReadBuffer( GL_BACK );
	::glReadPixels( 0, 0, tileWidth, tileHeight, GL_RGBA, GL_UNSIGNED_BYTE, framebuffer );

#ifdef OUTPUT_TILE
	std::vector<unsigned char> out;
	LodePNG::encode( out, framebuffer, tileWidth, tileHeight );
	LodePNG::saveFile( out, std::string( "tile.png" ) );
#endif // OUTPUT_TILE
}

std::pair< Point<float>, float > CPUStippler::calculateCellCentroid( const CPUStippler::extents &extent ) {
	unsigned char *fbPtr = framebuffer;

	float area = 0.0f, areaDensity = 0.0f;
	float xSum = 0.0f;
	float ySum = 0.0f;

	float xStep = ( extent.maxX - extent.minX ) / (float)tileWidth;
	float yStep = ( extent.maxY - extent.minY ) / (float)tileHeight;
	float xCurrent;
	float yCurrent;

	yCurrent = extent.minY;
	for ( unsigned int y = 0; y < tileHeight; ++y, yCurrent+=yStep ) {
		xCurrent = extent.minX;
		for ( unsigned int x = 0; x < tileWidth; ++x, xCurrent += xStep ) {
			float density = (float)(*fbPtr);
			fbPtr+=4;

			areaDensity += density;
			xSum += density * xCurrent;
			ySum += density * ( extent.maxY - ( yCurrent - extent.minY ) );
		}
	}

	area = areaDensity * xStep * yStep;

	Point<float> pt;
	pt.x = xSum / areaDensity;
	pt.y = ySum / areaDensity;

	return std::make_pair( pt, area / 255.0f );
}

CPUStippler::extents CPUStippler::getCellExtents( EdgeMap::iterator &cell ) {
	using std::numeric_limits;

	extents extent;

	extent.minX = extent.minY = numeric_limits<float>::max();
	extent.maxX = extent.maxY = numeric_limits<float>::min();

	for ( EdgeList::iterator value_iter = cell->second.begin(); value_iter != cell->second.end(); ++value_iter ) {
		if ( value_iter->begin.x < extent.minX ) extent.minX = value_iter->begin.x;
		if ( value_iter->end.x < extent.minX ) extent.minX = value_iter->end.x;
		if ( value_iter->begin.y < extent.minY ) extent.minY = value_iter->begin.y;
		if ( value_iter->end.y < extent.minY ) extent.minY = value_iter->end.y;

		if ( value_iter->begin.x > extent.maxX ) extent.maxX = value_iter->begin.x;
		if ( value_iter->end.x > extent.maxX ) extent.maxX = value_iter->end.x;
		if ( value_iter->begin.y > extent.maxY ) extent.maxY = value_iter->begin.y;
		if ( value_iter->end.y > extent.maxY ) extent.maxY = value_iter->end.y;
	}

	return extent;
}

bool operator==(Point<float> const& p1, Point<float> const& p2)
{
	using std::abs;
	using std::numeric_limits;

	return abs( p1.x - p2.x ) < numeric_limits<float>::epsilon() && 
		abs( p1.y - p2.y ) < numeric_limits<float>::epsilon();
}

std::size_t hash_value(Point<float> const& p) {
	using std::size_t;
	using boost::hash_combine;

    size_t seed = 0;

    hash_combine(seed, p.x);
    hash_combine(seed, p.y);

    return seed;
}
