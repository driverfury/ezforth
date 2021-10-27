# ezforth

Forth compiler made in ANSI C89.

Target platforms:
- [x] Linux
- [ ] Win32

## Quick start

You can find a simple hello world program ```test/hlowrld.ezf``` file:

```forth
: star 42 emit ;
: margin cr 30 spaces ;
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
./ezforth test/hlowrld.ezf
```

Run it.

```bash
./a.out
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

## Features

- [x] Stack manipulation ops
- [x] Words (macros)
- [x] Conditional blocks (if, do)
- [ ] Conditional (begin, while, again, until ...)
- [x] Return stack
- [ ] I/O
- [ ] Source file inclusion ('include' op)
- [ ] Hardcode flib.s inside the C source code
- [x] Our own assembler? Ambitious
