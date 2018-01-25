/*
1st pass converts everything into uppercase? 
*/

#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <malloc.h>

#define ASCIINUM '0'
#define ASCIIUPPER ('A' - 10)
#define ASCIILOWER ('a' - 10)
#define SIGNED  1
#define UNSIGNED 0

//begin Source 1 = https://stackoverflow.com/questions/9907160/how-to-convert-enum-names-to-string-in-c
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
    1,    //OPCODE(ADD)   
    5,    //OPCODE(AND)  
    0,    //OPCODE(BR)  
    0,    //OPCODE(BRN)   
    0,    //OPCODE(BRZ)   
    0,    //OPCODE(BRP)   
    0,    //OPCODE(BRNZ)   
    0,    //OPCODE(BRNP)   
    0,    //OPCODE(BRZP)   
    0,    //OPCODE(BRNZP)   
    0,    //OPCODE(HALT)   
    12,    //OPCODE(JMP)   
    4,    //OPCODE(JSR)   
    4,    //OPCODE(JSRR)   
    2,    //OPCODE(LDB)   
    4,    //OPCODE(LDW)   
    14,    //OPCODE(LEA)   
    0,    //OPCODE(NOP)   
    9,    //OPCODE(NOT)   
    12,    //OPCODE(RET)   
    13,    //OPCODE(RSHFL)   
    13,    //OPCODE(RSHFA)   
    8,    //OPCODE(RTI)   
    3,    //OPCODE(STB)   
    7,    //OPCODE(STW)   
    15,    //OPCODE(TRAP)   
    9    //OPCODE(XOR)   
};

//end SOURCE1

//GLOBAL VARIABLES
FILE *fileIn = NULL;
FILE *fileOut = NULL;
char errorMessage[120];
uint32_t lineNumber = 1;
uint16_t result = 0;

//FUNCTION PROTOTYPES
bool isLabelValid(char* label);
enum OPCODE getOpcode(char* opcode);
int getPseudoOp(char* pseudoOp);
void output(uint16_t output);
int hexToInt(char* hex);
int decToInt(char* dec);
uint16_t opcodeToASM(enum OPCODE opcode);
uint16_t RegisterToInt(char* reg);

void error(int32_t errorCode, char* extraMessage) {
  printf("\nLINE #%d\t", lineNumber);
  switch(errorCode) {
    case 0:
    case 1:
      printf("ERROR 1: UNDEFINED LABEL\t");
      break;
    case 2:
      printf("ERROR 2: INVALID OPCODE\t");
      break;
    case 3:
      printf("ERROR 3: INVALID CONSTANT\t");
      break;
    case 4:
      printf("ERROR 4: MISCELLANEOUS \t");
      break;
    default:
      break;
  }
  if(extraMessage) {
    printf("%s",extraMessage);
  }
  printf("\n");
  // if(fileOut != NULL) {
  //   fclose(fileOut);
  // }
  // fileOut = fopen("out.obj", "w"); //TODO: change to non-magic string
  // fputs("", fileOut); //clear contents of the output file if there is an error
  // fclose(fileOut);
  // exit(errorCode);
}

void main_testLabel(void) {
	char* label1 = "AND";
	char* label2 = "$AND1";
	char* label3 = "AND1";
	char* label4 = "PUTS";
	printf("%d %d %d %d", isLabelValid(label1), isLabelValid(label2), isLabelValid(label3), isLabelValid(label4));
}


void main_testGetOpcode(void) {
	char* opcode1 = "AND";
	char* opcode2 = "BRNZP";
	char* opcode3 = "LEL";
	char* opcode4 = "HALT";
	printf("%d %d %d %d", getOpcode(opcode1), getOpcode(opcode2), getOpcode(opcode3), getOpcode(opcode4));
}

void main_TestParse1(void) {
  char* pOp1 = ".ORIG";
  char* pOp2 = ".FILL";
  char* pOp3 = ".END";
  char* pOp4 = "HALT";
  printf("PSEUDO\n%d %d %d %d", getPseudoOp(pOp1), getPseudoOp(pOp2), getPseudoOp(pOp3), getPseudoOp(pOp4));

  printf("HEX\n%d %d %d %d", hexToInt("x-1000"), hexToInt("a8909"), hexToInt("xBEEF"), hexToInt("xBEEFF"));

  printf("DEC\n%d %d %d %d", decToInt("#-1000"), decToInt("x8909"), decToInt("#1000"), decToInt("#65000"));
}



/*
By the time this function is called the user is certain that the literal token is a numeric literal.
The caller should know how many bits this number can occupy and whether or not that number is signed or unsigned
*/
int toLiteral(char* literal, uint8_t maxBits, bool isSigned) {
  int num = 0;
  int i;
  int max = 1 << maxBits;
  bool throwError = false;
  if(literal[0] == 'x') {
    num = hexToInt(literal+1); //skip the x
  } else if (literal[0] == '#') {
    num = decToInt(literal+1);
  } else {
    sprintf(errorMessage, "%s literal is not formatted correctly", literal);
    throwError = true;
  }
  // check if within bounds
  if(isSigned) {
    if(num >= max) {
      sprintf(errorMessage, "%s literal is too large. Max = %d", literal, max-1);
      throwError = true;
    }
  } else {
    if(num < 0) {
      if(num*-2 > max) {
        sprintf(errorMessage, "%s literal is too low. Min = %d. ", literal, (max>>1)*-1);
        throwError = true;
      }
    } else {
      if(2*num >= max) {
        sprintf(errorMessage, "%s literal is too high. Max = %d", literal, (max>>1)-1);
        throwError = true;
      }
    }
  }
  if(throwError) {
    error(3, errorMessage);
  }
}

/*
call to distinguish between literal and register for Opcodes with options for either
*/
bool isLiteral(char* literal) {
  return (literal[0] == 'x' || literal[0] == '#');
}


void main(int32_t argc, char* argv[]) {
  int i;
  if(argc == 3) {
    fileIn = fopen(argv[1], "r");
    fileOut = fopen(argv[2], "w");
  } else {
    error(4, "");
    printf("expected ./assemble <input.asm> <output.obj>\n");
  }

  ssize_t line_size = 0;
  ssize_t line_buf_size = 0;
  char *token = malloc(10);
  char *string;
  char *delimiters = ", \t\n";
  char *b = malloc(10);
  char *c = malloc(10);
  char *a = malloc(10);

  line_size = getline(&string, &line_buf_size, fileIn);
  string[strlen(string) - 1] = '\0'; // getline adds a newline character after the string. remove that.
  while(line_size != -1) {
    result = 0;
    token = strtok(string, delimiters);
    enum OPCODE opcode = getOpcode(token);
    result = opcodeToASM(opcode);
    switch(opcode) {
      case ADD:
      case AND:
        a = strtok(NULL, delimiters);
        result += (RegisterToInt(a) << 9);
        b = strtok(NULL, delimiters);
        result += (RegisterToInt(b) << 6);
        c = strtok(NULL, delimiters);

        printf("%s %s %s\n", a, b, c);
        if(isLiteral(c)) {
          result += toLiteral(c, 5, SIGNED);
          result += 1<<5;
        } else {
          result += RegisterToInt(c);
        }
        break;   
      //TODO: BR goes to labels which have been generated in the first pass  
      case JMP:
      case JSRR:
        a = strtok(NULL, delimiters);
        result += (RegisterToInt(a) << 6);
        break;
      case JSR:
        result += 1<<11;
        //TODO: LABEL
        break;
      case LDB:
      case LDW:
      case STB:
      case STW:
        a = strtok(NULL, delimiters);
        result += (RegisterToInt(a) << 9);
        b = strtok(NULL, delimiters);
        result += (RegisterToInt(b) << 6);
        c = strtok(NULL, delimiters);
        result += toLiteral(c,6,SIGNED);  //unsure if difference b/t STB/STW & LDB/LDW (left shift 1 bit) is @ compile time or runtime? 
        break;
      case LEA:
        a = strtok(NULL, delimiters);
        result += (RegisterToInt(a) << 9);
        b = strtok(NULL, delimiters);
        //TODO: b is label
        break;
      case NOT:        
        a = strtok(NULL, delimiters);
        result += (RegisterToInt(a) << 9);
        b = strtok(NULL, delimiters);
        result += (RegisterToInt(b) << 6);
        result += 0x3F;
        break;
      case RET:
        result += 7<<6;
        break;
      case RTI:
        //RTI is just the opcode and all 0s after that, so nothing extra needed
        break;
      case SHF:
      
        break;
      default:
        sprintf(errorMessage, "1st token = %s", token);
        break;
    }
    output(result);
    line_size = getline(&string, &line_buf_size, fileIn);
    lineNumber++;
  }
  free(string);
  fclose(fileOut);
}

void main_testOutput(void) {
	output(3000);
	output(4800);
	output(65535);
	output(0);
}

void main_testIO(int32_t argc, char* argv[]) {
  int i;
  if(argc == 3) {
    fileIn = fopen(argv[1], "r");
    fileOut = fopen(argv[2], "w");
  } else {
    error(4, "");
    printf("expected ./assemble <input.asm> <output.obj>\n");
  }
 fclose(fileIn);
 fclose(fileOut);
}

/*
returns the enum for the pseudoOp if it is one, else -1
*/
int getPseudoOp(char* pseudoOp) {
  int code = -1;
  int i = 0;
  if(pseudoOp[0] != '.') {
    sprintf(errorMessage, "%s is not a valid pseudoOp", pseudoOp);
    error(2, errorMessage);
  } else {
    pseudoOp+=1; // skip the "."
    while(pseudoOps[i] != NULL) {
      if(strcmp(pseudoOps[i], pseudoOp) == 0) {
        code = i;
        break;
      } 
      i++;
    }
  }
  return code;
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
  if(label[0] == 'x') {
    valid = false;
  } else {
    int i = 0;
    while(label[i] != NULL) {
      if(!isalnum(label[i])) {
        valid = false;
        sprintf(errorMessage, "label %s is not alphanumeric", label);
        error(1, errorMessage);
        break;
      }
      i++;
    }
  }
  if(strcmp("IN", label) == 0 || strcmp("OUT", label) == 0 || strcmp("GETC", label) == 0 || strcmp("PUTS", label) == 0) {
    valid = false;
    sprintf(errorMessage, "label %s is an opcode", label);
    error(1, errorMessage);
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
  int number = decToInt(reg+1); // skip the R
  if(number > 7 || number < 0 || (reg[0] != 'r' && reg[0] != 'R')) {
    sprintf(errorMessage, "%s is not a valid register, should be R0-R7", reg);
    error(4, errorMessage);
  }
  return (uint16_t) number;
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
	char string [8];
  sprintf(string, "0x%04X\n", output);
	fputs(string, fileOut);
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

  sprintf(errorMessage, "%s is a bad hex literal. Needs form of \"x-####\" - optional",hex);
  for(i = strlen(hex)-1; i >= end; i--) {  // end skips the 'x' (and if negative '-') character(s)
    if(hex[i] >= '0' && hex[i] <= '9') {
      constant = ASCIINUM;
    } else if (hex[i] >= 'A' && hex[i] <= 'F') {
      constant = ASCIIUPPER;
    }  else if (hex[i] >= 'a' && hex[i] <= 'f') {
      constant = ASCIILOWER;
    } else {
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

  for(i = strlen(dec)-1; i >= end; i--) {  // end skips the 'x' (and if negative '-') character(s)
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

