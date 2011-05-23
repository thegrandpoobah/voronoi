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

#include "GLee/GLee.h"

#include "glslprogram.h"

GLSLProgram::GLSLProgram( std::string &vertexProgram /* = "" */, std::string &fragmentProgram /* = "" */ ) :
vertexShader(0), fragmentShader(0) {
	glslProgram = ::glCreateProgram();

	if ( !vertexProgram.empty() ) {
		const char *src = vertexProgram.c_str();

		vertexShader = ::glCreateShader( GL_VERTEX_SHADER );
		::glShaderSource( vertexShader, 1, &src, NULL );
		::glCompileShader( vertexShader );
		::glAttachShader( glslProgram, vertexShader );
	}
	if ( !fragmentProgram.empty() ) {
		const char *src = fragmentProgram.c_str();
		fragmentShader = ::glCreateShader( GL_FRAGMENT_SHADER );
		::glShaderSource( fragmentShader, 1, &src, NULL );
		::glCompileShader( fragmentShader );
		::glAttachShader( glslProgram, fragmentShader );
	}

	::glLinkProgram( glslProgram );
//	printProgramInfoLog( glslProgram );
}

GLSLProgram::~GLSLProgram() {
	if ( fragmentShader != 0 ) {
		::glDeleteShader( fragmentShader );
	}
	if ( vertexShader != 0 ) {
		::glDeleteShader( vertexShader );
	}
	::glDeleteProgram( glslProgram );
}

GLuint GLSLProgram::getParameter( std::string &parameter ) {
	return ::glGetUniformLocation( glslProgram, parameter.c_str() );
}

void GLSLProgram::startProgram() {
	::glUseProgram( glslProgram );
}

void GLSLProgram::endProgram() {
	::glUseProgram( 0 );
}

