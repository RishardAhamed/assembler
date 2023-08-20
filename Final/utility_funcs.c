#include "lib.h"
#include "variables.h"
#include "functions.h"

#define MAX_REG_NAME_SIZE 4

char regs[REGS_SIZE][MAX_REG_NAME_SIZE] = {"r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10",
                                           "r11", "r12", "r13", "r14", "r15"};
Bool isMacroOpening(char *s)
{
    return !strcmp(s, "macro") ? True : False;
}

Bool isMacroClosing(char *s)
{
    return !strcmp(s, "endm") ? True : False;
}
Bool isPossiblyUseOfMacro(char *s)
{
    return !isLabelDeclaration(s) && !isOperation(s) && !isComment(s) && !isInstructionStrict(s) && !isMacroClosing(s) && !isMacroOpening(s);
}

Bool isLegalMacroName(char *s)
{
    return !isInstructionStrict(s) && !isOperation(s) ? True : False;
}

Bool isInstruction(char *s)
{

    if ((!strcmp(s, DATA) || !strcmp(s, STRING) || !strcmp(s, ENTRY) || !strcmp(s, EXTERNAL)))
        return True;

    else if (strstr(s, DATA) != NULL || strstr(s, STRING) != NULL || strstr(s, ENTRY) != NULL || strstr(s, EXTERNAL) != NULL)
    {
        /*         yieldError(missinSpaceAfterInstruction); */
        return True;
    }
    else
        return False;
}

Bool isInstructionStrict(char *s)
{

    return ((!strcmp(s, DATA) || !strcmp(s, STRING) || !strcmp(s, ENTRY) || !strcmp(s, EXTERNAL))) ? True : False;
}

Bool isRegistery(char *s)
{
    int i;
    int len = strlen(s);
    if (s[0] == 'r' && len >= 2)
    {
        for (i = 0; i < REGS_SIZE; i++)
        {
            if (strcmp(regs[i], s) == 0)
                return True;
        }
    }
    return False;
}
Bool isValidImmediateParamter(char *s)
{
    int i, len = strlen(s);
    if (len < 2 || s[0] != '#' || (!(s[1] == '-' || s[1] == '+' || isdigit(s[1]))))
        return False;
    for (i = 2; i < len; i++)
        if (!isdigit(s[i]))
            return False;
    return True;
}
Bool hasValidBrackets(char *s)
{
    int len = strlen(s);
    char *opening = strchr(s, '[');
    char *closing = strchr(s, ']');
    if (!opening || !closing || closing < opening || s[len - 1] != ']')
    {
        return False;
    }
    return True;
}

Bool checkInnerRegistry(char *s)
{
    char *opening;
    char *closing;
    Bool result;

    opening = strchr(s, '[');
    opening++;
    closing = strchr(s, ']');
    *closing = '\0';

    result = isRegistery(opening);

    *closing = ']';

    return result;
}

Bool isIndexParameter(char *s)
{
    if (strlen(s) < 5)
        return False;
    if (!hasValidBrackets(s))
        return False;
    return checkInnerRegistry(s);
}

Bool isValidIndexParameter(char *s)
{
    char *opening;
    int len = strlen(s);

    if (len < 6)
        return False;

    if (!(s[len - 1] == ']' && s[len - 4] == 'r' && s[len - 5] == '['))
    {
        return False;
    }

    if (!checkInnerRegistry(s))
        return False;

    opening = strchr(s, '[');
    opening++;

    if (isRegistery(opening) && getRegisteryNumber(opening) < 10)
    {
        return False;
    }

    return True;
}

Bool isComment(char *s)
{
    s = trimFromLeft(s);
    return s[0] == ';' ? True : False;
}
Bool isOperation(char *s)
{
    return (getOperationByName(s) != NULL) ? True : False;
}

Bool isLabelDeclarationStrict(char *s)
{
    return s[strlen(s) - 1] == ':' ? True : False;
}

Bool isLabelDeclaration(char *s)
{
    return strchr(s, ':') != NULL ? True : False;
}
int getInstructionType(char *s)
{
    if (strstr(s, DATA))
        return _TYPE_DATA;
    if (strstr(s, STRING))
        return _TYPE_STRING;
    if (strstr(s, ENTRY))
        return _TYPE_ENTRY;
    if (strstr(s, EXTERNAL))
        return _TYPE_EXTERNAL;
    return False;
}
Bool isReservedKeyword(char *s)
{
    return isRegistery(s) || isOperationName(s);
}
char *getInstructionNameByType(int type)
{
    char *instructionName = NULL;

    switch (type)
    {
    case _TYPE_DATA:
        instructionName = DATA;
        break;
    case _TYPE_STRING:
        instructionName = STRING;
        break;
    case _TYPE_ENTRY:
        instructionName = ENTRY;
        break;
    case _TYPE_EXTERNAL:
        instructionName = EXTERNAL;
        break;
    default:
        break;
    }

    return instructionName ? strcat(instructionName, "\0") : NULL;
}

char *getInstructionName(char *s)
{
    if (strstr(s, DATA) != NULL)
        return DATA;
    if (strstr(s, STRING) != NULL)
        return STRING;
    if (strstr(s, ENTRY) != NULL)
        return ENTRY;
    if (strstr(s, EXTERNAL) != NULL)
        return EXTERNAL;

    return 0;
}

Bool verifyLabelNaming(char *s)
{
    int i;
    int labelLength = strlen(s);

    if (!isalpha(s[0]))
        return False;

    if (labelLength > MAX_LABEL_LEN || labelLength < 1)
        return False;

    if (isReservedKeyword(s))
        return False;

    for (i = 0; i < labelLength; i++)
    {
        if (!isalnum(s[i]))
            return False;
    }

    return True;
}

int getRegisteryNumber(char *s)
{
    s++;
    return atoi(s);
}

Bool checkStartsWithAlpha(char *s)
{
    if (isalpha(s[0]) == 0)
    {
        return yieldError(illegalLabelNameUseOfCharacters);
    }
    return True;
}

Bool checkValidLength(char *s)
{
    int labelLength = strlen(s);
    if (labelLength > MAX_LABEL_LEN || labelLength < 1)
    {
        return yieldError(illegalLabelNameLength);
    }
    return True;
}

Bool checkNotReservedKeywords(char *s)
{
    if (isRegistery(s))
    {
        return yieldError(illegalLabelNameUseOfSavedKeywordUsingRegisteryName);
    }
    if (isOperationName(s))
    {
        return yieldError(illegalLabelNameUseOfSavedKeywordUsingOperationName);
    }
    return True;
}

Bool checkValidCharacters(char *s)
{
    int i;
    i = 0;
    for (i = 0; s[i]; i++)
    {
        if (!isalnum(s[i]))
        {
            return yieldError(illegalLabelNameUseOfCharacters);
        }
    }
    return True;
}

Bool verifyLabelNamingAndPrintErrors(char *s)
{
    if (!checkStartsWithAlpha(s))
    {
        return False;
    }
    if (!checkValidLength(s))
    {
        return False;
    }
    if (!checkNotReservedKeywords(s))
    {
        return False;
    }
    if (!checkValidCharacters(s))
    {
        return False;
    }
    return True;
}
