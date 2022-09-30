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

assert 0 "return 0;"
assert 222 "return 222;"
assert 73 "return 42+134-103;"
assert 81 "return 3 - 122 + 200 ;"
assert 54 "return 4+5*10;"
assert 99 "return 4/3+(99-1);"
assert 1 "return ( 10 - 7 ) / 2 * 1 ;"
assert 7 "return +7;"
assert 72 "return -8*(-9);"
assert 1 "return 3 == 3;"
assert 1 "return 10 != 16;"
assert 0 "return 0 < 1 <= 0;"
assert 1 "return 4 > -7 == 3 >= 3;"
assert 39 "a = -1; b = 42; z = 3; return (z*a)+b;"
assert 4 "c = d = 2; e = c * d; f = e * (1 / c * e); return e;"
assert 88 "eight = 8; eleven = 11; return eight * eleven;"
assert 3 "_ni=2; y_4=_ni*2; return (y_4+_ni)/2;"
assert 5 "___g_o=5; _=1; __=0; return ___g_o + __; __; _;"
assert 2 "if (1 == 1) return 2; 3; 4;"
assert 0 "if (2 * 3 != 6) return 25; else return -9 - (-9);"
assert 1 "return1 = 1; if (0) 1; return return1;"
assert 1 "ifa = 3; else_ =-1; return1 = 1; if (0) return 1; return ifa + else_ - return1;"
assert 2 "c = 1; if (c == 10) return 1; else if (0 == c-1) return 2; else return 3;"
assert 128 "i = 1; while (i < 100) i = i*2; return i;"
assert 1 "i = 1; while (1 == 2) i; return i;"
assert 45 "s = 0; for (i=1; i<10; i=i+1) s = s+i; return s;"
assert 6 "i = 0; for (; i<5; i=i+3) i=i; return i;"
assert 7 "i=0; for(;;) if (i == 7) return i; else i = i+1;"

echo OK
