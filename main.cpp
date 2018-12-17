#include <iostream>
#include "femtoyaml.hpp"

int main()
{
    auto src = femtoyaml::load(std::cin);
    std::cout << src.to_debug_string() << std::endl;
    std::cout << src.to_string();
}
