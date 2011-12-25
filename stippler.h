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

#ifndef STIPPLER_H
#define STIPPLER_H

#ifdef WIN32
#define NOMINMAX
#include <windows.h>
#endif // WIN32

#include <GL/gl.h>

#include <string>
#include <vector>

#include <boost/unordered_map.hpp>

#include "istippler.h"
#include "parse_arguments.h"
#include "utility.h"
#include "bitmap.h"

class Stippler : public IStippler {
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
	Stippler( std::string &image_path, const StipplingParameters &parameters );
	~Stippler();

	void distribute();
	void paint();
	float getAverageDisplacement();
	void render( std::string &output_path );

	void useColour();
	void noOverlap();
	void fixedRadius();
	void sizingFactor(float sizingFactor);
protected:
	void createInitialDistribution();
	void createVoronoiDiagram();

	extents getCellExtents( EdgeMap::iterator &cell );

	void redistributeStipples();

	void renderCell( EdgeMap::iterator &cell, const extents &extent );
	std::pair< Point<float>, float > calculateCellCentroid( EdgeMap::iterator &cell, const extents &extent );

	// this is just for the front end
	void createCircleDisplayList();
private:
	void clipAndFill( unsigned char *bitmap, const float projection[9], float insideX, float insideY, float Fx1, float Fy1, float Fx2, float Fy2);
protected:
	unsigned char *framebuffer;

	EdgeMap edges;

	GLuint dl_circle;

	float *vertsX, *vertsY;
	float *radii;
	float displacement;

	Bitmap image;

	unsigned int tileWidth, tileHeight;

	const StipplingParameters &parameters;
};

bool operator==(Point<float> const& p1, Point<float> const& p2);
std::size_t hash_value(Point<float> const& p);

#endif // STIPPLER_H
