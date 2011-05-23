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

#ifndef GPUSTIPPLER_H
#define GPUSTIPPLER_H

#ifdef WIN32
#include <windows.h>
#endif // WIN32

#include <gl/gl.h>

#include <string>
#include <map>
#include <vector>

#include "abstractstippler.h"

class GLSLProgram;

class GPUStippler : public AbstractStippler {
public:
	GPUStippler( std::string &image_path, const unsigned int points );
	~GPUStippler();
protected:
	void renderCell( EdgeMap::iterator &cell, const extents &extent );
	std::pair< Point<float>, float > calculateCellCentroid( const extents &extent );
private:
	void transferDataToReadBuffer( const extents &extent );

	void createCellBuffer();
	void createCalculationBuffer();
	void initializeGLSL();

	std::string readFile( const char *filename );
private:
	GLuint cellFBO, cellTexture;
	GLuint calculationFBO, calculationTextures[2];

	GLSLProgram *dataMappingShader;
	GLSLProgram *summingShader;
};

#endif // GPUSTIPPLER_H
