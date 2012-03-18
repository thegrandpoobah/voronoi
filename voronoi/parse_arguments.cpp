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

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <exception>

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/program_options.hpp>

#include "parse_arguments.h"

void showSamples() {
	using std::cout;
	using std::endl;

	cout << "Sample usages" << endl << endl;
	cout << "  voronoi [input] [output]" << endl;
	cout << "\tCreates a stipple SVG image named [output] from [input] image with 4000\n\tstipple points in black and white." << endl;
	cout << "  voronoi -s 16000 [input] [output]" << endl;
	cout << "\tCreates a stipple SVG image named [output] from [input] image with\n\t16000 stipple points in black and white." << endl;
	cout << "  voronoi -c [input] [output]" << endl;
	cout << "\tCreates a stipple SVG image named [output] from [input] image with 4000\n\tstipple points in colour." << endl;
	cout << endl;
}

std::auto_ptr<Voronoi::StipplingParameters> parseArguments( int argc, char *argv[] ) {
	using namespace boost::program_options;

	using std::auto_ptr;
	using std::string;
	using std::cout;
	using std::cerr;
	using std::endl;
	using std::runtime_error;
	using std::exception;

	options_description requiredOpts( "Required Options" );
	requiredOpts.add_options()
		( "input-file,I", value< string >()->required(), "Name of input file" )
		( "output-file,O", value< string >()->required(), "Name of output file" );

	options_description helpOpts( "Help Options" );
	helpOpts.add_options()( "help", "Help!" );

	options_description basicOpts( "Basic Options" );
	basicOpts.add_options()
		( "stipples,s", value< int >()->default_value(4000), "Number of Stipple Points to use" )
		( "colour-output,c", "Produce a coloured stipple drawing" );

	options_description advancedOpts( "Advanced Options" );
	advancedOpts.add_options()
		( "threshold,t", value< float >()->default_value(0.1f, "0.1"), "How long to wait for Voronoi diagram to converge" )
		( "no-overlap,n", "Ensure that stipple points do not overlap with each other" )
		( "fixed-radius,f", "Fixed radius stipple points imply a significant loss of tonal properties" )
		( "sizing-factor,z", value< float >()->default_value(1.0f, "1.0"), "The final stipple radius is multiplied by this factor" )
		( "subpixels,p", value< int >()->default_value(5, "5"), "Controls the tile size of centroid computations." )
		( "log,l", "Determines output verbosity" );

	positional_options_description positional;
	positional.add( "input-file", 1 );
	positional.add( "output-file", 1 );

	options_description all( "All Options" );
	all
		.add( requiredOpts )
		.add( basicOpts )
		.add( advancedOpts );

	// parse the parameters
	try {
		variables_map help_variables;
		store( command_line_parser( argc, argv ).options( helpOpts ).allow_unregistered().run(), help_variables );
		notify( help_variables );

		if ( help_variables.count( "help" ) > 0 ) {
			showSamples();
			cout << requiredOpts << endl;
			cout << basicOpts << endl;
			cout << advancedOpts << endl;

			return auto_ptr<Voronoi::StipplingParameters>( NULL );
		}

		variables_map vm;
		store( command_line_parser( argc, argv ).options( all ).positional( positional ).run(), vm );
		notify( vm );

		auto_ptr<Voronoi::StipplingParameters> params( new Voronoi::StipplingParameters() );

		string inputFile = vm["input-file"].as<string>();
		params->inputFile = new char[inputFile.length() + 1]; memset(params->inputFile, 0, inputFile.length() + 1);
		inputFile.copy(params->inputFile, inputFile.length());
		params->outputFile = vm["output-file"].as<string>();

		if (vm["stipples"].as<int>() <= 0) {
			throw runtime_error("Stipple renderings must have at least 1 stipple point.");
		}
		params->points = (unsigned int)vm["stipples"].as<int>();
		if (vm["threshold"].as<float>() < 0.005f) {
			throw runtime_error("Convergence threshold parameter must be greater than 0.005");
		}
		params->threshold = vm["threshold"].as<float>();
		params->createLogs = vm.count("log") > 0;
		params->useColour = vm.count("colour-output") > 0;
		params->noOverlap = vm.count("no-overlap") > 0;
		params->fixedRadius = vm.count("fixed-radius") > 0;
		if (vm["sizing-factor"].as<float>() < 0.0f) {
			throw runtime_error("Sizing factor parameter must be greater than 0.");
		}
		params->sizingFactor = vm["sizing-factor"].as<float>();
		if (vm["subpixels"].as<int>() <= 1) {
			throw runtime_error("Sub-pixel density parameter must be greater than or equal to 1.");
		}
		params->subpixels = (unsigned int)vm["subpixels"].as<int>();

		return params;
	} catch ( exception const &e ) {
		cerr << e.what() << endl;
		cout << endl;
		showSamples();
		cout << endl;
		cout << "For more information invoke help via \"voronoi --help\"." << endl;

		throw exception();
	}
}
