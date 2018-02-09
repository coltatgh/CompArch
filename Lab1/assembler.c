/*
Name 1: Karl Solomon
Name 2: Colton Lewis
UTEID 1: kws653
UTEID 2: ctl492 
*/
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <malloc.h>

#define ASCIINUM '0'
#define ASCIIUPPER ('A' - 10)
#define ASCIILOWER ('a' - 10)
#define SIGNED  1
#define UNSIGNED 0

#define MAX_LINE_LENGTH 255
enum
{
   DONE, OK, EMPTY_LINE
};

/* Source 1 = https:/*stackoverflow.com/questions/9907160/how-to-convert-enum-names-to-string-in-c
*/
#define FOREACH_OPCODE(OPCODE) \
        OPCODE(ADD)   \
        OPCODE(AND)  \
        OPCODE(BR)   \
        OPCODE(BRN)   \
        OPCODE(BRZ)   \
        OPCODE(BRP)   \
        OPCODE(BRNZ)   \
        OPCODE(BRNP)   \
        OPCODE(BRZP)   \
        OPCODE(BRNZP)   \
        OPCODE(HALT)   \
        OPCODE(JMP)   \
        OPCODE(JSR)   \
        OPCODE(JSRR)   \
        OPCODE(LDB)   \
        OPCODE(LDW)   \
        OPCODE(LSHF)  \
        OPCODE(LEA)   \
        OPCODE(NOP)   \
        OPCODE(NOT)   \
        OPCODE(RET)   \
        OPCODE(RSHFL)   \
        OPCODE(RSHFA)   \
        OPCODE(RTI)   \
        OPCODE(STB)   \
        OPCODE(STW)   \
        OPCODE(TRAP)   \
        OPCODE(XOR)   \

#define FOREACH_PSEUDOOP(PSEUDOOP) \
        PSEUDOOP(ORIG)   \
        PSEUDOOP(FILL)  \
        PSEUDOOP(END)   \

#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_STRING(STRING) #STRING,

enum PSEUDOOP {
    FOREACH_PSEUDOOP(GENERATE_ENUM)
};

static const char *pseudoOps[] = {
    FOREACH_PSEUDOOP(GENERATE_STRING)
};

enum OPCODE {
    FOREACH_OPCODE(GENERATE_ENUM)
};

static const char *opcodes[] = {
    FOREACH_OPCODE(GENERATE_STRING)
};

static const uint8_t opcodeMap[] = {
    1,    /*OPCODE(ADD)*/   
    5,    /*OPCODE(AND)*/  
    0,    /*OPCODE(BR)*/  
    0,    /*OPCODE(BRN) */  
    0,    /*OPCODE(BRZ)   */
    0,    /*OPCODE(BRP)   */
    0,    /*OPCODE(BRNZ)  */ 
    0,    /*OPCODE(BRNP)   */
    0,    /*OPCODE(BRZP)   */
    0,    /*OPCODE(BRNZP)   */
    0,    /*OPCODE(HALT)   */
    12,    /*OPCODE(JMP)   */
    4,    /*OPCODE(JSR)   */
    4,    /*OPCODE(JSRR)   */
    2,    /*OPCODE(LDB)   */
    6,    /*OPCODE(LDW)   */
    13,    /*OPCODE(LSHF) */
    14,    /*OPCODE(LEA)  */
    0,    /*OPCODE(NOP)   */
    9,    /*OPCODE(NOT)   */
    12,    /*OPCODE(RET)   */
    13,    /*OPCODE(RSHFL)   */
    13,    /*OPCODE(RSHFA)   */
    8,    /*OPCODE(RTI)   */
    3,    /*OPCODE(STB)   */
    7,    /*OPCODE(STW)   */
    15,    /*OPCODE(TRAP)   */
    9    /*OPCODE(XOR)   */
};
/*SOURCE1*/

/*GLOBAL VARIABLES*/
FILE *fileIn = NULL;
FILE *fileOut = NULL;
char* fileOutName = NULL;
char errorMessage[120];
uint32_t lineNumber = 0;
uint16_t startAddress = 0;
uint16_t result = 0;
struct Label* labelTable = NULL;
bool originFound = false;
char *token;

/*
LABEL TABEL
GENERATED ON THE FIRST PASS OF THE ANALYSIS/PARSER
USED ON THE SECOND PASS
*/
static int labelTableLength = 0;
static int labelTableMaxLength = 0;
struct Label {
  char* name;
  uint16_t line;
};

/*FUNCTION PROTOTYPES*/
bool isLabelValid(char* label);
bool isLabel(char* label);
enum OPCODE getOpcode(char* opcode);
int getPseudoOp(char* pseudoOp);
void output(uint16_t output);
int hexToInt(char* hex);
int decToInt(char* dec);
uint16_t opcodeToASM(enum OPCODE opcode);
uint16_t RegisterToInt(char* reg);
void main_1stPass(void);
void main_2ndPass(void);
void verifyOriginFound(void);
void verifyBitLength(int num, int maxBits, bool isSigned, bool isOffset);
int labelToLineNumber(char* label);
int readAndParse( FILE * pInfile, char * pLine, char ** pLabel, char ** pOpcode, char ** pArg1, char ** pArg2, char ** pArg3, char ** pArg4);
void verifyNumberOfOperands(char* a, char* b, char* c, char* d, int numOperands);

void error(int32_t errorCode, char* extraMessage) {
  switch(errorCode) {
    case 0:
    case 1:
      printf("exit(1): line #%d\tUNDEFINED LABEL\t", lineNumber);
      break;
    case 2:
      printf("exit(2): line #%d\tINVALID OPCODE\t", lineNumber);
      break;
    case 3:
      printf("exit(3): line #%d\tINVALID CONSTANT\t", lineNumber);
      break;
    case 4:
      printf("exit(4): line #%d\tMISCELLANEOUS\t", lineNumber);
      break;
    default:
      break;
  }
  if(extraMessage) {
    printf("%s", extraMessage);
  }
  printf("\n\n");
  fflush(fileOut); /* NEEDED THIS*/
  if(fileOut != NULL) {
    fclose(fileOut);
  }
  fileOut = fopen(fileOutName, "w");
  fclose(fileOut);
  free(fileOutName);
  exit(errorCode);
}

void verifyOriginFound(void) {
  if(!originFound) {
    error(4, "NON-TRIVIAL LINE BEFORE .ORIG");
  }
}

bool isPsuedoOp(char* label) {
  bool pOp = false;
  if(label[0] == '.') {
    int i = 0;
    while(pseudoOps[i] != NULL) {
      if(strcmp(label+1, pseudoOps[i]) == 0) {
        pOp = true;
        break;
      }
      i++;
    }
  }
  return pOp;
}

bool isRegister(char* string) {
  bool isRegister = false;
  if(strlen(string) == 2) {
    if(string[0] == 'R') {
      if(string[1] >= '0' && string[1] <= '7')
        isRegister = true;
    }
  }
  return isRegister;
} 


int getPseudoOp(char* label) {
  int pOp = -1;
  if(label[0] == '.') {
    int i = 0;
    while(pseudoOps[i] != NULL) {
      if(strcmp(label+1, pseudoOps[i]) == 0) {
        pOp = i;
        break;
      }
      i++;
    }
  }
  if(pOp == -1) {
    sprintf(errorMessage, "%s is not a valid pseudoOp", label);
    error(4, errorMessage);
  }
  return pOp;
}

/*
True if input label exists in the label Table
*/
bool isLabel(char* label) {
  int i;
  struct Label l;
  bool result = false;
  for(i = 0; i < labelTableLength; i++) {
    l = labelTable[i];
    if(strcmp(label, l.name) == 0) {
      result = true;
      break;
    }
  }
  return result;
}

void addLabel(char* label, int line) {
  if(isLabel(label)) {
    sprintf(errorMessage, "Label %s already exists", label);
    error(4, errorMessage);
    return;
  }
  struct Label *newLabel;
  newLabel = malloc(sizeof(struct Label));
  newLabel->name = malloc(strlen(label));
  strcpy(newLabel->name, label);
  newLabel->line = line;
  if(labelTableLength >= labelTableMaxLength) {
    if(labelTableLength == 0) {
      labelTable = malloc(10*sizeof(struct Label));
      labelTableMaxLength = 10;
    } else {
      labelTable = realloc(labelTable, 2*labelTableMaxLength*sizeof(struct Label));
      labelTableMaxLength *= 2;
    }
  }
  memcpy(labelTable + labelTableLength, newLabel, sizeof(struct Label));
  labelTableLength++;
  free(newLabel);
}

int labelToLineNumber(char* label) {
  isLabelValid(label); /*throws error if label isn't valid*/
  int i;
  struct Label l;
  int result = -1;
  for(i = 0; i < labelTableLength; i++) {
    l = labelTable[i];
    if(strcmp(label, l.name) == 0) {
      result = l.line;
      break;
    }
  }
  if(result == -1) {
    sprintf(errorMessage, "label %s doesn't exist", label);
    error(1, errorMessage);
  }
  return result;
}


void verifyBitLength(int num, int maxBits, bool isSigned, bool isOffset) {
  int max = 1 << maxBits;
  bool throwError = false;
  if(!isSigned) {
    if(num >= max) {
      sprintf(errorMessage, "%d literal is too large. Max = %d", num, max-1);
      throwError = true;
    } else {
      if(num < 0) {
        sprintf(errorMessage, "%d is negative, but must be unsigned", num);
        throwError = true;
      }
    }
  } else {
    if(num < 0) {
      if(num*-2 > max) {
        sprintf(errorMessage, "%d literal is too low. Min = %d. ", num, (max>>1)*-1);
        throwError = true;
      }
    } else {
      if(2*num >= max) {
        sprintf(errorMessage, "%d literal is too high. Max = %d", num, (max>>1)-1);
        throwError = true;
      }
    }
  }
  if(throwError) {
    if(isOffset) {
      error(4, errorMessage);
    } else {
      error(3, errorMessage);
    }
  }
}

/*
By the time this function is called the user is certain that the literal token is a numeric literal.
The caller should know how many bits this number can occupy and whether or not that number is signed or unsigned
*/
int toLiteral(char* literal, uint8_t maxBits, bool isSigned) {
  int num = 0;
  int i;
  bool throwError = false;
  if(literal[0] == 'X') {
    num = hexToInt(literal+1); /*skip the x*/
  } else if (literal[0] == '#') {
    num = decToInt(literal+1);
  } else {
    sprintf(errorMessage, "%s literal is not formatted correctly", literal);
    throwError = true;
  }
  /* check if within bounds*/
  verifyBitLength(num, maxBits, isSigned, false);
  if(throwError) {
    error(3, errorMessage);
  }
  return num;
}

/*
call to distinguish between literal and register for Opcodes with options for either
*/
bool isLiteral(char* literal) {
  return (literal[0] == 'X' || literal[0] == '#');
}

void main(int32_t argc, char* argv[]) {
  int i;
  if(argc == 3) {
    fileIn = fopen(argv[1], "r");
    fileOut = fopen(argv[2], "w");
    fileOutName = malloc(strlen(argv[2]) + 1);
    strcpy(fileOutName, argv[2]);
  } else {
    error(4, "Invalid input arguments expected ./assemble <input>.asm <output>.obj");
  }
  main_1stPass();
  fclose(fileIn);
  fileIn = fopen(argv[1], "r");
  lineNumber = 0;
  main_2ndPass();
  fclose(fileIn);
  fclose(fileOut);
}

void handleComments(char* string) {
  char *comment = strchr(string, ';');
  if(comment != NULL) {
    string[strlen(string) - strlen(comment)] = '\0'; 
  }
}

/*
1st Pass
Populates labelTable
Error checks for the following (thus, don't need to do these in 2nd pass):
  1. invalid labels
  2. begin/end w/ .ORIG & .END
  3. start address is 16bit
  4. invalid opcodes
Outputs the start address @ .ORIG
*/
void main_1stPass(void) {
  char string[MAX_LINE_LENGTH + 1], *label, *opcode, *a, *b, *c, *d;
  int lRet;
  lineNumber = 0;
  originFound = false;
  do {
    lRet = readAndParse( fileIn, string, &label, &opcode, &a, &b, &c, &d );
    if( lRet != DONE && lRet != EMPTY_LINE ) {
      if(isPsuedoOp(opcode)) {
        switch(getPseudoOp(opcode)) {
          case ORIG:
            verifyNumberOfOperands(a, b, c, d, 1);
            if(!originFound) {
              originFound = true;
              startAddress = toLiteral(a, 16, UNSIGNED);
              if((startAddress % 2) != 0) {
                sprintf(errorMessage, "start address %d must be word-aligned (even)", startAddress);
                error(3, errorMessage);
              }
              output(startAddress);
            }
            break;
          case FILL:
            verifyNumberOfOperands(a, b, c, d, 1);
            verifyOriginFound();
            break;
          case END:
            verifyOriginFound();
            return;
            break;
           default:
            error(4, "invalid pseudoOp");
            break;
        }
      } else if(getOpcode(opcode) == -1){
        sprintf(errorMessage, "%s", opcode);
        error(2, errorMessage);
      }
      verifyOriginFound();    /*first meaningful line has to be .ORIG*/
      if(isLabelValid(label) && (strcmp(label, "") != 0)) {
        addLabel(label, lineNumber);
      }
      lineNumber++;
    }
  } while( lRet != DONE );
  error(4, "NO .END");
  return;
}

void verifyEndOfLine(char* string, char* delimiters) {
  token = strtok(string, delimiters);
  if(token != NULL) {
    sprintf(errorMessage, "Unexpected operand %s", token);
    error(4, errorMessage);
  }
}

/*
all pseudo-op/orig/end formatting is handled, I just need to have the line start counting from where
.ORIG is and handle .END/.FILL
*/
void main_2ndPass(void) {

/* Note: MAX_LINE_LENGTH, OK, EMPTY_LINE, and DONE are defined values */

/*To call readAndParse, you would use the following:*/
  char string[MAX_LINE_LENGTH + 1], *label, *opcode, *a, *b, *c, *d;
  int lRet;
  do {
    lRet = readAndParse( fileIn, string, &label, &opcode, &a, &b, &c, &d );
    if( lRet != DONE && lRet != EMPTY_LINE ) {
      if(isPsuedoOp(opcode)) {
        int pOp = getPseudoOp(opcode);
        switch(getPseudoOp(opcode)) {
          case ORIG:
            continue;
            break;
          case FILL:
            result = toLiteral(a, 17, SIGNED);  /*FIXME: ".FILL can take a signed number or an unsigned number"*/
            if(result > (1<<15)) {
              result = (result - ((1<<16)-1))-1;
            }
            break;
          case END:
            return;
            break;
        }
      } else {
        int tmp;
        int opC = getOpcode(opcode);
        result = opcodeToASM(opC);
        switch(opC) {
          case ADD:
          case AND:
          case XOR:
            verifyNumberOfOperands(a, b, c, d, 3);
            result += (RegisterToInt(a) << 9);
            result += (RegisterToInt(b) << 6);
            if(isLiteral(c)) {
              tmp = toLiteral(c, 5, SIGNED);
              result |= (tmp & 0x1F);
              result += 1<<5;
            } else {
              result += RegisterToInt(c);
            }
            break;  
          case BR:
          case BRN:
          case BRZ:
          case BRP:
          case BRNZ:
          case BRNP:
          case BRZP:
          case BRNZP:
            verifyNumberOfOperands(a, b, c, d, 1);
            tmp = labelToLineNumber(a) - (lineNumber + 2);
            verifyBitLength(tmp, 9, SIGNED, true);
            result |= (tmp & 0x1FF);
            if(opC == BRN || opC == BRNZ || opC == BRNP || opC == BRNZP || opC == BR) {
              result += (1<<11);
            }
            if (opC == BRZ || opC == BRNZ || opC == BRZP || opC == BRNZP || opC == BR) {
              result += (1<<10);
            }
            if(opC == BRP || opC == BRZP || opC == BRNP || opC == BRNZP || opC == BR) {
              result += (1<<9);
            }
            break;
          case HALT:
            verifyNumberOfOperands(a, b, c, d, 0);
            result = opcodeToASM(TRAP);
            result += 0x25;
            break;
          case JMP:
          case JSRR:
            verifyNumberOfOperands(a, b, c, d, 1);
            result += (RegisterToInt(a) << 6);
            break;
          case JSR:
            verifyNumberOfOperands(a, b, c, d, 1);
            result += 1<<11;
            tmp = labelToLineNumber(a) - (lineNumber + 2);
            verifyBitLength(tmp, 11, SIGNED, true);
            result |= (tmp & 0x7FF);
            break;
          case LDB:
          case STB:
          case LDW: /*word-aligned needs handling later*/
          case STW:
            verifyNumberOfOperands(a, b, c, d, 3);
            result += (RegisterToInt(a) << 9);
            result += (RegisterToInt(b) << 6);
            tmp = toLiteral(c,6,SIGNED);
            result |= (tmp & 0x3F);
            break;
          case LEA:
            verifyNumberOfOperands(a, b, c, d, 2);
            result += (RegisterToInt(a) << 9);
            tmp = labelToLineNumber(b) - (lineNumber + 2);
            verifyBitLength(tmp, 9, SIGNED, true);
            result |= (tmp & 0x1FF);
            break;
          case NOT:        
            verifyNumberOfOperands(a, b, c, d, 2);
            result += (RegisterToInt(a) << 9);
            result += (RegisterToInt(b) << 6);
            result += 0x3F;
            break;
          case RET:
            verifyNumberOfOperands(a, b, c, d, 0);
            result += 7<<6;
            break;
          case RTI:
          case NOP:
            verifyNumberOfOperands(a, b, c, d, 0);
            /*RTI & NOP is just the opcode and all 0s after that, so nothing extra needed*/
            break;
          case LSHF:
          case RSHFL:
          case RSHFA:
            verifyNumberOfOperands(a, b, c, d, 3);
            result += (RegisterToInt(a) << 9);
            result += (RegisterToInt(b) << 6);
            tmp = toLiteral(c, 4, UNSIGNED);
            result |= (tmp & 0xF);
            if(opC == RSHFL) {
              result += (1<<4);
            } else if (opC == RSHFA) {
              result += (3<<4);
            }
            break;
          case TRAP:
            verifyNumberOfOperands(a, b, c, d, 1);
            tmp = toLiteral(a, 8, UNSIGNED);
            result |= (tmp & 0xFF);
            break;
          default:
            sprintf(errorMessage, "%s", opcode);
            error(2, errorMessage);
            break;
        }
      }
      output(result);
      lineNumber++;
    }
  } while( lRet != DONE );
  return;
}
/*
Checks what is expected to be a label. This is used in the first pass, before storing a label in the label map.
In the second pass should lookup labels in that map

Labels should:
1. be only alphanumeric characters
2. not start w/ the letter x
3. not be "IN", "OUT", "GETC", or "PUTS"
4. be case-sensitive
*/
bool isLabelValid(char* label) {
  bool valid = true;
  if(label[0] == 'X' || (label[0] >= '0' && label[0] <= '9')) {
    valid = false;
    sprintf(errorMessage, "label %s is invalid", label);
    error(4, errorMessage);
  } else {
    int i = 0;
    while(label[i] != '\0') {
      if(!isalnum(label[i])) {
        valid = false;
        sprintf(errorMessage, "label %s is not alphanumeric", label);
        error(4, errorMessage);
        break;
      }
      if(i >= 20) {
        valid = false;
        sprintf(errorMessage, "label %s is greater than 20 characters", label);
        error(4, errorMessage);
        break;
      }
      i++;
    }
  }
  if(strcmp("IN", label) == 0 || strcmp("OUT", label) == 0 || strcmp("GETC", label) == 0 || strcmp("PUTS", label) == 0) {
    valid = false;
    sprintf(errorMessage, "label %s is an opcode", label);
    error(4, errorMessage);
  }
  if(getOpcode(label) != -1) {
    valid = false;
  }
  return valid;
}


/*
Looks up what is expected to be an opcode in the opcode library.
If it matches a known opcode it will return that opcode's enum value, else -1
*/
enum OPCODE getOpcode(char* opcode) {
  int8_t code = -1;
  int8_t i = 0;
  while(opcodes[i] != NULL) {
  	if(strcmp(opcodes[i], opcode) == 0) {
  	  code = i;
  	  break;
  	} 
  	i++;
  }
  return code;
}

uint16_t RegisterToInt(char* reg)  {
  uint16_t regVal = -1;
  if(!isRegister(reg)) {
    sprintf(errorMessage, "%s is not a valid register, should be R0-R7", reg);
    error(4, errorMessage);
  } else {
    regVal = (uint16_t) decToInt(reg+1);
  }
  return regVal;
}

/*
convert a known opcode into the assembly offset number that it should be encoded into
*/
uint16_t opcodeToASM(enum OPCODE opcode) {
  uint16_t assembly = (uint16_t) opcodeMap[opcode];
  assembly = assembly << 12;
  return assembly;
}

/*
Formats output. Expects output to be a uint16_t. Will convert that number to a string and put 0x in front
1. prints a "0x" before every entry
2. prints every entry as a 4 digit hex in all caps
*/
void output(uint16_t output) {
  fprintf(fileOut, "0x%.4X\n", output);
}

/*
case insensitive convert to int
start with "-####" and convert to int
no bounds checking yet
*/
int hexToInt(char* hex) {
  int i;
  int val = 0;
  int end = 0;
  int scalar = 1;
  int constant;
  int length = strlen(hex);
  bool negative = (hex[0] == '-');
  if(negative) {
    end = 1;
  }

  for(i = strlen(hex)-1; i >= end; i--) {  /* end skips the 'x' (and if negative '-') character(s)*/
    if(hex[i] >= '0' && hex[i] <= '9') {
      constant = ASCIINUM;
    } else if (hex[i] >= 'A' && hex[i] <= 'F') {
      constant = ASCIIUPPER;
    }  else if (hex[i] >= 'a' && hex[i] <= 'f') {
      constant = ASCIILOWER;
    } else {
      sprintf(errorMessage, "%s is a bad hex literal. Needs form of \"x-####\" - optional",hex);
      error(3, errorMessage);
      val = -1;
      break;
    }
    val += scalar*((int) hex[i]-constant);
    scalar *=16;
  }
  if(negative) { 
    val *= -1;
  }
  return val;
}

/*
case insensitive convert to int
start with "-####" and convert to int
no bounds checking yet
*/
int decToInt(char* dec) {
  int i;
  int val = 0;
  int end = 0;
  int scalar = 1;
  int constant;
  int length = strlen(dec);
  bool negative = (dec[0] == '-');
  if(negative) {
    end = 1;
  }
  sprintf(errorMessage, "%s is a bad dec literal. Needs form of \"#-XXXXX\" - optional",dec);

  for(i = strlen(dec)-1; i >= end; i--) {  /* end skips the 'x' (and if negative '-') character(s)*/
    if(dec[i] >= '0' && dec[i] <= '9') {
      constant = ASCIINUM;
    } else {
      error(3, errorMessage);
      val = -1;
      break;
    }
    val += scalar*((int) dec[i]-constant);
    scalar *=10;
  }
  if(negative) { 
    val *= -1;
  }
  return val;
}


/*
BEGIN Source 2: http:/*users.ece.utexas.edu/~patt/15s.460N/labs/lab1/Lab1Functions.html
*/

int  readAndParse( FILE * pInfile, char * pLine, char ** pLabel, char ** pOpcode, char ** pArg1, char ** pArg2, char ** pArg3, char ** pArg4) {
  char * lRet, * lPtr;
  int i;
  if( !fgets( pLine, MAX_LINE_LENGTH, pInfile ) )
  return( DONE );
  for( i = 0; i < strlen( pLine ); i++ )
  pLine[i] = toupper( pLine[i] );

       /* convert entire line to lowercase */
  *pLabel = *pOpcode = *pArg1 = *pArg2 = *pArg3 = *pArg4 = pLine + strlen(pLine);

  /* ignore the comments */
  lPtr = pLine;

  while( *lPtr != ';' && *lPtr != '\0' &&
  *lPtr != '\n' ) 
  lPtr++;

  *lPtr = '\0';
  if( !(lPtr = strtok( pLine, "\t\n\r ," ) ) ) 
  return( EMPTY_LINE );

  if( getOpcode( lPtr ) == -1 && lPtr[0] != '.' ) /* found a label */   
  {
  *pLabel = lPtr;
  if( !( lPtr = strtok( NULL, "\t\n\r ," ) ) ) return( OK );
  }

       *pOpcode = lPtr;

  if( !( lPtr = strtok( NULL, "\t\n\r ," ) ) ) return( OK );

       *pArg1 = lPtr;

       if( !( lPtr = strtok( NULL, "\t\n\r ," ) ) ) return( OK );

  *pArg2 = lPtr;
  if( !( lPtr = strtok( NULL, "\t\n\r ," ) ) ) return( OK );

  *pArg3 = lPtr;

  if( !( lPtr = strtok( NULL, "\t\n\r ," ) ) ) return( OK );

  *pArg4 = lPtr;

  return( OK );
}
/*
END Source 2: http:/*users.ece.utexas.edu/~patt/15s.460N/labs/lab1/Lab1Functions.html
*/

void verifyNumberOfOperands(char* a, char* b, char* c, char* d, int numOperands) {
  bool throwError = false;
  switch(numOperands) {
    case 0:
      throwError = strlen(a);
    break;
    case 1:
      throwError = (strlen(b) != 0) || (strlen(a) == 0);
    break;
    case 2:
      throwError = (strlen(c) != 0) || (strlen(b) == 0);
    break;
    case 3:
      throwError = (strlen(d) != 0) || (strlen(c) == 0);
    break;
    default:
      printf("unexpected number of operands passed to verify %d", numOperands);
    break;
  }
  if(throwError) {  
    sprintf(errorMessage, "incorrect number of operands. operands = %s %s %s %s. expected number = %d", a, b, c, d, numOperands);
    error(4, errorMessage);
  }
}