src = $(wildcard ../sources/*.cpp)
hdr = $(wildcard ../sources/*.hpp)

CXXFLAGS = -g -std=c++17 -Wall -Wextra -Wshadow -pedantic -D_GLIBCXX_DEBUG -fsanitize=address -fsanitize=undefined
CXXRLSFLAGS = -O2 -std=c++17 -Wall -Wextra -Wshadow -pedantic

test: test.cpp $(src) $(hdr)
	$(CXX) -o test $(CXXFLAGS) test.cpp $(src)

.PHONY: clean
clean:
	rm -f test