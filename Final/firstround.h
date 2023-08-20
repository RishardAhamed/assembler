

Bool handleOperation(char *operationName, char *args);
Bool parseOperands(char *src, char *des, const Operation *op, AddrMethodsOptions active[2]);
Bool validateOperandMatch(AddrMethodsOptions allowedAddrs, AddrMethodsOptions active[2], char *operand, int type);
Bool handleInstruction(int type, char *firstToken, char *nextTokens, char *line);
Bool handleLabel(char *labelName, char *nextToken, char *line);
