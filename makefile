CXXCOMPILER    = g++
CXXFLAGS       = -std=c++20 -Wall -Wextra -Werror
CXXLINKERFLAGS = -lraylib -Iinclude

OUTPUT = -o bin/function-view
SOURCE = src/*.cpp

BUILDFLAGS = -O3

build:
	mkdir -p bin
	$(CXXCOMPILER) $(SOURCE) $(BUILDFLAGS) $(OUTPUT) $(CXXLINKERFLAGS)
