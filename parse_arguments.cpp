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

void validate( boost::program_options::variables_map &vm ) {
	using std::string;

	if ( ( vm.count( "input-file" ) == 0 && vm.count( "output-file" ) == 0 ) ) {
		throw string( "Input and output file must be specified" );
	}
}

std::auto_ptr<StipplingParameters> parseArguments( int argc, char *argv[] ) {
	using namespace boost::program_options;

	using std::auto_ptr;
	using std::string;
	using std::cout;
	using std::cerr;
	using std::endl;
	using std::exception;

	options_description genericOpts( "Basic Options" );
	genericOpts.add_options()
		( "algorithm,a", value< char >(), "Chooses which Stippling Algorithm will be used" )
		( "stipples,s", value< unsigned int >(), "Number of Stipple Points to use" )
		( "threshold,t", value< float >(), "How long to wait for Voronoi diagram to converge" )
		( "input-file,I", value< string >(), "Name of input file" )
		( "output-file,O", value< string >(), "Name of output file" )
		( "log,l", value< bool >(), "Determines output verbosity" );

	positional_options_description positional;
	positional.add( "input-file", 1 );
	positional.add( "output-file", 1 );

	// parse the parameters
	variables_map vm;
	store( command_line_parser( argc, argv ).options( genericOpts ).positional( positional ).run(), vm );
	notify( vm );

	try {
		auto_ptr<StipplingParameters> params( new StipplingParameters() );

		validate( vm );

		if ( vm.count( "help" ) > 0 ) {
			throw exception();
		}

		params->inputFile = vm["input-file"].as<string>();
		params->outputFile = vm["output-file"].as<string>();

		if ( vm.count("algorithm") > 0 ) {
			switch ( vm["algorithm"].as<char>() ) {
				case 'c':
				case 'C':
					params->algorithm = StipplingParameters::CPU;
					break;
				case 'g':
				case 'G':
					params->algorithm = StipplingParameters::GPU;
					break;
				default:
					break;
			}
		}

		if ( vm.count("stipples") > 0 ) {
			params->points = vm["stipples"].as<unsigned int>();
		}

		if ( vm.count("threshold") > 0 ) {
			params->threshold = vm["threshold"].as<float>();
		}

		if ( vm.count("log") > 0 ) {
			params->createLogs = vm["log"].as<bool>();
		}

		return params;
	} catch ( string &s ) {
		cerr << s << endl;
		cout << genericOpts << endl;

		throw exception();
	} catch ( ... ) {
		cout << genericOpts << endl;

		throw exception();
	}
}
