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

#ifndef STIPPLER_IMPL_H
#define STIPPLER_IMPL_H

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif // _WIN32

#include <string>
#include <vector>

#include <boost/unordered_map.hpp>

#include "stippler.h"
#include "istippler.h"
#include "utility.h"
#include "bitmap.h"

class Stippler : public IStippler {
protected:
	typedef std::vector< Edge< float > > EdgeList;
	typedef boost::unordered_map< Point < float >, EdgeList > EdgeMap;
public:
	Stippler( const StipplingParameters &parameters );
	~Stippler();

	void distribute();
	float getAverageDisplacement();
	void getStipples( StipplePoint *dst );
protected:
	void createInitialDistribution();
	void createVoronoiDiagram();

	Extents<float> getCellExtents( EdgeList &edgeList );

	void redistributeStipples();

	std::pair< Point<float>, float > calculateCellCentroid( Point<float> &inside, EdgeList &edgeList );
	Line<float> createClipLine( float insideX, float insideY, float x1, float y1, float x2, float y2 );
protected:
	EdgeMap edges;

	float *vertsX, *vertsY;
	float *radii;
	float displacement;

	Bitmap image;

	const StipplingParameters &parameters;
};

bool operator==(Point<float> const& p1, Point<float> const& p2);
std::size_t hash_value(Point<float> const& p);

#endif // STIPPLER_IMPL_H
