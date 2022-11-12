#!/bin/bash

cat << EOF > tmp2.c
int nine() { return 9; }
int seven() { return 7; }
int mean2(int a, int b) { return (a + b) / 2; }
int sum6(int a, int b, int c, int d, int e, int f) { return a+b+c+d+e+f; }
int a(int a, int b) { return 2*a + b; }
EOF
cc -c tmp2.c

assert() {
    expected="$1"
    input="$2"

    ./tinycc "$input" > tmp.s
    cc -o tmp tmp.s tmp2.o
    ./tmp
    
    actual="$?"

    if [ "$actual" = "$expected" ]; then
        echo "$input"
        echo "-> $actual"
    else
        echo "$input -> $expected expected, but got $actual"
        exit 1
    fi
}

assert 0 "int main(){ return 0; }"
assert 222 "int main(){ return 222; }"
assert 73 "int main(){ return 42+134-103; }"
assert 81 "int main(){ return 3 - 122 + 200 ; }"
assert 54 "int main(){ return 4+5*10; }"
assert 99 "int main(){ return 4/3+(99-1); }"
assert 1 "int main(){ return ( 10 - 7 ) / 2 * 1 ; }"
assert 7 "int main(){ return +7; }"
assert 72 "int main(){ return -8*(-9); }"
assert 1 "int main(){ return 3 == 3; }"
assert 1 "int main(){ return 10 != 16; }"
assert 0 "int main(){ return 0 < 1 <= 0; }"
assert 1 "int main(){ return 4 > -7 == 3 >= 3; }"

assert 39 "int main(){ int a; a=-1; int b; b=42; int z; z=3; return (z*a)+b; }"
assert 4 "int main(){ int c; int d; c=d=2; int e; e=c*d; int f; f=e*(1/c*e); return e; }"
assert 88 "int main(){ int eight; eight=8; int eleven; eleven=11; return eight*eleven; }"
assert 3 "int main(){ int _ni; _ni=2; int y_4; y_4=_ni*2; return (y_4+_ni)/2; }"
assert 5 "int main(){ int ___g_o; ___g_o=5; int _; _=1; int __; __=0; return ___g_o + __; __; _; }"
assert 2 "int main(){ if (1==1) return 2; 3; 4; }"
assert 0 "int main(){ if (2*3!=6) return 25; else return -9-(-9); }"
assert 1 "int main(){ int return1; return1=1; if (0) 1; return return1; }"
assert 1 "int main(){ int ifa; ifa=3; int else_; else_=-1; int return1; return1=1; if (0) return 1; return ifa+else_-return1; }"
assert 2 "int main(){ int c; c=1; if (c==10) return 1; else if (0==c-1) return 2; else return 3; }"
assert 128 "int main(){ int i; i=1; while (i<100) i=i*2; return i; }"
assert 1 "int main(){ int i; i=1; while (1==2) i; return i; }"
assert 45 "int main(){ int s; s=0; int i; for (i=1; i<10; i=i+1) s=s+i; return s; }"
assert 6 "int main(){ int i; i=0; for (; i<5; i=i+3) i=i; return i; }"
assert 7 "int main(){ int i; i=0; for(;;) if (i==7) return i; else i=i+1; }"
assert 10 "int main(){ int s; s=0; int i; for(i=0; i<=4;) { s=s+i; i=i+1; } return s; }"
assert 0 "int main(){ int idx; idx=10; while(idx!=0) { idx; idx=idx-1; idx; } return idx; }"

assert 2 "int main(){ return nine()-seven(); }"
assert 63 "int main(){ int s; s=0; int i; for (i=0; i<seven(); i=i+1) { s=s+nine(); } return s; }"
assert 15 "int main(){ return mean2(19,11); }"
assert 21 "int main(){ return sum6(1,2,3,4,5,6); }"
assert 2 "int main(){ return mean2(sum6(1,2,4-1,8/2,-1,-3),-2); }"
assert 5 "int main(){ return a(2,1); }"
assert 4 "int main(){ return a(1,2); }"

assert 5 "int b(int a, int b){ return a*3+b; } int main() { int a; int b; a=1; b=2; return b(a,b); }"
assert 13 \
"int fib(int n) { if (n==1) return 0; else if (n==2) return 1; else return fib(n-1)+fib(n-2); }
int main() { return fib(1+3+1+3); }"

assert 9 "int main() { int a; a=9; return *&a; }"
assert 7 "int main() { int a; int *b; int **c; int **d; a=7; b=&a; c=&b; d=&b; if (c==d) return **c; else return 0; }"
assert 5 "int main() { int a; int *b; a=12; b=&a; *b=*b-7; return a; }"
assert 3 "int main() { int a; int b; a=3; b=5; return *(&b+(&a-&b)); }"

assert 3 "int foo(int a, int b) { int c; c = 3; return *(&b+8); } int main() { return foo(1, 3); }"

echo OK
