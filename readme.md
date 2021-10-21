# ezforth

Forth compiler made in ANSI C89.

## Quick start

Save this simple program in ```program.ezf``` file.

```forth
: star 42 emit ;
: margin cr 20 spaces ;
: blip  margin star ;
: dblip margin star 3 spaces star ;
: mblip margin 2 spaces star 2 spaces ;
: bar margin star star star star star ;

: print_F     bar   blip  bar   blip  blip ;
: print_O     bar   dblip dblip dblip bar  ;
: print_R     bar   dblip bar   blip  space star blip 2 spaces star ;
: print_T     bar   mblip mblip mblip mblip ;
: print_H     dblip dblip bar   dblip dblip ;

: hello print_F cr print_O cr print_R cr print_T cr print_H cr ;
hello

0 exit
```

Compile it with the following command:

```bash
./ezforth program.ezf
```

Run it.

```bash
./program.exe
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
- [ ] Hardcode ezforthlib.s inside the C source code (one for every platform)
- [ ] Multi-platform compiler
