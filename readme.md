# ezforth

Forth compiler made in ANSI C89.

Target platforms:
- [x] Linux
- [ ] Win32

## How to build?

No make file needed. Just:

```bash
./build.sh
```

Note: gcc is needed. If you have another compiler just compile ```src/ezforth.c``` source file.

## Quick start

You can find a simple hello world program ```test/hlowrld.ezf``` file:

```forth
: star 42 emit ;
: margin cr 10 spaces ;
: blip  margin star ;
: dblip margin star 3 spaces star ;
: mblip margin 2 spaces star 2 spaces ;
: bar  margin star star star star star ;

: F bar   blip  bar   blip  blip  cr ;
: O bar   dblip dblip dblip bar   cr ;
: R bar   dblip bar   blip space star blip 2 spaces star cr ;
: T bar   mblip mblip mblip mblip cr ;
: H dblip dblip bar   dblip dblip cr ;

F O R T H cr

0 exit
```

Compile it with the following command:

```bash
./ezforth test/hlowrld.ezf hello
```

Run it.

```bash
./hello
```

This will be the output:

```
          *****
          *
          *****
          *
          *

          *****
          *   *
          *   *
          *   *
          *****

          *****
          *   *
          *****
          * *
          *  *

          *****
            *
            *
            *
            *

          *   *
          *   *
          *****
          *   *
          *   *
```

## Features and TODOs

- [x] Stack manipulation ops
- [x] Words (macros)
- [x] Conditional blocks (if, do)
- [ ] Conditional (begin, while, again, until ...)
- [x] Return stack
- [x] Variables and constants
- [ ] Arrays
- [ ] I/O
- [ ] Exit and quit inside a word definition
- [ ] Source file inclusion ('include' op)
- [ ] Hardcode flib.s inside the C source code
- [x] Our own assembler

## Documentation

### Introduction

This is an implementation of a Forth-like language (almost all instruction are ANSI Forth).

We only deal with words, which in our case (x86 platform) are 32 bits wide.

### Comments

You can put comments between parenthesis (spaces required after the '(').

```forth
( This is a comment )
( This is a comment too)
```

Or line comments like so:

```forth
\ Anything from the '\' til the enf of the line will be considered as a comment
```

### Stack manipulation

You can push a number onto the stack by typing it like so:

```forth
590
```

You can also push a negative number (no space between minus '-' and the number).

```forth
-8433
```

Stack manipulation instructions:
```forth
swap        \ a b     -- b a            Swap the last elements of the stack
dup         \ n       -- n n            Duplicate the last element of the stack
over        \ a b     -- a b a          Duplicate the second-last element of the stack
rot         \ a b c   -- b c a          Rotate the last 3 elements of the stack to the left
drop        \ n       --                Pop the last element of the stack
2swap       \ a b c d -- c d a b        Swap the last 2 pairs of elements of the stack
2dup        \ a b     -- a b a b        Duplicate the last pair of elements of the stack
2over       \ a b c d -- a b c d a b    Duplicate the second-last pair of the stack
2drop       \ a b     --                Drop the last 2 elements of the stack
```

### Arithmetic

Arithmetic instructions:

```forth
+           \ a b     -- sum            Push a+b (a and b popped) onto the stack
-           \ a b     -- sub            Push a-b (a and b popped) onto the stack
*           \ a b     -- mul            Push a*b (a and b popped) onto the stack
/           \ a b     -- quot           Push a/b (a and b popped) onto the stack
mod         \ a b     -- rem            Push the remainder of a/b (a and b popped) onto the stack
/mod        \ a b     -- rem quot       Push the remainder and quotient of a/b (a and b popped) onto the stack

neg         \ a       -- neg            Push -a (a popped) onto the stack
1+          \ a       -- inc            Push a+1 (a popped) onto the stack
1-          \ a       -- dec            Push a-1 (a popped) onto the stack
abs         \ a       -- abslt          Push the absolute value of a (a popped) onto the stack
min         \ a b     -- mn             Push the minimum between a and b (a and b popped) onto the stack
max         \ a b     -- mx             Push the maximum between a and b (a and b popped) onto the stack
```

### Bitwise operations

```forth
invert      \ a       -- n              Push a with flipped bits (a popped) onto the stack
and         \ a       -- d              Push a&b (a and b popped) onto the stack
or          \ a       -- r              Push a|b (a and b popped) onto the stack
xor         \ a       -- x              Push a^b (a and b popped) onto the stack
2*          \ a       -- sl             Push a shifted left (a popped) onto the stack
2/          \ a       -- sr             Push a shifted right (a popped) onto the stack
```

### Boolean logic

Logic instructions:

```forth
true        \         -- t              Push -1 (true value) onto the stack
false       \         -- 0              Push 0 (false value) onto the stack
=           \ a b     -- v              Push true (a and b popped) onto the stack if a==b otherwise push false
<>          \ a b     -- v              Push true (a and b popped) onto the stack if a!=b otherwise push false
<           \ a b     -- v              Push true (a and b popped) onto the stack if a<b otherwise push false
<=          \ a b     -- v              Push true (a and b popped) onto the stack if a<=b otherwise push false
>           \ a b     -- v              Push true (a and b popped) onto the stack if a>b otherwise push false
>=          \ a b     -- v              Push true (a and b popped) onto the stack if a>=b otherwise push false
0=          \ a       -- v              Push (a popped) true onto the stack if a==0 otherwise push false
0<          \ a       -- v              Push (a popped) true onto the stack if a<0 otherwise push false
0>          \ a       -- v              Push (a popped) true onto the stack if a>0 otherwise push false
?dup        \ a       -- a (a)          Duplicate a if a!=0
```
