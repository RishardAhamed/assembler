#include "lib.h"
#include "variables.h"
#include "functions.h"

static const Operation operations[OP_SIZE] = {
    {0x0001, 0, "mov", {1, 1, 1, 1}, {0, 1, 1, 1}},
    {0x0002, 0, "cmp", {1, 1, 1, 1}, {1, 1, 1, 1}},
    {0x0004, 10, "add", {1, 1, 1, 1}, {0, 1, 1, 1}},
    {0x0004, 11, "sub", {1, 1, 1, 1}, {0, 1, 1, 1}},
    {0x0010, 0, "lea", {0, 1, 1, 0}, {0, 1, 1, 1}},
    {0x0020, 10, "clr", {0, 0, 0, 0}, {0, 1, 1, 1}},
    {0x0020, 11, "not", {0, 0, 0, 0}, {0, 1, 1, 1}},
    {0x0020, 12, "inc", {0, 0, 0, 0}, {0, 1, 1, 1}},
    {0x0020, 13, "dec", {0, 0, 0, 0}, {0, 1, 1, 1}},
    {0x0200, 10, "jmp", {0, 0, 0, 0}, {0, 1, 1, 0}},
    {0x0200, 11, "bne", {0, 0, 0, 0}, {0, 1, 1, 0}},
    {0x0200, 12, "jsr", {0, 0, 0, 0}, {0, 1, 1, 0}},
    {0x1000, 0, "red", {0, 0, 0, 0}, {0, 1, 1, 1}},
    {0x2000, 0, "prn", {0, 0, 0, 0}, {1, 1, 1, 1}},
    {0x4000, 0, "rts", {0, 0, 0, 0}, {0, 0, 0, 0}},
    {0x8000, 0, "stop", {0, 0, 0, 0}, {0, 0, 0, 0}},
};

int startsWith(const char *s1, const char *s2)
{
    while (*s1 && *s2)
    {
        if (*s1 != *s2)
        {
            return 1;
        }
        s1++;
        s2++;
    }
    return *s2 ? 1 : 0;
}

int findOperationIndex(char *s, int (*comparator)(const char *, const char *))
{
    int i = 0;
    while (i < OP_SIZE)
    {
        if (comparator(operations[i].keyword, s) == 0)
            return i;
        i++;
    }
    return -1;
}

Bool isOperationName(char *s)
{
    return findOperationIndex(s, strcmp) != -1 ? True : False;
}

const Operation *getOperationByName(char *s)
{
    int i = findOperationIndex(s, strcmp);
    return i != -1 ? &operations[i] : NULL;
}

const Operation *getOperationByIndex(unsigned int i)
{
    return i < OP_SIZE ? &operations[i] : NULL;
}

const char *getOperationName(char *s)
{
    int i = findOperationIndex(s, startsWith);
    return i != -1 ? operations[i].keyword : NULL;
}

int getOpIndex(char *s)
{
    return findOperationIndex(s, strcmp);
}

Bool isOperationNotStrict(char *s)
{
    return findOperationIndex(s, startsWith) != -1 ? True : False;
}