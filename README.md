# Weighted Voronoi Stippler

The Weighted Voronoi Stippler is a command line tool that creates
stippling art in SVG format based on an input PNG image.

## Usage Instructions

Usage instructions are available via the 
[documentation page](http://www.saliences.com/projects/npr/stippling/options.html).

## Windows Binary

A Windows binary is available via the 
[project homepage](http://www.saliences.com/projects/npr/stippling/stippling.html).

## Building from Source

The program is primarily developed using Visual Studio 2010, but 
contributors have been kind enough to create a MAKEFILE that allows 
the tool to be compiled under Linux and Mac OS X.

The code depends on the following third party libraries for compilation:

* [Boost](http://www.boost.org) (Version 1.46 as of this writing).
* [picoPNG](http://members.gamedev.net/lode/projects/LodePNG/) (included in the source tree).
* An implementation of [Fortune's algorithm](http://www.skynet.ie/~sos/mapviewer/voronoi.php) 
provided by O'Sullivan (included in the source tree).

## Testing

There are no unit tests (contributions welcome!), but there is a fairly 
large corpus of images included with the tool to do repetitive testing with.