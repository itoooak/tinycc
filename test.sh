#!/bin/bash

assert() {
    expected="$1"
    input="$2"

    cc -o tinycc tinycc.c
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

echo OK
