CXX = g++
# -MMD -MP genera dependencias de headers (.d) para recompilar al editar un .h
CXXFLAGS = -std=c++2b -Wall -g -pthread -MMD -MP # Añadido -pthread
LDFLAGS = -pthread # Añadido -pthread

TARGET = main
SRCS = main.cpp \
	   containers/ListsDemo.cpp \
	   containers/BinaryTreeDemo.cpp \
	   containers/AVLDemo.cpp \
	   containers/HeapDemo.cpp \
	   containers/HashDemo.cpp

OBJS = $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(LDFLAGS) $^ -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(OBJS:.o=.d) $(TARGET)

-include $(OBJS:.o=.d)

.PHONY: all clean