#include <iostream>
#include "femtoyaml.hpp"

int main()
{
    auto src = femtoyaml::load_string(femtoyaml::load(std::cin).to_string());
    std::cout << src.to_string();
}
