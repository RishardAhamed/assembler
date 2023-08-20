#include "lib.h"
#include "variables.h"
#include "functions.h"

static void processLine(char *line, int len, FILE *src, FILE *target);
static void handleRemainingContent(int len, char *line, FILE *src, FILE *target);
static void resetLine(char *line, char *lineClone, int *index);
void (*setState)(State) = &setGlobalState;
State (*globalState)() = &getGlobalState;

void handleMacroOpening(char **next, char *macroName, long *start, FILE *src)
{
    *next = strtok(NULL, " \t\n\f\r");

    if (*next == NULL)
    {
        yieldError(macroDeclaretionWithoutDefiningMacroName);
        (*setState)(assemblyCodeFailedToCompile);
        return;
    }
    if (!isLegalMacroName(*next))
    {
        yieldError(illegalMacroNameUseOfSavedKeywords);
        (*setState)(assemblyCodeFailedToCompile);
        return;
    }
    *start = ftell(src);
    strcpy(macroName, *next);
}

void handleMacroClosing(char *macroName, long *start, long *end, char *line, FILE *src)
{
    *end = ftell(src) - strlen(line) + 1;
    addMacro(macroName, *start, *end);
    *start = *end = 0;
    memset(macroName, 0, MAX_LABEL_LEN);
}

void handleMacroUsage(char *token, char *line, FILE *src, FILE *target)
{
    Item *p = getMacro(token);
    if (p != NULL)
    {
        long c, toCopy = p->val.m.end - p->val.m.start;
        long lastPosition = 0;
        fseek(target, -strlen(line), SEEK_CUR);
        fprintf(target, "%s", "\0");
        lastPosition = ftell(src);
        fseek(src, p->val.m.start, SEEK_SET);

        for (; --toCopy && (c = fgetc(src)) != EOF;)
            fputc(c, target);

        fseek(src, lastPosition, SEEK_SET);
    }
}

void parseMacros(char *line, char *token, FILE *src, FILE *target)
{
    void (*currentLineNumberPlusPlus)() = &increaseCurrentLineNumber;
    static char macroName[MAX_LABEL_LEN] = {0}, *next;
    static Bool isReadingMacro = False;
    static long start = 0, end = 0;
    extern Bool isPossiblyUseOfMacro(char *s);
    extern Bool isMacroOpening(char *s);
    extern Bool isMacroClosing(char *s);

    if (!isReadingMacro && !isMacroOpening(token))
    {
        fprintf(target, "%s", line);
        (*currentLineNumberPlusPlus)();
    }
    if (isPossiblyUseOfMacro(token) || isMacroOpening(token) || isMacroClosing(token))
    {
        if (isMacroOpening(token))
        {
            handleMacroOpening(&next, macroName, &start, src);
            isReadingMacro = True;
        }
        else if (isMacroClosing(token))
        {
            handleMacroClosing(macroName, &start, &end, line, src);
            isReadingMacro = False;
        }
        else
        {
            handleMacroUsage(token, line, src, target);
        }
    }
}

void parseSourceFile(FILE *src, FILE *target)
{
    char line[MAX_LINE_LEN] = {0};
    char c;
    int i = 0;

    void (*resetCurrentLineCounter)() = &resetCurrentLineNumber;
    (*resetCurrentLineCounter)();

    while ((c = fgetc(src)) != EOF)
    {
        line[i++] = c;

        if (i >= MAX_LINE_LEN - 2 && !isspace(c))
            c = '\n';

        if ((*globalState)() == assemblyCodeFailedToCompile)
            return;

        if (c == '\n' && i > 0)
        {
            processLine(line, i, src, target);
            i = 0;
        }
    }

    handleRemainingContent(i, line, src, target);

    if ((*globalState)() != assemblyCodeFailedToCompile)
        (*setState)(firstround);
}

static void processLine(char *line, int len, FILE *src, FILE *target)
{
    char *token;
    char lineClone[MAX_LINE_LEN] = {0};
    strncpy(lineClone, line, len);
    token = strtok(lineClone, " \t\n\f\r");

    if (token)
        parseMacros(line, token, src, target);

    resetLine(line, lineClone, &len);
}

static void handleRemainingContent(int len, char *line, FILE *src, FILE *target)
{
    if (len > 0)
        processLine(line, len, src, target);
}

static void resetLine(char *line, char *lineClone, int *index)
{
    memset(lineClone, 0, *index);
    memset(line, 0, *index);
}
