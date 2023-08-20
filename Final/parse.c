#include "lib.h"
#include "variables.h"
#include "functions.h"
static void (*currentLineNumberPlusPlus)() = &increaseCurrentLineNumber;
static void (*resetCurrentLineCounter)() = &resetCurrentLineNumber;
void processToken(char *p, int *num, int *n, char *c, int *size, Bool *isValid)
{
    sscanf(p, "%d%n%c", num, n, c);
    if (*c == '.' && *n > 0)
        *isValid = yieldError(wrongArgumentTypeNotAnInteger);
    *num = atoi(p);
    if (!(*num) && *p != '0')
        *isValid = yieldError(expectedNumber);

    *n = *num = *c = 0;
    (*size)++;
}

Bool countAndVerifyDataArguments(char *line)
{
    Bool isValid = True;
    int size = 0, n = 0, num = 0;
    char c = 0;
    char args[MAX_LINE_LEN + 1] = {0}, *p;
    line = strstr(line, DATA) + strlen(DATA);

    strcpy(args, line);

    isValid = verifyCommaSyntax(args);
    p = strtok(line, ", \t\n\f\r");

    while (p != NULL)
    {
        processToken(p, &num, &n, &c, &size, &isValid);
        p = strtok(NULL, ", \t\n\f\r");
    }

    if (isValid)
        increaseDataCounter(size);

    return isValid;
}

Bool checkInitialCommas(char **s_ptr, int *commasCounter_ptr, Bool *isValid_ptr)
{
    char *s = *s_ptr;
    while ((*s == ',' || isspace(*s)) && *s != '\0')
    {
        if (*s == ',')
            (*commasCounter_ptr)++;
        s++;
    }
    if (!*s && *commasCounter_ptr > 0)
        return yieldError(wrongCommasSyntaxIllegalApearenceOfCommasInLine);
    else if (*s && strlen(s) && *commasCounter_ptr > 0)
        *isValid_ptr = yieldError(illegalApearenceOfCommaBeforeFirstParameter);

    *s_ptr = s;
    return *isValid_ptr;
}

Bool checkToken(char **s_ptr, Bool *insideToken_ptr, int *commasCounter_ptr, Bool *isFirstToken_ptr, Bool *isValid_ptr)
{
    char *s = *s_ptr;
    while (*s == ',' || isspace(*s))
    {
        if (*s == ',')
            (*commasCounter_ptr)++;
        s++;
    }
    if (*s && (isprint(*s) && !isspace(*s)))
        *insideToken_ptr = True;

    *s_ptr = s;
    return *isValid_ptr;
}

Bool verifyCommaSyntax(char *line)
{
    int commasCounter = 0;
    Bool insideToken = False;
    Bool isFirstToken = True;
    Bool isValid = True;
    char *s = trimFromLeft(line);

    isValid = checkInitialCommas(&s, &commasCounter, &isValid);
    commasCounter = 0;

    while (s && *s != '\0')
    {
        if (insideToken)
        {
            if (commasCounter > 1)
            {
                isValid = yieldError(wrongCommasSyntaxExtra);
                commasCounter = 1;
            }
            else if (commasCounter < 1 && !isFirstToken)
                isValid = yieldError(wrongCommasSyntaxMissing);

            isFirstToken = False;

            while (*s != '\0' && !isspace(*s) && *s != ',')
                s++;

            if (*s == ',' || isspace(*s))
            {
                insideToken = False;
                commasCounter = 0;
                s--;
            }
        }
        else
        {
            isValid = checkToken(&s, &insideToken, &commasCounter, &isFirstToken, &isValid);
        }
        s++;
    }

    if (commasCounter)
        isValid = yieldError(illegalApearenceOfCommaAfterLastParameter);

    return isValid;
}

Bool checkQuotes(char *args, char **opening, char **closing)
{
    *opening = strchr(args, '\"');
    if (!*opening || !**opening)
    {
        yieldError(expectedQuotes);
        yieldError(closingQuotesForStringIsMissing);
        return False;
    }
    else
    {
        *closing = strrchr(args, '\"');
        if (*opening == *closing && (*opening[0] == args[0]))
            return yieldError(closingQuotesForStringIsMissing);
        if (*opening == *closing && (*opening[0] != args[0]))
            return yieldError(expectedQuotes);
    }
    return True;
}

void calculateSizeAndUpdateDataCounter(char *opening, char *closing)
{
    int size = strlen(opening) - strlen(closing);
    increaseDataCounter(size);
}

Bool countAndVerifyStringArguments(char *line)
{
    char *args, *closing = 0, *opening = 0;
    args = strstr(line, STRING) + strlen(STRING);
    args = trimFromLeft(args);

    if (!*args)
        return yieldError(emptyStringDeclatretion);
    if (!checkQuotes(args, &opening, &closing))
        return False;

    calculateSizeAndUpdateDataCounter(opening, closing);
    return True;
}

Bool handleLabelToken(char *token, char *line)
{
    State (*globalState)() = &getGlobalState;
    char *next = 0;
    if (!isLabelDeclarationStrict(token))
    {
        char lineClone[MAX_LINE_LEN] = {0}, *rest = 0;
        strcpy(lineClone, line);
        yieldError(missingSpaceBetweenLabelDeclaretionAndInstruction);
        token = line;
        next = strchr(line, ':');
        next++;
        *next = '\0';
        rest = strchr(lineClone, ':');
        rest++;
        sprintf(line, "%s%c%s", token, ' ', rest);
        strncpy(lineClone, line, strlen(line));
        next = (*globalState)() == firstround ? strtok(lineClone, " \t\n\f\r") : strtok(lineClone, ", \t\n\f\r");
        return parseLine(next, line) && False;
    }
    else
    {
        next = (*globalState)() == firstround ? strtok(NULL, " \t\n\f\r") : strtok(NULL, ", \t\n\f\r");
        if (!next)
            return yieldError(emptyLabelDecleration);

        if ((*globalState)() == firstround)
            return handleLabel(token, next, line);
        else
            return parseLine(next, line + strlen(token) + 1);
    }
}

Bool handleInstructionToken(char *token, char *line)
{
    State (*globalState)() = &getGlobalState;
    char *next;
    int type;
    type = getInstructionType(token);
    if (!isInstructionStrict(token))
    {
        yieldError(missinSpaceAfterInstruction);
        token = getInstructionName(token);
    }
    next = (*globalState)() == firstround ? strtok(NULL, " \t\n\f\r") : strtok(NULL, ", \t\n\f\r");

    if (next == NULL)
    {
        if (type == _TYPE_DATA || type == _TYPE_STRING)
            return type == _TYPE_DATA ? yieldWarning(emptyDataDeclaretion) : yieldError(emptyStringDeclatretion);
        else
            return type == _TYPE_ENTRY ? yieldWarning(emptyEntryDeclaretion) : yieldWarning(emptyExternalDeclaretion);
    }
    else
    {
        if ((*globalState)() == firstround)
            return handleInstruction(type, token, next, line);
        else
        {
            if (type == _TYPE_DATA)
                return writeDataInstruction(next);
            else if (type == _TYPE_STRING)
                return writeStringInstruction(next);
            else
                return True;
        }
    }
}

Bool parseLine(char *token, char *line)
{
    State (*globalState)() = &getGlobalState;

    if (isComment(token))
        return True;

    if (isLabelDeclaration(token))
        return handleLabelToken(token, line);

    if (isInstruction(token))
        return handleInstructionToken(token, line);

    if (isOperation(token))
    {
        char args[MAX_LINE_LEN] = {0};
        strcpy(args, (line + strlen(token)));
        return (*globalState)() == firstround ? handleOperation(token, args) : writeOperationBinary(token, args);
    }

    if (strlen(token) > 1)
        return yieldError(undefinedTokenNotOperationOrInstructionOrLabel);
    else
        return yieldError(illegalApearenceOfCharacterInTheBegningOfTheLine);
}

Bool handleSingleLine(char *line)
{
    State (*globalState)() = &getGlobalState;
    char lineCopy[MAX_LINE_LEN] = {0};
    Bool result = True;
    char *token;
    strcpy(lineCopy, line);
    token = ((*globalState)() == firstround) ? strtok(lineCopy, " \t\n\f\r") : strtok(lineCopy, ", \t\n\f\r");
    result = parseLine(token, line);
    (*currentLineNumberPlusPlus)();
    return result;
}
void printRunState(void)
{
    State (*globalState)() = &getGlobalState;
    char *(*fileName)() = &getFileNamePath;

    if ((*globalState)() == secondround)
        printf("\n\n\nSecond Run:(%s)\n", (*fileName)());
    else if ((*globalState)() == firstround)
        printf("\n\n\nFirst Run:(%s)\n", (*fileName)());
}

Bool processLine(char *line, int i)
{
    if (i > 0)
    {
        Bool isValidCode = handleSingleLine(line);
        memset(line, 0, MAX_LINE_LEN);
        return isValidCode;
    }
    return True;
}

void parseAssemblyCode(FILE *src)
{
    State (*globalState)() = &getGlobalState;
    void (*setState)(State) = &setGlobalState;
    int c = 0, i = 0;
    char line[MAX_LINE_LEN] = {0};
    Bool isValidCode = True;
    State nextState;

    (*resetCurrentLineCounter)();
    printRunState();

    while (((c = fgetc(src)) != EOF))
    {
        if (isspace(c) && i > 0)
            line[i++] = ' ';
        else if (!isspace(c))
            line[i++] = c;

        if (i >= MAX_LINE_LEN - 2 || c == '\n')
        {
            isValidCode = processLine(line, i) && isValidCode;
            i = 0;
        }
    }

    isValidCode = processLine(line, i) && isValidCode;

    nextState = !isValidCode ? assemblyCodeFailedToCompile : ((*globalState)() == firstround ? secondround : exportFiles);

    (*resetCurrentLineCounter)();
    (*setState)(nextState);
}
