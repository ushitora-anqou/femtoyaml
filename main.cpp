#include <iostream>
#include "femtoyaml.hpp"

int main()
{
    std::cout << femtoyaml::load(std::cin).to_string() << std::endl;
}
