#include "lib.h"
#include "variables.h"
#include "functions.h"

int processArguments(char *args, char **first, char **second, char **extra)
{
    int argCount = 0;
    *first = strtok(args, ", \t\n\f\r");
    if (*first)
    {
        argCount++;
        *second = strtok(NULL, ", \t\n\f\r");
        if (*second)
        {
            argCount++;
            *extra = strtok(NULL, ", \t\n\f\r");
            if (*extra)
                argCount++;
        }
    }
    return argCount;
}

/* Computes the size based on the active operations*/
int computeSize(const Operation *p, AddrMethodsOptions active[2])
{
    int size = 2;
    if (active[0].immediate || active[1].immediate)
        size++;
    if ((active[0].direct || active[0].index) || (active[1].direct || active[1].index))
        size += 2;
    if (!p->funct && (!active[0].direct && !active[0].immediate && !active[0].index && !active[0].reg) && (!active[1].direct && !active[1].immediate && !active[1].index && !active[1].reg))
        size = 1;
    return size;
}

Bool handleOperation(char *operationName, char *args)
{
    const Operation *p = getOperationByName(operationName);
    AddrMethodsOptions active[2] = {{0, 0, 0, 0}, {0, 0, 0, 0}};
    char *first = NULL, *second = NULL, *extra = NULL;
    Bool areOperandsLegal = True;
    int argCount;

    if (*args)
        areOperandsLegal = verifyCommaSyntax(args);

    argCount = processArguments(args, &first, &second, &extra);

    if (argCount > 2)
        areOperandsLegal = yieldError(extraOperandsPassed);

    areOperandsLegal = parseOperands(first, second, p, active) && areOperandsLegal;

    if (areOperandsLegal)
    {
        int size = computeSize(p, active);
        active[0].direct = active[0].immediate = active[0].index = active[0].reg = 0;
        active[1].direct = active[1].immediate = active[1].index = active[1].reg = 0;
        increaseInstructionCounter(size);
    }

    return areOperandsLegal;
}

int countOperandsPassed(char *src, char *des)
{
    int operandsPassedCount = 0;
    if (src)
        operandsPassedCount++;
    if (des)
        operandsPassedCount++;
    return operandsPassedCount;
}

int countExpectedOperands(const Operation *op)
{
    int expectedOperandsCount = 0;
    if (op->src.direct || op->src.immediate || op->src.index || op->src.reg)
        expectedOperandsCount++;
    if (op->des.direct || op->des.immediate || op->des.index || op->des.reg)
        expectedOperandsCount++;
    return expectedOperandsCount;
}

void adjustSourceAndDestinationOperands(char **src, char **des, int expectedOperandsCount, int operandsPassedCount)
{
    if (expectedOperandsCount == 1 && operandsPassedCount == 1)
    {
        *des = *src;
        *src = 0;
    }
}

Bool validateOperandsByType(const Operation *op, char *src, char *des, AddrMethodsOptions active[2])
{
    Bool isValid = True;
    if ((op->src.direct || op->src.immediate || op->src.reg || op->src.index) && (op->des.direct || op->des.immediate || op->des.reg || op->des.index))
    {
        isValid = (!src) ? yieldError(requiredSourceOperandIsMissin) : validateOperandMatch(op->src, active, src, 0);
        isValid = (!des) ? yieldError(requiredDestinationOperandIsMissin) : validateOperandMatch(op->des, active, des, 1) && isValid;
    }
    else if (op->src.direct || op->src.immediate || op->src.reg || op->src.index)
    {
        isValid = (!src) ? yieldError(requiredSourceOperandIsMissin) : validateOperandMatch(op->src, active, src, 0);
    }
    else if (op->des.direct || op->des.immediate || op->des.reg || op->des.index)
    {
        isValid = (!des) ? yieldError(requiredDestinationOperandIsMissin) : validateOperandMatch(op->des, active, des, 1);
    }
    return isValid;
}

Bool parseOperands(char *src, char *des, const Operation *op, AddrMethodsOptions active[2])
{
    Bool isValid;
    int operandsPassedCount = countOperandsPassed(src, des);
    int expectedOperandsCount = countExpectedOperands(op);

    adjustSourceAndDestinationOperands(&src, &des, expectedOperandsCount, operandsPassedCount);

    if (expectedOperandsCount == operandsPassedCount && expectedOperandsCount == 0)
        return True;

    isValid = (operandsPassedCount > expectedOperandsCount) ? yieldError(extraOperandsPassed) : True;
    return validateOperandsByType(op, src, des, active) && isValid;
}

typedef enum
{
    OPERAND_TYPE_NONE,
    OPERAND_TYPE_IMMEDIATE,
    OPERAND_TYPE_DIRECT_INDEX,
    OPERAND_TYPE_REG,
    OPERAND_TYPE_DIRECT
} OperandType;

OperandType getOperandType(char *operand)
{
    if (isValidImmediateParamter(operand))
        return OPERAND_TYPE_IMMEDIATE;
    if (isValidIndexParameter(operand))
        return OPERAND_TYPE_DIRECT_INDEX;
    if (isRegistery(operand))
        return OPERAND_TYPE_REG;
    if (verifyLabelNaming(operand))
        return OPERAND_TYPE_DIRECT;
    return OPERAND_TYPE_NONE;
}

Bool isValidOperandForOperation(AddrMethodsOptions allowedAddrs, OperandType operandType, AddrMethodsOptions active[2], int type)
{
    switch (operandType)
    {
    case OPERAND_TYPE_REG:
        if (!allowedAddrs.reg)
            return False;
        active[type].reg = 1;
        break;
    case OPERAND_TYPE_IMMEDIATE:
        if (!allowedAddrs.immediate)
            return False;
        active[type].immediate = 1;
        break;
    case OPERAND_TYPE_DIRECT:
        if (!allowedAddrs.direct)
            return False;
        active[type].direct = 1;
        break;
    case OPERAND_TYPE_DIRECT_INDEX:
        if (!allowedAddrs.index)
            return False;
        active[type].index = 1;
        break;
    default:
        return False;
    }
    return True;
}

Bool validateOperandMatch(AddrMethodsOptions allowedAddrs, AddrMethodsOptions active[2], char *operand, int type)
{
    OperandType operandType = getOperandType(operand);

    if (operandType == OPERAND_TYPE_NONE)
    {
        return type == 1 ? yieldError(illegalInputPassedAsOperandDesOperand) : yieldError(illegalInputPassedAsOperandSrcOperand);
    }

    if (isIndexParameter(operand) && operandType != OPERAND_TYPE_DIRECT_INDEX)
    {
        return yieldError(registeryIndexOperandTypeIfOutOfAllowedRegisteriesRange);
    }

    if (!isValidOperandForOperation(allowedAddrs, operandType, active, type))
    {
        return type == 1 ? yieldError(desOperandTypeIsNotAllowed) : yieldError(srcOperandTypeIsNotAllowed);
    }

    return True;
}

Bool processLabelDeclaration(int type, char *firstToken, char *line)
{
    int dataCounter = getDC();
    firstToken[strlen(firstToken) - 1] = '\0';
    if (isLabelNameAlreadyTaken(firstToken, Symbol))
    {
        yieldError(illegalSymbolNameAlreadyInUse);
        return False;
    }

    if ((type == _TYPE_DATA && countAndVerifyDataArguments(line)) ||
        (type == _TYPE_STRING && countAndVerifyStringArguments(line)))
    {
        return addSymbol(firstToken, dataCounter, 0, 1, 0, 0);
    }
    return False;
}

Bool processEntryOrExternal(int type, char *nextTokens)
{
    Bool result;
    char *labelName = cloneString(nextTokens);
    nextTokens = strtok(NULL, " \t\n\f\r");
    if (nextTokens)
    {
        yieldError(illegalApearenceOfCharactersInTheEndOfTheLine);
        free(labelName);
        return False;
    }
    result = (type == _TYPE_ENTRY) ? addSymbol(labelName, 0, 0, 0, 1, 0) : addSymbol(labelName, 0, 0, 0, 0, 1);
    free(labelName);
    return result;
}

Bool handleInstruction(int type, char *firstToken, char *nextTokens, char *line)
{
    if (!isInstruction(firstToken))
    {
        if (isLabelDeclaration(firstToken))
        {
            return processLabelDeclaration(type, firstToken, line);
        }
        yieldError(undefinedOperation);
        return False;
    }

    if (type == _TYPE_DATA)
    {
        return countAndVerifyDataArguments(line);
    }
    else if (type == _TYPE_STRING)
    {
        return countAndVerifyStringArguments(line);
    }
    else if (type == _TYPE_ENTRY || type == _TYPE_EXTERNAL)
    {
        return nextTokens ? processEntryOrExternal(type, nextTokens) : yieldError(emptyDeclaretionOfEntryOrExternalVariables);
    }
    return False;
}

Bool processInstruction(char *labelName, char *nextToken, char *line)
{
    int instruction = getInstructionType(nextToken);
    if (!isInstructionStrict(nextToken))
    {
        yieldError(missinSpaceAfterInstruction);
        nextToken = getInstructionNameByType(instruction);
    }

    if (instruction == _TYPE_ENTRY || instruction == _TYPE_EXTERNAL)
    {
        char *next = strtok(NULL, " \t\n\f\r");
        return next ? handleInstruction(instruction, nextToken, next, line) : yieldWarning(emptyLabelDecleration);
    }
    return handleInstruction(instruction, labelName, nextToken, line);
}

Bool processOperation(char *labelName, char *nextToken, char *line)
{
    int icAddr = getIC();
    char args[MAX_LINE_LEN] = {0};
    int offset = (int)(strlen(labelName) + strlen(nextToken) + 1);
    strcpy(args, &line[offset]);
    return handleOperation(nextToken, args) ? addSymbol(labelName, icAddr, 1, 0, 0, 0) : False;
}

Bool handleLabel(char *labelName, char *nextToken, char *line)
{
    if (!labelName || !nextToken || !line)
        return False;

    if (isInstruction(nextToken))
    {
        return processInstruction(labelName, nextToken, line);
    }
    else if (isOperation(nextToken))
    {
        return processOperation(labelName, nextToken, line);
    }
    else
    {
        yieldError(illegalLabelUseExpectedOperationOrInstruction);
        return False;
    }
}