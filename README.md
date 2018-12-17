# FemtoYAML

Yet another tiny header-only YAML parser/serializer for C++.

## Example usage

```yaml
- item 0
- item 1
- key 0: value 0
  key 1:
      key 2: item 2
      key 3:
          - item 3
          - item 4
  key 2: value 1
- item 5
```

```cpp
#include <fstream>
#include <iostream>
#include "femtoyaml.hpp"

int main()
{
    // Use femtoyaml::load() to read a YAML file
    // or you can read a YAML string directly by using femtoyaml::load_string().
    std::ifstream ifs("example.yml");
    auto yml = femtoyaml::load(ifs);

    // You can get the value inside the container by using get():
    //     string: get<std::string>()
    //     list:   get<femtoyaml::list>()
    //     map:    get<femtoyaml::map>()
    // In actual, femtoyaml::list is an alias of std::vector<femtoyaml::value>
    // and femtoyaml::map is std::unordered_map<std::string, femtoyaml::value>.
    std::cout
        << yml.get<femtoyaml::list>()[0].get<std::string>();  // output "item 0"

    // You can use operator[] to access elements of lists or maps (hash).
    std::cout
        << yml[2]["key 1"]["key 3"][1].get<std::string>();  // output "item 4"

    // Want to do different jobs depending on the content? value::visit() is
    // perfect for you! value::visit() takes functions as its arguments and call
    // them depending on the content's type.
    yml.visit(
        [&](const femtoyaml::list& lst) {
            std::cout << "List: size=" << lst.size();
        },
        [&](const auto&) {
            std::cout << "Not list";
        });  // output "List: size=4"

    // Also you can use value::is_*() functions.
    if (yml[0].is_string()) std::cout << "first item: string";
    if (yml[2].is_map()) std::cout << "third item: map";

    // FemtoYAML is not only a YAML parser but also a YAML serializer.
    // You can use value::to_string() to serialize a value into YAML,
    std::cout << yml.to_string();  // output a YAML formatted string.
    // or use femtoyaml::serialize()
    femtoyaml::serialize(std::cout, yml);  // output the same
}

```

## Why femto?

Inspired by PicoJSON.

## License

MIT.

## How to parse YAML?

YAML is formatted by indent, so it is easy to read for humans but
not so easy for computers. YAML parsers should always consider where they are
i.e. how many spaces they ate, which is relatively difficult.

FemtoYAML (and maybe many other implementations such as libyaml) solve this problem
by separating it up into two steps: tokenizing and (real) parsing.
The latter step is relatively simple (LL(1) parser),
so what we need to think carefully is the former.

In the first step, or 'tokenizing step',
we read the input string and split it into small parts or 'tokens'.
Here, we convert the structures using indents to ones using (virtual)
parentheses.

For example, let's say the input YAML string is:

```yaml
- - item 1
  - item 2
- key 1: value 1
  key 2: value 2
```

Then what we get after tokenizing is:

```
BLOCK-SEQUENCE-START
BLOCK-ENTRY
BLOCK-SEQUENCE-START
BLOCK-ENTRY
SCALAR(item 1, plain)
BLOCK-ENTRY
SCALAR(item 2, plain)
BLOCK-END
BLOCK-ENTRY
BLOCK-MAPPING-START
KEY
SCALAR(key 1, plain)
VALUE
SCALAR(value 1, plain)
KEY
SCALAR(key 2, plain)
VALUE
SCALAR(value 2, plain)
BLOCK-END
BLOCK-END
EOF
```

Here, we can find not indents but `BLOCK-SEQUENCE-START`, `BLOCK-MAPPING-START`
and `BLOCK-END`, which are marks for starts and ends of lists and maps.
`BLOCK-SEQUENCE-START` and `BLOCK-MAPPING-START` are something like
open parentheses and `BLOCK-END` is like close parentheses.
They are nested corresponding to increase of indents.

Theses tokens are much easier to parse.

(Maybe to be continued...)
