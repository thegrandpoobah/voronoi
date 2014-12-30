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

#include "stippler_impl.h"

#include <fstream>
#include <limits>
#include <cstring>

#include <boost/random.hpp>

#include "VoronoiDiagramGenerator.h"

Stippler::Stippler( const StipplingParameters &parameters )
: IStippler(),
vertsX(new float[parameters.points]), vertsY(new float[parameters.points]), radii(new float[parameters.points]),
displacement(std::numeric_limits<float>::max()),
image(parameters.inputFile),
parameters(parameters) {
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

	float w = (float)(image.getWidth() - 1), h = (float)(image.getHeight() - 1);
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

void Stippler::getStipples( StipplePoint *dst ) {
	StipplePoint *workingPtr;

	for (unsigned int i = 0; i < parameters.points; i++ ) {
		workingPtr = &(dst[i]);

		workingPtr->x = vertsX[i];
		workingPtr->y = vertsY[i];
		workingPtr->radius = radii[i];

		image.getColour(vertsX[i], vertsY[i], workingPtr->r, workingPtr->g, workingPtr->b); 
	}
}

void Stippler::createVoronoiDiagram() {
	VoronoiDiagramGenerator generator;

	generator.generateVoronoi( vertsX, vertsY, parameters.points, 
		0.0f, (float)(image.getWidth() - 1), 0.0f, (float)(image.getHeight() - 1) );

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
	using std::pow;
	using std::sqrt;
	using std::pair;
	using std::make_pair;
	using std::vector;

	vector< pair< Point< float >, EdgeList > > vectorized;
	for ( EdgeMap::iterator key_iter = edges.begin(); key_iter != edges.end(); ++key_iter ) {
		vectorized.push_back(make_pair(key_iter->first, key_iter->second));
	}

	float local_displacement = 0.0f;

	#pragma omp parallel for reduction(+:local_displacement)
	for (int i = 0; i < (int)vectorized.size(); i++) {
		pair< Point< float >, EdgeList > item = vectorized[i];
		pair< Point<float>, float > centroid = calculateCellCentroid( item.first, item.second );

		radii[i] = centroid.second;
		vertsX[i] = centroid.first.x;
		vertsY[i] = centroid.first.y;

		local_displacement += sqrt( pow( item.first.x - centroid.first.x, 2.0f ) + pow( item.first.y - centroid.first.y, 2.0f ) );
	}

	displacement = local_displacement / vectorized.size(); // average out the displacement
}

inline Line<float> Stippler::createClipLine( float insideX, float insideY, float x1, float y1, float x2, float y2 ) {
	using std::abs;
	using std::numeric_limits;

	Line<float> l;

	// if the floating point version of the line collapsed down to one
	// point, then just ignore it all
	if (abs(x1 - x2) < numeric_limits<float>::epsilon() && abs(y1 - y2) < numeric_limits<float>::epsilon()) {
		l.a = .0f;
		l.b = .0f;
		l.c = .0f;

		return l;
	}

	l.a = -(y1 - y2);
	l.b = x1 - x2;
	l.c = (y1 - y2) * x1 - (x1 - x2) * y1;

	// make sure the known inside point falls on the correct side of the clipping plane
	if ( insideX * l.a + insideY * l.b + l.c > 0.0f ) {
		l.a *= -1;
		l.b *= -1;
		l.c *= -1;
	}

	return l;
}

std::pair< Point<float>, float > Stippler::calculateCellCentroid( Point<float> &inside, EdgeList &edgeList ) {
	using std::make_pair;
	using std::numeric_limits;
	using std::vector;
	using std::floor;
	using std::ceil;
	using std::abs;
	using std::sqrt;
	using std::pow;

	vector< Line<float> > clipLines;
	Extents<float> extent = getCellExtents(edgeList);

	unsigned int x, y;

	float xDiff = ( extent.maxX - extent.minX );
	float yDiff = ( extent.maxY - extent.minY );

	unsigned int tileWidth = (unsigned int)ceil(xDiff) * parameters.subpixels;
	unsigned int tileHeight = (unsigned int)ceil(yDiff) * parameters.subpixels;

	float xStep = xDiff / (float)tileWidth;
	float yStep = yDiff / (float)tileHeight;

	float spotDensity, areaDensity = 0.0f, maxAreaDensity = 0.0f;
	float xSum = 0.0f;
	float ySum = 0.0f;

	float xCurrent;
	float yCurrent;

	// compute the clip lines
	for ( EdgeList::iterator value_iter = edgeList.begin(); value_iter != edgeList.end(); ++value_iter ) {
		Line<float> l = createClipLine( inside.x, inside.y, 
			value_iter->begin.x, value_iter->begin.y,
			value_iter->end.x, value_iter->end.y );
	
		if (l.a < numeric_limits<float>::epsilon() && abs(l.b) < numeric_limits<float>::epsilon()) {
			continue;
		}

		clipLines.push_back(l);
	}

	for ( y = 0, yCurrent = extent.minY; y < tileHeight; ++y, yCurrent += yStep ) {
		for ( x = 0, xCurrent = extent.minX; x < tileWidth; ++x, xCurrent += xStep ) {
			// a point is outside of the polygon if it is outside of all clipping planes
			bool outside = false;
			for ( vector< Line<float> >::iterator iter = clipLines.begin(); iter != clipLines.end(); iter++ ) {
				if ( xCurrent * iter->a + yCurrent * iter->b + iter->c >= 0.0f ) {
					outside = true;
					break;
				}
			}

			if (!outside) {
				spotDensity = image.getIntensity(xCurrent, yCurrent);

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
	if (areaDensity > numeric_limits<float>::epsilon()) {
		pt.x = xSum / areaDensity;
		pt.y = ySum / areaDensity;
	} else {
		// if for some reason, the cell is completely white, then the centroid does not move
		pt.x = inside.x;
		pt.y = inside.y;
	}

	float closest = numeric_limits<float>::max(),
		  farthest = numeric_limits<float>::min(),
		  distance;
	float x0 = pt.x, y0 = pt.y,
	      x1, x2, y1, y2;

	for ( EdgeList::iterator value_iter = edgeList.begin(); value_iter != edgeList.end(); ++value_iter ) {
		x1 = value_iter->begin.x; x2 = value_iter->end.x;
		y1 = value_iter->begin.y; y2 = value_iter->end.y;

		distance = abs( ( x2 - x1 ) * ( y1 - y0 ) - ( x1 - x0 ) * ( y2 - y1 ) ) / sqrt( pow( x2 - x1, 2.0f ) + pow( y2 - y1, 2.0f ) );
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

Extents<float> Stippler::getCellExtents( Stippler::EdgeList &edgeList ) {
	using std::numeric_limits;

	Extents<float> extent;

	extent.minX = extent.minY = numeric_limits<float>::max();
	extent.maxX = extent.maxY = numeric_limits<float>::min();

	for ( EdgeList::iterator value_iter = edgeList.begin(); value_iter != edgeList.end(); ++value_iter ) {
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

