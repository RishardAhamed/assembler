#include "lib.h"
#include "variables.h"
#include "functions.h"

extern void fileCreationFailure(char *fileName);
static char *(*generateFileName)() = &getFileNamePath;

void createFile(char *extension, void (*writeToFileFunction)(FILE *))
{
    FILE *file;
    char *fileName = (*generateFileName)();
    strcat(fileName, extension);
    file = fopen(fileName, "w+");

    if (file != NULL)
    {
        (*writeToFileFunction)(file);
        fclose(file);
        free(fileName);
    }
    else
    {
        fileCreationFailure(fileName);
    }
}

void exportFilesMainHandler()
{
    puts("Finished Successfully, about to export files!\n");

    createFile(".ob", writeMemoryImageToObFile);

    if (areEntriesExist())
    {
        createFile(".ent", writeEntriesToFile);
    }
    if (areExternalsExist())
    {
        createFile(".ext", writeExternalsToFile);
    }
}

void generateObFile()
{
    createFile(".ob", writeMemoryImageToObFile);
}

void createEntriesFile()
{
    createFile(".ent", writeEntriesToFile);
}

void createExternalsFile()
{
    createFile(".ext", writeExternalsToFile);
}
