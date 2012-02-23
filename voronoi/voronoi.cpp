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

#ifdef WIN32
// needed by opengl
#define NOMINMAX
#include <windows.h>
#endif // WIN32

// stl
#include <string>
#include <memory>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <cmath>

// boost
#include <boost/timer.hpp>

// PNG library
#include <picopng.h>

// stippler library
#include <istippler.h>

// local
#include "parse_arguments.h"

void write_configuration( std::ostream &output, const Voronoi::StipplingParameters &parameters ) {
	using std::endl;
	using std::abs;
	using std::numeric_limits;

	output << "Generating " << parameters.points << " stipples." << endl;
	output << "Options: ";
	if ( parameters.useColour ) {
		output << "Coloured stipples";
	} else {
		output << "Black stipples";
	}

	if ( parameters.noOverlap ) {
		output << ", Non-overlapping stipples";
	} else {
		output << ", overlapping stipples";
	}

	if ( parameters.fixedRadius ) {
		output << ", Fixed radius";
	} else {
		output << ", Variable radius";
	}

	output << ", Subpixel density of " << parameters.subpixels;

	if ( abs( parameters.sizingFactor - 1.0f ) > numeric_limits<float>::epsilon() ) {
		output << ", Sizing factor of " << parameters.sizingFactor;
	}

	if ( abs( parameters.threshold - 0.1f ) > numeric_limits<float>::epsilon() ) {
		output << ", Displacement Threshold of " << parameters.threshold;
	}

	output << endl;
}

void render( STIPPLER_HANDLE stippler, const Voronoi::StipplingParameters &parameters ) {
	StipplePoint *points = new StipplePoint[parameters.points];

	stippler_getStipples(stippler, points);

	using namespace std;

	ofstream outputStream( parameters.outputFile.c_str() );

	if ( !outputStream.is_open() ) {
		stringstream s;
		s << "Unable to open output file " << parameters.outputFile;
                std::cout << s.str().c_str() << std::endl;
		throw exception();
	}

	PNG::PNGFile *png = PNG::load( parameters.inputFile );
	unsigned long w = png->w, h = png->h;
	PNG::freePng(png);

	outputStream << "<?xml version=\"1.0\" ?>" << endl;
	outputStream << "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\" \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">" << endl;
	outputStream << "<svg width=\"" << w << "\" height=\"" << h << "\" version=\"1.1\" xmlns=\"http://www.w3.org/2000/svg\">" << endl;
	
	float radius;
	for ( unsigned int i = 0; i < parameters.points; ++i ) {
		if ( parameters.fixedRadius ) {
			radius = 0.5f; // gives circles with 1px diameter
		} else {
			radius = points[i].radius;
		}
		radius *= parameters.sizingFactor;

		if ( !parameters.useColour ) {
			points[i].r = points[i].g = points[i].b = 0;
		}

		outputStream << "<circle cx=\"" << points[i].x << "\" cy=\"" << points[i].y << "\" r=\"" << radius << "\" fill=\"rgb(" << (unsigned int)points[i].r << "," << (unsigned int)points[i].g << "," << (unsigned int)points[i].b << ")\" />" << endl;
	}
	outputStream << "</svg>" << endl;

	outputStream.close();

	delete[] points;
}

int main( int argc, char *argv[] ) {
	using std::auto_ptr;
	using std::exception;
	using std::ofstream;
	using std::cout;
	using std::cerr;
	using std::endl;
	using std::setprecision;
	using std::setiosflags;
	using std::ios;
	using std::min;
	using boost::timer;

	auto_ptr<Voronoi::StipplingParameters> parameters;
	
	try {
		parameters = parseArguments( argc, argv );
		if (parameters.get() == NULL) {
			return 0;
		}
	} catch ( exception e ) {
		return -1;
	}

	STIPPLER_HANDLE stippler;

	ofstream log;
	if ( parameters->createLogs ) {
		log.open( "log.txt" );
	}

	try {
		stippler = create_stippler( parameters.get() );
	} catch ( exception e ) {
		delete[] parameters.get()->inputFile;
		cerr << e.what() << endl;

		return -1;
	}

	write_configuration( cout, *(parameters.get()) );
	if ( parameters->createLogs ) {
		write_configuration( log, *(parameters.get()) );
	}

	int iteration = 0;
	float t = parameters->threshold + 1.0f;
	do {
		timer profiler;

		stippler_distribute(stippler);

		if ( parameters->createLogs ) {
			log << "Iteration " << (++iteration) << " completed in " << profiler.elapsed() << " seconds." << endl;
			cout << "Iteration " << iteration << " completed in " << profiler.elapsed() << " seconds." << endl;
		}

		t = stippler_getAverageDisplacement( stippler );

		if ( parameters->createLogs ) {
			log << "Current Displacement: " << t << endl;
			cout << "Current Displacement: " << t << endl;
		}

		cout << setiosflags(ios::fixed) << setprecision(2) << min((parameters->threshold / t * 100), 100.0f) << "% Complete" << endl; 
	} while ( t > parameters->threshold );

	// render final result to SVG
	try {
		render( stippler, *(parameters.get()) );
	} catch (exception e) {
		cerr << e.what();
	}

	delete[] parameters.get()->inputFile;

	if ( parameters->createLogs ) {
		log.close();
	}

	destroy_stippler( stippler );

	return 0;
}
