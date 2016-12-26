#
# Author: Joshua Inscoe
# Last modified: November 17, 2016
#
# File: Makefile
#



CXX = g++
CXXFLAGS = -std=c++0x


EXE = annoy


LIBS =

HDRS =
SRCS = annoy.cpp
OBJS = $(SRCS:.cpp=.o)


$(EXE): $(HDRS) $(OBJS) Makefile
	$(CXX) $(CXXFLAGS) $(OBJS) $(LIBS) -o $@

$(OBJS): $(HDRS) $(SRCS) Makefile



clean:
	rm -rf $(OBJS) $(EXE) $(EXE).dSYM core 2>/dev/null
