CC=g++
CFLAGS=-Iinclude -O3 -Wall -D_GLIBCXX_ISE_CXX11_ABI=1
LDFLAGS=-Llib -lDetailPlace -lGlobalPlace -lLegalizer -lPlacement -lParser -lPlaceCommon
SOURCES=src/data.cpp src/GlobalPlacer.cpp src/ExampleFunction.cpp src/main.cpp src/viaHandler.cpp src/CellLegalizer.cpp src/CG.cpp src/DP.cpp
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=place

all: $(SOURCES) bin/$(EXECUTABLE)
	
bin/$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) $(LDFLAGS) lp_solve/liblpsolve55.a -ldl -o $@

%.o: %.c ${INCLUDES}
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf *.o bin/$(EXECUTABLE)
