function fail()
{
    echo "ERROR"
    exit 1
}

function unittest()
{
    echo "$1" > _test.in
    ./main < _test.in > _test.out
    echo "$2" > _test.ans
    diff _test.out _test.ans || fail
}

#unittest "abc" "SCALAR(abc, plain)
#EOF
#string(abc)"
#unittest "123" "SCALAR(123, plain)
#EOF
#string(123)"
#unittest "
#- abc
#- def" "BLOCK-SEQUENCE-START
#BLOCK-ENTRY
#SCALAR(abc, plain)
#BLOCK-ENTRY
#SCALAR(def, plain)
#BLOCK-END
#EOF
#list(string(abc), string(def))"
#
#unittest "
#-
#    - def
#    - ghi
#- jkl" "BLOCK-SEQUENCE-START
#BLOCK-ENTRY
#BLOCK-SEQUENCE-START
#BLOCK-ENTRY
#SCALAR(def, plain)
#BLOCK-ENTRY
#SCALAR(ghi, plain)
#BLOCK-END
#BLOCK-ENTRY
#SCALAR(jkl, plain)
#BLOCK-END
#EOF
#list(list(string(def), string(ghi)), string(jkl))"
#
#unittest "
#- item 1
#- item 2
#-
#  - item 3.1
#  - item 3.2" "BLOCK-SEQUENCE-START
#BLOCK-ENTRY
#SCALAR(item 1, plain)
#BLOCK-ENTRY
#SCALAR(item 2, plain)
#BLOCK-ENTRY
#BLOCK-SEQUENCE-START
#BLOCK-ENTRY
#SCALAR(item 3.1, plain)
#BLOCK-ENTRY
#SCALAR(item 3.2, plain)
#BLOCK-END
#BLOCK-END
#EOF
#list(string(item 1), string(item 2), list(string(item 3.1), string(item 3.2)))"
#
#unittest "
#- - item 1
#  - item 2" "BLOCK-SEQUENCE-START
#BLOCK-ENTRY
#BLOCK-SEQUENCE-START
#BLOCK-ENTRY
#SCALAR(item 1, plain)
#BLOCK-ENTRY
#SCALAR(item 2, plain)
#BLOCK-END
#BLOCK-END
#EOF
#list(list(string(item 1), string(item 2)))"
#
#unittest "
#abc: def
#ghi: jkl" "BLOCK-MAPPING-START
#KEY
#SCALAR(abc, plain)
#VALUE
#SCALAR(def, plain)
#KEY
#SCALAR(ghi, plain)
#VALUE
#SCALAR(jkl, plain)
#BLOCK-END
#EOF
#map(ghi: string(jkl), abc: string(def))"
#
#unittest "
#-
#  key 1: value 1
#  key 2: value 2" "BLOCK-SEQUENCE-START
#BLOCK-ENTRY
#BLOCK-MAPPING-START
#KEY
#SCALAR(key 1, plain)
#VALUE
#SCALAR(value 1, plain)
#KEY
#SCALAR(key 2, plain)
#VALUE
#SCALAR(value 2, plain)
#BLOCK-END
#BLOCK-END
#EOF
#list(map(key 2: string(value 2), key 1: string(value 1)))"
#
#unittest "
#- - item 1
#  - item 2
#- key 1: value 1
#  key 2: value 2" "BLOCK-SEQUENCE-START
#BLOCK-ENTRY
#BLOCK-SEQUENCE-START
#BLOCK-ENTRY
#SCALAR(item 1, plain)
#BLOCK-ENTRY
#SCALAR(item 2, plain)
#BLOCK-END
#BLOCK-ENTRY
#BLOCK-MAPPING-START
#KEY
#SCALAR(key 1, plain)
#VALUE
#SCALAR(value 1, plain)
#KEY
#SCALAR(key 2, plain)
#VALUE
#SCALAR(value 2, plain)
#BLOCK-END
#BLOCK-END
#EOF
#list(list(string(item 1), string(item 2)), map(key 2: string(value 2), key 1: string(value 1)))"
#
#unittest "
#key:
#- item 1
#- item 2" "BLOCK-MAPPING-START
#KEY
#SCALAR(key, plain)
#VALUE
#BLOCK-ENTRY
#SCALAR(item 1, plain)
#BLOCK-ENTRY
#SCALAR(item 2, plain)
#BLOCK-END
#EOF
#map(key: list(string(item 1), string(item 2)))"

unittest "abc" "string(abc)"

unittest "123" "string(123)"

unittest "
- abc
- def" "list(string(abc), string(def))"

unittest "
-
    - def
    - ghi
- jkl" "list(list(string(def), string(ghi)), string(jkl))"

unittest "
- item 1
- item 2
-
  - item 3.1
  - item 3.2" "list(string(item 1), string(item 2), list(string(item 3.1), string(item 3.2)))"

unittest "
- - item 1
  - item 2" "list(list(string(item 1), string(item 2)))"

unittest "
abc: def
ghi: jkl" "map(ghi: string(jkl), abc: string(def))"

unittest "
-
  key 1: value 1
  key 2: value 2" "list(map(key 2: string(value 2), key 1: string(value 1)))"

unittest "
- - item 1
  - item 2
- key 1: value 1
  key 2: value 2" "list(list(string(item 1), string(item 2)), map(key 2: string(value 2), key 1: string(value 1)))"

unittest "
key:
- item 1
- item 2" "map(key: list(string(item 1), string(item 2)))"
