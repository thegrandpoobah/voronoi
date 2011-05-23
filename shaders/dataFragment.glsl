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

uniform sampler2D texelUnit;
uniform float subarea;
uniform float threshold;
uniform vec4 extents;

void main(void) {
	vec4 data = texture2D( texelUnit, gl_TexCoord[0].st );
	vec2 pos = vec2( extents[0] + gl_TexCoord[0].s * extents[1], extents[2] - gl_TexCoord[0].t * extents[3] );
	if ( data.x > threshold ) {
		gl_FragColor = vec4( data.x, data.x * pos.x, data.x * pos.y, data.x * subarea );
	} else {
		gl_FragColor = vec4( 0, 0, 0, 0 );
	}
	//gl_FragColor = vec4( data.x, pos.x, pos.y, data.x * subarea );
}