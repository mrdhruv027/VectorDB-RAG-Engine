CXX ?= g++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra

ifeq ($(OS),Windows_NT)
    LDFLAGS = -lws2_32 -D_WIN32_WINNT=0x0A00
    TARGET = db.exe
    TEST_TARGET = run_tests.exe
else
    LDFLAGS = -lpthread
    TARGET = db
    TEST_TARGET = run_tests
endif

all: $(TARGET) $(TEST_TARGET)

$(TARGET): main.cpp src/*.hpp httplib.h
	$(CXX) $(CXXFLAGS) main.cpp -o $(TARGET) $(LDFLAGS)

$(TEST_TARGET): src/tests.cpp src/*.hpp
	$(CXX) $(CXXFLAGS) src/tests.cpp -o $(TEST_TARGET) $(LDFLAGS)

test: $(TEST_TARGET)
	./$(TEST_TARGET)

clean:
	rm -f db db.exe run_tests run_tests.exe

.PHONY: all test clean
