# FemtoYAML

Tiny header-only YAML parser for C++.

# Example usage

```cpp
#include <iostream>
#include "femtoyaml.hpp"

int main()
{
    using namespace femtoyaml;
    auto v =
        femtoyaml::load_string("- item 1\n- key 1:\n  - item 2\n  - - item 3");
    std::cout << v.get<list>()[1]
                     .get<map>()["key 1"]
                     .get<list>()[1]
                     .get<list>()[0]
                     .get<std::string>()
              << std::endl; // will output "item 3"
}
```

# Why femto?

Inspired by PicoJSON.

# License

MIT.
