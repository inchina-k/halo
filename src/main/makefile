src = $(wildcard ../sources/*.cpp)
hdr = $(wildcard ../sources/*.hpp)

CXXFLAGS = -g -std=c++17 -Wall -Wextra -Wshadow -pedantic -D_GLIBCXX_DEBUG -fsanitize=address -fsanitize=undefined
CXXRLSFLAGS = -O2 -std=c++17 -Wall -Wextra -Wshadow -pedantic

main: main.cpp $(src) $(hdr)
	$(CXX) -o main $(CXXFLAGS) main.cpp $(src)

.PHONY: release
release:
	$(CXX) -o main $(CXXRLSFLAGS) main.cpp $(src)

.PHONY: clean
clean:
	rm -f main