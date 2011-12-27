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

#include <fstream>
#include <sstream>
#include <limits>

#include <boost/random.hpp>

#include "VoronoiDiagramGenerator.h"

Stippler::Stippler( const StipplingParameters &parameters )
: IStippler(),
parameters(parameters),
tileWidth(128), tileHeight(128),
displacement(std::numeric_limits<float>::max()),
vertsX(new float[parameters.points]), vertsY(new float[parameters.points]), radii(new float[parameters.points]),
image(parameters.inputFile) {
	createInitialDistribution();
}

Stippler::~Stippler() {
	delete[] radii;
	delete[] vertsX;
	delete[] vertsY;
}

void Stippler::distribute() {
	createVoronoiDiagram();
	redistributeStipples();
}

float Stippler::getAverageDisplacement() {
	return displacement;
}

void Stippler::createInitialDistribution() {
	using std::ceil;

	// find initial distribution
	boost::mt19937 rng;
	boost::uniform_01<boost::mt19937, float> generator( rng );

	float w = (float)image.getWidth(), h = (float)image.getHeight();
	float xC, yC;

	for ( unsigned int i = 0; i < parameters.points; ) {
		xC = generator() * w;
		yC = generator() * h;

		// do a nearest neighbour search on the vertices
		if ( ceil(generator() * 255.0f) <= image.getIntensity( xC, yC ) ) {
			vertsX[i] = xC;
			vertsY[i] = yC;
			radii[i] = 0.0f;

			i++;
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
			image.getColour(vertsX[i], vertsY[i], r, g, b);
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
}

inline Stippler::line Stippler::createClipLine( float FinsideX, float FinsideY, float Fx1, float Fy1, float Fx2, float Fy2 ) {
	using std::abs;
	using std::numeric_limits;

	Stippler::line l;

	// if the floating point version of the line collapsed down to one
	// point, then just ignore it all
	if (abs(Fx1 - Fx2) < std::numeric_limits<float>::epsilon() && abs(Fy1 - Fy2) < std::numeric_limits<float>::epsilon()) {
		l.a = .0f;
		l.b = .0f;
		l.c = .0f;

		return l;
	}

	l.a = -(Fy1 - Fy2);
	l.b = Fx1 - Fx2;
	l.c = (Fy1 - Fy2) * Fx1 - (Fx1 - Fx2) * Fy1;

	// make sure the known inside point falls on the correct side of the clipping plane
	if ( FinsideX * l.a + FinsideY * l.b + l.c > 0 ) {
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
	using std::floor;
	using std::abs;

	vector<line> clipLines;

	int x, y;

	float xStep = ( extent.maxX - extent.minX ) / (float)tileWidth;
	float yStep = ( extent.maxY - extent.minY ) / (float)tileHeight;

	float spotDensity, areaDensity = 0.0f, maxAreaDensity = 0.0f;
	float xSum = 0.0f;
	float ySum = 0.0f;

	float xCurrent;
	float yCurrent;

	// compute the clip lines
	for ( EdgeList::iterator value_iter = cell->second.begin(); value_iter != cell->second.end(); ++value_iter ) {
		line l = createClipLine( cell->first.x, cell->first.y, 
			value_iter->begin.x, value_iter->begin.y,
			value_iter->end.x, value_iter->end.y );
	
		if (l.a < numeric_limits<float>::epsilon() && abs(l.b) < numeric_limits<float>::epsilon()) {
			continue;
		}

		clipLines.push_back(l);
	}

	for ( y = 0, yCurrent = extent.minY; y < (int)tileHeight; ++y, yCurrent += yStep ) {
		for ( x = 0, xCurrent = extent.minX; x < (int)tileWidth; ++x, xCurrent += xStep ) {
			// a point is outside of the polygon if it is outside of all clipping planes
			bool outside = false;
			for ( vector<line>::iterator iter = clipLines.begin(); iter != clipLines.end(); iter++ ) {
				if ( xCurrent * iter->a + yCurrent * iter->b + iter->c >= 0 ) {
					outside = true;
					break;
				}
			}

			if (!outside) {
				spotDensity = image.getIntensity(floor(xCurrent), floor(yCurrent));

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

std::auto_ptr<IStippler> make_stippler( const StipplingParameters &parameters ) {
	using std::auto_ptr;

	return auto_ptr<IStippler>( new Stippler( parameters ) );
}
