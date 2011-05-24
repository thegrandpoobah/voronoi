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

#ifndef ABSTRACTSTIPPLER_H
#define ABSTRACTSTIPPLER_H

#ifdef WIN32
#include <windows.h>
#endif // WIN32

#include <GL/gl.h>

#include <string>
#include <vector>

#include <boost/unordered_map.hpp>

#include "stippler.h"
#include "utility.h"
#include "bitmap.h"

class AbstractStippler : public Stippler {
protected:
	typedef std::vector< Edge< float > > EdgeList;
	typedef boost::unordered_map< Point < float >, EdgeList > EdgeMap;

	struct extents {
		float minX;
		float minY;
		float maxX;
		float maxY;
	};
public:
	AbstractStippler( std::string &image_path, const unsigned int points );
	virtual ~AbstractStippler();

	void distribute();
	void paint();
	float getAverageDisplacement();
	void render( std::string &output_path );

	void useColour();
protected:
	void createInitialDistribution();
	void createVoronoiDiagram();

	float getIntensityThreshold();

	extents getCellExtents( EdgeMap::iterator &cell );

	void redistributeStipples();

	virtual void renderCell( EdgeMap::iterator &cell, const extents &extent );
	virtual std::pair< Point<float>, float > calculateCellCentroid( const extents &extent ) = 0;

	// this is just for the front end
	void createCircleDisplayList();
protected:
	EdgeMap edges;

	unsigned int points;

	GLuint dl_circle;

	float *vertsX, *vertsY;
	float *radii;
	float displacement;

	Bitmap image;

	unsigned int tileWidth, tileHeight;
	bool _useColour;
};

bool operator==(Point<float> const& p1, Point<float> const& p2);
std::size_t hash_value(Point<float> const& p);

#endif // ABSTRACTSTIPPLER_H
