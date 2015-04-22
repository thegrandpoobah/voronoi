# Weighted Voronoi Stippler

The Weighted Voronoi Stippler is a command line tool that creates
stippling art in SVG format based on an input PNG image.

## Usage Instructions

Usage instructions are available via the 
[documentation page](http://www.saliences.com/projects/npr/stippling/options.html).

## Windows Binary

A Windows binary is available via the 
[project homepage](http://www.saliences.com/projects/npr/stippling/index.html).

## Building from Source

### Windows

There is a Visual Studio solution file included with the project that you can
use to compile the application from source. However, you must have the [Boost](http://www.boost.org) library installed on your system before it will compile cleanly.

### Mac OS X

A makefile was contributed (thanks!) which can be used to build a binary
from source on Mac OS X. Using [Homebrew](http://brew.sh/), you can quickly 
install Boost as a necessary dependency.

    brew install boost

Then, simply compile with 

    make

Note that by default, Mac OS X uses the Clang compiler which does not support
the [OpenMP](http://openmp.org) library and as such will see significant
deterioriated performance. GCC is available for Mac OS X and can be used
to compile the application, but installing GCC, Boost, and OpenMP are beyond
the scope of this document.

### Building on Linux

The makefile works on Linux as well, so as long as you install Boost and OpenMP
on your machine (either using your distribution's package manager or via source),
it should just work.

## Testing

There are no unit tests (contributions welcome!), but there is a corpus of images included with the tool to do repetitive testing with.

## Included Third Party Libraries

The following two libraries are included and have been tweaked slightly to get rid
of compilation warnings.

* [picoPNG](http://members.gamedev.net/lode/projects/LodePNG/) (included in the source tree).
* An implementation of [Fortune's algorithm](http://www.skynet.ie/~sos/mapviewer/voronoi.php) 
provided by O'Sullivan (included in the source tree).

