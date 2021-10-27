/**
 * TODO:
 * [x] We cannot compile words as functions because pushing the ebp etc...
 *     (function preamble and postamble) will affect the stack and the words
 *     won't work anymore.
 *     Possible solution: we can define words as AST and then replace the word
 *     with its code (like inline functions or macros).
 * [ ] Make my own stdio.h since we only use printf (or make a tiny executable
 *     without depending on redist: https://youtube.com/watch?v=5tg_TbURMy0)
 * [ ] Is exit a standard function (ANSI Forth)? Check it.
 * [x] Cross-platform ezforthlib.s
 * [ ] T_MULDIV and T_MULDIVMOD
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdarg.h>

#define isspace(c)\
    ((c) ==  ' ' || (c) == '\t' || (c) == '\v' ||\
     (c) == '\n' || (c) == '\r' || (c) == '\f')
#define isdigit(c) ((c) >= '0' && (c) <= '9')

int
streq(char *s1, char *s2)
{
    while(*s1 && *s2)
    {
        if(*s1 != *s2)
        {
            return(0);
        }
        ++s1;
        ++s2;
    }

    return(*s1 == *s2);
}

void
strncopy(char *d, char *s, int n)
{
    int i;

    i = 0;
    while(i < n && *s)
    {
        *d = *s;
        ++s;
        ++d;
        ++i;
    }
}

char *srcfile;
char *src;
int srcl;

void
fatal(char *fmt, ...)
{
    va_list ap;

    printf("%s:%d: ", srcfile, srcl);

    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
    printf("\n");

    exit(1);
}

void
init(char *_srcfile, char *_src)
{
    srcfile = _srcfile;
    src = _src;
    srcl = 1;
}

enum
{
    T_EOF = 0,

    T_LPAREN,
    T_RPAREN,

    T_COLON,
    T_SEMI,

    T_WORD,
    T_INT,
    T_DOT,
    T_EMIT,
    T_SPACE,
    T_SPACES,
    T_CR,

    T_EXIT,

    T_SWAP,
    T_DUP,
    T_OVER,
    T_ROT,
    T_DROP,
    T_2SWAP,
    T_2DUP,
    T_2OVER,
    T_2DROP,

    T_NEGATE,
    T_ADD,
    T_SUB,
    T_MUL,
    T_DIV,
    T_MOD,
    T_DIVMOD,
    T_MULDIV,
    T_MULDIVMOD,

    T_1ADD,
    T_1SUB,
    T_2ADD,
    T_2SUB,
    T_2MUL,
    T_2DIV,

    T_ABS,
    T_MIN,
    T_MAX,

    T_INVERT,
    T_EQ,
    T_NEQ,
    T_LT,
    T_GT,
    T_0EQ,
    T_0LT,
    T_0GT,

    T_AND,
    T_OR,
    T_DUPIFNOT0,

    T_IF,
    T_ELSE,
    T_THEN,
    T_DO,
    T_LOOP,
    T_PLOOP,

    T_RIN,
    T_ROUT,
    T_RAT,

    T_COUNT
};

int token;
char word[32];
int tokenval;

int tokpb;
int tokpbval;
char wordpb[32];

/**
 * NOTE(driverfury):
 *
 * When a word is defined we store the position in the source file where it
 * starts. We don't need to store the end because we now that we need to
 * stop when the semicolon ';' is encountered (no nested word definitions
 * allowed).
 * 
 * When a word is "called" (expanded), we set the src to the word definition
 * start.
 *
 */
struct WordDef
{
    char name[32];
    char *code;
    int line;
};
struct WordDef wdefs[100];
int wdefsc;

void
pushwdef(char *name, char *start, int line)
{
    assert(wdefsc < 100);
    strncopy(wdefs[wdefsc].name, name, 32);
    wdefs[wdefsc].code = start;
    wdefs[wdefsc].line = line;
    ++wdefsc;
}

char *
getwdef(char *name)
{
    char *code;
    int i;

    code = 0;
    for(i = wdefsc - 1;
        i >= 0;
        --i)
    {
        if(streq(wdefs[i].name, name))
        {
            code = wdefs[i].code;
            break;
        }
    }

    return(code);
}

void
putback()
{
    int i;

    tokpb = token;
    tokpbval = tokenval;
    for(i = 0;
        i < 32;
        ++i)
    {
        wordpb[i] = word[i];
    }
}

int
next()
{
    int i;
    int isnum;
    int isneg;

    if(tokpb)
    {
        token = tokpb;
        tokenval = tokpbval;
        for(i = 0;
            i < 32;
            ++i)
        {
            word[i] = wordpb[i];
        }
        tokpb = 0;
    }
    else
    {
        token = T_EOF;
        i = 0;
        isnum = 1;
        isneg = 0;
        if(*src)
        {
            /* Skip spaces */
            while(isspace(*src))
            {
                if(*src == '\n')
                {
                    ++srcl;
                }
                ++src;
            }

            while(*src && !isspace(*src))
            {
                word[i] = *src;
                if(isnum && !isdigit(*src))
                {
                    if(i == 0 && *src == '-')
                    {
                        isneg = 1;
                    }
                    else
                    {
                        isnum = 0;
                    }
                }
                ++src;
                ++i;
            }
            word[i] = 0;

            if((i > 0 && isnum && !isneg) ||
               (i > 1 && isnum && isneg))
            {
                token = T_INT;
                i = 0;
                tokenval = 0;
                if(isneg)
                {
                    ++i;
                }
                while(word[i])
                {
                    tokenval = tokenval*10 + (word[i] - '0');
                    ++i;
                }
                if(isneg)
                {
                    tokenval = -tokenval;
                }
            }
            else
            {
                if(word[0])
                {
                         if(streq(word, "("))      { token = T_LPAREN; }
                    else if(streq(word, ")"))      { token = T_RPAREN; }
                    else if(streq(word, ":"))      { token = T_COLON; }
                    else if(streq(word, ";"))      { token = T_SEMI; }
                    else if(streq(word, "."))      { token = T_DOT; }
                    else if(streq(word, "emit"))   { token = T_EMIT; }
                    else if(streq(word, "space"))  { token = T_SPACE; }
                    else if(streq(word, "spaces")) { token = T_SPACES; }
                    else if(streq(word, "cr"))     { token = T_CR; }
                    else if(streq(word, "exit"))   { token = T_EXIT; }
                    else if(streq(word, "swap"))   { token = T_SWAP; }
                    else if(streq(word, "dup"))    { token = T_DUP; }
                    else if(streq(word, "over"))   { token = T_OVER; }
                    else if(streq(word, "rot"))    { token = T_ROT; }
                    else if(streq(word, "drop"))   { token = T_DROP; }
                    else if(streq(word, "2swap"))  { token = T_2SWAP; }
                    else if(streq(word, "2dup"))   { token = T_2DUP; }
                    else if(streq(word, "2over"))  { token = T_2OVER; }
                    else if(streq(word, "2drop"))  { token = T_2DROP; }
                    else if(streq(word, "negate")) { token = T_NEGATE; }
                    else if(streq(word, "+"))      { token = T_ADD; }
                    else if(streq(word, "-"))      { token = T_SUB; }
                    else if(streq(word, "*"))      { token = T_MUL; }
                    else if(streq(word, "/"))      { token = T_DIV; }
                    else if(streq(word, "mod"))    { token = T_MOD; }
                    else if(streq(word, "/mod"))   { token = T_DIVMOD; }
                    else if(streq(word, "*/"))     { token = T_MULDIV; }
                    else if(streq(word, "*/mod"))  { token = T_MULDIVMOD; }
                    else if(streq(word, "1+"))     { token = T_1ADD; }
                    else if(streq(word, "1-"))     { token = T_1SUB; }
                    else if(streq(word, "2+"))     { token = T_2ADD; }
                    else if(streq(word, "2-"))     { token = T_2SUB; }
                    else if(streq(word, "2*"))     { token = T_2MUL; }
                    else if(streq(word, "2/"))     { token = T_2DIV; }
                    else if(streq(word, "abs"))    { token = T_ABS; }
                    else if(streq(word, "min"))    { token = T_MIN; }
                    else if(streq(word, "max"))    { token = T_MAX; }
                    else if(streq(word, "invert")) { token = T_INVERT; }
                    else if(streq(word, "="))      { token = T_EQ; }
                    else if(streq(word, "<>"))     { token = T_NEQ; }
                    else if(streq(word, "<"))      { token = T_LT; }
                    else if(streq(word, ">"))      { token = T_GT; }
                    else if(streq(word, "0="))     { token = T_0EQ; }
                    else if(streq(word, "0<"))     { token = T_0LT; }
                    else if(streq(word, "0>"))     { token = T_0GT; }
                    else if(streq(word, "and"))    { token = T_AND; }
                    else if(streq(word, "or"))     { token = T_OR; }
                    else if(streq(word, "?dup"))   { token = T_DUPIFNOT0; }
                    else if(streq(word, "if"))     { token = T_IF; }
                    else if(streq(word, "else"))   { token = T_ELSE; }
                    else if(streq(word, "then"))   { token = T_THEN; }
                    else if(streq(word, "do"))     { token = T_DO; }
                    else if(streq(word, "loop"))   { token = T_LOOP; }
                    else if(streq(word, "+loop"))  { token = T_PLOOP; }
                    else if(streq(word, ">R"))     { token = T_RIN; }
                    else if(streq(word, "R>"))     { token = T_ROUT; }
                    else if(streq(word, "R@"))     { token = T_RAT; }
                    else
                    {
                        token = T_WORD;
                    }
                }
            }
        }
    }

    return(token);
}

void
expect(int kind)
{
    /* TODO */
    assert(next() == kind);
}

int lblcount = 3000;

char *
genlbl()
{
    char *l;
    int n;

    l = (char *)malloc(13);
    l[0] = '.';
    l[1] = 'L';
    n = snprintf(l+2, (size_t)10, "%d", lblcount);
    ++lblcount;
    l[n+2] = 0;

    return(l);
}

void
freelbl(char *l)
{
    free(l);
}

void
compileins(FILE *fout)
{
    char *lbl1;
    char *lbl2;
    int t;

    if(next() != T_EOF)
    {
        switch(token)
        {
            case T_LPAREN:
            {
                /* Skip comments */
                while(next() != T_RPAREN);
            } break;

            case T_COLON:
            {
                fatal("You cannot define a word inside a word definition!\n");
            } break;

            case T_WORD:
            {
                char *wcode;
                char *srcprev;
                int srclprev;

                srcprev = src;
                srclprev = srcl;
                wcode = getwdef(word);

                if(wcode)
                {
                    src = wcode;

                    t = next();
                    while(t != T_SEMI)
                    {
                        putback();
                        compileins(fout);
                        t = next();
                    }

                    src = srcprev;
                    srcl = srclprev;
                }
                else
                {
                    fatal("Unknown word '%s'", word);
                }
            } break;

            case T_INT:
            {
                fprintf(fout, "\tpushl $%d\n", tokenval);
            } break;

            case T_DOT:
            {
                fprintf(fout, "\tcall print_int\n");
                fprintf(fout, "\taddl $4,%%esp\n");
                fprintf(fout, "\tpushl $' '\n");
                fprintf(fout, "\tcall print_char\n");
                fprintf(fout, "\taddl $4,%%esp\n");
            } break;

            case T_EMIT:
            {
                fprintf(fout, "\tcall print_char\n");
                fprintf(fout, "\taddl $4,%%esp\n");
            } break;

            case T_SPACE:
            {
                fprintf(fout, "\tpushl $' '\n");
                fprintf(fout, "\tcall print_char\n");
                fprintf(fout, "\taddl $4,%%esp\n");
            } break;

            case T_SPACES:
            {
                lbl1 = genlbl();
                lbl2 = genlbl();

                fprintf(fout, "\tpushl $' '\n");
                fprintf(fout, "%s:\n", lbl1);
                fprintf(fout, "\tmovl 4(%%esp),%%eax\n");
                fprintf(fout, "\tcmpl $0,%%eax\n");
                fprintf(fout, "\tjle %s\n", lbl2);
                fprintf(fout, "\tcall print_char\n");
                fprintf(fout, "\tdecl 4(%%esp)\n");
                fprintf(fout, "\tjmp %s\n", lbl1);
                fprintf(fout, "%s:\n", lbl2);
                fprintf(fout, "\taddl $8,%%esp\n");

                freelbl(lbl1);
                freelbl(lbl2);
            } break;

            case T_CR:
            {
                fprintf(fout, "\tpushl $10\n");
                fprintf(fout, "\tcall print_char\n");
                fprintf(fout, "\taddl $4,%%esp\n");
            } break;

            case T_EXIT:
            {
                fprintf(fout, "\tjmp ezforth_exit\n");
            } break;

            case T_SWAP:
            {
                fprintf(fout, "\tpopl %%eax\n");
                fprintf(fout, "\tpopl %%edx\n");
                fprintf(fout, "\tpushl %%eax\n");
                fprintf(fout, "\tpushl %%edx\n");
            } break;

            case T_DUP:
            {
                fprintf(fout, "\tpopl %%eax\n");
                fprintf(fout, "\tpushl %%eax\n");
                fprintf(fout, "\tpushl %%eax\n");
            } break;

            case T_OVER:
            {
                fprintf(fout, "\tpushl 4(%%esp)\n");
            } break;

            case T_ROT:
            {
                fprintf(fout, "\tpopl %%ecx\n");
                fprintf(fout, "\tpopl %%ebx\n");
                fprintf(fout, "\tpopl %%eax\n");
                fprintf(fout, "\tpushl %%ebx\n");
                fprintf(fout, "\tpushl %%ecx\n");
                fprintf(fout, "\tpushl %%eax\n");
            } break;

            case T_DROP:
            {
                fprintf(fout, "\taddl $4,%%esp\n");
            } break;

            case T_2SWAP:
            {
                fprintf(fout, "\tpopl %%edx\n");
                fprintf(fout, "\tpopl %%ecx\n");
                fprintf(fout, "\tpopl %%ebx\n");
                fprintf(fout, "\tpopl %%eax\n");
                fprintf(fout, "\tpushl %%ecx\n");
                fprintf(fout, "\tpushl %%edx\n");
                fprintf(fout, "\tpushl %%eax\n");
                fprintf(fout, "\tpushl %%ebx\n");
            } break;

            case T_2DUP:
            {
                fprintf(fout, "\tpushl 4(%%esp)\n");
                fprintf(fout, "\tpushl 4(%%esp)\n");
            } break;

            case T_2OVER:
            {
                fprintf(fout, "\tpushl 12(%%esp)\n");
                fprintf(fout, "\tpushl 12(%%esp)\n");
            } break;

            case T_2DROP:
            {
                fprintf(fout, "\taddl $8,%%esp\n");
            } break;

            case T_NEGATE:
            {
                fprintf(fout, "\tpopl %%eax\n");
                fprintf(fout, "\tnegl %%eax\n");
                fprintf(fout, "\tpushl %%eax\n");
            } break;

            case T_ADD:
            {
                fprintf(fout, "\tpopl %%eax\n");
                fprintf(fout, "\tpopl %%edx\n");
                fprintf(fout, "\taddl %%edx,%%eax\n");
                fprintf(fout, "\tpushl %%eax\n");
            } break;

            case T_SUB:
            {
                fprintf(fout, "\tpopl %%edx\n");
                fprintf(fout, "\tpopl %%eax\n");
                fprintf(fout, "\tsubl %%edx,%%eax\n");
                fprintf(fout, "\tpushl %%eax\n");
            } break;

            case T_MUL:
            {
                fprintf(fout, "\tpopl %%eax\n");
                fprintf(fout, "\tpopl %%edx\n");
                fprintf(fout, "\timul %%edx\n");
                fprintf(fout, "\tpushl %%eax\n");
            } break;

            case T_DIV:
            {
                fprintf(fout, "\tpopl %%ebx\n");
                fprintf(fout, "\tpopl %%eax\n");
                fprintf(fout, "\tcdq\n");
                fprintf(fout, "\tidivl %%ebx\n");
                fprintf(fout, "\tpushl %%eax\n");
            } break;

            case T_MOD:
            {
                fprintf(fout, "\tpopl %%ebx\n");
                fprintf(fout, "\tpopl %%eax\n");
                fprintf(fout, "\tcdq\n");
                fprintf(fout, "\tidivl %%ebx\n");
                fprintf(fout, "\tpushl %%edx\n");
            } break;

            case T_DIVMOD:
            {
                fprintf(fout, "\tpopl %%ebx\n");
                fprintf(fout, "\tpopl %%eax\n");
                fprintf(fout, "\tcdq\n");
                fprintf(fout, "\tidivl %%ebx\n");
                fprintf(fout, "\tpushl %%edx\n");
                fprintf(fout, "\tpushl %%eax\n");
            } break;

            case T_MULDIV:
            {
                /* TODO */
                assert(0);
            } break;

            case T_MULDIVMOD:
            {
                /* TODO */
                assert(0);
            } break;

            case T_1ADD:
            {
                fprintf(fout, "\tpopl %%eax\n");
                fprintf(fout, "\tincl %%eax\n");
                fprintf(fout, "\tpushl %%eax\n");
            } break;

            case T_1SUB:
            {
                fprintf(fout, "\tpopl %%eax\n");
                fprintf(fout, "\tdecl %%eax\n");
                fprintf(fout, "\tpushl %%eax\n");
            } break;

            case T_2ADD:
            {
                fprintf(fout, "\tpopl %%eax\n");
                fprintf(fout, "\tincl %%eax\n");
                fprintf(fout, "\tincl %%eax\n");
                fprintf(fout, "\tpushl %%eax\n");
            } break;

            case T_2SUB:
            {
                fprintf(fout, "\tpopl %%eax\n");
                fprintf(fout, "\tdecl %%eax\n");
                fprintf(fout, "\tdecl %%eax\n");
                fprintf(fout, "\tpushl %%eax\n");
            } break;

            case T_2MUL:
            {
                fprintf(fout, "\tpopl %%eax\n");
                fprintf(fout, "\tsall %%eax\n");
                fprintf(fout, "\tpushl %%eax\n");
            } break;

            case T_2DIV:
            {
                fprintf(fout, "\tpopl %%eax\n");
                fprintf(fout, "\tmovl $2,%%ebx\n");
                fprintf(fout, "\tcdq\n");
                fprintf(fout, "\tidivl %%ebx\n");
                fprintf(fout, "\tpushl %%eax\n");
            } break;

            case T_ABS:
            {
                fprintf(fout, "\tpopl %%eax\n");
                fprintf(fout, "\tmovl %%eax,%%edx\n");
                fprintf(fout, "\tsarl $31,%%eax\n");
                fprintf(fout, "\tmovl %%eax,%%ebx\n");
                fprintf(fout, "\txorl %%edx,%%ebx\n");
                fprintf(fout, "\tsubl %%eax,%%ebx\n");
                fprintf(fout, "\tpushl %%ebx\n");
            } break;

            case T_MIN:
            {
                lbl1 = genlbl();
                lbl2 = genlbl();

                fprintf(fout, "\tpopl %%eax\n");
                fprintf(fout, "\tpopl %%ebx\n");
                fprintf(fout, "\tcmpl %%ebx,%%eax\n");
                fprintf(fout, "\tjl %s\n", lbl1);
                fprintf(fout, "\tpushl %%ebx\n");
                fprintf(fout, "\tjmp %s\n", lbl2);
                fprintf(fout, "%s:\n", lbl1);
                fprintf(fout, "\tpushl %%eax\n");
                fprintf(fout, "%s:\n", lbl2);

                freelbl(lbl1);
                freelbl(lbl2);
            } break;

            case T_MAX:
            {
                lbl1 = genlbl();
                lbl2 = genlbl();

                fprintf(fout, "\tpopl %%eax\n");
                fprintf(fout, "\tpopl %%ebx\n");
                fprintf(fout, "\tcmpl %%ebx,%%eax\n");
                fprintf(fout, "\tjg %s\n", lbl1);
                fprintf(fout, "\tpushl %%ebx\n");
                fprintf(fout, "\tjmp %s\n", lbl2);
                fprintf(fout, "%s:\n", lbl1);
                fprintf(fout, "\tpushl %%eax\n");
                fprintf(fout, "%s:\n", lbl2);

                freelbl(lbl1);
                freelbl(lbl2);
            } break;

            case T_INVERT:
            {
                fprintf(fout, "\tpopl %%eax\n");
                fprintf(fout, "\tnotl %%eax\n");
                fprintf(fout, "\tpushl %%eax\n");
            } break;

            case T_EQ:
            {
                lbl1 = genlbl();
                lbl2 = genlbl();

                fprintf(fout, "\tpopl %%ecx\n");
                fprintf(fout, "\tpopl %%eax\n");
                fprintf(fout, "\tcmp %%ecx,%%eax\n");
                fprintf(fout, "\tje %s\n", lbl1);
                fprintf(fout, "\tpushl $0\n");
                fprintf(fout, "\tjmp %s\n", lbl2);
                fprintf(fout, "%s:\n", lbl1);
                fprintf(fout, "\tpushl $-1\n");
                fprintf(fout, "%s:\n", lbl2);

                freelbl(lbl1);
                freelbl(lbl2);
            } break;

            case T_NEQ:
            {
                lbl1 = genlbl();
                lbl2 = genlbl();

                fprintf(fout, "\tpopl %%ecx\n");
                fprintf(fout, "\tpopl %%eax\n");
                fprintf(fout, "\tcmp %%ecx,%%eax\n");
                fprintf(fout, "\tjne %s\n", lbl1);
                fprintf(fout, "\tpushl $0\n");
                fprintf(fout, "\tjmp %s\n", lbl2);
                fprintf(fout, "%s:\n", lbl1);
                fprintf(fout, "\tpushl $-1\n");
                fprintf(fout, "%s:\n", lbl2);

                freelbl(lbl1);
                freelbl(lbl2);
            } break;

            case T_LT:
            {
                lbl1 = genlbl();
                lbl2 = genlbl();

                fprintf(fout, "\tpopl %%ecx\n");
                fprintf(fout, "\tpopl %%eax\n");
                fprintf(fout, "\tcmp %%ecx,%%eax\n");
                fprintf(fout, "\tjl %s\n", lbl1);
                fprintf(fout, "\tpushl $0\n");
                fprintf(fout, "\tjmp %s\n", lbl2);
                fprintf(fout, "%s:\n", lbl1);
                fprintf(fout, "\tpushl $-1\n");
                fprintf(fout, "%s:\n", lbl2);

                freelbl(lbl1);
                freelbl(lbl2);
            } break;

            case T_GT:
            {
                lbl1 = genlbl();
                lbl2 = genlbl();

                fprintf(fout, "\tpopl %%ecx\n");
                fprintf(fout, "\tpopl %%eax\n");
                fprintf(fout, "\tcmp %%ecx,%%eax\n");
                fprintf(fout, "\tjg %s\n", lbl1);
                fprintf(fout, "\tpushl $0\n");
                fprintf(fout, "\tjmp %s\n", lbl2);
                fprintf(fout, "%s:\n", lbl1);
                fprintf(fout, "\tpushl $-1\n");
                fprintf(fout, "%s:\n", lbl2);

                freelbl(lbl1);
                freelbl(lbl2);
            } break;

            case T_0EQ:
            {
                lbl1 = genlbl();
                lbl2 = genlbl();

                fprintf(fout, "\tpopl %%eax\n");
                fprintf(fout, "\tcmp $0,%%eax\n");
                fprintf(fout, "\tje %s\n", lbl1);
                fprintf(fout, "\tpushl $0\n");
                fprintf(fout, "\tjmp %s\n", lbl2);
                fprintf(fout, "%s:\n", lbl1);
                fprintf(fout, "\tpushl $-1\n");
                fprintf(fout, "%s:\n", lbl2);

                freelbl(lbl1);
                freelbl(lbl2);
            } break;

            case T_0LT:
            {
                lbl1 = genlbl();
                lbl2 = genlbl();

                fprintf(fout, "\tpopl %%eax\n");
                fprintf(fout, "\tcmp $0,%%eax\n");
                fprintf(fout, "\tjl %s\n", lbl1);
                fprintf(fout, "\tpushl $0\n");
                fprintf(fout, "\tjmp %s\n", lbl2);
                fprintf(fout, "%s:\n", lbl1);
                fprintf(fout, "\tpushl $-1\n");
                fprintf(fout, "%s:\n", lbl2);

                freelbl(lbl1);
                freelbl(lbl2);
            } break;

            case T_0GT:
            {
                lbl1 = genlbl();
                lbl2 = genlbl();

                fprintf(fout, "\tpopl %%eax\n");
                fprintf(fout, "\tcmp $0,%%eax\n");
                fprintf(fout, "\tjg %s\n", lbl1);
                fprintf(fout, "\tpushl $0\n");
                fprintf(fout, "\tjmp %s\n", lbl2);
                fprintf(fout, "%s:\n", lbl1);
                fprintf(fout, "\tpushl $-1\n");
                fprintf(fout, "%s:\n", lbl2);

                freelbl(lbl1);
                freelbl(lbl2);
            } break;

            case T_AND:
            {
                lbl1 = genlbl();
                lbl2 = genlbl();

                fprintf(fout, "\tpopl %%ecx\n");
                fprintf(fout, "\tpopl %%eax\n");
                fprintf(fout, "\tcmp $0,%%eax\n");
                fprintf(fout, "\tje %s\n", lbl1);
                fprintf(fout, "\tcmp $0,%%ecx\n");
                fprintf(fout, "\tje %s\n", lbl1);
                fprintf(fout, "\tpushl $-1\n");
                fprintf(fout, "\tjmp %s\n", lbl2);
                fprintf(fout, "%s:\n", lbl1);
                fprintf(fout, "\tpushl $0\n");
                fprintf(fout, "%s:\n", lbl2);

                freelbl(lbl1);
                freelbl(lbl2);
            } break;

            case T_OR:
            {
                lbl1 = genlbl();
                lbl2 = genlbl();

                fprintf(fout, "\tpopl %%ecx\n");
                fprintf(fout, "\tpopl %%eax\n");
                fprintf(fout, "\tcmp $0,%%eax\n");
                fprintf(fout, "\tjne %s\n", lbl1);
                fprintf(fout, "\tcmp $0,%%ecx\n");
                fprintf(fout, "\tjne %s\n", lbl1);
                fprintf(fout, "\tpushl $0\n");
                fprintf(fout, "\tjmp %s\n", lbl2);
                fprintf(fout, "%s:\n", lbl1);
                fprintf(fout, "\tpushl $-1\n");
                fprintf(fout, "%s:\n", lbl2);

                freelbl(lbl1);
                freelbl(lbl2);
            } break;

            case T_DUPIFNOT0:
            {
                lbl1 = genlbl();

                fprintf(fout, "\tpopl %%eax\n");
                fprintf(fout, "\tcmp $0,%%eax\n");
                fprintf(fout, "\tje %s\n", lbl1);
                fprintf(fout, "\tpushl %%eax\n");
                fprintf(fout, "%s:\n", lbl1);
                fprintf(fout, "\tpushl %%eax\n");

                freelbl(lbl1);
            } break;

            case T_IF:
            {
                int haselse;

                lbl1 = genlbl();
                lbl2 = genlbl();

                fprintf(fout, "\tpopl %%eax\n");
                fprintf(fout, "\tcmp $0,%%eax\n");
                fprintf(fout, "\tje %s\n", lbl1);

                haselse = 0;
                t = next();
                putback();
                while(t != T_THEN)
                {
                    if(t == T_ELSE)
                    {
                        haselse = 1;
                        expect(T_ELSE);
                        fprintf(fout, "\tjmp %s\n", lbl2);
                        fprintf(fout, "%s:\n", lbl1);
                    }

                    compileins(fout);

                    t = next();
                    putback();
                }
                expect(T_THEN);

                if(haselse)
                {
                    fprintf(fout, "%s:\n", lbl2);
                }
                else
                {
                    fprintf(fout, "%s:\n", lbl1);
                }

                freelbl(lbl1);
                freelbl(lbl2);
            } break;

            case T_DO:
            {
                lbl1 = genlbl();

                fprintf(fout, "%s:\n", lbl1);

                t = next();
                putback();
                while(t != T_LOOP && t != T_PLOOP)
                {
                    compileins(fout);
                    t = next();
                    putback();
                }
                expect(t);

                if(t == T_PLOOP)
                {
                    fprintf(fout, "\tpopl %%eax\n");
                    fprintf(fout, "\tpopl %%ecx\n");
                    fprintf(fout, "\tpopl %%edx\n");
                    fprintf(fout, "\taddl %%eax,%%ecx\n");
                }
                else
                {
                    fprintf(fout, "\tpopl %%ecx\n");
                    fprintf(fout, "\tpopl %%edx\n");
                    fprintf(fout, "\tpushl %%edx\n");
                    fprintf(fout, "\tincl %%ecx\n");
                }
                fprintf(fout, "\tpushl %%edx\n");
                fprintf(fout, "\tpushl %%ecx\n");
                fprintf(fout, "\tcmpl %%edx,%%ecx\n");
                fprintf(fout, "\tjl %s\n", lbl1);
                fprintf(fout, "\tpopl %%ecx\n");
                fprintf(fout, "\tpopl %%edx\n");

                freelbl(lbl1);
            } break;

            case T_RIN:
            {
                fprintf(fout, "\tpopl %%eax\n");
                fprintf(fout, "\tmovl rstackp,%%ebx\n");
                fprintf(fout, "\tsubl $4,%%ebx\n");
                fprintf(fout, "\tmovl %%ebx,rstackp\n");
                fprintf(fout, "\tmovl %%eax,(rstackp)\n");
            } break;

            case T_ROUT:
            {
                fprintf(fout, "\tmovl (rstackp),%%eax\n");
                fprintf(fout, "\tmovl rstackp,%%ebx\n");
                fprintf(fout, "\tsubl $4,%%ebx\n");
                fprintf(fout, "\tmovl %%ebx,rstackp\n");
                fprintf(fout, "\tpushl %%eax\n");
            } break;

            case T_RAT:
            {
                fprintf(fout, "\tmovl (rstackp),%%eax\n");
                fprintf(fout, "\tpushl %%eax\n");
            } break;

            default:
            {
                fatal("Invalid instruction!");
            } break;
        }
    }
}

void
compile(FILE *fout)
{
    int t;
    char *wcode;

    fprintf(fout, ".code32\n");
    fprintf(fout, ".text\n");
    fprintf(fout, ".include \"ezforthlib.s\"\n");
    fprintf(fout, ".global _main\n");
    fprintf(fout, "_main:\n");
    fprintf(fout, "\tpushl %%ebp\n");
    fprintf(fout, "\tmovl %%esp,%%ebp\n");
    fprintf(fout, "\tmovl $rstack_end,rstackp\n");
    t = next();
    putback();
    while(t != T_EOF)
    {
        if(token == T_COLON)
        {
            next();
            expect(T_WORD);
            wcode = src;
            pushwdef(word, wcode, srcl);

            do
            {
                t = next();
            }
            while(t != T_SEMI);
        }
        else
        {
            compileins(fout);
        }
        t = next();
        putback();
    }

    fprintf(fout, "\tmovl $0,%%eax\n");
    fprintf(fout, "\tleave\n");
    fprintf(fout, "\tret\n");
    fprintf(fout, "\n");
}

void
usage(char *pname)
{
    printf("Usage: %s <source_file>\n", pname);
}

int
main(int argc, char *argv[])
{
    FILE *fout;
    FILE *fin;
    char *fnameout;
    char *fnamein;
    long fsizein;
    char *srcin;
    long cread;

    if(argc < 2)
    {
        usage(argv[0]);
        return(0);
    }

    fnameout = "test.s";
    fout = fopen(fnameout, "w");
    if(!fout)
    {
        printf("[!] ERROR: Cannot open output file '%s'\n", fnameout);
        exit(1);
    }

    fnamein = argv[1];
    fin = fopen(fnamein, "r");
    if(!fin)
    {
        printf("[!] ERROR: Cannot open input file '%s'\n", fnamein);
        exit(1);
    }

    fseek(fin, 0, SEEK_END);
    fsizein = ftell(fin);
    fseek(fin, 0, SEEK_SET);
    srcin = (char *)malloc(fsizein + 1);
    cread = fread(srcin, 1, fsizein, fin);
    if(!cread)
    {
        printf("[!] ERROR: Cannot read the file '%s'\n", fnamein);
        exit(1);
    }
    fclose(fin);
    srcin[cread] = 0;

    init(fnamein, srcin);
    compile(fout);
    fclose(fout);

    return(0);
}
