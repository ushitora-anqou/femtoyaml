main: main.cpp femtoyaml.hpp
	clang++ -std=c++17 -g3 -O0 main.cpp -o $@

test: main test.sh
	./test.sh

.PHONY: test
