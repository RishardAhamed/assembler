#include "lib.h"
#include "variables.h"
#include "functions.h"

#define EXT_LEN 4
#define MSG_LEN 128

void displayError(const char *errorMsg, const char *details)
{
    fprintf(stderr, "\n#############################################\n");
    fprintf(stderr, errorMsg, details);
    fprintf(stderr, "\n#############################################\n\n");
}

int main(int argc, char *argv[])
{
    int counter;
    if (argc <= 1)
    {
        displayError("No source files passed to the assembler!", NULL);
        exit(1);
    }

    counter = 1;
    while (counter < argc)
    {
        oneFileHandle(argv[counter]);
        counter++;
    }

    closeOpenLogFiles();

    return 0;
}

void setupFiles(char *arg, char **fileName, FILE **src, FILE **target)
{
    void (*assignPath)(char *) = &setFileNamePath;

    *fileName = (char *)calloc(strlen(arg) + EXT_LEN, sizeof(char *));
    if (!*fileName)
    {
        displayError("Memory allocation error for filename: %s", arg);
        return;
    }

    strncpy(*fileName, arg, strlen(arg));
    strcat(*fileName, ".as");
    assignPath(*fileName);

    if (!(*src = fopen(*fileName, "r")))
    {
        displayError("Can't open source code file %s", *fileName);
        free(*fileName);
        *fileName = NULL;
        return;
    }

    (*fileName)[strlen(*fileName) - 1] = 'm';
    assignPath(*fileName);

    if (!(*target = fopen(*fileName, "w+")))
    {
        displayError("Can't create expanded source code file %s", *fileName);
        fclose(*src);
        free(*fileName);
        *fileName = NULL;
        *src = NULL;
        return;
    }
}

void processfirstround(FILE *target)
{
    rewind(target);
    parseAssemblyCode(target);
}

void processsecondround(char *fileName, FILE *target)
{
    calcFinalAddrsCountersValues();
    updateFinalSymbolTableValues();
    allocMemoryImg();
    printSymbolTable();
    rewind(target);
    parseAssemblyCode(target);

    if (getGlobalState() == exportFiles)
    {
        fileName[strlen(fileName) - 3] = '\0';
        setFileNamePath(fileName);
        exportFilesMainHandler();
    }
    else
    {
        printf("\nErrors in Second Run, won't export files!\n");
    }
}

void assemble(char *fileName, FILE *src, FILE *target)
{
    State (*currentGlobalState)() = &getGlobalState;
    void (*modifyState)(State) = &setGlobalState;

    initTables();
    modifyState(parsingMacros);
    resetMemoryCounters();
    parseSourceFile(src, target);
    printMacroTable();
    freeHashTable(Macro);

    if (currentGlobalState() == firstround)
    {
        processfirstround(target);

        if (currentGlobalState() == secondround)
        {
            processsecondround(fileName, target);
        }
        else
        {
            printf("\nErrors in First Run, skipping Second Run and won't export files!\n");
        }

        freeHashTable(Symbol);
    }
    else
    {
        printf("\nFailed to expand macros for %s due to code errors. Proceeding to next file if available.\n\n", fileName);
    }

    free(fileName);
    fclose(src);
    fclose(target);
}

void oneFileHandle(char *arg)
{
    FILE *src = NULL, *target = NULL;
    char *fileName = NULL;

    setupFiles(arg, &fileName, &src, &target);

    if (src && target && fileName)
    {
        assemble(fileName, src, target);
    }
}
