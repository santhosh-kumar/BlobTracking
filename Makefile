#compiler
CC=g++

#library
#for opencv 2.1
#LDFLAGS=-lcxcore -lcv -lhighgui -lippiem64t -lippmem64t -lippcvem64t -lippcoreem64t  -liomp5 -lpthread -lm
#for opencv2.2 and 2.3 onwards, use the following
LIBS=-lopencv_core -lopencv_highgui -lopencv_imgproc -lopencv_objdetect -lopencv_video -lopencv_ml

#headers
CFLAGS=-c -Wall -I/usr/local/include/opencv -I/usr/local/include/opencv2 -I/usr/local/include/ -I/usr/local/boost/include -fopenmp
SRCDIR=$(CURDIR)

SOURCES=$(wildcard $(SRCDIR)/*.cpp)
	
OBJECTS = $(SOURCES:.cpp=.o)

EXECUTABLE = $(CURDIR)/BlobTracker

all: $(OBJECTS) $(EXECUTABLE)

clean: 
	rm -rf $(EXECUTABLE)
	rm ./*.o

$(EXECUTABLE): $(OBJECTS) 
	$(CC) -Wall -L$(IPP)/sharedlib -L/usr/local/lib $(LIBS)  $(OBJECTS)  -o $@  `pkg-config --cflags --libs opencv` 

$(OBJECTS): $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES) 
