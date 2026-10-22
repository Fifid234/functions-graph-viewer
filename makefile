CXXCOMPILER    = g++
CXXFLAGS       = -std=c++20 -Wall -Wextra -Werror
CXXLINKERFLAGS = -lraylib -Iinclude

OUTPUT = -o bin/main
SOURCE = src/*.cpp

DEBUGFLAGS 	 = -fsanitize=address -O0 -g
RELEASEFLAGS = -O3

debug:
	$(CXXCOMPILER) $(SOURCE) $(DEBUGFLAGS) $(OUTPUT) $(CXXLINKERFLAGS)

release:
	$(CXXCOMPILER) $(SOURCE) $(RELEASEFLAGS) $(OUTPUT) $(CXXLINKERFLAGS)
