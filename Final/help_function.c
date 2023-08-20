#include "lib.h"
#include "variables.h"
#include "functions.h"

int calculateHexSize(int num)
{
    int i;
    int size = 0;
    for (i = num; i > 0; i /= 16)
        size++;

    return size;
}

char *decToHex(int num)
{
    char *hex;
    int size;
    if (num == 0)
        return "0";

    size = calculateHexSize(num);
    hex = calloc(size + 1, sizeof(char));
    sprintf(hex, "%05x", num);

    return hex;
}

char *cloneString(char *s)
{
    char *copy;
    int len;
    if (!(*s))
        return "";

    len = strlen(s) + 1;
    copy = calloc(len, sizeof(char));
    memcpy(copy, s, len);

    return copy;
}

char *trimFromLeft(char *s)
{
    while (isspace(*s) && *s != '\0')
        s++;

    return s;
}

char *hexDigitToBinary(char hexDigit)
{
    switch (hexDigit)
    {
    case '0':
        return "0000";
    case '1':
        return "0001";
    case '2':
        return "0010";
    case '3':
        return "0011";
    case '4':
        return "0100";
    case '5':
        return "0101";
    case '6':
        return "0110";
    case '7':
        return "0111";
    case '8':
        return "1000";
    case '9':
        return "1001";
    case 'A':
    case 'a':
        return "1010";
    case 'B':
    case 'b':
        return "1011";
    case 'C':
    case 'c':
        return "1100";
    case 'D':
    case 'd':
        return "1101";
    case 'E':
    case 'e':
        return "1110";
    case 'F':
    case 'f':
        return "1111";
    default:
        return "";
    }
}

char *numToBin(int num)
{
    int i;
    unsigned int result;
    char *word, hex[6];
    word = (char *)calloc(BINARY_WORD_SIZE + 1, sizeof(char *));

    if (num < 0)
    {
        result = abs(num);
        result = ~result;
        result++;
        sprintf(hex, "%05x", (int)(result & 0x4ffff));
    }
    else
    {
        sprintf(hex, "%05x", (int)num & 0xfffff);
    }

    for (i = 0; hex[i] != '\0'; i++)
    {
        strcat(word, hexDigitToBinary(hex[i]));
    }

    strcat(word, "\0");
    return word;
}

HexWord *convertBinaryWordToHex(BinaryWord *word)
{
    int i = BINARY_WORD_SIZE - 1;
    char hexDigits[4] = {0};
    HexWord *newHex = (HexWord *)malloc(sizeof(HexWord));

    while (i >= 0)
    {
        hexDigits[i % 4] = word->digit[i].on ? '1' : '0';

        if (i % 4 == 0)
        {
            unsigned hexPart = binaryStringToHexNumber(hexDigits);
            switch (i / 4)
            {
            case 0:
                newHex->_A = hexPart;
                break;
            case 1:
                newHex->_B = hexPart;
                break;
            case 2:
                newHex->_C = hexPart;
                break;
            case 3:
                newHex->_D = hexPart;
                break;
            case 4:
                newHex->_E = hexPart;
                break;
            }
            memset(hexDigits, 0, 4);
        }

        i--;
    }

    return newHex;
}

unsigned binaryStringToHexNumber(char binaryStr[4])
{
    unsigned i;
    char *binaryStrings[16] = {"0000", "0001", "0010", "0011", "0100", "0101", "0110", "0111",
                               "1000", "1001", "1010", "1011", "1100", "1101", "1110", "1111"};
    for (i = 0; i < 16; i++)
    {
        if (!strcmp(binaryStr, binaryStrings[i]))
            return i;
    }

    return 0;
}
