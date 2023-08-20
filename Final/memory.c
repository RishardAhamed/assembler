#include "lib.h"
#include "variables.h"
#include "functions.h"

static BinaryWord *binaryImg = NULL;
static HexWord *hexImg = NULL;
unsigned static IC = MEMORY_START;
unsigned static DC = 0;
unsigned static ICF = 0;
unsigned static DCF = 0;

extern HexWord *convertBinaryWordToHex(BinaryWord *word);
extern char *numToBin(int num);
extern unsigned binaryStringToHexNumber(char binaryStr[4]);

unsigned getDC() { return DC; }
unsigned getIC() { return IC; }
unsigned getICF() { return ICF; }
unsigned getDCF() { return DCF; }
void increaseDataCounter(int amount)
{
    DC += amount;
}
void increaseInstructionCounter(int amount)
{
    IC += amount;
}
void freeMemoryImg()
{
    if (binaryImg != NULL)
    {
        free(binaryImg);
        binaryImg = NULL;
    }
    if (hexImg != NULL)
    {
        free(hexImg);
        hexImg = NULL;
    }
}

void initializeHexImg(int index)
{
    hexImg[index]._A = 0;
    hexImg[index]._B = 0;
    hexImg[index]._C = 0;
    hexImg[index]._D = 0;
    hexImg[index]._E = 0;
}

void initializeBinaryImg(int index)
{
    int j = 0;
    while (j < BINARY_WORD_SIZE)
    {
        binaryImg[index].digit[j].on = 0;
        j++;
    }
}

void allocMemoryImg()
{
    const int totalSize = DCF - MEMORY_START;
    int i = 0;

    freeMemoryImg();

    binaryImg = (BinaryWord *)malloc(totalSize * sizeof(BinaryWord));
    hexImg = (HexWord *)malloc(totalSize * sizeof(HexWord));

    while (i < totalSize)
    {
        initializeHexImg(i);
        initializeBinaryImg(i);
        i++;
    }
}

void resetMemoryCounters()
{
    IC = MEMORY_START;
    DC = 0;
    ICF = 0;
    DCF = 0;
}
void printBinaryImg()
{
    int i;
    int totalSize = DCF - MEMORY_START;
    for (i = 0; i < totalSize; i++)
    {
        printf("%04d ", MEMORY_START + i);
        printWordBinary(i);
    }
}
void addWord(int value, DataType type)
{
    if (type == Code)
        addWordToCodeImage(numToBin(value));
    else if (type == Data)
        addWordToDataImage(numToBin(value));
}
void addWordToDataImage(char *s)
{
    wordStringToWordObj(s, Data);
    DC++;
}
void addWordToCodeImage(char *s)
{
    wordStringToWordObj(s, Code);
    IC++;
}
void handleBinaryImage(unsigned index, int pos, Bool isPrint, char *s)
{
    if (isPrint)
    {
        if (pos % 4 == 0)
            printf(" ");
        printf("%d", binaryImg[index].digit[pos].on ? 1 : 0);
    }
    else
    {
        binaryImg[index].digit[pos].on = s[pos] == '1' ? 1 : 0;
    }
}

void wordStringToWordObj(char *s, DataType type)
{
    int pos = 0;
    int index = type == Code ? IC - MEMORY_START : DC - MEMORY_START;

    while (pos < BINARY_WORD_SIZE)
    {
        handleBinaryImage(index, pos, False, s);
        pos++;
    }
}

void printWordBinary(unsigned index)
{
    int pos = 0;

    while (pos < BINARY_WORD_SIZE)
    {
        handleBinaryImage(index, pos, True, NULL);
        pos++;
    }
    printf("\n");
}

void calcFinalAddrsCountersValues()
{

    ICF = IC;
    DCF = ICF + DC;
    DC = IC;
    IC = MEMORY_START;
}

void writeMemoryImage(FILE *fp, Bool printToConsole)
{
    extern BinaryWord *binaryImg;
    extern HexWord *hexImg;
    int i = 0;
    int totalSize = DCF - MEMORY_START;

    if (printToConsole)
        printf("%d %d\n", ICF - MEMORY_START, DCF - ICF);
    fprintf(fp, "%d %d\n", ICF - MEMORY_START, DCF - ICF);

    while (i < totalSize)
    {
        hexImg[i] = *convertBinaryWordToHex(&binaryImg[i]);
        if (printToConsole)
            printf("%04d A%x-B%x-C%x-D%x-E%x\n", MEMORY_START + i, hexImg[i]._A, hexImg[i]._B, hexImg[i]._C, hexImg[i]._D, hexImg[i]._E);
        fprintf(fp, "%04d A%x-B%x-C%x-D%x-E%x\n", MEMORY_START + i, hexImg[i]._A, hexImg[i]._B, hexImg[i]._C, hexImg[i]._D, hexImg[i]._E);
        i++;
    }
}

void printMemoryImgInRequiredObjFileFormat()
{
    writeMemoryImage(stdout, True);
}

void writeMemoryImageToObFile(FILE *fp)
{
    writeMemoryImage(fp, False);
}
