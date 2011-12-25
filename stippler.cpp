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

#include "stippler.h"

#include <GL/glu.h>

#include <fstream>
#include <sstream>
#include <limits>

#include <boost/random.hpp>

#include "VoronoiDiagramGenerator.h"

Stippler::Stippler( std::string &image_path, const StipplingParameters &parameters )
: IStippler(),
parameters(parameters),
tileWidth(128), tileHeight(128),
displacement(std::numeric_limits<float>::max()),
vertsX(new float[parameters.points]), vertsY(new float[parameters.points]), radii(new float[parameters.points]),
image(image_path),
dl_circle(0) {
	createInitialDistribution();
	createCircleDisplayList();
}

Stippler::~Stippler() {
	::glDeleteLists( dl_circle, 1 );

	delete[] radii;
	delete[] vertsX;
	delete[] vertsY;
}

void Stippler::distribute() {
	createVoronoiDiagram();
	redistributeStipples();
}

void Stippler::paint() {
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
	for ( unsigned int i = 0; i < parameters.points; ++i ) {
		if ( parameters.useColour ) {
			image.getColour( (unsigned int)std::ceil(vertsX[i]), (unsigned int)std::ceil(vertsY[i]), r, g, b );
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

float Stippler::getAverageDisplacement() {
	return displacement;
}

void Stippler::createInitialDistribution() {
	// find initial distribution
	boost::mt19937 rng;
	boost::uniform_01<boost::mt19937, float> generator( rng );

	for ( unsigned int i = 0; i < parameters.points; ) {
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

void Stippler::render( std::string &output_path ) {
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
	
	float radius;
	for ( unsigned int i = 0; i < parameters.points; ++i ) {
		if ( parameters.useColour ) {
			image.getColour((unsigned int)std::ceil(vertsX[i]), (unsigned int)std::ceil(vertsY[i]), r, g, b);
		}

		if ( parameters.fixedRadius ) {
			radius = 0.5f; // gives circles with 1px diameter
		} else {
			radius = radii[i];
		}
		radius *= parameters.sizingFactor;

		outputStream << "<circle cx=\"" << vertsX[i] << "\" cy=\"" << vertsY[i] << "\" r=\"" << radius << "\" fill=\"rgb(" << (int)r << "," << (int)g << "," << (int)b << ")\" />" << endl;
	}
//stroke=\"black\" stroke-width=\"1\" 
	outputStream << "</svg>" << endl;

	outputStream.close();
}

void Stippler::createCircleDisplayList() {
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

void Stippler::createVoronoiDiagram() {
	VoronoiDiagramGenerator generator;

	generator.generateVoronoi( vertsX, vertsY, parameters.points, 
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

void Stippler::redistributeStipples() {
	// remember what the viewport used to look like
	GLint params[4];
	::glGetIntegerv( GL_VIEWPORT, params );

	::glViewport( 0, 0, tileWidth, tileHeight );

	unsigned int j = 0;
	displacement = 0.0f;
	for ( EdgeMap::iterator key_iter = edges.begin(); key_iter != edges.end(); ++key_iter ) {
		extents extent = getCellExtents( key_iter );

		std::pair< Point<float>, float > centroid = calculateCellCentroid( key_iter, extent );

		float rad = centroid.second;
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

//#define OUTPUT_TILE
#ifdef OUTPUT_TILE
#include "lodepng.h"
unsigned int cellNumber = 0;
#endif // OUTPUT_TILE

inline void Stippler::createProjection( const Stippler::extents &extent, float *projection ) {
	float scaleX =(float)(tileWidth - 1)/(extent.maxX - extent.minX),
		scaleY = (float)(tileHeight - 1)/(extent.maxY - extent.minY);
	
	projection[0] = scaleX;
	projection[1] = 0.0f;
	projection[2] = -extent.minX * scaleX;
	projection[3] = 0.0f;
	projection[4] = scaleY;
	projection[5] = -extent.minY * scaleY;
	projection[6] = 0.0f;
	projection[7] = 0.0f;
	projection[8] = 1.0f;
}

inline Stippler::line Stippler::createClipLine( const float projection[9], float FinsideX, float FinsideY, float Fx1, float Fy1, float Fx2, float Fy2 ) {
	using std::floor;
	using std::abs;

	int x1 = (int)floor(Fx1 * projection[0] + Fy1 * projection[1] + projection[2] + 0.5f),
		y1 = (int)floor(Fx1 * projection[3] + Fy1 * projection[4] + projection[5] + 0.5f),
		x2 = (int)floor(Fx2 * projection[0] + Fy2 * projection[1] + projection[2] + 0.5f),
		y2 = (int)floor(Fx2 * projection[3] + Fy2 * projection[4] + projection[5] + 0.5f),
		insideX = (int)floor(FinsideX * projection[0] + FinsideY * projection[1] + projection[2] + 0.5f),
		insideY = (int)floor(FinsideX * projection[3] + FinsideY * projection[4] + projection[5] + 0.5f);

	Stippler::line l;

	// if the floating point version of the line collapsed down to one
	// point, then just ignore it all
	if (x1 - x2 == 0 && y1 - y2 ==0) {
		l.a = 0;
		l.b = 0;
		l.c = 0;

		return l;
	}

	l.a = -(y1 - y2);
	l.b = x1 - x2;
	l.c = (y1 - y2) * x1 - (x1 - x2) * y1;

	// make sure the pixel falls on the correct side of the clipping plane
	if ( insideX * l.a + insideY * l.b + l.c > 0 ) {
		l.a *= -1;
		l.b *= -1;
		l.c *= -1;
	}

	return l;
}

std::pair< Point<float>, float > Stippler::calculateCellCentroid( EdgeMap::iterator &cell, const Stippler::extents &extent ) {
	using std::make_pair;
	using std::numeric_limits;
	using std::vector;

	float projection[9];
	
	vector<line> clipLines;

	int x, y;

	float xStep = ( extent.maxX - extent.minX ) / (float)tileWidth;
	float yStep = ( extent.maxY - extent.minY ) / (float)tileHeight;

	float spotDensity, areaDensity = 0.0f, maxAreaDensity = 0.0f;
	float xSum = 0.0f;
	float ySum = 0.0f;

	float xCurrent;
	float yCurrent;

	createProjection(extent, projection);
	
	// compute the clip lines
	for ( EdgeList::iterator value_iter = cell->second.begin(); value_iter != cell->second.end(); ++value_iter ) {
		line l = createClipLine( projection, 
			cell->first.x, cell->first.y, 
			value_iter->begin.x, value_iter->begin.y,
			value_iter->end.x, value_iter->end.y );
	
		if (l.a == 0 && l.b == 0) {
			continue;
		}

		clipLines.push_back(l);
	}

	for ( y = 0, yCurrent = extent.minY; y < tileHeight; ++y, yCurrent += yStep ) {
		for ( x = 0, xCurrent = extent.minX; x < tileWidth; ++x, xCurrent += xStep ) {
			// something is outside of the polygon, if a point is outside of all clipping planes
			bool outside = false;
			for ( vector<line>::iterator iter = clipLines.begin(); iter != clipLines.end(); iter++ ) {
				if ( x * iter->a + y * iter->b + iter->c >= 0 ) {
					outside = true;
					break;
				}
			}

			if (!outside) {
				spotDensity = (float)image.getDiscreteIntensity(std::floor(xCurrent), std::floor(yCurrent));

				areaDensity += spotDensity;
				maxAreaDensity += 255.0f;
				xSum += spotDensity * xCurrent;
				ySum += spotDensity * yCurrent;
			}
		}
	}

	float area = areaDensity * xStep * yStep / 255.0f;
	float maxArea = maxAreaDensity * xStep * yStep / 255.0f;

	Point<float> pt;
	pt.x = xSum / areaDensity;
	pt.y = ySum / areaDensity;

	float closest = numeric_limits<float>::max(),
		  farthest = numeric_limits<float>::min(),
		  distance;
	float x0 = pt.x, y0 = pt.y,
	      x1, x2, y1, y2;

	for ( EdgeList::iterator value_iter = cell->second.begin(); value_iter != cell->second.end(); ++value_iter ) {
		x1 = value_iter->begin.x; x2 = value_iter->end.x;
		y1 = value_iter->begin.y; y2 = value_iter->end.y;

		distance = ::abs( ( x2 - x1 ) * ( y1 - y0 ) - ( x1 - x0 ) * ( y2 - y1 ) ) / ::sqrt( ::pow( x2 - x1, 2.0f ) + ::pow( y2 - y1, 2.0f ) );
		if ( closest > distance ) {
			closest = distance;
		}
		if ( farthest < distance ) {
			farthest = distance;
		}
	}

	float radius;
	if ( parameters.noOverlap ) {
		radius = closest;
	} else {
		radius = farthest;
	}
	radius *= area / maxArea;

	return make_pair( pt, radius );
}

Stippler::extents Stippler::getCellExtents( EdgeMap::iterator &cell ) {
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
