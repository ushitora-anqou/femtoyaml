main: main.cpp femtoyaml.hpp
	g++ -o $@ -std=c++17 -g3 -O0 $^

test: main test.sh
	./test.sh

.PHONY: test
