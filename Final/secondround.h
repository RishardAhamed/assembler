

Bool writeOperationBinary(char *operationName, char *args);

void writeAdditionalOperandsWords(const Operation *op, AddrMethodsOptions active, char *value);

Bool writeDataInstruction(char *token);

Bool writeStringInstruction(char *s);

void writeSecondWord(char *first, char *second, AddrMethodsOptions active[2], const Operation *op);

void writeFirstWord(const Operation *op);

void writeDirectOperandWord(char *labelName);

void writeImmediateOperandWord(char *n);

Bool detectOperandType(char *operand, AddrMethodsOptions active[2], int type);

char *parseLabelNameFromIndexAddrOperand(char *s);

int parseRegNumberFromIndexAddrOperand(char *s);
