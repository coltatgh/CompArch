/***************************************************************/
/*                                                             */
/*   LC-3b Simulator                                           */
/*                                                             */
/*   EE 460N                                                   */
/*   The University of Texas at Austin                         */
/*                                                             */
/***************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

/***************************************************************/
/*                                                             */
/* Files:  ucode        Microprogram file                      */
/*         isaprogram   LC-3b machine language program file    */
/*                                                             */
/***************************************************************/

/***************************************************************/
/* These are the functions you'll have to write.               */
/***************************************************************/

void eval_micro_sequencer();
void cycle_memory();
void eval_bus_drivers();
void drive_bus();
void latch_datapath_values();

/***************************************************************/
/* A couple of useful definitions.                             */
/***************************************************************/
#define FALSE 0
#define TRUE  1

/***************************************************************/
/* Use this to avoid overflowing 16 bits on the bus.           */
/***************************************************************/
#define Low16bits(x) ((x) & 0xFFFF)

/***************************************************************/
/* Definition of the control store layout.                     */
/***************************************************************/
#define CONTROL_STORE_ROWS 64
#define INITIAL_STATE_NUMBER 18

/***************************************************************/
/* Definition of bit order in control store word.              */
/***************************************************************/
enum CS_BITS {                                                  
    IRD,
    COND2, COND1, COND0,
    J5, J4, J3, J2, J1, J0,
    LD_MAR,
    LD_MDR,
    LD_IR,
    LD_BEN,
    LD_REG,
    LD_CC,
    LD_PC,
    LD_SSR,
    TOGL_PSR,
    LD_PSR,
    GATE_PC,
    GATE_MDR,
    GATE_ALU,
    GATE_MARMUX,
    GATE_SHF,
    GATE_SSR,
    GATE_PSR,
    GATE_VECTOR,
    PCMUX1, PCMUX0,
    DRMUX1, DRMUX0,
    SR1MUX1, SR1MUX0,
    ADDR1MUX,               //35
    ADDR2MUX1, ADDR2MUX0,
    MARMUX,
    SSRMUX,
    ALUK1, ALUK0,
    MIO_EN,
    R_W,
    DATA_SIZE,
    LSHF1,
    CONTROL_STORE_BITS
} CS_BITS;

/***************************************************************/
/* Functions to get at the control bits.                       */
/***************************************************************/
int GetIRD(int *x)           { return(x[IRD]); }
int GetCOND(int *x)          { return((x[COND2] << 2) + (x[COND1] << 1) + x[COND0]); }  //Colt
int GetJ(int *x)             { return((x[J5] << 5) + (x[J4] << 4) +
                      (x[J3] << 3) + (x[J2] << 2) +
                      (x[J1] << 1) + x[J0]); }
int GetLD_MAR(int *x)        { return(x[LD_MAR]); }
int GetLD_MDR(int *x)        { return(x[LD_MDR]); }
int GetLD_IR(int *x)         { return(x[LD_IR]); }
int GetLD_BEN(int *x)        { return(x[LD_BEN]); }
int GetLD_REG(int *x)        { return(x[LD_REG]); }
int GetLD_CC(int *x)         { return(x[LD_CC]); }
int GetLD_PC(int *x)         { return(x[LD_PC]); }

int GetLD_SSR(int *x)        { return(x[LD_SSR]);}      //Colt
int GetTOGL_PSR(int *x)      { return(x[TOGL_PSR]);}    //Colt
int GetLD_PSR(int *x)        { return(x[LD_PSR]);}      //Colt

int GetGATE_PC(int *x)       { return(x[GATE_PC]); }
int GetGATE_MDR(int *x)      { return(x[GATE_MDR]); }
int GetGATE_ALU(int *x)      { return(x[GATE_ALU]); }
int GetGATE_MARMUX(int *x)   { return(x[GATE_MARMUX]); }
int GetGATE_SHF(int *x)      { return(x[GATE_SHF]); }

int GetGATE_SSR(int *x)      { return(x[GATE_SSR]);}            //Colt
int GetGATE_PSR(int *x)      { return(x[GATE_PSR]);}            //Colt
int GetGATE_VECTOR(int *x)   { return(x[GATE_VECTOR]);}         //Colt

int GetPCMUX(int *x)         { return((x[PCMUX1] << 1) + x[PCMUX0]); }
int GetDRMUX(int *x)         { return((x[DRMUX1] << 1) + x[DRMUX0]); }
int GetSR1MUX(int *x)        { return((x[SR1MUX1] << 1) + x[SR1MUX0]); }
int GetADDR1MUX(int *x)      { return(x[ADDR1MUX]); }
int GetADDR2MUX(int *x)      { return((x[ADDR2MUX1] << 1) + x[ADDR2MUX0]); }
int GetMARMUX(int *x)        { return(x[MARMUX]); }

int GetSSRMUX(int *x)        { return(x[SSRMUX]);}              //Colt

int GetALUK(int *x)          { return((x[ALUK1] << 1) + x[ALUK0]); }
int GetMIO_EN(int *x)        { return(x[MIO_EN]); }
int GetR_W(int *x)           { return(x[R_W]); }
int GetDATA_SIZE(int *x)     { return(x[DATA_SIZE]); } 
int GetLSHF1(int *x)         { return(x[LSHF1]); }

/***************************************************************/
/* The control store rom.                                      */
/***************************************************************/
int CONTROL_STORE[CONTROL_STORE_ROWS][CONTROL_STORE_BITS];

/***************************************************************/
/* Main memory.                                                */
/***************************************************************/
/* MEMORY[A][0] stores the least significant byte of word at word address A
   MEMORY[A][1] stores the most significant byte of word at word address A 
   There are two write enable signals, one for each byte. WE0 is used for 
   the least significant byte of a word. WE1 is used for the most significant 
   byte of a word. */

#define WORDS_IN_MEM    0x08000 
#define MEM_CYCLES      5
#define VECTOR_TABLE    0x200
int MEMORY[WORDS_IN_MEM][2];

/***************************************************************/

/***************************************************************/

/***************************************************************/
/* LC-3b State info.                                           */
/***************************************************************/
#define LC_3b_REGS 8

int RUN_BIT;    /* run bit */
int BUS;    /* value of the bus */

typedef struct System_Latches_Struct{

int PC,        /* program counter */
    MDR,    /* memory data register */
    MAR,    /* memory address register */
    IR,        /* instruction register */
    N,        /* n condition bit */
    Z,        /* z condition bit */
    P,        /* p condition bit */
    BEN;        /* ben register */

int READY;    /* ready bit */
  /* The ready bit is also latched as you don’t want the memory system to assert it 
     at a bad point in the cycle*/

int REGS[LC_3b_REGS]; /* register file. */

int MICROINSTRUCTION[CONTROL_STORE_BITS]; /* The microinstruction */

int STATE_NUMBER; /* Current State Number - Provided for debugging */ 

/* For lab 4 */
int INTV; /* Interrupt vector register */
int EXCV; /* Exception vector register */
int SSP; /* Initial value of system stack pointer */
int SSR; /* Colton: best to separate SSP from SSR? */
int PSR;
int BACKUP_REGS[LC_3b_REGS]; /* backup register file lol. */

} System_Latches;

/* Data Structure for Latch */

System_Latches CURRENT_LATCHES, NEXT_LATCHES;

/***************************************************************/
/* A cycle counter.                                            */
/***************************************************************/
int CYCLE_COUNT;

/***************************************************************/
/*                                                             */
/* Procedure : help                                            */
/*                                                             */
/* Purpose   : Print out a list of commands.                   */
/*                                                             */
/***************************************************************/
void help() {                                                    
    printf("----------------LC-3bSIM Help-------------------------\n");
    printf("go               -  run program to completion       \n");
    printf("run n            -  execute program for n cycles    \n");
    printf("mdump low high   -  dump memory from low to high    \n");
    printf("rdump            -  dump the register & bus values  \n");
    printf("?                -  display this help menu          \n");
    printf("quit             -  exit the program                \n\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : cycle                                           */
/*                                                             */
/* Purpose   : Execute a cycle                                 */
/*                                                             */
/***************************************************************/
void cycle() {                                                

  eval_micro_sequencer();   
  cycle_memory();
  eval_bus_drivers();
  drive_bus();
  latch_datapath_values();

  CURRENT_LATCHES = NEXT_LATCHES;

  CYCLE_COUNT++;
}

/***************************************************************/
/*                                                             */
/* Procedure : run n                                           */
/*                                                             */
/* Purpose   : Simulate the LC-3b for n cycles.                 */
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
/* Purpose   : Simulate the LC-3b until HALTed.                 */
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

    printf("\nMemory content [0x%0.4x..0x%0.4x] :\n", start, stop);
    printf("-------------------------------------\n");
    for (address = (start >> 1); address <= (stop >> 1); address++)
    printf("  0x%0.4x (%d) : 0x%0.2x%0.2x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
    printf("\n");

    /* dump the memory contents into the dumpsim file */
    fprintf(dumpsim_file, "\nMemory content [0x%0.4x..0x%0.4x] :\n", start, stop);
    fprintf(dumpsim_file, "-------------------------------------\n");
    for (address = (start >> 1); address <= (stop >> 1); address++)
    fprintf(dumpsim_file, " 0x%0.4x (%d) : 0x%0.2x%0.2x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
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
    printf("Cycle Count  : %d\n", CYCLE_COUNT);
    printf("PC           : 0x%0.4x\n", CURRENT_LATCHES.PC);
    printf("IR           : 0x%0.4x\n", CURRENT_LATCHES.IR);
    printf("STATE_NUMBER : 0x%0.4x\n\n", CURRENT_LATCHES.STATE_NUMBER);
    printf("BUS          : 0x%0.4x\n", BUS);
    printf("MDR          : 0x%0.4x\n", CURRENT_LATCHES.MDR);
    printf("MAR          : 0x%0.4x\n", CURRENT_LATCHES.MAR);
    printf("CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
    printf("Registers:\n");
    for (k = 0; k < LC_3b_REGS; k++)
    printf("%d: 0x%0.4x\n", k, CURRENT_LATCHES.REGS[k]);
    printf("\n");

    /* dump the state information into the dumpsim file */
    fprintf(dumpsim_file, "\nCurrent register/bus values :\n");
    fprintf(dumpsim_file, "-------------------------------------\n");
    fprintf(dumpsim_file, "Cycle Count  : %d\n", CYCLE_COUNT);
    fprintf(dumpsim_file, "PC           : 0x%0.4x\n", CURRENT_LATCHES.PC);
    fprintf(dumpsim_file, "IR           : 0x%0.4x\n", CURRENT_LATCHES.IR);
    fprintf(dumpsim_file, "STATE_NUMBER : 0x%0.4x\n\n", CURRENT_LATCHES.STATE_NUMBER);
    fprintf(dumpsim_file, "BUS          : 0x%0.4x\n", BUS);
    fprintf(dumpsim_file, "MDR          : 0x%0.4x\n", CURRENT_LATCHES.MDR);
    fprintf(dumpsim_file, "MAR          : 0x%0.4x\n", CURRENT_LATCHES.MAR);
    fprintf(dumpsim_file, "CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
    fprintf(dumpsim_file, "Registers:\n");
    for (k = 0; k < LC_3b_REGS; k++)
    fprintf(dumpsim_file, "%d: 0x%0.4x\n", k, CURRENT_LATCHES.REGS[k]);
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
/* Procedure : init_control_store                              */
/*                                                             */
/* Purpose   : Load microprogram into control store ROM        */ 
/*                                                             */
/***************************************************************/
void init_control_store(char *ucode_filename) {                 
    FILE *ucode;
    int i, j, index;
    char line[200];

    printf("Loading Control Store from file: %s\n", ucode_filename);

    /* Open the micro-code file. */
    if ((ucode = fopen(ucode_filename, "r")) == NULL) {
    printf("Error: Can't open micro-code file %s\n", ucode_filename);
    exit(-1);
    }

    /* Read a line for each row in the control store. */
    for(i = 0; i < CONTROL_STORE_ROWS; i++) {
    if (fscanf(ucode, "%[^\n]\n", line) == EOF) {
        printf("Error: Too few lines (%d) in micro-code file: %s\n",
           i, ucode_filename);
        exit(-1);
    }

    /* Put in bits one at a time. */
    index = 0;

    for (j = 0; j < CONTROL_STORE_BITS; j++) {
        /* Needs to find enough bits in line. */
        if (line[index] == '\0') {
        printf("Error: Too few control bits in micro-code file: %s\nLine: %d\n",
               ucode_filename, i);
        exit(-1);
        }
        if (line[index] != '0' && line[index] != '1') {
        printf("Error: Unknown value in micro-code file: %s\nLine: %d, Bit: %d\n",
               ucode_filename, i, j);
        exit(-1);
        }

        /* Set the bit in the Control Store. */
        CONTROL_STORE[i][j] = (line[index] == '0') ? 0:1;
        index++;
    }

    /* Warn about extra bits in line. */
    if (line[index] != '\0')
        printf("Warning: Extra bit(s) in control store file %s. Line: %d\n",
           ucode_filename, i);
    }
    printf("\n");
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

/***************************************************************/
/*                                                             */
/* Procedure : initialize                                      */
/*                                                             */
/* Purpose   : Load microprogram and machine language program  */ 
/*             and set up initial state of the machine.        */
/*                                                             */
/***************************************************************/
void initialize(char *ucode_filename, char *program_filename, int num_prog_files) { 
    int i;
    init_control_store(ucode_filename);

    init_memory();
    for ( i = 0; i < num_prog_files; i++ ) {
    load_program(program_filename);
    while(*program_filename++ != '\0');
    }
    CURRENT_LATCHES.Z = 1;
    CURRENT_LATCHES.STATE_NUMBER = INITIAL_STATE_NUMBER;
    memcpy(CURRENT_LATCHES.MICROINSTRUCTION, CONTROL_STORE[INITIAL_STATE_NUMBER], sizeof(int)*CONTROL_STORE_BITS);
    CURRENT_LATCHES.SSP = 0x3000; /* Initial value of system stack pointer */

    //Colton: AM I MISSING ANY VALUES???
    CURRENT_LATCHES.PSR = 0x8002;   //Consider just changing to a single bit
    CURRENT_LATCHES.EXCV = 0;
    CURRENT_LATCHES.INTV = 0;

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
    if (argc < 3) {
    printf("Error: usage: %s <micro_code_file> <program_file_1> <program_file_2> ...\n",
           argv[0]);
    exit(1);
    }

    printf("LC-3b Simulator\n\n");

    initialize(argv[1], argv[2], argc - 2);

    if ( (dumpsim_file = fopen( "dumpsim", "w" )) == NULL ) {
    printf("Error: Can't open dumpsim file\n");
    exit(-1);
    }

    while (1)
    get_command(dumpsim_file);

}

/***************************************************************/
/* Do not modify the above code, except for the places indicated 
   with a "MODIFY:" comment.

   Do not modify the rdump and mdump functions.

   You are allowed to use the following global variables in your
   code. These are defined above.

   CONTROL_STORE
   MEMORY
   BUS

   CURRENT_LATCHES
   NEXT_LATCHES

   You may define your own local/global variables and functions.
   You may use the functions to get at the control bits defined
   above.

   Begin your code here                          */
/***************************************************************/
int getOpcode(){
    return (CURRENT_LATCHES.IR & 0xF000) >> 12;
}

int getSR1(int *micro){
    int sr1;
    if(GetSR1MUX(micro) == 0)
        sr1 = (CURRENT_LATCHES.IR & 0x0E00) >> 9;
    else if(GetSR1MUX(micro) == 1){
        sr1 = (CURRENT_LATCHES.IR & 0x01C0) >> 6;
    }
    else if(GetSR1MUX(micro) == 2){
        sr1 = 6;
    }

    sr1 = sr1 & 0x07;   /* precaution for weird sext */
    return sr1;
}

int16_t signExtend(uint16_t signedNumber, uint16_t bitsOccupied) {
  int16_t result = signedNumber;
  if(signedNumber & (1 << (bitsOccupied-1))) {
    signedNumber = signedNumber - (1<<bitsOccupied);
    int i = bitsOccupied;
    for(i = bitsOccupied; i < 16; i++) {
      result |= (1<<i);
    }
    if(result != (int16_t) signedNumber) {
      printf("ERROR: SEXT ADDER DIDN'T WORK. ADDER = %d RAW SEXT = %d\n", signedNumber, result);
    }
  }
  return result;          
}

#define Low8bits(x)  ((x)  & 0xFF)
#define High8bits(x)  ((x)  & 0xFF00)

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
  MEMORY[address/2][1] = High8bits(word) >> 8;
  MEMORY[address/2][0] = Low8bits(word);
}

/*Input byte should be formatted in the least significant byte*/
void writeByte(uint16_t address, uint16_t byte) {
  if(address % 2)
    MEMORY[address/2][1] = Low8bits(byte);
  else
    MEMORY[address/2][0] = Low8bits(byte);
}

/*Returns 1 if user mode and 0 if system*/
int getPSRMode() {
    int mode;
    if((CURRENT_LATCHES.PSR & 0x8000) == 0)
        mode = 0;
    else
        mode = 1;
    return mode;
}


int checkForExceptions() {

    int vector = 0;
    bool isTrap = getOpcode() == 15;
    bool inVecTable = (NEXT_LATCHES.MAR < 0x0200);
    bool isOOB = (NEXT_LATCHES.MAR < 0x3000);
    bool isUserMode = (getPSRMode() == 1);
    bool isWordAccess = ( (CURRENT_LATCHES.STATE_NUMBER == 6) || (CURRENT_LATCHES.STATE_NUMBER == 7) );
    bool isOddAddress = ( NEXT_LATCHES.MAR % 2 == 1);
    int nextOpcode = ( (NEXT_LATCHES.IR & 0xF000) >> 12) & 0x0F;
    bool isInvalidOpcode = ( (nextOpcode == 10) || (nextOpcode == 11) );

    //check for protection exception:
    if( !(isTrap && inVecTable) && isOOB && isUserMode )
        vector = 0x02;

    //check for unaligned:
    else if(isWordAccess && isOddAddress)
        vector = 0x03;

    else if(isInvalidOpcode)
        vector = 0x04;

    return vector;
}

bool INT_TRIGGERED = false;
int checkForInterrupts() {
    int vector;
    if(CYCLE_COUNT >= 300){
        vector = 1;
        INT_TRIGGERED = true;
    }
    else{
        vector = 0;
    }
    return vector;
}


int *micro;
void eval_micro_sequencer() {
  /* 
   * Evaluate the address of the next state according to the 
   * micro sequencer logic. Latch the next microinstruction.
   */
   int nextState;
   micro = CURRENT_LATCHES.MICROINSTRUCTION;
   int microIRD = GetIRD(micro);
   int microJ = GetJ(micro);

   /* If state 32, use the opcode to get next state */
   if(microIRD)
        nextState = getOpcode();

    else {
        int microCond = GetCOND(micro);

        switch(microCond){
            /* J bits represent the next state */
            case 0:
                nextState = microJ;
                break;

            /* Wait for Memory Ready */
            case 1:
                if(CURRENT_LATCHES.READY)
                    nextState = microJ + 2;
                else
                    nextState = microJ;
                break;

            /* Branch */
            case 2:
                if(CURRENT_LATCHES.BEN)
                    nextState = 22;
                else
                    nextState = INITIAL_STATE_NUMBER;
                break;

            /* Addressing Mode */
            case 3:
                if((CURRENT_LATCHES.IR & 0x0800) == 0)
                    nextState = 20;
                else
                    nextState = 21;
                break;

            /* Addressing Mode */
            case 4:
                if((CURRENT_LATCHES.INTV == 1) && (getPSRMode() == 1))
                    nextState = microJ + 32;
                else
                    nextState = microJ;
                break;
        }
    }

    /* Latch the next Microinstruction*/
    memcpy(NEXT_LATCHES.MICROINSTRUCTION, CONTROL_STORE[nextState], sizeof(int)*CONTROL_STORE_BITS);
    NEXT_LATCHES.STATE_NUMBER = nextState;
    printf("Current state is %d, and Next state is %d\n", CURRENT_LATCHES.STATE_NUMBER, nextState); 
}


int readyCount = 1;
int FROM_MEM = 0;
void cycle_memory() {
 
  /* 
   * This function emulates memory and the WE logic. 
   * Keep track of which cycle of MEMEN we are dealing with.  
   * If fourth, we need to latch Ready bit at the end of 
   * cycle to prepare microsequencer for the fifth cycle.  
   */
   micro = CURRENT_LATCHES.MICROINSTRUCTION;
   int microCond = GetCOND(micro);

   if((microCond == 1) && (readyCount < 4)){
        NEXT_LATCHES.READY = FALSE;
        readyCount++;
    }

    else if(readyCount == 4){
        NEXT_LATCHES.READY = TRUE;
        readyCount++;
    }

    else if(readyCount == MEM_CYCLES){

        if(GetR_W(micro) == 0){
            
            /* READ */
            if(GetDATA_SIZE(micro) == 0)
                FROM_MEM = readByte(CURRENT_LATCHES.MAR);  /* Byte */
            if(GetDATA_SIZE(micro) == 1)
                FROM_MEM = readWord(CURRENT_LATCHES.MAR); /* Word */
            
        }else if(GetR_W(micro) == 1){

            /* WRITE */
            if(GetDATA_SIZE(micro) == 0)
                writeByte( CURRENT_LATCHES.MAR, CURRENT_LATCHES.MDR );  /* Byte */
            if(GetDATA_SIZE(micro) == 1)
                writeWord( CURRENT_LATCHES.MAR, CURRENT_LATCHES.MDR ); /* Word */

        }

        readyCount = 1;
        NEXT_LATCHES.READY = FALSE;

    }
}


int MARMUX_IN_LEFT;
int MARMUX_IN_RIGHT;
int MARMUX_IN;
int PC_IN;
int ALU_IN;
int SHF_IN;
int MDR_IN;
int VECTOR_IN;
void eval_bus_drivers() {

  /* 
   * Datapath routine emulating operations before driving the bus.
   * Evaluate the input of tristate drivers 
   *         Gate_MARMUX,
   *         Gate_PC,
   *         Gate_ALU,
   *         Gate_SHF,
   *         Gate_MDR.
   */

   micro = CURRENT_LATCHES.MICROINSTRUCTION;


   /* Caluclate MARMUX value */
    /* LSHF(ZEXT(IR[7:0])) */
    MARMUX_IN_LEFT = (CURRENT_LATCHES.IR & 0x00FF) << 1;/* Do this right? */

    /* Evaluate the output of address adder and assign */
    /* Evaluate ADDR1 output */
    int ADDR1_OUT;
    if(GetADDR1MUX(micro) == 0){
        ADDR1_OUT = CURRENT_LATCHES.PC;
    }else{
        ADDR1_OUT = CURRENT_LATCHES.REGS[getSR1(micro)];
    }

    /* Evaluate ADDR2 output */
    int ADDR2_OUT;
    switch(GetADDR2MUX(micro)){
        case 0:
            ADDR2_OUT = 0;
            break;
        case 1:
            ADDR2_OUT = CURRENT_LATCHES.IR & 0x003F;
            ADDR2_OUT = signExtend(ADDR2_OUT, 6);
            break;
        case 2:
            ADDR2_OUT = CURRENT_LATCHES.IR & 0x01FF;
            ADDR2_OUT = signExtend(ADDR2_OUT, 9);
            break;
        case 3:
            ADDR2_OUT = CURRENT_LATCHES.IR & 0x07FF;
            ADDR2_OUT = signExtend(ADDR2_OUT, 11);
            break;
    }
    if(GetLSHF1(micro))
        ADDR2_OUT = ADDR2_OUT << 1;

    MARMUX_IN_RIGHT = ADDR1_OUT + ADDR2_OUT;

   if(GetMARMUX(micro) == 0)
        MARMUX_IN = MARMUX_IN_LEFT;
    else
        MARMUX_IN = MARMUX_IN_RIGHT;
    

   /* PC is always easy */
   PC_IN = CURRENT_LATCHES.PC;


   /* Calculate ALU output */
    int IN_A = CURRENT_LATCHES.REGS[getSR1(micro)];
    int IN_B;
    int BSize = 0;
    if( (CURRENT_LATCHES.IR & 0x0020) == 0){
        IN_B = CURRENT_LATCHES.REGS[CURRENT_LATCHES.IR & 0x0007];
        BSize = 16;
    }
    else{
        IN_B = CURRENT_LATCHES.IR & 0x01F;
        BSize = 5;
    }

    IN_A = signExtend(IN_A, 16);
    IN_B = signExtend(IN_B, BSize);
    switch(GetALUK(micro)){
        case 0: 
            ALU_IN = IN_A + IN_B;
            break;
        case 1:
            ALU_IN = IN_A & IN_B;
            break;
        case 2:
            ALU_IN = IN_A ^ IN_B;
            break;
        case 3:
            ALU_IN = IN_A;
            break;
    }

    ALU_IN = Low16bits(ALU_IN);   /* Mask the lower 16 bits to avoid weird sign issues later */


    /* Gate SHF */
    bool arithmetic;
    bool toRight;

    /* Is it an arithmetic shift? */
    if( (CURRENT_LATCHES.IR & 0x0020) == 0)
        arithmetic = FALSE;
    else
        arithmetic = TRUE;

    /* Is it to the right? */
    if( (CURRENT_LATCHES.IR & 0x0010) == 0)
        toRight = FALSE;
    else
        toRight = TRUE;

    int amount4 = CURRENT_LATCHES.IR & 0x0F;

    /* Implement shift... Edge cases? */
    int MSBit;
    if( (IN_A & 0x8000) != 0)
        MSBit = 1;
    else
        MSBit = 0;

    if(toRight){
        int maskOR = 0x8000;
        int maskAND = 0x7FFF;
        SHF_IN = Low16bits(IN_A) >> amount4;
        int i;
        for(i = 0; i < amount4; i++){

            if(arithmetic && MSBit)     /* also need to fill 1's in the top 16 bits? */
                SHF_IN |= maskOR;
            else
                SHF_IN &= maskAND;
            maskOR = maskOR >> 1;
            maskAND = maskAND >> 1;
        }
    }
    else{
        SHF_IN = Low16bits(IN_A << amount4);
    }


    /* Gate MDR ... MAR[0]??? */
    MDR_IN = Low16bits(CURRENT_LATCHES.MDR);
    /* if(GetDATA_SIZE(micro) == 0) */
        /* MDR_IN = MDR_IN & 0x00FF; */  /* mask to 1 byte */

    /* Value will hold address from the vector table */
    if(CURRENT_LATCHES.EXCV != 0){
        VECTOR_IN = VECTOR_TABLE + (2*CURRENT_LATCHES.EXCV);           
    }
    else if(CURRENT_LATCHES.INTV != 0){
        VECTOR_IN = VECTOR_TABLE + (2*CURRENT_LATCHES.INTV);
    }
    else{
        VECTOR_IN = VECTOR_TABLE;
    }

}

int BUS;
bool jumped34 = false;
void drive_bus() {

  /* 
   * Datapath routine for driving the bus from one of the 5 possible 
   * tristate drivers. 
   */
   int* micro = CURRENT_LATCHES.MICROINSTRUCTION;
   int DRIVER_COUNT = 0;

    if(GetGATE_MARMUX(micro)){
        BUS = Low16bits(MARMUX_IN);
        DRIVER_COUNT++;
    }
    if(GetGATE_PC(micro)){
        BUS = Low16bits(PC_IN);
        DRIVER_COUNT++;
    }
    if(GetGATE_ALU(micro)){
        BUS = Low16bits(ALU_IN);
        DRIVER_COUNT++;
    }
    if(GetGATE_SHF(micro)){
        BUS = Low16bits(SHF_IN);
        DRIVER_COUNT++;
    }
    if(GetGATE_MDR(micro)){
        BUS = Low16bits(MDR_IN);
        DRIVER_COUNT++;
    }
    if(GetGATE_SSR(micro)){
        BUS = Low16bits(CURRENT_LATCHES.SSP);
        DRIVER_COUNT++;
    }
    if(GetGATE_PSR(micro)){
        BUS = Low16bits(CURRENT_LATCHES.PSR);
        DRIVER_COUNT++;
    }
    if(GetGATE_VECTOR(micro)){
        BUS = Low16bits(VECTOR_IN);
        printf("Gating vector address of %d\n", VECTOR_IN);
        DRIVER_COUNT++;

        if(CURRENT_LATCHES.EXCV != 0){
            NEXT_LATCHES.EXCV == 0;
            jumped34 = false;
        }
        else if(CURRENT_LATCHES.INTV != 0){
            NEXT_LATCHES.INTV = 0;
        }
        else{
            printf("How the hell did we get here with no vec set???");
        }
    }


    if(DRIVER_COUNT > 1)
        printf("Too many drivers on the bus\n");

}


bool backingRegsUp = true;
void latch_datapath_values() {

  /* 
   * Datapath routine for computing all functions that need to latch
   * values in the data path at the end of this cycle.  Some values
   * require sourcing the bus; therefore, this routine has to come 
   * after drive_bus.
   */       
   int *micro = CURRENT_LATCHES.MICROINSTRUCTION;

    if(GetLD_MAR(micro)){
        NEXT_LATCHES.MAR = BUS;
   }


    if(GetLD_MDR(micro)){
        
        if(GetMIO_EN(micro)){

            /* LOADS */
            NEXT_LATCHES.MDR = Low16bits(FROM_MEM);

        }else{
            /* MIO value of zero means bus right? */
            if(GetDATA_SIZE(micro) == 0)
                NEXT_LATCHES.MDR = BUS & 0xFF;
            else
                NEXT_LATCHES.MDR = Low16bits(BUS);          
        }
   }


   if(GetLD_IR(micro)){
        NEXT_LATCHES.IR = BUS;
   }


   if(GetLD_BEN(micro)){

        /* This right? */
        int bitN = CURRENT_LATCHES.IR & 0x0800;
        if(bitN != 0)
            bitN = 1;
        int bitZ = CURRENT_LATCHES.IR & 0x0400;
        if(bitZ != 0)
            bitZ = 1;
        int bitP = CURRENT_LATCHES.IR & 0x0200;
        if(bitP != 0)
            bitP = 1;

        if( (CURRENT_LATCHES.N && bitN) || (CURRENT_LATCHES.Z && bitZ) || (CURRENT_LATCHES.P && bitP) )
            NEXT_LATCHES.BEN = TRUE;
        else
            NEXT_LATCHES.BEN = FALSE;
   }


   if(GetLD_REG(micro)){

        int DestR;
        if(GetDRMUX(micro) == 0)
            DestR = (CURRENT_LATCHES.IR & 0x0E00) >> 9;
        else if(GetDRMUX(micro) == 1)
            DestR = 7;
        else
            DestR = 6;

        NEXT_LATCHES.REGS[DestR] = BUS;
   }


   if(GetLD_CC(micro))
   {
        NEXT_LATCHES.N = (signExtend(BUS, 16) < 0);
        NEXT_LATCHES.Z = (BUS == 0);
        NEXT_LATCHES.P = (signExtend(BUS, 16) > 0);
   }


   if(GetLD_PC(micro)){

        switch(GetPCMUX(micro)){
            case 0:
                NEXT_LATCHES.PC = CURRENT_LATCHES.PC + 2;
                break;
            case 1:
                NEXT_LATCHES.PC = BUS;
                break;
            case 2:
                NEXT_LATCHES.PC = MARMUX_IN_RIGHT;
                break;
            case 3:
                NEXT_LATCHES.PC = CURRENT_LATCHES.PC - 2;
                break;
        }
    }


    if(GetLD_SSR(micro)) {
        if(GetSSRMUX(micro) == 0)
            NEXT_LATCHES.SSP = CURRENT_LATCHES.SSP - 2;
        else
            NEXT_LATCHES.SSP = CURRENT_LATCHES.SSP + 2;
    }


    if(GetTOGL_PSR(micro)){
        if(getPSRMode() == 0)
            NEXT_LATCHES.PSR = CURRENT_LATCHES.PSR | 0x8000;
        else
            NEXT_LATCHES.PSR = CURRENT_LATCHES.PSR & 0x7FFF;

    }


    if(GetLD_PSR(micro)){
        NEXT_LATCHES.PSR = BUS;

    }

    //Info Exchanged between reg files?
    if(GetTOGL_PSR(micro)){
        if(backingRegsUp){
            int i;
            for(i=0; i < LC_3b_REGS; i++)
                NEXT_LATCHES.BACKUP_REGS[i] = CURRENT_LATCHES.REGS[i];
            backingRegsUp = false;
        }
        else {
            int i;
            for(i=0; i < LC_3b_REGS; i++)
                NEXT_LATCHES.REGS[i] = CURRENT_LATCHES.BACKUP_REGS[i];
            backingRegsUp = true;
        }
    }

    //last thing before the next cycle starts
    //Check for interrupts
    int iVector = 0;
    if(!INT_TRIGGERED)
        iVector = checkForInterrupts();
    if(iVector > 0)
        NEXT_LATCHES.INTV = iVector;

    //check for exceptions
    int eVector = checkForExceptions();
    int nextState;
    if((eVector > 1) && (!jumped34)) {
        NEXT_LATCHES.EXCV = eVector;
        nextState = 34;
        jumped34 = true;
        memcpy(NEXT_LATCHES.MICROINSTRUCTION, CONTROL_STORE[nextState], sizeof(int)*CONTROL_STORE_BITS);
        NEXT_LATCHES.STATE_NUMBER = nextState;
        printf("JK, Now Next state is %d\n", nextState); 
    }
}

