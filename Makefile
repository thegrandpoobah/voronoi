OPENMPFLAG = -fopenmp

CXXFLAGS =	-Wall -fpermissive $(OPENMPFLAG) -O2

LNKFLAGS =	$(OPENMPFLAG) -O2

INCLUDES =	-I./picopng -I./stippler -I./voronoi

LIBS = -lboost_program_options

OBJS =	picopng/picopng.o stippler/bitmap.o stippler/stippler_api.o stippler/stippler.o stippler/VoronoiDiagramGenerator.o voronoi/parse_arguments.o voronoi/voronoi.o

VPATH =	%.cpp

# openmp is not available on Mac OS X when using Clang
UNAME := $(shell uname)
ifeq ($(UNAME), Darwin)
	OPENMPFLAG =
endif

all:	voronoi_stippler
.SUFFIXES: .cpp .o


voronoi_stippler:	$(OBJS)
	$(CXX) $(LNKFLAGS) -o voronoi_stippler $(OBJS) $(LIBS)

clean:
	rm -f $(OBJS)

cleanall:	clean
	rm -f voronoi_stippler


.cpp.o:
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@
