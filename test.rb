#!/usr/bin/ruby

require "yaml"

def test(input)
  open("_test.in", "w") do |fh|
    fh.puts input
  end
  `./main < _test.in > _test.out`
  fail "[ERROR] #{input}" unless YAML.load(open("_test.out").read) == YAML.load(input)
end

test "abc"

test "123"

test "
- abc
- def"

test "
-
    - def
    - ghi
- jkl"

test "
- item 1
- item 2
-
  - item 3.1
  - item 3.2"

test "
- - item 1
  - item 2"

test "
abc: def
ghi: jkl"

test "
-
  key 1: value 1
  key 2: value 2"

test "
- - item 1
  - item 2
- key 1: value 1
  key 2: value 2"

test "
key:
- item 1
- item 2"
