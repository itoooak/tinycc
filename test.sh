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

assert 0 "0;"
assert 222 "222;"
assert 73 "42+134-103;"
assert 81 " 3 - 122 + 200 ;"
assert 54 "4+5*10;"
assert 99 "4/3+(99-1);"
assert 1 " ( 10 - 7 ) / 2 * 1 ;"
assert 7 "+7;"
assert 72 "-8*(-9);"
assert 1 "3 == 3;"
assert 1 "10 != 16;"
assert 0 "0 < 1 <= 0;"
assert 1 "4 > -7 == 3 >= 3;"
assert 39 "a = -1; b = 42; z = 3; (z*a)+b;"
assert 4 "c = d = 2; e = c * d; f = e * (1 / c * e); e;"
assert 88 "eight = 8; eleven = 11; eight * eleven;"
assert 3 "_ni=2; y_4=_ni*2; (y_4+_ni)/2;"
assert 5 "___g_o=5; _=1; __=0; return ___g_o + __; __; _;"
assert 2 "if (1 == 1) return 2; 3; 4;"
assert 0 "if (2 * 3 != 6) return 25; else return -9 - (-9);"
assert 1 "return1 = 1; if (0) 1; return1;"
assert 1 "ifa = 3; else_ =-1; return1 = 1; if (0) 1; ifa + else_ - return1;"

echo OK
