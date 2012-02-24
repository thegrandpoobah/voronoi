CXXFLAGS =	-fpermissive -fopenmp -O2

LNKFLAGS =	-fopenmp -O2

INCLUDES =	-I./picopng -I./stippler -I./voronoi

LIBS =		-lboost_program_options

OBJS =	picopng/picopng.o stippler/bitmap.o stippler/stippler_api.o stippler/stippler.o stippler/utility.o stippler/VoronoiDiagramGenerator.o voronoi/parse_arguments.o voronoi/voronoi.o

VPATH =	%.cpp


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
