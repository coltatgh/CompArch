/*
   REFER TO THE SUBMISSION INSTRUCTION FOR DETAILS

   Name 1: Karl Solomon
   Name 2: Colton Lewis
   UTEID 1: Kws653
   UTEID 2: Ctl492
*/

/***************************************************************/
/*                                                             */
/*   LC-3b Instruction Level Simulator                         */
/*                                                             */
/*   EE 460N                                                   */
/*   The University of Texas at Austin                         */
/*                                                             */
/***************************************************************/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

/***************************************************************/
/*                                                             */
/* Files: isaprogram   LC-3b machine language program file     */
/*                                                             */
/***************************************************************/

/***************************************************************/
/* These are the functions you'll have to write.               */
/***************************************************************/

void process_instruction();
int16_t fetch_instruction();
void execute(uint16_t instruction);
uint16_t readWord(uint16_t address);
/***************************************************************/
/* A couple of useful definitions.                             */
/***************************************************************/
#define FALSE 0
#define TRUE  1

/***************************************************************/
/* Use this to avoid overflowing 16 bits on the bus.           */
/***************************************************************/
#define Low16bits(x) ((x) & 0xFFFF)
#define Low8bits(x)  ((x)  & 0xFF)
#define High8bits(x)  ((x)  & 0xFF00)

/***************************************************************/
/* Main memory.                                                */
/***************************************************************/
/* MEMORY[A][0] stores the least significant byte of word at word address A
   MEMORY[A][1] stores the most significant byte of word at word address A 
*/

#define WORDS_IN_MEM    (0x08000) 
int MEMORY[WORDS_IN_MEM][2];

/***************************************************************/
/*Opcode handling                                              */
/***************************************************************/
#define OPCODE_MASK 0xF000
#define Nbit 0x0800
#define Zbit 0x0400
#define Pbit 0x0200

enum OPCODES {
  BR_NOP, /*BR, NOP*/
  ADD,
  LDB,
  STB,
  JSR_R, /*JSR, JSRR*/
  AND,
  LDW,
  STW,
  RTI,
  XOR_NOT, /*XOR, NOT*/
  UNUSED,
  UNUSED2,
  JMP_RET,
  SHF,
  LEA,
  TRAP
};

/***************************************************************/
/* LC-3b State info.                                           */
/***************************************************************/
#define LC_3b_REGS 8

int RUN_BIT;        /* run bit */


typedef struct System_Latches_Struct {
 int PC,                /* program counter */
 N,                /* n condition bit */
 Z,                /* z condition bit */
 P;                /* p condition bit */
 int REGS[LC_3b_REGS]; /* register file. */
} System_Latches;

/* Data Structure for Latch */

System_Latches CURRENT_LATCHES, NEXT_LATCHES;  /*Karl: when to use CURRENT_LATCHES VS NEXT_LATCHES IN SWITCH STATEMENT???*/

/***************************************************************/
/* A cycle counter.                                            */
/***************************************************************/
int INSTRUCTION_COUNT;

/***************************************************************/
/*                                                             */
/* Procedure : help                                            */
/*                                                             */
/* Purpose   : Print out a list of commands                    */
/*                                                             */
/***************************************************************/
void help() {                                                    
 printf("----------------LC-3b ISIM Help-----------------------\n");
 printf("go               -  run program to completion         \n");
 printf("run n            -  execute program for n instructions\n");
 printf("mdump low high   -  dump memory from low to high      \n");
 printf("rdump            -  dump the register & bus values    \n");
 printf("?                -  display this help menu            \n");
 printf("quit             -  exit the program                  \n\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : cycle                                           */
/*                                                             */
/* Purpose   : Execute a cycle                                 */
/*                                                             */
/***************************************************************/
void cycle() {                                                
 process_instruction();
 CURRENT_LATCHES = NEXT_LATCHES;
 INSTRUCTION_COUNT++;
}

/***************************************************************/
/*                                                             */
/* Procedure : run n                                           */
/*                                                             */
/* Purpose   : Simulate the LC-3b for n cycles                 */
/*                                                             */
/***************************************************************/
void run(int num_cycles) {                                      
 int i;

 if (RUN_BIT == FALSE) {
   printf("Can't simulate, Simulator is halted\n\n");
   return;
 }

 printf("Simulating for %d cycles...\n\n", num_cycles);
 for (i = 0; i < num_cycles; i++) {
   if (CURRENT_LATCHES.PC == 0x0000) {
            RUN_BIT = FALSE;
            printf("Simulator halted\n\n");
            break;
   }
   cycle();
 }
}

/***************************************************************/
/*                                                             */
/* Procedure : go                                              */
/*                                                             */
/* Purpose   : Simulate the LC-3b until HALTed                 */
/*                                                             */
/***************************************************************/
void go() {                                                     
 if (RUN_BIT == FALSE) {
   printf("Can't simulate, Simulator is halted\n\n");
   return;
 }

 printf("Simulating...\n\n");
 while (CURRENT_LATCHES.PC != 0x0000)
   cycle();
 RUN_BIT = FALSE;
 printf("Simulator halted\n\n");
}

/***************************************************************/ 
/*                                                             */
/* Procedure : mdump                                           */
/*                                                             */
/* Purpose   : Dump a word-aligned region of memory to the     */
/*             output file.                                    */
/*                                                             */
/***************************************************************/
void mdump(FILE * dumpsim_file, int start, int stop) {          
 int address; /* this is a byte address */

 printf("\nMemory content [0x%.4x..0x%.4x] :\n", start, stop);
 printf("-------------------------------------\n");
 for (address = (start >> 1); address <= (stop >> 1); address++)
   printf("  0x%.4x (%d) : 0x%.2x%.2x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
 printf("\n");

 /* dump the memory contents into the dumpsim file */
 fprintf(dumpsim_file, "\nMemory content [0x%.4x..0x%.4x] :\n", start, stop);
 fprintf(dumpsim_file, "-------------------------------------\n");
 for (address = (start >> 1); address <= (stop >> 1); address++)
   fprintf(dumpsim_file, " 0x%.4x (%d) : 0x%.2x%.2x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
 fprintf(dumpsim_file, "\n");
 fflush(dumpsim_file);
}

/***************************************************************/
/*                                                             */
/* Procedure : rdump                                           */
/*                                                             */
/* Purpose   : Dump current register and bus values to the     */   
/*             output file.                                    */
/*                                                             */
/***************************************************************/
void rdump(FILE * dumpsim_file) {                               
 int k; 

 printf("\nCurrent register/bus values :\n");
 printf("-------------------------------------\n");
 printf("Instruction Count : %d\n", INSTRUCTION_COUNT);
 printf("PC                : 0x%.4x\n", CURRENT_LATCHES.PC);
 printf("CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
 printf("Registers:\n");
 for (k = 0; k < LC_3b_REGS; k++)
   printf("%d: 0x%.4x\n", k, CURRENT_LATCHES.REGS[k]);
 printf("\n");

 /* dump the state information into the dumpsim file */
 fprintf(dumpsim_file, "\nCurrent register/bus values :\n");
 fprintf(dumpsim_file, "-------------------------------------\n");
 fprintf(dumpsim_file, "Instruction Count : %d\n", INSTRUCTION_COUNT);
 fprintf(dumpsim_file, "PC                : 0x%.4x\n", CURRENT_LATCHES.PC);
 fprintf(dumpsim_file, "CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
 fprintf(dumpsim_file, "Registers:\n");
 for (k = 0; k < LC_3b_REGS; k++)
   fprintf(dumpsim_file, "%d: 0x%.4x\n", k, CURRENT_LATCHES.REGS[k]);
 fprintf(dumpsim_file, "\n");
 fflush(dumpsim_file);
}

/***************************************************************/
/*                                                             */
/* Procedure : get_command                                     */
/*                                                             */
/* Purpose   : Read a command from standard input.             */  
/*                                                             */
/***************************************************************/
void get_command(FILE * dumpsim_file) {                         
 char buffer[20];
 int start, stop, cycles;

 printf("LC-3b-SIM> ");

 scanf("%s", buffer);
 printf("\n");

 switch(buffer[0]) {
 case 'G':
 case 'g':
   go();
   break;

 case 'M':
 case 'm':
   scanf("%i %i", &start, &stop);
   mdump(dumpsim_file, start, stop);
   break;

 case '?':
   help();
   break;
 case 'Q':
 case 'q':
   printf("Bye.\n");
   exit(0);

 case 'R':
 case 'r':
   if (buffer[1] == 'd' || buffer[1] == 'D')
            rdump(dumpsim_file);
   else {
            scanf("%d", &cycles);
            run(cycles);
   }
   break;

 default:
   printf("Invalid Command\n");
   break;
 }
}

/***************************************************************/
/*                                                             */
/* Procedure : init_memory                                     */
/*                                                             */
/* Purpose   : Zero out the memory array                       */
/*                                                             */
/***************************************************************/
void init_memory() {                                           
 int i;

 for (i=0; i < WORDS_IN_MEM; i++) {
   MEMORY[i][0] = 0;
   MEMORY[i][1] = 0;
 }
}

/**************************************************************/
/*                                                            */
/* Procedure : load_program                                   */
/*                                                            */
/* Purpose   : Load program and service routines into mem.    */
/*                                                            */
/**************************************************************/
void load_program(char *program_filename) {                   
 FILE * prog;
 int ii, word, program_base;

 /* Open program file. */
 prog = fopen(program_filename, "r");
 if (prog == NULL) {
   printf("Error: Can't open program file %s\n", program_filename);
   exit(-1);
 }

 /* Read in the program. */
 if (fscanf(prog, "%x\n", &word) != EOF)
   program_base = word >> 1;
 else {
   printf("Error: Program file is empty\n");
   exit(-1);
 }

 ii = 0;
 while (fscanf(prog, "%x\n", &word) != EOF) {
   /* Make sure it fits. */
   if (program_base + ii >= WORDS_IN_MEM) {
            printf("Error: Program file %s is too long to fit in memory. %x\n",
            program_filename, ii);
            exit(-1);
   }

   /* Write the word to memory array. */
   MEMORY[program_base + ii][0] = word & 0x00FF;
   MEMORY[program_base + ii][1] = (word >> 8) & 0x00FF;
   ii++;
 }

 if (CURRENT_LATCHES.PC == 0) CURRENT_LATCHES.PC = (program_base << 1);

 printf("Read %d words from program into memory.\n\n", ii);
}

/************************************************************/
/*                                                          */
/* Procedure : initialize                                   */
/*                                                          */
/* Purpose   : Load machine language program                */ 
/*             and set up initial state of the machine.     */
/*                                                          */
/************************************************************/
void initialize(char *program_filename, int num_prog_files) { 
 int i;

 init_memory();
 for ( i = 0; i < num_prog_files; i++ ) {
   load_program(program_filename);
   while(*program_filename++ != '\0');
 }
 CURRENT_LATCHES.Z = 1;  
 NEXT_LATCHES = CURRENT_LATCHES;
   
 RUN_BIT = TRUE;
}

/***************************************************************/
/*                                                             */
/* Procedure : main                                            */
/*                                                             */
/***************************************************************/
int main(int argc, char *argv[]) {                              
 FILE * dumpsim_file;

 /* Error Checking */
 if (argc < 2) {
   printf("Error: usage: %s <program_file_1> <program_file_2> ...\n",
          argv[0]);
   exit(1);
 }

 printf("LC-3b Simulator\n\n");

 initialize(argv[1], argc - 1);

 if ( (dumpsim_file = fopen( "dumpsim", "w" )) == NULL ) {
   printf("Error: Can't open dumpsim file\n");
   exit(-1);
 }

 while (1)
   get_command(dumpsim_file);
   
}

/***************************************************************/
/* Do not modify the above code.
  You are allowed to use the following global variables in your
  code. These are defined above.

  MEMORY

  CURRENT_LATCHES
  NEXT_LATCHES

  You may define your own local/global variables and functions.
  You may use the functions to get at the control bits defined
  above.

  Begin your code here                                          */

/***************************************************************/

int16_t fetch_instruction(){
  return readWord(CURRENT_LATCHES.PC);   /*Colton: any  internal checks necessary here?*/
}

void process_instruction(){
  NEXT_LATCHES = CURRENT_LATCHES; /* keep before calling execute. might be unnecessary, but I'm being extra safe*/
  incrementPC();
  execute((uint16_t)fetch_instruction());

 /*  function: process_instruction
  *  
  *    Process one instruction at a time  
  *       -Fetch one instruction
  *       -Decode 
  *       -Execute
  *       -Update NEXT_LATCHES  /*DONE
  */


}

/*
returns a bitmask of instruction from startBit to endBit (inclusive) that has been shifted into the least significant bits
*/
uint16_t maskAndShiftDown(uint16_t instruction, uint16_t startBit, uint16_t endBit) {
  int result = 0;
  int i = 0;
  for(i = 0; i <= (endBit-startBit); i++) {
    result <<=1;
    result |= 1;
  }
  result <<= startBit;
  instruction = instruction & result;
  instruction >>= startBit;
  return instruction;
}

uint16_t readRegister(uint16_t registerNumber) {
  int reg = CURRENT_LATCHES.REGS[registerNumber];
  reg = Low16bits(reg);
  return reg;
}

void writeRegister(uint16_t registerNumber, uint16_t newValue) {
  NEXT_LATCHES.REGS[registerNumber] = newValue;
}

int16_t signExtend(uint16_t signedNumber, uint16_t bitsOccupied) {
  int16_t result = 0;
  if(signedNumber & (1 << (bitsOccupied-1))) {
    signedNumber = ~signedNumber;             
    signedNumber += 1;
  }
  return (int16_t) signedNumber;          /* Colton: compare this to piazza  //Karl: compare whole function to piazza or what?*/
}

uint16_t readWord(uint16_t address) {
  uint16_t word;                              
  word = Low16bits(MEMORY[address/2][1]);     
  word <<= 8;
  word += Low16bits(MEMORY[address/2][0]);
  return word;
}

uint16_t readByte(uint16_t address) {
  uint16_t byte;
  if(address % 2)
    byte = Low8bits(MEMORY[address/2][1]);
  else                                    
    byte = Low8bits(MEMORY[address/2][0]);
  return byte;
}

void writeWord(uint16_t address, uint16_t word) {
  MEMORY[address/2][1] = High8bits(word);
  MEMORY[address/2][0] = Low8bits(word);
}

/*Input byte should be formatted in the least significant byte*/
void writeByte(uint16_t address, uint16_t byte) {
  if(address % 2)
    MEMORY[address/2][1] = Low8bits(byte);
  else
    MEMORY[address/2][0] = Low8bits(byte);
}

void incrementPC(void) {
  NEXT_LATCHES.PC += 2;
}

void setConditionCodes(int16_t value) {
  if(value > 0) {
    NEXT_LATCHES.N = 0;
    NEXT_LATCHES.Z = 0;
    NEXT_LATCHES.P = 1;
  } else if(value < 0) {
    NEXT_LATCHES.N = 1;
    NEXT_LATCHES.Z = 0;
    NEXT_LATCHES.P = 0;
  } else {
    NEXT_LATCHES.N = 0;
    NEXT_LATCHES.Z = 1;
    NEXT_LATCHES.P = 0;
  }
}

/* likely need to handle doing operations with numbers that I SEXT, but that are cast as uint16_t*/
void execute(uint16_t instruction) {
  uint8_t opcode = (instruction & OPCODE_MASK) >> 12;
  uint16_t dr = maskAndShiftDown(instruction, 9, 11);
  uint16_t r1 = readRegister(dr);
  uint16_t r2 = readRegister(maskAndShiftDown(instruction, 6,8));
  uint16_t r3 = readRegister(maskAndShiftDown(instruction, 0,2));
  uint16_t bit5Mask = maskAndShiftDown(instruction, 5,5);
  uint16_t bit4Mask = maskAndShiftDown(instruction, 4,4);
  uint16_t bit11Mask = maskAndShiftDown(instruction, 11,11);
  uint16_t imm5 = maskAndShiftDown(instruction, 0,4);
  uint16_t imm4 = maskAndShiftDown(instruction, 0,3);
  uint16_t imm6 = maskAndShiftDown(instruction, 0,5);
  uint16_t PCOffset9 = maskAndShiftDown(instruction, 0, 8);  
  uint16_t PCOffset11 = maskAndShiftDown(instruction, 0, 10);
  uint16_t result;
  switch(opcode) {
    case ADD:
      if(bit5Mask) {
        result = (int16_t) r2 + signExtend(imm5, 5);  /*Karl: do we need to typecase r2 to int16_t for the add to work right or nah? */
      } else {
        result = (int16_t) r2 + (int16_t) r3;
      }
      writeRegister(dr, (int16_t) result);
      setConditionCodes((int16_t) result);
    break;
    case AND:
      if(bit5Mask) {
        result = r2 & signExtend(imm5, 5);
      } else {
        result = r2 & r3;
      }
      writeRegister(dr, result);
      setConditionCodes((int16_t) result);
    break;
    case BR_NOP:
      /*r1 contains nzp bits*/
      if(r1) {
        /*BR*/
        bool branch = false;
        if(instruction & Nbit) {
          if(CURRENT_LATCHES.N) {
            branch = true;
          }
        }
        if(instruction & Zbit) {
          if(CURRENT_LATCHES.Z) {
            branch = true;
          }
        }
        if(instruction & Pbit) {
          if(CURRENT_LATCHES.P) {
            branch = true;
          }
        }
        if(branch){
          int16_t offset = signExtend(PCOffset9, 9);
          NEXT_LATCHES.PC = NEXT_LATCHES.PC + (offset << 1); /*write with NEXT_LATCHES.PC b/c assignment is post-increment*/
        }
      } else {
        /*NOP, do nothing*/
      }
    break;
    case JMP_RET: /*works for JMP & RET b/c in RET r2 is just R7*/
      NEXT_LATCHES.PC = r2;
    break;
    case JSR_R:
      writeRegister(7, NEXT_LATCHES.PC);
      if(instruction & (1<<11)) {
        /*JSR*/
        PCOffset11 = signExtend(PCOffset11, 11);
        NEXT_LATCHES.PC += (PCOffset11 << 1); /*write with NEXT_LATCHES.PC b/c assignment is post-increment*/
      } else {
        /*JSRR*/        
        NEXT_LATCHES.PC = r2;
      }
    break;
    case LDB:
    /*8-bit contents of memory at this address are sign-extended and stored into DR*/
      r1 = signExtend(readByte(r2 + signExtend(imm6, 6)),8);
      writeRegister(dr, r1);
      setConditionCodes((int16_t) dr);
    break;
    case LDW:
      r1 = readWord(r2 + (signExtend(imm6, 6) << 1));
      writeRegister(dr, r1);      
      setConditionCodes((int16_t) dr);
    break;
    case LEA:
      r1 = NEXT_LATCHES.PC + (signExtend(PCOffset9, 9) << 1); 
      writeRegister(dr, r1);
      setConditionCodes((int16_t) dr);
    break;
    case RTI:
    /*You do not have to implement the RTI instruction for this lab. You can assume that the
    input file to your simulator will not contain any RTI instructions*/
    break;
    case SHF:
      if((instruction & (1<<4)) == 0)
        r2 <<= imm4;
      else if ((instruction & (1<<5)) == 0)
        r2 >>= imm4;
      else {
        int i = 0;
        uint16_t firstBit = instruction & 0x8000;
        for(i = 0; i < imm4; i++) {
          r2 >>= 1;
          r2 |= firstBit;
        }
      }
      r1 = r2;
      writeRegister(dr, r1);
      setConditionCodes((int16_t) dr);
    break;
    case STB:
      writeByte(r2 + signExtend(imm6, 6), r1);
    break;
    case STW:
      writeWord(r2 + (signExtend(imm6,6) << 1), r1);
    break;
    case TRAP:
      writeRegister(7, NEXT_LATCHES.PC);
      NEXT_LATCHES.PC = readWord(maskAndShiftDown(instruction, 0,7) << 1);
    break;
    case XOR_NOT:
      if(instruction & (1<<5)) {
        r1 = r2 ^ signExtend(imm5, 5);
      } else {
        r1 = r2 ^ r3;
      }
      writeRegister(dr, r1);      
      setConditionCodes((int16_t) dr);
    break;
    default:
    break;
  }
}