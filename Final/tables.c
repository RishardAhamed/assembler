#include "lib.h"
#include "variables.h"
#include "functions.h"
static Item *symbols[HASHSIZE] = {0};
static Item *macros[HASHSIZE] = {0};
static unsigned entriesCount = 0;
static unsigned externalCount = 0;

static ExtListItem *extListHead = NULL;
extern unsigned getICF();
extern Bool verifyLabelNaming(char *s);

void findAllExternals();
void addExtListItem(char *name);
void resetExtList();
ExtListItem *findExtOpListItem(char *name);
void updateExtPositionData(char *name, unsigned base, unsigned offset);
void freeTableItem(Item *item);
void freeTablesMemory();
Item *updateMacroValues(Item *macro, int start, int end);
void handleNullSymbol(Item *p);
Item *handleMacroError(Item *macro);
void checkSymbolAndApplyUpdate(int i);
void handleItemAttributes(Item *item);
ExtListItem *findExtOpListItem(char *name)
{
    extern ExtListItem *extListHead;
    ExtListItem *p = extListHead;
    for (; p != NULL; p = p->next)

    {
        if (p->name[0] != '\0' && strcmp(name, p->name) == 0)

            return p;
    }
    return NULL;
}

void freePosNodes(ExtPositionData *posData)
{
    ExtPositionData *pos = posData, *nextPos;
    while (pos != NULL)
    {
        nextPos = pos->next;
        free(pos);
        pos = nextPos;
    }
}

void resetExtList()
{
    ExtListItem *currentItem = extListHead, *nextItem;
    externalCount = 0;

    while (currentItem != NULL)
    {
        nextItem = currentItem->next;
        freePosNodes(currentItem->value.next);
        free(currentItem);
        currentItem = nextItem;
    }

    extListHead = NULL;
}

ExtPositionData *createExtPositionData(unsigned base, unsigned offset)
{
    ExtPositionData *newData = (ExtPositionData *)malloc(sizeof(ExtPositionData));
    newData->base = base;
    newData->offset = offset;
    return newData;
}

void updateExtPositionData(char *name, unsigned base, unsigned offset)
{
    ExtListItem *listItem = findExtOpListItem(name);

    if (listItem->value.base)
    {
        ExtPositionData *newData = createExtPositionData(base, offset);
        newData->next = listItem->value.next;
        listItem->value.next = newData;
    }
    else
    {
        listItem->value.base = base;
        listItem->value.offset = offset;
    }

    externalCount++;
}

void addExtListItem(char *name)
{
    ExtListItem *newItem = (ExtListItem *)calloc(1, sizeof(ExtListItem));
    strncpy(newItem->name, name, strlen(name));

    if (extListHead == NULL)
    {
        extListHead = newItem;
    }
    else
    {
        ExtListItem *currentItem = extListHead;
        while (currentItem->next != NULL)
        {
            currentItem = currentItem->next;
        }
        currentItem->next = newItem;
    }
}

unsigned computeHash(char *s)
{
    unsigned hashval = 0;
    while (*s != '\0')
    {
        hashval = *s + 31 * hashval;
        s++;
    }
    return hashval;
}

unsigned hash(char *s)
{
    return computeHash(s) % HASHSIZE;
}

Item *searchItem(char *s, ItemType type, int hashIndex)
{
    Item *np;
    for (np = (type == Symbol ? symbols[hashIndex] : macros[hashIndex]); np != NULL; np = np->next)
    {
        if (!strcmp(s, np->name))
            return np;
    }
    return NULL;
}

Item *lookup(char *s, ItemType type)
{
    return searchItem(s, type, hash(s));
}

void initializeItem(Item *np, char *name, ItemType type)
{
    np->name = cloneString(name);
    if (type == Symbol)
    {
        np->val.s.attrs.code = 0;
        np->val.s.attrs.entry = 0;
        np->val.s.attrs.external = 0;
        np->val.s.attrs.data = 0;
        np->val.s.base = 0;
        np->val.s.value = 0;
        np->val.s.offset = 0;
    }
    else if (type == Macro)
    {
        np->val.m.start = -1;
        np->val.m.end = -1;
    }
}

void addNewItemToTable(Item *np, ItemType type, unsigned hashval)
{
    np->next = (type == Symbol ? symbols[hashval] : macros[hashval]);
    if (type == Symbol)
        symbols[hashval] = np;
    else
        macros[hashval] = np;
}

Item *install(char *name, ItemType type)
{
    unsigned hashval;
    Item *np;
    np = (Item *)malloc(sizeof(Item));

    if (np == NULL)
    {
        yieldError(memoryAllocationFailure);
        return NULL;
    }
    else
    {
        initializeItem(np, name, type);
        hashval = hash(name);
        addNewItemToTable(np, type, hashval);
    }

    return np;
}

void updateSymbolAttributes(Item *p, unsigned isCode, unsigned isData, unsigned isEntry, unsigned isExternal)
{
    if (isEntry)
        p->val.s.attrs.entry = 1;
    if (isCode)
        p->val.s.attrs.code = 1;
    if (isData)
        p->val.s.attrs.data = 1;
    if (isExternal)
        p->val.s.attrs.external = 1;
}

Bool checkSymbolUpdateValidity(Item *p, unsigned value, unsigned isCode, unsigned isData, unsigned isEntry, unsigned isExternal)
{
    if ((p->val.s.attrs.external) && (value || isData || isEntry || isCode))
        return yieldError(illegalOverrideOfExternalSymbol);

    else if ((p->val.s.attrs.code || p->val.s.attrs.data || p->val.s.attrs.entry) && isExternal)
        return yieldError(illegalOverrideOfLocalSymbolWithExternalSymbol);

    else if ((isData && isCode) || (isCode && p->val.s.attrs.data) || (isData && p->val.s.attrs.code))
        return yieldError(illegalSymbolNameAlreadyInUse);

    return True;
}

Bool updateSymbol(Item *p, unsigned value, unsigned isCode, unsigned isData, unsigned isEntry, unsigned isExternal)
{
    if (!checkSymbolUpdateValidity(p, value, isCode, isData, isEntry, isExternal))
        return False;

    if (value)
    {
        unsigned base = 0;
        unsigned offset = 0;
        offset = value % 16;
        base = value - offset;
        p->val.s.value = value;
        p->val.s.base = base;
        p->val.s.offset = offset;
    }

    updateSymbolAttributes(p, isCode, isData, isEntry, isExternal);

    return True;
}

Bool addSymbol(char *name, unsigned value, unsigned isCode, unsigned isData, unsigned isEntry, unsigned isExternal)
{
    unsigned base;
    unsigned offset;
    Item *p;

    if (name[strlen(name) - 1] == ':')
        name[strlen(name) - 1] = '\0';

    if (!verifyLabelNamingAndPrintErrors(name))
        return False;
    p = lookup(name, Symbol);
    if (p != NULL)
        return updateSymbol(p, value, isCode, isData, isEntry, isExternal);
    else
    {
        p = install(name, Symbol);
        offset = value % 16;
        base = value - offset;
        p->val.s.value = value;
        p->val.s.base = base;
        p->val.s.offset = offset;
        updateSymbolAttributes(p, isCode, isData, isEntry, isExternal);
    }

    return True;
}

Item *getSymbol(char *name)
{
    return lookup(name, Symbol);
}

int getSymbolBaseAddress(char *name)
{
    Item *p = lookup(name, Symbol);
    if (p == NULL)
        return -1;

    return p->val.s.base;
}

Item *findItem(char *name)
{
    return lookup(name, Symbol);
}

int getSymbolOffset(char *name)
{
    Item *p = findItem(name);
    return p != NULL ? p->val.s.offset : -1;
}

Bool isItemExist(char *name)
{
    return findItem(name) != NULL ? True : False;
}

Bool isSymbolExist(char *name)
{
    return isItemExist(name);
}

Bool isItemExternal(char *name)
{
    Item *p = findItem(name);
    return p != NULL ? p->val.s.attrs.external : False;
}

Bool isExternal(char *name)
{
    return isItemExternal(name);
}

Bool isItemEntry(char *name)
{
    Item *p = findItem(name);
    return p != NULL ? (p->val.s.attrs.entry == 1 ? True : False) : False;
}

Bool isEntry(char *name)
{
    return isItemEntry(name);
}

Bool isItemNonEmpty(char *name)
{
    Item *p = findItem(name);
    return p != NULL ? ((p->val.s.attrs.code || p->val.s.attrs.data) ? True : False) : False;
}

Bool isNonEmptyExternal(char *name)
{
    return isItemNonEmpty(name);
}

Bool isNonEmptyEntry(char *name)
{
    return isItemNonEmpty(name);
}

Bool isTakenLabelName(char *name, ItemType type)
{
    Item *p = findItem(name);

    if (name[strlen(name) - 1] == ':')
        name[strlen(name) - 1] = '\0';

    if (p != NULL)
    {
        if (type == Symbol)
        {
            if (p->val.s.attrs.data || p->val.s.attrs.code)
                return True;
            if (p->val.s.attrs.entry)
                return (!p->val.s.attrs.data && !p->val.s.attrs.code && !p->val.s.attrs.external) ? False : True;
            if (p->val.s.attrs.external)
                return (!p->val.s.attrs.data && !p->val.s.attrs.code && !p->val.s.attrs.entry) ? False : True;
        }

        else if (type == Macro)
            return p->val.m.start != -1 ? False : True;
    }

    return False;
}

Bool isLabelNameAlreadyTaken(char *name, ItemType type)
{
    return isTakenLabelName(name, type);
}

void handleNullSymbol(Item *p)
{
    if (p == NULL)
        yieldError(symbolDoesNotExist);
}

Item *calculateSymbolAddress(Item *p, int newValue)
{
    handleNullSymbol(p);

    if (p != NULL)
    {
        p->val.s.offset = newValue % 16;
        p->val.s.base = newValue - p->val.s.offset;
        p->val.s.value = newValue;
    }

    return p;
}

Item *updateSymbolAddressValue(char *name, int newValue)
{
    return calculateSymbolAddress(getSymbol(name), newValue);
}

Item *getMacro(char *s)
{
    return lookup(s, Macro);
}

Item *handleMacroError(Item *macro)
{
    if (macro != NULL)
    {
        yieldError(illegalMacroNameAlreadyInUse);
        return NULL;
    }

    return macro;
}

Item *addMacro(char *name, int start, int end)
{
    Item *macro = handleMacroError(lookup(name, Macro));

    if (macro == NULL)
    {
        macro = install(name, Macro);
        updateMacroValues(macro, start, end);
    }

    return macro;
}

Item *updateMacroValues(Item *macro, int start, int end)
{
    if (macro)
    {
        if (start != -1)
            macro->val.m.start = start;
        if (end != -1)
            macro->val.m.end = end;
    }

    return macro;
}

Item *updateMacro(char *name, int start, int end)
{
    return updateMacroValues(getMacro(name), start, end);
}

void checkSymbolAndApplyUpdate(int i)
{
    if (symbols[i] != NULL)
        updateFinalValueOfSingleItem(symbols[i]);
}

void updateFinalSymbolTableValues()
{
    int i;
    for (i = 0; i < HASHSIZE; i++)
    {
        checkSymbolAndApplyUpdate(i);
    }
}

void handleItemAttributes(Item *item)
{
    if (item->val.s.attrs.entry)
        entriesCount++;
    if (item->val.s.attrs.external)
        addExtListItem(item->name);
    if (item->val.s.attrs.data)
        calculateSymbolAddress(item, item->val.s.value + getICF());
}

void updateFinalValueOfSingleItem(Item *item)
{
    handleItemAttributes(item);
    if (item->next != NULL)
        updateFinalValueOfSingleItem(item->next);
}

Bool areEntriesExist()
{
    return entriesCount > 0 ? True : False;
}
Bool areExternalsExist()
{
    return externalCount > 0 ? True : False;
}

void writeExternalsToFile(FILE *fp)
{
    ExtListItem *p = extListHead;
    while (p != NULL)
    {
        if (p->value.base)
            writeSingleExternal(fp, p->name, p->value.base, p->value.offset, p->value.next);
        p = p->next;
    }
}

void writeSingleExternal(FILE *fp, char *name, unsigned base, unsigned offset, ExtPositionData *next)
{

    fprintf(fp, "%s BASE %u\n", name, base);
    fprintf(fp, "%s OFFSET %u\n", name, offset);
    if (next != NULL)
        writeSingleExternal(fp, name, next->base, next->offset, next->next);
}

void writeEntriesToFile(FILE *fp)
{
    int i = 0;
    int totalCount = 0;
    while (i < HASHSIZE)
    {
        if (symbols[i] != NULL && totalCount <= entriesCount)
            totalCount += writeSingleEntry(symbols[i], fp, 0);
        i++;
    }
}

int writeSingleEntry(Item *item, FILE *fp, int count)
{
    if (item->val.s.attrs.entry)
    {
        fprintf(fp, "%s,%d,%d\n", item->name, item->val.s.base, item->val.s.offset);
        count++;
    }
    if (item->next != NULL)
        writeSingleEntry(item->next, fp, count);

    return count;
}

void initTables()
{
    extern unsigned externalCount, entriesCount;
    int i = 0;
    if (extListHead != NULL)
        resetExtList();

    externalCount = entriesCount = 0;
    while (i < HASHSIZE)
    {
        symbols[i] = NULL;
        macros[i] = NULL;
        i++;
    }
}

void freeHashTable(ItemType type)
{

    int i = 0;
    while (i < HASHSIZE)
    {

        if (type == Symbol)
        {
            if (symbols[i] != NULL)
                freeTableItem(symbols[i]);
        }
        else
        {
            if (macros[i] != NULL)
                freeTableItem(macros[i]);
        }

        i++;
    }
}

void freeTableItem(Item *item)
{
    if (item->next != NULL)
        freeTableItem(item->next);
    /*     printf("item->name:%s\n", item->name); */
    free(item);
    return;
}

void printMacroTable()
{
    int i = 0;
    printf("\n\t ~ MACRO TABLE ~ \n");
    printf("\tname\tstart\tend");
    while (i < HASHSIZE)
    {
        if (macros[i] != NULL)
            printMacroItem(macros[i]);
        i++;
    }
    printf("\n\n");
}

int printMacroItem(Item *item)
{

    printf("\n\t%s\t %5d\t%6d", item->name, item->val.m.start, item->val.m.end);
    if (item->next != NULL)
        printMacroItem(item->next);
    return 0;
}

void printSymbolTable()
{
    int i = 0;

    printf("\n\t\t ~ SYMBOL TABLE ~ \n");
    printf("name\tvalue\tbase\toffset\tattributes");

    while (i < HASHSIZE)
    {
        if (symbols[i] != NULL)
            printSymbolItem(symbols[i]);
        i++;
    }
    printf("\n\n");
}

int printSymbolItem(Item *item)
{
    /*  printf("line 94, inside printSymbolItem \n");
     */

    printf("\n%s\t%u\t%u\t%u\t", item->name, item->val.s.value, item->val.s.base, item->val.s.offset);
    if (!item->val.s.attrs.code && !item->val.s.attrs.data && !item->val.s.attrs.entry && !item->val.s.attrs.external)
        printf("   ");

    else
    {
        if ((item->val.s.attrs.code || item->val.s.attrs.data) && (item->val.s.attrs.entry || item->val.s.attrs.external))
        {
            if (item->val.s.attrs.code)
                printf("code,");
            else
                printf("data,");

            if (item->val.s.attrs.entry)
                printf("entry");
            else
                printf("external");
        }
        else
        {
            if (item->val.s.attrs.code)
                printf("code");
            else if (item->val.s.attrs.data)
                printf("data");
            else if (item->val.s.attrs.entry)
                printf("entry");
            else
                printf("external");
        }
    }

    if (item->next != NULL)
        printSymbolItem(item->next);
    return 0;
}