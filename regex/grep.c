#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define max_insns 9999

static void error(const char *plaint) {
    fprintf(stderr, "%s\n", plaint);
    exit(1);
}

enum { op_expect, op_jump, op_split };
static int ops[max_insns], args[max_insns];
static unsigned cp = 0;  // compile pointer

static char visited[max_insns];
static char set0[max_insns], set1[max_insns];
static char *agenda = set0, *next = set1;

static void spread(unsigned pc, char *result) {
    for (;;) { switch (ops[--pc]) {
        case op_expect:
            result[pc] = 1;
            return;
        case op_jump:
            pc = args[pc];
            break;
        case op_split:
            if (visited[pc]) return;
            visited[pc] = 1;
            spread(args[pc], result);
            break;
        }
    }
}

static int match(unsigned start, const char *s) {
    memset(agenda, 0, cp);
    memset(visited, 0, cp);
    spread(start, agenda);
    for (; *s; ++s) {
        memset(next, 0, cp);
        memset(visited, 0, cp);
        for (unsigned pc = 1; pc < cp; ++pc)
            if (agenda[pc] && *s == args[pc])
                spread(pc, next);
        char *tmp = agenda; agenda = next; next = tmp;
    }
    return agenda[0];  // (0 is the accepting state)
}

static void really_emit(int op, int arg) {
    if (max_insns <= cp) error("Pattern too long");
    ops[cp] = op;
    args[cp] = arg;
    ++cp;
}

static unsigned emit(int op, int arg, unsigned k) { // k for continuation
    if (cp != k) really_emit(op_jump, k);
    really_emit(op, arg);
    return cp;
}

static const char *pattern, *pp; // start, current parsing position

static int eat(char c) {
    return pattern < pp && pp[-1] == c ? (--pp, 1) : 0;
}

static unsigned parsing(int precedence, unsigned k) {
    unsigned rhs;
    if (pattern == pp || pp[-1] == '(' || pp[-1] == '|')
        rhs = k;
    else if (eat(')')) {
        rhs = parsing(0, k);
        if (!eat('(')) error("Mismatched ')'");
    }
    else if (eat('*')) {
        rhs = emit(op_split, 0, k); // (The 0 is a placeholder...
        args[rhs-1] = parsing(6, rhs); // ... filled in here.)
    }
    else
        rhs = emit(op_expect, *--pp, k);
    while (pattern < pp && pp[-1] != '(') {
        int prec = pp[-1] == '|' ? 2 : 4;
        if (prec < precedence) break;
        if (eat('|'))
            rhs = emit(op_split, rhs, parsing(prec+1, k));
        else
            rhs = parsing(prec+1, rhs);
    }
    return rhs;
}

static unsigned parse(const char *string) {
    pattern = string; pp = pattern + strlen(pattern);
    unsigned k = parsing(0, emit(op_expect, '\0', 0)); // no input char == '\0'
    if (pattern != pp) error("Bad pattern");
    return k;
}

int main(int argc, char **argv) {
    if (argc != 2) error("usage");
    unsigned start = parse(argv[1]);
    char line[9999];
    while (fgets(line, sizeof line, stdin)) {
        line[strlen(line) - 1] = '\0';
        if (match(start, line))
            puts(line);
    }
    return 0;
}
