Bool isMacroOpening(char *s);

Bool isMacroClosing(char *s);

Bool isPossiblyUseOfMacro(char *s);

Bool isLegalMacroName(char *s);

Bool isInstruction(char *s);

Bool isInstructionStrict(char *s);

Bool isRegistery(char *s);

Bool isValidImmediateParamter(char *s);

Bool isValidIndexParameter(char *s);

Bool isIndexParameter(char *s);

Bool isComment(char *s);

Bool isOperation(char *s);

Bool isLabelDeclarationStrict(char *s);

Bool isLabelDeclaration(char *s);

int getInstructionType(char *s);

int getRegisteryNumber(char *s);

char *getInstructionName(char *s);

char *getInstructionNameByType(int type);

Bool verifyLabelNaming(char *s);

Bool verifyLabelNamingAndPrintErrors(char *s);
