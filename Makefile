CXX = g++
CXXFLAGS = -std=c++2b -Wall -g -pthread # Añadido -pthread
LDFLAGS = -pthread # Añadido -pthread

TARGET = main
SRCS = main.cpp \
	   containers/ListsDemo.cpp \
	   containers/TreeDemo.cpp \
	   containers/HeapDemo.cpp \
	   containers/HashDemo.cpp \
	   containers/AVLMapDemo.cpp \
	   containers/BTreeDemo.cpp

OBJS = $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(LDFLAGS) $^ -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean