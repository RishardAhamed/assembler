void oneFileHandle(char *arg);
void setupFiles(char *arg, char **fileName, FILE **src, FILE **target);
void displayError(const char *errorMsg, const char *details);
void processfirstround(FILE *target);
void processsecondround(char *fileName, FILE *target);
void assemble(char *fileName, FILE *src, FILE *target);
void initTables(void);
void freeHashTable(ItemType type);