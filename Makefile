sources = $(wildcard *.cpp)
headers = $(wildcard *.h)
objects = $(sources:%.cpp=%.o)
deps = $(sources:%.cpp=%.d)
CFLAGS += -Wall
CXXFLAGS += -Wall

all: foochess

clean:
	rm -f $(objects) foochess

foochess: $(objects)
	$(CXX) $(LDFLAGS) $(LOADLIBES) $(LDLIBS) $^ -Wall -o foochess

#a special debug build
debug: CXXFLAGS += -D DEBUG
debug: $(objects)
	$(CXX) $(LDFLAGS) $(LOADLIBES) $(LDLIBS) $^ -D DEBUG -Wall -o foochess

-include $(deps)

