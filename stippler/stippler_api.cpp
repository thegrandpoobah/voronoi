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

#include "istippler.h"
#include "stippler.h"
#include "stippler_impl.h"

namespace {
	char *last_error_message = NULL;

	void setLastError(const char *what) {
		if (last_error_message != NULL) {
			delete[] last_error_message;
		}

		last_error_message = new char[::strlen(what) + 1];
		::memset(last_error_message, 0, ::strlen(what) + 1);
		::strncpy(last_error_message, what, ::strlen(what));
	}
}

void stippler_lib_init() {
	// there is nothing to init
}

void stippler_lib_destroy() {
	if (last_error_message != NULL) {
		delete[] last_error_message;
		last_error_message = NULL;
	}
}

STIPPLER_HANDLE create_stippler( StipplingParameters *parameters ) {
	try {
		return reinterpret_cast<STIPPLER_HANDLE>( new Stippler( *parameters ) );
	} catch (std::runtime_error const &e) {
		setLastError(e.what());
		return NULL;
	}
}

void destroy_stippler( STIPPLER_HANDLE handle ) {
	delete reinterpret_cast<IStippler *>(handle);
}

void stippler_distribute( STIPPLER_HANDLE handle ) {
	(reinterpret_cast<IStippler *>(handle))->distribute();
}

float stippler_getAverageDisplacement( STIPPLER_HANDLE handle ) {
	return (reinterpret_cast<IStippler *>(handle))->getAverageDisplacement();
}

void stippler_getStipples( STIPPLER_HANDLE handle, StipplePoint *dst ) {
	return (reinterpret_cast<IStippler *>(handle))->getStipples(dst);
}

const char *stippler_getLastError() {
	return last_error_message;
}
