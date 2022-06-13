#!/bin/bash

assert() {
    expected="$1"
    input="$2"

    ./tinycc "$input" > tmp.s
    cc -o tmp tmp.s
    ./tmp
    
    actual="$?"

    if [ "$actual" = "$expected" ]; then
        echo "$input -> $actual"
    else
        echo "$input -> $expected expected, but got $actual"
        exit 1
    fi
}

assert 0 0
assert 222 222
assert 73 "42+134-103"
assert 81 " 3 - 122 + 200 "
assert 54 "4+5*10"
assert 99 "4/3+(99-1)"
assert 1 " ( 10 - 7 ) / 2 * 1 "
assert 7 "+7"
assert 72 "-8*(-9)"
assert 1 "3 == 3"
assert 1 "10 != 16"
assert 0 "0 < 1 <= 0"
assert 1 "4 > -7 == 3 >= 3"

echo OK
