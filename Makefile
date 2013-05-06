sources = $(wildcard *.cpp)
headers = $(wildcard *.h)
objects = $(sources:%.cpp=%.o)
deps = $(sources:%.cpp=%.d)
CFLAGS += -Wall
CXXFLAGS += -Wall

#Uncomment this line  to get a boatload of debug output.
#CPPFLAGS = -DSHOW_NETWORK
#override CPPFLAGS += -DSHOW_WARNINGS

all: foochess

.PHONY: clean all subdirs

libclient_%.o: override CXXFLAGS += -fPIC
libclient_%.o: %.cpp *$(headers)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c -o $@ $<

clean:
	rm -f $(objects) foochess

foochess: $(objects)
	$(CXX) $(LDFLAGS) $(LOADLIBES) $(LDLIBS) $^ -Wall -o foochess

-include $(deps)

