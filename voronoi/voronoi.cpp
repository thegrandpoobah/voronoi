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

// boost
#include <boost/timer.hpp>

// stippler library
#include <istippler.h>

// local
#include "parse_arguments.h"

void write_configuration( std::ostream &output, const StipplingParameters &parameters ) {
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

	auto_ptr<StipplingParameters> parameters;
	
	try {
		parameters = parseArguments( argc, argv );
		if (parameters.get() == NULL) {
			return 0;
		}
	} catch ( exception e ) {
		return -1;
	}

	auto_ptr<IStippler> stippler;

	ofstream log;
	if ( parameters->createLogs ) {
		log.open( "log.txt" );
	}

	try {
		stippler = make_stippler( *( parameters.get() ) );
	} catch ( exception e ) {
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

		stippler->distribute();

		if ( parameters->createLogs ) {
			log << "Iteration " << (++iteration) << " completed in " << profiler.elapsed() << " seconds." << endl;
			cout << "Iteration " << iteration << " completed in " << profiler.elapsed() << " seconds." << endl;
		}

		t = stippler->getAverageDisplacement();

		if ( parameters->createLogs ) {
			log << "Current Displacement: " << t << endl;
			cout << "Current Displacement: " << t << endl;
		}

		cout << setiosflags(ios::fixed) << setprecision(2) << min((parameters->threshold / t * 100), 100.0f) << "% Complete" << endl; 
	} while ( t > parameters->threshold );

	// render final result to SVG
	try {
		stippler->render( parameters->outputFile );
	} catch (exception e) {
		cerr << e.what();
	}

	if ( parameters->createLogs ) {
		log.close();
	}

	return 0;
}
