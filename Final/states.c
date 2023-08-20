#include "lib.h"
#include "variables.h"
#include "functions.h"

static State globalState = startProgram;
static char *path = NULL;
static unsigned currentLineNumber = 1;

/* Private helper functions */
static void cloneAndAssignPath(const char *source);
static void modifyCurrentLineNumber(int change);

void setGlobalState(State newState)
{
    globalState = newState;
}

State getGlobalState()
{
    return globalState;
}

void setFileNamePath(char *s)
{
    if (s && *s)
    {
        cloneAndAssignPath(s);
    }
}

char *getFileNamePath()
{
    return cloneString(path);
}

void resetCurrentLineNumber()
{
    modifyCurrentLineNumber(-currentLineNumber + 1);
}

void increaseCurrentLineNumber()
{
    modifyCurrentLineNumber(1);
}

int getCurrentLineNumber()
{
    return currentLineNumber;
}

/* Helper function definitions*/
static void cloneAndAssignPath(const char *source)
{
    path = (char *)realloc(path, strlen(source) + 1);
    if (path)
    {
        strcpy(path, source);
    }
}

static void modifyCurrentLineNumber(int change)
{
    currentLineNumber += change;
}
