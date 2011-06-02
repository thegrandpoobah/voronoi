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
#include <windows.h>

// remove stupidness
#undef min
#undef max

#endif // WIN32

// stl
#include <string>
#include <memory>
#include <iostream>
#include <iomanip>
#include <fstream>

// boost
#include <boost/timer.hpp>

// SDL
#include <SDL.h>

// OpenGL
#include <GL/gl.h>
#include <GL/glu.h>

// local
#include "bitmap.h"
#include "stippler.h"
#include "abstractstippler.h"
#include "parse_arguments.h"
#include "guicon.h"

#ifdef WIN32
int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow ) {
#else 
int main( int argc, char *argv[] ) {
#endif // WIN32
#ifdef WIN32
	int argc = __argc;
	char **argv = __argv;

	RedirectIOToConsole();
#endif // WIN32
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

	::SDL_Init( SDL_INIT_VIDEO );

	::SDL_WM_SetCaption( "Weighted Voronoi Stippling", "Weighted Voronoi Stippling" );

	::SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 8 );
	::SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 8 );
	::SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 8 );
	::SDL_GL_SetAttribute( SDL_GL_ALPHA_SIZE, 8 );
	::SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 24 );
	::SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );

	::SDL_Surface *screen = ::SDL_SetVideoMode( 640, 480, 32, SDL_OPENGL | SDL_HWSURFACE );

	::glClearColor( 0.0, 0.0, 0.0, 0.0 );
	::glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	::SDL_GL_SwapBuffers();

	auto_ptr<Stippler> stippler;

	ofstream log;
	if ( parameters->createLogs ) {
		log.open( "log.txt" );
	}

	try {
		stippler = auto_ptr<Stippler>( new CPUStippler( parameters->inputFile, parameters->points ) );
		if ( parameters->useColour ) {
			stippler->useColour();
		}
	} catch ( exception e ) {
		cerr << e.what() << endl;

		return -1;
	}

	cout << "Generating " << parameters->points << " stipples." << endl;

	if ( parameters->createLogs ) {
		log << "Generating " << parameters->points << " stipples." << endl;

		log << "Displacement Threshold is " << parameters->threshold << "." << endl;
		cout << "Displacement Threshold is " << parameters->threshold << "." << endl;
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

		stippler->paint();
	
		::SDL_GL_SwapBuffers();

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

	::SDL_Quit();

	return 0;
}
