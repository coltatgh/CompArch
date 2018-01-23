#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>


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

#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_STRING(STRING) #STRING,

enum OPCODE {
    FOREACH_OPCODE(GENERATE_ENUM)
};

static const char *opcodes[] = {
    FOREACH_OPCODE(GENERATE_STRING)
};
//end SOURCE1

//GLOBAL VARIABLES
FILE *fileIn;
FILE *fileOut;
char errorMessage[100];

//FUNCTION PROTOTYPES
bool isLabelValid(char* label);
int getOpcode(char* opcode);





void error(int32_t errorCode, char* extraMessage) {
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
    printf(extraMessage);
  }
  printf("\n");
  //exit(errorCode);
}



void main(void) {
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


void main_testOutput(void) {
	output(3000);
	output(4800);
	output(65535);
	output(0);
}

// void main_testIO(int32_t argc, char* argv[]) {
// 	int i;
//   if(argc == 3) {
//     fileIn = fopen(argv[1], "r");
//     fileOut = fopen(argv[2], "w");
//   } else {
//     error(4, "")
//     printf("expected ./assemble <input.asm> <output.obj>\n");
//   }
//   main_testOutput();
//   fclose(fileIn);
//   fclose(fileOut);
// }

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
int getOpcode(char* opcode) {
  int code = -1;
  int i = 0;
  while(opcodes[i] != NULL) {
	if(strcmp(opcodes[i], opcode) == 0) {
	  code = i;
	  break;
	} 
	i++;
  }
  return code;
}

/*
Formats output. Expects output to be a uint16_t. Will convert that number to a string and put 0x in front
1. prints a "0x" before every entry
2. prints every entry as a 4 digit hex in all caps
*/
void output(uint16_t output) {
	char* string [8];
	sprintf(string, "0x%04X\n", output);
	fputs(string, fileOut);
}






