sources = $(wildcard *.cpp)
headers = $(wildcard *.h)
objects = $(sources:%.cpp=%.o)
deps = $(sources:%.cpp=%.d)
CFLAGS +=-g -Wall
CXXFLAGS +=-g -Wall

all: foochess

clean:
	rm -f $(objects) foochess

install: foochess
	cp foochess /usr/bin
	gzip foochess.man
	cp foochess.man.gz /usr/share/man/man1/foochess.1.gz
	gzip -d foochess.man.gz

uninstall:
	rm /usr/bin/foochess
	rm /usr/share/man/man1/foochess.1.gz

foochess: $(objects)
	$(CXX) $(LDFLAGS) $(LOADLIBES) $(LDLIBS) $^ -Wall -o foochess

#a special debug build
debug: CXXFLAGS += -D DEBUG
debug: $(objects)
	$(CXX) $(LDFLAGS) $(LOADLIBES) $(LDLIBS) $^ -D DEBUG -Wall -o foochess

-include $(deps)

