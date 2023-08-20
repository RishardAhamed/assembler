

unsigned hash(char *s);
Item *lookup(char *s, ItemType type);
Item *install(char *name, ItemType type);
Bool addSymbol(char *name, unsigned value, unsigned isCode, unsigned isData, unsigned isEntry, unsigned isExternal);
Bool updateSymbol(Item *p, unsigned value, unsigned isCode, unsigned isData, unsigned isEntry, unsigned isExternal);
Item *getSymbol(char *name);
int getSymbolBaseAddress(char *name);
int getSymbolOffset(char *name);
Bool isSymbolExist(char *name);
Bool isExternal(char *name);
Bool isEntry(char *name);
Bool isNonEmptyExternal(char *name);
Bool isNonEmptyEntry(char *name);
Bool isLabelNameAlreadyTaken(char *name, ItemType type);
Item *updateSymbolAddressValue(char *name, int newValue);
void updateFinalSymbolTableValues();
void updateFinalValueOfSingleItem(Item *item);
Bool areEntriesExist();
Bool areExternalsExist();
void printSymbolTable();
int printSymbolItem(Item *item);
void writeEntriesToFile(FILE *fp);
int writeSingleEntry(Item *item, FILE *fp, int count);
Item *addMacro(char *name, int start, int end);
Item *getMacro(char *s);
Item *updateMacro(char *name, int start, int end);
void printMacroTable();
int printMacroItem(Item *item);
ExtListItem *findExtOpListItem(char *name);
void resetExtList();
void updateExtPositionData(char *name, unsigned base, unsigned offset);
void addExtListItem(char *name);
void writeExternalsToFile(FILE *fp);
void writeSingleExternal(FILE *fp, char *name, unsigned base, unsigned offset, ExtPositionData *next);