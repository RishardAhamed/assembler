#include "lib.h"
#include "variables.h"
#include "functions.h"

Bool handleTwoOperands(const Operation *op, char *first, char *second, AddrMethodsOptions active[2]);
Bool handleSingleOperand(const Operation *op, char *first, AddrMethodsOptions active[2]);

void handleFirstOperandForSecondWord(unsigned *word, char *operand, AddrMethodsOptions active);
void handleSecondOperandForSecondWord(unsigned *word, char *operand, AddrMethodsOptions active);

Bool writeOperationBinary(char *operationName, char *args)
{
    const Operation *op = getOperationByName(operationName);
    char *first = strtok(args, ", \t\n\f\r");
    char *second = strtok(NULL, ", \t\n\f\r");
    AddrMethodsOptions active[2] = {{0, 0, 0, 0}, {0, 0, 0, 0}};

    unsigned firstWord = (A << 16) | op->op;
    addWord(firstWord, Code);

    if (first && second)
    {
        return handleTwoOperands(op, first, second, active);
    }
    else if (!second && first)
    {
        return handleSingleOperand(op, first, active);
    }
    else if (!first && !second && !op->funct)
    {
        return True;
    }
    else
    {
        return False;
    }
}

Bool handleTwoOperands(const Operation *op, char *first, char *second, AddrMethodsOptions active[2])
{
    if (detectOperandType(first, active, 0) && detectOperandType(second, active, 1))
    {
        writeSecondWord(first, second, active, op);
        writeAdditionalOperandsWords(op, active[0], first);
        writeAdditionalOperandsWords(op, active[1], second);
        return True;
    }
    return False;
}

Bool handleSingleOperand(const Operation *op, char *first, AddrMethodsOptions active[2])
{
    char *second = first;
    if (detectOperandType(first, active, 1))
    {
        writeSecondWord(first, second, active, op);
        writeAdditionalOperandsWords(op, active[1], second);
        return True;
    }
    return False;
}

void writeAdditionalOperandsWords(const Operation *op, AddrMethodsOptions active, char *value)
{
    if (active.index)
    {
        char *labelName = parseLabelNameFromIndexAddrOperand(value);
        writeDirectOperandWord(labelName);
    }
    else if (active.direct)
    {
        writeDirectOperandWord(value);
    }
    else if (active.immediate)
    {
        writeImmediateOperandWord(value);
    }
}

Bool processTokenAndAddWord(char *token)
{
    int num = atoi(token);
    addWord((A << 16) | num, Data);
    return True;
}

char *getNextToken()
{
    return strtok(NULL, ", \t\n\f\r");
}

Bool writeDataInstruction(char *token)
{
    while (token)
    {
        if (!processTokenAndAddWord(token))
        {
            return False;
        }
        token = getNextToken();
    }
    return True;
}

void addStringCharactersToData(char *start)
{
    int i, len = strlen(start);
    for (i = 0; i < len - 2; i++)
    {
        addWord((A << 16) | start[i], Data);
    }
}

Bool writeStringInstruction(char *s)
{
    char *start = strchr(s, '\"') + 1;
    addStringCharactersToData(start);
    addWord((A << 16) | '\0', Data);
    return True;
}

void writeSecondWord(char *first, char *second, AddrMethodsOptions active[2], const Operation *op)
{
    unsigned secondWord = (A << 16) | (op->funct << 12);

    handleFirstOperandForSecondWord(&secondWord, first, active[0]);
    handleSecondOperandForSecondWord(&secondWord, second, active[1]);

    addWord(secondWord, Code);
}

void handleFirstOperandForSecondWord(unsigned *word, char *operand, AddrMethodsOptions active)
{
    if (operand && (active.reg || active.index))
    {
        *word |= active.reg ? (getRegisteryNumber(operand) << 8) : (parseRegNumberFromIndexAddrOperand(operand) << 8);
        *word |= active.reg ? (REGISTER_DIRECT_ADDR << 6) : (INDEX_ADDR << 6);
    }
    else if (operand && (active.direct || active.immediate))
    {
        *word |= (0 << 8) | (active.direct ? (DIRECT_ADDR << 6) : (IMMEDIATE_ADDR << 6));
    }
}

void handleSecondOperandForSecondWord(unsigned *word, char *operand, AddrMethodsOptions active)
{
    if (operand && (active.reg || active.index))
    {
        *word |= active.reg ? (getRegisteryNumber(operand) << 2) : (parseRegNumberFromIndexAddrOperand(operand) << 2);
        *word |= active.reg ? (REGISTER_DIRECT_ADDR) : (INDEX_ADDR);
    }
    else if (operand && (active.direct || active.immediate))
    {
        *word |= (0 << 2) | (active.direct ? (DIRECT_ADDR) : (IMMEDIATE_ADDR));
    }
}

unsigned getWordWithFlag(unsigned flag, unsigned value)
{
    return (flag << 16) | value;
}

void handleExternalLabel(char *labelName)
{
    unsigned offset;
    unsigned base = getIC();
    addWord(getWordWithFlag(E, 0), Code);
    offset = getIC();
    addWord(getWordWithFlag(E, 0), Code);
    updateExtPositionData(labelName, base, offset);
}

void handleInternalLabel(char *labelName)
{
    unsigned base = getSymbolBaseAddress(labelName);
    unsigned offset = getSymbolOffset(labelName);
    addWord(getWordWithFlag(R, base), Code);
    addWord(getWordWithFlag(R, offset), Code);
}

void writeDirectOperandWord(char *labelName)
{
    if (isExternal(labelName))
    {
        handleExternalLabel(labelName);
    }
    else
    {
        handleInternalLabel(labelName);
    }
}

void writeImmediateOperandWord(char *n)
{
    n++;
    addWord(getWordWithFlag(A, atoi(n)), Code);
}

Bool handleOperandType(char *operand, AddrMethodsOptions *active)
{
    if (isRegistery(operand))
    {
        active->reg = 1;
        return True;
    }
    else if (isValidImmediateParamter(operand))
    {
        active->immediate = 1;
        return True;
    }
    else if (isValidIndexParameter(operand))
    {
        active->index = 1;
        return True;
    }
    else if (isSymbolExist(operand))
    {
        if (isEntry(operand) && !isNonEmptyEntry(operand))
        {
            return yieldError(entryDeclaredButNotDefined);
        }
        active->direct = 1;
        return True;
    }
    else
    {
        return yieldError(labelNotExist);
    }
}

Bool detectOperandType(char *operand, AddrMethodsOptions active[2], int type)
{
    return handleOperandType(operand, &active[type]);
}

char *findOpeningBracket(char *s)
{
    return strchr(s, '[');
}

char *findClosingBracket(char *s)
{
    return strchr(s, ']');
}

char *parseLabelNameFromIndexAddrOperand(char *s)
{
    char *p = findOpeningBracket(s);
    if (p)
    {
        *p = 0;
    }
    return s;
}

void moveToNextCharacterAfterBracket(char **s)
{
    char *openingBracket = findOpeningBracket(*s);
    if (openingBracket)
    {
        *s = openingBracket + 1;
    }
}

int parseRegNumberFromIndexAddrOperand(char *s)
{
    char *p = findClosingBracket(s);
    moveToNextCharacterAfterBracket(&s);

    if (p)
    {
        *p = 0;
    }

    return getRegisteryNumber(s);
}
